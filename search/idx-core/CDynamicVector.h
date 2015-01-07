#ifndef CDYNAMICVECTOR_H_INCLUDED
#define CDYNAMICVECTOR_H_INCLUDED

#include <vector>
#include "AtomicInteger.h"
#include "CCriticalSection.h"

using namespace std;

template <class T>
class CDynVector
{
private:
	vector<T*> vVectors;

	size_t m_nItemsPerVector;
	size_t m_nItemsPerVectorMask;
	char m_nItemsPerVectorShiftBits;
	bool m_fZeroMem;
	volatile int m_nSize,m_nNumber;

	T* newBlock()
	{
		size_t cbBlock=m_nItemsPerVector*sizeof(T)+64;
		T* tmp=(T*)malloc(cbBlock); // Optimize for memset() with SSE instructions
		assert(tmp!=NULL);
		// TODO: ALIGNED MALLOC
		// assert(0==(((size_t)tmp)&0x3F)); // check if it's correctly aligned
		if (m_fZeroMem)
		{
            memset(tmp,0,m_nItemsPerVector*sizeof(T));
		}
		return tmp;
	}

	bool readyBlock(const size_t inum)
	{
		bool ret=true;
		if (inum>=m_nNumber) // maybe in need of new block
		{
			while (inum>=m_nNumber)
			{
				T* mem=newBlock();
				if (NULL!=mem)
				{
					vVectors.push_back(mem);
					m_nNumber++;
				}else ret=false;
			}
		}
		return ret;
	}

	CDynVector();
	CDynVector(const CDynVector&);

public:
	CDynVector(const size_t nItemsPerVector,const bool fZeroMem=true):m_fZeroMem(fZeroMem)
	{
		assert(nItemsPerVector>0);
		m_nSize=0;
		m_nNumber=0;
		char i;
		for (i=0;(((size_t)1)<<i)<nItemsPerVector;i++);
		m_nItemsPerVectorShiftBits=i;
		m_nItemsPerVector=((size_t)1)<<i;
		m_nItemsPerVectorMask=((size_t)1<<i)-1;
	}

	~CDynVector()
	{
		for (typename vector<T*>::const_iterator it=vVectors.begin();it!=vVectors.end();it++)
		{
			free((*it));
		}
		vVectors.clear();
		m_nNumber=0;
		m_nSize=0;
	}

	T* push_back(const T& v)
	{
		size_t idx,inum;
		T* ret;
		idx=m_nSize++;
		inum=idx>>m_nItemsPerVectorShiftBits;
		if (!readyBlock(inum)) return NULL;

		assert(m_nNumber==vVectors.size());
		assert(inum<m_nNumber);
		ret=&vVectors[inum][idx&m_nItemsPerVectorMask];
		if (NULL!=&v) *ret=v;
		return ret;
	}

	T& operator [](size_t pos)
	{
		T* ret=NULL;
		if (pos<m_nSize)
		{
			ret=&vVectors[pos>>m_nItemsPerVectorShiftBits][pos&m_nItemsPerVectorMask];
		}
		return *ret;
	}

    int incSize() {return ++m_nSize;}
	int decSize() {return --m_nSize;}
	int reSize(const int newSize) {return m_nSize=newSize;}

	int size() const{return m_nSize;}
	int number() const{return m_nNumber;}
};

#endif // CDYNAMICVECTOR_H_INCLUDED
