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
#include "fx_sip.h"
#include "fx_user.h"
#include "fx_contact.h"
#include <signal.h>

struct unacked_list *unackedlist;

static gchar *generate_set_state_body(StateType state);
static gchar *generate_keep_alive_body();
static gchar *generate_modify_info(gint info_type, const gchar *value, const gchar *customConfig);

User *fetion_user_new(const gchar *no , const gchar *password)
{
	User *user = (User*)g_malloc0(sizeof(User));

 	struct sigaction sa;
 	sa.sa_handler = SIG_IGN;
 	sigaction(SIGPIPE, &sa, 0 );

	memset(user, 0, sizeof(User));
	if(strlen(no) == 11){
		strcpy(user->mobileno , no);
		user->loginType = LOGIN_TYPE_MOBILENO;
	}else{
		strcpy(user->sId , no);
		user->loginType = LOGIN_TYPE_FETIONNO;
	}
	strcpy(user->password , password);
	user->contactList = fetion_contact_new();
	user->groupList = fetion_group_new();
	user->sip = NULL;
	user->verification = NULL;
	user->customConfig = NULL;
	user->ssic = NULL;

	return user;
}

void fetion_user_set_userid(User *user, const gchar *userid1)
{
	strcpy(user->userId, userid1);
}

void fetion_user_set_sid(User *user, const gchar *sId1)
{
	strcpy(user->sId, sId1);
}

void fetion_user_set_mobileno(User *user, const gchar *mobileno1)
{
	strcpy(user->mobileno, mobileno1);
}

void fetion_user_set_verification_code(User *user, const gchar *code)
{
	g_return_if_fail(user != NULL);
	g_return_if_fail(code != NULL);

	user->verification->code = (gchar*)g_malloc0(strlen(code) + 1);
	strcpy(user->verification->code, code);
}

void fetion_user_free(User* user)
{
	g_return_if_fail(user != NULL);
	g_free(user->ssic);
	g_free(user->customConfig);
	fetion_verification_free(user->verification);
	fetion_sip_free(user->sip);
	g_free(user);
}

static gint set_state_cb(fetion_account *UNUSED(ac), const gchar *sipmsg, struct transaction *UNUSED(trans))
{
	purple_debug_info("fetion", "%s", sipmsg);
	return 0;
}

gint fetion_user_set_state(fetion_account *ac, gint state)
{
	SipHeader *eheader;
	fetion_sip *sip = ac->user->sip;
	gchar *body;
	gchar *res;
	struct transaction *trans;

	fetion_sip_set_type(sip , SIP_SERVICE);
	eheader = fetion_sip_event_header_new(SIP_EVENT_SETPRESENCE);
	fetion_sip_add_header(sip , eheader);
	trans = transaction_new();
	transaction_set_callid(trans, sip->callid);
	transaction_set_callback(trans, set_state_cb);
	transaction_add(ac, trans);
	body = generate_set_state_body(state);
	res = fetion_sip_to_string(sip , body);
	if(send(ac->sk, res, strlen(res), 0) == -1) return -1;
	ac->user->state = state;
	g_free(body);
	g_free(res);
	purple_debug_info("user","user state changed to %d" , state);
	return 0;
}

static gint modify_info_cb(fetion_account *ac, const gchar *sipmsg, struct transaction *UNUSED(trans))
{
	gint code = fetion_sip_get_code(sipmsg);
	if(code != 200) {
		purple_notify_error(ac->gc, NULL, _("Failed"), _("Modify account information failed"));
		return -1;
	}
	return 0;
}

gint fetion_modify_info(fetion_account *ac, gint info_type, const gchar *value)
{
	fetion_sip *sip = ac->user->sip;
	gchar     *body;
	gchar     *sipmsg;
	SipHeader *eheader;
	struct transaction *trans;


	fetion_sip_set_type(sip, SIP_SERVICE);
	eheader = fetion_sip_event_header_new(SIP_EVENT_SETUSERINFO);
	fetion_sip_add_header(sip , eheader);
	trans = transaction_new();
	transaction_set_callid(trans, sip->callid);
	transaction_set_callback(trans, modify_info_cb);
	transaction_add(ac, trans);
	body = generate_modify_info(info_type, value, ac->user->customConfig);
	sipmsg = fetion_sip_to_string(sip, body);
	g_free(body);
	if(send(ac->sk, sipmsg, strlen(sipmsg), 0) == -1) { g_free(sipmsg); return -1; }
	g_free(sipmsg);

	return 0;
}

