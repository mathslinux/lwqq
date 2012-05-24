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

    /* Set http method: GET or POST, 0 mean get, 1 mean post */
    int (*set_method)(struct LwqqHttpRequest *request, int method);
    
    /* Send a request to server */
    int (*do_request)(struct LwqqHttpRequest *request, 
                      int *http_code, char **response, int *response_len);
    
    /* Set our http client header */
    void (*set_header)(struct LwqqHttpRequest *request, const char *name,
                       const char *value);

    /* Set default http header */
    void (*set_default_header)(struct LwqqHttpRequest *request);

    /**
     * Get header, return a alloca memory, so caller has responsibility
     * free the memory
     */
    char * (*get_header)(struct LwqqHttpRequest *request, const char *name);

    /**
     * Get Cookie, return a alloca memory, so caller has responsibility
     * free the memory
     */
    char * (*get_cookie)(struct LwqqHttpRequest *request, const char *name);
    
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
 * @param method Http method. e.g. 0 mean GET, 1 mean POST
 * 
 * @return 
 */
LwqqHttpRequest *lwqq_http_request_new(const char *uri, int method);

#endif  /* LWQQ_HTTP_H */
