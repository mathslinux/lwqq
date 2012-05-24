/**
 * @file   type.c
 * @author mathslinux <riegamaths@gmail.com>
 * @date   Sun May 20 23:01:57 2012
 * 
 * @brief  Linux WebQQ Data Struct API
 * 
 * 
 */

#include "type.h"
#include "smemory.h"

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
    if (!username || !password)
        return NULL;

    LwqqClient *lc = s_malloc0(sizeof(*lc));
    lc->username = s_strdup(username);
    lc->password = s_strdup(password);
    lc->myself = lwqq_buddy_new();
    if (!lc->myself) {
        goto failed;
    }

    return lc;
    
failed:
    lwqq_client_free(lc);
    return NULL;
}

static void vc_free(LwqqVerifyCode *vc)
{
    if (vc) {
        s_free(vc->str);
        s_free(vc->type);
        s_free(vc->img);
        s_free(vc);
    }
}

/** 
 * Free LwqqClient instance
 * 
 * @param client LwqqClient instance
 */
void lwqq_client_free(LwqqClient *client)
{
    if (!client)
        return ;

    /* Free LwqqVerifyCode instance */
    s_free(client->username);
    s_free(client->password);
    s_free(client->version);
    vc_free(client->vc);
    s_free(client->ptvfsession);
    s_free(client->ptcz);
    s_free(client->skey);
    s_free(client->ptwebqq);
    s_free(client->ptuserinfo);
    s_free(client->uin);
    s_free(client->ptisp);
    s_free(client->pt2gguin);
    s_free(client->clientid);
    lwqq_buddy_free(client->myself);
        
    /* Free friends list */
    LwqqBuddy *b_entry, *next;
    LIST_FOREACH_SAFE(b_entry, &client->friends, entries, next) {
        LIST_REMOVE(b_entry, entries);
        s_free(b_entry);
    }
        
    s_free(client);
}

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
    s_free(buddy);
}
