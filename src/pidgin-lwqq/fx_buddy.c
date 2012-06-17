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
#include "fx_user.h"
#include "fx_sip.h"
#include "fx_buddy.h"
#include "fx_contact.h"
#include "info.h"

static gchar* http_connection_encode_url(const gchar* url);
static gint http_connection_get_body_length(const gchar *http);
static void update_portrait(fetion_account *ac, Contact *contact_cur);
static gint download_portrait_cb(gpointer data, gint source, const gchar *error_message);
static gint get_portrait_cb(gpointer data, gint source, const gchar *error_message);
static gchar *generate_add_buddy_body(const gchar *no,
	   	gint notype, gint buddylist, const gchar *localname , const gchar *desc);
static Contact *parse_add_buddy_response(const gchar *sipmsg, gint *status_code, gchar **errMsg);
static Contact *parse_syncinfo(Contact *clist, const gchar *sipmsg);
static Contact *parse_addbuddyapplication(const gchar *sipmsg);
static gchar *generate_handle_contact_request_body(const gchar *sipuri
		, const gchar *userid, const gchar *localname
		, gint buddylist, gint result );

static Group *fx_group_find_by_id(Group *group_list, gint id)
{
	Group *group_cur;
	foreach_contactlist(group_list, group_cur)
		if(group_cur->groupid == id) return group_cur;

	return (Group*)0;
}

/*void fx_blist_init(fetion_account *ac)
{
	Contact *contact_cur;
	Group   *group_cur;
	User    *user = ac->user;
	PurpleAccount *account = ac->account;
	PurpleGroup   *group = NULL;
	PurpleBuddy   *buddy = NULL;

	foreach_contactlist(user->groupList, group_cur) {
		if(!(group = purple_find_group(group_cur->groupname)))
			group = purple_group_new(group_cur->groupname);
	}

	foreach_contactlist(user->contactList, contact_cur) {

		if(!(buddy = purple_find_buddy(account, contact_cur->userId)))
			buddy = purple_buddy_new(account, contact_cur->userId, contact_cur->nickname);

		group_cur = fx_group_find_by_id(user->groupList, contact_cur->groupid);
		if(!(group = purple_find_group(group_cur->groupname)))
			group = purple_group_new(group_cur->groupname);

		purple_blist_add_buddy(buddy, NULL, group, NULL);

		if(contact_cur->localname[0] != '\0')
			purple_blist_alias_buddy(buddy, contact_cur->localname);

		purple_blist_alias_buddy(buddy, contact_cur->localname);
		purple_prpl_got_user_status(account, contact_cur->userId, "Offline", NULL);
	}
}*/
/***HERE***/
void fx_blist_init(fetion_account *ac)
{
    LwqqClient* lc;
    LwqqErrorCode err=0;
	PurpleConnection *pc;
	PurpleAccount *account = ac->account;
	PurpleGroup   *group = NULL;
	PurpleBuddy   *buddy = NULL;

    lc=ac->qq;

	/*foreach_contactlist(user->groupList, group_cur) {
		if(!(group = purple_find_group(group_cur->groupname)))
			group = purple_group_new(group_cur->groupname);
	}*/
    group = purple_group_new("临时");

    //int id=0;
    if (err == LWQQ_EC_OK) {
        LwqqBuddy *bu;
        LIST_FOREACH(bu, &lc->friends, entries) {
            //lwqq_info_get_friend_detail_info(lc,bu,&err);

            if(!(buddy = purple_find_buddy(account, bu->qqnumber)))
                buddy = purple_buddy_new(account,bu->qqnumber,bu->nick);

            purple_blist_add_buddy(buddy,NULL,group,NULL);
            purple_prpl_got_user_status(account, bu->qqnumber, "Online", NULL);
        }
    }
    //pc = purple_account_get_connection(ac->account);

	//purple_connection_set_display_name(pc, "hi");

	/*foreach_contactlist(user->contactList, contact_cur) {

		if(!(buddy = purple_find_buddy(account, contact_cur->userId)))
			buddy = purple_buddy_new(account, contact_cur->userId, contact_cur->nickname);

		group_cur = fx_group_find_by_id(user->groupList, contact_cur->groupid);
		if(!(group = purple_find_group(group_cur->groupname)))
			group = purple_group_new(group_cur->groupname);

		purple_blist_add_buddy(buddy, NULL, group, NULL);

		if(contact_cur->localname[0] != '\0')
			purple_blist_alias_buddy(buddy, contact_cur->localname);

		purple_blist_alias_buddy(buddy, contact_cur->localname);
		purple_prpl_got_user_status(account, contact_cur->userId, "Offline", NULL);
	}*/
}

