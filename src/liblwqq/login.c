/**
 * @file   login.c
 * @author mathslinux <riegamaths@gmail.com>
 * @date   Sun May 20 02:21:43 2012
 * 
 * @brief  Linux WebQQ Login API
 * 
 * 
 */

#include <string.h>
#include "login.h"
#include "logger.h"
#include "http.h"
#include "smemory.h"

static void get_verify_code(LwqqClient *lc, LwqqErrorCode *err)
{
#define LWQQ_LOGIN_HOST "http://ptlogin2.qq.com"
#define VCCHECKPATH "/check"
#define APPID "1003903"
    if (!err)
        return ;

    LwqqHttpRequest *req;  
    char uri[512];
    int http_code;
    char *response = NULL;
    int response_len;
    int ret;

    snprintf(uri, sizeof(uri), "%s%s?uin=%s&appid=%s", LWQQ_LOGIN_HOST,
             VCCHECKPATH, lc->username, APPID);
    req = lwqq_http_request_new(uri);
    if (!req) {
        lwqq_log(LOG_ERROR, "Create request instance failed\n");
        *err = LWQQ_ERROR;
        goto failed;
    }
    
    lwqq_log(LOG_NOTICE, "Send a request to: %s\n", uri);
    req->set_default_header(req);
    ret = req->do_request(req, &http_code, &response, &response_len);
    if (ret) {
        *err = LWQQ_NETWORK_ERROR;
        goto failed;
    }
    if (http_code != 200) {
        *err = LWQQ_HTTP_ERROR;
        goto failed;
    }
    lwqq_log(LOG_NOTICE, "Get response verify code: %s\n", response);

    /**
     * 
	 * The http message body has two format:
	 *
	 * ptui_checkVC('1','9ed32e3f644d968809e8cbeaaf2cce42de62dfee12c14b74');
	 * ptui_checkVC('0','!LOB');
	 * The former means we need verify code image and the second
	 * parameter is vc_type.
	 * The later means we don't need the verify code image. The second
	 * parameter is the verify code. The vc_type is in the header
	 * "Set-Cookie".
	 */
    char *c = strstr(response, "ptui_checkVC");
    char *s;
    if (!c) {
        *err = LWQQ_HTTP_ERROR;
        goto failed;
    }
    c = strchr(response, '\'');
    if (!c) {
        *err = LWQQ_HTTP_ERROR;
        goto failed;
    }
    c++;
    lc->vc = s_malloc0(sizeof(*lc->vc));
    if (*c == '0') {
        /* We got the verify code. */
        s = c;
        c = strstr(s, "'");
        s = c + 1;
        c = strstr(s, "'");
        s = c + 1;
        c = strstr(s, "'");
        *c = '\0';
        lc->vc->type = s_strdup("0");
        lc->vc->str = s_strdup(s);
        lwqq_log(LOG_NOTICE, "Verify code: %s\n", lc->vc->str);
    } else if (*c == '1') {
        /* We need get the verify image. */
        s = c;
        c = strstr(s, "'");
        s = c + 1;
        c = strstr(s, "'");
        s = c + 1;
        c = strstr(s, "'");
        *c = '\0';
        lc->vc->type = s_strdup("1");
        lc->vc->str = s_strdup(s);
        lwqq_log(LOG_NOTICE, "We need verify code image: %s\n", lc->vc->str);
    }
    
    s_free(response);
    lwqq_http_request_free(req);
    return ;
    
failed:
    s_free(response);
    lwqq_http_request_free(req);
}

static void calculate_password_md5(LwqqClient *lc)
{
    
}

static void do_login(LwqqClient *lc, LwqqErrorCode *err)
{
    if (!err)
        return ;
}

/** 
 * WebQQ login function
 * Step:
 * 1. Get verify code
 * 2. Calculate password's md5
 * 3. Do real login 
 * 4. check whether logining successfully
 * 
 * @param client Lwqq Client 
 * @param err Error code
 */
void lwqq_login(LwqqClient *client, LwqqErrorCode *err)
{
    if (!client || !err) {
        lwqq_log(LOG_ERROR, "Invalid pointer\n");
        return ;
    }

    /**
     * First, we get the verify code from server.
     * If server provide us a image and let us enter code shown
     * in image number, in this situation, we just return LWQQ_LOGIN_NEED_VC
     * simply, so user should call lwqq_login() again after he set correct
     * code to vc->str;
     * Else, if we can get the code directly, do login immediately.
     * 
     */
    if (!client->vc) {
        get_verify_code(client, err);
        switch (*err) {
        case LWQQ_LOGIN_NEED_VC:
            lwqq_log(LOG_WARNING, "Need to enter verify code\n");
            return ;
        
        case LWQQ_NETWORK_ERROR:
            lwqq_log(LOG_ERROR, "Network error\n");
            return ;

        case LWQQ_OK:
            lwqq_log(LOG_DEBUG, "Get verify code OK\n");
            break;

        default:
            lwqq_log(LOG_ERROR, "Unknown error\n");
            return ;
        }
    }

    calculate_password_md5(client);
    do_login(client, err);
}
