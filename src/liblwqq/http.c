#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ghttp.h>
#include "smemory.h"
#include "http.h"
#include "logger.h"

#define LWQQ_HTTP_USER_AGENT "User-Agent: Mozilla/5.0 \
(X11; Linux x86_64; rv:10.0) Gecko/20100101 Firefox/10.0"

static int lwqq_http_do_request(LwqqHttpRequest *request, int method, char *body);
static void lwqq_http_set_header(LwqqHttpRequest *request, const char *name,
                                 const char *value);
static void lwqq_http_set_default_header(LwqqHttpRequest *request);
static char *lwqq_http_get_header(LwqqHttpRequest *request, const char *name);
static char *lwqq_http_get_cookie(LwqqHttpRequest *request, const char *name);

static void lwqq_http_set_header(LwqqHttpRequest *request, const char *name,
                                const char *value)
{
    if (!request->req || !name || !value)
        return ;

    ghttp_set_header(request->req, name, value);
}

static void lwqq_http_set_default_header(LwqqHttpRequest *request)
{
    lwqq_http_set_header(request, "User-Agent", LWQQ_HTTP_USER_AGENT);
    lwqq_http_set_header(request, "Accept", "text/html, application/xml;q=0.9, "
                         "application/xhtml+xml, image/png, image/jpeg, "
                         "image/gif, image/x-xbitmap, */*;q=0.1");
    lwqq_http_set_header(request, "Accept-Language", "en-US,zh-CN,zh;q=0.9,en;q=0.8");
    lwqq_http_set_header(request, "Accept-Charset", "GBK, utf-8, utf-16, *;q=0.1");
    lwqq_http_set_header(request, "Accept-Encoding", "deflate, gzip, x-gzip, "
                         "identity, *;q=0");
    lwqq_http_set_header(request, "Connection", "Keep-Alive");
}

static char *lwqq_http_get_header(LwqqHttpRequest *request, const char *name)
{
    if (!name) {
        lwqq_log(LOG_ERROR, "Invalid parameter\n");
        return NULL; 
    }

    const char *h = ghttp_get_header(request->req, name);
    if (!h) {
        lwqq_log(LOG_WARNING, "Cant get http header\n");
        return NULL;
    }

    return s_strdup(h);
}

static char *lwqq_http_get_cookie(LwqqHttpRequest *request, const char *name)
{
    if (!name) {
        lwqq_log(LOG_ERROR, "Invalid parameter\n");
        return NULL; 
    }
    
    char *cookie = ghttp_get_cookie(request->req, name);
    if (!cookie) {
        lwqq_log(LOG_WARNING, "No cookie: %s\n", name);
        return NULL;
    }

    lwqq_log(LOG_DEBUG, "Parse Cookie: %s=%s\n", name, cookie);
    return cookie;
}

/** 
 * Free Http Request
 * 
 * @param request 
 */
void lwqq_http_request_free(LwqqHttpRequest *request)
{
    if (!request)
        return ;
    
    if (request) {
        s_free(request->response);
        ghttp_request_destroy(request->req);
        s_free(request);
    }
}

/** 
 * Create a new Http request instance
 *
 * @param uri Request service from
 * 
 * @return 
 */
LwqqHttpRequest *lwqq_http_request_new(const char *uri)
{
    if (!uri) {
        return NULL;
    }

    LwqqHttpRequest *request;
    request = s_malloc0(sizeof(*request));
    
    request->req = ghttp_request_new();
    if (!request->req) {
        /* Seem like request->req must be non null. FIXME */
        goto failed;
    }
    if (ghttp_set_uri(request->req, (char *)uri) == -1) {
        lwqq_log(LOG_WARNING, "Invalid uri: %s\n", uri);
        goto failed;
    }

    request->do_request = lwqq_http_do_request;
    request->set_header = lwqq_http_set_header;
    request->set_default_header = lwqq_http_set_default_header;
    request->get_header = lwqq_http_get_header;
    request->get_cookie = lwqq_http_get_cookie;
    return request;

failed:
    if (request) {
        lwqq_http_request_free(request);
    }
    return NULL;
}

static int lwqq_http_do_request(LwqqHttpRequest *request, int method, char *body)
{
    if (!request->req)
        return -1;

    ghttp_status status;
    char *buf;
    int have_read_bytes = 0;
    ghttp_type m;
    char **resp = &request->response;

    /* Clear off last response */
    if (*resp) {
        s_free(*resp);
        *resp = NULL;
    }

    /* Set http method */
    if (method == 0) {
        m = ghttp_type_get;
    } else if (method == 1) {
        m = ghttp_type_post;
    } else {
        lwqq_log(LOG_WARNING, "Wrong http method\n");
        goto failed;
    }
    if (ghttp_set_type(request->req, m) == -1) {
        lwqq_log(LOG_WARNING, "Set request type error\n");
        goto failed;
    }

    /* For POST method, set http body */
    if (m == ghttp_type_post && body) {
        ghttp_set_body(request->req, body, strlen(body));
    }
    
    if (ghttp_prepare(request->req)) {
        goto failed;
    }

    for ( ; ; ) {
        int len = 0;
        status = ghttp_process(request->req);
        if(status == ghttp_error) {
            goto failed;
        }
        /* NOTE: buf may NULL, notice it */
        buf = ghttp_get_body(request->req);
        if (buf) {
            len = ghttp_get_body_len(request->req);
            *resp = s_realloc(*resp, have_read_bytes + len);
            memcpy(*resp + have_read_bytes, buf, len);
            have_read_bytes += len;
        }
        if(status == ghttp_done) {
            /* NOTE: Ok, done */
            break;
        }
    }

    /* NB: *response may null */
    if (*resp == NULL) {
        goto failed;
    }
    
    /* OK, done */
    /* Realloc a byte, cause *resp has no termial char '\0' */
    *resp = s_realloc(*resp, have_read_bytes + 1);
    (*resp)[have_read_bytes] = '\0';
    request->http_code = ghttp_status_code(request->req);
    return 0;

failed:
    if (*resp) {
        s_free(*resp);
        *resp = NULL;
    }
    return -1;
}

#if 0
int main(int argc, char *argv[])
{
    if (argc != 2)
        return -1;
    
    char *uri = argv[1];
    LwqqHttpRequest *req = lwqq_http_request_new(uri);
    if (req) {
        int ret = 0;
        ret = req->do_request(req, 0, NULL);
        ret = req->do_request(req, 0, NULL);
        ret = req->do_request(req, 0, NULL);
        ret = req->do_request(req, 0, NULL);
        if (ret == 0) {
            printf ("code: %d\n", req->http_code);
            printf ("buf: %s\n", req->response);
        }
        lwqq_http_request_free(req);
    }
        
    return 0;
}
#endif
