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
#include <stdlib.h>
#include "info.h"
#include "url.h"
#include "logger.h"
#include "http.h"
#include "smemory.h"
#include "json.h"

/** 
 * Parse friend category information
 * 
 * @param lc 
 * @param json Point to the first child of "result"'s value
 */
static void parse_categories_child(LwqqClient *lc, json_t *json)
{
    LwqqFriendCategory *cate;
    json_t *cur;
    char *index, *sort, *name;
    
    /* Make json point category reference */
    while (json) {
        if (json->text && !strcmp(json->text, "categories")) {
            break;
        }
        json = json->next;
    }
    if (!json) {
        return ;
    }
    
    json = json->child;    //point to the array.[]
    for (cur = json->child; cur != NULL; cur = cur->next) {
        index = json_parse_simple_value(cur, "index");
        sort = json_parse_simple_value(cur, "sort");
        name = json_parse_simple_value(cur, "name");
        cate = s_malloc0(sizeof(*cate));
        if (index) {
            cate->index = atoi(index);
        }
        if (sort) {
            cate->sort = atoi(sort);
        }
        if (name) {
            cate->name = s_strdup(name);
        }

        /* Add to categories list */
        LIST_INSERT_HEAD(&lc->categories, cate, entries);
    }

    /* add the default category */
    cate = s_malloc0(sizeof(*cate));
    cate->index = 0;
    cate->index = 1;
    cate->name = s_strdup("My Friends");
    LIST_INSERT_HEAD(&lc->categories, cate, entries);
}

/** 
 * Parse info child
 * 
 * @param lc 
 * @param json Point to the first child of "result"'s value
 */
static void parse_info_child(LwqqClient *lc, json_t *json)
{
    LwqqBuddy *buddy;
    json_t *cur;
    
    /* Make json point "info" reference */
    while (json) {
        if (json->text && !strcmp(json->text, "info")) {
            break;
        }
        json = json->next;
    }
    if (!json) {
        return ;
    }
    
    json = json->child;    //point to the array.[]
    for (cur = json->child; cur != NULL; cur = cur->next) {
        buddy = s_malloc0(sizeof(*buddy));
        buddy->face = s_strdup(json_parse_simple_value(cur, "face"));
        buddy->flag = s_strdup(json_parse_simple_value(cur, "flag"));
        buddy->nick = s_strdup(json_parse_simple_value(cur, "nick"));
        buddy->uin = s_strdup(json_parse_simple_value(cur, "uin"));
                                
        /* Add to categories list */
        LIST_INSERT_HEAD(&lc->friends, buddy, entries);
    }
}

/** 
 * Parse marknames child
 * 
 * @param lc 
 * @param json Point to the first child of "result"'s value
 */
static void parse_marknames_child(LwqqClient *lc, json_t *json)
{
    LwqqBuddy *buddy;
    char *uin, *markname;
    json_t *cur;
    
    /* Make json point "info" reference */
    while (json) {
        if (json->text && !strcmp(json->text, "marknames")) {
            break;
        }
        json = json->next;
    }
    if (!json) {
        return ;
    }

    json = json->child;    //point to the array.[]
    for (cur = json->child; cur != NULL; cur = cur->next) {
        uin = json_parse_simple_value(cur, "uin");
        markname = json_parse_simple_value(cur, "markname");
        if (!uin || !markname)
            continue;
        
        buddy = lwqq_buddy_find_buddy_by_uin(lc, uin);
        if (!buddy)
            continue;

        /* Free old markname */
        if (buddy->markname) 
            s_free(buddy->markname);
        buddy->markname = s_strdup(markname);
    }
}

/** 
 * Parse friends child
 * 
 * @param lc 
 * @param json Point to the first child of "result"'s value
 */
static void parse_friends_child(LwqqClient *lc, json_t *json)
{
    LwqqBuddy *buddy;
    char *uin, *cate_index;
    json_t *cur;
    
    /* Make json point "info" reference */
    while (json) {
        if (json->text && !strcmp(json->text, "friends")) {
            break;
        }
        json = json->next;
    }
    if (!json) {
        return ;
    }

//    {"flag":0,"uin":1907104721,"categories":0}
    json = json->child;    //point to the array.[]
    for (cur = json->child; cur != NULL; cur = cur->next) {
        uin = json_parse_simple_value(cur, "uin");
        cate_index = json_parse_simple_value(cur, "categories");
        if (!uin || !cate_index)
            continue;
        
        buddy = lwqq_buddy_find_buddy_by_uin(lc, uin);
        if (!buddy)
            continue;

        buddy->cate_index = atoi(cate_index);
    }
}

