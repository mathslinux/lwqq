/**
 * @file   type.c
 * @author mathslinux <riegamaths@gmail.com>
 * @date   Sun May 20 23:01:57 2012
 * 
 * @brief  Linux WebQQ Data Struct API
 * 
 * 
 */

#include <string.h>
#include "type.h"
#include "smemory.h"
#include "logger.h"

/** 
 * Create a new lwqq client
 * 
 * @param username QQ username
 * @param password QQ password
 * 
 * @return A new LwqqClient instance, or NULL failed
 */
LwqqClient *lwqq_client_new(const char *username, const char *password)
{
    if (!username || !password) {
        lwqq_log(LOG_ERROR, "Username or password is null\n");
        return NULL;
    }

    LwqqClient *lc = s_malloc0(sizeof(*lc));
    lc->username = s_strdup(username);
    lc->password = s_strdup(password);
    lc->myself = lwqq_buddy_new();
    if (!lc->myself) {
        goto failed;
    }
    lc->myself->qqnumber = s_strdup(username);
    lc->myself->uin = s_strdup(username);

    lc->cookies = s_malloc0(sizeof(*(lc->cookies)));
    return lc;
    
failed:
    lwqq_client_free(lc);
    return NULL;
}

/** 
 * Get cookies needby by webqq server
 * 
 * @param lc 
 * 
 * @return Cookies string on success, or null on failure
 */
char *lwqq_get_cookies(LwqqClient *lc)
{
    if (lc->cookies && lc->cookies->lwcookies) {
        return s_strdup(lc->cookies->lwcookies);
    }

    return NULL;
}

static void vc_free(LwqqVerifyCode *vc)
{
    if (vc) {
        s_free(vc->str);
        s_free(vc->type);
        s_free(vc->img);
        s_free(vc->uin);
        s_free(vc);
    }
}

static void cookies_free(LwqqCookies *c)
{
    if (c) {
        s_free(c->ptvfsession);
        s_free(c->ptcz);
        s_free(c->skey);
        s_free(c->ptwebqq);
        s_free(c->ptuserinfo);
        s_free(c->uin);
        s_free(c->ptisp);
        s_free(c->pt2gguin);
        s_free(c->verifysession);
        s_free(c->lwcookies);
        s_free(c);
    }
}

static void lwqq_categories_free(LwqqFriendCategory *cate)
{
    if (!cate)
        return ;

    s_free(cate->name);
    s_free(cate);
}

/** 
 * Free LwqqClient instance
 * 
 * @param client LwqqClient instance
 */
void lwqq_client_free(LwqqClient *client)
{
    LwqqBuddy *b_entry, *b_next;
    LwqqFriendCategory *c_entry, *c_next;
    LwqqGroup *g_entry, *g_next;

    if (!client)
        return ;

    /* Free LwqqVerifyCode instance */
    s_free(client->username);
    s_free(client->password);
    s_free(client->version);
    vc_free(client->vc);
    cookies_free(client->cookies);
    s_free(client->clientid);
    s_free(client->seskey);
    s_free(client->cip);
    s_free(client->index);
    s_free(client->port);
    s_free(client->status);
    s_free(client->vfwebqq);
    s_free(client->psessionid);
    lwqq_buddy_free(client->myself);
        
    /* Free friends list */
    LIST_FOREACH_SAFE(b_entry, &client->friends, entries, b_next) {
        LIST_REMOVE(b_entry, entries);
        lwqq_buddy_free(b_entry);
    }

    /* Free categories list */
    LIST_FOREACH_SAFE(c_entry, &client->categories, entries, c_next) {
        LIST_REMOVE(c_entry, entries);
        lwqq_categories_free(c_entry);
    }

    
    /* Free groups list */
    LIST_FOREACH_SAFE(g_entry, &client->groups, entries, g_next) {
        LIST_REMOVE(g_entry, entries);
        lwqq_group_free(g_entry);
    }
        
    s_free(client);
}

/************************************************************************/
/* LwqqBuddy API */

/** 
 * 
 * Create a new buddy
 * 
 * @return A LwqqBuddy instance
 */
