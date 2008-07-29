/* vim: set ts=4 sw=8 noet ai nocindent syntax=c: */
/*
 * Copyright (C) 2008 Daniel Morales <daniel@daniel.com.uy>
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <glib/gstdio.h>
#include <gtk/gtk.h>

#include <thunar-vfs/thunar-vfs.h>
#include <thunarx/thunarx.h>

#include "libshares-util.h"
#include "libshares-xml.h"
#include "shares.h"

#define XML_FILE "dialogs.xml"

static gboolean tsp_ask_perms   (gboolean     need_r,
                                 gboolean     need_w,
                                 gboolean     need_x);

static gboolean tsp_check_perms (const gchar *path,
                                 gboolean     is_writable);

/* Gets a local file from the thunar-x file info*/
gchar*
tsp_get_local_file (ThunarxFileInfo *filex)
{
	gchar *file, *file_local;

	file = thunarx_file_info_get_uri (filex);
	file_local = g_filename_from_uri (file, NULL, NULL);

	g_free (file);
	return file_local;
}

/* Safe string comparison */
gboolean
tsp_str_equal (const char *txt1,
               const char *txt2)
{
	if (G_STR_EMPTY (txt1) || G_STR_EMPTY (txt2)){
		if (G_STR_EMPTY (txt1) && G_STR_EMPTY (txt2)){
			return TRUE;
		}
		return FALSE;
	}

	return g_str_equal (txt1, txt2);
}

/* Un-share a folder */
gboolean
tsp_shares_unshare (const gchar *path)
{
	gboolean ret = FALSE;
	gboolean is_shared;
	GError  *err = NULL;

	/* Check if this path is shared */
	shares_get_path_is_shared (path, &is_shared, NULL);
	if (is_shared)
	{
		/* Un-share it */
		ret = shares_modify_share (path, NULL, &err);
		if (!ret){
			tsp_show_error (NULL, err->message);
			g_error_free (err);
		} else {
			ret = TRUE;
		}
	}

	return ret;
}

/* Share a folder */
ShareInfo *
tsp_shares_share (const gchar  *file_local,
                  const gchar  *name,
                  const gchar  *comments,
                  gboolean      is_writable,
                  gboolean      guests_ok)
{
	ShareInfo *share_info = NULL;
	gboolean   ret;
	GError    *err = NULL;

	/* Check share name */
	if (G_STR_EMPTY (name)){
		tsp_show_error (NULL, _("Please, write a name."));
		return NULL;
	}

	/* TODO: check if the share name is already used(?) */

	if (tsp_check_perms (file_local, is_writable))
	{
		share_info = g_new0 (ShareInfo, 1);

		/* Fill the struct */
		share_info->path = g_strdup (file_local);
		share_info->share_name = g_strdup (name);
		if (G_STR_EMPTY (comments)){
			share_info->comment = g_strdup ("");
		} else {
			share_info->comment = g_strdup (comments);
		}
		share_info->is_writable = is_writable;
		share_info->guest_ok = guests_ok;

		/* Share it */
		ret = shares_modify_share (file_local, share_info, &err);
		if (!ret){
			tsp_show_error (NULL, err->message);
			g_error_free (err);
			shares_free_share_info (share_info);
			share_info = NULL;
		}
	}
	
	return share_info;
}

/* Displays an error message :(*/
void
tsp_show_error (const char *text,
                const char *secondary)
{
	GtkBuilder *ui;
	GtkWidget  *dialog;

	ui = tsp_xml_get_file (XML_FILE, 
				"error_dialog", &dialog,
				NULL);

	g_object_set (G_OBJECT (dialog), "secondary-text", secondary, NULL);

	if (text){
		gtk_message_dialog_set_markup (GTK_MESSAGE_DIALOG (dialog), text);
	} else {
		gtk_message_dialog_set_markup (GTK_MESSAGE_DIALOG (dialog), _("Cannot modify the share:"));
	}

	gtk_dialog_run (GTK_DIALOG (dialog));

	gtk_widget_destroy (dialog);
	g_object_unref (ui);
}

/* Asks to the user if we can change the permissions of the folder */
static gboolean
tsp_ask_perms (gboolean  need_r,
               gboolean  need_w,
               gboolean  need_x)
{
	GtkBuilder *ui;
	GtkWidget  *dialog;
	gboolean    result;

	/* Get the dialogs */
	ui = tsp_xml_get_file (XML_FILE, 
				"permissions_dialog", &dialog,
				NULL);

	result = gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_OK;

	gtk_widget_destroy (dialog);
	g_object_unref (ui);

	return result;
}

/* Checks if the current file has the necesary permissions */
static gboolean
tsp_check_perms (const gchar *path,
                 gboolean     is_writable)
{
	struct stat st;
	gboolean    need_r;
	gboolean    need_w;
	gboolean    need_x;
	mode_t      new_mode;
	mode_t      mode;

	if (stat (path, &st) != 0)
		return FALSE;

	mode = st.st_mode;

	new_mode = mode;

	need_r = (mode & THUNAR_VFS_FILE_MODE_OTH_READ) == 0;
	new_mode |= THUNAR_VFS_FILE_MODE_OTH_READ;

	need_w = is_writable && (mode & THUNAR_VFS_FILE_MODE_OTH_WRITE) == 0;
	if (need_w)
		new_mode |= THUNAR_VFS_FILE_MODE_OTH_WRITE;

	need_x = (mode & THUNAR_VFS_FILE_MODE_OTH_EXEC) == 0;
	new_mode |= THUNAR_VFS_FILE_MODE_OTH_EXEC;

	if (need_r || need_w || need_x)
	{
		if (!tsp_ask_perms (need_r, need_w, need_x))
			return FALSE;
#ifdef G_ENABLE_DEBUG
		g_message ("Changing permissions of '%s'", path);
#endif
		if (g_chmod (path, new_mode) != 0)
		{
			tsp_show_error (NULL, _("Error when changing folder permissions."));
			return FALSE;
		}
    }
	return TRUE;
}
