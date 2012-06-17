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
#include "fx_login.h"
#include "fx_buddy.h"
#include "async.h"
#include <eventloop.h>
#include <openssl/rsa.h>
#include <openssl/sha.h>

#define VERIFY_TYPE_SSI 1
#define VERIFY_TYPE_SIP 2

struct verify_data {
	gint type;                /* ssi verify || sipc verify */
	fetion_account *ac;        /* common data */
	PurpleSslConnection *ssl; /* used by ssi verify */
	gint sipc_conn;           /* used by sipc verify */
	gchar response[BUFLEN];   /* used by sipc verify */
	gchar *data;
} verify_data;

/*private method*/
static gchar *generate_auth_body(User* user);
static gchar *generate_configuration_body(User* user);
static void parse_personal_info(xmlNodePtr node , User* user);
static void parse_contact_list(xmlNodePtr node , User* user, int *group_count, int *buddy_count);
static void parse_stranger_list(xmlNodePtr node , User* user);
static void parse_ssi_auth_success(xmlNodePtr node , User* user);
static void parse_ssi_auth_failed(xmlNodePtr node , User* user);
static guchar *strtohex(const gchar *in, gint *len);
static gchar *hextostr(const guchar *in, gint len);
static gchar *hash_password_v1(const guchar *b0, gint b0len, const guchar *password, gint psdlen);
static gchar *hash_password_v2(const gchar *userid , const gchar *passwordhex);
static gchar *hash_password_v4(const gchar *userid , const gchar *password);
static gchar *generate_cnouce() ;
static gint download_cfg(gpointer data, gint source, const gchar * error_message);
static void ssi_auth_cb(gpointer data, PurpleSslConnection *source, PurpleInputCondition *cond);
static int sipc_reg_action(gpointer data, gint source, const gchar *error_message);
static gint parse_configuration_xml(User *user, const char *xml);
static gchar *decode_base64(const gchar *in, gint *len);
static gint parse_sipc_verification(User *user, const gchar *str);
static void parse_sipc_reg_response(const gchar *reg_response, gchar **nouce, gchar **key);
static gchar *generate_aes_key();
static gint parse_sipc_auth_response(const gchar *auth_response, User *user, gint *group_count, gint *buddy_count);

char* generate_response(const char* nouce , const char* userid 
		, const char* password , const char* publickey , const char* key)
{
	char* psdhex = hash_password_v4(userid , password);
	char modulus[257];
	char exponent[7];
	int ret, flen;
	BIGNUM *bnn, *bne;
	unsigned char *out;
	unsigned char *nonce , *aeskey , *psd , *res;
	int nonce_len , aeskey_len , psd_len;
	RSA *r = RSA_new();

	key = NULL;

	memset(modulus, 0, sizeof(modulus));
	memset(exponent, 0, sizeof(exponent));

	memcpy(modulus , publickey , 256);
	memcpy(exponent , publickey + 256 , 6);
	nonce = (guchar*)g_malloc0(strlen(nouce) + 1);
	memcpy(nonce , (guchar*)nouce , strlen(nouce));
	nonce_len = strlen(nouce);
	psd = strtohex(psdhex , &psd_len);
	aeskey = strtohex(generate_aes_key() , &aeskey_len);
	res = (guchar*)g_malloc0(nonce_len + aeskey_len + psd_len + 1);
	memcpy(res , nonce , nonce_len);
	memcpy(res + nonce_len , psd , psd_len );
	memcpy(res + nonce_len + psd_len , aeskey , aeskey_len);

	bnn = BN_new();
	bne = BN_new();
	BN_hex2bn(&bnn, modulus);
	BN_hex2bn(&bne, exponent);
	r->n = bnn;	r->e = bne;	r->d = NULL;
//	RSA_print_fp(stdout, r, 5);
	flen = RSA_size(r);
	out =  (guchar*)g_malloc0(flen);
	purple_debug_info("fetion", "start encrypting response");
	ret = RSA_public_encrypt(nonce_len + aeskey_len + psd_len,
			res , out, r, RSA_PKCS1_PADDING);
	if (ret < 0){
		purple_debug_info("fetion", "encrypt response failed!");
		g_free(res); 
		g_free(aeskey);
		g_free(psd);
		g_free(nonce);
		return NULL;
	}
	RSA_free(r);
	purple_debug_info("fetion", "encrypting reponse success");
	g_free(res); 
	g_free(aeskey);
	g_free(psd);
	g_free(nonce);
	return hextostr(out , ret);
}

static void pic_ok_cb(fetion_account *ac, PurpleRequestFields *fields)
{
	const gchar *code;
	code = purple_request_fields_get_string(fields, "code_entry");

	fetion_user_set_verification_code(ac->user, code);
	if(verify_data.type == VERIFY_TYPE_SSI) {
		purple_ssl_connect(ac->account, SSI_SERVER,
				PURPLE_SSL_DEFAULT_PORT, 
				(PurpleSslInputFunction)ssi_auth_action,
				(PurpleSslErrorFunction)0, ac);
	} else if(verify_data.type == VERIFY_TYPE_SIP) {
		sipc_aut_action(verify_data.sipc_conn, ac, verify_data.response);
	}
}

static void pic_cancel_cb(fetion_account *ac, PurpleRequestFields *UNUSED(fields))
{
	//fetion_verification_free(ac->user->verification);
	//ac->user->verification = NULL;
	purple_connection_error_reason(ac->gc,
    					PURPLE_CONNECTION_ERROR_AUTHENTICATION_FAILED,
				       _("Login Failed."));
}

