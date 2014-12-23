#include "pch.h"
#include "CConstants.h"
#include "Database.h"

#include <iomanip>
using namespace std;

extern bool fVerbose;

vector<string> CDatabase::vStopWords;

int CDatabase::IncludeTextFile(const string& path,const string& exts,int boost)
{
    bool analyze=false;
	if (string::npos!=exts.find(string("\"")+CTextFile::GetFileExt(path.c_str(),true)+"\";")) // validate the file extension
    {
        analyze=true;
    }
	LockTextFiles();
	int res=vFileList.size();
	CTextFile *clsFile=new CTextFile(res,boost,path,analyze);
	if (!clsFile->isValid()) // check if it's opened normally
	{
		delete clsFile;
		clsFile=NULL;
		res=-1;
	}else
	{
        vFileList.push_back(clsFile);
	}
	UnlockTextFiles();
	return res;
}

int CDatabase::NumberOfTextFiles()
{
	int res;
	LockTextFiles();
	res=vFileList.size();
	UnlockTextFiles();
	return res;
}

void CDatabase::CloseTextFiles()
{
	LockTextFiles();
	for (vector<CTextFile*>::iterator it=vFileList.begin();it!=vFileList.end();it++)
		(*it)->closefile();
	UnlockTextFiles();
}

CDatabase::CDatabase():vWordIDToNode(NULL)
{
	memset(&trieroot,0,sizeof(struct TRIENODE));
}

CDatabase::~CDatabase()
{
	LockTextFiles();
	for (vector<CTextFile*>::iterator it=vFileList.begin();it!=vFileList.end();it++)
		delete (*it);
	vFileList.clear();
	for (vector<pair<CDynVector<struct TRIENODE>*,CDynVector<struct RECORD>*> >::iterator it=vThreadPool.begin();it!=vThreadPool.end();it++)
	{
		delete it->first;
		delete it->second;
	}
	vThreadPool.clear();
	UnlockTextFiles();
}

void CDatabase::ResetTaskPool()
{
    m_idxTask.set(0);
    m_cntWord.set(0);
    m_cntRecords.set(0);
}

CTextFile* CDatabase::FetchTask()
{
	int res=m_idxTask.inc_return();
	if (res>NumberOfTextFiles()) return NULL; // all of tasks are done or in progress
	return vFileList[res-1];
}

int CDatabase::Hash(const int x) const
{
	int r=(x*HASH_MULTIPLIER)%(sizeof(mtxHashMutex)/sizeof(CCriticalSection));
	assert(r>=0);
	return r;
}

int CDatabase::HashAddr(const void* x) const
{
	return Hash((size_t)x);
}

void CDatabase::LockElement(const int i)
{
	mtxHashMutex[i].enter();
}

void CDatabase::UnlockElement(const int i)
{
	mtxHashMutex[i].leave();
}

int CDatabase::InsertWord(struct TRIENODE *node,const char* szWord,struct RECORD *rec,CDynVector<struct TRIENODE>* vNodes) // szWord must be in lower case
{
	int ch=*szWord;
	int hv;
	assert(node!=NULL);
	if (ch==0) // this is exactly the node of the word
	{
		if (-1==node->iWordID) return -1;
		if (rec!=NULL)
		{
			struct RECORD *prev;
			hv=HashAddr((int*)&node->iWordID);
			LockElement(hv);
			if (!node->iWordID) node->iWordID.set(m_cntWord.inc_return());
			prev=node->records;
			node->records=rec;
			UnlockElement(hv);
			rec->nextrec=prev; // insert this record into the link list
			node->iCntOccurrence.inc();
		}else node->iWordID.set(-1);
		return node->iWordID;
	}else
	{
		ch=CTextFile::chartoidx(ch);
		assert(ch>=0 && ch<sizeof(node->next)/sizeof(struct TRIENODE*));
		struct TRIENODE *tmp;
		tmp=node->next[ch];
		if (NULL==tmp)
		{
			bool fwritten=false;
			tmp=vNodes->push_back(*(struct TRIENODE*)0);
			assert(tmp!=NULL);
			hv=HashAddr(&node->next[ch]);
			LockElement(hv);
			if (NULL==node->next[ch])
				node->next[ch]=tmp;
			else
				fwritten=true;
			UnlockElement(hv);
			if (fwritten)
			{
				tmp=node->next[ch];
				vNodes->decSize();
			}else tmp->parent=node;
		}
		return InsertWord(tmp,szWord+1,rec,vNodes);
	}
}

