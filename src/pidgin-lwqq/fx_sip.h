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

#ifndef FETION_SIP_H
#define FETION_SIP_H
#define SIP_BUFFER_SIZE 2048

#include "fx_types.h"

typedef enum {
	SIP_REGISTER = 1 ,
	SIP_SERVICE ,
 	SIP_SUBSCRIPTION , 
	SIP_NOTIFICATION ,
	SIP_INVITATION , 
	SIP_INCOMING , 
	SIP_OPTION , 
	SIP_MESSAGE ,
	SIP_SIPC_4_0 ,
	SIP_ACKNOWLEDGE ,
	SIP_UNKNOWN
} SipType;

typedef enum {
	NOTIFICATION_TYPE_PRESENCE ,
	NOTIFICATION_TYPE_CONTACT ,
	NOTIFICATION_TYPE_CONVERSATION ,
	NOTIFICATION_TYPE_REGISTRATION ,
	NOTIFICATION_TYPE_SYNCUSERINFO ,
	NOTIFICATION_TYPE_PGGROUP ,
	NOTIFICATION_TYPE_UNKNOWN
} NotificationType;

typedef enum {
	NOTIFICATION_EVENT_PRESENCECHANGED ,
	NOTIFICATION_EVENT_ADDBUDDYAPPLICATION ,
	NOTIFICATION_EVENT_USERENTER ,
	NOTIFICATION_EVENT_USERLEFT ,
	NOTIFICATION_EVENT_DEREGISTRATION , 
	NOTIFICATION_EVENT_SYNCUSERINFO ,
	NOTIFICATION_EVENT_PGGETGROUPINFO , 
	NOTIFICATION_EVENT_UNKNOWN
} NotificationEvent;

typedef enum {
	SIP_EVENT_PRESENCE = 0,
	SIP_EVENT_SETPRESENCE ,
	SIP_EVENT_CONTACT ,
	SIP_EVENT_CONVERSATION ,
	SIP_EVENT_CATMESSAGE ,
	SIP_EVENT_SENDCATMESSAGE ,
	SIP_EVENT_STARTCHAT ,
	SIP_EVENT_INVITEBUDDY ,
	SIP_EVENT_GETCONTACTINFO ,
	SIP_EVENT_CREATEBUDDYLIST ,
	SIP_EVENT_DELETEBUDDYLIST ,
	SIP_EVENT_SETCONTACTINFO ,
	SIP_EVENT_SETUSERINFO ,
	SIP_EVENT_SETBUDDYLISTINFO ,
	SIP_EVENT_DELETEBUDDY ,
	SIP_EVENT_ADDBUDDY ,
	SIP_EVENT_KEEPALIVE ,
	SIP_EVENT_DIRECTSMS ,
	SIP_EVENT_SENDDIRECTCATSMS ,
	SIP_EVENT_HANDLECONTACTREQUEST ,
	SIP_EVENT_PGGETGROUPLIST ,
	SIP_EVENT_PGGETGROUPINFO , 
	SIP_EVENT_PGGETGROUPMEMBERS ,
	SIP_EVENT_PGSENDCATSMS , 
	SIP_EVENT_PGPRESENCE
} SipEvent;

typedef enum {
	INFO_NUDGE,
	INFO_UNKNOWN
} InfoType;

typedef enum {
	INFO_ACTION_ACCEPT,
	INFO_ACTION_CANCEL, 
	INFO_ACTION_UNKNOWN
} InfoActionType;

fetion_sip* fetion_sip_new0(User *user);
fetion_sip* fetion_sip_clone(fetion_sip* sip);
SipHeader* fetion_sip_header_new(const gchar *name , const gchar *value);
void fetion_sip_set_type(fetion_sip *sip, SipType type);
void fetion_sip_set_callid(fetion_sip *sip, gint callid);
SipHeader* fetion_sip_authentication_header_new(const gchar *response);
SipHeader* fetion_sip_ack_header_new(const gchar *code, const gchar *algorithm, const gchar *type, const gchar *guid);
SipHeader *fetion_sip_event_header_new(gint eventType);
SipHeader* fetion_sip_credential_header_new(const gchar *credential);
void fetion_sip_add_header(fetion_sip *sip, SipHeader *header);
gchar *fetion_sip_to_string(fetion_sip *sip, const gchar *body);
void fetion_sip_free(fetion_sip *sip);
gchar *fetion_sip_get_sid_by_sipuri(const gchar *sipuri);
gint fetion_sip_get_attr(const gchar *sip, const gchar *name, gchar *result);
gint fetion_sip_get_length(const gchar* sipmsg);
gint fetion_sip_get_code(const gchar *sip);
gint fetion_sip_get_type(const gchar *sip);
void fetion_sip_get_auth_attr(const gchar *auth, gchar **ipaddress, gint *port, gchar **credential);
void fetion_sip_parse_notification(const gchar *sip, gint *type, gint *event, gchar **xml);
gint fetion_sip_parse_info(const gchar *sipmsg, InfoType *type);
void fetion_sip_parse_userleft(const gchar *sipmsg, gchar **sipuri);
gint fetion_sip_parse_sipc(const gchar *sipmsg, gint *callid, gchar **xml);

#endif
