#include "pch.h"
#include "CConstants.h"
#include "CTextFile.h"
#include <sys/stat.h>
#include <fcntl.h>
#include <algorithm>

using namespace std;

CTextFile::CTextFile(const int id,const int score,const string& path,bool toprocess,void *segmenter): \
	m_iFileID(id),m_strFilePath(path),m_fNeedProcess(toprocess),m_WordData(NULL) // open the file here
{
    m_fValidFile=true;
    m_iBaseScore=score;
    m_strFileExt=GetFileExt(m_strFilePath.c_str(),false);
    {
        int lenExt;
		string s=GetFileExt(m_strFilePath.c_str(),false,'/');
		lenExt=getFileExt().length();
		m_strFileName = s;//lenExt?s.substr(0,s.length()-lenExt-1):s;
    }
    {
        string s=getFilePath();
		m_strFileDirectory = s.substr(0,s.length()-string(GetFileExt(m_strFilePath.c_str(),false,'/')).length());
    }
    // special file type
    if (!m_strFileExt.compare("desktop"))
    {
        int flag=0;
        fin.open(m_strFilePath.c_str());
        if (fin)
        {
            string line;
            while (getline(fin,line) && flag!=3)
            {
                size_t pos;
                if (0==(flag&1) && 0==(pos=line.find("Name=")))
                {
                    flag|=1;
                    m_strFileName=line.substr(pos+5);
                }else if (0==(flag&2) && 0==(pos=line.find("Exec=")))
                {
                    flag|=2;
                    m_strFilePath=line.substr(pos+5);
                }
            }
            m_iFileType=APP;
            m_iBaseScore+=BOOST_APP;
            m_fNeedProcess=false;
            fin.close();
        }else goto error;
    }else
    {
        m_iFileType=DOC;
    }
    m_strFileNameLC = m_strFileName;
    m_strFileNamePinyinLC = m_strFileName;
    extern string pinyinize(void* seg, char* data, size_t len, FILE *result);
    char *data;
    data = strdup(m_strFileName.c_str());
    if (NULL!=data)
    {
        m_strFileNamePinyinLC = pinyinize(segmenter, data, m_strFileName.length(), NULL);
        free(data);
    }
    transform(m_strFileNameLC.begin(), m_strFileNameLC.end(), m_strFileNameLC.begin(), (int (*)(int))tolower);
    //printf("pinyin: %p %s %s\n",segmenter, m_strFileName.c_str(),m_strFileNamePinyinLC.c_str());
    transform(m_strFileNamePinyinLC.begin(), m_strFileNamePinyinLC.end(), m_strFileNamePinyinLC.begin(), (int (*)(int))tolower);
    // process file contentes
    if (toprocess)
    {
        fin.open(path.c_str(),ios::binary);
        if (!fin)
        {
            cerr<<"Error opening "<<m_strFilePath<<" code = "<<errno<<endl;
            goto error;
        }
        assert(fin);
        fin.seekg(0,ios::end);
        m_iFileSize=fin.tellg();
        if (m_iFileSize<=2) // file too small
        {
error:
            m_iFileSize=0;
            m_fValidFile=false;
            return;
        }
        char ch1,ch2,ch3;
        fin.seekg(0,ios::beg);
        if (fin.get(ch1) && fin.get(ch2)) // read Byte Order Mask(BOM)
        {
            m_fBigEndian=false;m_fWideChar=false;m_fUTF8=false;

            if (ch1==(char)0xFE && ch2==(char)0xFF) {m_fBigEndian=true;m_fWideChar=true;} //UTF-16 Big Endian
            else if (ch1==(char)0xFF && ch2==(char)0xFE) m_fWideChar=true; //UTF-16 Little Endian
            else if (ch1==(char)0xEF && ch2==(char)0xBB && fin.get(ch3) && ch3==(char)0xBF) m_fUTF8=true; // UTF-8
            else fin.seekg(0,ios::beg);
        }else goto error;
        m_iDataOffset=fin.tellg();
        m_iFileSize-=fin.tellg();
        unsigned long long contentLen;
        contentLen=(unsigned long long)m_iFileSize;
        if (m_fWideChar) contentLen>>=1;

        m_WordData=new CDynVector<pair<int,pair<int,streampos> > >(contentLen/(AVERAGE_WORD_LENGTH+1)+1); // average word length is 5 for english. another 1 for space
        assert(m_WordData!=NULL);

        closefile();
    }else
    {
        struct stat64 st;
        lstat64(path.c_str(),&st);
        if (S_ISDIR(st.st_mode))
        {
            m_strFilePath+="/";
            m_iFileType=FOLDER;
            m_iBaseScore+=BOOST_DIRECTORY;
            m_fNeedProcess=false;
        }else
        {
            m_iFileSize=st.st_size;
        }
        m_iDataOffset=0;
        m_fUTF8=true;
        m_fBigEndian=false;
        m_fWideChar=false;
    }
}