/** 
 * Get QQ friends information. These information include basic friend
 * information, friends group information, and so on
 * 
 * @param lc 
 * @param err 
 */
void lwqq_info_get_friends_info(LwqqClient *lc, LwqqErrorCode *err)
{
    char msg[256] ={0};
    char *buf;
    LwqqHttpRequest *req = NULL;  
    int ret;
    json_t *json = NULL, *json_tmp;
    char *value = NULL;

    if (!err) {
        *err = LWQQ_EC_ERROR;
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
        *err = LWQQ_EC_NETWORK_ERROR;
        goto done;
    }
    if (req->http_code != 200) {
        *err = LWQQ_EC_HTTP_ERROR;
        goto done;
    }

    /**
     * Here, we got a json object like this:
     * {"retcode":0,"result":{"friends":[{"flag":0,"uin":1907104721,"categories":0},{"flag":0,"uin":1745701153,"categories":0},{"flag":0,"uin":445284794,"categories":0},{"flag":0,"uin":4188952283,"categories":0},{"flag":0,"uin":276408653,"categories":0},{"flag":0,"uin":1526867107,"categories":0}],"marknames":[{"uin":276408653,"markname":""}],"categories":[{"index":1,"sort":1,"name":""},{"index":2,"sort":2,"name":""},{"index":3,"sort":3,"name":""}],"vipinfo":[{"vip_level":1,"u":1907104721,"is_vip":1},{"vip_level":1,"u":1745701153,"is_vip":1},{"vip_level":1,"u":445284794,"is_vip":1},{"vip_level":6,"u":4188952283,"is_vip":1},{"vip_level":0,"u":276408653,"is_vip":0},{"vip_level":1,"u":1526867107,"is_vip":1}],"info":[{"face":294,"flag":8389126,"nick":"","uin":1907104721},{"face":0,"flag":518,"nick":"","uin":1745701153},{"face":0,"flag":526,"nick":"","uin":445284794},{"face":717,"flag":8388614,"nick":"QQ","uin":4188952283},{"face":81,"flag":8389186,"nick":"Kernel","uin":276408653},{"face":0,"flag":2147484166,"nick":"Q","uin":1526867107}]}}
     * 
     */
    ret = json_parse_document(&json, req->response);
    if (ret != JSON_OK) {
        lwqq_log(LOG_ERROR, "Parse json object of friends error: %s\n", req->response);
        *err = LWQQ_EC_ERROR;
        goto done;
    }
    
    /**
     * Frist, we parse retcode that indicate whether we get
     * correct response from server
     */
    value = json_parse_simple_value(json, "retcode");
    if (!value || strcmp(value, "0")) {
        lwqq_log(LOG_ERROR, "Parse json object error: %s\n", req->response);
        goto json_error;
    }

    /**
     * Second, Check whether there is a "result" key in json object
     */
    json_tmp = json_find_first_label_all(json, "result");
    if (!json_tmp) {
        lwqq_log(LOG_ERROR, "Parse json object error: %s\n", req->response);
        goto json_error;
    }

    /** Third, it seems everyone is right now, we start parsing information
     * now
     */
    if (json_tmp && json_tmp->child && json_tmp->child->child ) {
        json_tmp = json_tmp->child->child;

        /* Parse friend category information */
        parse_categories_child(lc, json_tmp);

        /**
         * Parse friends information.
         * Firse, we parse object's "info" child
         * Then, parse object's "marknames" child
         * Last, parse object's "friends" child
         */
        parse_info_child(lc, json_tmp);
        parse_marknames_child(lc, json_tmp);
        parse_friends_child(lc, json_tmp);
    }
        
done:
    if (json)
        json_free_value(&json);
    lwqq_http_request_free(req);
    return ;

json_error:
    *err = LWQQ_EC_ERROR;
    /* Free temporary string */
    if (json)
        json_free_value(&json);
    lwqq_http_request_free(req);
}