void process_presence(fetion_account *ac, const gchar *xml)
{
	Contact *contact_list;
	Contact *cnt;
	User    *user = ac->user;
	Group   *group_cur;
	const gchar  *state = NULL;
	const gchar  *name;
	PurpleBuddy  *buddy;
	PurpleGroup  *group;
	PurpleAccount *account = ac->account;
	gchar         alias[BUFLEN], status[BUFLEN];
	gchar        *sid;

	contact_list = fetion_user_parse_presence_body(xml , user);
	foreach_contactlist(contact_list, cnt) {

		if(!(buddy = purple_find_buddy(account, cnt->userId))) {
			buddy = purple_buddy_new(account, cnt->userId, cnt->localname);
			group_cur = fx_group_find_by_id(user->groupList, cnt->groupid);
			group = purple_find_group(group_cur->groupname);
			purple_blist_add_buddy(buddy, NULL, group, NULL);
		}

		if(cnt->localname[0] == '\0')
			purple_blist_alias_buddy(buddy, cnt->nickname);
		//state = get_status_id(cnt->state);
        state = "Online";
		snprintf(alias, sizeof(alias) - 1, "%s", cnt->localname[0] == '\0' ? cnt->nickname : cnt->localname);
		purple_blist_server_alias_buddy(buddy, alias);
		name = cnt->localname[0] == '\0'? cnt->nickname : cnt->localname;

		*status = '\0';
		if(cnt->relationStatus == RELATION_STATUS_UNAUTHENTICATED){
			snprintf(status, sizeof(status) - 1, "%s", _("[Unverified]"));
		}else if(cnt->serviceStatus == BASIC_SERVICE_ABNORMAL){
			if(cnt->carrierStatus == CARRIER_STATUS_CLOSED){
				snprintf(status, sizeof(status) - 1, "%s", _("[Has shut fetion service]"));
			}else{
				if(cnt->carrier[0] != '\0'){
					snprintf(status , sizeof(status) - 1, "%s",
								 _("[Online with SMS]"));
					if(cnt->mobileno[0] == '\0')
						snprintf(status , sizeof(status) - 1, "%s",
									 _("[Has shut fetion service]"));
				}else
					snprintf(status, sizeof(status) - 1, "%s",
								 	_("[Has shut fetion service]"));
				
			}
		}else if(cnt->carrierStatus == CARRIER_STATUS_DOWN){
			if(cnt->carrier[0] != '\0'){
				snprintf(status, sizeof(status) - 1, "%s",
							   	_("[Out of service]"));
			}
		}

		sid = fetion_sip_get_sid_by_sipuri(cnt->sipuri);
		snprintf(alias, sizeof(alias) - 1, "%s%s", name, status);
		purple_blist_alias_buddy(buddy, *alias == '\0' ? sid : alias);
		purple_prpl_got_user_status(account, cnt->userId, state,
			   			"impresa", cnt->impression,
						"fetionno", sid,
					 	"mobileno", cnt->mobileno[0] == '\0' ? _("Unexposed") : cnt->mobileno,
						NULL);
		g_free(sid); sid = (gchar*)0;
		
		update_portrait(ac, cnt);
	}
}

void process_sync_info(fetion_account *ac, const gchar *sipmsg)
{
	Contact *cnt;
	gchar    buf[BUFLEN];
	PurpleBuddy *buddy, *new_buddy;
	PurpleGroup *group;
	Group *grp;

	if(!(cnt = parse_syncinfo(ac->user->contactList, sipmsg))) return;
	if(cnt->relationStatus == 1) {
		snprintf(buf, sizeof(buf) - 1, _("'%s' has accepted your add-buddy request"), cnt->localname);
		purple_notify_info(ac->gc, NULL, _("Success"), buf);
		if(!(buddy = purple_find_buddy(ac->account, cnt->userId))) {
			grp = fetion_group_list_find_by_id(ac->user->groupList, cnt->groupid);
			if(!(group = purple_find_group(grp->groupname))) return;
			new_buddy = purple_buddy_new(ac->account, cnt->userId, cnt->localname);
			purple_blist_add_buddy(new_buddy, NULL, group, NULL);
			/*TODO alias buddy */ 
		}
	} else {
		if((buddy = purple_find_buddy(ac->account, cnt->userId)))
			purple_blist_remove_buddy(buddy);
		snprintf(buf, sizeof(buf) - 1, _("'%s' has declined your add-buddy request"), cnt->localname);
		purple_notify_error(ac->gc, NULL, _("Failed"), buf);
	}
}