static gint sms_myself_cb(fetion_account *ac, const gchar *sipmsg, struct transaction *UNUSED(trans))
{
	gint code = fetion_sip_get_code(sipmsg);
	if(code != 200 && code != 280) {
		purple_notify_error(ac->gc, NULL, _("Failed"), _("send sms to phone failed,unknown reason."));
		return -1;
	}
	return 0;
}

gint fetion_sms_myself(fetion_account *ac, const gchar *msg)
{
	SipHeader *toheader;
	SipHeader *eheader;
	gchar     *sipmsg;
	fetion_sip *sip = ac->user->sip;
	struct transaction *trans;

	fetion_sip_set_type(sip, SIP_MESSAGE);
	toheader = fetion_sip_header_new("T", ac->user->sipuri);
	eheader  = fetion_sip_event_header_new(SIP_EVENT_SENDCATMESSAGE);
	fetion_sip_add_header(sip, toheader);
	fetion_sip_add_header(sip, eheader);
	trans = transaction_new();
	transaction_set_callid(trans, sip->callid);
	transaction_set_callback(trans, sms_myself_cb);
	transaction_add(ac, trans);
	sipmsg = fetion_sip_to_string(sip, msg);
	purple_debug_info("fetion", "sent a message to myself");
	if(send(ac->sk, sipmsg, strlen(sipmsg), 0) == -1)  { g_free(sipmsg); return -1; }
	g_free(sipmsg);
	return 0;
}

static gint keep_alive_cb(fetion_account *ses, const gchar *sipmsg, struct transaction *UNUSED(trans))
{
	gint code;
	if((code = fetion_sip_get_code(sipmsg)) == 200) {
		purple_debug_info("util", "success keep alive %d\n", ses->sk);
	}	return 0;
}

gint fetion_user_keep_alive(fetion_account *ac)
{
	fetion_sip *sip = ac->user->sip;
	SipHeader *eheader;
	gchar *res, *body;
	struct transaction *trans;

	fetion_sip_set_type(sip , SIP_REGISTER);
	eheader = fetion_sip_event_header_new(SIP_EVENT_KEEPALIVE);
	fetion_sip_add_header(sip , eheader);
	trans = transaction_new();
	transaction_set_callid(trans, sip->callid);
	transaction_set_callback(trans, keep_alive_cb);
	transaction_add(ac, trans);
	body = generate_keep_alive_body();
	res = fetion_sip_to_string(sip , body);
	g_free(body);
	if(send(ac->sk, res, strlen(res), 0) == -1) { g_free(free); return -1; }
	g_free(res); 
	return 0;
}

Group *fetion_group_new()
{
	Group* list = (Group*)g_malloc0(sizeof(Group));
	list->pre = list;
	list->next = list;
	return list;
}
void fetion_group_list_append(Group *head, Group *group)
{
	head->next->pre = group;
	group->next = head->next;
	group->pre = head;
	head->next = group;
}

void fetion_group_list_prepend(Group *head, Group *group)
{
	head->pre->next = group;
	group->next = head;
	group->pre = head->pre;
	head->pre = group;
}

void fetion_group_list_remove(Group *group)
{
	group->next->pre = group->pre;
	group->pre->next = group->next;
}

void fetion_group_remove(Group *head, gint groupid)
{
	Group *gl_cur;
	foreach_grouplist(head , gl_cur){
		if(gl_cur->groupid == groupid){
			gl_cur->pre->next = gl_cur->next;
			gl_cur->next->pre = gl_cur->pre;
			free(gl_cur);
			break;
		}
	}
}

