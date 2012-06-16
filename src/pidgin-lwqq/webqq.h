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

#ifndef FETION_H
#define FETION_H

#ifdef __cplusplus
extern "C" {
#endif
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <sys/socket.h>
#include <netdb.h>

#include <stdint.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include <unistd.h>
#define _XOPEN_SOURCE
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <dirent.h>
#include <sys/stat.h>
#include <libxml/parser.h>
#include <pthread.h>
#include <openssl/crypto.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/rand.h>

#include "glib/gi18n.h"
#include "blist.h"
#include "debug.h"
#include "account.h"
#include "request.h"

#include "login.h"

#ifdef UNUSED
#elif defined(__GNUC__)
# 	define UNUSED(x) UNUSED_ ## x __attribute__((unused))
#else
#	define UNUSED(x) x
#endif

#define PROTO_VERSION "4.0.2510"
#define LOGIN_TYPE_FETIONNO    			1
#define LOGIN_TYPE_MOBILENO    			0
#define BOUND_MOBILE_ENABLE    			1
#define BOUND_MOBILE_DISABLE   			0
#define BASIC_SERVICE_NORMAL   			1
#define BASIC_SERVICE_ABNORMAL 			0
#define CARRIER_STATUS_OFFLINE			-1
#define CARRIER_STATUS_NORMAL  			0
#define CARRIER_STATUS_DOWN    			1
#define CARRIER_STATUS_CLOSED           2 
#define RELATION_STATUS_AUTHENTICATED   1
#define RELATION_STATUS_UNAUTHENTICATED 0


#define NAV_SERVER "nav.fetion.com.cn"
#define SSI_SERVER "uid.fetion.com.cn"
#define DISPLAY_VERSION "1.1"

#define FETION_NUDGE 0
	
#define BUFLEN 4096

#include "fx_types.h"


gint push_cb(gpointer data, gint source, const gchar *error_message);

xmlNodePtr xml_goto_node(xmlNodePtr node , const gchar *xml);
gchar *xml_convert(xmlChar* in);
struct transaction *transaction_new();
const gchar *get_status_id(const gchar* state);
void transaction_free(struct transaction *trans);
void transaction_set_callid(struct transaction *trans, gint callid);
void transaction_set_userid(struct transaction *trans, const gchar *userid);
void transaction_set_callback(struct transaction *trans, TransCallback callback);
void transaction_set_timeout(struct transaction *trans, GSourceFunc timeout, gpointer data);
void transaction_set_msg(struct transaction *trans, const gchar *msg);
void transaction_wait(fetion_account *ses, struct transaction *trans);
void transaction_wakeup(fetion_account *ses, struct transaction *trans);
void transaction_add(fetion_account *ses, struct transaction *trans);
void transaction_remove(fetion_account *ses, struct transaction *trans);
fetion_account *session_new(PurpleAccount *account);
fetion_account *session_clone(fetion_account *ac);
void session_set_userid(fetion_account *ses, const gchar *userid);
void session_add(fetion_account *ac);
fetion_account *session_find(const gchar *key);
fetion_account *session_find_by_sk(gint sk);
void session_remove(fetion_account *ses);
void session_destroy(fetion_account *ses);


#ifdef __cplusplus
}
#endif

#endif