static void fetion_code_request(fetion_account *ac, const gchar *code_data, gint code_size)
{
	PurpleRequestFieldGroup *field_group;
	PurpleRequestField *code_entry;
	PurpleRequestField *code_pic;
	PurpleRequestFields *fields;

	fields = purple_request_fields_new();
	field_group = purple_request_field_group_new((gchar*)0);
	purple_request_fields_add_group(fields, field_group);

	code_pic = purple_request_field_image_new("code_pic", _("Confirmation code"), code_data, code_size);
	purple_request_field_group_add_field(field_group, code_pic);

	code_entry = purple_request_field_string_new("code_entry", _("Please input the code"), "", FALSE);
	purple_request_field_group_add_field(field_group, code_entry);

	purple_request_fields(ac->account, NULL,
		   		//ac->user->verification->tips, (gchar*)0,
                "hi",(gchar*)0,
				fields, _("OK"), G_CALLBACK(pic_ok_cb), 
			   	_("Cancel"), G_CALLBACK(pic_cancel_cb),
			   	ac->account, NULL, NULL, ac);
}
void pic_read(gpointer data)
{
    gchar* pic;
    gint pic_len;
    gchar path[50];
    struct stat buf;
    int fp;
    
	fetion_account *ac = (fetion_account*)data;
    LwqqClient* qq=ac->qq;
    snprintf(path,50,"/tmp/%s.jpep",qq->username);
    stat(path,&buf);
    pic_len = buf.st_size;
    pic = g_malloc0(pic_len);
    fp = open(path,O_RDONLY);
    read(fp,pic,pic_len);
    close(fp);
    fetion_code_request(ac,pic,pic_len);
}
void msg_come(LwqqClient* lc,LwqqHttpRequest* UNUSED(req),void* data)
{
    LwqqRecvMsg *msg;
    if (!SIMPLEQ_EMPTY(&lc->msg_list->head)) {
        msg = SIMPLEQ_FIRST(&lc->msg_list->head);
        if (msg->msg->content) {
            purple_debug_info("account","%s\n",msg->msg->content);
        }
        SIMPLEQ_REMOVE_HEAD(&lc->msg_list->head, entries);
    }
}
void friends_all_complete(LwqqClient* lc,LwqqHttpRequest* req,void* data)
{
    purple_debug_info("account","friends_all_complete\n");
    fetion_account* ac = (fetion_account*)data;
    fx_blist_init(ac);

    //开始接受消息循环.
    lwqq_async_add_listener(lc,MSG_COME,msg_come,NULL);
    lc->msg_list->poll_msg(lc->msg_list);
}

void login_complete(LwqqClient* lc,LwqqHttpRequest *req,void* data)
{
	const gchar *status_id;
    LwqqErrorCode err=lc->async->last_err;
    fetion_account* ac = (fetion_account*)data;
    PurpleAccount* account = ac->account;
    PurplePresence* presence;
	PurpleConnection *pc = purple_account_get_connection(account);
    if(lc->status="")lc->status="Online";

    purple_debug_info("account","login_complete");
	status_id = get_status_id(lc->status);
	presence = purple_account_get_presence(account);
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

    lwqq_async_add_listener(lc,FRIENDS_ALL_COMPLETE,friends_all_complete,data);
    lwqq_info_get_friends_info(lc,&err);

    return;
done:
    lwqq_client_free(ac->qq);
}
static gint pic_read_cb(gpointer data, gint source, const gchar *UNUSED(message))
{
	gint n, len;
	gchar sipmsg[BUFLEN];
	xmlDocPtr  doc;
	xmlNodePtr node;
	gchar *code, *pos;
	gchar *pic;
	gint piclen;

	fetion_account *ac = (fetion_account*)data;

	len = ac->data ? strlen(ac->data) : 0;
	if((n = recv(source, sipmsg, strlen(sipmsg), 0)) == -1) return -1;

	sipmsg[n] = 0;
	if(n == 0) {
		purple_input_remove(ac->conn);
		close(source);
		if(! ac->data) return 0;
		if(!(pos = strstr(ac->data, "\r\n\r\n"))) {
			g_free(ac->data);
			ac->data = (gchar*)0;
			return -1;
		}
		doc = xmlParseMemory(pos + 4, strlen(pos + 4));
		node = xmlDocGetRootElement(doc);
		node = node->xmlChildrenNode;
		ac->user->verification->guid = (gchar*)xmlGetProp(node , BAD_CAST "id");
		code = (gchar*)xmlGetProp(node , BAD_CAST "pic");
		xmlFreeDoc(doc);
		purple_debug_info("fetion", "Generating verification code picture");
		pic = decode_base64(code , &piclen);
		g_free(code);
		fetion_code_request(ac, pic, piclen);
		g_free(pic);
		g_free(ac->data);
		ac->data = (gchar*)0;

		return 0;
	}

	ac->data = (gchar*)realloc(ac->data, len + n + 1);
	memcpy(ac->data + len, sipmsg, n + 1);

	return 0;
}

static gint pic_code_cb(gpointer data, gint source, const gchar *UNUSED(message))
{
	gchar cookie[BUFLEN];
	gchar http[BUFLEN];
	fetion_account *ac = (fetion_account*)data;
	User *user = ac->user;

	if(user->ssic)	snprintf(cookie, sizeof(cookie) - 1, "Cookie: ssic=%s\r\n", user->ssic);

	snprintf(http, sizeof(http) - 1,
		 	"GET /nav/GetPicCodeV4.aspx?algorithm=%s HTTP/1.1\r\n"
		    "%sHost: %s\r\n"
		    "User-Agent: IIC2.0/PC "PROTO_VERSION"\r\n"
		    "Connection: close\r\n\r\n",
		   	user->verification->algorithm == NULL ? "" : user->verification->algorithm,
		   	user->ssic == NULL ? "" : cookie, NAV_SERVER);
	
	if(send(source, http, strlen(http), 0) == -1) return -1;
	ac->data = (gchar*)0;
	ac->conn = purple_input_add(source, PURPLE_INPUT_READ,
				(PurpleInputFunction)pic_read_cb, data);
	return 0;
}

gboolean ssi_auth_action(gpointer data, PurpleSslConnection * gsc, gint UNUSED(con))
{
	gchar sslbuf[BUFLEN];
	gchar noUri[256];
	gchar verifyUri[256];
	gchar *password;
	gint passwordType;
	fetion_account *ac = (fetion_account*)data;
	User *user = ac->user;
	
	purple_debug_info("fetion", "initialize ssi authentication action");
	password = hash_password_v4(user->userId , user->password);
	memset(noUri, 0, sizeof(noUri));
	if(user->loginType == LOGIN_TYPE_MOBILENO)
		snprintf(noUri, sizeof(noUri) - 1, "mobileno=%s" , user->mobileno);
	else
		snprintf(noUri, sizeof(noUri) - 1, "sid=%s" , user->sId);
	memset(verifyUri, 0, sizeof(verifyUri));
	if(user->verification != NULL && user->verification->code != NULL) {
		snprintf(verifyUri, sizeof(verifyUri) - 1,
			   				"&pid=%s&pic=%s&algorithm=%s",
						   	user->verification->guid,
						   	user->verification->code,
						   	user->verification->algorithm);
	}
	passwordType = (strlen(user->userId) == 0 ? 1 : 2);
	snprintf(sslbuf, sizeof(sslbuf) - 1,
		   			"GET /ssiportal/SSIAppSignInV4.aspx?%s"
				    "&domains=fetion.com.cn%s&v4digest-type=%d&v4digest=%s\r\n"
				    "User-Agent: IIC2.0/pc "PROTO_VERSION"\r\n"
					"Host: %s\r\n"
				    "Cache-Control: private\r\n"
				    "Connection: Keep-Alive\r\n\r\n",
				    noUri, verifyUri, passwordType, password, SSI_SERVER);
	purple_ssl_write(gsc, sslbuf, strlen(sslbuf));

	purple_ssl_input_add(gsc, (PurpleSslInputFunction)ssi_auth_cb, ac);

	return TRUE;
}

