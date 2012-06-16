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
#include "notify.h"
#include "debug.h"
#include "pounce.h"
#include "fx_contact.h"
#include "fx_sip.h"
#include "fx_chat.h"

static gchar *generate_invite_friend_body(const gchar *sipuri);
static gchar *generate_send_nudge_body();
static void parse_send_sms_to_phone(const gchar *xml, gint *daycount, gint *mountcount);

void process_message_cb(fetion_account *ac, const gchar *sipmsg)
{
	gchar len[16], callid[16], sequence[16];
	gchar sendtime[32], from[64], rep[256], *msg, *sid;
	PurpleConnection *pc;
	Contact *cnt;

	fetion_sip_get_attr(sipmsg , "F" , from);
	fetion_sip_get_attr(sipmsg , "L" , len);
	fetion_sip_get_attr(sipmsg , "I" , callid);
	fetion_sip_get_attr(sipmsg , "Q" , sequence);
	fetion_sip_get_attr(sipmsg , "D" , sendtime);	

	msg = strstr(sipmsg, "\r\n\r\n") + 4;

	snprintf(rep, sizeof(rep) - 1 ,
			"SIP-C/4.0 200 OK\r\n"
			"I: %s\r\n"
			"Q: %s\r\n"
			"F: %s\r\n\r\n",
		   	callid, sequence, from);

	pc = purple_account_get_connection(ac->account);
	sid = fetion_sip_get_sid_by_sipuri(from);
	cnt = fetion_contact_list_find_by_sid(ac->user->contactList, sid);
	serv_got_im(pc, cnt->userId, msg, 0, time(NULL));
	g_free(sid);
	send(ac->sk, rep, strlen(rep), 0);

}

static gint process_invite_conn_cb(gpointer data, gint source, const gchar *UNUSED(error_message))
{
	gchar *sipres;
	SipHeader *aheader;
	SipHeader *theader;
	SipHeader *mheader;
	SipHeader *nheader;
	process_invite_data *conn_data = (process_invite_data*)data;
	fetion_account *ses = conn_data->ac;
	fetion_sip     *sip = ses->user->sip;

	ses->sk = source;

	/* start listen subthread */
	if((ses->conn = purple_input_add(source, PURPLE_INPUT_READ,
					(PurpleInputFunction)push_cb, ses)) == 0) return -1;

	fetion_sip_set_type(sip, SIP_REGISTER);
	aheader = fetion_sip_credential_header_new(conn_data->credential);
	theader = fetion_sip_header_new("K" , "text/html-fragment");
	mheader = fetion_sip_header_new("K" , "multiparty");
	nheader = fetion_sip_header_new("K" , "nudge");
	fetion_sip_add_header(sip , aheader);
	fetion_sip_add_header(sip , theader);
	fetion_sip_add_header(sip , mheader);
	fetion_sip_add_header(sip , nheader);
	sipres = fetion_sip_to_string(sip , NULL);
	if(send(ses->sk, sipres, strlen(sipres), 0) == -1) { g_free(sipres); return -1; }
	g_free(sipres);
	return 0;
}

gint process_invite_cb(fetion_account *ac, const gchar *sipmsg)
{
	gchar  from[128], auth[128], *ipaddress;
	gchar  buf[1024];
	gint   port;
	gchar          *credential, *sid;
	Contact        *cnt;
	fetion_account *ses;
	process_invite_data *data = g_malloc0(sizeof(process_invite_data));

	fetion_sip_get_attr(sipmsg , "F" , from);
	fetion_sip_get_attr(sipmsg , "A" , auth);
	fetion_sip_get_auth_attr(auth , &ipaddress , &port , &credential);

	snprintf(buf, sizeof(buf) - 1,
		   	"SIP-C/4.0 200 OK\r\n"
			"F: %s\r\n"
			"I: 61\r\n"
			"Q: 200002 I\r\n\r\n",
			from);
	if(send(ac->sk, buf, strlen(buf), 0) == -1) { g_free(data); return -1; }

	sid = fetion_sip_get_sid_by_sipuri(from);
	cnt = fetion_contact_list_find_by_sid(ac->user->contactList, sid);
	ses = session_clone(ac);
	session_set_userid(ses, cnt->userId);	
	session_add(ses);
	data->ac = ses;
	strncpy(data->credential, credential, sizeof(data->credential) - 1);
	if(!(ses->conn_data = purple_proxy_connect(NULL, ac->account, ipaddress, port,
				  (PurpleProxyConnectFunction)process_invite_conn_cb, data))) {
		if(!(ses->conn_data = purple_proxy_connect(NULL, ac->account, ipaddress, 443,
						(PurpleProxyConnectFunction)process_invite_conn_cb, data))) {
			g_free(ipaddress);
			g_free(credential);
			g_free(sid);
			session_remove(ses);
			return -1;
		}
	}	

	g_free(ipaddress);
	g_free(credential);
	g_free(sid);
	return 0;
}

