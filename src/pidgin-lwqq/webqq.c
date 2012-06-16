/***************************************************************************
 *   Copyright (C) 2010 by lwp                                             *
 *   levin108@gmail.com                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Suite 500, Boston, MA 02110-1335, USA.            *
 *                                                                         *
 *   OpenSSL linking exception                                             *
 *   --------------------------                                            *
 *   If you modify this Program, or any covered work, by linking or        *
 *   combining it with the OpenSSL project's "OpenSSL" library (or a       *
 *   modified version of that library), containing parts covered by        *
 *   the terms of OpenSSL/SSLeay license, the licensors of this            *
 *   Program grant you additional permission to convey the resulting       *
 *   work. Corresponding Source for a non-source form of such a            *
 *   combination shall include the source code for the parts of the        *
 *   OpenSSL library used as well as that of the covered work.             *
 ***************************************************************************/

#define PURPLE_PLUGINS

#include <glib.h>

#include "webqq.h"
#include "notify.h"
#include "plugin.h"
#include "version.h"
#include "accountopt.h"

#include "fx_sip.h"
#include "fx_user.h"
#include "fx_login.h"
#include "fx_buddy.h"
#include "fx_contact.h"
#include "fx_chat.h"
#include "fx_blist.h"

#ifdef ENABLE_NLS
#       ifdef _WIN32
#               include <win32dep.h>
#       endif
#       include <glib/gi18n.h>
#	include <locale.h>
#else
//#define _(a) a
#endif

PurplePlugin  *openfetion_plugin = NULL;
GSList        *sessions;
GSList        *buddy_to_added;

static void process_push_cb(fetion_account *ac, const gchar *sipmsg);
static void process_notify_cb(fetion_account *ac, const gchar *sipmsg);

static const char *fx_list_icon(PurpleAccount *UNUSED(a), PurpleBuddy *UNUSED(b))
{
	return "openfetion";
}


static void fx_keep_alive(PurpleConnection *gc)
{
/*	fetion_account *ses;
	GSList *list = sessions;
	fetion_account *ac = purple_connection_get_protocol_data(gc); 
	fetion_user_keep_alive(ac);
	while(list) {
		ses = (fetion_account*)(list->data);
		if(ses->sk != 0)
			fetion_user_keep_alive(ses);
		list = list->next;
	}	*/
}

static void fx_tooltip_text(PurpleBuddy *buddy, PurpleNotifyUserInfo *user_info, gboolean UNUSED(full))
{
	PurpleStatus *status;
	const gchar *impresa, *alias, *sid, *mobileno;

	g_return_if_fail(buddy != NULL);

	status = purple_presence_get_active_status(purple_buddy_get_presence(buddy));
	impresa = purple_status_get_attr_string(status, "impresa");
	sid = purple_status_get_attr_string(status, "fetionno");
	mobileno = purple_status_get_attr_string(status, "mobileno");
	alias = purple_buddy_get_alias(buddy);

	purple_notify_user_info_add_pair(user_info, _("FetionNo"), sid);
	purple_notify_user_info_add_pair(user_info, _("MobileNo"), mobileno);
	purple_notify_user_info_add_pair(user_info, _("Alias"), alias);
	purple_notify_user_info_add_pair(user_info, _("Signature"), impresa);

}

static void fx_alias_buddy(PurpleConnection *gc, const gchar *who, const gchar *alias)
{
	fetion_account *ac = purple_connection_get_protocol_data(gc);
	fetion_contact_set_displayname(ac, who, alias);
}

static void fx_remove_buddy(PurpleConnection *gc, PurpleBuddy *buddy, PurpleGroup *UNUSED(group))
{
	fetion_account *ac = purple_connection_get_protocol_data(gc);
	const gchar *userid = purple_buddy_get_name(buddy);
	fetion_contact_delete_buddy(ac, userid);
}

static void fx_rename_group(PurpleConnection *gc,
	   	const gchar *old_name, PurpleGroup *group, GList *UNUSED(moved_buddies))
{

	fetion_account *ac = purple_connection_get_protocol_data(gc);
	Group *blist = fetion_group_list_find_by_name(ac->user->groupList, old_name);
	const gchar *name = purple_group_get_name(group);
	fetion_buddylist_edit(ac, blist->groupid, name);

}

static void fx_group_buddy(PurpleConnection *gc, const gchar *who,
		    const gchar *UNUSED(old_group), const gchar *new_group)
{
	gchar buf[BUFLEN];
	fetion_account *ac = purple_connection_get_protocol_data(gc);
	Contact *cnt = fetion_contact_list_find_by_userid(ac->user->contactList, who);
	Group   *group = fetion_group_list_find_by_name(ac->user->groupList, new_group);
	if(!group) {
		snprintf(buf, sizeof(buf) - 1, _("'%s' is not a valid group of this account."), new_group);
		purple_notify_error(ac->gc, NULL, _("Failed"), buf);
		return;
	}
	fetion_contact_move_to_group(ac, cnt->userId, group->groupid);
}

