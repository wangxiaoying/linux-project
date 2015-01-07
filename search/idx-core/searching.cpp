#include "pch.h"
#include "Database.h"
#include "CResultPool.h"
#include "CConstants.h"

#include <fstream>
#include <algorithm>
#include <iomanip>
using namespace std;

#include "searching.h"
#include <sys/stat.h>
#include <fcntl.h>

extern bool fVerbose;

string& replace(string& str,const string& oldValue,const string& newValue)
{
	for (string::size_type pos=0;pos!=string::npos;pos+=newValue.length())
	{
		if ((pos=str.find(oldValue,pos))!=string::npos)
			str.replace(pos,oldValue.length(),newValue);
		else break;
	}
	return str;
}

int ParseCmd(const char* strCmd,const vector<CDatabase*> &vDB, bool onlyInName, int maxResults, struct RESULT **results, const struct PREFERENCES &pref)
{
	vector<vector<string> > vPhrases;
	vector<string> vWords;
	string sThisWord;
	int iJoinType=1; // 0=cat 1=and 2=or
	int ch,posl=-1; // position of previous left bracket of quote
	for (int i=0;0<(ch=tolower(CTextFile::fixch(strCmd[i])));i++)
	{
		switch (ch)
		{
		case '\n':
		case '\r':
			goto scanningfin;

        // separators
		case '+':
		case ',':
		case ' ':
		case '\t':
		case '\"': // quotes are also delimiters
			if (sThisWord.length()>0) // this word is over
			{
				if (posl==-1 && (!sThisWord.compare("and") || !sThisWord.compare("or"))) // control keyword
				{
					if (!vWords.size())
					{
						cerr<<"ERROR: No keywords before 'and'/'or'"<<endl;
						return -1;
					}else
					{
						vPhrases.push_back(vWords);
						vWords.clear();
					}
					if (!sThisWord.compare("and")) iJoinType=1;else iJoinType=2;
				}else
                    vWords.push_back(sThisWord);

				sThisWord.clear();
			}
			break;
		default:
			if (CTextFile::chartoidx(ch)!=-1 || onlyInName)
			{
				sThisWord+=(char)ch;
				break;
			}else
			{
				cerr<<"ERROR: Invalid character '"<<ch<<"' in the expression. It may be not suitable for searching."<<endl;
				return -1;
			}
		};
		switch (ch)
		{
		case '"':
			if (posl==-1) // left bracket
			{
				posl=i;
			}else // right bracket
			{
				if (vWords.size())
				{
					vPhrases.push_back(vWords);
					vWords.clear();
				}else
				{
					cerr<<"ERROR: No keywords in the brackets"<<endl;
					return -1;
				}
				posl=-1;
			}
			break;
		};
	}
scanningfin:
	if (posl!=-1)
	{
		cerr<<"ERROR: BRACKETS DON'T MATCH. @"<<posl<<endl;
		return -1;
	}
	if (sThisWord.length()>0) vWords.push_back(sThisWord);
	if (vWords.size()>0)
	{
		vPhrases.push_back(vWords);
		vWords.clear();
	}
	if (!vPhrases.size())
	{
		cerr<<"ERROR: No valid keywords"<<endl;
		return -1;
	}
	if (fVerbose)
	{
		cout<<"->Parser Output: Join Type="<<iJoinType<<endl;
		for (vector<vector<string> >::const_iterator it1=vPhrases.begin();it1!=vPhrases.end();it1++)
		{
			cout<<"Phrase:";
			for (vector<string>::const_iterator it2=it1->begin();it2!=it1->end();it2++)
			{
				cout<<(*it2)<<',';
			}
			cout<<endl;
		}
	}
	CResultPool cFinalResult;
	for (vector<vector<string> >::const_iterator it=vPhrases.begin();it!=vPhrases.end();it++)
	{
		CResultPool cThisResult;
		for (size_t i=0;i<vDB.size();i++)
		{
            if (onlyInName)
            {
                vDB[i]->SearchName(*it,cThisResult,i+1);
            }
            else
            {
                if (vDB[i]->SearchPhrase(*it,cThisResult,i+1)<0)
                {
                    cerr<<"ERROR: Keyword contains words that don't consist in the database."<<endl;
                }
            }
		}
		if (it==vPhrases.begin())
			cFinalResult=cThisResult;
		else switch (iJoinType)
		{
		case 1:
			cFinalResult=cThisResult*cFinalResult; // for better results
			break;
		case 2:
			cFinalResult=cFinalResult+cThisResult;
			break;
		}
	}
	cFinalResult.organize();
	int i=0;
	*results=new struct RESULT[maxResults];
	struct RESULT *r;
	if (onlyInName)
	{
        if (strstr("preferences",strCmd))
        {
            r=&(*results)[i++];
            strcpy(r->name,"Alfred Preferences");
            strcpy(r->path,"");
            r->type=SET;
        }


        if (pref.get_web)
        {
            r=&(*results)[i++];
            strcpy(r->name,strCmd);
            strcpy(r->path,"http://www.baidu.com");
            r->type=WEB;
        }
        if (pref.get_sys)
        {
            if (strstr("screenshot",strCmd))
            {
                r=&(*results)[i++];
                strcpy(r->name,"Take a screenshot");
                strcpy(r->path,"gnome-screenshot -a");
                r->type=SYS;
            }
        }
	}
	for (vector<RESULTTYPE>::const_iterator it=cFinalResult.begin();it!=cFinalResult.end() && i<maxResults;it++)
	{
		const RESID resid=CResultPool::resid(*it);
		const CTextFile &cText=vDB[CResultPool::RESIDtoDBID(resid)-1]->operator[](CResultPool::RESIDtoFileID(resid));
		//const int posWord=CResultPool::wpos(*it);
		// Command-Line Output
		r=NULL;
		if ((cText.getFileType()==APP && pref.get_app) || (cText.getFileType()==DOC && pref.get_document) \
            || (cText.getFileType()==FOLDER && pref.get_folder))
            r=&(*results)[i++];
        if (r!=NULL)
        {
            //r->offset=cText.findWordbyPos(CResultPool::wpos(*it));
            //r->score=CResultPool::relevance(*it);
            strcpy(r->name,cText.getFileName().c_str());
            strcpy(r->path,cText.getFilePath().c_str());
            r->type=cText.getFileType();
            //r->type=(SUPPORT_TYPE)CResultPool::relevance(*it);
		}
	}
	return i;//cFinalResult.size();
}
