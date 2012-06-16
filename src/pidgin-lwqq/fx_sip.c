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

#define _XOPEN_SOURCE
#include "webqq.h"
#include "fx_sip.h"
#include "fx_types.h"

gint callid = 1;

fetion_sip* fetion_sip_new0(User *user)
{
	fetion_sip* sip = g_malloc0(sizeof(fetion_sip));
	strcpy(sip->from , user->sId);
	sip->sequence = 2;
	sip->header = NULL;
	user->sip = sip;
	return sip;
}

fetion_sip* fetion_sip_clone(fetion_sip* sip)
{
	fetion_sip* res = g_malloc0(sizeof(fetion_sip));
	memcpy(res, sip, sizeof(fetion_sip));
	sip->header = NULL;
	return res;
}

SipHeader* fetion_sip_header_new(const gchar *name , const gchar *value)
{
	SipHeader* header = g_malloc0(sizeof(SipHeader));
	strcpy(header->name , name);
	header->value = (gchar*)g_malloc0(strlen(value) + 1);
	strcpy(header->value , value);
	header->next = NULL;
	return header;
}

void fetion_sip_set_type(fetion_sip *sip, SipType type)
{
	sip->type = type;
	sip->callid = callid;
}

void fetion_sip_set_callid(fetion_sip *sip, gint callid)
{
	sip->callid = callid;
}

SipHeader *fetion_sip_authentication_header_new(const gchar *response)
{
	gint  len;
	gchar *res;
	gchar start[] = "Digest response=\"";
	gchar end[]   = "\",algorithm=\"SHA1-sess-v4\"";
	SipHeader* header;
	
	len = strlen(start) + strlen(end) + strlen(response) + 1;
	res = (gchar*)g_malloc0(len);
	sprintf(res, "%s%s%s" , start , response , end);
	header = (SipHeader*)malloc(sizeof(SipHeader));
	memset(header , 0 , sizeof(SipHeader));
	strcpy(header->name , "A");
	header->value = res;
	return header;
}

SipHeader* fetion_sip_ack_header_new(const gchar *code, const gchar *algorithm, const gchar *type, const gchar *guid)
{
	gchar ack[512];
	sprintf(ack , "Verify response=\"%s\",algorithm=\"%s\",type=\"%s\",chid=\"%s\""
			 	, code , algorithm , type , guid);
	return fetion_sip_header_new("A" , ack);
}

SipHeader *fetion_sip_event_header_new(gint eventType)
{
	gchar event[48];
	memset(event, 0, sizeof(event));
	switch(eventType) {
		case SIP_EVENT_PRESENCE :
			strcpy(event , "PresenceV4");
			break;
		case SIP_EVENT_SETPRESENCE :
			strcpy(event , "SetPresenceV4");
			break;
		case SIP_EVENT_CATMESSAGE :
			strcpy(event , "CatMsg");
			break;
		case SIP_EVENT_SENDCATMESSAGE :
			strcpy(event , "SendCatSMS");
			break;
		case SIP_EVENT_STARTCHAT :
			strcpy(event , "StartChat");
			break;
		case SIP_EVENT_GETCONTACTINFO :
			strcpy(event , "GetContactInfoV4");
			break;
		case SIP_EVENT_CONVERSATION :
			strcpy(event , "Conversation");
			break;
		case SIP_EVENT_INVITEBUDDY :
			strcpy(event , "InviteBuddy");
			break;
		case SIP_EVENT_CREATEBUDDYLIST :
			strcpy(event , "CreateBuddyList");
			break;
		case SIP_EVENT_DELETEBUDDYLIST :
			strcpy(event , "DeleteBuddyList");
			break;
		case SIP_EVENT_SETCONTACTINFO :
			strcpy(event , "SetContactInfoV4");
			break;
		case SIP_EVENT_SETUSERINFO :
			strcpy(event , "SetUserInfoV4");
			break;
		case SIP_EVENT_SETBUDDYLISTINFO :
			strcpy(event , "SetBuddyListInfo");
			break;
		case SIP_EVENT_DELETEBUDDY :
			strcpy(event , "DeleteBuddyV4");
			break;
		case SIP_EVENT_ADDBUDDY :
			strcpy(event , "AddBuddyV4");
			break;
		case SIP_EVENT_KEEPALIVE :
			strcpy(event , "KeepAlive");
			break;
		case SIP_EVENT_DIRECTSMS :
			strcpy(event , "DirectSMS");
			break;
		case SIP_EVENT_HANDLECONTACTREQUEST :
			strcpy(event , "HandleContactRequestV4");
			break;
		case SIP_EVENT_SENDDIRECTCATSMS :
			strcpy(event , "SendDirectCatSMS");
			break;
		case SIP_EVENT_PGGETGROUPLIST:
			strcpy(event , "PGGetGroupList");
			break;
		case SIP_EVENT_PGGETGROUPINFO:
			strcpy(event , "PGGetGroupInfo");
			break;
		case SIP_EVENT_PGPRESENCE:
			strcpy(event , "PGPresence");
			break;
		case SIP_EVENT_PGGETGROUPMEMBERS:
			strcpy(event , "PGGetGroupMembers");
			break;
		case SIP_EVENT_PGSENDCATSMS:
			strcpy(event , "PGSendCatSMS");
			break;
		default:
			break;
	}
	return fetion_sip_header_new("N" , event);
}

