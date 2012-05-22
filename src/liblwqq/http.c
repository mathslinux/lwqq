#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ghttp.h>
#include "smemory.h"
#include "http.h"
#include "logger.h"

static int lwqq_http_do_request(LwqqHttpRequest *request, int *http_code,
                                char **response, int *response_len);
static void lwqq_http_set_header(LwqqHttpRequest *request, const char *name,
                                 const char *value);

static void lwqq_http_set_header(LwqqHttpRequest *request, const char *name,
                                const char *value)
{
    if (!request->req || !name || !value)
        return ;

    ghttp_set_header(request->req, name, value);
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
    request = s_malloc(sizeof(*request));
    memset(request, 0, sizeof(*request));
    
    request->req = ghttp_request_new();
    if (!request->req) {
        /* Seem like request->req must be non null. FIXME */
        goto failed;
    }
    if (ghttp_set_uri(request->req, (char *)uri) == -1) {
        lwqq_log(LOG_WARNING, "Invalid uri: %s\n", uri);
        goto failed;
    }
    if(ghttp_set_type(request->req, ghttp_type_get) == -1) {
        lwqq_log(LOG_WARNING, "Set request type error\n");
        goto failed;
    }

    request->do_request = lwqq_http_do_request;
    request->set_header = lwqq_http_set_header;
    return request;

failed:
    if (request) {
        lwqq_http_request_free(request);
    }
    return NULL;
}

static int lwqq_http_do_request(LwqqHttpRequest *request, int *http_code,
                         char **response, int *response_len)
{
    if (!request->req || !http_code || !response || !response_len)
        return -1;

    ghttp_status status;
    char *buf;
    int have_read_bytes = 0;

    *response = NULL;
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
            *response = s_realloc(*response, have_read_bytes + len);
            memcpy(*response + have_read_bytes, buf, len);
            have_read_bytes += len;
        }
        if(status == ghttp_done) {
            /* NOTE: Ok, done */
            break;
        }
    }
    
    /* OK, done */
    *response_len = have_read_bytes;
    *http_code = ghttp_status_code(request->req);
    printf("Status code -> %d\n", ghttp_status_code(request->req));
    return 0;

failed:
    return -1;
}

#if 0
int main(int argc, char *argv[])
{
    char *uri = "http://www.google.com";
    LwqqHttpRequest *req = lwqq_http_request_new(uri);
    if (req) {
        int http_code;
        char *response;
        int response_len;
        int ret;
        ret = req->do_request(req, &http_code, &response, &response_len);
        if (ret == 0) {
            printf ("code: %d\n", http_code);
            printf ("buf: %s\n", response);
            if (response)
                free(response);
        }
        lwqq_http_request_free(req);
    }
        
    return 0;
}
#endif
