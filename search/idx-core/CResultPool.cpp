#include "pch.h"
#include "CResultPool.h"
#include "CConstants.h"

void CResultPool::sortbyPos()
{
	if (m_sortkey!=1)
	{
		m_sortkey=1;
		sort(vResults.begin(),vResults.end(),cmpPos);
	}
}

void CResultPool::sortbyRel()
{
	if (m_sortkey!=2)
	{
		m_sortkey=2;
		sort(vResults.begin(),vResults.end(),cmpRel);
	}
}

bool CResultPool::cmpPos(const RESULTTYPE &x,const RESULTTYPE &y)
{
	if (resid(x)<resid(y)) return true;
	if (resid(x)>resid(y)) return false;
	return wpos(x)<wpos(y);
}

bool CResultPool::cmpRel(const RESULTTYPE &x,const RESULTTYPE &y)
{
	return relevance(x)>relevance(y);
}

void CResultPool::copyFrom(const CResultPool& src)
{
	this->clear();
	this->m_sortkey=src.m_sortkey;
	this->vResults.assign(src.vResults.begin(),src.vResults.end());
}

CResultPool& operator +(CResultPool &a,CResultPool &b) // Merge Sort Algorithm
{
	CResultPool *tmp=new CResultPool(a.size()+b.size());
	assert(tmp!=NULL);
	if (tmp!=NULL)
	{
		a.sortbyPos();b.sortbyPos();
		//merge(a.vResults.begin(),a.vResults.end(),b.vResults.begin(),b.vResults.end(),tmp->vResults.begin()); No idea why it doesn't work. Anyway I can make it by myself.

		vector<RESULTTYPE>::const_iterator it1,it2,it;
		for (it1=a.begin(),it2=b.begin();it1!=a.end() && it2!=b.end();)
		{
			if (CResultPool::cmpPos(*it1,*it2))
				it=it1++;else it=it2++;
			tmp->push_back(*it);
		}
		for (;it1!=a.end();it1++) tmp->push_back(*it1);
		for (;it2!=b.end();it2++) tmp->push_back(*it2);
		assert(tmp->size()==a.size()+b.size());

		tmp->m_sortkey=1;
	}
	return *tmp;
}

int CResultPool::weigh_dist(const int dist)
{
	if (dist<TOLERANCE_VERY_RELEVANT_DISTANCE) return 3;
	else if (dist<TOLERANCE_RELEVANT_DISTANCE) return 1;
	else return 0;
}

CResultPool& operator *(CResultPool &a,CResultPool &b)
{
	CResultPool *tmp=new CResultPool();
	assert(tmp!=NULL);
	if (tmp!=NULL)
	{
		a.sortbyPos();b.sortbyPos();
		vector<RESULTTYPE>::const_iterator it1,it2,it;
		for (it1=a.begin(),it2=b.begin();it1!=a.end() && it2!=b.end();)
		{
			RESID r1=CResultPool::resid(*it1),r2;
			for (;it2!=b.end() && CResultPool::resid(*it2)<r1;it2++);
			r2=r1;
			if (it2!=b.end() && r1==(r2=CResultPool::resid(*it2))) // same resource
			{
				int wp1=CResultPool::wpos(*it1);
				int mindist=abs(wp1-CResultPool::wpos(*it2));it=it2;

				for (;it2!=b.end() && CResultPool::resid(*it2)==r1 && abs(wp1-CResultPool::wpos(*it2))<mindist;it2++)
				{
					it=it2;
					mindist=abs(wp1-CResultPool::wpos(*it2));
				} // find nearest occurrence

				tmp->vResults.push_back(make_pair(make_pair(r1,wp1),CResultPool::weigh_dist(mindist)+CResultPool::relevance(*it1)+CResultPool::relevance(*it2)));

				it1++;it2=it;
			}else for (;it1!=a.end() && CResultPool::resid(*it1)<r2;it1++); // skip the resource
		}
		tmp->m_sortkey=1;
	}
	return *tmp;
}

void CResultPool::organize() // Calculate total relevance score of each file. Then sort the files by relevance.
{
	sortbyPos();
	vector<RESULTTYPE>::iterator it,it1,it2,itmax;
	for (it=it1=vResults.begin();it1!=vResults.end();*it++=*it1,it1=it2)
	{
		int sumScore=0,maxScore=it1->second;
		itmax=it1;
		for (it2=it1+1;it2!=vResults.end() && resid(*it1)==resid(*it2);it2++)
		{
			sumScore+=it2->second;
			if (it2->second>maxScore) {maxScore=it2->second;itmax=it2;}
		}
		it1->first.second=itmax->first.second;
		it1->second+=sumScore+CResultPool::RESIDtoBonus(it1->first.first);
	}
	vResults.resize(it-vResults.begin());
	sortbyRel();
}