SipHeader* fetion_sip_credential_header_new(const gchar *credential)
{
	gchar value[64];
	memset(value , 0, sizeof(value));
	sprintf(value , "TICKS auth=\"%s\"" , credential);
	return fetion_sip_header_new("A" , value);
}

void fetion_sip_add_header(fetion_sip *sip, SipHeader *header)
{
	SipHeader* pos = sip->header;
	if(!pos) {
		sip->header = header;
		return;
	}

	while(pos) {
		if(!pos->next) {
			pos->next = header;
			break;
		}
		pos = pos->next;
	}
}

char* fetion_sip_to_string(fetion_sip* sip , const char* body)
{
	char *res , *head , buf[1024] , type[128];
	SipHeader *pos , *tmp;
	int len = 0;

	pos = sip->header;
	while(pos){
		len += (strlen(pos->value) + strlen(pos->name) + 5);
		pos = pos->next;
	}
	len += (body == NULL ? 100 : strlen(body) + 100 );
	res = (gchar*)g_malloc0(len + 1);
	memset(type, 0 , sizeof(type));
	switch(sip->type){
		case SIP_REGISTER     : strcpy(type , "R");		break;
		case SIP_SUBSCRIPTION :	strcpy(type , "SUB");	break;
		case SIP_SERVICE 	  : strcpy(type , "S");		break;
		case SIP_MESSAGE      : strcpy(type , "M");		break;
		case SIP_INCOMING	  : strcpy(type , "IN");	break;
		case SIP_OPTION 	  : strcpy(type , "O");		break;
		case SIP_INVITATION	  : strcpy(type , "I");		break;
		case SIP_ACKNOWLEDGE  : strcpy(type , "A");		break;
		default:	break;
	};

	if(*type == '\0'){
		g_free(res);
		return NULL;
	}

	sprintf(buf, "%s fetion.com.cn SIP-C/4.0\r\n"
			"F: %s\r\n"
			"I: %d\r\n"
			"Q: 2 %s\r\n",
			type,
			sip->from,
			sip->callid,
			type);

	strcat(res , buf);

	pos = sip->header;
	while(pos){
		len = strlen(pos->value) + strlen(pos->name) + 5;
		head = (gchar*)g_malloc0(len);
		sprintf(head, "%s: %s\r\n", pos->name, pos->value);
		strcat(res , head);
		tmp = pos;
		pos = pos->next;
		g_free(head);
		g_free(tmp->value);
		g_free(tmp);
	}
	if(body){
		sprintf(buf, "L: %d\r\n\r\n", strlen(body));
		strcat(res, buf);
		strcat(res, body);
	}else{
		strcat(res, "\r\n");
	}
	callid ++;
	sip->header = NULL;
	return res;
}
void fetion_sip_free(fetion_sip *sip)
{
	g_free(sip);
}

gchar *fetion_sip_get_sid_by_sipuri(const gchar *sipuri)
{
	gchar *res, *pos;
	gint n;
	pos = strstr(sipuri, ":") + 1;
	n = strlen(pos) - (strstr(pos , "@") == 0 ? 0 : strlen(strstr(pos , "@"))) ;
	res = (gchar*)g_malloc0(n + 1);
	strncpy(res, pos, n);
	return res;
}

