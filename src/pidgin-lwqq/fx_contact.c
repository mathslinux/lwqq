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
#include "fx_contact.h"
#include "fx_sip.h"
#include "fx_user.h"
#include "fx_types.h"
/*private */

static gchar *generate_subscribe_body(const gchar *version);
static gchar *generate_contact_info_body(const gchar *userid);
static gchar *generate_set_displayname_body(const gchar *userid , const gchar *name);
static gchar *generate_move_to_group_body(const gchar *userid, gint buddylist);
static gchar *generate_delete_buddy_body(const gchar *userid);

Contact* fetion_contact_new()
{
	Contact* list = (Contact*)g_malloc0(sizeof(Contact));
	memset(list , 0 , sizeof(Contact));
	list->state = P_HIDDEN;
	list->pre = list;
	list->next = list;
	return list;
}
void fetion_contact_list_append(Contact* cl , Contact* contact)
{
	cl->next->pre = contact;
	contact->next = cl->next;
	contact->pre = cl;
	cl->next = contact;
}

Contact* fetion_contact_list_find_by_userid(Contact* contactlist , const char* userid)
{
	Contact* cl_cur;
	foreach_contactlist(contactlist , cl_cur){
		if(strcmp(cl_cur->userId , userid) == 0)
			return cl_cur;
	}
	return NULL;
}

Contact* fetion_contact_list_find_by_sid(Contact *contactlist , const gchar *sid)
{
	Contact *cl_cur;
	gchar *sid1;
	foreach_contactlist(contactlist , cl_cur){
		sid1 = fetion_sip_get_sid_by_sipuri(cl_cur->sipuri);
		if(strcmp(sid , sid1) == 0){
			free(sid1);
			return cl_cur;
		}
		free(sid1);
	}
	return (Contact*)0;
}

Contact *fetion_contact_list_find_by_mobileno(Contact *contactlist, const char *mobileno)
{
	Contact *cl_cur;
	foreach_contactlist(contactlist, cl_cur) {
		if(strcmp(cl_cur->mobileno, mobileno) == 0)
			return cl_cur;
	}
	return NULL;
}

void fetion_contact_list_remove_by_userid(Contact* contactlist , const char* userid)
{
	Contact *cl_cur;
	foreach_contactlist(contactlist , cl_cur){
		if(strcmp(cl_cur->userId , userid) == 0){
			cl_cur->pre->next = cl_cur->next;
			cl_cur->next->pre = cl_cur->pre;
			free(cl_cur);
			break;
		}
	}
}

void fetion_contact_list_remove(Contact *contact)
{
	contact->next->pre = contact->pre;
	contact->pre->next = contact->next;
}
void fetion_contact_list_free(Contact* contact)
{
	Contact *cl_cur , *del_cur;
	for(cl_cur = contact->next ; cl_cur != contact ;){
		cl_cur->pre->next = cl_cur->next;
		cl_cur->next->pre = cl_cur->pre;
		del_cur = cl_cur;
		cl_cur = cl_cur->next;
		free(del_cur);
	}
	free(contact);
}
gint fetion_contact_subscribe_only(gint sk, User* user)
{
	gchar *res, *body;
	fetion_sip* sip;
	SipHeader* eheader;

	sip = user->sip;
	fetion_sip_set_type(sip , SIP_SUBSCRIPTION);
	eheader = fetion_sip_event_header_new(SIP_EVENT_PRESENCE);
	if(!eheader) return -1;
	fetion_sip_add_header(sip , eheader);
	body = generate_subscribe_body("0");
	if(!body) { free(eheader);return -1; }
	res = fetion_sip_to_string(sip , body);
	if(!res) { free(eheader);free(body);return -1; }
	g_free(body);
	if(send(sk , res , strlen(res), 0) == -1) { 
		g_free(res);
		return -1;
	}
	return 0;
}
gint fetion_contact_get_contact_info(fetion_account *ac, const gchar *userid, TransCallback callback)
{
	fetion_sip *sip = ac->user->sip;
	SipHeader *eheader;
	Contact   *contact;
	gchar *res , *body;

	struct transaction *trans;
	contact = fetion_contact_list_find_by_userid(ac->user->contactList, userid);
	body = generate_contact_info_body(contact->userId);
	if(!body) return -1;
	fetion_sip_set_type(sip , SIP_SERVICE);
	eheader = fetion_sip_event_header_new(SIP_EVENT_GETCONTACTINFO);
 	trans = transaction_new();
	transaction_set_callid(trans, sip->callid);
	transaction_set_callback(trans, callback);
	transaction_add(ac, trans);
	fetion_sip_add_header(sip , eheader);
	res = fetion_sip_to_string(sip , body);
	g_free(body);
	if(send(ac->sk, res, strlen(res), 0) == -1) return -1;
	g_free(res);
	return 0;
}