static void fx_remove_group(PurpleConnection *gc, PurpleGroup *group)
{
	const gchar *name = purple_group_get_name(group);
	fetion_account *ac = purple_connection_get_protocol_data(gc);
	Group *blist = fetion_group_list_find_by_name(ac->user->groupList, name);
	if(!blist) return;
	fetion_buddylist_delete(ac, blist->groupid);
}

static GList *fx_attention_types(PurpleAccount *UNUSED(account))
{
	PurpleAttentionType *attn;
	static GList *list = NULL;

	if (!list) {
		attn = g_new0(PurpleAttentionType, 1);
		attn->name = _("Nudge");
		attn->incoming_description = _("%s has nudged you!");
		attn->outgoing_description = _("Nudging %s...");
		list = g_list_append(list, attn);
	}

	return list;
}

static gboolean fx_send_attention(PurpleConnection *gc, const gchar *who, guint UNUSED(type))
{
	PurpleBuddy    *buddy;
	PurplePresence *presence;
	PurpleStatus   *status;
	const gchar    *status_id;
	fetion_account  *sec;
	fetion_account  *ac = purple_connection_get_protocol_data(gc);

	if(!(buddy = purple_find_buddy(ac->account, who))) return 0;
	presence = purple_buddy_get_presence(buddy);
	status   = purple_presence_get_active_status(presence);
	status_id = purple_status_get_id(status);

	/* online,need invite */
	if(strcmp(status_id, "Offline") != 0) {
		if(!(sec = session_find(who)))
			new_chat(ac, who, (gchar*)0);
		else 
			fetion_send_nudge(sec, who);
		return TRUE;
	}

	return FALSE;
}

static int fx_im_send(PurpleConnection *gc, const gchar *who, const gchar *what, PurpleMessageFlags UNUSED(flags))
{
	PurpleBuddy    *buddy;
	PurplePresence *presence;
	PurpleStatus   *status;
	const gchar    *status_id;
	fetion_account  *sec;
	PurpleConversation *conv;
	fetion_account  *ac = purple_connection_get_protocol_data(gc);

	Contact        *cnt;
	gint            shutdown = 0;

	if(!(buddy = purple_find_buddy(ac->account, who))) return 0;
	presence = purple_buddy_get_presence(buddy);
	status   = purple_presence_get_active_status(presence);
	status_id = purple_status_get_id(status);

	/*cnt = fetion_contact_list_find_by_userid(ac->user->contactList, who);
	if(cnt->relationStatus == RELATION_STATUS_UNAUTHENTICATED) {
		if(!(conv = purple_find_conversation_with_account(
						PURPLE_CONV_TYPE_ANY, who, ac->account))) return -1;
			purple_conversation_write(conv, NULL,
					  _("Failed to send message: Unverified Buddy!"),
					  PURPLE_MESSAGE_ERROR, time(NULL));
			return -1;
	}
	
	if(cnt->serviceStatus == BASIC_SERVICE_ABNORMAL){
		if(cnt->carrierStatus == CARRIER_STATUS_CLOSED){
			shutdown = 1;
		}else{
			if((cnt->carrier[0] != '\0' && cnt->mobileno[0] == '\0') || cnt->carrier[0] == '\0')
				shutdown = 1;
		}
	}else if(cnt->carrierStatus == CARRIER_STATUS_DOWN)
		if(cnt->carrier[0] != '\0') shutdown = 1;

	if(shutdown) {
		if(!(conv = purple_find_conversation_with_account(
						PURPLE_CONV_TYPE_ANY, who, ac->account))) return -1;
			purple_conversation_write(conv, NULL,
					  _("Fail to send message: Buddy has cancled Fetion service!"),
					  PURPLE_MESSAGE_ERROR, time(NULL));
			return -1;
	}

	// online,need invite 
	if(strcmp(status_id, "Offline") != 0) {
		if(!(sec = session_find(who))) new_chat(ac, who, what);
		else  fetion_send_sms(sec, who, what);
		return 1;
	}*/

	fetion_send_sms(ac, who, what);
	return 1;
}
/*static int fx_im_send(PurpleConnection *gc, const gchar *who, const gchar *what, PurpleMessageFlags UNUSED(flags))
{
	PurpleBuddy    *buddy;
	PurplePresence *presence;
	PurpleStatus   *status;
	const gchar    *status_id;
	fetion_account  *sec;
	PurpleConversation *conv;
	fetion_account  *ac = purple_connection_get_protocol_data(gc);

	Contact        *cnt;
	gint            shutdown = 0;

	if(!(buddy = purple_find_buddy(ac->account, who))) return 0;
	presence = purple_buddy_get_presence(buddy);
	status   = purple_presence_get_active_status(presence);
	status_id = purple_status_get_id(status);

	cnt = fetion_contact_list_find_by_userid(ac->user->contactList, who);
	if(cnt->relationStatus == RELATION_STATUS_UNAUTHENTICATED) {
		if(!(conv = purple_find_conversation_with_account(
						PURPLE_CONV_TYPE_ANY, who, ac->account))) return -1;
			purple_conversation_write(conv, NULL,
					  _("Failed to send message: Unverified Buddy!"),
					  PURPLE_MESSAGE_ERROR, time(NULL));
			return -1;
	}
	
	if(cnt->serviceStatus == BASIC_SERVICE_ABNORMAL){
		if(cnt->carrierStatus == CARRIER_STATUS_CLOSED){
			shutdown = 1;
		}else{
			if((cnt->carrier[0] != '\0' && cnt->mobileno[0] == '\0') || cnt->carrier[0] == '\0')
				shutdown = 1;
		}
	}else if(cnt->carrierStatus == CARRIER_STATUS_DOWN)
		if(cnt->carrier[0] != '\0') shutdown = 1;

	if(shutdown) {
		if(!(conv = purple_find_conversation_with_account(
						PURPLE_CONV_TYPE_ANY, who, ac->account))) return -1;
			purple_conversation_write(conv, NULL,
					  _("Fail to send message: Buddy has cancled Fetion service!"),
					  PURPLE_MESSAGE_ERROR, time(NULL));
			return -1;
	}

	/ * online,need invite * /
	if(strcmp(status_id, "Offline") != 0) {
		if(!(sec = session_find(who))) new_chat(ac, who, what);
		else  fetion_send_sms(sec, who, what);
		return 1;
	}

	fetion_send_sms(ac, who, what);
	return 1;
}*/

