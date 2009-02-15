/* vim: set ts=4 sw=8 noet ai nocindent syntax=c: */
/*
 * Copyright (C) 2009 Daniel Morales <daniel@daniel.com.uy>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef __LIBSHARES_MAIN_H__
#define __LIBSHARES_MAIN_H__

#include <glib.h>
#include <libshares/shares.h>
#include <thunarx/thunarx.h>

G_BEGIN_DECLS

#define G_STR_EMPTY(x) ((x) == NULL || (x)[0] == '\0')

gchar     *libshares_get_local_file (ThunarxFileInfo *file);

gboolean   libshares_str_equal      (const char      *txt1,
                                     const char      *txt2);

void       libshares_show_error     (const char      *text,
                                     const char      *secondary);

gboolean   libshares_ask_user       (const char      *text);

void       libshares_monitor_feed   (const gchar *uri);

gboolean   libshares_is_shareable   (ThunarxFileInfo *info);

gboolean   libshares_check_owner    (ThunarxFileInfo *info);

gboolean   libshares_shares_unshare (const gchar     *path);

ShareInfo *libshares_shares_share   (const gchar     *file_local,
                                     const gchar     *name,
                                     const gchar     *comments,
                                     gboolean         is_writable,
                                     gboolean         guests_ok,
                                     const gchar     *old_name);

G_END_DECLS

#endif /*  __LIBSHARES_MAIN_H__ */
