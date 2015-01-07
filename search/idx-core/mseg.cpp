#include "pch.h"
#include "style.h"
#include "mseg.h"
#include <vector>
#include <string>
#include <fstream>
#include <algorithm>
using namespace std;

struct Word
{
    string simp,py;
};

vector<Word> words;
vector<string> chsw,chspy;

bool cmp(const Word& a, const Word& b)
{
    return a.simp.compare(b.simp)<0;
}

int processDict()
{
    FILE *dict=fopen("cedict_ts.u8","rt");
    FILE *out=fopen("words.txt","wb");

    static char line[2048];
    static char trad[100],simp[100];
    long cntLines=0;

    while (!feof(dict) && NULL!=fgets(line,2048,dict))
    {
        if (line[0]=='#') continue;

        ++cntLines;
        sscanf(line,"%s %s",trad,simp);

        const size_t offset=strlen(trad)+strlen(simp);
        assert(line[offset+2]=='[');

        char pinyin[200];
        for (size_t i=offset+3,j=0;line[i]!=0 && j<sizeof(pinyin)/sizeof(pinyin[0]);i++)
        {
            switch (line[i])
            {
            case ' ':
                //pinyin[j++]='\'';
                break;
            case '0'...'9':
                continue;
            case ']':
                pinyin[j++]=0;
                goto lPyEnd;
                break;
            default:
                pinyin[j++]=line[i];
                break;
            }
        }
        lPyEnd:
        Word w;
        w.simp=simp;
        w.py=pinyin;
        words.push_back(w);
    }
    printf("%ld lines in total. %u words.\n",cntLines,words.size());

    // sort
    stable_sort(words.begin(),words.end(),cmp);

    // output
    for (size_t i=0;i<words.size();i++)
    {
        fprintf(out,"%s %s\n",words[i].simp.c_str(),words[i].py.c_str());
    }

    fclose(dict);
    fclose(out);
    return 0;
}


template <class T>
bool validRange(pair<typename vector<T>::iterator,typename vector<T>::iterator>& rng)
{
    return rng.first<rng.second;
}

int loadDict()
{
    ifstream fwordtbl("words.txt",ios::binary);
    if (!fwordtbl) return -1;
    string line;
    long cntLine=0;
    while (getline(fwordtbl,line))
    {
        ++cntLine;
        string w=line.substr(0,line.find(' '));
        string py=line.substr(line.find(' ')+1,string::npos);
        if (w.length()<=0 || py.length()<=0)
        {
            printf("line %ld error.\n",cntLine);
        }
        chsw.push_back(w);
        chspy.push_back(py);
    }
    fwordtbl.close();
}

string pinyinize(void* seg, char* data, size_t len, FILE *result)
{
    mmseg_set_buffer(seg,data,len);
    string lastWord="";
    string lastPinyin="";
    string ret;
    while (1)
    {
        u2 len=0,symlen=0;
        const char *tok;
        if (NULL!=(tok=mmseg_peek_token(seg,&len,&symlen)) && len!=0 && symlen!=0)
        {
            const string thisWord(tok,symlen);
            const string compoundWord=lastWord+thisWord;
            pair<vector<string>::iterator,vector<string>::iterator> rng;

            if (!binary_search(chsw.begin(),chsw.end(),compoundWord))
            {
                rng=equal_range(chsw.begin(),chsw.end(),thisWord);
                // fprintf(result,"%s[%s]",lastWord.c_str(),lastPinyin.c_str());
                ret+=lastPinyin;
                lastWord=thisWord;
                if (rng.first<rng.second)
                    lastPinyin="";
                else
                {
                    // lastPinyin="?"+thisWord;
                    lastPinyin=thisWord;
                }
            }else
            {
                rng=equal_range(chsw.begin(),chsw.end(),compoundWord);
                lastWord+=thisWord;
            }
            if (rng.first<rng.second)
            {
                lastPinyin.append(chspy[rng.first-chsw.begin()]);
            }
            // fprintf(result,"%*.*s %u %u\n",symlen,symlen,tok,len,symlen);
            mmseg_pop_token(seg,len);
        }else
            break;
    }
    ret+=lastPinyin;
    // fprintf(result,"%s[%s]",lastWord.c_str(),lastPinyin.c_str());
    return ret;
}

int main444()
{
    // processDict();
    loadDict();

    FILE *text=fopen("test.txt","rb");
    fseek(text,0,SEEK_END);
    size_t len=ftell(text);
    char* data=(char*)malloc(len+1);
    rewind(text);
    fread(data,1,len,text);
    data[len]=0;
    fclose(text);

    FILE *result=fopen("result.txt","wt");
    void *mgr,*seg;
    mgr=mmseg_new_manager(".");
    seg=mmseg_get_segmenter(mgr);
    fprintf(result,"%s\n",pinyinize(seg,data,len,result).c_str());
    mmseg_delete_manager(mgr);

    fclose(result);
    free(data);
    return 0;

}