gint fetion_sip_get_attr(const gchar *sip, const gchar *name, gchar *result)
{
	gchar m_name[16];
	gchar *pos;
	gint  n;

	sprintf(m_name, "%s: ", name);
	if(!strstr(sip, m_name)) return -1;
	pos = strstr(sip , m_name) + strlen(m_name);

	if(!strstr(pos , "\r\n")) n = strlen(pos);
	else n = strlen(pos) - strlen(strstr(pos, "\r\n"));
	strncpy(result , pos , n);
	result[n] = '\0';
	return 0;
}

gint fetion_sip_get_length(const gchar *sip)
{
	gchar res[6];
	gchar name[] = "L";
	if(fetion_sip_get_attr(sip , name , res) == -1)
		return 0;
	return atoi(res);
}

gint fetion_sip_get_code(const gchar *sip)
{
	gchar *pos , res[32];
	gint n;

	memset(res, 0, sizeof(res));
	if(strstr(sip , "4.0 ") == NULL)
	    return 400;
	pos = strstr(sip , "4.0 ") + 4;
	if(strstr(pos , " ") == NULL)
	    return 400;
	n = strlen(pos) - strlen(strstr(pos , " "));
	strncpy(res , pos , n);
	return atoi(res);
}

gint fetion_sip_get_type(const gchar *sip)
{
	gchar res[128];
	gint n;

	if(!strstr(sip, " "))
		return SIP_UNKNOWN;

	n = strlen(sip) - strlen(strstr(sip , " "));
	memset(res, 0, sizeof(res));
	strncpy(res , sip , n);
	if(strcmp(res , "I") == 0 )
		return SIP_INVITATION;
	if(strcmp(res , "M") == 0 )
		return SIP_MESSAGE;
	if(strcmp(res , "BN") == 0)
		return SIP_NOTIFICATION;
	if(strcmp(res , "SIP-C/4.0") == 0
		|| strcmp(res , "SIP-C/2.0") == 0)
		return SIP_SIPC_4_0;
	if(strcmp(res , "IN") == 0)
		return SIP_INCOMING;
	if(strcmp(res , "O") == 0 )
		return SIP_OPTION;
	return SIP_UNKNOWN;
		
}

void fetion_sip_parse_notification(const gchar *sip, gint *type, gint *event, gchar **xml)
{
	gchar type1[16] , *pos;
	xmlChar *event1;
	xmlDocPtr doc;
	xmlNodePtr node;
	fetion_sip_get_attr(sip , "N" , type1);
	if(strcmp(type1 , "PresenceV4") == 0)
		*type = NOTIFICATION_TYPE_PRESENCE;
	else if(strcmp(type1 , "Conversation") == 0)
		*type = NOTIFICATION_TYPE_CONVERSATION;
	else if(strcmp(type1 , "contact") == 0)
		*type = NOTIFICATION_TYPE_CONTACT;
	else if(strcmp(type1 , "registration") == 0)
		*type = NOTIFICATION_TYPE_REGISTRATION;
	else if(strcmp(type1 , "SyncUserInfoV4") == 0)
		*type = NOTIFICATION_TYPE_SYNCUSERINFO;
	else if(strcmp(type1 , "PGGroup") == 0)
	    	*type = NOTIFICATION_TYPE_PGGROUP;
	else
		*type = NOTIFICATION_TYPE_UNKNOWN;

	if(!(pos = strstr(sip , "\r\n\r\n"))) {
		*event = NOTIFICATION_TYPE_UNKNOWN;
		return;
	}
	*xml = (gchar*)g_malloc0(strlen(pos) + 1);
	strcpy(*xml , pos + 4);
	doc = xmlParseMemory(*xml , strlen(*xml));
	node = xmlDocGetRootElement(doc);
	node = xml_goto_node(node , "event");
	event1 = xmlGetProp(node ,  BAD_CAST "type");
	if(xmlStrcmp(event1, BAD_CAST "Support") == 0) {
		xmlFree(event1);
		node = node->next;
		event1 = xmlGetProp(node, BAD_CAST "type");
		if(xmlStrcmp(event1, BAD_CAST "UserEntered") == 0) {
			*event = NOTIFICATION_EVENT_USERENTER;
			xmlFree(event1);
			xmlFreeDoc(doc);
			return;
		}
		*event = NOTIFICATION_EVENT_UNKNOWN;
		xmlFree(event1);
		xmlFreeDoc(doc);
		return;
	}
	if(xmlStrcmp(event1 , BAD_CAST "PresenceChanged") == 0)
		*event = NOTIFICATION_EVENT_PRESENCECHANGED;
	else if(xmlStrcmp(event1 , BAD_CAST "UserEntered") == 0)
		*event = NOTIFICATION_EVENT_USERENTER;
	else if(xmlStrcmp(event1 , BAD_CAST "UserLeft") == 0)
		*event = NOTIFICATION_EVENT_USERLEFT;
	else if(xmlStrcmp(event1 , BAD_CAST "deregistered") == 0)
		*event = NOTIFICATION_EVENT_DEREGISTRATION;
	else if(xmlStrcmp(event1 , BAD_CAST "SyncUserInfo") == 0)
		*event = NOTIFICATION_EVENT_SYNCUSERINFO;
	else if(xmlStrcmp(event1 , BAD_CAST "AddBuddyApplication") == 0)
		*event = NOTIFICATION_EVENT_ADDBUDDYAPPLICATION;
	else if(xmlStrcmp(event1 , BAD_CAST "PGGetGroupInfo") == 0)
	    	*event = NOTIFICATION_EVENT_PGGETGROUPINFO;
	else
		*event = NOTIFICATION_EVENT_UNKNOWN;
	xmlFree(event1);
	xmlFreeDoc(doc);
}