Group *fetion_group_list_find_by_id(Group *head, gint id)
{
	Group *gl_cur;
	foreach_grouplist(head , gl_cur)
		if(gl_cur->groupid == id) return gl_cur;
	return (Group*)0;
}

Group *fetion_group_list_find_by_name(Group *head, const gchar *name)
{
	Group *gl_cur;
	foreach_grouplist(head , gl_cur)
		if(strcmp(gl_cur->groupname, name) == 0) return gl_cur;
	return (Group*)0;
}

Verification *fetion_verification_new()
{
	Verification* ver = g_malloc0(sizeof(Verification));
	ver->algorithm = NULL;
	ver->guid = NULL;
	ver->type = NULL;
	ver->text = NULL;
	ver->tips = NULL;
	ver->code = NULL;
	return ver;
}
void fetion_verification_free(Verification *ver)
{
	g_return_if_fail(ver != NULL);
	g_free(ver->algorithm);
	g_free(ver->type);
	g_free(ver->text);
	g_free(ver->tips);
	g_free(ver->guid);
	g_free(ver->code);
	g_free(ver);
}

Contact *fetion_user_parse_presence_body(const gchar *body, User *user)
{
	xmlDocPtr doc;
	xmlNodePtr node , cnode;
	xmlChar* pos;
	Contact* contact;
	Contact* contactres;
	Contact* contactlist = user->contactList;
	Contact* currentContact;

	contactres = fetion_contact_new();

	doc = xmlParseMemory(body , strlen(body));
	node = xmlDocGetRootElement(doc);
	node = xml_goto_node(node , "c");
	while(node != NULL)
	{
		pos = xmlGetProp(node , BAD_CAST "id");
		currentContact = fetion_contact_list_find_by_userid(contactlist , (char*)pos);
		if(currentContact == NULL) {
			/*not a valid information*/
			/*debug_error("User %s is not a valid user" , (char*)pos);*/
			node = node->next;
			continue;
		}
		cnode = node->xmlChildrenNode;
		if(xmlHasProp(cnode , BAD_CAST "sid")) {
			pos = xmlGetProp(cnode , BAD_CAST "sid");
			strcpy(currentContact->sId ,  (char*)pos);
			xmlFree(pos);
		}
		if(xmlHasProp(cnode , BAD_CAST "m")) {
			pos = xmlGetProp(cnode , BAD_CAST "m");
			strcpy(currentContact->mobileno ,  (char*)pos);
			xmlFree(pos);
		}
		if(xmlHasProp(cnode , BAD_CAST "l")) {
			pos = xmlGetProp(cnode , BAD_CAST "l");
			currentContact->scoreLevel = atoi((char*)pos);
			xmlFree(pos);
		}
		if(xmlHasProp(cnode , BAD_CAST "n")) {
			pos = xmlGetProp(cnode , BAD_CAST "n");
			strcpy(currentContact->nickname ,  (char*)pos);
			xmlFree(pos);
		}
		if(xmlHasProp(cnode , BAD_CAST "i")) {
			pos = xmlGetProp(cnode , BAD_CAST "i");
			strcpy(currentContact->impression ,  (char*)pos);
			xmlFree(pos);
		}
		if(xmlHasProp(cnode , BAD_CAST "p")) {
			pos = xmlGetProp(cnode , BAD_CAST "p");
			if(strcmp(currentContact->portraitCrc, (char*)pos) == 0
					|| strcmp((char*)pos, "0") == 0)
				currentContact->imageChanged = 0;
			else
				currentContact->imageChanged = 1;
			strcpy(currentContact->portraitCrc ,  (char*)pos);
			xmlFree(pos);
		} else {
			currentContact->imageChanged = 0;
		}

		if(xmlHasProp(cnode , BAD_CAST "c")) {
			pos = xmlGetProp(cnode , BAD_CAST "c");
			strcpy(currentContact->carrier , (char*)pos);
			xmlFree(pos);
		}
		if(xmlHasProp(cnode , BAD_CAST "cs")) {
			pos = xmlGetProp(cnode , BAD_CAST "cs");
			currentContact->carrierStatus = atoi((char*)pos);
			xmlFree(pos);
		}
		if(xmlHasProp(cnode , BAD_CAST "s")) {
			pos = xmlGetProp(cnode , BAD_CAST "s");
			currentContact->serviceStatus = atoi((char*)pos);
			xmlFree(pos);
		}
#if 0
		if(xmlHasProp(cnode , BAD_CAST "sms")){
			pos = xmlGetProp(cnode , BAD_CAST "sms");
			xmlFree(pos);
		}
#endif
		cnode = xml_goto_node(node , "pr");
		if(xmlHasProp(cnode , BAD_CAST "dt")) {
			pos = xmlGetProp(cnode , BAD_CAST "dt");
			strcpy(currentContact->devicetype ,  *((char*)pos) == '\0' ? "PC" : (char*)pos);
			xmlFree(pos);
		}
		if(xmlHasProp(cnode , BAD_CAST "b")) {
			pos = xmlGetProp(cnode , BAD_CAST "b");
			currentContact->state = atoi((char*)pos);
			xmlFree(pos);
		}
		contact = fetion_contact_new();
		memset(contact , 0 , sizeof(contact));
		memcpy(contact , currentContact , sizeof(Contact));
		fetion_contact_list_append(contactres , contact);
		node = node->next;
	}
	xmlFreeDoc(doc);
	return contactres;
}

