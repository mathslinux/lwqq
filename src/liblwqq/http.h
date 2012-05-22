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
    int (*do_request)(struct LwqqHttpRequest *request, 
                      int *http_code, char **response, int *response_len);
    void (*set_header)(struct LwqqHttpRequest *request, const char *name,
                       const char *value);
    
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
