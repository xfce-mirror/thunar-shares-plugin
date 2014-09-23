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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <gio/gio.h>

#include <glib/gstdio.h>
#include <gtk/gtk.h>
#include <glib/gi18n-lib.h>

#include <thunarx/thunarx.h>

#include "libshares-util.h"
#include "shares.h"

static gboolean tsp_ask_perms   (void);

static gboolean tsp_check_perms (const gchar *path,
                                 gboolean     is_writable,
                                 gboolean     guests_ok);

/**
 * libshares_get_local_file:
 * @filex: Thunarx File Info
 *
 * Gets a local file from the thunar-x file info.
 *
 * Return value: A newly allocated string containing the absolute
 * path. The returned string should be freed when no longer needed. 
 **/
gchar*
libshares_get_local_file (ThunarxFileInfo *filex)
{
  gchar *file, *file_local;

  file = thunarx_file_info_get_uri (filex);
  file_local = g_filename_from_uri (file, NULL, NULL);

  g_free (file);
  return file_local;
}

/**
 * libshares_str_equal:
 * @txt1: String #1 to compare.
 * @txt2: String #2 to compare.
 *
 * Compares two strings. Is NULL-safe.
 *
 * Return value: #TRUE if both strings are equal.
 **/
gboolean
libshares_str_equal (const char *txt1,
                     const char *txt2)
{
  if (G_STR_EMPTY (txt1) || G_STR_EMPTY (txt2))
  {
    if (G_STR_EMPTY (txt1) && G_STR_EMPTY (txt2)){
      return TRUE;
    }
    return FALSE;
  }

  return g_str_equal (txt1, txt2);
}

/**
 * libshares_shares_unshare:
 * @path: Absolute path of the folder.
 *
 * UnShare a folder.
 *
 * Return value: #TRUE if the folder was unshared correctly.
 **/
gboolean
libshares_shares_unshare (const gchar *path)
{
  gboolean ret = FALSE;
  gboolean is_shared;
  gboolean result;
  GError  *error = NULL;

  /* Check if this path is shared */
  result = shares_get_path_is_shared (path, &is_shared, &error);

  /* Check error */
  if (!result)
  {
    libshares_show_error (NULL, error->message);
    g_error_free (error);
    error = NULL;
  }

  if (is_shared)
  {
    /* Un-share it */
    ret = shares_modify_share (path, NULL, &error);
    if (!ret){
      libshares_show_error (NULL, error->message);
      g_error_free (error);
    } else {
      ret = TRUE;
    }
  }

  return ret;
}

/**
 * libshares_shares_share:
 * @file_local: Absolute path of the folder.
 * @name: A name for the Share. Must be unique and < 12 chars long.
 * @comments: Folder comment (optional)
 * @is_writable: Allow write access.
 * @guests_ok: Allow guest access.
 * @old_name: Optional. Set this when editing info of a share.
 *
 * Shares a folder.
 *
 * Return value: A new ShareInfo struct if the folder was shared
 * correctly, or NULL if error. Free this with shares_free_share_info ().
 **/
ShareInfo *
libshares_shares_share (const gchar  *file_local,
                        const gchar  *name,
                        const gchar  *comments,
                        gboolean      is_writable,
                        gboolean      guests_ok,
                        const gchar  *old_name)
{
  ShareInfo *share_info = NULL;
  gboolean   exists;
  gboolean   ret;
  GError    *err = NULL;

  /* Check share name */
  if (G_STR_EMPTY (name)){
    libshares_show_error (NULL, _("Please, write a name."));
    return NULL;
  }

  /* Check length */
  if (g_utf8_strlen (name, -1) > 12)
  {
    /* Warn the user */
    if (!libshares_ask_user (_("Share name too long. Some old clients may have problems to access it, continue anyway?")))
      return NULL;
  }

  /* Do the name check only if this is a new share, or if
     the user is changing the share name */
  if ((old_name == NULL) || (g_utf8_collate (name, old_name) != 0))
  {
    /* Check if the share name is already used */
    if (!shares_get_share_name_exists (name, &exists, &err))
    {
      gchar *str;

      str = g_strdup_printf (_("Error while getting share information: %s"), err->message);
      libshares_show_error (NULL, str);
      g_free (str);
      g_error_free (err);

      return NULL;
    }

    if (exists)
    {
      libshares_show_error (NULL, _("Another share has the same name"));
      return NULL;
    }
  }

  if (tsp_check_perms (file_local, is_writable, guests_ok))
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
    if (!ret)
    {
      libshares_show_error (NULL, err->message);
      g_error_free (err);
      shares_free_share_info (share_info);
      share_info = NULL;
    }
  }

  return share_info;
}