void CTextFile::closefile()
{
	if (fin.is_open())
	{
        fin.close();
	}
}

void CTextFile::reOpen()
{
    fin.open(m_strFilePath.c_str(),ios::binary);
    if (fin)
    {
        fin.seekg(m_iDataOffset);
        m_iWordPosition=1;
	}
}

CTextFile::~CTextFile()
{
	closefile();
	if (m_WordData!=NULL) {delete m_WordData;m_WordData=NULL;}
}

int CTextFile::ReadWord(const int cbBuf,char* buf)
{
	int ch=0,lastch=-1;
	int i,posDelta=0,offsetWord;
	if (!fin.is_open()) return -1;
	assert((m_WordData!=NULL) && (fin.is_open()));
	assert(cbBuf>0 && buf!=NULL);
	for (i=0;ReadChar(&ch);lastch=ch)
	{
		if (myisalpha(ch) || (i>0 && chartoidxsp(ch)>0 && myisalpha(lastch))) // this character is an alphabet or hyphen or single quote
		{
			if (i==0) // beginning
			{
				if (myisdigit(lastch)) // A normal word shouldn't begin with any digits
					i=-1; // this word should be skipped
				else offsetWord=(int)fin.tellg()-(m_fWideChar?2:1);
			}
			if (i!=-1)
			{
				if (i<cbBuf) buf[i]=tolower(ch);
				i++;
			}
		}else
		{
			if (i==-1) i=0;
			// punctuation
			switch (ch)
			{
			case ':':
			case '"':
			case ',':
			case '(':
			case ')':
			//case '/':
				posDelta+=1;break;
			case ';':
			//case '\t':
				posDelta+=2;break;
			case '.':
			case '!':
			case '?':
				posDelta+=3;break;
			case '\n':
				posDelta+=2;break;
			};
			if (!i)
			{
				m_iWordPosition+=posDelta;
				posDelta=0;
			}else if (i>0)
			{
lastword:
				// A normal word shouldn't end with any digits
				if (!myisdigit(ch) && i<cbBuf)
				{
					buf[i]=0; // add terminal character
					goto validword;
				}
				i=0;
			}
		}
	}
	if (i<=0)
	{
		//data->resize(m_iCurSize);
		return -1; // end of file
	}else goto lastword;
validword:
	int iCurPos=m_iWordPosition;
	//cout<<buf<<endl; //print each word
	m_WordData->push_back(  make_pair(iCurPos,make_pair(-1,offsetWord)) );
	m_iWordPosition+=posDelta+1;
	return iCurPos;
}

int CTextFile::ReportWordID(const int idxWord)
{
	assert(getWordCount()>0);
	if (idxWord==-1)
		m_WordData->decSize();
	else
		(*m_WordData)[m_WordData->size()-1].second.first=idxWord;
	return idxWord;
}

streampos CTextFile::findWordbyPos(const int posWord,size_t *pIndex) const // Binary search
{
	if (posWord<=0 || posWord>m_iWordPosition || m_WordData==NULL || !m_WordData->size()) return -1;
	int left=0,right=m_WordData->size()-1;
	while (left<=right)
	{
		size_t mid=(left+right)>>1;
		int midValue=(*m_WordData)[mid].first;
		if (midValue==posWord)
		{
			if (NULL!=pIndex) *pIndex=mid;
			return (*m_WordData)[mid].second.second;
		}
		if (midValue<posWord) left=mid+1;else right=mid-1;
	}
	return -1;
}