void process_add_buddy(fetion_account *ac, const gchar *sipmsg)
{
	Contact *cnt;
	extern GSList *buddy_to_added;

	cnt = parse_addbuddyapplication(sipmsg);
	//purple_account_request_add(ac->account, sid, (gchar*)0, desc, _("\nI want to add you as a friend"));
	buddy_to_added = g_slist_append(buddy_to_added, cnt);
	purple_blist_request_add_buddy(ac->account,
		   					       cnt->userId,
								   ac->user->groupList->next->groupname,
								   cnt->localname);
}

gint get_info_cb(fetion_account *ac, const gchar *sipmsg, struct transaction *UNUSED(trans))
{
	gchar      *pos, *cur;
	xmlDocPtr   doc;
	xmlNodePtr  node;
	xmlChar    *cs;
	Contact    *cnt;
	PurpleNotifyUserInfo *info;
	PurpleConnection     *pc;
	gchar *province, *city, *sid;

	pos = strstr(sipmsg , "\r\n\r\n") + 4;
	doc = xmlParseMemory(pos , strlen(pos));
	if(!doc) return -1;
	node = xmlDocGetRootElement(doc);
	node = node->xmlChildrenNode;
	if(!xmlHasProp(node, BAD_CAST "user-id")) return -1;
	cs = xmlGetProp(node, BAD_CAST "user-id");
	if(!(cnt = fetion_contact_list_find_by_userid(ac->user->contactList, (gchar*)cs))) return -1;
	if(xmlHasProp(node , BAD_CAST "carrier-region")){
		cs = xmlGetProp(node , BAD_CAST "carrier-region");
		pos = (char*)cs;

		for(cur = cnt->country;*pos && *pos != '.';*cur ++ = *pos ++);
		*cur = '\0'; pos ++;
		for(cur = cnt->province;*pos && *pos != '.';*cur ++ = *pos ++);
		*cur = '\0'; pos ++;
		for(cur = cnt->city;*pos && *pos != '.';*cur ++ = *pos ++);
		*cur = '\0';
		xmlFree(cs);
	}

	info = purple_notify_user_info_new();
	purple_notify_user_info_add_pair(info, _("Nickname"), cnt->nickname);
	purple_notify_user_info_add_pair(info, _("Gender"), 
						cnt->gender == 1 ? _("Male") : ( cnt->gender == 2 ? _("Female") : _("Secrecy")));
	purple_notify_user_info_add_pair(info, _("Mobile"), cnt->mobileno);
	purple_notify_user_info_add_section_break(info);
	sid = fetion_sip_get_sid_by_sipuri(cnt->sipuri);
	purple_notify_user_info_add_pair(info, _("Fetion"), sid);
	purple_notify_user_info_add_pair(info, _("Signature"), cnt->impression);
	province = get_province_name(cnt->province);
	city = get_city_name(cnt->province, cnt->city);
	purple_notify_user_info_add_pair(info, _("Province"), province);
	purple_notify_user_info_add_pair(info, _("City"), city);
	purple_notify_user_info_add_pair(info, _("Service Provider"), cnt->carrier);

	pc = purple_account_get_connection(ac->account);
	purple_notify_userinfo(pc, cnt->userId, info, NULL, NULL);
	purple_notify_user_info_destroy(info);

	g_free(province); g_free(city); g_free(sid);

	return 0;
}

