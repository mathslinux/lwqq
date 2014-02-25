#include "lwqq.h"
#include "lwjs.h"

#include <stdint.h>

typedef struct LwjsPlugin{
	LwqqPlugin super;
	LwqqCommand hash_backup;
	lwqq_js_t* js;
}LwjsPlugin;

#ifdef WITH_MOZJS

#include <jsapi.h>

struct lwqq_js_t {
    JSRuntime* runtime;
    JSContext* context;
    JSObject*  global;
};

static JSClass global_class = { 
    "global", 
    JSCLASS_GLOBAL_FLAGS|JSCLASS_NEW_RESOLVE, 
    JS_PropertyStub, 
    JS_PropertyStub, 
    JS_PropertyStub, 
    JS_StrictPropertyStub, 
    JS_EnumerateStub, 
    JS_ResolveStub, 
    JS_ConvertStub, 
    NULL, 
    JSCLASS_NO_OPTIONAL_MEMBERS 
 }; 

static void report_error(JSContext *cx,  const char *message, JSErrorReport *report)
{ 
    lwqq_verbose(3, "%s:%u:%s\n",
            report->filename ? report->filename : "<no filename>", 
            (unsigned int) report->lineno, 
            message); 
} 

lwqq_js_t* lwqq_js_init()
{
    lwqq_js_t* h = s_malloc0(sizeof(*h));
    h->runtime = JS_NewRuntime(8L*1024L*1024L);
    h->context = JS_NewContext(h->runtime, 16*1024);
    JS_SetOptions(h->context, 
	JSOPTION_VAROBJFIX|JSOPTION_COMPILE_N_GO|JSOPTION_NO_SCRIPT_RVAL);
    JS_SetErrorReporter(h->context, report_error);
#ifdef MOZJS_185
    h->global = JS_NewCompartmentAndGlobalObject(h->context, &global_class, NULL);
#else
    h->global = JS_NewGlobalObject(h->context,&global_class,NULL);
#endif
    JS_InitStandardClasses(h->context, h->global);
    return h;
}

lwqq_jso_t* lwqq_js_load(lwqq_js_t* js,const char* file)
{
    JSObject* global = JS_GetGlobalObject(js->context);
#ifdef MOZJS_185
    JSObject* script = JS_CompileFile(js->context, global, file);
#else
    JSScript* script = JS_CompileUTF8File(js->context,global,file);
#endif
    JS_ExecuteScript(js->context, global, script, NULL);
    return (lwqq_jso_t*)script;
}
void lwqq_js_load_buffer(lwqq_js_t* js,const char* content)
{
    JSObject* global = JS_GetGlobalObject(js->context);
    JS_EvaluateScript(js->context,global,content,strlen(content),NULL,0,NULL);
}
void lwqq_js_unload(lwqq_js_t* js,lwqq_jso_t* obj)
{
    //JS_DecompileScriptObject(js->context, obj, <#const char *name#>, <#uintN indent#>);
}

char* lwqq_js_hash(const char* uin,const char* ptwebqq,lwqq_js_t* js)
{
    JSObject* global = JS_GetGlobalObject(js->context);
    jsval res;
    jsval argv[2];
    char* res_;

    JSString* uin_ = JS_NewStringCopyZ(js->context, uin);
    JSString* ptwebqq_ = JS_NewStringCopyZ(js->context, ptwebqq);
    argv[0] = STRING_TO_JSVAL(uin_);
    argv[1] = STRING_TO_JSVAL(ptwebqq_);
    JS_CallFunctionName(js->context, global, "P", 2, argv, &res);

    res_ = JS_EncodeString(js->context,JSVAL_TO_STRING(res));

    return res_;
}

void lwqq_js_close(lwqq_js_t* js)
{
    JS_DestroyContext(js->context);
    JS_DestroyRuntime(js->runtime);
    JS_ShutDown();
    s_free(js);
}
#if 0

static void hash_func(char** result,LwqqClient* lc,LwjsPlugin* pl)
{
	s_free(*result);
	const char* uin = lc->myself->uin;
	const char* ptwebqq = lwqq_http_get_cookie(lwqq_get_http_handle(lc), "ptwebqq");
	char* res = lwqq_js_hash(uin, ptwebqq, pl->js);
	*result = res;
}

static void plugin_init(LwqqPlugin* plugin,LwqqClient* lc)
{
	LwjsPlugin* pl = (LwjsPlugin*)plugin;
	memcpy(&pl->hash_backup,&lc->events->hash_func,sizeof(LwqqCommand));
	memset(&lc->events->hash_func,0,sizeof(LwqqCommand));
	lwqq_add_event(lc->events->hash_func,_C_(3p,hash_func,&lc->args->hash_result,lc));
}

static void plugin_remove(LwqqPlugin* plugin,LwqqClient* lc)
{
	LwjsPlugin* pl = (LwjsPlugin*)plugin;
	vp_cancel(lc->events->hash_func);
	memcpy(&lc->events->hash_func,&pl->hash_backup,sizeof(LwqqCommand));
	memset(&pl->hash_backup,0,sizeof(LwqqCommand));
}
LwqqPlugin* lwjs_make_plugin(const char* filepath)
{
	LwjsPlugin* jsp = s_malloc0(sizeof(*jsp));
	lwqq_js_t* js = lwqq_js_init();
	lwqq_js_load(js,filepath);
	jsp->js = js;
	jsp->super.init = plugin_init;
	jsp->super.remove = plugin_remove;
	return (LwqqPlugin*)jsp;
}
void lwjs_clean_plugin(LwqqPlugin* pl)
{
	LwjsPlugin* jsp = (LwjsPlugin*)pl;
	s_free(pl);
}
#endif
#else

struct lwqq_js_t{
};
lwqq_js_t* lwqq_js_init()
{
	return NULL;
}
void lwqq_js_close(lwqq_js_t* js)
{
}
#endif

