/**
 * @file   login.c
 * @author mathslinux <riegamaths@gmail.com>
 * @date   Sun May 20 02:21:43 2012
 * 
 * @brief  Linux WebQQ Login API
 *  Login logic is based on the gtkqq implementation
 *  written by HuangCongyu <huangcongyu2006@gmail.com>
 * 
 * 
 */

#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <alloca.h>
#include <time.h>
#include <sys/time.h>
#include "login.h"
#include "logger.h"
#include "http.h"
#include "smemory.h"
#include "md5.h"
#include "url.h"
#include "json.h"

/* URL for webqq login */
#define LWQQ_URL_LOGIN_HOST "http://ptlogin2.qq.com"
#define LWQQ_URL_CHECK_HOST "http://check.ptlogin2.qq.com"
#define VCCHECKPATH "/check"
#define APPID "1003903"
#define LWQQ_URL_SET_STATUS "http://d.web2.qq.com/channel/login2"

/* URL for get webqq version */
#define LWQQ_URL_VERSION "http://ui.ptlogin2.qq.com/cgi-bin/ver"

static void set_online_status(LwqqClient *lc, char *status, LwqqErrorCode *err);

static void get_verify_code(LwqqClient *lc, LwqqErrorCode *err)
{
    LwqqHttpRequest *req;  
    char url[512];
    char *response = NULL;
    int ret;
    char chkuin[64];

    snprintf(url, sizeof(url), "%s%s?uin=%s&appid=%s", LWQQ_URL_CHECK_HOST,
             VCCHECKPATH, lc->username, APPID);
    req = lwqq_http_create_default_request(url, err);
    if (!req) {
        goto failed;
    }
    
    snprintf(chkuin, sizeof(chkuin), "chkuin=%s", lc->username);
    req->set_header(req, "Cookie", chkuin);
    ret = req->do_request(req, 0, NULL);
    if (ret) {
        *err = LWQQ_EC_NETWORK_ERROR;
        goto failed;
    }
    if (req->http_code != 200) {
        *err = LWQQ_EC_HTTP_ERROR;
        goto failed;
    }

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
    response = req->response;
    lwqq_log(LOG_NOTICE, "Get response verify code: %s\n", response);

    char *c = strstr(response, "ptui_checkVC");
    char *s;
    if (!c) {
        *err = LWQQ_EC_HTTP_ERROR;
        goto failed;
    }
    c = strchr(response, '\'');
    if (!c) {
        *err = LWQQ_EC_HTTP_ERROR;
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

        /* We need get the ptvfsession from the header "Set-Cookie" */
        char *ptvfsession = req->get_cookie(req, "ptvfsession");
        if (ptvfsession) {
            lc->ptvfsession = s_strdup(ptvfsession);
            s_free(ptvfsession);
        } else {
            lwqq_log(LOG_WARNING, "Cant get cookie ptvfsession\n");
        }
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
    
    lwqq_http_request_free(req);
    return ;
    
failed:
    lwqq_http_request_free(req);
}

/**
 * Calculate md5 by user's password
 * Workflow(thank kernelhcy):
 * First, compute check sum of password for three times.
 * Then, join the result with the capitalizaion of the verify code.
 * Compute the chekc sum of the new string.
 * 
 * @param lc
 *
 * @return The result of calculate
 */
static char *calculate_password_md5(LwqqClient *lc)
{
//    char *s = get_pwvc_md52(lc);
    unsigned char buf[128] = {0};
    int i;

    lutil_md5_digest((unsigned char *)lc->password, strlen(lc->password), (char *)buf);
    lutil_md5_digest(buf, 16 , (char *)buf);
    lutil_md5_data(buf, 16, (char *)buf);
    for (i = strlen((char *)buf) - 1; i >= 0; i--) {
        if (islower(buf[i]))
            buf[i] = toupper(buf[i]);
    }
    strncat((char *)buf, lc->vc->str, strlen(lc->vc->str) + 1);
    lutil_md5_data(buf, strlen((char *)buf), (char *)buf);
    for( i = strlen((char *)buf) - 1; i >= 0 ; i--) {
        if (islower(buf[i]))
            buf[i]= toupper(buf[i]);
    }
    
    lwqq_log(LOG_NOTICE, "Get password md5: %s\n", buf);
    return s_strdup((char *)buf);
}

static int sava_cookie(LwqqClient *lc, LwqqHttpRequest *req, LwqqErrorCode *err)
{
    /* FIXME, is cookie less than 2K always? */
    char cookie[2048];
    
    lc->ptcz = req->get_cookie(req, "ptcz");
    lc->skey = req->get_cookie(req, "skey");
    lc->ptwebqq = req->get_cookie(req, "ptwebqq");
    lc->ptuserinfo = req->get_cookie(req, "ptuserinfo");
    lc->uin = req->get_cookie(req, "uin");
    lc->ptisp = req->get_cookie(req, "ptisp");
    lc->pt2gguin = req->get_cookie(req, "pt2gguin");
    if (!lc->ptcz || !lc->skey || !lc->ptwebqq || !lc->ptuserinfo ||
        !lc->uin || !lc->ptisp || !lc->pt2gguin) {
        *err = LWQQ_EC_ERROR;
        lwqq_log(LOG_ERROR, "Parse cookie error\n");
        return -1;
    }
    snprintf(cookie, sizeof(cookie), "ptwebqq=%s; ptisp=%s; "
             "ptcz=%s; ptuserinfo=%s; skey=%s; uin=%s; pt2gguin=%s; ",
             lc->ptwebqq, lc->ptisp, lc->ptcz, lc->ptuserinfo,
             lc->skey, lc->uin, lc->pt2gguin);
    if (lc->cookie) {
        s_free(lc->cookie);
    }
    lc->cookie = s_strdup(cookie);
    return 0;
}

/** 
 * Do really login
 * 
 * @param lc 
 * @param md5 The md5 calculated from calculate_password_md5() 
 * @param err 
 */
static void do_login(LwqqClient *lc, const char *md5, LwqqErrorCode *err)
{
    char url[1024];
    LwqqHttpRequest *req;
    char *response = NULL;
    int ret;
    char ptvfsession[128];

    snprintf(url, sizeof(url), "%s/login?u=%s&p=%s&verifycode=%s&"
             "webqq_type=10&remember_uin=1&aid=1003903&login2qq=1&"
             "u1=http%%3A%%2F%%2Fweb.qq.com%%2Floginproxy.html"
             "%%3Flogin2qq%%3D1%%26webqq_type%%3D10&h=1&ptredirect=0&"
             "ptlang=2052&from_ui=1&pttype=1&dumy=&fp=loginerroralert&"
             "action=2-11-7438&mibao_css=m_webqq&t=1&g=1",
             LWQQ_URL_LOGIN_HOST, lc->username, md5, lc->vc->str);

    req = lwqq_http_create_default_request(url, err);
    if (!req) {
        goto done;
    }
    /* Setup http header */
    if (lc->ptvfsession) {
        snprintf(ptvfsession, sizeof(ptvfsession), "ptvfsession=%s", lc->ptvfsession);
        req->set_header(req, "Cookie", ptvfsession);
    }

    /* Send request */
    ret = req->do_request(req, 0, NULL);
    if (ret) {
        *err = LWQQ_EC_NETWORK_ERROR;
        goto done;
    }
    if (req->http_code != 200) {
        *err = LWQQ_EC_HTTP_ERROR;
        goto done;
    }

    response = req->response;
    char *p = strstr(response, "\'");
    if (!p) {
        *err = LWQQ_EC_ERROR;
        goto done;
    }
    char buf[4] = {0};
    int status;
    strncpy(buf, p + 1, 1);
    status = atoi(buf);

    switch (status) {
    case 0:
        if (sava_cookie(lc, req, err)) {
            goto done;
        }
        break;
        
    case 1:
        lwqq_log(LOG_WARNING, "Server busy! Please try again\n");
        *err = LWQQ_EC_ERROR;
        goto done;

    case 2:
        lwqq_log(LOG_ERROR, "Out of date QQ number\n");
        *err = LWQQ_EC_ERROR;
        goto done;

    case 3:
        lwqq_log(LOG_ERROR, "Wrong password\n");
        *err = LWQQ_EC_ERROR;
        goto done;

    case 4:
        lwqq_log(LOG_ERROR, "Wrong verify code\n");
        *err = LWQQ_EC_ERROR;
        goto done;

    case 5:
        lwqq_log(LOG_ERROR, "Verify failed\n");
        *err = LWQQ_EC_ERROR;
        goto done;

    case 6:
        lwqq_log(LOG_WARNING, "You may need to try login again\n");
        *err = LWQQ_EC_ERROR;
        goto done;

    case 7:
        lwqq_log(LOG_ERROR, "Wrong input\n");
        *err = LWQQ_EC_ERROR;
        goto done;

    case 8:
        lwqq_log(LOG_ERROR, "Too many logins on this IP. Please try again\n");
        *err = LWQQ_EC_ERROR;
        goto done;

    default:
        *err = LWQQ_EC_ERROR;
        lwqq_log(LOG_ERROR, "Unknow error");
        goto done;
    }

    set_online_status(lc, "online", err);
done:
    lwqq_http_request_free(req);
}

/**
 * Get WebQQ version from tencent server
 * The response is like "ptuiV(201205211530)", what we do is get "201205211530"
 * from the response and set it to lc->version.
 * 
 * @param lc 
 * @param err *err will be set LWQQ_EC_OK if everything is ok, else
 *        *err will be set LWQQ_EC_ERROR.
 */

static void get_version(LwqqClient *lc, LwqqErrorCode *err)
{
    LwqqHttpRequest *req;
    char *response = NULL;
    int ret;

    req = lwqq_http_create_default_request(LWQQ_URL_VERSION, err);
    if (!req) {
        goto done;
    }

    /* Send request */
    lwqq_log(LOG_DEBUG, "Get webqq version from %s\n", LWQQ_URL_VERSION);
    ret = req->do_request(req, 0, NULL);
    if (ret) {
        *err = LWQQ_EC_NETWORK_ERROR;
        goto done;
    }
    response = req->response;
    if (strstr(response, "ptuiV")) {
        char *s, *t;
        char *v;
        s = strchr(response, '(');
        t = strchr(response, ')');
        if (!s || !t) {
            *err = LWQQ_EC_ERROR;
            goto done;
        }
        s++;
        v = alloca(t - s + 1);
        memset(v, 0, t - s + 1);
        strncpy(v, s, t - s);
        lc->version = s_strdup(v);
        *err = LWQQ_EC_OK;
    }

done:
    lwqq_http_request_free(req);
}

static char *generate_clientid()
{
    int r;
    struct timeval tv;
    long t;
    char buf[20] = {0};
    
    srand(time(NULL));
    r = rand() % 90 + 10;
    if (gettimeofday(&tv, NULL)) {
        return NULL;
    }
    t = tv.tv_usec % 1000000;
    snprintf(buf, sizeof(buf), "%d%ld", r, t);
    return s_strdup(buf);
}

/** 
 * Set online status, this is the last step of login
 * 
 * @param err
 * @param lc 
 */
static void set_online_status(LwqqClient *lc, char *status, LwqqErrorCode *err)
{
    char msg[1024] ={0};
    char *ptwebqq;
    char *buf;
    LwqqHttpRequest *req = NULL;  
    char *response = NULL;
    int ret;
    json_t *json = NULL;
    char *value;

    if (!status || !err) {
        *err = LWQQ_EC_ERROR;
        goto done ;
    }

    lc->clientid = generate_clientid();
    if (!lc->clientid) {
        lwqq_log(LOG_ERROR, "Generate clientid error\n");
        *err = LWQQ_EC_ERROR;
        goto done ;
    }

    /* Do we really need ptwebqq */
    ptwebqq = lc->ptwebqq ? lc->ptwebqq : "";
    snprintf(msg, sizeof(msg), "{\"status\":\"%s\",\"ptwebqq\":\"%s\","
             "\"passwd_sig\":""\"\",\"clientid\":\"%s\""
             ", \"psessionid\":null}"
             ,status, lc->ptwebqq
             ,lc->clientid);
    buf = url_encode(msg);
    snprintf(msg, sizeof(msg), "r=%s", buf);
    s_free(buf);

    /* Create a POST request */
    req = lwqq_http_create_default_request(LWQQ_URL_SET_STATUS, err);
    if (!req) {
        goto done;
    }

    /* Set header needed by server */
    req->set_header(req, "Cookie2", "$Version=1");
    req->set_header(req, "Referer", "http://d.web2.qq.com/proxy.html?v=20101025002");
    req->set_header(req, "Content-type", "application/x-www-form-urlencoded");
    
    /* Set http cookie */
    if (lc->cookie)
        req->set_header(req, "Cookie", lc->cookie);
    
    ret = req->do_request(req, 1, msg);
    if (ret) {
        *err = LWQQ_EC_NETWORK_ERROR;
        goto done;
    }
    if (req->http_code != 200) {
        *err = LWQQ_EC_HTTP_ERROR;
        goto done;
    }

    /**
     * Here, we got a json object like this:
     * {"retcode":0,"result":{"uin":1421032531,"cip":2013211875,"index":1060,"port":43415,"status":"online","vfwebqq":"e7ce7913336ad0d28de9cdb9b46a57e4a6127161e35b87d09486001870226ec1fca4c2ba31c025c7","psessionid":"8368046764001e636f6e6e7365727665725f77656271714031302e3133332e34312e32303200006b2900001544016e0400533cb3546d0000000a4046674d4652585136496d00000028e7ce7913336ad0d28de9cdb9b46a57e4a6127161e35b87d09486001870226ec1fca4c2ba31c025c7","user_state":0,"f":0}}
     * 
     */
    response = req->response;
    ret = json_parse_document(&json, response);
    if (ret != JSON_OK) {
        *err = LWQQ_EC_ERROR;
        goto done;
    }

    if (!(value = json_parse_simple_value(json, "retcode"))) {
        *err = LWQQ_EC_ERROR;
        goto done;
    }
    /**
     * Do we need parse "seskey? from kernelhcy's code, we need it,
     * but from the response we got like above, we dont need
     * 
     */
    if ((value = json_parse_simple_value(json, "seskey"))) {
        lc->seskey = s_strdup(value);
    }

    if ((value = json_parse_simple_value(json, "cip"))) {
        lc->cip = s_strdup(value);
    }

    if ((value = json_parse_simple_value(json, "index"))) {
        lc->index = s_strdup(value);
    }

    if ((value = json_parse_simple_value(json, "port"))) {
        lc->port = s_strdup(value);
    }

    if ((value = json_parse_simple_value(json, "status"))) {
        /* This really need? */
        lc->status = s_strdup(value);
    }

    if ((value = json_parse_simple_value(json, "vfwebqq"))) {
        lc->vfwebqq = s_strdup(value);
    }

    if ((value = json_parse_simple_value(json, "psessionid"))) {
        lc->psessionid = s_strdup(value);
    }

    *err = LWQQ_EC_OK;
    
done:
    if (json)
        json_free_value(&json);
    lwqq_http_request_free(req);
}

/** 
 * WebQQ login function
 * Step:
 * 1. Get webqq version
 * 2. Get verify code
 * 3. Calculate password's md5
 * 4. Do real login 
 * 5. check whether logining successfully
 * 
 * @param client Lwqq Client 
 * @param err Error code
 */
void lwqq_login(LwqqClient *client, LwqqErrorCode *err)
{
    if (!client || !err) {
        *err = LWQQ_EC_ERROR;
        lwqq_log(LOG_ERROR, "Invalid pointer\n");
        return ;
    }

    /* First: get webqq version */
    get_version(client, err);
    if (*err) {
        lwqq_log(LOG_ERROR, "Get webqq version error\n");
        return ;
    }
    lwqq_log(LOG_NOTICE, "Get webqq version: %s\n", client->version);

    /**
     * Second, we get the verify code from server.
     * If server provide us a image and let us enter code shown
     * in image number, in this situation, we just return LWQQ_EC_LOGIN_NEED_VC
     * simply, so user should call lwqq_login() again after he set correct
     * code to vc->str;
     * Else, if we can get the code directly, do login immediately.
     * 
     */
    if (!client->vc) {
        get_verify_code(client, err);
        switch (*err) {
        case LWQQ_EC_LOGIN_NEED_VC:
            lwqq_log(LOG_WARNING, "Need to enter verify code\n");
            return ;
        
        case LWQQ_EC_NETWORK_ERROR:
            lwqq_log(LOG_ERROR, "Network error\n");
            return ;

        case LWQQ_EC_OK:
            lwqq_log(LOG_DEBUG, "Get verify code OK\n");
            break;

        default:
            lwqq_log(LOG_ERROR, "Unknown error\n");
            return ;
        }
    }
    
    /* Third: calculate the md5 */
    char *md5 = calculate_password_md5(client);

    /* Last: do real login */
    do_login(client, md5, err);
    s_free(md5);
}

/** 
 * WebQQ logout function
 * 
 * @param client Lwqq Client 
 * @param err Error code
 */
void lwqq_logout(LwqqClient *client, LwqqErrorCode *err)
{
    char url[512];
    LwqqHttpRequest *req = NULL;  
    int ret;
    json_t *json = NULL;
    char *value;
    struct timeval tv;
    long int re;

    if (!client || !err) {
        *err = LWQQ_EC_ERROR;
        lwqq_log(LOG_ERROR, "Invalid pointer\n");
        return ;
    }

    /* Get the milliseconds of now */
    if (gettimeofday(&tv, NULL)) {
        *err = LWQQ_EC_ERROR;
        return ;
    }
    re = tv.tv_usec / 1000;
    re += tv.tv_sec;
    
    /* Do we really need ptwebqq */
    snprintf(url, sizeof(url), "%s/channel/logout2?clientid=%s&psessionid=%s&t=%ld",
             "http://d.web2.qq.com", client->clientid, client->psessionid, re);

    /* Create a GET request */
    req = lwqq_http_create_default_request(url, err);
    if (!req) {
        goto done;
    }

    /* Set header needed by server */
    req->set_header(req, "Referer", "http://ptlogin2.qq.com/proxy.html?v=20101025002");
    
    /* Set http cookie */
    if (client->cookie)
        req->set_header(req, "Cookie", client->cookie);
    
    ret = req->do_request(req, 0, NULL);
    if (ret) {
        lwqq_log(LOG_ERROR, "Send logout request failed\n");
        *err = LWQQ_EC_NETWORK_ERROR;
        goto done;
    }
    if (req->http_code != 200) {
        *err = LWQQ_EC_HTTP_ERROR;
        goto done;
    }

    ret = json_parse_document(&json, req->response);
    if (ret != JSON_OK) {
        *err = LWQQ_EC_ERROR;
        goto done;
    }

    /* Check whether logout correctly */
    value = json_parse_simple_value(json, "retcode");
    if (!value || strcmp(value, "0")) {
        *err = LWQQ_EC_ERROR;
        goto done;
    }
    value = json_parse_simple_value(json, "result");
    if (!value || strcmp(value, "ok")) {
        *err = LWQQ_EC_ERROR;
        goto done;
    }

    /* Ok, seems like all thing is ok */
    *err = LWQQ_EC_OK;
    
done:
    if (json)
        json_free_value(&json);
    lwqq_http_request_free(req);    
}