static gint add_buddy_cb(fetion_account *ses, const gchar *sipmsg, struct transaction *UNUSED(trans))
{
	gint code, status_code;
	gchar   *errMsg, errBuf[BUFLEN], nameBuf[BUFLEN];
	Contact *cnt;
	Group   *blist;
	PurpleBuddy *buddy;
	PurpleGroup *grp;
	code = fetion_sip_get_code(sipmsg);

	if(code == 200) {
		if(!(cnt = parse_add_buddy_response(sipmsg, &status_code, &errMsg))) {
			purple_notify_error(ses->gc, NULL, _("Error"), _("Add buddy error.Unknown reason"));
		   	return -1;
		}
		if(status_code != 200) {
			snprintf(errBuf, sizeof(errBuf) - 1, _("Add buddy error.%s."), errMsg ? errMsg : "Unknown reason");
			if(errMsg) g_free(errMsg);
			purple_notify_error(ses->gc, NULL, _("Error"), errBuf);
			g_free(cnt);
		   	return -1;
		}
		if(!(blist = fetion_group_list_find_by_id(ses->user->groupList, cnt->groupid))) {
			purple_notify_error(ses->gc, NULL, _("Error"), _("Add buddy error.Unknown reason"));
			g_free(cnt);
			return -1;
		}

		if(!(grp = purple_find_group(blist->groupname))) {
			purple_notify_error(ses->gc, NULL, _("Error"), _("Add buddy error.Unknown reason"));
			g_free(cnt);
			return -1;
		}
		fetion_contact_list_append(ses->user->contactList, cnt);
		buddy = purple_buddy_new(ses->account, cnt->userId, (gchar*)0);
		purple_buddy_set_protocol_data(buddy, NULL);
		purple_blist_add_buddy(buddy, NULL, grp, NULL);
		snprintf(nameBuf, sizeof(nameBuf) - 1, "%s[Unverified]", cnt->localname);
		purple_blist_alias_buddy(buddy, nameBuf);
		purple_blist_server_alias_buddy(buddy, nameBuf);
		purple_prpl_got_user_status(ses->account, cnt->userId, "Offline", NULL);
	} else if(code == 421 || code == 420) {
		purple_notify_error(ses->gc, NULL, _("Error"), _("Add buddy error.Unknown reason"));
		return -1;
	} else {
		purple_notify_error(ses->gc, NULL, _("Error"), _("Add buddy error.Unknown reason"));
		return -1;
	}

	return 0;
}

static gint handle_contact_cb(fetion_account *UNUSED(ac), const gchar *sipmsg, struct transaction *UNUSED(trans))
{
	purple_debug_info("fetion", "%s", sipmsg);
	return 0;
}

void fx_add_buddy(PurpleConnection *gc, PurpleBuddy *buddy,	PurpleGroup *group)
{
	fetion_account *ac = purple_connection_get_protocol_data(gc);
	const gchar *buddy_name;
	const gchar *group_name;
	const gchar *alias;
	gchar     errMsg[BUFLEN];
	Group     *grp;
	User      *user = ac->user;
	fetion_sip *sip = user->sip;
	SipHeader *eheader, *ackheader;
	gchar     *res, *body;
	extern GSList *buddy_to_added;
	struct transaction *trans;
	alias = purple_buddy_get_alias(buddy);
	buddy_name = purple_buddy_get_name(buddy);
	group_name = purple_group_get_name(group);

	/* process add buddy request */
	Contact *cnt;
	GSList  *cur = buddy_to_added;
	while(cur) {
		cnt = (Contact*)(cur->data);
		if(strcmp(cnt->userId, buddy_name) == 0) {
			if(!(grp = fetion_group_list_find_by_name(ac->user->groupList, group_name))) {
				purple_notify_error(gc, NULL, _("Error"), _("Not a valid group"));
				purple_blist_remove_buddy(buddy);
				return;
			}

			fetion_sip_set_type(sip , SIP_SERVICE);
			eheader = fetion_sip_event_header_new(SIP_EVENT_HANDLECONTACTREQUEST);
			fetion_sip_add_header(sip , eheader);
			trans = transaction_new();
			transaction_set_callid(trans, sip->callid);
			transaction_set_callback(trans, handle_contact_cb);
			transaction_add(ac, trans);
			body = generate_handle_contact_request_body(cnt->sipuri,
				   		cnt->userId, cnt->localname, grp->groupid, 1);
			res = fetion_sip_to_string(sip , body);
			if(send(ac->sk, res, strlen(res), 0 ) == -1) {
				purple_notify_error(gc, NULL, _("Error"), _("Network Error!"));
				purple_blist_remove_buddy(buddy);
	   			return;
			}
			g_free(body);
			purple_prpl_got_user_status(ac->account, cnt->userId, "Offline", NULL);
			buddy_to_added = g_slist_remove(buddy_to_added, cnt);
			return;
		}
	}

	/* remove the buddy just added */
	purple_blist_remove_buddy(buddy);

	/* add a new buddy */
	if(strlen(buddy_name) > 11) return;
	
	if(!(grp = fetion_group_list_find_by_name(ac->user->groupList, group_name))) {
		snprintf(errMsg, sizeof(errMsg) - 1, _("'%s' is not a valid group\n"), group_name);
		purple_notify_error(gc, NULL, _("Error"), errMsg);
		return;
	}

	fetion_sip_set_type(sip , SIP_SERVICE);
	eheader = fetion_sip_event_header_new(SIP_EVENT_ADDBUDDY);
	trans = transaction_new();
	transaction_set_userid(trans, buddy_name);
	transaction_set_callid(trans, sip->callid);
	transaction_set_callback(trans, add_buddy_cb);
	transaction_add(ac, trans);
	fetion_sip_add_header(sip , eheader);
	if(user->verification != NULL && user->verification->algorithm != NULL)	{
		ackheader = fetion_sip_ack_header_new(user->verification->code
											, user->verification->algorithm
											, user->verification->type
											, user->verification->guid);
		fetion_sip_add_header(sip , ackheader);
	}
	body = generate_add_buddy_body(buddy_name, strlen(buddy_name) == 11 ? MOBILE_NO : FETION_NO,
				 grp->groupid, g_strdup(alias), g_strdup(user->nickname));
	purple_prpl_got_user_status(ac->account, buddy_name, "Offline", NULL);
	res = fetion_sip_to_string(sip , body);
	printf("%s\n", res);
	g_free(body);
	if(send(ac->sk, res, strlen(res), 0) == -1) { g_free(res); return; }
	g_free(res);
}


