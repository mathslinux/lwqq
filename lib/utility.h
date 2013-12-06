#ifndef LWQQ_UTIL_H_H
#define LWQQ_UTIL_H_H
#include "type.h"

typedef enum {
    LWQQ_CT_ENABLE_IGNORE = 1<<0,
    LWQQ_CT_CHOICE_MODE = 1<<1
}LwqqCTFlags;

typedef struct LwqqConfirmTable {
    LwqqAnswer answer;
    LwqqCTFlags flags;
    char* title;            //< read
    char* body;             //< read
    char* exans_label;      //< extra answer label, read
    char* input_label;      //< read
    char* yes_label;
    char* no_label;
    char* input;            //< write
    LwqqCommand cmd;
}LwqqConfirmTable;

typedef struct LwqqTypeMap{
    int type;
    const char* str;
}LwqqTypeMap;

void lwqq_ct_free(LwqqConfirmTable* table);

int lwqq_util_mapto_type(const struct LwqqTypeMap* maps,const char* key);
const char* lwqq_util_mapto_str(const struct LwqqTypeMap* maps,int type);

LwqqOpCode lwqq_util_save_img(void* ptr,size_t len,const char* path,const char* dir);

char* lwqq_util_hashN(const char* uin,const char* ptwebqq,void*);
char* lwqq_util_hashO(const char* uin,const char* ptwebqq,void*);
char* lwqq_util_hashP(const char* uin,const char* ptwebqq,void*);
char* lwqq_util_hashQ(const char* uin,const char* ptwebqq,void* _unused);

#define lwqq_group_pretty_name(g) (g->markname?:g->name)
#define lwqq_buddy_pretty_name(b) (b->markname?:b->nick)
#define lwqq_override(to,from_alloc) {char* tmp_ = from_alloc;if(tmp_){s_free(to);to=tmp_;}}

#ifdef WIN32
#define LWQQ_PATH_SEP "\\"
#else
#define LWQQ_PATH_SEP "/"
#endif

#define TABLE_BEGIN_LONG(name,rettp,paratp,init) \
    rettp name(paratp k){\
        rettp ret_ = init;\
        switch(k){

#define TABLE_BEGIN(name,type,init) TABLE_BEGIN_LONG(name,type,long,init)

#define TR(k,v)\
            case k:ret_ = v;break;
#define TABLE_END()\
        }\
        return ret_;\
    }

/* dynamic string manipulation nano-library */
#ifndef ds_init
struct ds { char * d; int p; int s; };
#define ds_initializer {NULL,0,0}
#define ds_init(x) do { x.d = (char *)0; x.p = x.s = 0; } while(0)
#define ds_rewind(x) x.p = 0;
#define ds_last(x) ((x).d[(x).p-1])
#define ds_free(x) do { if (x.d) free(x.d); ds_init(x); } while(0)
#define ds_redim(x) do { if (x.p >= x.s) x.d = realloc(x.d, x.s += 32); } while(0)
#define ds_sure(x,sz) do { if ((x).p+sz >= (x).s) (x).d = realloc((x).d,(x).s += sz+32);} while(0);
#define ds_poke(x,c) do { ds_redim((x)); (x).d[(x).p++] = c; } while(0)
//#define ds_pokes(x,t) do { const char * p = t; while (*p) ds_poke((x), *p++); } while(0)
#define ds_pokes(x,t) do {\
    int size = strlen(t);\
    ds_sure((x),size); \
    if((x).p&&!ds_last(x)) (x).p --;\
    strcpy((x).d+(x).p,t);\
    (x).p+=size;}while(0);
void ds_cat_(struct ds* str,...);
#define ds_cat(x,...) ds_cat_(&x,__VA_ARGS__)
const char* ds_itos(int n);
#define ds_c_str(x) (x.d)
#endif /* ds_init */
    
#endif