static gint sipc_reg_cb(gpointer data, gint source, const gchar *UNUSED(error_message))
{

	gchar sipmsg[BUFLEN];
	gint  n;
	gchar  *nonce, *key, *aeskey, *response;

	fetion_account *ac = (fetion_account*)data;

	if((n = recv(source , sipmsg , sizeof(sipmsg), 0)) < 0)	return -1;
	sipmsg[n] = '\0';
	parse_sipc_reg_response(sipmsg, &nonce, &key);
	aeskey = generate_aes_key();
	response = generate_response(nonce, ac->user->userId,
							ac->user->password, key, aeskey);

	/* fill verify_data for pic confirm */
	strncpy(verify_data.response, response, sizeof(verify_data.response));

	g_free(key);
	g_free(aeskey);
	g_free(nonce);

	if(sipc_aut_action(source, ac, response) == -1) {
		g_free(response);
		return -1;
	}
	g_free(response);

	return 0;
}
/***HERE***/
static gint sipc_aut_cb(gpointer data, gint source, const gchar *UNUSED(error_message))
{
	gchar  sipmsg[BUFLEN * 20], *pos;
	gint   n;
	guint  length = 0;
	gint   group_count, buddy_count;
	PurpleConnection *pc;
	fetion_account *ac = (fetion_account*)data;

	if((n = recv(source, sipmsg, sizeof(sipmsg) - 1, 0)) == -1)	return -1;
	sipmsg[n] = '\0';

	length = ac->data ? strlen(ac->data) : 0;
	ac->data = (gchar*)realloc(ac->data, length + strlen(sipmsg) + 1);
	memcpy(ac->data + length, sipmsg, strlen(sipmsg) + 1);

	if(strstr(ac->data, "\r\n\r\n")) {
		length = fetion_sip_get_length(ac->data);
		pos = strstr(ac->data, "\r\n\r\n") + 4;
		if(length < strlen(pos)) {
			n = strlen(ac->data) - (strlen(pos) - length);
			ac->data = (gchar*)realloc(ac->data, n + 1);
			ac->data[n] = '\0';
			goto aut_fin;
		} else if (length == strlen(pos)) {
			goto aut_fin;
		}
	}

	return 0;

aut_fin:

	if(!purple_input_remove(ac->conn)) return -1;

	parse_sipc_auth_response(ac->data, ac->user, &group_count, &buddy_count);
	if(fetion_contact_has_ungrouped(ac->user->contactList)) {
		Group *grp = fetion_group_new();
		strncpy(grp->groupname, "未分组", sizeof(grp->groupname));
		grp->groupid = 0;
		fetion_group_list_append(ac->user->groupList, grp);
	}
		

	g_free(ac->data);
	ac->data = (gchar*)0;
	ac->sk = source;

	pc = purple_account_get_connection(ac->account);
	if(USER_AUTH_NEED_CONFIRM(ac->user)) {
		verify_data.type = VERIFY_TYPE_SIP;
		verify_data.sipc_conn = source;
		if(ac->conn_data) {
			purple_proxy_connect_cancel(ac->conn_data);
			ac->conn_data = NULL;
		}
		ac->conn_data = purple_proxy_connect(NULL, ac->account, NAV_SERVER, 80,
							(PurpleProxyConnectFunction)pic_code_cb, ac);
		return -1;
	} else if(USER_AUTH_ERROR(ac->user) ) {
		purple_connection_error_reason(pc, PURPLE_CONNECTION_ERROR_AUTHENTICATION_FAILED,
							       _("Incorrect password."));
		return -1;
	}

	purple_connection_set_state(pc, PURPLE_CONNECTED);

	fx_blist_init(ac);

	if(fetion_contact_subscribe_only(source, ac->user) == -1) return -1;

	purple_connection_set_display_name(pc, ac->user->nickname);

	ac->conn = purple_input_add(source, PURPLE_INPUT_READ,
			(PurpleInputFunction)push_cb, ac);

	return 0;
}

static gint sipc_reg_action(gpointer data, gint source, const gchar *UNUSED(error_message))
{
	gchar *sipmsg;
	gchar *cnouce = generate_cnouce();
	gint   ret;
	fetion_account    *ac = (fetion_account*)data;

	fetion_sip* sip = fetion_sip_new0(ac->user);

	purple_debug_info("fetion", "initialize sipc registeration action");

	fetion_sip_set_type(sip , SIP_REGISTER);
	SipHeader* cheader = fetion_sip_header_new("CN" , cnouce);
	SipHeader* client = fetion_sip_header_new("CL" , "type=\"pc\" ,version=\""PROTO_VERSION"\"");
	fetion_sip_add_header(sip , cheader);
	fetion_sip_add_header(sip , client);
	g_free(cnouce);
	sipmsg = fetion_sip_to_string(sip , NULL);
	purple_debug_info("fetion", "start registering to sip server(%s:%d)"
			 , ac->user->sipcProxyIP , ac->user->sipcProxyPort);
	if((ret = write(source , sipmsg , strlen(sipmsg))) == -1) return -1;
	g_free(sipmsg);
	ac->conn = purple_input_add(source, PURPLE_INPUT_READ,
			(PurpleInputFunction)sipc_reg_cb, ac);

	return 0;
}
gint sipc_aut_action(gint sk, fetion_account *ac, const gchar *response)
{
	gchar* sipmsg;
	gchar* xml;
	User  *user = ac->user;
	SipHeader* aheader = NULL;
	SipHeader* akheader = NULL;
	SipHeader* ackheader = NULL;
	fetion_sip* sip = ac->user->sip;

	purple_debug_info("fetion", "Initialize sipc authencation action");

	xml = generate_auth_body(user);
	fetion_sip_set_type(sip , SIP_REGISTER);
	aheader = fetion_sip_authentication_header_new(response);
	akheader = fetion_sip_header_new("AK" , "ak-value");
	fetion_sip_add_header(sip , aheader);
	fetion_sip_add_header(sip , akheader);
	if(user->verification != NULL && user->verification->algorithm != NULL)	{
		ackheader = fetion_sip_ack_header_new(user->verification->code
											, user->verification->algorithm
											, user->verification->type
											, user->verification->guid);
		fetion_sip_add_header(sip , ackheader);
	}
	sipmsg = fetion_sip_to_string(sip , xml);

	fetion_verification_free(user->verification);
	user->verification = NULL;
	purple_debug_info("fetion", "Start sipc authentication , with ak-value");

	if(send(sk, sipmsg, strlen(sipmsg), 0) == -1) { g_free(sipmsg); return -1; }
	g_free(sipmsg);

	if(!purple_input_remove(ac->conn)) return -1;

	ac->data = (gchar*)0;
	ac->left_len = 0;

	ac->conn = purple_input_add(sk, PURPLE_INPUT_READ,
				(PurpleInputFunction)sipc_aut_cb, ac);

	return 0;
}

