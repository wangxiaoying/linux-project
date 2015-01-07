/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* ***** BEGIN LICENSE BLOCK *****
* Version: GPL 2.0
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License. You should have
* received a copy of the GPL license along with this program; if you
* did not, you can find it at http://www.gnu.org/
*
* Software distributed under the License is distributed on an "AS IS" basis,
* WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
* for the specific language governing rights and limitations under the
* License.
*
* The Original Code is Coreseek.com code.
*
* Copyright (C) 2007-2008. All Rights Reserved.
*
* Author:
*	Li monan <li.monan@gmail.com>
*
* ***** END LICENSE BLOCK ***** */

#include <fstream>
#include <string>
#include <iostream>
#include <cstdio>
#include <algorithm>
#include <map>
#include  <stdlib.h>
#include "bsd_getopt.h"
#include "UnigramCorpusReader.h"
#include "UnigramDict.h"
#include "SynonymsDict.h"
#include "SegmenterManager.h"
#include "Segmenter.h"
#include "csr_utils.h"

using namespace std;
using namespace css;

#define SEGMENT_OUTPUT 1

void usage(const char* argv_0) {
	printf("Coreseek COS(tm) MM Segment 1.0\n");
	printf("Copyright By Coreseek.com All Right Reserved.\n");
	printf("Usage: %s <option> <file>\n",argv_0);
	printf("-u <unidict>           Unigram Dictionary\n");
	printf("-r           Combine with -u, used a plain text build Unigram Dictionary, default Off\n");
	printf("-b <Synonyms>           Synonyms Dictionary\n");
	printf("-h            print this help and exit\n");
	return;
}
int segment(const char* file,Segmenter* seg);
/*
Use this program
Usage:
1)
./ngram [-d dict_path] [file] [outfile]
Do segment. Will print segment result to stdout
-d the path with contains unidict & bidict
file: the file to be segment, must encoded in UTF-8 format. [*nix only] if file=='-', read data from stdin
2)
./ngram -u file [outfile]
Build unigram dictionary from corpus file.
file: the unigram corpus. Use \t the separate each item.
eg.
item	3
n:2	a:1
if outfile not assigned , file.uni will be used as output
3)
./ngram -u unidict -b file [outfile]
Build bigram
if outfile not assigned , file.bi will be used as output
*/

int mmsegmain(int argc, char **argv) {
    SegmenterManager* mgr = new SegmenterManager();
    int nRet = 0;
    nRet = mgr->init(".");
    if(nRet == 0){
        //init ok, do segment.
        Segmenter* seg = mgr->getSegmenter();
        segment(argv[1],seg);
    }
    delete mgr;
	return 0;
}

extern "C" void* mmseg_new_manager(const char *dictPath)
{
    SegmenterManager* mgr = new SegmenterManager();
    if (0==mgr->init(dictPath))
        return mgr;
    else
    {
        delete mgr;
        return NULL;
    }
}

extern "C" void mmseg_delete_manager(void *mgr)
{
    if (mgr!=NULL)
        delete (SegmenterManager*)mgr;
}

extern "C" void* mmseg_get_segmenter(void *mgr)
{
    if (NULL==mgr)
        return NULL;
    else
        return ((SegmenterManager*)mgr)->getSegmenter();
}

extern "C" void mmseg_delete_segmenter(void *seg)
{
    if (seg!=NULL)
        delete (Segmenter*)seg;
}

extern "C" int mmseg_set_buffer(void *seg, char *buf, size_t len)
{
    if (NULL==seg || NULL==buf)
        return -1;
    else
    {
        ((Segmenter*)seg)->setBuffer((u1*)buf,len);
        return 0;
    }
}

extern "C" const char* mmseg_peek_token(void *seg, u2* len, u2* symlen)
{
    if (NULL==seg)
        return NULL;
    else
    {
        return (const char*) (((Segmenter*)seg)->peekToken((u2&)(*len), (u2&)(*symlen)));
    }
}

extern "C" void mmseg_pop_token(void *seg, u2 len)
{
    if (seg!=NULL)
        ((Segmenter*)seg)->popToken(len);
}

int segment(const char* file,Segmenter* seg)
{
	std::istream *is;

	is = new std::ifstream(file, ios::in | ios::binary);
	if (! *is)
		return -1;

	std::string line;

	unsigned long srch,str;
	str = currentTimeMillis();
	//load data.
	int length;
	is->seekg (0, ios::end);
	length = is->tellg();
	is->seekg (0, ios::beg);
	char* buffer = new char [length+1];
	is->read (buffer,length);
	buffer[length] = 0;
	//begin seg
	seg->setBuffer((u1*)buffer,length);
	u2 len = 0, symlen = 0;
	//check 1st token.
	unsigned char txtHead[3] = {239,187,191};
	char* tok = (char*)seg->peekToken(len, symlen);
	seg->popToken(len);
	if(len == 3 && memcmp(tok,txtHead,sizeof(char)*3) == 0) {
		//check is 0xFEFF
		//do nothing
	}else{
		printf("%*.*s/",symlen,symlen,tok);
	}
	while(1){
		len = 0;
		char* tok = (char*)seg->peekToken(len,symlen);
		if(!tok || !*tok || !len)
			break;
		seg->popToken(len);

		if(*tok == '\r')
			continue;
		if(*tok == '\n'){
			printf("\n");
			continue;
		}
		//printf("[%d]%*.*s/x ",len,len,len,tok);
		printf("%*.*s/",symlen,symlen,tok);
		//printf("%s",tok);
	}
	srch = currentTimeMillis() - str;
	printf("\n\nTime elapsed: %ld ms.\n", srch);
	//found out the result
	delete is;
	return 0;
}
