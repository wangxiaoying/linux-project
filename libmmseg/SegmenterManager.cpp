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

#include "Segmenter.h"
#include "SegmenterManager.h"

namespace css {


const char g_ngram_unigram_dict_name[] = "uni.lib";
const char g_synonyms_dict_name[] = "synonyms.dat";

/** 
     *  Return a newly created segmenter
     */
Segmenter *SegmenterManager::getSegmenter()
{
	Segmenter* seg = NULL;
	if(m_method == SEG_METHOD_NGRAM){
		seg = seg_freelist_.alloc();
		//init seg
		seg->m_unidict = &m_uni;
		seg->m_symdict = &m_sym;
	}		
	return seg;
}
int SegmenterManager::init(const char* path, u1 method)
{
	if( method != SEG_METHOD_NGRAM)
		return -4; //unsupport segmethod.
	
	if( m_inited )
		return 0; //only can be init once.
	
	char buf[1024];
	memset(buf,0,sizeof(buf));
	if(!path)
		memcpy(buf,".",1);
	else
		memcpy(buf,path,strlen(path));
	int nLen = (int)strlen(path);
	//check is end.
#ifdef WIN32
	if(buf[nLen-1] != '\\'){
		buf[nLen] = '\\';
		nLen++;
	}
#else
	if(buf[nLen-1] != '/'){
		buf[nLen] = '/';
		nLen++;
	}
#endif
	m_method = method;
	int nRet = 0;

	if(method == SEG_METHOD_NGRAM) {
		seg_freelist_.set_size(64);
		memcpy(&buf[nLen],g_ngram_unigram_dict_name,strlen(g_ngram_unigram_dict_name));
		nRet = m_uni.load(buf);

		if(nRet!=0){
			printf("Unigram dictionary load Error\n");
			return nRet;
		}

		memcpy(&buf[nLen],g_synonyms_dict_name,strlen(g_synonyms_dict_name));
		buf[nLen+strlen(g_synonyms_dict_name)] = 0;
		//load g_synonyms_dict_name, we do not care the load in right or not
		nRet = m_sym.load(buf);
		if(nRet!=0 && nRet != -1){
			printf("Synonyms dictionary format Error\n");
		}
		nRet = 0;
		m_inited = 1;
		return nRet;
	}
	return -1;
}

void SegmenterManager::clear()
{
    seg_freelist_.free();
}
SegmenterManager::SegmenterManager()
		:m_inited(0)
{
	m_method = SEG_METHOD_NGRAM;
}
SegmenterManager::~SegmenterManager()
{
	clear();
}
} /* End of namespace css */

