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
#ifndef FX_BUDDY_H
#define FX_BUDDY_H

typedef struct {
   	guint           conn;
	gint            source;
	gint            size;
	gint            sum;
	guchar         *data;
	Contact        *cnt;
   	fetion_account *ac;
} portrait_trans;

typedef struct {
	Contact        *cnt;
   	fetion_account *ac;
} portrait_data;

portrait_trans *portrait_trans_new();
void portrait_trans_free(portrait_trans *trans);
void fx_blist_init(fetion_account *ac);
void process_presence(fetion_account *ac, const gchar *xml);
gint get_info_cb(fetion_account *ac, const gchar *sipmsg, struct transaction *trans);
void fx_add_buddy(PurpleConnection *gc, PurpleBuddy *buddy,	PurpleGroup *group);
void process_sync_info(fetion_account *ac, const gchar *sipmsg);
void process_add_buddy(fetion_account *ac, const gchar *sipmsg);
gchar *get_city_name(const gchar *province, const gchar *city);
gchar *get_province_name(const gchar *province);

#endif
