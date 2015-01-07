#ifndef CTEXTFILE_H_INCLUDED
#define CTEXTFILE_H_INCLUDED

#include "Object.h"
#include "CDynamicVector.h"
#include <iostream>
#include <fstream>
#include <string>
#include "searching.h"
#include "mseg.h"
#include <zlib.h>

using namespace std;

class CTextFile: public Object
{
private:
	CTextFile();
	CTextFile(const CTextFile&);

	string m_strFilePath;
	string m_strFileName;
	string m_strFileNameLC;
	string m_strFileNamePinyinLC;
	string m_strFileDirectory;
	string m_strFileExt;
	int m_iFileID;
	int m_iBaseScore;
	SUPPORT_TYPE m_iFileType;

	int m_iWordPosition;
	ifstream fin;

	CDynVector<pair<int,pair<int,streampos> > > *m_WordData; // Index -> <Word Position,<Word ID, Byte Offset>>
	streampos m_iFileSize;
	streampos m_iDataOffset;

	bool m_fBigEndian;
	bool m_fWideChar;
	bool m_fUTF8;
	bool m_fValidFile;

	bool m_fNeedProcess;

public:
    CTextFile(gzFile& fp);
	CTextFile(const int,const int,const string& path,bool toprocess,void *seg);
	~CTextFile();

	int getFileID() const {return m_iFileID;}
	string getFilePath() const {return m_strFilePath;}
	streampos getFileSize() const {return m_iFileSize;}
	int getBonus() const {return m_iBaseScore;}
	SUPPORT_TYPE getFileType() const {return m_iFileType;}

    void store(gzFile& fp);

    void reOpen();
    bool needProcess() const {return m_fNeedProcess;}
    bool isValid() const {return m_fValidFile;}
	bool isOpen() const {return fin&&fin.is_open();} // whether the input stream is valid
	bool isEOF() const {return fin.is_open()&&fin.eof();} // whether the input stream reaches the end
	void closefile();

	int ReadWord(const int cbBuf,char* buf); // returns word position
	int ReportWordID(const int idxWord);
	int getWordCount() const {assert(m_WordData!=NULL);return m_WordData->size();}

	streampos findWordbyPos(const int posWord,size_t *pIndex=NULL) const; // Word Position -> ByteOffset, Index
	int findWordbyIndex(const int iIndex,int *pWordPos=NULL) const; // Index -> WordID, WordPosition

	static bool myisalpha(const int x)
	{
		return (x&0x40) && (x&0x1F)<=26 && !(x&0xFFFFFF80) && (x&0x1F);
	}

	static bool myisdigit(const int x)
	{
		return (x&0x30)==0x30 && !(x&0xFFFFFFC0) && (x&0xF)<=9;
	}

	static int fixch(const int x)
	{
		switch (x)
		{
		case 8216:
		case 8217:return '\'';
		case 8220:
		case 8221:return '\"';
		case 8212:return 0; // dash
		case -96:return ' ';
		default:
			return x;
		}
	}

	static char idxtochar(const int idx)
	{
		if (idx>=0 && idx<=25)
            return 'a'+idx;
		switch (idx)
		{
		case 26:return '-';break;
		case 27:return '\'';break;
		default:return '?';break;
		}
	}

	static int chartoidxsp(const int ch)
	{
		switch (ch)
		{
		case '-':
			return 26;break;
		case '\'':
			return 27;break;
		default:
			return -1;break;
		}
	}

	static int chartoidx(int ch) // it must be lower case
	{
		assert(!(ch>='A' && ch<='Z'));
		if (myisalpha(ch))
			return ch-'a';
		else
			return chartoidxsp(ch);
	}

	static const char* GetFileExt(const char* lpStringPath,const bool bLowerCase=true,const char chDot='.')
	{
		static char nullstring[1]={0};
		const char *p=strchr(lpStringPath,chDot);
		if (NULL==p) return nullstring;else lpStringPath=p+1;

		while (NULL!=(p=strchr(lpStringPath,chDot))) lpStringPath=p+1;
		if (bLowerCase) for (p=lpStringPath;*p;p++) *const_cast<char*>(p)=tolower(*p);
		return lpStringPath;
	}

	const string& getFileExt() const
	{
        return m_strFileExt;
    }

	const string& getFileName() const
	{
		return m_strFileName;
	}

	const string& getLowerCaseFileName() const
	{
        return m_strFileNameLC;
	}

	const string& getPinyinFileName() const
	{
	    return m_strFileNamePinyinLC;
	}

    const string& getFileDirectory() const
	{
        return m_strFileDirectory;
	}

private:
	int DecodeChar(const char ch1,const char ch2)
	{
		int ch;
		if (m_fBigEndian)
			ch=(ch1<<8)|ch2;
		else if (m_fWideChar)
			ch=ch1|(ch2<<8);
		else ch=ch1;
		return fixch(ch);
	}

	bool ReadChar(int* ch)
	{
		char ch1,ch2=-1,ch3=-1;
		assert(ch!=NULL);
		if (fin.get(ch1) && (!m_fWideChar || fin.get(ch2)))
		{
			if ((!m_fWideChar) && (ch1&0x80)) // UTF-8
			{
				if (!fin.get(ch2)) return false;
				if ((ch1&0xE0)==0xC0 && (ch2&0xC0)==0x80) // 000080 - 0007FF 110xxxxx 10xxxxxx
				{
					*ch=((ch1&0x1F)<<6)|(ch2&0x3F);
					m_fUTF8=true;
				}else if ((ch1&0xF0)==0xE0 && (ch2&0xC0)==0x80) // 000800 - 00FFFF 1110xxxx 10xxxxxx 10xxxxxx
				{
					if (!fin.get(ch3) || (ch3&0xC0)!=0x80)
						return false;
					*ch=((ch1&0xF)<<12)|((ch2&0x3F)<<6)|(ch3&0x3F);
					m_fUTF8=true;
				}else return false;
				*ch=fixch(*ch);
			}else *ch=DecodeChar(ch1,ch2);
			return true;
		}
		else
			return false;
	}
};

#endif // CTEXTFILE_H_INCLUDED
