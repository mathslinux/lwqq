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

#ifndef FETION_CONTACT_H
#define FETION_CONTACT_H

#include "fx_types.h"

typedef enum
{
	FETION_NO = 1,
	MOBILE_NO 
} NumberType;

typedef enum
{
	BUDDY_OK = 200 ,
	BUDDY_SAME_USER_DAILY_LIMIT = 486 ,
	BUDDY_USER_EXIST = 521 ,
	BUDDY_BAD_REQUEST = 400
} AddBuddyType;

#define foreach_contactlist(head , cl) \
	for(cl = head ; (cl = cl->next) != head ;)

Contact* fetion_contact_new();
void fetion_contact_list_append(Contact* cl , Contact* contact);
Contact *fetion_contact_list_find_by_userid(Contact* contactlist , const char* userid);
Contact *fetion_contact_list_find_by_mobileno(Contact *contactlist, const char *mobileno);
Contact* fetion_contact_list_find_by_sid(Contact *contactlist , const gchar *sid);
void fetion_contact_list_remove_by_userid(Contact* contactlist , const char* userid);
void fetion_contact_list_remove(Contact *contact);
void fetion_contact_list_free(Contact* contactlist);
gint fetion_contact_has_ungrouped(Contact *contactlist);
gint fetion_contact_has_strangers(Contact *contactlist);
gint fetion_contact_subscribe_only(gint sk, User* user);
gint fetion_contact_get_contact_info(fetion_account *ac, const gchar *userid, TransCallback callback);
Contact* fetion_contact_get_contact_info_by_no(User* user , const char* no , NumberType nt);
int fetion_contact_set_mobileno_permission(User* user , const char* userid , int show);
gint fetion_contact_set_displayname(fetion_account *ac, const gchar *userid, const gchar *name);
gint fetion_contact_move_to_group(fetion_account *ac, const gchar *userid, gint buddylist);
gint fetion_contact_delete_buddy(fetion_account *ac, const gchar *userid);
Contact* fetion_contact_add_buddy(User* user , const char* no
					 , NumberType notype , int buddylist
					  , const char* localname , const char* desc
					  , int phraseid , int* statuscode);

Contact* fetion_contact_handle_contact_request(User* user, const char* sipuri
					   , const char* userid , const char* localname
					   , int buddylist , int result);
#endif