static void fx_get_info(PurpleConnection *gc, const char *who)
{
	fetion_account *ses = purple_connection_get_protocol_data(gc);
	fetion_contact_get_contact_info(ses, who, (TransCallback)get_info_cb);
}

static void fx_set_status(PurpleAccount *account, PurpleStatus *status)
{
	const gchar *status_id;
	gint  state;
	PurpleConnection *gc = purple_account_get_connection(account);
	fetion_account *ses = purple_connection_get_protocol_data(gc);

	status_id = purple_status_get_id(status);

	if (!strcmp(status_id, "Online"))   	state = P_ONLINE;
	else if (!strcmp(status_id, "Away"))	state = P_AWAY;
	else if (!strcmp(status_id, "Busy"))	state = P_BUSY;
	else if (!strcmp(status_id, "Hidden"))	state = P_HIDDEN;
	else if (!strcmp(status_id, "Offline"))	state = P_OFFLINE;
	else                                    state = 400;

	fetion_user_set_state(ses, state);
}

static gchar *fx_status_text(PurpleBuddy *buddy)
{
	PurplePresence *presence;
	PurpleStatus   *status;
	const gchar    *msg;

	presence = purple_buddy_get_presence(buddy);
	status   = purple_presence_get_active_status(presence);
	msg = purple_status_get_attr_string(status, "impresa");
	if(msg && *msg)	return g_markup_escape_text(msg, -1);
	return NULL;
}

static void send_sms_cb(PurpleBuddy *buddy, const gchar *text)
{
	PurpleConnection *gc;
	fetion_account    *ses;
	const gchar *userid = purple_buddy_get_name(buddy);

	gc = purple_account_get_connection(buddy->account);
	ses = purple_connection_get_protocol_data(gc);

	fetion_send_sms_to_phone(ses, userid, text);
}

static void fx_send_sms(PurpleBlistNode *node, gpointer UNUSED(data))
{
	PurpleBuddy *buddy;
	PurpleConnection *gc;

	g_return_if_fail(PURPLE_BLIST_NODE_IS_BUDDY(node));

	buddy = (PurpleBuddy *) node;
	gc = purple_account_get_connection(buddy->account);

	purple_request_input(gc, NULL, _("Send a mobile message."), NULL,
			     NULL, TRUE, FALSE, NULL,
			     _("Send"), G_CALLBACK(send_sms_cb),
			     _("Cancel"), NULL,
			     purple_connection_get_account(gc),
			     purple_buddy_get_name(buddy), NULL, buddy);
}

static void send_sms_to_me_cb(PurpleConnection *gc, const gchar *text)
{
	fetion_account    *ses;

	g_return_if_fail(NULL != gc && NULL != gc->proto_data);

	ses = purple_connection_get_protocol_data(gc);

	fetion_sms_myself(ses, text);
}