static void ssi_auth_cb(gpointer data, PurpleSslConnection *source, PurpleInputCondition *UNUSED(cond))
{
	xmlDocPtr  doc;
	xmlNodePtr node;
	fetion_account *ac = (fetion_account*)data;
	PurpleConnection *pc;
	User          *user = ac->user;
	const gchar   *uri = "nav.fetion.com.cn";
	gchar  ssi_response[BUFLEN];
	gint   n;
	gchar *pos;
	gchar *xml;

	n = purple_ssl_read(source, ssi_response, sizeof(ssi_response));
	ssi_response[n] = '\0';
	purple_ssl_close(source);

	if(strstr(ssi_response , "ssic=")){
		pos = strstr(ssi_response , "ssic=") + 5;
		n = strlen(pos) - strlen(strstr(pos , ";"));
		user->ssic = (gchar*)g_malloc0(n + 1);
		strncpy(user->ssic , pos , n);
	}

	if(!(xml = strstr(ssi_response, "\r\n\r\n"))) {
		pc = purple_account_get_connection(ac->account);
		purple_connection_error_reason(pc,
			       PURPLE_CONNECTION_ERROR_AUTHENTICATION_FAILED,
						       _("Login Failed."));
		return;
	}

	doc = xmlParseMemory(xml + 4 , strlen(xml + 4));
	node = xmlDocGetRootElement(doc);
	pos = (gchar*)xmlGetProp(node , BAD_CAST "status-code");
	user->loginStatus = atoi(pos);
	node = node->xmlChildrenNode;

	if(atoi(pos) == 200) {
		fetion_verification_free(user->verification);
		user->verification = NULL;
		purple_debug_info("fetion", "ssi login success");
		parse_ssi_auth_success(node , user);
	} else {
		purple_debug_info("fetion", "ssi login failed , status-code :%s" , pos);
		parse_ssi_auth_failed(node , user);
		g_free(pos);
		xmlFreeDoc(doc);
		if(USER_AUTH_NEED_CONFIRM(user)) {
			verify_data.type = VERIFY_TYPE_SSI;
			verify_data.ssl = source;
			ac->conn_data = purple_proxy_connect(NULL, ac->account, NAV_SERVER, 80,
								(PurpleProxyConnectFunction)pic_code_cb, ac);

		} else {
			pc = purple_account_get_connection(ac->account);
			purple_connection_error_reason(pc,
				       PURPLE_CONNECTION_ERROR_AUTHENTICATION_FAILED,
							       _("Incorrect password."));
		}
		return;
	}

	g_free(pos);
	xmlFreeDoc(doc);

	if(ac->conn_data) {
		purple_proxy_connect_cancel(ac->conn_data);
		ac->conn_data = NULL;
	}
	/* download configuration routine */
	ac->conn_data = purple_proxy_connect(NULL, ac->account,
			uri, 80, (PurpleProxyConnectFunction)download_cfg, ac);

	if(!ac->conn_data) {
		purple_debug_error("fetion", "connect to cfg server failed\n");
		pc = purple_account_get_connection(ac->account);
		purple_connection_error_reason(pc,
			       PURPLE_CONNECTION_ERROR_AUTHENTICATION_FAILED,
					       _("Login Failed."));
	}

	return;
}
static void parse_sipc_reg_response(const gchar *reg_response, gchar **nouce, gchar **key)
{
	gchar  digest[2048] = { 0 };
	gchar *pos;
	gint n;

	fetion_sip_get_attr(reg_response , "W" , digest);

	pos = strstr(digest , "nonce") + 7;

	n = strlen(pos) - strlen(strstr(pos , "\","));
	*nouce = (gchar*)g_malloc0(n + 1);
	strncpy(*nouce , pos , n);
	(*nouce)[n] = '\0';

	pos = strstr(pos , "key") + 5;
	n = strlen(pos) - strlen(strstr(pos , "\","));
	*key = (gchar*)g_malloc0(n + 1);
	strncpy(*key , pos , n);
	(*key)[n] = '\0';
	purple_debug_info("fetion", "register to sip server success");
	purple_debug_info("fetion", "nonce:%s" , *nouce);
}
static void parse_sms_frequency(xmlNodePtr node , User *user)
{
	xmlChar *res;

	node = node->xmlChildrenNode;
	if(xmlHasProp(node , BAD_CAST "day-limit")){
		res = xmlGetProp(node , BAD_CAST "day-limit");
		user->smsDayLimit = atoi((char*)res);
		xmlFree(res);
	}
	if(xmlHasProp(node , BAD_CAST "day-count")){
		res = xmlGetProp(node , BAD_CAST "day-count");
		user->smsDayCount = atoi((char*)res);
		xmlFree(res);
	}
	if(xmlHasProp(node , BAD_CAST "month-limit")){
		res = xmlGetProp(node , BAD_CAST "month-limit");
		user->smsMonthLimit = atoi((char*)res);
		xmlFree(res);
	}
	if(xmlHasProp(node , BAD_CAST "month-count")){
		res = xmlGetProp(node , BAD_CAST "month-count");
		user->smsMonthCount = atoi((char*)res);
		xmlFree(res);
	}
}

static gint cfg_cb(gpointer data, gint source, const gchar *UNUSED(error_message))
{
	gint n, length = 0;
	gchar msg[BUFLEN * 10], *pos;
	fetion_account *ac = (fetion_account*)data;

	if((n = recv(source, msg, sizeof(msg), 0)) == -1) return -1;
	msg[n] = '\0';
	if(n == 0) {
		/* get cfg failed */
		if(!strstr(ac->data, "HTTP/1.1 200")) {
			g_free(ac->data);
			ac->data = (gchar*)0;
			close(source);
	   		purple_input_remove(ac->conn);
			purple_connection_error_reason(ac->gc,
				       PURPLE_CONNECTION_ERROR_AUTHENTICATION_FAILED,
							       _("Download configuration file error."));
			return -1;

		}
		if(!(pos = strstr(ac->data, "\r\n\r\n"))) return -1;
		pos += 4;
		if(parse_configuration_xml(ac->user, pos) == -1) {
			g_free(ac->data);
			ac->data = (gchar*)0;
			close(source);
	   		purple_input_remove(ac->conn);
			return -1;
		}
		
		g_free(ac->data);
		ac->data = (gchar*)0;
		close(source);
	   	purple_input_remove(ac->conn);

		ac->conn_data = purple_proxy_connect(NULL, ac->account,
							ac->user->sipcProxyIP, ac->user->sipcProxyPort,
							(PurpleProxyConnectFunction)sipc_reg_action, ac);

		if(!ac->conn_data) {
			purple_debug_error("fetion", "connect to ssi server failed");
			purple_connection_error_reason(ac->gc,
				       PURPLE_CONNECTION_ERROR_AUTHENTICATION_FAILED,
							       _("Login Failed."));
			return -1;
		}
	   	return 0;
   	}
	length = ac->data ? strlen(ac->data) : 0;
	ac->data =  (gchar*)realloc(ac->data, length + n + 1);
	memcpy(ac->data + length, msg, n + 1);

	return 0;
}