static void update_portrait(fetion_account *ac, Contact *contact_cur)
{
	const gchar *crc;
	PurpleAccount *account = ac->account;
	PurpleBuddy   *buddy;
	portrait_data *data;

	g_return_if_fail(ac != NULL && ac->user != NULL);

	if(!(buddy = purple_find_buddy(account, contact_cur->userId))) return;
	crc = purple_buddy_icons_get_checksum_for_user(buddy);
	if((!crc && contact_cur->portraitCrc[0] == '\0') || (crc && strcmp(crc, contact_cur->portraitCrc) == 0)) return;

	data = g_malloc0(sizeof(portrait_data));
	data->cnt = contact_cur;
	data->ac = ac;
	purple_proxy_connect(NULL, ac->account,
			ac->user->portraitServerName, 80,
			(PurpleProxyConnectFunction)download_portrait_cb, data);
}

static gint download_portrait_cb(gpointer data, gint source, const gchar *UNUSED(error_message))
{
	gchar *encodedSipuri, *encodedSsic;
	gchar  http[BUFLEN], uri[BUFLEN];
	portrait_data  *udata;
	fetion_account *ac;
	portrait_trans *trans;

	udata = (portrait_data*)data;
	ac    = udata->ac;

	g_return_val_if_fail(ac->user != NULL, -1);

	snprintf(uri, sizeof(uri) - 1 , "/%s/getportrait.aspx" , ac->user->portraitServerPath);

	encodedSipuri = http_connection_encode_url(udata->cnt->sipuri);
	encodedSsic = http_connection_encode_url(ac->user->ssic);
	snprintf(http, sizeof(http) - 1, "GET %s?Uri=%s"
			  "&Size=120&c=%s HTTP/1.1\r\n"
			  "User-Agent: IIC2.0/PC "PROTO_VERSION"\r\n"
			  "Accept: image/pjpeg;image/jpeg;image/bmp;"
			  "image/x-windows-bmp;image/png;image/gif\r\n"
			  "Host: %s\r\nConnection: Keep-Alive\r\n\r\n",
			  uri, encodedSipuri, encodedSsic, ac->user->portraitServerName);

	if(send(source, http, strlen(http), 0) == -1) goto pcb_fin;

	trans = portrait_trans_new();
	trans->source = source;
	trans->cnt = udata->cnt;
	trans->ac = ac;
	trans->data = (guchar*)0;
	trans->size = 0;
	trans->conn = purple_input_add(source, PURPLE_INPUT_READ,
						(PurpleInputFunction)get_portrait_cb, trans);

	g_free(udata);
	g_free(encodedSipuri);
	g_free(encodedSsic);
	return 0;
pcb_fin:
	g_free(udata);
	g_free(encodedSipuri);
	g_free(encodedSsic);
	g_free(udata);
	close(source);
	return -1;
}

