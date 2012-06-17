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

#ifndef FETION_USER_H
#define FETION_USER_H

#include "fx_types.h"

#define MODIFY_INFO_NICKNAME 0
#define MODIFY_INFO_IMPRESA  1

#define USER_AUTH_NEED_CONFIRM(u) ((u)->loginStatus == 421 || (u)->loginStatus == 420)
#define USER_AUTH_ERROR(u)        ((u)->loginStatus == 401 || (u)->loginStatus == 400 || (u)->loginStatus == 404)

User *fetion_user_new(const gchar *no , const gchar *password);
void fetion_user_set_userid(User* user , const char* userid);
void fetion_user_set_sid(User* user , const char* sId);
void fetion_user_set_mobileno(User* user , const char* mobileno);
void fetion_user_set_verification_code(User* user , const char* code);
void fetion_user_free(User* user);
gint fetion_user_set_state(fetion_account *ac, gint state);
gint fetion_user_keep_alive(fetion_account *ac);
gint fetion_sms_myself(fetion_account *ac, const gchar *msg);
gint fetion_modify_info(fetion_account *ac, gint info_type, const gchar *value);
#define foreach_grouplist(head , gl) for(gl = head ; (gl = gl->next) != head ;)
Group* fetion_group_new();
void fetion_group_list_append(Group* head , Group* group);
void fetion_group_list_prepend(Group* head , Group* group);
void fetion_group_list_remove(Group *group);
void fetion_group_remove(Group* head , int groupid);
Group *fetion_group_list_find_by_id(Group *head, gint id);
Group *fetion_group_list_find_by_name(Group *head, const gchar *name);
Verification* fetion_verification_new();
void fetion_verification_free(Verification* ver);
Contact *fetion_user_parse_presence_body(const gchar *body , User* user);
Contact *fetion_user_parse_syncuserinfo_body(const gchar *body , User* user);
gint fetion_user_set_sms_status(User *user , int days);
static inline void fetion_user_set_st(User *user, int state){ user->state = state; }

#endif