gint fetion_contact_has_ungrouped(Contact *contactlist)
{
	Contact *cur;

	foreach_contactlist(contactlist , cur){
		if(cur->groupid == BUDDY_LIST_NOT_GROUPED)
		    return 1;
	}
	return 0;

}

gint fetion_contact_has_strangers(Contact *contactlist)
{
	Contact *cur;

	foreach_contactlist(contactlist , cur){
		if(cur->groupid == BUDDY_LIST_STRANGER)
		    return 1;
	}
	return 0;

}

gint fetion_contact_set_displayname(fetion_account *ac, const gchar *userid, const gchar *name)
{
	fetion_sip *sip = ac->user->sip;
	SipHeader *eheader;
	gchar     *res, *body;
	Contact   *cnt;

	cnt = fetion_contact_list_find_by_userid(ac->user->contactList, userid);

	fetion_sip_set_type(sip , SIP_SERVICE);
	eheader = fetion_sip_event_header_new(SIP_EVENT_SETCONTACTINFO);
	fetion_sip_add_header(sip , eheader);
	body = generate_set_displayname_body(cnt->userId , name);
	res = fetion_sip_to_string(sip , body);
	g_free(body);
	if(send(ac->sk, res, strlen(res), 0) == -1) { g_free(res); return -1; }
	g_free(res);

	return 0;
}
gint fetion_contact_move_to_group(fetion_account *ac, const gchar *userid, gint buddylist)
{
	fetion_sip *sip = ac->user->sip;
	SipHeader *eheader;
	gchar     *res, *body;

	fetion_sip_set_type(sip , SIP_SERVICE);
	eheader = fetion_sip_event_header_new(SIP_EVENT_SETCONTACTINFO);
	fetion_sip_add_header(sip , eheader);
	body = generate_move_to_group_body(userid , buddylist);
	res = fetion_sip_to_string(sip , body);
	g_free(body);
	if(send(ac->sk, res, strlen(res), 0) == -1) { g_free(res); return -1; }
	g_free(res);
	return 0;
}
gint fetion_contact_delete_buddy(fetion_account *ac, const gchar *userid)
{
	fetion_sip *sip = ac->user->sip;
	SipHeader *eheader;
	gchar     *res, *body;
	Contact   *cnt;

	if(!(cnt = fetion_contact_list_find_by_userid(ac->user->contactList, userid))) return -1;
	fetion_sip_set_type(sip , SIP_SERVICE);
	eheader = fetion_sip_event_header_new(SIP_EVENT_DELETEBUDDY);
	fetion_sip_add_header(sip , eheader);
	body = generate_delete_buddy_body(cnt->userId);
	res = fetion_sip_to_string(sip , body);
	g_free(body);
	if(send(ac->sk, res, strlen(res), 0) == -1) { g_free(res); return -1; }
	g_free(res);
	return 0;
}
#if 0

Contact* fetion_contact_handle_contact_request(User* user
		, const char* sipuri , const char* userid
		, const char* localname , int buddylist , int result)
{
	fetion_sip* sip = user->sip;
	SipHeader* eheader;
	char *res , *body;
	int ret;
	Contact* contact;
	fetion_sip_set_type(sip , SIP_SERVICE);
	eheader = fetion_sip_event_header_new(SIP_EVENT_HANDLECONTACTREQUEST);
	if(eheader == NULL){
		return NULL;
	}
	fetion_sip_add_header(sip , eheader);
	body = generate_handle_contact_request_body(sipuri , userid , localname , buddylist , result);
	if(body == NULL){
		return NULL;
	}
	res = fetion_sip_to_string(sip , body);
	free(body);
	if(res == NULL){
		return NULL;
	}
	tcp_connection_send(sip->tcp , res , strlen(res));
	free(res);
	res = fetion_sip_get_response(sip);
	if(res == NULL){
		return NULL;
	}
	ret = fetion_sip_get_code(res);
	switch(ret)
	{
		case 200 :
			contact = parse_handle_contact_request_response(res);
			free(res);
			if(contact == NULL){
				debug_info("handle contact request from (%s) failed" , userid);
				return NULL;
			}
			fetion_contact_list_append(user->contactList , contact);
			debug_info("handle contact request from (%s) success" , userid);
			return contact;
		default:
			free(res);
			debug_info("handle contact request from (%s) failed" , userid);
			return NULL;
	}
	return NULL;
}
#endif