static gint get_portrait_cb(gpointer data, gint source, const gchar *UNUSED(error_message))
{
	gchar buf[BUFLEN], *pos;
	gint n, hl;
	portrait_trans *trans;
	fetion_account  *ac;

	trans = (portrait_trans*)data;

	ac = trans->ac;

	if((n = recv(source, buf, sizeof(buf), 0)) == -1) goto pt_fin;

	if(trans->size == 0) {
		if((pos = strstr(buf, "HTTP/1.1 404"))) goto pt_fin;
		if((pos = strstr(buf, "HTTP/1.1 200"))) goto pt_fnd;
		if((pos = strstr(buf, "HTTP/1.1 302"))) { printf("302 portrait\n"); goto pt_fin; }
		goto pt_fin;

	} else {
		memcpy(trans->data + trans->size, buf, n);
		trans->size += n;
		if(trans->size == trans->sum) goto pt_upd;
	}

	return 0;
pt_fnd:
	if(!(pos = strstr(buf, "\r\n\r\n"))) goto pt_fin;
	if((trans->sum = http_connection_get_body_length(buf)) == 0) goto pt_fin;
	hl = pos - buf + 4;
	trans->data = (guchar*)g_malloc0(trans->sum);
	trans->size = n - hl;
	memcpy(trans->data, buf + hl, trans->size);
	if(trans->size != trans->sum) return 0;
pt_upd:
	purple_buddy_icons_set_for_user(trans->ac->account, trans->cnt->userId,
		   	trans->data, trans->size, trans->cnt->portraitCrc);
pt_fin:
	purple_input_remove(trans->conn);
	g_free(trans);
	return 0;
}

static gchar* http_connection_encode_url(const gchar* url)
{
	gchar pos, *res;
	gchar tmp[2];
	gint i = 1;
	res = (gchar*)g_malloc0(2048);
	if(!res) return (gchar*)0;
	pos = url[0];
	memset(res , 0 , 2048);
	while(pos != '\0') {
		if(pos == '/')
			strcat(res , "%2f");
		else if(pos == '@')
			strcat(res , "%40");
		else if(pos == '=')
			strcat(res , "%3d");
		else if(pos == ':')
			strcat(res , "%3a");
		else if(pos == ';')
			strcat(res , "%3b");
		else if(pos == '+'){
			strcat(res , "%2b");
		}else{
			memset(tmp, 0, sizeof(tmp));
			sprintf(tmp, "%c", pos);
			strcat(res, tmp);
		}
		pos = (url + (i ++))[0];
	}
	return res;
}

static gint http_connection_get_body_length(const gchar *http)
{
	gchar *pos , length[16];
	gint   len;
	
	pos = strstr(http , "Content-Length: ");
	if(!pos) return 0;
	pos += 16;
	len = strlen(pos) - strlen(strstr(pos , "\r\n"));
	memset(length, 0, sizeof(length));
	strncpy(length , pos , (len<9)?len:9);
	return atoi(length);
}

portrait_trans *portrait_trans_new()
{
	return (portrait_trans*)g_malloc0(sizeof(portrait_trans));
}
void portrait_trans_free(portrait_trans *trans)
{
	g_free(trans);
}

gchar *get_city_name(const gchar *province, const gchar *city)
{
	gchar path[] = RES_DIR"city.xml"; 
	xmlChar *res;
	xmlDocPtr doc;
	xmlNodePtr node;
	doc = xmlParseFile(path);
	if(!doc) return (gchar*)0;
	node = xmlDocGetRootElement(doc);
	node = node->xmlChildrenNode;
	while(node) {
		if(node->type != XML_ELEMENT_NODE) {
			node = node->next;
			continue;
		}
		res = xmlGetProp(node , BAD_CAST "id");
		if(xmlStrcmp(res , BAD_CAST province) == 0)	{
			node = node->xmlChildrenNode;
			while(node)	{
				if(node->type != XML_ELEMENT_NODE) {
					node = node->next;
					continue;
				}
				xmlFree(res);
				res = xmlGetProp(node , BAD_CAST "id");
				if(xmlStrcmp(res , BAD_CAST city) == 0)	{
					xmlFree(res);
					return (gchar*)xmlNodeGetContent(node);
					break;
				}
				node = node->next;
			}
			break;
		}
		xmlFree(res);
		node = node->next;
	}
	return (gchar*)0;
}

gchar *get_province_name(const gchar *province)
{
	gchar path[] = RES_DIR"province.xml"; 
	xmlChar* res;
	xmlDocPtr doc;
	xmlNodePtr node;
	doc = xmlReadFile(path, "UTF-8", XML_PARSE_RECOVER);
	if(!doc) return (gchar*)0;
	node = xmlDocGetRootElement(doc);
	node = node->xmlChildrenNode;
	while(node) {
		res = xmlGetProp(node , BAD_CAST "id");
		if(xmlStrcmp(res , BAD_CAST province) == 0) {
			return (gchar*)xmlNodeGetContent(node);
			xmlFree(res);
			break;
		}
		xmlFree(res);
		node = node->next;
	}
	xmlFreeDoc(doc);
	return (gchar*)0;
}