bool CDatabase::ThreadEntry()
{
	int cntTasks=0;
	CTextFile* cFile;
	char bufWord[MAX_WORD_LENGTH+1]={0};

	// Allocate necessary memory for our data structures
	CDynVector<struct TRIENODE>* vNodes=new CDynVector<struct TRIENODE>(NODES_MALLOC_DELTA);
	CDynVector<struct RECORD>* vRecords=new CDynVector<struct RECORD>(RECORDS_MALLOC_DELTA);
	assert(vNodes!=NULL && vRecords!=NULL);
	if (vNodes==NULL || vRecords==NULL) return false;

	mtxThreadPool.enter();
	vThreadPool.push_back(make_pair(vNodes,vRecords));
	mtxThreadPool.leave();

	// Insert stop words first
    for (vector<string>::const_iterator it=vStopWords.begin();it!=vStopWords.end();it++)
        InsertWord(&trieroot,it->c_str(),NULL,vNodes);

	// Now we can start
	clock_t clstart=clock();
	while (NULL!=(cFile=FetchTask()))
	{
        if (!cFile->isValid()) continue;
		cntTasks++;
		if (!cFile->needProcess()) continue;
		int pos;
		cFile->reOpen();
		assert(cFile->isOpen());
		while (-1!=(pos=cFile->ReadWord(sizeof(bufWord)-1,bufWord)))
		{
			struct RECORD thisRec={cFile->getFileID(),pos,NULL};
			if (-1==cFile->ReportWordID(InsertWord(&trieroot,bufWord,vRecords->push_back(thisRec),vNodes)))
				vRecords->decSize();
		}
		if (fVerbose)
		{
			mtxThreadPool.enter();
			cout<<"["<<cFile->getFileName()<<"] Word Count="<<cFile->getWordCount()<<endl;
			mtxThreadPool.leave();
		}
		cFile->closefile();
	}
	mtxThreadPool.enter();
	cout<<"Thread: done in "<<setprecision(3)<<((float)(clock()-clstart))/CLOCKS_PER_SEC<<"s Tasks: "<<cntTasks<<" Nodes: "<<vNodes->size()<<" Records: "<<vRecords->size()<<endl;
	mtxThreadPool.leave();
	m_cntRecords.add(vRecords->size());
	return cntTasks>0;
}

int CDatabase::enumtree(const struct TRIENODE* node)
{
	int cnt=0;
	assert(node!=NULL);
	if ((int)node->iWordID>0)
	{
		(*vWordIDToNode)[node->iWordID]=node;
		cnt+=node->iCntOccurrence;
	}
	for (int i=0;i<sizeof(node->next)/sizeof(struct TRIENODE*);i++)
	{
		if (node->next[i]!=NULL) cnt+=enumtree(node->next[i]);
	}
	return cnt;
}

bool CDatabase::BuildWordIDToNodeTable()
{
	// assert(m_cntWord>0);
	if (NULL!=vWordIDToNode) delete vWordIDToNode;
	vWordIDToNode=new vector<const struct TRIENODE*>(m_cntWord+1);
	assert(vWordIDToNode!=NULL);
	if (vWordIDToNode!=NULL)
	{
		int ret=enumtree(&trieroot);
		return true;
	}else return false;
}

string CDatabase::FindWordByNode(const struct TRIENODE* node) const
{
	assert(node!=NULL);
	if (node==NULL || node->parent==NULL) return "";
	const struct TRIENODE* parent=node->parent;
	for (int i=0;i<sizeof(parent->next)/sizeof(struct TRIENODE*);i++)
	{
		if (parent->next[i]==node) return FindWordByNode(parent)+CTextFile::idxtochar(i);
	}
	assert(false);
	return "!";
}

string CDatabase::FindWordByID(const uint32_t idxWord) const
{
	return FindWordByNode(FindWordNode(idxWord));
}

