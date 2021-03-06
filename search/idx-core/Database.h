#ifndef DATABASE_H
#define DATABASE_H

#include "CConstants.h"
#include "CCriticalSection.h"
#include "CTextFile.h"
#include "CResultPool.h"
#include "AtomicInteger.h"
#include <vector>
using namespace std;

struct RECORD
{
	int iFileID;
	int iPosition;
	struct RECORD* nextrec;
};

struct TRIENODE // node of Trie Tree
{
	AtomicInteger iWordID; // negative when in excluding dictionary
	AtomicInteger iCntOccurrence;
	struct RECORD* records;
	struct TRIENODE* parent;
	struct TRIENODE* next[28]; // 0-25 for a-z ; 26 for hyphen - ; 27 for '
}; //32*4=128 bytes. good align!


class CDatabase: public Object
{
private:
	vector<CTextFile*> vFileList;CCriticalSection mtxFileList;

	struct TRIENODE trieroot;
	vector<const struct TRIENODE*>* vWordIDToNode;
	static vector<string> vStopWords;

	AtomicInteger m_idxTask,m_cntWord,m_cntRecords;
	CCriticalSection mtxHashMutex[NUMBER_OF_HASH_MUTEX];

	vector<pair<CDynVector<struct TRIENODE>*,CDynVector<struct RECORD>*> > vThreadPool;
	CCriticalSection mtxThreadPool;

	CDatabase(const CDatabase&); // disable copy constructor
public:
	CDatabase();
	~CDatabase();

	// Dictionary manger
	static void setStopWords(const vector<string>& v);

	// Load/Save
	bool load(const string& dbFile);
	bool save(const string& dbFile);
	int saveTrieTree(const struct TRIENODE* node, gzFile& fp);
	int loadTrieTree(struct TRIENODE* node, gzFile& fp, const struct TRIENODE* parent, CDynVector<struct TRIENODE>* vNodes, CDynVector<struct RECORD>* vRecords);

	// Text file manager
	int IncludeTextFile(void *seg,const string& path,const string& validExts,int boost); // return a unique id of the text file
	int NumberOfTextFiles();
	void LockTextFiles() {mtxFileList.enter();}
	void UnlockTextFiles() {mtxFileList.leave();}
	void CloseTextFiles();
	const CTextFile& operator[](const size_t i)
	{
		CTextFile *tmp=NULL;
		LockTextFiles();
		if (i<vFileList.size()) tmp=vFileList[i];
		UnlockTextFiles();
		return *tmp;
	}

	// Multi-threaded indexing
	bool ThreadEntry();
	void ResetTaskPool();
	CTextFile* FetchTask();
	int Hash(const int x) const;
	int HashAddr(const void* x) const;
	void LockElement(const int i);
	void UnlockElement(const int i);

	int InsertWord(struct TRIENODE *node,const char* szWord,struct RECORD *rec,CDynVector<struct TRIENODE>* vNodes);

	// Search
	bool BuildWordIDToNodeTable();
	int enumtree(const struct TRIENODE* node);
	uint32_t FindWordID(const char *szWord) const; // szWord -> WordID
	string FindWordByID(const uint32_t idxWord) const; // WordID -> szWord
	string FindWordByNode(const struct TRIENODE* node) const; // Node -> szWord
	const struct TRIENODE* FindWordNode(const uint32_t idxWord) const; // WordID -> Node

	int SearchPhrase(const vector<string> &vWords, CResultPool &cResult, const int idxDB);
	int SearchName(const vector<string> &vWords, CResultPool &cResult, const int idxDB);
};
#endif
