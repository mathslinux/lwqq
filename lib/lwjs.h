#ifndef LWQQ_JS_H_H
#define LWQQ_JS_H_H
#include "lwqq-config.h"

typedef struct lwqq_js_t lwqq_js_t;
typedef void   lwqq_jso_t;

lwqq_js_t* lwqq_js_init();
void lwqq_js_close(lwqq_js_t* js);
#ifdef WITH_MOZJS
lwqq_jso_t* lwqq_js_load(lwqq_js_t* js,const char* file);
void lwqq_js_unload(lwqq_js_t* js,lwqq_jso_t* obj);
#endif


char* lwqq_js_hash(const char* uin,const char* ptwebqq,lwqq_js_t* js);
#endif