void process_enter_cb(fetion_account *ses, const gchar *sipmsg)
{
	GSList *cur = ses->trans_wait;
	struct transaction *trans;
	ses->chan_ready = 1;
	while(cur) {
		trans = (struct transaction*)(cur->data);
		fetion_send_sms(ses, trans->userId, trans->msg);
		transaction_wakeup(ses, trans);
		cur = ses->trans_wait;
	}
	purple_debug_info("fetion", "%s\n" , sipmsg);
}

void process_left_cb(fetion_account *ses, const gchar *sipmsg)
{
	gchar *sipuri;
	
	fetion_sip_parse_userleft(sipmsg, &sipuri);
	session_remove(ses);
	session_destroy(ses);

	purple_debug_info("fetion", "%s\n", sipmsg);
	g_free(sipuri);
}

static gint sms_response_cb(fetion_account *ac, const gchar *sipmsg, struct transaction *trans)
{
	gint code = fetion_sip_get_code(sipmsg);
	PurpleConversation *conv;

	purple_timeout_remove(trans->timer);

	if(code == 200 || code == 280) return 0;

	if(!(conv = purple_find_conversation_with_account(PURPLE_CONV_TYPE_ANY, trans->userId,
						  ac->account))) return -1;
	purple_conversation_write(conv, NULL,
					  _("Message sent failed:"),
					  PURPLE_MESSAGE_ERROR, time(NULL));
	purple_conversation_write(conv, NULL, trans->msg,
					  PURPLE_MESSAGE_ERROR, time(NULL));
	purple_conversation_write(conv, NULL, sipmsg,
					  PURPLE_MESSAGE_RAW, time(NULL));

	return 0;
}

static void sms_timeout_cb(gpointer data)
{
	sms_timeout_data *st_data = (sms_timeout_data*)data;
	PurpleConversation *conv;
	struct transaction *trans = st_data->trans;

	if(!(conv = purple_find_conversation_with_account(PURPLE_CONV_TYPE_ANY, trans->userId,
						  st_data->ac->account))) return;
	purple_conversation_write(conv, NULL,
					  _("Message sent timeout:"),
					  PURPLE_MESSAGE_ERROR, time(NULL));
	purple_conversation_write(conv, NULL, trans->msg,
					  PURPLE_MESSAGE_ERROR, time(NULL));
	purple_timeout_remove(trans->timer);
	transaction_remove(st_data->ac, trans);
	g_free(st_data);
}

gint fetion_send_sms(fetion_account *ses, const gchar* who, const gchar *msg)
{
	fetion_sip *sip = ses->user->sip;
	SipHeader *toheader, *cheader, *kheader, *nheader;
	Contact   *cnt;
	gchar     *res;

	sms_timeout_data *st_data;

	if(!(cnt = fetion_contact_list_find_by_userid(ses->user->contactList, who))) return -1;
	struct transaction *trans = transaction_new();

	transaction_set_userid(trans, who);
	transaction_set_msg(trans, msg);

	if(ses->chan_ready) {
		fetion_sip_set_type(sip , SIP_MESSAGE);
		nheader  = fetion_sip_event_header_new(SIP_EVENT_CATMESSAGE);
		toheader = fetion_sip_header_new("T" , cnt->sipuri);
		cheader  = fetion_sip_header_new("C" , "text/plain");
		kheader  = fetion_sip_header_new("K" , "SaveHistory");
		fetion_sip_add_header(sip , toheader);
		fetion_sip_add_header(sip , cheader);
		fetion_sip_add_header(sip , kheader);
		fetion_sip_add_header(sip , nheader);
		transaction_set_callid(trans, sip->callid);
		transaction_set_callback(trans, sms_response_cb);
		st_data = g_malloc0(sizeof(sms_timeout_data));
		st_data->ac = ses;
		st_data->trans = trans;
		transaction_set_timeout(trans, (GSourceFunc)sms_timeout_cb, st_data);
		transaction_add(ses, trans);
		res = fetion_sip_to_string(sip , msg);
		if(send(ses->sk, res, strlen(res), 0) == -1) { g_free(res); return -1; }
		g_free(res);
	} else {
		transaction_wait(ses, trans);
	}

	return 0;
}

static gint send_sms_to_phone_cb(fetion_account *ac, const gchar *sipmsg, struct transaction *UNUSED(trans))
{
	gint   code, daycount, monthcount;
	PurpleConnection *pc;
	gchar *pos;	

	if((code = fetion_sip_get_code(sipmsg)) != 280) {
		pc = purple_account_get_connection(ac->account);
		purple_notify_error(pc, NULL, "message", "send message to mobile failed");
		return -1;
	} else{
		if(!(pos = strstr(sipmsg, "\r\n\r\n"))) return -1;
		parse_send_sms_to_phone(pos + 4, &daycount, &monthcount);
		purple_debug_info("message", "send message to mobile success, "
				"%d messages sent today, %d messages sent this monty",
				daycount, monthcount);
		return 0;
	}
	return 0;
}