static GList *fx_blist_node_menu(PurpleBlistNode * node)
{
	GList *menu = NULL;
	PurpleMenuAction *act;
	PurpleBuddy      *buddy;

	if (PURPLE_BLIST_NODE_IS_BUDDY(node)) {

		buddy = (PurpleBuddy*)node; 
		g_return_val_if_fail(buddy != NULL, NULL);
		act = purple_menu_action_new(_("Send to Mobile"),
				     	PURPLE_CALLBACK(fx_send_sms),
				     	NULL, NULL);
		menu = g_list_append(menu, act);

		return menu;
	} else {
		return (GList*)0;
	}
}

gint push_cb(gpointer data, gint source, const gchar *UNUSED(error_message))
{
	gchar sipmsg[BUFLEN * 10], *h, *pos, *msg;
	gint  n, data_len;
	guint len;
	fetion_account *ses = (fetion_account*)data;

	if((n = recv(source, sipmsg, sizeof(sipmsg), 0)) == -1) return -1;
	sipmsg[n] = '\0';

	data_len = ses->data ? strlen(ses->data) : 0;
	ses->data = (gchar*)realloc(ses->data, data_len + n + 1);
	memcpy(ses->data + data_len, sipmsg, n + 1);
recheck:
	data_len = strlen(ses->data);
	if((pos = strstr(ses->data, "\r\n\r\n"))) {
		pos += 4;
		h = (gchar*)g_malloc0(data_len - strlen(pos) + 1);
		memcpy(h, ses->data, data_len - strlen(pos));
		h[data_len - strlen(pos)] = '\0';
		if(strstr(h, "L: ")) {
			len = fetion_sip_get_length(ses->data);
			if(len <= strlen(pos)) {
				msg = (gchar*)g_malloc0(strlen(h) + len + 1);
				memcpy(msg, ses->data, strlen(h) + len);
				msg[strlen(h) + len] = '\0';
				process_push_cb(ses, msg);

				memmove(ses->data, ses->data + strlen(msg), data_len - strlen(msg));
				ses->data = (gchar*)realloc(ses->data, data_len - strlen(msg) + 1);
				ses->data[data_len - strlen(msg)] = '\0';

				g_free(msg); msg = (gchar*)0;
				g_free(h); h = (gchar*)0;
				goto recheck;
			}
		} else {
			process_push_cb(ses, h);
			memmove(ses->data, ses->data + strlen(h), data_len - strlen(h));
			ses->data = (gchar*)realloc(ses->data, data_len - strlen(h) + 1);
			ses->data[data_len - strlen(h)] = '\0';

			g_free(h); h = (gchar*)0;
			goto recheck;
		}
		g_free(h);
	} 

	return 0;
}

static void process_sipc_cb(fetion_account *ses, const gchar *sipmsg)
{
	gchar  callid[16];
	gint   callid0;
	struct transaction *trans;
	GSList *trans_cur;

	fetion_sip_get_attr(sipmsg, "I", callid);
	callid0 = atoi(callid);

	trans_cur = ses->trans;

	while(trans_cur) {
		trans = (struct transaction*)(trans_cur->data);
		if(trans->callid == callid0) {
			if(trans->callback)
				(trans->callback)(ses, sipmsg, trans);
			transaction_remove(ses, trans);
			break;
		}
		trans_cur = g_slist_next(trans_cur);
	}

}

static void process_dereg_cb(fetion_account *ses, const gchar *UNUSED(sipmsg))
{
	PurpleConnection *pc;
	pc = purple_account_get_connection(ses->account);
	purple_connection_error_reason(pc,
			       PURPLE_CONNECTION_ERROR_AUTHENTICATION_FAILED,
				       _("Your account has logined elsewhere. You are forced to quit."));
}

static void process_info_cb(fetion_account *ses, const gchar *sipmsg)
{
	InfoType type;
	gchar sipuri[48], callid[48], seq[48], buf[BUFLEN], *sid;
	fetion_sip_parse_info(sipmsg, &type);
	if(type == INFO_NUDGE) {
		memset(callid, 0, sizeof(callid));
		memset(seq, 0, sizeof(seq));
		memset(sipuri, 0, sizeof(sipuri));

		snprintf(buf, sizeof(buf) -1, "SIP-C/4.0 200 OK\r\n"
					   "F: %s\r\n"
					   "I: %s \r\n"
					   "Q: %s\r\n\r\n",
					   sipuri , callid , seq);
		send(ses->sk, buf, strlen(buf), 0);
		sid = fetion_sip_get_sid_by_sipuri(sipuri);
		purple_prpl_got_attention(ses->gc, sid, FETION_NUDGE);
	}
}

static void process_push_cb(fetion_account *ses, const gchar *sipmsg)
{
	gint type;
	
	type = fetion_sip_get_type(sipmsg);
	switch(type){
		case SIP_NOTIFICATION :	process_notify_cb(ses, sipmsg);		break;
		case SIP_MESSAGE:		process_message_cb(ses, sipmsg);	break;
		case SIP_INVITATION:	process_invite_cb(ses, sipmsg);		break;
		case SIP_INCOMING :		process_info_cb(ses, sipmsg);		break;
		case SIP_SIPC_4_0:		process_sipc_cb(ses, sipmsg);		break;
		default: break;
			//printf("%s\n" , pos->message);
	}
}

