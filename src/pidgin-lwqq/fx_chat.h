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
#ifndef FX_CHAT_H
#define fX_CHAT_H

typedef struct {
	fetion_account *ac;  
	struct transaction *trans;
} sms_timeout_data;

typedef struct {
	struct transaction *trans;
	gchar *credential;
} invite_data;

typedef struct {
	fetion_account *ac;
	gchar         credential[1024];
} process_invite_data;

void process_message_cb(fetion_account *ac, const gchar *sipmsg);
gint process_invite_cb(fetion_account *ses, const gchar *sipmsg);
void process_left_cb(fetion_account *sec, const gchar *sipmsg);
void process_enter_cb(fetion_account *ses, const gchar *sipmsg);
gint fetion_send_nudge(fetion_account *ses, const gchar *who);
gint fetion_send_sms(fetion_account *ac, const gchar* who, const gchar *msg);
gint fetion_send_sms_to_phone(fetion_account *ac, const gchar *userid, const gchar *msg);
gint new_chat(fetion_account *ac, const gchar *sid, const gchar *what);

#endif