LwqqBuddy *lwqq_buddy_new()
{
    LwqqBuddy *b = s_malloc0(sizeof(*b));
    return b;
}

/** 
 * Free a LwqqBuddy instance
 * 
 * @param buddy 
 */
void lwqq_buddy_free(LwqqBuddy *buddy)
{
    if (!buddy)
        return ;

    s_free(buddy->uin);
    s_free(buddy->qqnumber);
    s_free(buddy->face);
    s_free(buddy->occupation);
    s_free(buddy->phone);
    s_free(buddy->allow);
    s_free(buddy->college);
    s_free(buddy->reg_time);
    s_free(buddy->constel);
    s_free(buddy->blood);
    s_free(buddy->homepage);
    s_free(buddy->stat);
    s_free(buddy->country);
    s_free(buddy->city);
    s_free(buddy->personal);
    s_free(buddy->nick);
    s_free(buddy->shengxiao);
    s_free(buddy->email);
    s_free(buddy->province);
    s_free(buddy->gender);
    s_free(buddy->mobile);
    s_free(buddy->vip_info);
    s_free(buddy->markname);
    s_free(buddy->flag);
    s_free(buddy->client_type);
    
    s_free(buddy);
}

/** 
 * Find buddy object by buddy's uin member
 * 
 * @param lc Our Lwqq client object
 * @param uin The uin of buddy which we want to find
 * 
 * @return 
 */
LwqqBuddy *lwqq_buddy_find_buddy_by_uin(LwqqClient *lc, const char *uin)
{
    LwqqBuddy *buddy;
    
    if (!lc || !uin)
        return NULL;

    LIST_FOREACH(buddy, &lc->friends, entries) {
        if (buddy->uin && (strcmp(buddy->uin, uin) == 0))
            return buddy;
    }

    return NULL;
}

/* LwqqBuddy API END*/
/************************************************************************/

/** 
 * Create a new group
 * 
 * @return A LwqqGroup instance
 */
LwqqGroup *lwqq_group_new()
{
    LwqqGroup *g = s_malloc0(sizeof(*g));
    return g;
}

/** 
 * 
 * Create a new group member
 * 
 * @return A LwqqGroupMember instance
 */
LwqqGroup *lwqq_group_member_new()
{
    LwqqGroup *m = s_malloc0(sizeof(*m));
    return m;
}

/** 
 * Free a LwqqGroup instance
 * 
 * @param group
 */
void lwqq_group_free(LwqqGroup *group)
{
    LwqqGroupMember *m_entry, *m_next;

    if (!group)
        return ;

    s_free(group->name);
    s_free(group->gid);
    s_free(group->code);
    s_free(group->account);
    s_free(group->markname);
    s_free(group->face);
    s_free(group->memo);
    s_free(group->class);
    s_free(group->fingermemo);
    s_free(group->createtime);
    s_free(group->level);
    s_free(group->owner);
    s_free(group->flag);
    s_free(group->option);

    /* Free LwGroupMember list */
    LIST_FOREACH_SAFE(m_entry, &group->members, entries, m_next) {
        LIST_REMOVE(m_entry, entries);
        lwqq_group_member_free(m_entry);
    }	

    s_free(group);
}

/** 
 * Free a LwqqGroupMember instance
 * 
 * @param member
 */
void lwqq_group_member_free(LwqqGroupMember *member)
{
    if (!member)
        return ;

    s_free(member->uin);
    s_free(member->client_type);
    s_free(member->stat);
    s_free(member->nick);
    s_free(member->country);
    s_free(member->province);
    s_free(member->city);
    s_free(member->gender);
    s_free(member->vip_level);
       
    s_free(member);
}

/** 
 * Find group object by group's gid member
 * 
 * @param lc Our Lwqq client object
 * @param uin The gid of group which we want to find
 * 
 * @return A LwqqGroup instance 
 */
LwqqGroup *lwqq_group_find_group_by_gid(LwqqClient *lc, const char *gid)
{
    LwqqGroup *group;
    
    if (!lc || !gid)
        return NULL;

    LIST_FOREACH(group, &lc->groups, entries) {
        if (group->gid && (strcmp(group->gid, gid) == 0))
            return group;
    }

    return NULL;
}