static gint download_cfg(gpointer data, gint source, const gchar *UNUSED(error_message))
{
	gchar http[BUFLEN] , *body;
	fetion_account *ac = (fetion_account*)data;
	const gchar *uri = "nav.fetion.com.cn";

	body = generate_configuration_body(ac->user);
	snprintf(http, sizeof(http), "POST /nav/getsystemconfig.aspx HTTP/1.1\r\n"
				   "User-Agent: IIC2.0/PC "PROTO_VERSION"\r\n"
				   "Host: %s\r\n"
				   "Connection: Close\r\n"
				   "Content-Length: %ld\r\n\r\n%s",
				   uri , strlen(body) , body);
	g_free(body);
	if(send(source, http, strlen(http), 0) == -1) return -1;
	ac->data = (gchar*)0;
	if((ac->conn = purple_input_add(source, PURPLE_INPUT_READ,
				(PurpleInputFunction)cfg_cb, ac)) == 0) return -1;

	return 0;
}

static gint parse_sipc_auth_response(const gchar *auth_response, User *user, gint *group_count, gint *buddy_count)
{
	gchar *pos;
	xmlChar *buf;
	xmlDocPtr doc;
	xmlNodePtr rootnode;
	xmlNodePtr node;
	xmlNodePtr node1;
	gint code;

	code = fetion_sip_get_code(auth_response);
	user->loginStatus = code;

	if(code == 200){
		fetion_verification_free(user->verification);
		user->verification = NULL;
		purple_debug_info("fetion", "sipc authentication success");
	}else if(code == 421 || code == 420){
		parse_sipc_verification(user , auth_response);
		return 2;
	}else{
		fetion_verification_free(user->verification);
		user->verification = NULL;
		purple_debug_error("fetion", "Sipc authentication failed\n");
		printf("%s\n", auth_response);
		return -1;
	}
	if(!strstr(auth_response, "\r\n\r\n")) return -1;

	pos = strstr(auth_response , "\r\n\r\n") + 4;
	if(!pos) return -1;
	doc = xmlParseMemory(pos , strlen(pos));
	if(!doc) return -1;
	rootnode = xmlDocGetRootElement(doc); 
	node = rootnode->xmlChildrenNode;
	buf = xmlGetProp(node, BAD_CAST "public-ip");
	strcpy(user->publicIp, (gchar*)buf);
	xmlFree(buf);
	buf = xmlGetProp(node, BAD_CAST "last-login-ip");
	strcpy(user->lastLoginIp, (gchar*)buf);
	xmlFree(buf);
	buf = xmlGetProp(node, BAD_CAST "last-login-time");
	strcpy(user->lastLoginTime, (gchar*)buf);
	xmlFree(buf);
	node = node->next;
	node1 = node->xmlChildrenNode;
	parse_personal_info(node1, user);
	node1 = xml_goto_node(node, "custom-config");
	buf = xmlGetProp(node1, BAD_CAST "version");
	strcpy(user->customConfigVersion, (gchar*)buf);
	xmlFree(buf);
	buf = xmlNodeGetContent(node1);
	if(xmlStrlen(buf) > 0){
		user->customConfig = g_malloc0(strlen((gchar*)buf) + 1);
		strcpy(user->customConfig, (gchar*)buf);
	}
	xmlFree(buf);
	node1 = xml_goto_node(node , "contact-list");
	parse_contact_list(node1 , user, group_count, buddy_count);
	node1 = xml_goto_node(node , "chat-friends");
	if(node1) parse_stranger_list(node1 , user);
	
	node1 = xml_goto_node(node , "quota-frequency");
	if(node1) parse_sms_frequency(node1 , user);
	
	xmlFreeDoc(doc);
	return 0;
}

