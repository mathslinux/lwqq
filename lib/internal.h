#ifndef LWQQ_INTERNAL_H_H
#define LWQQ_INTERNAL_H_H
#include "type.h"
#include "json.h"
#include "lwqq-config.h"
#include "async.h"
#ifdef WIN32
#include "win32.h"
#endif

#ifndef LWQQ_ENABLE_SSL
#define LWQQ_ENABLE_SSL 0
#endif

#if LWQQ_ENABLE_SSL
//ssl switcher
#define SSL_(ssl,normal) ssl
#else
#define SSL_(ssl,normal) normal
#endif

#define H_ SSL_("https://","http://")
//normal ssl switcher
#define S_(normal) SSL_("ssl.",normal)
//standard http header+ssl switcher
#define H_S_ H_ S_("")

#define WEBQQ_PROXY SSL_("cfproxy.html?v=20110331002&callback=1","proxy.html?v=20110331002&callback=1")

#define WEBQQ_LOGIN_UI_HOST H_"ui.ptlogin2.qq.com"
#define WEBQQ_CHECK_HOST    H_ S_("check.")"ptlogin2.qq.com"
#define WEBQQ_LOGIN_HOST    H_S_"ptlogin2.qq.com"
#define WEBQQ_CAPTCHA_HOST  H_S_"captcha.qq.com"
#define WEBQQ_D_HOST        H_"d.web2.qq.com"
#define WEBQQ_S_HOST        "http://s.web2.qq.com"

#define WEBQQ_D_REF_URL     WEBQQ_D_HOST"/"WEBQQ_PROXY
#define WEBQQ_S_REF_URL     WEBQQ_S_HOST"/proxy.html?v=201103311002&callback=1"
#define WEBQQ_LOGIN_REF_URL WEBQQ_LOGIN_HOST"/proxy.html"
#define WEBQQ_VERSION_URL   WEBQQ_LOGIN_UI_HOST"/cgi-bin/ver"

#define WEBQQ_LOGIN_LONG_REF_URL(buf) (snprintf(buf,sizeof(buf),\
            WEBQQ_LOGIN_UI_HOST"/cgi-bin/login?daid=164&target=self&style=5&mibao_css=m_webqq&appid=1003903&enable_qlogin=0&no_verifyimg=1&s_url=http%%3A%%2F%%2Fweb2.qq.com%%2Floginproxy.html&f_url=loginerroralert&strong_login=1&login_stat=%d&t=%lu",lc->stat,LTIME),buf)


#define slist_free_all(list) \
while(list!=NULL){ \
    void *ptr = list; \
    list = list->next; \
    s_free(ptr); \
}
#define slist_append(list,node) \
(node->next = list,node)

struct str_list_{
    char* str;
    struct str_list_* next;
};

struct str_list_* str_list_prepend(struct str_list_* list,const char* str);
#define str_list_free_all(list) \
while(list!=NULL){ \
    struct str_list_ *ptr = list; \
    list = list->next; \
    s_free(ptr->str);\
    s_free(ptr); \
}

int lwqq__get_retcode_from_str(const char* str);
json_t *lwqq__parse_retcode_result(json_t *json,int* retcode);

LwqqAsyncEvent* lwqq__request_captcha(LwqqClient* lc,LwqqVerifyCode* c);
int lwqq__process_empty(LwqqHttpRequest* req);

#define lwqq__jump_if_http_fail(req,err) if(req->http_code !=200) {err=LWQQ_EC_ERROR;goto done;}
#define lwqq__jump_if_json_fail(json,str,err) \
    if(json_parse_document(&json,str)!=JSON_OK){\
        lwqq_log(LOG_ERROR, "Parse json object of add friend error: %s\n", str);\
        err = LWQQ_EC_NOT_JSON_FORMAT; goto done;  }
#define lwqq__jump_if_retcode_fail(retcode) if(retcode != LWQQ_EC_OK) goto done;
#define lwqq__jump_if_ev_fail(ev,err) do{\
    if(ev->failcode != LWQQ_CALLBACK_VALID){err=LWQQ_EC_ERROR;goto done;}\
    if(ev->result != LWQQ_EC_OK){err=LWQQ_EC_ERROR;goto done;}\
}while(0);

#define lwqq__return_if_ev_fail(ev) do{\
    if(ev->failcode != LWQQ_CALLBACK_VALID) return;\
    if(ev->result != LWQQ_EC_OK) return;\
}while(0);

#define lwqq__clean_json_and_req(json,req) do{\
    if(json) json_free_value(&json);\
    lwqq_http_request_free(req);\
}while(0);

#define lwqq__log_if_error(err,req) if(err) lwqq_log(LOG_ERROR,"unexpected response \n\thttp:%d, response:\n\t%s\n",\
		req->http_code,req->response);
#define lwqq__has_post() (lwqq_verbose(3,"%s\n%s\n",url,post),1),post
#define lwqq__hasnot_post() (lwqq_verbose(3,"%s\n",url),0),NULL
#define __LWQQ_API_LEVEL_4__ if(LWQQ_VERBOSE_LEVEL>=4)\
													lwqq_http_set_option(req, LWQQ_HTTP_VERBOSE,1L);

/** ===================json part==================*/
#define lwqq__json_get_int(json,k,def) s_atoi(json_parse_simple_value(json,k),def)
#define lwqq__json_get_long(json,k,def) s_atol(json_parse_simple_value(json,k),def)
#define lwqq__json_get_value(json,k) s_strdup(json_parse_simple_value(json,k))
#define lwqq__json_get_string(json,k) json_unescape_s(json_parse_simple_value(json,k))
#define lwqq__json_parse_child(json,k,sub) sub=json_find_first_label(json,k);if(sub) sub=sub->child;

//json function expand
json_t *json_find_first_label_all (const json_t * json, const char *text_label);
char *json_parse_simple_value(json_t *json, const char *key);
char *json_unescape_s(char* str);

#endif