static void process_notify_cb(fetion_account *ac, const gchar *sipmsg)
{
	gint   event;
	gint   notification_type;
	gchar  *xml;

	fetion_sip_parse_notification(sipmsg , &notification_type , &event , &xml);

	switch(notification_type) {
		case NOTIFICATION_TYPE_PRESENCE:
			if(event == NOTIFICATION_EVENT_PRESENCECHANGED)	process_presence(ac , xml);
			break;
		case NOTIFICATION_TYPE_CONVERSATION :
			if(event == NOTIFICATION_EVENT_USERLEFT) {
				process_left_cb(ac, sipmsg);
				break;
			} else 	if(event == NOTIFICATION_EVENT_USERENTER) {
				process_enter_cb(ac, sipmsg);
				break;
			}
			break;
		case NOTIFICATION_TYPE_REGISTRATION :
			if(event == NOTIFICATION_EVENT_DEREGISTRATION)	process_dereg_cb(ac, sipmsg);
			break;
		case NOTIFICATION_TYPE_SYNCUSERINFO :
			if(event == NOTIFICATION_EVENT_SYNCUSERINFO)	process_sync_info(ac, sipmsg);
			break;
		case NOTIFICATION_TYPE_CONTACT :
			if(event == NOTIFICATION_EVENT_ADDBUDDYAPPLICATION)	process_add_buddy(ac, sipmsg);
			break;
#if 0
		case NOTIFICATION_TYPE_PGGROUP :
			break;
#endif
		default:
			break;
	}
	g_free(xml);
}

void check_info(LwqqClient* lc){
    /*LwqqRecvMsg *msg;
    while(1){
        usleep(1000);
        pthread_mutex_lock(&lc->msg_list->mutex);
        if (!SIMPLEQ_EMPTY(&lc->msg_list->head)) {
            msg = SIMPLEQ_FIRST(&lc->msg_list->head);
            if (msg->msg->content) {
                //\printf ("########################content: %s\n", msg->msg->content);
                purple_debug_info("account","content:%s\n",msg->msg->content);
            }
            SIMPLEQ_REMOVE_HEAD(&lc->msg_list->head, entries);
        }
        pthread_mutex_unlock(&lc->msg_list->mutex);
    }
*/
}
void clean_all_buddies(PurpleAccount *ac)
{
    GSList* purple_get_buddies();
}
static void fx_login(PurpleAccount *account)
{
	PurplePresence *presence;
	PurpleConnection *pc = purple_account_get_connection(account);
	const gchar *username = purple_account_get_username(account);
	const gchar *password = purple_connection_get_password(pc);
	const gchar *status_id;
    LwqqErrorCode err;
	fetion_account  *ac = session_new(account);

	/* construct a user object */
 	ac->user = fetion_user_new(username, password);
    username = "1421032531";
    password = "1234567890";
    ac->qq = lwqq_client_new(username,password);
	ac->account = account;
	ac->gc = pc;
	ac->chan_ready = 1;

	purple_connection_set_protocol_data(pc, ac);

	presence = purple_account_get_presence(account);

	purple_connection_update_progress(pc, "Connecting", 1, 2);
	/*purple_ssl_connect(ac->account, SSI_SERVER,
			PURPLE_SSL_DEFAULT_PORT, 
			(PurpleSslInputFunction)ssi_auth_action,
			(PurpleSslErrorFunction)0, ac);*/
    lwqq_login(ac->qq,&err);
    if(ac->qq->status="")ac->qq->status="Online";

	status_id = get_status_id(ac->qq->status);
	//if(ac->user->state == 0) status_id = "Hidden";
	purple_presence_set_status_active(presence, status_id, TRUE);
    if (err == LWQQ_EC_LOGIN_NEED_VC) {
        pic_read(ac);
        //lc->vc->str = get_vc();
        //printf ("get vc: %s\n", lc->vc->str);

        //lwqq_login(lc, &err);
    } else if (err != LWQQ_EC_OK) {
        //lwqq_log(LOG_ERROR, "Login error, exit\n");
        goto done;
    }
    //lwqq_log(LOG_NOTICE, "Login successfully\n");
    purple_connection_set_state(pc,PURPLE_CONNECTED);
    purple_debug_info("account","connected ok\n");

    fx_blist_init(ac);
    /*ac->qq->msg_list->poll_msg(ac->qq->msg_list);
    pthread_t th;
    pthread_init(&th);
    pthread_create(&th,NULL,check_info,ac->qq);
*/
    return;
done:
    lwqq_client_free(ac->qq);

}

