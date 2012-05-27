/**
 * @file   info.c
 * @author mathslinux <riegamaths@gmail.com>
 * @date   Sun May 27 19:48:58 2012
 * 
 * @brief  Fetch QQ information. e.g. friends information, group information.
 * 
 * 
 */

#include <string.h>
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
    json_t *json = NULL, *json_tmp;
    char *value = NULL;

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

    /**
     * Here, we got a json object like this:
     * {"retcode":0,"result":{"friends":[{"flag":0,"uin":1907104721,"categories":0},{"flag":0,"uin":1745701153,"categories":0},{"flag":0,"uin":445284794,"categories":0},{"flag":0,"uin":4188952283,"categories":0},{"flag":0,"uin":276408653,"categories":0},{"flag":0,"uin":1526867107,"categories":0}],"marknames":[{"uin":276408653,"markname":""}],"categories":[{"index":1,"sort":1,"name":""},{"index":2,"sort":2,"name":""},{"index":3,"sort":3,"name":""}],"vipinfo":[{"vip_level":1,"u":1907104721,"is_vip":1},{"vip_level":1,"u":1745701153,"is_vip":1},{"vip_level":1,"u":445284794,"is_vip":1},{"vip_level":6,"u":4188952283,"is_vip":1},{"vip_level":0,"u":276408653,"is_vip":0},{"vip_level":1,"u":1526867107,"is_vip":1}],"info":[{"face":294,"flag":8389126,"nick":"","uin":1907104721},{"face":0,"flag":518,"nick":"","uin":1745701153},{"face":0,"flag":526,"nick":"","uin":445284794},{"face":717,"flag":8388614,"nick":"QQ","uin":4188952283},{"face":81,"flag":8389186,"nick":"Kernel","uin":276408653},{"face":0,"flag":2147484166,"nick":"Q","uin":1526867107}]}}
     * 
     */
    printf ("\n========================================\n");
    ret = json_parse_document(&json, req->response);
    if (ret != JSON_OK) {
        lwqq_log(LOG_ERROR, "Parse json object of friends error\n");
        *err = LWQQ_ERROR;
        goto done;
    }
    
    /**
     * Frist, we parse retcode that indicate whether we get
     * correct response from server
     */
    value = json_parse_simple_value(json, "retcode");
    if (!value || strcmp(value, "0")) {
        lwqq_log(LOG_ERROR, "Parse json object error\n");
        goto json_error;
    }

    /**
     * Second, Check whether there is a "result" key in json object
     */
    json_tmp = json_find_first_label_all(json, "result");
    if (!json_tmp) {
        lwqq_log(LOG_ERROR, "Parse json object error\n");
        goto json_error;
    }

    /* Third, it seems everyone is right now, we parse categories now */
    if (json_tmp && json_tmp->child && json_tmp->child->child ) {
        /* Make json_tmp point category reference */
        json_tmp = json_tmp->child->child;
        while (json_tmp->next) {
            if (!strcmp("categories", json_tmp->text)) {
                break;
            }
            json_tmp = json_tmp->next;
        }
        if (json_tmp) {
            json_tmp = json_tmp->child;    //point to the array.[]
            json_t *cur;
            char *index, *sort, *name;
            for (cur = json_tmp->child; cur != NULL; cur = cur->next) {
                index = json_parse_simple_value(cur, "index");
                sort = json_parse_simple_value(cur, "sort");
                name = json_parse_simple_value(cur, "name");
                if (index && sort && name)
                    printf ("====%s, %s, %s\n", index, sort, name);
            }
            //add the default category
            /* cate = qq_category_new(); */
            /* cate -> name = g_string_new("My Friends"); */
            /* cate -> index = 0; */
            /* g_ptr_array_add(info -> categories, (gpointer)cate); */
        }
    }
        
done:
    if (json)
        json_free_value(&json);
    lwqq_http_request_free(req);
    return ;

json_error:
    *err = LWQQ_ERROR;
    /* Free temporary string */
    if (json)
        json_free_value(&json);
    lwqq_http_request_free(req);
}
