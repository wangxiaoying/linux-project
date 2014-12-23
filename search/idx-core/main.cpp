#include "pch.h"
#include "style.h"
#include "AtomicInteger.h"
#include "Object.h"
#include "Database.h"
#include "CDynamicVector.h"
#include "CThread.h"
#include "searching.h"

#include <algorithm>
#include <string>
using namespace std;

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <unistd.h>

bool fVerbose=false,fRecursive=false;

void* BuildingWorkerThreadProc(void* lpParam)
{
	CThread* me=(CThread*)lpParam;
	CDatabase* clsDB=(CDatabase*)me->getlpParam();
	clsDB->ThreadEntry();
	return me->exit();
}

template <typename T1,typename T2>
bool cmpPair2nd(const pair<T1,T2> &a,const pair<T1,T2> &b)
{
	return a.second>b.second;
}

int NumberOfProcessors()
{
    int ret=sysconf(_SC_NPROCESSORS_ONLN);
    printf("%d cores installed\n",ret);
    return ret;
}

void printusage(char* strAppPath)
{
	cout<<"NAME"<<endl<<"\tkindexing - search engine based on inverted index"<<endl;
	cout<<"USAGE"<<endl<<"\tkindexing [-r] [-v] [-ext extension] [-hexie dictfile] [directory1] [directory2] ... [directoryn]"<<endl;
	cout<<"OPTIONS"<<endl;
	cout<<"\t-r, -R, --recursive"<<endl<<"\t\tsearch for text files recursively\n"<<endl;
	cout<<"\t-v, -V"<<endl<<"\t\tverbose\n"<<endl;
	cout<<"\t-ext extension"<<endl<<"\t\tspecify one valid extension of text files to be indexed\n"<<endl;
	cout<<"\t-hexie dictfile"<<endl<<"\t\texclude words listed in the file when building index\n"<<endl;
}

long searchdir(const string& parentdirectory,vector<pair<string,int> >& vFiles,int boost)
{
    DIR *dirptr;
    long cnt;
    struct dirent* entry;
    if (NULL==(dirptr=opendir(parentdirectory.c_str())))
    {
        cerr<<"[X] failed to call opendir("<<parentdirectory<<")"<<endl;
        return 0;
    }
    while (NULL!=(entry=readdir(dirptr)))
    {
        if (!strcmp(entry->d_name,".") || !strcmp(entry->d_name,"..")) continue;

        string szPath = parentdirectory+"/"+string(entry->d_name);

        struct stat64 filestat;
        if (lstat64(szPath.c_str(),&filestat)<0)
        {
            fprintf(stderr,"[X] failed to call lstat(%s) %d\n",entry->d_name,errno);
            continue;
        }
        if (S_ISDIR(filestat.st_mode))
        {
            vFiles.push_back(make_pair(szPath,BOOST_DIRECTORY));
            if (fRecursive)
                cnt+=searchdir(szPath,vFiles,boost);
        }else
        {
            long long llFileSize = filestat.st_size;
            if (llFileSize>0) // ignore empty files
            {
                vFiles.push_back(make_pair(szPath,boost));
                if (fVerbose) cout<<"added "<<szPath<<endl;
                ++cnt;
            }
        }
    }
    closedir(dirptr);
    return cnt;
}

extern "C" void idx_set_verbose(bool enabled)
{
    fVerbose=enabled;
}

extern "C" int idx_set_stopwords(const char *dictfile)
{
    ifstream fin(dictfile);
    if (!fin)
    {
        return 1;
    }
    else
    {
        vector<string> stopWords;
        string sWord;
        while (fin>>sWord)
        {
            string::size_type n=sWord.length();
            for (int n=sWord.length();n>0 && (sWord[n-1]==L'\n' || sWord[n-1]==L'\r');n--);
            if (n>0)
            {
                sWord=sWord.substr(0,n);
                transform(sWord.begin(), sWord.end(), sWord.begin(), (int (*)(int))tolower);
                stopWords.push_back(sWord);
                if (fVerbose)
                {
                    cout<<"Stop word: "<<sWord<<endl;
                }
            }
        }
        CDatabase::setStopWords(stopWords);
        fin.close();
        return 0;
    }
}