static gchar *generate_set_state_body(StateType state)	
{
	gchar s[16];
	gchar data[] = "<args></args>";
	xmlChar* res;
	xmlDocPtr doc;
	xmlNodePtr node;
	doc = xmlParseMemory(data , strlen(data));
	node = xmlDocGetRootElement(doc);
	node = xmlNewChild(node , NULL , BAD_CAST "presence" , NULL);
	node = xmlNewChild(node , NULL , BAD_CAST "basic" , NULL);
	snprintf(s, sizeof(s) - 1 , "%d" , state);
	xmlNewProp(node , BAD_CAST "value" , BAD_CAST s);
	xmlDocDumpMemory(doc , &res , NULL);
	xmlFreeDoc(doc);
	return xml_convert(res);
}
static gchar *generate_keep_alive_body()
{
	gchar args[] = "<args></args>";
	xmlChar *res;
	xmlDocPtr doc;
	xmlNodePtr node;
	doc = xmlParseMemory(args , strlen(args));
	node = xmlDocGetRootElement(doc);
	node = xmlNewChild(node , NULL , BAD_CAST "credentials" , NULL);
	xmlNewProp(node , BAD_CAST "domains" , BAD_CAST "fetion.com.cn");
	xmlDocDumpMemory(doc , &res , NULL);
	xmlFreeDoc(doc);
	return xml_convert(res);
}
static gchar *generate_modify_info(gint info_type, const gchar *value, const gchar *customConfig)
{
	gchar      args[] = "<args></args>";
	xmlChar   *res;
	xmlDocPtr  doc;
	xmlNodePtr node , cnode;
	doc = xmlParseMemory(args, strlen(args));
	node = xmlDocGetRootElement(doc);
	node = xmlNewChild(node, NULL, BAD_CAST "userinfo", NULL);
	cnode = xmlNewChild(node, NULL, BAD_CAST "personal", NULL);
	switch(info_type) {
		case MODIFY_INFO_NICKNAME :
			xmlNewProp(cnode, BAD_CAST "nickname", BAD_CAST value);
			break;
		case MODIFY_INFO_IMPRESA :
			xmlNewProp(cnode, BAD_CAST "impresa", BAD_CAST value);
			break;
		default:
			break;
	}
	xmlNewProp(cnode, BAD_CAST "version", BAD_CAST "0");
	cnode = xmlNewChild(node, NULL, BAD_CAST "custom-config", BAD_CAST customConfig);
	xmlNewProp(cnode, BAD_CAST "type", BAD_CAST "PC");
	xmlNewProp(cnode, BAD_CAST "version", BAD_CAST "0");
	xmlDocDumpMemory(doc, &res, NULL);
	xmlFreeDoc(doc);
	return xml_convert(res);
}
