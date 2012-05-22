/**
 * @file   login.c
 * @author mathslinux <riegamaths@gmail.com>
 * @date   Sun May 20 02:21:43 2012
 * 
 * @brief  Linux WebQQ Login API
 * 
 * 
 */

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

    snprintf(uri, sizeof(uri), "%s%s?uin=%sappid=%s", LWQQ_LOGIN_HOST,
             VCCHECKPATH, lc->username, APPID);
    req = lwqq_http_request_new(uri);
    if (!req) {
        lwqq_log(LOG_ERROR, "Create request instance failed\n");
        *err = LWQQ_ERROR;
        goto failed;
    }
    
    lwqq_log(LOG_NOTICE, "Send a request to: %s\n", uri);
    ret = req->do_request(req, &http_code, &response, &response_len);
    if (ret) {
        *err = LWQQ_NETWORK_ERROR;
        goto failed;
    }

    lwqq_log(LOG_NOTICE, "Get response verify code: %s\n", response);
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