extern "C" int idx_add_documents(const char *directory, bool recursive, void **collection, int boost)
{
    vector<pair<string,int> > *vFiles;
    if (NULL==collection)
        return 1;
    else if (NULL==*collection)
    {
        vFiles=new vector<pair<string,int> >;
        *collection=vFiles;
    }
    else
    {
        vFiles=(vector<pair<string,int> >*)(*collection);
    }
    fRecursive=recursive;
    cout<<"Searching for files...";
    searchdir(string(directory),*vFiles,boost);
    cout<<"ok ("<<vFiles->size()<<" files in total)"<<endl;
    return 0;
}

struct TASK_DESC
{
    void* files;
    const char *filterExt;
    void **db;
    struct PREFERENCES pref;
    int done;
};

int __create_database(void* files, const char *filterExt, void **db, bool multi_core, bool pinyin_conversion)
{
    string strExts;
    if (filterExt!=NULL)
        strExts=filterExt;
    else
        strExts="\"txt\";";

    cout<<"Building index database (it may take a long time)..."<<endl;
    // Add Text files to Database
    CDatabase *dbDefault=new CDatabase;
    assert(dbDefault!=NULL);
    if (dbDefault==NULL) return 1;

    if (files!=NULL)
    {
        vector<pair<string,int> > *vFiles = (vector<pair<string,int> >*)files;
        sort(vFiles->begin(),vFiles->end(),cmpPair2nd<string,int>);
        for (vector<pair<string,int> >::const_iterator it=vFiles->begin();it!=vFiles->end();it++)
        {
            if (fVerbose)
            {
                cout<<it->first<<endl;
            }
            dbDefault->IncludeTextFile(it->first,strExts,it->second);
        }
    }
    // Create thread pool and assign jobs
    vector<CThread*> vWorkerThread(multi_core?NumberOfProcessors():1);
    dbDefault->ResetTaskPool();
    // Begin processing
    for (size_t i=0;i<vWorkerThread.size();i++)
    {
        vWorkerThread[i]=new CThread(BuildingWorkerThreadProc,dbDefault);
    }
    // Wait for all tasks to be done
    for (size_t i=0;i<vWorkerThread.size();i++)
    {
        vWorkerThread[i]->waitforme();
        delete vWorkerThread[i];
    }
    assert(NULL==dbDefault->FetchTask());
    vWorkerThread.clear();
    cout<<"Finalizing (just a moment)...";
    dbDefault->BuildWordIDToNodeTable();
    cout<<"done"<<endl;
    *db=(void*)dbDefault;
    return 0;
}

void* TaskProc(void* lpParam)
{
	CThread* me=(CThread*)lpParam;
	struct TASK_DESC* task=(struct TASK_DESC*)me->getlpParam();
	task->done=__create_database(task->files,task->filterExt,task->db,task->pref.multi_core,task->pref.pinyin_search);
	return me->exit();
}

extern "C" int* idx_create_database_task(void* files, const void *ppref, void **db)
{
    struct TASK_DESC *task = new TASK_DESC;
    string exts;
    task->db=db;
    task->files=files;
    task->pref=*(const struct PREFERENCES*)ppref;
    if (task->pref.file_c)
    {
        exts+="\"c\";";
        exts+="\"cpp\";";
        exts+="\"h\";";
        exts+="\"py\";";
        exts+="\"ruby\";";
    }
    if (task->pref.file_h)
    {
        exts+="\"htm\";";
        exts+="\"html\";";
        exts+="\"xml\";";
    }
    if (task->pref.file_txt)
    {
        exts+="\"txt\";";
        exts+="\"text\";";
    }
    task->filterExt=strdup(exts.c_str());
    task->done=2333;
    CThread *thread = new CThread(TaskProc,(void*)task);
    return &task->done;
}

