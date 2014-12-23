#ifndef CRESULTPOOL_H_INCLUDED
#define CRESULTPOOL_H_INCLUDED

#include <algorithm>
#include <vector>
using namespace std;

typedef uint64_t RESID;
typedef pair<pair<RESID,int>,int> RESULTTYPE;

class CResultPool
{
private:
	vector<RESULTTYPE> vResults; // <<Resource ID,Word Position>,Relevance>
	int m_sortkey;

	static bool cmpPos(const RESULTTYPE &x,const RESULTTYPE &y);
	static bool cmpRel(const RESULTTYPE &x,const RESULTTYPE &y);

public:
	static RESID MakeRESID(const int idxDB,const int idxFile,const int bonus=0)
	{
		assert(0==(idxDB&(~0xFF)));
		assert(0==(idxFile&(~0xFFFFFF)));
		return (idxDB<<24)|idxFile|( ((uint64_t)bonus)<<32 );
	}
	static int RESIDtoDBID(const RESID x) {return (x>>24)&0xFF;}
	static int RESIDtoFileID(const RESID x) {return x&0xFFFFFF;}
	static int RESIDtoBonus(const RESID x) {return x>>32;}

	static int wpos(const RESULTTYPE& x) {return x.first.second;}
	static int relevance(const RESULTTYPE& x) {return x.second;}
	static RESID resid(const RESULTTYPE& x) {return x.first.first;}
	CResultPool(const size_t defsize):m_sortkey(0) {vResults.reserve(defsize);}
	CResultPool():m_sortkey(0){};

	// copy
	void copyFrom(const CResultPool& src);
	CResultPool(const CResultPool& src) {copyFrom(src);}
	CResultPool& operator =(const CResultPool& src) {copyFrom(src);return *this;}
	// insert
	void push_back(const RESID x,const int iPos,const int iRel=1) {vResults.push_back(make_pair(make_pair(x,iPos),iRel));m_sortkey=0;}
	void push_back(const RESULTTYPE &x) {vResults.push_back(x);m_sortkey=0;}
	// sort
	void sortbyPos();
	void sortbyRel();
	// join
	friend CResultPool& operator +(CResultPool &a,CResultPool &b);
	friend CResultPool& operator *(CResultPool &a,CResultPool &b);
	// organize
	void organize();
	// weigh
	static int weigh_dist(const int dist);
	// iterator
	vector<RESULTTYPE>::const_iterator begin() const{return vResults.begin();}
	vector<RESULTTYPE>::const_iterator end() const{return vResults.end();}


	size_t size() const{return vResults.size();}
	void clear() {vResults.clear();m_sortkey=0;}
};

#endif // CRESULTPOOL_H_INCLUDED