gint fetion_send_sms_to_phone(fetion_account *ac, const gchar *userid,
			   	const gchar *msg)
{
	
	SipHeader *toheader;
	SipHeader *eheader;
	SipHeader *aheader;
	gchar     *res;
	User      *user = ac->user;
	fetion_sip *sip = user->sip;
	gchar astr[1024];
	Contact   *cnt;
	struct transaction *trans;

	if(!(cnt = fetion_contact_list_find_by_userid(user->contactList, userid))) return -1;

	fetion_sip_set_type(sip , SIP_MESSAGE);
	toheader = fetion_sip_header_new("T" , cnt->sipuri);
	eheader  = fetion_sip_event_header_new(SIP_EVENT_SENDCATMESSAGE);
	fetion_sip_add_header(sip , toheader);
	if(user->verification != NULL){
		snprintf(astr, sizeof(astr) - 1, "Verify algorithm=\"picc\",chid=\"%s\",response=\"%s\""
				, user->verification->guid
				, user->verification->code);
		aheader = fetion_sip_header_new("A" , astr);
		fetion_sip_add_header(sip , aheader);
	}
	trans = transaction_new();
	transaction_set_callid(trans, sip->callid);
	transaction_set_callback(trans, send_sms_to_phone_cb);
	transaction_add(ac, trans);
	fetion_sip_add_header(sip , eheader);
	res = fetion_sip_to_string(sip , msg);
	if(send(ac->sk, res, strlen(res), 0) == -1) { g_free(res); return -1; }
	//debug_info("Sent a message to (%s)`s mobile phone" , sid);
	g_free(res);
	return 0;
}

gint fetion_send_nudge(fetion_account *ses, const gchar *who)
{
	SipHeader *toheader;
	gchar     *res;
	gchar     *body;
	fetion_sip *sip = ses->user->sip;
	Contact   *cnt;

	cnt = fetion_contact_list_find_by_userid(ses->user->contactList, who);
	fetion_sip_set_type(sip , SIP_INCOMING);
	toheader = fetion_sip_header_new("T" , cnt->sipuri);
	fetion_sip_add_header(sip , toheader);
	body = generate_send_nudge_body();
	res = fetion_sip_to_string(sip , body);
	g_free(body);
	if(send(ses->sk, res, strlen(res), 0) == -1) { g_free(res); return -1; }
	g_free(res);
	return 0;
}

static gint invite_buddy_cb(fetion_account *ses, const gchar *UNUSED(sipmsg), struct transaction *trans)
{
	if(trans->msg[0] == '\0')
		fetion_send_nudge(ses, trans->userId);
	else
		fetion_send_sms(ses, trans->userId, trans->msg);
	return 0;
}

static gint chat_reg_cb(fetion_account *ses, const gchar *UNUSED(sipmsg), struct transaction *trans)
{
	fetion_sip *sip = ses->user->sip;
	SipHeader *eheader;
	gchar     *body, *res;
	struct transaction *trans0;
	Contact *cnt;

	fetion_sip_set_type(sip , SIP_SERVICE);
	eheader = fetion_sip_event_header_new(SIP_EVENT_INVITEBUDDY);
	fetion_sip_add_header(sip , eheader);
	cnt = fetion_contact_list_find_by_userid(ses->user->contactList, trans->userId);
	body = generate_invite_friend_body(cnt->sipuri);
	trans0 = transaction_new();
	transaction_set_userid(trans0, trans->userId);
	transaction_set_msg(trans0, trans->msg);
	transaction_set_callid(trans0, sip->callid);
	transaction_set_callback(trans0, invite_buddy_cb);
	transaction_add(ses, trans0);
	res = fetion_sip_to_string(sip , body);	
	g_free(body);
	if(send(ses->sk, res, strlen(res), 0) == -1) { g_free(res); return -1; }

	return 0;
}