static gchar *generate_aes_key()
{
	gint ret;
	gchar *key = (gchar*)g_malloc0(65);
	FILE *rand_fd = fopen("/dev/urandom", "r");
	if(rand_fd == NULL){
		g_free(key);
		return NULL;
	}
	ret = fread(key, 64, 1, rand_fd);
	if(ret != 1){
		g_free(key);
		fclose(rand_fd);
		return NULL;
	}
	fclose(rand_fd);
	return key;
}
static gchar* generate_auth_body(User* user)
{
	gchar basexml[] = "<args></args>";
	gchar state[5];
	xmlChar* buf = NULL;
	xmlDocPtr doc = NULL;
	xmlNodePtr rootnode = NULL;
	xmlNodePtr node = NULL;
	xmlNodePtr node1 = NULL;

	doc = xmlParseMemory( basexml , strlen(basexml));
	rootnode = xmlDocGetRootElement(doc); 
	node = xmlNewChild(rootnode , NULL , BAD_CAST "device" , NULL);
	xmlNewProp(node , BAD_CAST "machine-code" , BAD_CAST "001676C0E351");
	node = xmlNewChild(rootnode , NULL , BAD_CAST "caps" , NULL);
	xmlNewProp(node , BAD_CAST "value" , BAD_CAST "1ff");
	node = xmlNewChild(rootnode , NULL , BAD_CAST "events" , NULL);
	xmlNewProp(node , BAD_CAST "value" , BAD_CAST "7f");
	node = xmlNewChild(rootnode , NULL , BAD_CAST "user-info" , NULL);
	xmlNewProp(node , BAD_CAST "mobile-no" , BAD_CAST user->mobileno);
	xmlNewProp(node , BAD_CAST "user-id" , BAD_CAST user->userId);
	node1 = xmlNewChild(node , NULL , BAD_CAST "personal" , NULL);
	xmlNewProp(node1 , BAD_CAST "version" , BAD_CAST "0"/*user->personalVersion*/);
	xmlNewProp(node1 , BAD_CAST "attributes" , BAD_CAST "v4default");
	node1 = xmlNewChild(node , NULL , BAD_CAST "custom-config" , NULL);
	xmlNewProp(node1 , BAD_CAST "version" , BAD_CAST "0"/*user->customConfigVersion*/);
	node1 = xmlNewChild(node , NULL , BAD_CAST "contact-list" , NULL);
	xmlNewProp(node1 , BAD_CAST "version" , BAD_CAST "0"/*user->contactVersion*/);
	xmlNewProp(node1 , BAD_CAST "buddy-attributes" , BAD_CAST "v4default");
	node = xmlNewChild(rootnode , NULL , BAD_CAST "credentials" , NULL);
	xmlNewProp(node , BAD_CAST "domains" , BAD_CAST "fetion.com.cn");
	node = xmlNewChild(rootnode , NULL , BAD_CAST "presence" , NULL);
	node1 = xmlNewChild(node , NULL , BAD_CAST "basic" , NULL);
	snprintf(state, sizeof(state) - 1, "%d" , user->state);
	xmlNewProp(node1 , BAD_CAST "value" , BAD_CAST state);
	xmlNewProp(node1 , BAD_CAST "desc" , BAD_CAST "");
	xmlDocDumpMemory(doc , &buf , NULL);
	xmlFreeDoc(doc);
	return xml_convert(buf);
}
static void parse_personal_info(xmlNodePtr node , User* user)
{
	xmlChar *buf;
	char *pos;
	
	buf = xmlGetProp(node , BAD_CAST "version");
	strcpy(user->personalVersion , (char*)buf);
	xmlFree(buf);
	if(xmlHasProp(node , BAD_CAST "sid"))
	{
		buf = xmlGetProp(node , BAD_CAST "sid");
		strcpy(user->sId , (char*)buf);
		xmlFree(buf);
	}
	if(xmlHasProp(node , BAD_CAST "mobile-no"))
	{
		buf = xmlGetProp(node , BAD_CAST "mobile-no");
		if(xmlStrlen(buf)){
			user->boundToMobile = BOUND_MOBILE_ENABLE;
		}else{
			user->boundToMobile = BOUND_MOBILE_DISABLE;
		}
		strcpy(user->mobileno , (char*)buf);
		xmlFree(buf);
	}
	if(xmlHasProp(node , BAD_CAST "carrier-status"))
	{
		buf = xmlGetProp(node , BAD_CAST "carrier-status");
		user->carrierStatus = atoi((char*)buf);
		xmlFree(buf);
	}
	if(xmlHasProp(node , BAD_CAST "nickname"))
	{
		buf = xmlGetProp(node , BAD_CAST "nickname");
		strcpy(user->nickname , (char*)buf);
		xmlFree(buf);
	}
	if(xmlHasProp(node , BAD_CAST "gender"))
	{
		buf = xmlGetProp(node , BAD_CAST "gender");
		user->gender = atoi((char*)buf);
		xmlFree(buf);
	}
	if(xmlHasProp(node , BAD_CAST "sms-online-status"))
	{
		buf = xmlGetProp(node , BAD_CAST "sms-online-status");
		strcpy(user->smsOnLineStatus , (char*)buf);
		xmlFree(buf);
	}
	if(xmlHasProp(node , BAD_CAST "impresa"))
	{
		buf = xmlGetProp(node , BAD_CAST "impresa");
		strcpy(user->impression , (char*)buf);
		xmlFree(buf);
	}
	if(xmlHasProp(node , BAD_CAST "carrier-region"))
	{
		int n;
		buf = xmlGetProp(node , BAD_CAST "carrier-region");
		pos = (char*)buf;
		n = strlen(pos) - strlen(strstr(pos , "."));
		strncpy(user->country , pos , n);
		pos = strstr(pos , ".") + 1;
		n = strlen(pos) - strlen(strstr(pos , "."));
		strncpy(user->province , pos , n);
		pos = strstr(pos , ".") + 1;
		n = strlen(pos) - strlen(strstr(pos , "."));
		strncpy(user->city , pos , n);
		xmlFree(buf);
	}
}
static void parse_contact_list(xmlNodePtr node, User* user,
				int *group_count, int *buddy_count)
{
	xmlChar* buf = NULL;
	xmlNodePtr node1 , node2;
	Group* group = NULL;
	Contact* contact = NULL;
	int hasGroup = 1 , hasBuddy = 1;
	int nr = 0;

	*group_count = 0;
	*buddy_count = 0;

	buf = xmlGetProp(node , BAD_CAST "version");
	purple_debug_info("fetion", "start reading contact list ");
	if(strcmp(user->contactVersion , (char*) buf) == 0)
		return;
	strcpy(user->contactVersion , (char*)buf);
	xmlFree(buf);
	node1 = xml_goto_node(node , "buddy-lists");
	node2 = node1->xmlChildrenNode;
	user->groupCount = 0;

	while(node2 != NULL){
		hasGroup = 1;

		buf = xmlGetProp(node2 , BAD_CAST "id");
		group = fetion_group_list_find_by_id(user->groupList , atoi((char*)buf));
		if(group == NULL){
			hasGroup = 0;
			group = fetion_group_new();
		}
		group->groupid = atoi((char*)buf);
		xmlFree(buf);
		buf = xmlGetProp(node2 , BAD_CAST "name");
		strcpy(group->groupname , (char*)buf);
		xmlFree(buf);

		nr ++;
		group->dirty = 1;
		user->groupCount ++;
		
		if(hasGroup == 0){
			fetion_group_list_append(user->groupList , group);
			hasGroup = 1;
		}
		node2 = node2->next;
	}

	*group_count = nr;
	nr = 0;

	node1 = xml_goto_node(node , "buddies");
	node1 = node1->xmlChildrenNode;
	user->contactCount = 0;

	while(node1 != NULL){
		hasBuddy = 1;

		if(! xmlHasProp(node1 , BAD_CAST "i")){
			node1 = node1->next;
			continue;
		}
		buf = xmlGetProp(node1 , BAD_CAST "i");
		contact = fetion_contact_list_find_by_userid(user->contactList , (char*)buf);
		if(contact == NULL){
			hasBuddy = 0;
			contact = fetion_contact_new();
		}
		strcpy(contact->userId , (char*)buf);
		xmlFree(buf);

		/* maybe a buddy belongs to two groups */
		if(contact->dirty == 1){
			node = node->next;
			continue;
		}

		user->contactCount ++;
		/* set the dirty flags */
		contact->dirty = 1;
		nr ++;

		if(xmlHasProp(node1 , BAD_CAST "n")){
			buf = xmlGetProp(node1 , BAD_CAST "n");
			strcpy(contact->localname , (char*)buf);
			xmlFree(buf);
		}
		if(xmlHasProp(node1 , BAD_CAST "l")){
			buf = xmlGetProp(node1 , BAD_CAST "l");
			contact->groupid = atoi((char*)buf);
			if(xmlStrstr(buf , BAD_CAST ";") != NULL
					|| contact->groupid < 0)
					contact->groupid = 0;
			xmlFree(buf);
		}
		if(xmlHasProp(node1 , BAD_CAST "p")){
			buf = xmlGetProp(node1 , BAD_CAST "p");
			if(strstr((char*)buf , "identity=1") != NULL)
				contact->identity = 1;
			else
				contact->identity = 0;
			xmlFree(buf);
		}
		if(xmlHasProp(node1 , BAD_CAST "r")){
			buf = xmlGetProp(node1 , BAD_CAST "r");
			contact->relationStatus = atoi((char*)buf);
			xmlFree(buf);
		}

		if(xmlHasProp(node1 , BAD_CAST "u")){
			buf = xmlGetProp(node1 , BAD_CAST "u");
			strcpy(contact->sipuri , (char*)buf);
			//if(strstr((char*)buf , "tel") != NULL)
			//	contact->serviceStatus = STATUS_SMS_ONLINE;
			xmlFree(buf);
		}

		strcpy(contact->portraitCrc , "0");

		if(hasBuddy == 0){
			fetion_contact_list_append(user->contactList , contact);
			hasBuddy = 1;
		}
		node1 = node1->next;
	}

	*buddy_count = nr;
	purple_debug_info("fetion", "read contact list complete");
}