static gchar *generate_add_buddy_body(const gchar *no,
	   	gint notype, gint buddylist,
	   	const gchar *localname , const gchar *desc)
{
	const gchar args[] = "<args></args>";
	gchar uri[48];
	gchar groupid[16];
	xmlChar *res;
	xmlDocPtr doc;
	xmlNodePtr node;
	doc = xmlParseMemory(args , strlen(args));
	node = xmlDocGetRootElement(doc);
	node = xmlNewChild(node , NULL , BAD_CAST "contacts" , NULL);
	node = xmlNewChild(node , NULL , BAD_CAST "buddies" , NULL);
	node = xmlNewChild(node , NULL , BAD_CAST "buddy" , NULL);

	if(notype == FETION_NO)	snprintf(uri, sizeof(uri) - 1 , "sip:%s" , no);
	else snprintf(uri, sizeof(uri) - 1 , "tel:%s" , no);

	snprintf(groupid, sizeof(groupid) - 1 , "%d" , buddylist);
	xmlNewProp(node, BAD_CAST "uri" , BAD_CAST uri);
	xmlNewProp(node, BAD_CAST "local-name" , BAD_CAST localname);
	xmlNewProp(node, BAD_CAST "buddy-lists" , BAD_CAST groupid);
	xmlNewProp(node, BAD_CAST "desc" , BAD_CAST desc);
	xmlNewProp(node, BAD_CAST "expose-mobile-no" , BAD_CAST "1");
	xmlNewProp(node, BAD_CAST "expose-name" , BAD_CAST "1");
	xmlNewProp(node, BAD_CAST "addbuddy-phrase-id" , BAD_CAST "0");
	xmlDocDumpMemory(doc , &res , NULL);
	xmlFreeDoc(doc);
	return xml_convert(res);
}

static Contact *parse_add_buddy_response(const gchar *sipmsg, gint *status_code, gchar **errMsg)
{
	char *pos;
	Contact* contact;
	xmlChar* res;
	xmlDocPtr doc;
	xmlNodePtr node;
	contact = fetion_contact_new();
	pos = strstr(sipmsg , "\r\n\r\n") + 4;
	doc = xmlParseMemory(pos , strlen(pos));
	node = xmlDocGetRootElement(doc);
	node = xml_goto_node(node , "buddy");

	if(!node) {
		g_free(contact);
		xmlFreeDoc(doc);
		return (Contact*)0;
	}
	if(xmlHasProp(node , BAD_CAST "uri")) {
		res = xmlGetProp(node , BAD_CAST "uri");
		strcpy(contact->sipuri , (char*)res);
		xmlFree(res);
	}
	if(xmlHasProp(node , BAD_CAST "user-id")) {
		res = xmlGetProp(node , BAD_CAST "user-id");
		strcpy(contact->userId , (char*)res);
		xmlFree(res);
	}
	if(xmlHasProp(node , BAD_CAST "mobile-no")) {
		res = xmlGetProp(node , BAD_CAST "mobile-no");
		strcpy(contact->mobileno , (char*)res);
		xmlFree(res);
	}
	if(xmlHasProp(node , BAD_CAST "local-name")) {
		res = xmlGetProp(node , BAD_CAST "local-name");
		strcpy(contact->localname , (char*)res);
		xmlFree(res);
	}
	if(xmlHasProp(node , BAD_CAST "buddy-lists")) {
		res = xmlGetProp(node , BAD_CAST "buddy-lists");
		contact->groupid = atoi((char*)res);
		xmlFree(res);
	}
	if(xmlHasProp(node , BAD_CAST "status-code")) {
		res = xmlGetProp(node , BAD_CAST "status-code");
		*status_code = atoi((char*)res);
		xmlFree(res);
	}

	if(xmlHasProp(node , BAD_CAST "basic-service-status")) {
		res = xmlGetProp(node , BAD_CAST "basic-service-status");
		contact->serviceStatus = atoi((char*)res);
		xmlFree(res);
	}
	*errMsg = (gchar*)0;
	if(xmlHasProp(node, BAD_CAST "error-reason"))
		*errMsg = (gchar*)xmlGetProp(node, BAD_CAST "error-reason");
	contact->relationStatus = STATUS_NOT_AUTHENTICATED;
	xmlFreeDoc(doc);
	return contact;
}