static void fx_close(PurpleConnection *gc)
{
	fetion_account *ses;
	fetion_account *ac = purple_connection_get_protocol_data(gc);
    LwqqClient* qq = ac->qq;
    LwqqErrorCode err;
    lwqq_logout(qq,&err);
    lwqq_client_free(qq);
    ac->qq=NULL;
    purple_connection_set_protocol_data(gc,NULL);
	/*purple_input_remove(ac->conn);
	close(ac->sk);
	g_free(ac->data);
	ac->data = (gchar*)0;

	while(sessions) {
		ses = (fetion_account*)(sessions->data);
		session_remove(ses);
		session_destroy(ses);
	}
	fetion_user_free(ac->user);
	ac->user = NULL;
	purple_connection_set_protocol_data(gc, NULL);*/
}


static GList *fx_status_types(PurpleAccount *UNUSED(account))
{
	PurpleStatusType *status;
	GList *types = NULL;

	status = purple_status_type_new_with_attrs(PURPLE_STATUS_AVAILABLE,
					     "Online", _("Available"), FALSE, TRUE,FALSE,
						 "impresa", "impresa", purple_value_new(PURPLE_TYPE_STRING),
						 "fetionno", "fetionno", purple_value_new(PURPLE_TYPE_STRING), 
						 "mobileno", "mobileno", purple_value_new(PURPLE_TYPE_STRING), 
						NULL);
	types = g_list_append(types, status);

	status = purple_status_type_new_with_attrs(PURPLE_STATUS_AWAY,
					     "Away", _("Away"), FALSE, TRUE, FALSE,
						 "impresa", "impresa", purple_value_new(PURPLE_TYPE_STRING),
						 "fetionno", "fetionno", purple_value_new(PURPLE_TYPE_STRING), 
						 "mobileno", "mobileno", purple_value_new(PURPLE_TYPE_STRING), 
						  NULL);
	types = g_list_append(types, status);

	status = purple_status_type_new_with_attrs(PURPLE_STATUS_INVISIBLE,
					     "Hidden", _("Invisible"), FALSE, TRUE, FALSE,
						 "impresa", "impresa", purple_value_new(PURPLE_TYPE_STRING),
						 "fetionno", "fetionno", purple_value_new(PURPLE_TYPE_STRING), 
						 "mobileno", "mobileno", purple_value_new(PURPLE_TYPE_STRING), 
						  NULL);
	types = g_list_append(types, status);

	status = purple_status_type_new_with_attrs(PURPLE_STATUS_OFFLINE,
					     "Offline", _("Offline"), FALSE, TRUE, FALSE,
						 "impresa", "impresa", purple_value_new(PURPLE_TYPE_STRING),
						 "fetionno", "fetionno", purple_value_new(PURPLE_TYPE_STRING), 
						 "mobileno", "mobileno", purple_value_new(PURPLE_TYPE_STRING), 
						  NULL);
	types = g_list_append(types, status);

	status = purple_status_type_new_with_attrs(PURPLE_STATUS_UNAVAILABLE,
					     "Busy", _("Busy"), FALSE, TRUE, FALSE,
						 "impresa", "impresa", purple_value_new(PURPLE_TYPE_STRING),
						 "fetionno", "fetionno", purple_value_new(PURPLE_TYPE_STRING), 
						 "mobileno", "mobileno", purple_value_new(PURPLE_TYPE_STRING), 
						  NULL);
	types = g_list_append(types, status);

	return types;
}


static void action_about_openfetion(PurplePluginAction *action)
{
	PurpleConnection *gc = (PurpleConnection *) action->context;
	GString *info;
	gchar *title;

	g_return_if_fail(NULL != gc);

	info = g_string_new("<html><body>");
	g_string_append(info, _("<p><b>Author</b>:<br>\n"));
	g_string_append(info, "levin(<a href='http://twitter.com/levin108'>@levin108</a>)<br>\n");
	g_string_append(info, "<br/>\n");

	g_string_append(info, _("pidgin-openfetion is a Fetion protocol plugin for libpurple, <br/>"
				"implemented by the openfetion team.<br/>"
				"It supports most features of China Mobile's Fetion V4 protocol. <br/>"
				"It's lightweight and efficient.<br/><br/>"
				"Project homepage: http://code.google.com/p/ofetion/"));
	g_string_append(info, "<br/><br/>");
	g_string_append(info, _("<p><b>Translators</b></p>:<br/>\n"));
	/* Translators: HTML format, So do add a <br /> to every translator,
	 * such as Zhang <br/> \nWang <br/>
	 */
	g_string_append(info, _("translator-credits"));
	g_string_append(info, "\n</body></html>");

	title = g_strdup_printf(_("About OpenFetion %s"), DISPLAY_VERSION);
	purple_notify_formatted(gc, title, title, NULL, info->str, NULL, NULL);

	g_free(title);
	g_string_free(info, TRUE);
}