static gint invite_connect_cb(gpointer data, gint source, const gchar *UNUSED(error_message))
{
	SipHeader *theader, *mheader, *nheader, *aheader;
	invite_data *d = (invite_data*)data;
	gchar *res;

	fetion_account *ses;
	fetion_sip     *sip;
	if(!(ses = session_find(d->trans->userId))) { g_free(d); return -1; }
	ses->sk = source;

	sip = fetion_sip_clone(ses->user->sip);

	/* listen for this thread */
	if((ses->conn = purple_input_add(source, PURPLE_INPUT_READ,
					(PurpleInputFunction)push_cb, ses)) == 0) {
		g_free(d);
	   	return -1;
	}

	fetion_sip_set_type(sip , SIP_REGISTER);
	aheader = fetion_sip_credential_header_new(d->credential);
	theader = fetion_sip_header_new("K" , "text/html-fragment");
	mheader = fetion_sip_header_new("K" , "multiparty");
	nheader = fetion_sip_header_new("K" , "nudge");
	transaction_set_callid(d->trans, sip->callid);
	transaction_set_callback(d->trans, chat_reg_cb);
	transaction_add(ses, d->trans);
	fetion_sip_add_header(sip , aheader);
	fetion_sip_add_header(sip , theader);
	fetion_sip_add_header(sip , mheader);
	fetion_sip_add_header(sip , nheader);
	res = fetion_sip_to_string(sip , NULL);
	if(send(source, res, strlen(res), 0) == -1) { g_free(res); return -1; }
	g_free(res);
	g_free(d->credential);
	g_free(d);

	return 0;
}

static gint new_chat_cb(fetion_account *ac, const gchar *sipmsg, struct transaction *trans)
{
	gchar auth[256], *credential, *ip;
	gint  port;
	fetion_account *se;
	invite_data *data = g_malloc0(sizeof(invite_data));

	fetion_sip_get_attr(sipmsg , "A" , auth);
	fetion_sip_get_auth_attr(auth , &ip , &port , &credential);

	data->trans = transaction_new();
	memcpy(data->trans, trans, sizeof(struct transaction));
	data->credential = credential;

	se = session_clone(ac);
	session_set_userid(se, trans->userId);
	session_add(se);
	if(!(se->conn_data = purple_proxy_connect(NULL, ac->account, ip, port,
						(PurpleProxyConnectFunction)invite_connect_cb, data))) {
		if(!(se->conn_data = purple_proxy_connect(NULL, ac->account, ip, 443,
						(PurpleProxyConnectFunction)invite_connect_cb, data))) {
			session_remove(se);
			g_free(se);
			return -1;
		}
	}

	g_free(ip);

	return 0;
}

gint new_chat(fetion_account *ac, const gchar *userid, const gchar *what)
{
	fetion_sip *sip = ac->user->sip;
	gchar     *res;
	SipHeader *eheader;

	struct transaction *trans;

	/*start chat*/
	fetion_sip_set_type(sip , SIP_SERVICE);
	eheader = fetion_sip_event_header_new(SIP_EVENT_STARTCHAT);
	fetion_sip_add_header(sip , eheader);
	trans = transaction_new();
	transaction_set_callid(trans, sip->callid);
	transaction_set_userid(trans, userid);
	transaction_set_msg(trans, what);
	transaction_set_callback(trans, new_chat_cb);
	transaction_add(ac, trans);
	res = fetion_sip_to_string(sip , (gchar*)0);
	if(send(ac->sk, res, strlen(res), 0) == -1) { g_free(res); return -1; }
	
	g_free(res); 

	return 0;
}

static gchar *generate_invite_friend_body(const gchar *sipuri)
{
	xmlChar *buf;
	xmlDocPtr doc;
	xmlNodePtr node;
	gchar body[] = "<args></args>";
	doc = xmlParseMemory(body , strlen(body));
	node = xmlDocGetRootElement(doc);
	node = xmlNewChild(node , NULL , BAD_CAST "contacts" , NULL);
	node = xmlNewChild(node , NULL , BAD_CAST "contact" , NULL);
	xmlNewProp(node , BAD_CAST "uri" , BAD_CAST sipuri);
	xmlDocDumpMemory(doc , &buf , NULL);
	xmlFreeDoc(doc);
	return xml_convert(buf);
}

static gchar *generate_send_nudge_body()
{
	xmlChar *buf;
	xmlDocPtr doc;
	xmlNodePtr node;
	gchar body[] = "<is-composing></is-composing>";
	doc = xmlParseMemory(body , strlen(body));
	node = xmlDocGetRootElement(doc);
	node = xmlNewChild(node , NULL , BAD_CAST "state" , NULL);
	xmlNodeSetContent(node , BAD_CAST "nudge");
	xmlDocDumpMemory(doc , &buf , NULL);
	xmlFreeDoc(doc);
	return xml_convert(buf);
}

static void parse_send_sms_to_phone(const gchar *xml, gint *daycount, gint *mountcount)
{
	xmlDocPtr doc;
	xmlNodePtr node;
	xmlChar* res;
	doc = xmlParseMemory(xml , strlen(xml));
	node = xmlDocGetRootElement(doc);
	node = xml_goto_node(node , "frequency");
	res = xmlGetProp(node , BAD_CAST "day-count");
	*daycount = atoi((gchar*)res);
	xmlFree(res);
	res = xmlGetProp(node , BAD_CAST "month-count");
	*mountcount = atoi((gchar*)res);
	xmlFree(res);
	xmlFreeDoc(doc);
}