gint fetion_sip_parse_info(const gchar *sipmsg, InfoType *type)
{
	gchar     *pos = NULL;
	xmlDocPtr  doc = NULL;
	xmlNodePtr node = NULL;
	xmlChar   *res = NULL;

	*type = INFO_UNKNOWN;
	if(!(pos = strstr(sipmsg, "\r\n\r\n"))) return -1;
	doc = xmlParseMemory(pos + 4 , strlen(pos + 4));
	node = xmlDocGetRootElement(doc);
	node = node->xmlChildrenNode;
	res = xmlNodeGetContent(node);
	if(xmlStrcmp(res , BAD_CAST "nudge") == 0)
		*type = INFO_NUDGE;
	xmlFree(res);
	xmlFreeDoc(doc);
	return 0;
}

void fetion_sip_parse_userleft(const gchar *sipmsg, gchar **sipuri)
{
	gchar *pos = NULL;
	xmlDocPtr doc = NULL;
	xmlNodePtr node = NULL;
	xmlChar *res;

	printf("%s\n", sipmsg);

	pos = strstr(sipmsg , "\r\n\r\n") + 4;
	doc = xmlParseMemory(pos , strlen(pos));
	node = xmlDocGetRootElement(doc);
	node = xml_goto_node(node , "member");
	res = xmlGetProp(node , BAD_CAST "uri");
	*sipuri = (gchar*)malloc(xmlStrlen(res) + 1);
	memset(*sipuri, 0, xmlStrlen(res) + 1);
	strcpy(*sipuri , (gchar*)res);
	xmlFreeDoc(doc);
}

gint fetion_sip_parse_sipc(const gchar *sipmsg, gint *callid, gchar **xml)
{
	gchar callid_str[16];
	gchar *pos;
	gint n;
	gchar code[16];

	pos = strstr(sipmsg , " ") + 1;
	n = strlen(pos) - strlen(strstr(pos , " "));
	strncpy(code , pos , n);
	
	fetion_sip_get_attr(sipmsg , "I" , callid_str);
	*callid = atoi(callid_str);
	
	if(!(pos = strstr(sipmsg , "\r\n\r\n"))) { *xml = (gchar*)0; return -1;}

	*xml = (gchar*)malloc(strlen(pos + 4) + 1);
	memset(*xml , 0 , strlen(pos + 4) + 1);
	strcpy(*xml , pos);

	return atoi(code);
}

void fetion_sip_get_auth_attr(const gchar *auth, gchar **ipaddress, gint *port, gchar **credential)
{
	gchar *pos = strstr(auth , "address=\"") + 9;
	gint   n = strlen(pos) - strlen(strstr(pos , ":"));
	gchar  port_str[6] = { 0 };
	*credential = (gchar*)malloc(256);
	memset(*credential , 0 , 256);
	*ipaddress = (gchar*)malloc(256);
	memset(*ipaddress , 0 , 256);
	strncpy(*ipaddress , pos , n);
	pos = strstr(pos , ":") + 1;
	n = strlen(pos) - strlen(strstr(pos , ";"));
	strncpy(port_str , pos , n);
	*port = atoi(port_str);
	pos = strstr(pos , "credential=\"") + 12;
	strncpy(*credential , pos , strlen(pos) - 1);
}