uint32_t CDatabase::FindWordID(const char *szWord) const
{
	const struct TRIENODE *node=&trieroot;
	int ch;
	assert(node!=NULL);
	for (;ch=*szWord;szWord++)
	{
		switch (ch)
		{
		case '\r':
		case '\t':
		case '\n':
		case ' ':
			goto foundthenode;break;
		}
		if ((ch=CTextFile::chartoidx(ch))<0) return -1;// invalid word
		if (NULL==(node=node->next[ch])) return 0; // not found
	}
foundthenode:
	assert(node!=NULL);
	return node->iWordID;// it may be 0 or -1
}

int CDatabase::SearchName(const vector<string> &vWords,CResultPool &cResult,const int idxDB)
{
    assert(vWords.size()>0);
    LockTextFiles();
    for (size_t k=0;k<vWords.size();k++)
    {
        CResultPool test;
        for (vector<CTextFile*>::const_iterator it=vFileList.begin();it!=vFileList.end();it++)
        {
            CTextFile *file = *it;
            size_t pos = file->getLowerCaseFileName().find(vWords[k]);
            if (pos!=string::npos)
            {
                test.push_back(CResultPool::MakeRESID(idxDB,file->getFileID(),file->getBonus()),pos,vWords[k].length()-file->getLowerCaseFileName().length());
            }
        }
        if (k==0) cResult=test;else cResult=cResult*test;
    }
    UnlockTextFiles();
    return 1;
}

int CDatabase::SearchPhrase(const vector<string> &vWords,CResultPool &cResult,const int idxDB)
{
	size_t nResults=0;
	assert(vWords.size()>0);
	vector<int> vWordsID(vWords.size());

	for (size_t i=0;i<vWords.size();i++)
	{
		if ((vWordsID[i]=FindWordID(vWords[i].c_str()))<=0) return -1;
	}

	for (const struct RECORD* rec=FindWordNode(vWordsID[0])->records;rec!=NULL;)
	{
		const CTextFile &cText=this->operator[](rec->iFileID);
		size_t index;
		if (cText.findWordbyPos(rec->iPosition,&index)!=(streampos)-1)
		{
			int j,k;
			size_t i;
			for (i=1,k=index+1,j=0;i<vWords.size();k++)
			{
				int pos;
				int idWord;
				idWord=cText.findWordbyIndex(k,&pos);
				if ((int)idWord<=0) goto jNextRecord;
				if (idWord==vWordsID[i] || vWordsID[i]==-1) {i++;j++;}
				if (pos-(rec->iPosition)-j>TOLERANCE_DISTANCE_CATENATION) goto jNextRecord;
			}if (i<vWords.size()) goto jNextRecord;
		}else break;
		cResult.push_back(CResultPool::MakeRESID(idxDB,rec->iFileID),rec->iPosition);
		++nResults;
jNextRecord:
		rec=rec->nextrec;
	}
	return nResults;
}

const struct TRIENODE* CDatabase::FindWordNode(const uint32_t idxWord) const
{
	assert(vWordIDToNode!=NULL);
	assert(idxWord!=-1 && 0!=idxWord && idxWord<=m_cntWord);
	if (vWordIDToNode!=NULL && idxWord<=m_cntWord)
		return vWordIDToNode->operator[](idxWord);
	else return NULL;
}

void CDatabase::setStopWords(const vector<string>& v)
{
	vStopWords=v;
}

struct DATABASE_SERIALIZATION
{
    int numFiles;
    int cntWord;
    int cntRecords;
};

bool CDatabase::save(const string& dbFile)
{
    FILE* fp=fopen(dbFile.c_str(),"wb");
    if (NULL==fp) return false;
    LockTextFiles();
    DATABASE_SERIALIZATION d;
    d.numFiles=vFileList.size();
    d.cntWord=m_cntWord;
    d.cntRecords=m_cntRecords;
    fwrite(&d,sizeof(d),1,fp);
    for (vector<CTextFile*>::iterator it=vFileList.begin();it!=vFileList.end();it++)
    {
        CTextFile *tf=*it;
        tf->store(fp);
    }
    UnlockTextFiles();
    saveTrieTree(&trieroot,fp);
    fclose(fp);
    return true;
}

int countRecords(struct RECORD *rec)
{
    int ans=0;
    while (NULL!=rec)
    {
        ++ans;
        rec=rec->nextrec;
    }
    return ans;
}