static gchar *generate_subscribe_body(const gchar *version)
{
	xmlChar *buf;
	xmlDocPtr doc;
	xmlNodePtr node;
	gchar body[] = "<args></args>";
	doc = xmlParseMemory(body , strlen(body));
	node = xmlDocGetRootElement(doc);
	node = xmlNewChild(node , NULL , BAD_CAST "subscription" , NULL);
	xmlNewProp(node , BAD_CAST "self" , BAD_CAST "v4default;mail-count");
	xmlNewProp(node , BAD_CAST "buddy" , BAD_CAST "v4default");
	xmlNewProp(node , BAD_CAST "version" , BAD_CAST version);
	xmlDocDumpMemory(doc , &buf , NULL);
	xmlFreeDoc(doc);
	return xml_convert(buf);
}
static gchar *generate_contact_info_body(const gchar *userid)
{
	xmlChar *buf;
	xmlDocPtr doc;
	xmlNodePtr node;
	gchar body[] = "<args></args>";
	doc = xmlParseMemory(body , strlen(body));
	node = xmlDocGetRootElement(doc);
	node = xmlNewChild(node , NULL , BAD_CAST "contact" , NULL);
	xmlNewProp(node , BAD_CAST "user-id" , BAD_CAST userid);
	xmlDocDumpMemory(doc , &buf , NULL);
	xmlFreeDoc(doc);
	return xml_convert(buf);
	
}

static gchar *generate_set_displayname_body(const gchar *userid , const gchar *name)
{
	gchar args[] = "<args></args>";
	xmlChar *res;
	xmlDocPtr doc;
	xmlNodePtr node;
	doc = xmlParseMemory(args , strlen(args));
	node = xmlDocGetRootElement(doc);
	node = xmlNewChild(node , NULL , BAD_CAST "contacts" , NULL);
	node = xmlNewChild(node , NULL , BAD_CAST "contact" , NULL);
	xmlNewProp(node , BAD_CAST "user-id" , BAD_CAST userid);
	xmlNewProp(node , BAD_CAST "local-name" , BAD_CAST name);
	xmlDocDumpMemory(doc , &res , NULL);
	xmlFreeDoc(doc);
	return xml_convert(res);
}
gchar *generate_move_to_group_body(const gchar *userid, gint buddylist)
{
	gchar args[] = "<args></args>";
	gchar bl[5];
	xmlChar *res;
	xmlDocPtr doc;
	xmlNodePtr node;
	doc = xmlParseMemory(args , strlen(args));
	node = xmlDocGetRootElement(doc);
	node = xmlNewChild(node , NULL , BAD_CAST "contacts" , NULL);
	node = xmlNewChild(node , NULL , BAD_CAST "contact" , NULL);
	xmlNewProp(node , BAD_CAST "user-id" , BAD_CAST userid);
	sprintf(bl , "%d" , buddylist);
	xmlNewProp(node , BAD_CAST "buddy-lists" , BAD_CAST bl);
	xmlDocDumpMemory(doc , &res , NULL);
	xmlFreeDoc(doc);
	return xml_convert(res);
}
static gchar *generate_delete_buddy_body(const gchar *userid)
{
	gchar args[] = "<args></args>";
	xmlChar *res;
	xmlDocPtr doc;
	xmlNodePtr node;
	doc = xmlParseMemory(args , strlen(args));
	node = xmlDocGetRootElement(doc);
	node = xmlNewChild(node , NULL , BAD_CAST "contacts" , NULL);
	node = xmlNewChild(node , NULL , BAD_CAST "buddies" , NULL);
	node = xmlNewChild(node , NULL , BAD_CAST "buddy" , NULL);
	xmlNewProp(node , BAD_CAST "user-id" , BAD_CAST userid);
	xmlDocDumpMemory(doc , &res , NULL);
	xmlFreeDoc(doc);
	return xml_convert(res);
}
