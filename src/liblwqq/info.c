/**
 * @file   info.c
 * @author mathslinux <riegamaths@gmail.com>
 * @date   Sun May 27 19:48:58 2012
 * 
 * @brief  Fetch QQ information. e.g. friends information, group information.
 * 
 * 
 */

#include "info.h"
#include "url.h"
#include "logger.h"
#include "http.h"
#include "smemory.h"
#include "json.h"

void lwqq_info_get_friends_info(LwqqClient *lc, LwqqErrorCode *err)
{
    char msg[256] ={0};
    char *buf;
    LwqqHttpRequest *req = NULL;  
    int ret;
    json_t *json = NULL;

    if (!err) {
        *err = LWQQ_ERROR;
        goto done ;
    }

    snprintf(msg, sizeof(msg), "{\"h\":\"hello\",\"vfwebqq\":\"%s\"}",
             lc->vfwebqq);
    buf = url_encode(msg);
    snprintf(msg, sizeof(msg), "r=%s", buf);
    s_free(buf);

    /* Create a POST request */
    char url[512];
    snprintf(url, sizeof(url), "%s/api/get_user_friends2", "http://s.web2.qq.com");
    req = lwqq_http_create_default_request(url, err);
    if (!req) {
        goto done;
    }
    req->set_header(req, "Referer", "http://s.web2.qq.com/proxy.html?v=20101025002");
    req->set_header(req, "Content-Transfer-Encoding", "binary");
    req->set_header(req, "Content-type", "application/x-www-form-urlencoded");
    if (lc->cookie)
        req->set_header(req, "Cookie", lc->cookie);
    ret = req->do_request(req, 1, msg);
    if (ret) {
        *err = LWQQ_NETWORK_ERROR;
        goto done;
    }
    if (req->http_code != 200) {
        *err = LWQQ_HTTP_ERROR;
        goto done;
    }

    printf ("\n========================================\n");
    ret = json_parse_document(&json, req->response);
    if (ret != JSON_OK) {
        printf ("ret is %d\n", ret);
        *err = LWQQ_ERROR;
        goto done;
    }
    char *text = NULL;
    json_tree_to_string (json, &text);
    if (text) {
        s_free(text);
        puts(text);
    }
    char *value;
    if ((value = json_parse_simple_value(json, "uin"))) {
        printf ("=======%s\n", value);
    }
//    puts(req->response);
    printf ("========================================\n\n");
    /**
     * Here, we got a json object like this:
     * {"retcode":0,"result":{"friends":[{"flag":0,"uin":1907104721,"categories":0},{"flag":0,"uin":1745701153,"categories":0},{"flag":0,"uin":445284794,"categories":0},{"flag":0,"uin":4188952283,"categories":0},{"flag":0,"uin":276408653,"categories":0},{"flag":0,"uin":1526867107,"categories":0}],"marknames":[{"uin":276408653,"markname":""}],"categories":[{"index":1,"sort":1,"name":""},{"index":2,"sort":2,"name":""},{"index":3,"sort":3,"name":""}],"vipinfo":[{"vip_level":1,"u":1907104721,"is_vip":1},{"vip_level":1,"u":1745701153,"is_vip":1},{"vip_level":1,"u":445284794,"is_vip":1},{"vip_level":6,"u":4188952283,"is_vip":1},{"vip_level":0,"u":276408653,"is_vip":0},{"vip_level":1,"u":1526867107,"is_vip":1}],"info":[{"face":294,"flag":8389126,"nick":"","uin":1907104721},{"face":0,"flag":518,"nick":"","uin":1745701153},{"face":0,"flag":526,"nick":"","uin":445284794},{"face":717,"flag":8388614,"nick":"QQ","uin":4188952283},{"face":81,"flag":8389186,"nick":"Kernel","uin":276408653},{"face":0,"flag":2147484166,"nick":"Q","uin":1526867107}]}}
     * 
     */
done:
    if (json)
        json_free_value(&json);
    lwqq_http_request_free(req);
}