static void action_show_account_info(PurplePluginAction *action)
{
	PurpleConnection *gc = (PurpleConnection *) action->context;
	GString *info;
	gchar *province, *city;

	fetion_account *ac = purple_connection_get_protocol_data(gc);
	User *user = ac->user;

	info = g_string_new("<html><body>");

	province = get_province_name(user->province);
	city     = get_city_name(user->province, user->city);
	g_string_append_printf(info, _("<b>Last Login Time</b>: %s<br>\n"), user->lastLoginTime);
	g_string_append_printf(info, _("<b>Last Login IP</b>: %s<br>\n"), user->lastLoginIp);
	g_string_append_printf(info, _("<b>Public IP</b>: %s<br>\n"), user->publicIp);
	g_string_append(info, "<hr>");
	g_string_append_printf(info, _("<b>Fetion Number</b>: %s<br>\n"), user->sId);
	g_string_append_printf(info, _("<b>Mobile Number</b>: %s<br>\n"), user->mobileno);
	g_string_append_printf(info, _("<b>Login Time</b>: %s<br>\n"), user->lastLoginTime);
	g_string_append_printf(info, _("<b>NickName</b>: %s<br>\n"), user->nickname);
	g_string_append_printf(info, _("<b>Signature</b>: %s<br>\n"), user->impression);
	g_string_append_printf(info, _("<b>Province</b>: %s<br>\n"), province);
	g_string_append_printf(info, _("<b>City</b>: %s<br>\n"), city);
	g_string_append(info, "<hr>");
	g_string_append_printf(info, _("<b>SMS Sent Today</b>: %d<br>\n"), user->smsDayCount);
	g_string_append_printf(info, _("<b>SMS Sent This Month</b>: %d<br>\n"), user->smsMonthCount);
	g_string_append_printf(info, _("<b>SMS Limit Today</b>: %d<br>\n"), user->smsDayLimit);
	g_string_append_printf(info, _("<b>SMS Limit This Month</b>: %d<br>\n"), user->smsMonthLimit);


	g_string_append(info, "</body></html>");

	purple_notify_formatted(gc, NULL, _("Account Information"), NULL, info->str, NULL, NULL);

	g_string_free(info, TRUE);
	g_free(province); g_free(city);
}

static void modify_nickname_cb(PurpleConnection *gc, const gchar *text)
{
	fetion_account *ac;
	g_return_if_fail(NULL != gc && NULL != gc->proto_data);

	ac = purple_connection_get_protocol_data(gc);
	fetion_modify_info(ac, MODIFY_INFO_NICKNAME, text);
}

static void modify_impresa_cb(PurpleConnection *gc, const gchar *text)
{
	fetion_account *ac;
	g_return_if_fail(NULL != gc && NULL != gc->proto_data);

	ac = purple_connection_get_protocol_data(gc);
	fetion_modify_info(ac, MODIFY_INFO_IMPRESA, text);
}

static void action_modify_nickname(PurplePluginAction *action)
{
	PurpleConnection *gc = (PurpleConnection*)action->context;

	g_return_if_fail(NULL != gc && NULL != gc->proto_data);

	purple_request_input(gc, NULL, _("Change Nickname"), NULL,
			     NULL, FALSE, FALSE, NULL,
			     _("OK"), G_CALLBACK(modify_nickname_cb),
			     _("Cancel"), NULL,
			     purple_connection_get_account(gc),
			     NULL, NULL, gc);
}

static void action_modify_impresa(PurplePluginAction *action)
{
	PurpleConnection *gc = (PurpleConnection*)action->context;

	g_return_if_fail(NULL != gc && NULL != gc->proto_data);

	purple_request_input(gc, NULL, _("Change Signature"), NULL,
			     NULL, FALSE, FALSE, NULL,
			     _("OK"), G_CALLBACK(modify_impresa_cb),
			     _("Cancel"), NULL,
			     purple_connection_get_account(gc),
			     NULL, NULL, gc);
}

static void action_sms_myself(PurplePluginAction *action)
{
	PurpleConnection *gc = (PurpleConnection*)action->context;

	g_return_if_fail(NULL != gc && NULL != gc->proto_data);

	purple_request_input(gc, NULL, _("Send SMS to Your Phone"), NULL,
			     NULL, TRUE, FALSE, NULL,
			     _("Send"), G_CALLBACK(send_sms_to_me_cb),
			     _("Close"), NULL,
			     purple_connection_get_account(gc),
			     NULL, NULL, gc);
}