static void parse_stranger_list(xmlNodePtr node, User *user)
{
	xmlNodePtr node1 = node->xmlChildrenNode;
	xmlChar *buf = NULL;
	Contact *contact = NULL;
	int hasBuddy;
	while(node1 != NULL) {
		hasBuddy = 1;
		user->contactCount ++;
		buf = xmlGetProp(node1 , BAD_CAST "i");
		contact = fetion_contact_list_find_by_userid(user->contactList , (char*)buf);
		if(contact == NULL){
			hasBuddy = 0;
			contact = fetion_contact_new();
		}
		strcpy(contact->userId , (char*)buf);
		xmlFree(buf);
		buf = xmlGetProp(node1 , BAD_CAST "u");
		strcpy(contact->sipuri , (char*)buf);
		contact->groupid = BUDDY_LIST_STRANGER;
		contact->dirty = 1;

		if(hasBuddy == 0)
			fetion_contact_list_append(user->contactList , contact);
		
		node1 = node1->next;
	}
}
static void parse_ssi_auth_success(xmlNodePtr node , User* user)
{
	gchar *pos;
	pos = (gchar*)xmlGetProp(node , BAD_CAST "uri");
	strcpy(user->sipuri , pos);
	g_free(pos);
	pos = fetion_sip_get_sid_by_sipuri(user->sipuri);
	strcpy(user->sId , pos);
	g_free(pos);
	pos = (gchar*)xmlGetProp(node , BAD_CAST "mobile-no");
	strcpy(user->mobileno , pos);
	g_free(pos);
	pos = (gchar*)xmlGetProp(node , BAD_CAST "user-id");
	strcpy(user->userId , pos);
	g_free(pos);
}
static void parse_ssi_auth_failed(xmlNodePtr node , User* user)
{
	Verification *ver = fetion_verification_new();
	ver->algorithm = (gchar*)xmlGetProp(node, BAD_CAST "algorithm");
	ver->type      = (gchar*)xmlGetProp(node, BAD_CAST "type");
	ver->text      = (gchar*)xmlGetProp(node, BAD_CAST "text");
	ver->tips	   = (gchar*)xmlGetProp(node, BAD_CAST "tips");
	user->verification = ver;
}
static guchar *strtohex(const gchar *in, gint *len)
{
	guchar* out = (guchar*)g_malloc0(strlen(in)/2 );
	gint i = 0 , j = 0 , k = 0 ,length = 0;
	gchar tmp[3] = { 0 };
	gint inlength;
	inlength=(gint)strlen(in);
	while(i < inlength) {
		tmp[k++] = in[i++];
		tmp[k] = '\0';
		if(k == 2) {
			out[j++] = (guchar)strtol(tmp , (gchar**)NULL , 16);
			k = 0;
			length ++;
		}
	}
	if(len != NULL )
		*len = length;
	return out;
}
static gchar *hextostr(const guchar *in, gint len) 
{
	gchar *res = (gchar*)g_malloc0(len * 2 + 1);
	gint reslength;
	gint i = 0;
	while(i < len) {
		sprintf(res + i * 2 , "%02x" , in[i]);
		i ++;
	}
	i = 0;
	reslength=(gint)strlen(res);
	while(i < reslength) {
		res[i] = toupper(res[i]);
		i ++;
	};
	return res;
}
static gchar *hash_password_v1(const guchar *b0, gint b0len, const guchar *password, gint psdlen) 
{
	guchar *dst = (guchar*)g_malloc0(b0len + psdlen + 1);
	guchar tmp[20];
	gchar *res;
	memset(tmp , 0 , sizeof(tmp));
	memcpy(dst , b0 , b0len);
	memcpy(dst + b0len , password , psdlen);
	SHA_CTX ctx;
	SHA1_Init(&ctx);
	SHA1_Update(&ctx , dst , b0len + psdlen );
	SHA1_Final(tmp , &ctx);
	g_free(dst);
	res = hextostr(tmp , 20);
	return res;
}
static gchar *hash_password_v2(const gchar *userid , const gchar *passwordhex) 
{
	gint id = atoi(userid);
	gchar *res;
	guchar *bid = (guchar*)(&id);
	guchar ubid[4];
	gint bpsd_len;
	guchar* bpsd = strtohex(passwordhex , &bpsd_len);
	memcpy(ubid , bid , 4);
	res = hash_password_v1(ubid , sizeof(id) , bpsd , bpsd_len);
	g_free(bpsd);
	return res;
}
static gchar *hash_password_v4(const gchar *userid , const gchar *password)
{
	const gchar *domain = "fetion.com.cn:";
	gchar *res, *dst;
	guchar *udomain = (guchar*)g_malloc0(strlen(domain));
	guchar *upassword = (guchar*)g_malloc0(strlen(password));
	memcpy(udomain, (guchar*)domain, strlen(domain));
	memcpy(upassword, (guchar*)password, strlen(password));
	res = hash_password_v1(udomain, strlen(domain), upassword, strlen(password));
	g_free(udomain);
	g_free(upassword);
	if(userid == NULL || *userid == '\0') return res;
	dst = hash_password_v2(userid , res);
	g_free(res);
	return dst;
}
static gchar *generate_cnouce()
{
	gchar *cnouce = (gchar*)g_malloc0(33);
	sprintf(cnouce , "%04X%04X%04X%04X%04X%04X%04X%04X" , 
			rand() & 0xFFFF , rand() & 0xFFFF , 
			rand() & 0xFFFF , rand() & 0xFFFF ,
			rand() & 0xFFFF , rand() & 0xFFFF,
			rand() & 0xFFFF , rand() & 0xFFFF );
	return cnouce;
}

