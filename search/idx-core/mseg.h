#ifndef MSEG_H_INCLUDED
#define MSEG_H_INCLUDED


typedef unsigned short u2;
extern "C" void* mmseg_new_manager(const char *dictPath);
extern "C" void mmseg_delete_manager(void *mgr);
extern "C" void* mmseg_get_segmenter(void *mgr);
extern "C" void mmseg_delete_segmenter(void *seg);
extern "C" int mmseg_set_buffer(void *seg, char *buf, size_t len);
extern "C" const char* mmseg_peek_token(void *seg, u2* len, u2* symlen);
extern "C" void mmseg_pop_token(void *seg, u2 len);


#endif // MSEG_H_INCLUDED