extern "C" int idx_load_database(const char *dbfile, void **db)
{
    CDatabase *data = new CDatabase;
    if (data->load(dbfile))
    {
        *db=(void*)data;
        return 0;
    }else
        return 1;
}

extern "C" void idx_delete_database(void *db)
{
    delete (CDatabase*)db;
}

extern "C" int idx_save_database(const char *dbfile, void *db)
{
    return ((CDatabase*)db)->save(dbfile);
}

int ParseCmd(const char* strCmd, const vector<CDatabase*> &vDB, bool onlyInName, int maxResults, struct RESULT ** results, const struct PREFERENCES &pref);

extern "C" int idx_query(void *db, char* queryString, bool onlyInName, int maxResults, struct RESULT **results,const struct PREFERENCES *pref)
{
    if (NULL!=db)
    {
        vector<CDatabase*> dbs;
        dbs.push_back((CDatabase*)db);
        return ParseCmd(queryString,dbs,onlyInName,maxResults,results,*pref);
    }
    else
        return -1;
}

int coremain(int argc, char* argv[])
{
    atexit(Object::onExit);

    bool fCommandLineError=false;
	vector<string> vDir;
	void *fileCollection=NULL;
	void *db=NULL;
	string strExts("\"txt\";");

	if (argc==1)
	{
		printusage(argv[0]);
		return -1;
	}
	// Parsing command line
	for (int i=1;i<argc;i++)
	{
		if (strlen(argv[i])>1 && argv[i][0]==L'-') // it's a switch
		{
			if (!strcmp(argv[i],"-r") || !strcmp(argv[i],"-R") || !strcmp(argv[i],"--recursive"))
			{
				fRecursive=true;
			}else if (!strcmp(argv[i],"-v") || !strcmp(argv[i],"-V"))
			{
				fVerbose=true;
			}else if (!strcmp(argv[i],"-ext") && i+1<argc)
			{
				strExts+=string("\"")+argv[++i]+"\";";
			}else if (!strcmp(argv[i],"-hexie") && i+1<argc)
			{
                idx_set_stopwords(argv[++i]);
			}else
			{
				cerr<<"ERR: invalid option \""<<argv[i]<<"\""<<endl;
				fCommandLineError=true;
			}
		}else // insert the path into the vector
		{
			vDir.push_back(argv[i]);
		}
	}
	if (fCommandLineError) goto cleaning;
	// Add files

	for (vector<string>::const_iterator it=vDir.begin();it!=vDir.end();it++)
	{
		if (idx_add_documents(it->c_str(),fRecursive,&fileCollection,0))
		{
			cerr<<"ERROR: "<<*it<<" is not a directory."<<endl;
		}
	}
	idx_add_documents("/usr/share/applications",false,&fileCollection,0);

	vDir.clear();
	// Building Indexing Database
	volatile int *cond;
	struct PREFERENCES pref;
    memset(&pref,0,sizeof(pref));
    pref.get_app=1;
    pref.file_c=1;
    pref.file_txt=1;
    pref.get_document=1;
    pref.multi_core=1;
	cond=idx_create_database_task(fileCollection,&pref,&db);
	while (*cond==2333)
	{
        sleep(1);
	}
    //idx_load_database("/tmp/1.db",&db);
	idx_save_database("/tmp/1.db",db);
	// Process Queries
	cout<<"Enter q! to quit"<<endl;
	for (;;)
	{
		char strCmd[300];
		cout<<">> Keyword: ";
		if ((cin.getline(strCmd,sizeof(strCmd))) && strCmd!=strstr(strCmd,"q!"))
		{
            struct RESULT *r;

            int tot;
            tot=idx_query(db,strCmd,true,30,&r,&pref);
            for (int i=0;i<30 && i<tot;i++)
            {
                printf("%s @%s %d\n",r[i].name,r[i].path,r[i].type);
            }
            printf("%d results in total\n",tot);
		}else
            break;
	}
	idx_delete_database(db);
cleaning:
	cout<<"Goodbye!"<<endl;
    return 0;
}
