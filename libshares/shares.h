/* nautilus-share -- Nautilus File Sharing Extension
 *
 * Sebastien Estienne <sebastien.estienne@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 * (C) Copyright 2005 Ethium, Inc.
 */

#ifndef SHARES_H
#define SHARES_H

#include <glib.h>

typedef struct {
	char *path;
	char *share_name;
	char *comment;
	gboolean is_writable;
	gboolean guest_ok;
} ShareInfo;

#define SHARES_ERROR (shares_error_quark ())

typedef enum {
	SHARES_ERROR_FAILED,
	SHARES_ERROR_NONEXISTENT
} SharesError;

void free_all_shares (void);

GQuark shares_error_quark (void);

void shares_free_share_info (ShareInfo *info);

gboolean shares_get_path_is_shared (const char *path, gboolean *ret_is_shared, GError **error);

gboolean shares_get_share_info_for_path (const char *path, ShareInfo **ret_share_info, GError **error);

gboolean shares_get_share_name_exists (const char *share_name, gboolean *ret_exists, GError **error);

gboolean shares_get_share_info_for_share_name (const char *share_name, ShareInfo **ret_share_info, GError **error);

gboolean shares_modify_share (const char *old_path, ShareInfo *info, GError **error);

gboolean shares_get_share_info_list (GSList **ret_info_list, GError **error);

void shares_free_share_info_list (GSList *list);

gboolean shares_supports_guest_ok (gboolean *supports_guest_ok_ret, 
				   GError **error);

gboolean shares_has_owner_only (gboolean *supports_owner_only_ret, 
				   GError **error);

void shares_set_debug (gboolean error_on_refresh,
		       gboolean error_on_add,
		       gboolean error_on_modify,
		       gboolean error_on_remove);

#endif