/**
 * libshares_show_error:
 * @text: Main text (the bold one).
 * @secondary: Secondary text. (error details)
 *
 * Displays an error dialog message :(.
 *
 **/
void
libshares_show_error (const char *text,
                      const char *secondary)
{
  GtkWidget  *dialog;

  dialog = gtk_message_dialog_new (NULL,
                                   GTK_DIALOG_MODAL,
                                   GTK_MESSAGE_ERROR,
                                   GTK_BUTTONS_CLOSE,
                                   NULL);
  if (!text)
    text = _("Cannot modify the share:");

  g_object_set (G_OBJECT (dialog), "secondary-text", secondary, "text", text, NULL);

  gtk_dialog_run (GTK_DIALOG (dialog));

  gtk_widget_destroy (dialog);
}

/**
 * libshares_ask_user:
 * @text: Question :)
 *
 * Asks 'text' to the user.
 *
 * Return value: #TRUE if the users marks OK.
 **/
gboolean
libshares_ask_user (const char *text)
{
  GtkWidget *dialog;
  gboolean result;

  dialog = gtk_message_dialog_new (NULL,
                                   GTK_DIALOG_MODAL,
                                   GTK_MESSAGE_QUESTION,
                                   GTK_BUTTONS_OK_CANCEL,
                                   NULL);

  g_object_set (G_OBJECT (dialog), "text", text, NULL);

  result = gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_OK;

  gtk_widget_destroy (dialog);

  return result;
}

/**
 * libshares_is_shareable:
 * @info: FileInfo
 *
 * Checks if the fileinfo is shareable. (Folder, and local files checks).
 *
 * Return value: #TRUE is shareable.
 **/
gboolean
libshares_is_shareable (ThunarxFileInfo *info)
{
  gboolean retval;
  GFile   *file;

  if (!thunarx_file_info_is_directory (info)){
    return FALSE;
  }

  file = thunarx_file_info_get_location (info);

  retval = g_file_is_native (file);

  g_object_unref (file);

  return retval;
}

/**
 * libshares_check_owner:
 * @info: FileInfo
 *
 * Checks the owner only param, and the owner of the file.
 *
 * Return value: #TRUE is shareable.
 **/
gboolean
libshares_check_owner (ThunarxFileInfo *info)
{
  GFileInfo *fileinfo;
  gboolean   retval = TRUE;
  gboolean   owner_only;
  guint32    uid;

  if (shares_has_owner_only (&owner_only, NULL))
  {
    if (owner_only)
    {
      fileinfo = thunarx_file_info_get_file_info (info);

      uid = g_file_info_get_attribute_uint32 (fileinfo, G_FILE_ATTRIBUTE_UNIX_UID);

      retval = (geteuid () == uid);

      g_object_unref (fileinfo);
    }
  }

  return retval;
}

/* Asks to the user if we can change the permissions of the folder */
static gboolean
tsp_ask_perms (void)
{
  return libshares_ask_user (_("Thunar needs to add some permissions to your folder in order to share it. Do you agree?"));
}

/* Checks if the current file has the necesary permissions */
static gboolean
tsp_check_perms (const gchar *path,
                 gboolean     is_writable,
                 gboolean     guests_ok)
{
  struct stat st;
  mode_t      new_mode;
  mode_t      mode;
  mode_t      need_mask;

  if (g_stat (path, &st) != 0)
    return FALSE;

  new_mode = mode = st.st_mode;

  /* go+rx is necesary to guest enabled shares */
  if (guests_ok)
    new_mode |= (S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);

  /* go+w is necesary to writable shares */
  if (is_writable)
    new_mode |= (S_IWGRP | S_IWOTH);

  /* Compare both modes */
  need_mask = new_mode & ~mode;

  if (need_mask != 0)
  {
    if (!tsp_ask_perms ())
      return FALSE;
#ifdef G_ENABLE_DEBUG
    g_message ("Changing permissions of '%s'", path);
#endif
    if (g_chmod (path, new_mode) != 0)
    {
      libshares_show_error (NULL, _("Error when changing folder permissions."));
      return FALSE;
    }
  }
  return TRUE;
}
