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
#include "fx_types.h"
#include "fx_blist.h"

static gchar *generate_create_buddylist_body(const gchar *name);
static gchar *generate_delete_buddylist_body(gint id);
static gchar *generate_edit_buddylist_body(gint id , const gchar *name);

gint fetion_buddylist_create(User *user, const gchar *name)
{
	fetion_sip* sip = user->sip;
	SipHeader* eheader;
	gchar *res , *body;
	fetion_sip_set_type(sip , SIP_SERVICE);
	eheader = fetion_sip_event_header_new(SIP_EVENT_CREATEBUDDYLIST);
	fetion_sip_add_header(sip , eheader);
	body = generate_create_buddylist_body(name);
	res = fetion_sip_to_string(sip , body);
	g_free(body);

	g_free(res);

	return 0;
}
gint fetion_buddylist_delete(fetion_account *ac, gint id)
{
	fetion_sip *sip = ac->user->sip;
	SipHeader *eheader;
	gchar     *res , *body;

	fetion_sip_set_type(sip , SIP_SERVICE);
	eheader = fetion_sip_event_header_new(SIP_EVENT_DELETEBUDDYLIST);
	fetion_sip_add_header(sip , eheader);
	body = generate_delete_buddylist_body(id);
	res = fetion_sip_to_string(sip , body);
	g_free(body);
	if(send(ac->sk, res, strlen(res), 0) == -1) { g_free(res); return -1; }
	g_free(res);

	return 0;
}
gint fetion_buddylist_edit(fetion_account *ac, gint id, const gchar *name)
{
	fetion_sip *sip = ac->user->sip;
	SipHeader *eheader;
	gchar     *res, *body;

	fetion_sip_set_type(sip , SIP_SERVICE);
	eheader = fetion_sip_event_header_new(SIP_EVENT_SETBUDDYLISTINFO);
	fetion_sip_add_header(sip , eheader);
	body = generate_edit_buddylist_body(id , name);
	res = fetion_sip_to_string(sip , body);
	g_free(body);
	if(send(ac->sk, res, strlen(res), 0) == -1) { g_free(res); return -1; }
	g_free(res);

	return 0;
}

static gchar *generate_create_buddylist_body(const gchar *name)
{
	gchar args[] = "<args></args>";
	xmlChar *res;
	xmlDocPtr doc;
	xmlNodePtr node;
	doc = xmlParseMemory(args , strlen(args));
	node = xmlDocGetRootElement(doc);
	node = xmlNewChild(node , NULL , BAD_CAST "contacts" , NULL);
	node = xmlNewChild(node , NULL , BAD_CAST "buddy-lists" , NULL);
	node = xmlNewChild(node , NULL , BAD_CAST "buddy-list" , NULL);
	xmlNewProp(node , BAD_CAST "name" , BAD_CAST name);
	xmlDocDumpMemory(doc , &res , NULL);
	xmlFreeDoc(doc);
	return xml_convert(res);
}
static gchar *generate_edit_buddylist_body(gint id , const gchar *name)
{
	gchar args[] = "<args></args>";
	gchar ids[128];
	xmlChar *res;
	xmlDocPtr doc;
	xmlNodePtr node;
	doc = xmlParseMemory(args , strlen(args));
	node = xmlDocGetRootElement(doc);
	node = xmlNewChild(node , NULL , BAD_CAST "contacts" , NULL);
	node = xmlNewChild(node , NULL , BAD_CAST "buddy-lists" , NULL);
	node = xmlNewChild(node , NULL , BAD_CAST "buddy-list" , NULL);
	xmlNewProp(node , BAD_CAST "name" , BAD_CAST name);
	memset(ids, 0, sizeof(ids));
	snprintf(ids, sizeof(ids) - 1 , "%d" , id);
	xmlNewProp(node , BAD_CAST "id" , BAD_CAST ids);
	xmlDocDumpMemory(doc , &res , NULL);
	xmlFreeDoc(doc);
	return xml_convert(res);

}
static gchar *generate_delete_buddylist_body(gint id)
{
	gchar args[] = "<args></args>";
	gchar ida[4];
	memset(ida, 0, sizeof(ida));
	sprintf(ida , "%d" , id);
	xmlChar *res;
	xmlDocPtr doc;
	xmlNodePtr node;
	doc = xmlParseMemory(args , strlen(args));
	node = xmlDocGetRootElement(doc);
	node = xmlNewChild(node , NULL , BAD_CAST "contacts" , NULL);
	node = xmlNewChild(node , NULL , BAD_CAST "buddy-lists" , NULL);
	node = xmlNewChild(node , NULL , BAD_CAST "buddy-list" , NULL);
	xmlNewProp(node , BAD_CAST "id" , BAD_CAST ida);
	xmlDocDumpMemory(doc , &res , NULL);
	xmlFreeDoc(doc);
	return xml_convert(res);
}