int CTextFile::findWordbyIndex(const int iIndex,int *pWordPos) const
{
	assert(m_WordData!=NULL);
	if (iIndex<m_WordData->size())
	{
		if (NULL!=pWordPos) *pWordPos=(*m_WordData)[iIndex].first;
		return (*m_WordData)[iIndex].second.first;
	}else return 0;
}

struct TEXTFILE_SERIALIZATION
{
    int id;
    int basescore;
    long long filesize;
    long long dataoffset;
    int wordposition;
    int bigendian;
    int widechar;
    int utf8;
    int valid;
    SUPPORT_TYPE filetype;
    long long worddatasize;
};

void writeString(gzFile& fp,string s)
{
    size_t len=s.length();
    gzwrite(fp,&len,sizeof(len));
    gzwrite(fp,s.data(),len);
}

void readString(gzFile& fp,string& s)
{
    size_t len=0;

    gzread(fp,&len,sizeof(len));
    if (len!=0)
    {
        char *temp = new char[len+1];
        gzread(fp,temp,len);
        temp[len]=0;
        s=temp;
        delete[] temp;
    }else s="";
}

void CTextFile::store(gzFile& fp)
{
    TEXTFILE_SERIALIZATION d;
    d.id = m_iFileID;
    d.filesize = m_iFileSize;
    d.filetype = m_iFileType;
    d.basescore = m_iBaseScore;
    d.dataoffset = m_iDataOffset;
    d.wordposition = m_iWordPosition;
    d.bigendian = m_fBigEndian?1:0;
    d.widechar = m_fWideChar?1:0;
    d.utf8 = m_fUTF8?1:0;
    d.valid = m_fValidFile?1:0;
    d.worddatasize = (m_WordData==NULL)?0:m_WordData->size();
    gzwrite(fp,&d,sizeof(d));

    writeString(fp,m_strFilePath);
	writeString(fp,m_strFileName);
	writeString(fp,m_strFileNamePinyinLC);
	writeString(fp,m_strFileDirectory);
	writeString(fp,m_strFileExt);

	if (m_WordData!=NULL)
	{
        int n=m_WordData->size();
        for (int i=0;i<n;i++)
        {
            pair<int,pair<int,streampos> > p;
            p=(*m_WordData)[i];
            gzwrite(fp,&p,sizeof(p));
        }
	}
}

CTextFile::CTextFile(gzFile& fp):m_WordData(NULL) // open the file here
{
    TEXTFILE_SERIALIZATION d;
    gzread(fp,&d,sizeof(d));
    m_iFileID=d.id;
    m_iFileType=d.filetype;
    m_iBaseScore=d.basescore;
    m_iFileSize=d.filesize;
    m_iDataOffset=d.dataoffset;
    m_fBigEndian=d.bigendian;
    m_fUTF8=d.utf8;
    m_fWideChar=d.widechar;
    m_fValidFile=d.valid;
    m_iWordPosition=d.wordposition;
    m_WordData=new CDynVector<pair<int,pair<int,streampos> > >(d.worddatasize+1);

    readString(fp,m_strFilePath);
	readString(fp,m_strFileName);
	readString(fp,m_strFileNamePinyinLC);
	readString(fp,m_strFileDirectory);
	readString(fp,m_strFileExt);
	m_strFileNameLC = m_strFileName;
    transform(m_strFileNameLC.begin(), m_strFileNameLC.end(), m_strFileNameLC.begin(), (int (*)(int))tolower);

    if (d.worddatasize!=0)
    {
        for (long long i=0;i<d.worddatasize;i++)
        {
            pair<int,pair<int,streampos> > p;
            gzread(fp,&p,sizeof(p));
            m_WordData->push_back(p);
        }
    }
}