static gchar *decode_base64(const char* in , int* len)
{
 	unsigned int n , t = 0 , c = 0;
	gchar *res;
	unsigned char out[3];
	unsigned char inp[4];

	n = strlen(in);
	if(n % 4 != 0) {
		purple_debug_error("fetion", "Try to decode a string which is not a base64 string(decode_base64)");
		return NULL;
	}
	n = n / 4 * 3;
	if(len != NULL)	*len = n;
	res = (gchar*)malloc(n);
	memset(res , 0 , n);
	while(1) {
		memset(inp , 0 , 4);
		memset(out , 0 , 3);
		memcpy(inp , in + c , 4);
		c += 4;
		n = EVP_DecodeBlock(out , inp , 4 );
		memcpy(res + t , out , n);
		t += n;
		if(c >= strlen(in))
			break;
	}
	return res;
}

static gchar *generate_configuration_body(User *user)
{
	xmlChar* buf;
	xmlDocPtr doc;
	xmlNodePtr node , cnode;
	gchar body[] = "<config></config>";
	doc = xmlParseMemory(body , strlen(body));
	node = xmlDocGetRootElement(doc);
	cnode = xmlNewChild(node , NULL , BAD_CAST "user" , NULL);

	if(user->loginType == LOGIN_TYPE_FETIONNO)
		xmlNewProp(cnode , BAD_CAST "sid" , BAD_CAST user->sId);
	else
		xmlNewProp(cnode , BAD_CAST "mobile-no" , BAD_CAST user->mobileno);
	
	cnode = xmlNewChild(node , NULL , BAD_CAST "client" , NULL);
	xmlNewProp(cnode , BAD_CAST "type" , BAD_CAST "PC");
	xmlNewProp(cnode , BAD_CAST "version" , BAD_CAST PROTO_VERSION);
	xmlNewProp(cnode , BAD_CAST "platform" , BAD_CAST "W5.1");
	cnode = xmlNewChild(node , NULL , BAD_CAST "servers" , NULL);
	xmlNewProp(cnode , BAD_CAST "version",
				   	BAD_CAST "0");//user->configServersVersion);
	cnode = xmlNewChild(node , NULL , BAD_CAST "parameters" , NULL);
	xmlNewProp(cnode , BAD_CAST "version",
				   	BAD_CAST "0");//user->configParametersVersion);
	cnode = xmlNewChild(node , NULL , BAD_CAST "hints" , NULL);
	xmlNewProp(cnode , BAD_CAST "version",
				   	BAD_CAST "0");//user->configHintsVersion);
	xmlDocDumpMemory(doc , &buf , NULL);
	xmlFreeDoc(doc);	
	return xml_convert(buf);
}

static gint parse_configuration_xml(User *user, const char *xml)
{
	gchar sipcIP[20] , sipcPort[6]; 
	gchar* pos;
	gint n;
	xmlChar* res;
	xmlDocPtr doc;
	xmlNodePtr node;
	xmlNodePtr cnode;

	memset(sipcIP, 0, sizeof(sipcIP));
	memset(sipcPort, 0, sizeof(sipcPort));

	doc = xmlParseMemory(xml, strlen(xml));

	if(!doc) return -1;

	node = xmlDocGetRootElement(doc);
	cnode = xml_goto_node(node, "servers");
	if(cnode && xmlHasProp(cnode, BAD_CAST "version")){
		res = xmlGetProp(cnode, BAD_CAST "version");
		strcpy(user->configServersVersion, (gchar*)res);
		xmlFree(res);
	}
	cnode = xml_goto_node(node, "parameters");
	if(cnode && xmlHasProp(cnode, BAD_CAST "version")){
		res = xmlGetProp(cnode, BAD_CAST "version");
		strncpy(user->configParametersVersion, (gchar*)res, 
			sizeof(user->configParametersVersion));
		xmlFree(res);
	}
	cnode = xml_goto_node(node, "hints");
	if(cnode && xmlHasProp(cnode, BAD_CAST "version")){
		res = xmlGetProp(cnode, BAD_CAST "version");
		strncpy(user->configHintsVersion, (gchar*)res,
			sizeof(user->configHintsVersion));
		xmlFree(res);
	}
	cnode = xml_goto_node(node, "sipc-proxy");
	if(cnode){
		res = xmlNodeGetContent(cnode);
		n = strlen((gchar*)res) - strlen(strstr((gchar*)res , ":"));
		strncpy(user->sipcProxyIP , (gchar*)res , n);
		pos = strstr((gchar*)res , ":") + 1;
		user->sipcProxyPort = atoi(pos);
		xmlFree(res);
	}

	cnode = xml_goto_node(node , "get-uri");
	if(cnode){
		res = xmlNodeGetContent(cnode);
		pos = strstr((gchar*)res , "//") + 2;
		n = strlen(pos) - strlen(strstr(pos , "/"));
		strncpy(user->portraitServerName , pos , n);
		pos = strstr(pos , "/") + 1;
		n = strlen(pos) - strlen(strstr(pos , "/"));
		strncpy(user->portraitServerPath , pos , n);
		xmlFree(res);
	}
	return 0;
}

static gint parse_sipc_verification(User *user, const gchar *str)
{
	gchar *xml;
	gchar w[128];
	gint n = 0;
	xmlDocPtr     doc;
	xmlNodePtr    node;
	xmlChar      *res;
	Verification *ver;

	ver = fetion_verification_new();

	fetion_sip_get_attr(str , "W" , w);
	xml = strstr(w , "algorithm=") + 11;
	n = strlen(xml) - strlen(strstr(xml , "\""));
	ver->algorithm = (gchar*)g_malloc0(n + 1);
	strncpy(ver->algorithm , xml , n);
	xml = strstr(w , "type=") + 6;
	n = strlen(xml) - strlen(strstr(xml , "\""));
	ver->type = (gchar*)g_malloc0(n + 1);
	strncpy(ver->type , xml , n);

	xml = strstr(str , "\r\n\r\n");
	doc = xmlParseMemory(xml , strlen(xml));
	node = xmlDocGetRootElement(doc);
	node = node->xmlChildrenNode;
	res = xmlGetProp(node , BAD_CAST "text");
	n = xmlStrlen(res) + 1;
	ver->text = (gchar*)g_malloc0(n);
	strncpy(ver->text , (char*)res , n - 1);
	xmlFree(res);
	res = xmlGetProp(node , BAD_CAST "tips");
	n = xmlStrlen(res) + 1;
	ver->tips = (gchar*)g_malloc0(n);
	strncpy(ver->tips , (char*)res , n - 1);
	xmlFree(res);

	user->verification = ver;
	return 0;
}