static Contact *parse_syncinfo(Contact *clist, const gchar *sipmsg)
{
	gchar     *pos;
	Contact   *contact;
	xmlChar   *res;
	xmlDocPtr doc;
	xmlNodePtr node;

	if(!(pos = strstr(sipmsg , "\r\n\r\n"))) return (Contact*)0;
	doc = xmlParseMemory(pos + 4 , strlen(pos + 4));
	node = xmlDocGetRootElement(doc);
	node = xml_goto_node(node , "buddies");
	if(!node) {
		xmlFreeDoc(doc);
		return (Contact*)0;
	}
	node = node->xmlChildrenNode;
	while(node) {
		if(!xmlHasProp(node, BAD_CAST "action")) {
			node = node->next;
			continue;
		}

		res = xmlGetProp(node, BAD_CAST "action");
		if(strcmp((gchar*)res, "add") != 0) {
			xmlFree(res);
			node = node->next;
			continue;
		}
		xmlFree(res);

		if(! xmlHasProp(node , BAD_CAST "user-id")) return (Contact*)0;
		res = xmlGetProp(node , BAD_CAST "user-id");
		if(!(contact = fetion_contact_list_find_by_userid(clist, (gchar*)res))) {
			contact = fetion_contact_new();
			strcpy(contact->userId , (gchar*)res);
		}
		xmlFree(res);

		if(xmlHasProp(node, BAD_CAST "uri")) {
			res = xmlGetProp(node, BAD_CAST "uri");
			strcpy(contact->sipuri, (gchar*)res);
			xmlFree(res);
		}
		if(xmlHasProp(node, BAD_CAST "local-name")) {
			res = xmlGetProp(node, BAD_CAST "local-name");
			strcpy(contact->localname, (gchar*)res);
			xmlFree(res);
		}
		if(xmlHasProp(node, BAD_CAST "buddy-lists")) {
			res = xmlGetProp(node, BAD_CAST "buddy-lists");
			contact->groupid = atoi((char*)res);
			xmlFree(res);
		}
		if(xmlHasProp(node, BAD_CAST "relation-status")) {
			res = xmlGetProp(node, BAD_CAST "relation-status");
			contact->relationStatus = atoi((char*)res);
			xmlFree(res);
		}else
			contact->relationStatus = 0;

		node = node->next;
	}
	xmlFreeDoc(doc);
	return contact;
}

static Contact *parse_addbuddyapplication(const gchar *sipmsg)
{
	gchar   *pos = NULL;
	xmlDocPtr doc;
	xmlNodePtr node;
	xmlChar *res = NULL;
	Contact *cnt;
	pos = strstr(sipmsg , "\r\n\r\n") + 4;
	doc = xmlParseMemory(pos , strlen(pos));
	node = xmlDocGetRootElement(doc);
	node = xml_goto_node(node , "application");

	cnt = fetion_contact_new();

	res = xmlGetProp(node, BAD_CAST "uri");
	strcpy(cnt->sipuri, (gchar*)res);
	xmlFree(res);

	res = xmlGetProp(node, BAD_CAST "user-id");
	strcpy(cnt->userId, (gchar*)res);
	xmlFree(res);

	res = xmlGetProp(node, BAD_CAST "desc");
	strcpy(cnt->localname, (gchar*)res);
	xmlFree(res);

	xmlFreeDoc(doc);
	return cnt;
}

static gchar *generate_handle_contact_request_body(const gchar *sipuri
		, const gchar *userid, const gchar *localname
		, gint buddylist, gint result )
{
	gchar args[] = "<args></args>";
	gchar result_s[4];
	gchar buddylist_s[4];
	xmlChar *res;
	xmlDocPtr doc;
	xmlNodePtr node;
	doc = xmlParseMemory(args , strlen(args));
	node = xmlDocGetRootElement(doc);
	node = xmlNewChild(node, NULL, BAD_CAST "contacts", NULL);
	node = xmlNewChild(node, NULL, BAD_CAST "buddies", NULL);
	node = xmlNewChild(node, NULL, BAD_CAST "buddy", NULL);
	xmlNewProp(node, BAD_CAST "user-id", BAD_CAST userid);
	xmlNewProp(node, BAD_CAST "uri", BAD_CAST sipuri);
	snprintf(result_s, sizeof(result_s) - 1, "%d", result);
	snprintf(buddylist_s, sizeof(buddylist_s) - 1, "%d", buddylist);
	xmlNewProp(node, BAD_CAST "result", BAD_CAST result_s);
	xmlNewProp(node, BAD_CAST "buddy-lists", BAD_CAST buddylist_s);
	xmlNewProp(node, BAD_CAST "expose-mobile-no", BAD_CAST "1");
	xmlNewProp(node, BAD_CAST "expose-name", BAD_CAST "1");
	xmlNewProp(node, BAD_CAST "local-name", BAD_CAST localname);
	xmlDocDumpMemory(doc, &res, NULL);
	xmlFreeDoc(doc);
	return xml_convert(res);
}