struct NODE_SERIALIZATION
{
    int iWordID;
    int iCntOccurrence;
    int nRecordCount;
};

struct RECORD_SERIALIZATION
{
    int iFileID;
	int iPosition;
};

int CDatabase::loadTrieTree(struct TRIENODE* node, FILE* fp, const struct TRIENODE* parent, CDynVector<struct TRIENODE>* vNodes, CDynVector<struct RECORD>* vRecords)
{
	int cnt=0;
	assert(node!=NULL);
	node->parent=const_cast<TRIENODE*>(parent);
	{
        NODE_SERIALIZATION d;
        fread(&d,sizeof(d),1,fp);
        node->iWordID = d.iWordID;;
        node->iCntOccurrence = d.iCntOccurrence;
        struct RECORD *last = NULL;
        for (int i=0;i<d.nRecordCount;i++)
        {
            RECORD_SERIALIZATION rd;
            fread(&rd,sizeof(rd),1,fp);
            struct RECORD r, *p;
            r.iFileID = rd.iFileID;
            r.iPosition = rd.iPosition;
            p=vRecords->push_back(r);

            if (node->records==NULL) node->records=p;
            if (last!=NULL) last->nextrec=p;
            last=p;
            ++cnt;
        }
        if (last!=NULL) last->nextrec=NULL;
    }
	for (int i=0;i<sizeof(node->next)/sizeof(struct TRIENODE*);i++)
	{
        char ch = fgetc(fp);
        if (ch=='0')
        {
            node->next[i]=NULL;
        }else if (ch=='1')
        {
            node->next[i]=vNodes->push_back(*(struct TRIENODE*)0);
            cnt+=loadTrieTree(node->next[i],fp,node,vNodes,vRecords);
        }
	}
	return cnt;
}

int CDatabase::saveTrieTree(const struct TRIENODE* node, FILE* fp)
{
	int cnt=0;
	{
        NODE_SERIALIZATION d;
        d.iWordID = node->iWordID;
        d.iCntOccurrence = node->iCntOccurrence;
        d.nRecordCount = countRecords(node->records);
        fwrite(&d,sizeof(d),1,fp);
        struct RECORD *p = node->records;
        for (int i=0;i<d.nRecordCount;i++)
        {
            RECORD_SERIALIZATION rd;
            rd.iFileID = p->iFileID;
            rd.iPosition = p->iPosition;
            fwrite(&rd,sizeof(rd),1,fp);
            p=p->nextrec;
        }
    }
	for (int i=0;i<sizeof(node->next)/sizeof(struct TRIENODE*);i++)
	{
		if (node->next[i]!=NULL)
		{
            fputc('1',fp);
            cnt+=saveTrieTree(node->next[i],fp);
		}else
		{
            fputc('0',fp);
		}
	}
	return cnt;
}

bool CDatabase::load(const string& dbFile)
{
    FILE* fp=fopen(dbFile.c_str(),"rb");
    if (NULL==fp) return false;
    LockTextFiles();
    DATABASE_SERIALIZATION d;
    d.numFiles=vFileList.size();
    d.cntWord=m_cntWord;
    d.cntRecords=m_cntRecords;
    fread(&d,sizeof(d),1,fp);
    m_cntWord=d.cntWord;
    m_cntRecords=d.cntRecords;
    m_idxTask=d.numFiles+1;
    for (int i=0;i<d.numFiles;i++)
    {
        CTextFile* tf= new CTextFile(fp);
        vFileList.push_back(tf);
        //cout<<"loaded "<<tf->getFilePath()<<endl;
    }
    UnlockTextFiles();
    CDynVector<struct TRIENODE>* vNodes=new CDynVector<struct TRIENODE>(NODES_MALLOC_DELTA);
	CDynVector<struct RECORD>* vRecords=new CDynVector<struct RECORD>(RECORDS_MALLOC_DELTA);
	if (vNodes!=NULL && vRecords!=NULL)
	{
        vThreadPool.push_back(make_pair(vNodes,vRecords));
        int ret=loadTrieTree(&trieroot,fp,NULL,vNodes,vRecords);
        assert(ret==m_cntRecords);
	}

    fclose(fp);
    return true;
}
