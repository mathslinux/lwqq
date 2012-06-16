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
#include "webqq.h"
#include "fx_types.h"

extern GSList        *sessions;

xmlNodePtr xml_goto_node(xmlNodePtr node, const gchar *name)
{
	xmlNodePtr pos = node;
	xmlNodePtr tmp = NULL;
	while(pos != NULL) {
		if(strcmp(name , (gchar*)pos->name) == 0) return pos;
		tmp = pos->xmlChildrenNode;
		if(tmp != NULL && xmlStrcmp(tmp->name , BAD_CAST "text") != 0
		   &&tmp->type == XML_ELEMENT_NODE
		   && (tmp = xml_goto_node(tmp , name)) != NULL )
			return tmp;
		pos = pos->next;
	};
	return NULL;
}

gchar* xml_convert(xmlChar *in)
{
	gchar *res, *pos ;
	pos = strstr((gchar*)in, "?>") + 2;
	res = (gchar*)g_malloc0(strlen(pos) + 1);
	memcpy(res, pos, strlen(pos));
	xmlFree(in);
	return res;
}

const gchar *get_status_id(const gchar* status)
{
	/*switch(state) {
		case P_ONLINE:      return "Online";
		case P_MEETING:     return "Meeting";
		case P_RIGHTBACK:   return "Right back";
		case P_OUTFORLUNCH: return "Out for lunch";
		case P_AWAY:		return "Away";
		case P_ONTHEPHONE:  return "On the phone";
		case P_BUSY:        return "Busy";
		case P_DONOTDISTURB:return "Don't disturb";
		case P_HIDDEN:      return "Offline";
		case P_OFFLINE:     return "Offline";
		default:			return "Online";
	}
	return "Online";*/
    if(strcmp(status,"online")==0) return "Online";
    return "Online";

}

struct transaction *transaction_new() 
{
	struct transaction *trans;
	trans = g_malloc0(sizeof(struct transaction));
	memset(trans, 0, sizeof(struct transaction));
	return trans;
}

void transaction_free(struct transaction *trans)
{
	g_free(trans);
}

void transaction_set_callid(struct transaction *trans, gint callid)
{
	trans->callid = callid;
}

void transaction_set_userid(struct transaction *trans, const gchar *userId)
{
	snprintf(trans->userId, sizeof(trans->userId) - 1, "%s", userId);
}

void transaction_set_msg(struct transaction *trans, const gchar *msg)
{
	memset(trans->msg, 0, sizeof(trans->msg));
	if(msg)	snprintf(trans->msg, sizeof(trans->msg) - 1, "%s", msg);
}


void transaction_set_callback(struct transaction *trans, TransCallback callback)
{
	trans->callback = callback;
}

void transaction_set_timeout(struct transaction *trans, GSourceFunc timeout, gpointer data)
{
	trans->timer = purple_timeout_add_seconds(20, timeout, data);
}

void transaction_add(fetion_account *ses, struct transaction *trans)
{
	ses->trans = g_slist_append(ses->trans, trans);
}

void transaction_wait(fetion_account *ses, struct transaction *trans)
{
	ses->trans_wait = g_slist_append(ses->trans_wait, trans);
}

void transaction_wakeup(fetion_account *ses, struct transaction *trans)
{
	ses->trans_wait = g_slist_remove(ses->trans_wait, trans);
}

void transaction_remove(fetion_account *ses, struct transaction *trans)
{
	ses->trans = g_slist_remove(ses->trans, trans);
}

fetion_account *session_new(PurpleAccount *account)
{
	fetion_account *ses = g_malloc0(sizeof(fetion_account));
	ses->account = account;
	ses->trans = (GSList*)0;
	ses->trans_wait = (GSList*)0;
	ses->data = (gchar*)0;
	return ses;
}

fetion_account *session_clone(fetion_account *ac)
{
	fetion_account *ses = g_malloc0(sizeof(fetion_account));
	ses->account = ac->account;
	ses->trans = (GSList*)0;
	ses->trans_wait = (GSList*)0;
	ses->data = (gchar*)0;
	ses->user = ac->user;
	ses->gc = ac->gc;
	ses->chan_ready = 0;
	return ses;
}

void session_set_userid(fetion_account *ses, const gchar *userId)
{
	snprintf(ses->userId, sizeof(ses->userId) - 1, "%s", userId);
}

void session_add(fetion_account *ac)
{
	sessions = g_slist_append(sessions, ac);
}

fetion_account *session_find(const gchar *key)
{
	fetion_account *ac;
	GSList  *list = sessions;

	while(list) {
		ac = (fetion_account*)(list->data);
		if(strcmp(ac->userId, key) == 0)
			return ac;
		list = list->next;
	}
	return (fetion_account*)0;
}

fetion_account *session_find_by_sk(gint sk)
{
	fetion_account *ses;
	GSList  *list = sessions;

	while(list) {
		ses = (fetion_account *)(list->data);
		if(ses->sk == sk)
			return ses;
		list = list->next;
	}
	return (fetion_account*)0;
}

void session_remove(fetion_account *ses)
{
	sessions = g_slist_remove(sessions, ses);
}

void session_destroy(fetion_account *ses)
{
	g_return_if_fail(ses != NULL);
	purple_input_remove(ses->conn);
	g_free(ses->data);
	close(ses->sk);
	g_free(ses);
}
