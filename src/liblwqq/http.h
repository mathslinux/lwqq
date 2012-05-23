/**
 * @file   http.h
 * @author mathslinux <riegamaths@gmail.com>
 * @date   Mon May 21 23:07:29 2012
 * 
 * @brief  Linux WebQQ Http API
 * 
 * 
 */

#ifndef LWQQ_HTTP_H
#define LWQQ_HTTP_H

/**
 * Http request struct
 * 
 */
typedef struct LwqqHttpRequest {
    void *req;
    /* Send a request to server */
    int (*do_request)(struct LwqqHttpRequest *request, 
                      int *http_code, char **response, int *response_len);
    
    /* Set our http client header */
    void (*set_header)(struct LwqqHttpRequest *request, const char *name,
                       const char *value);

    /* Set default http header */
    void (*set_default_header)(struct LwqqHttpRequest *request);

    /* Get header, value will be stored in output which user passed */
    char * (*get_header)(struct LwqqHttpRequest *request, const char *name,
                         char *output, int maxlen);

    /* Get Cookie, value will be stored in output which user passed */
    char * (*get_cookie)(struct LwqqHttpRequest *request, const char *name,
                         char *output, int maxlen);
    
} LwqqHttpRequest;

/** 
 * Free Http Request
 * 
 * @param request 
 */
void lwqq_http_request_free(LwqqHttpRequest *request);

/** 
 * Create a new Http request instance
 * 
 * @param uri Request service from
 * 
 * @return 
 */
LwqqHttpRequest *lwqq_http_request_new(const char *uri);

#endif  /* LWQQ_HTTP_H */