static GList *plugin_actions(PurplePlugin *UNUSED(plugin), gpointer UNUSED(context))
{

	GList *m;
	PurplePluginAction *act;

	m = NULL;
	act = purple_plugin_action_new(_("Send SMS To Phone"), action_sms_myself);
	m = g_list_append(m, act);

	m = g_list_append(m, NULL);

	act = purple_plugin_action_new(_("View Information"), action_show_account_info);
	m = g_list_append(m, act);

	act = purple_plugin_action_new(_("Change Nickname"), action_modify_nickname);
	m = g_list_append(m, act);

	act = purple_plugin_action_new(_("Change Signature"), action_modify_impresa);
	m = g_list_append(m, act);

	m = g_list_append(m, NULL);

	act = purple_plugin_action_new(_("About OpenFetion"), action_about_openfetion);
	m = g_list_append(m, act);

    return m;
}

static PurplePluginProtocolInfo protocol_info = {
	0,
	NULL,			/* user_splits */
	NULL,			/* protocol_options */
	{"png", 0, 0, 96, 96, 0, PURPLE_ICON_SCALE_SEND},	/* icon_spec */
	fx_list_icon,	/* list_icon */
	NULL,			/* list_emblems */
	fx_status_text,	/* status_text */
	fx_tooltip_text,			/* tooltip_text */
	fx_status_types,	/* away_states */
	fx_blist_node_menu,	/* blist_node_menu */
	NULL,			/* chat_info */
	NULL,			/* chat_info_defaults */
	fx_login,		/* login */
	fx_close,		/* close */
	fx_im_send,		/* send_im */
	NULL,			/* set_info */
	NULL,			// fetion_typing,                  /* send_typing */
	fx_get_info,	/* get_info */
	fx_set_status,	/* set_status */
	NULL,			/* set_idle */
	NULL,			/* change_passwd */
	fx_add_buddy,	/* add_buddy */
	NULL,			/* add_buddies */
	fx_remove_buddy,	/* remove_buddy */
	NULL,	/* remove_buddies */
	NULL,		/* add_permit */
	NULL,		/* add_deny */
	NULL,		/* rem_permit */
	NULL,		/* rem_deny */
	NULL,	/* set_permit_deny */
	NULL,			/* join_chat */
	NULL,			/* reject_chat */
	NULL,			/* get_chat_name */
	NULL,	/* chat_invite */
	NULL,	/* chat_leave */
	NULL,			/* chat_whisper */
	NULL,	/* chat_send */
	fx_keep_alive,	/* keepalive */
	NULL,			/* register_user */
	NULL,			/* get_cb_info */
	NULL,			/* get_cb_away */
	fx_alias_buddy,	/* alias_buddy */
	fx_group_buddy,	/* group_buddy */
	fx_rename_group,	/* rename_group */
	NULL,			/* buddy_free */
	NULL,			/* convo_closed */
	NULL,			/* normalize */
	NULL,	/* set_buddy_icon */
	fx_remove_group,	/* remove_group */
	NULL,			/* get_cb_real_name */
	NULL,			/* set_chat_topic */
	NULL,			/* find_blist_chat */
	NULL,			/* roomlist_get_list */
	NULL,			/* roomlist_cancel */
	NULL,			/* roomlist_expand_category */
	NULL,			/* can_receive_file */
	NULL,			/* send_file */
	NULL,			/* new_xfer */
	NULL,			/* offline_message */
	NULL,			/* whiteboard_prpl_ops */
	NULL,	/* send_raw */
	NULL,			/* roomlist_room_serialize */

	NULL,
	fx_send_attention,
	fx_attention_types,
	sizeof(PurplePluginProtocolInfo), /* struct_size */
	NULL,							/* get_account_text_table */
	NULL,
	NULL
};

static PurplePluginInfo info = {
    PURPLE_PLUGIN_MAGIC,
    PURPLE_MAJOR_VERSION,
    PURPLE_MINOR_VERSION,
    PURPLE_PLUGIN_PROTOCOL,
    NULL,
    0,
    NULL,
    PURPLE_PRIORITY_DEFAULT,

    "WebQQ",
    "WebQQ",
    "1.1",

    N_("Fetion Plugin"),
    N_("libpurple plugin implementing Fetion Protocol version 4"),
    "Wenpeng Li <levin108@gmail.com>",
    "http://code.google.com/p/ofetion/",


	NULL,
    NULL,
    NULL,

    NULL,
    &protocol_info,
    NULL,
    plugin_actions, /* this tells libpurple the address of the function to call
                       to get the list of plugin actions. */
    NULL,
    NULL,
    NULL,
    NULL
};

static void
init_plugin(PurplePlugin *UNUSED(plugin))
{
#ifdef ENABLE_NLS
	setlocale(LC_ALL, "");
	bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
	bindtextdomain(GETTEXT_PACKAGE , LOCALE_DIR);
	textdomain(GETTEXT_PACKAGE);
#endif
	buddy_to_added = NULL;
}

PURPLE_INIT_PLUGIN(webqq, init_plugin, info)
