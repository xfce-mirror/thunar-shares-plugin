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
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gtk/gtk.h>
#include <glib/gi18n.h>

#include <thunarx/thunarx.h>

#include <libshares/shares.h>
#include <libshares/libshares-util.h>

#include "tsp-admin.h"
#include "tsp-admin-editor.h"

static void tsp_admin_editor_class_init (TspAdminEditorClass  *klass);
static void tsp_admin_editor_init       (TspAdminEditor       *editor);
static void tsp_admin_editor_finalize   (GObject              *object);

static void tsp_admin_editor_response   (GtkDialog            *dialog,
                                         gint                  response);

static gboolean tsp_admin_editor_save_changes (TspAdminEditor *editor);

static void tsp_admin_editor_set_path   (TspAdminEditor       *editor,
                                         const char           *path);

struct _TspAdminEditorClass
{
  GtkDialogClass __parent__;
};

struct _TspAdminEditor
{
  GtkDialog   __parent__;

  GtkWidget  *share_folder;
  GtkWidget  *share_folder_label;
  GtkWidget  *share_name;
  GtkWidget  *share_comments;
  GtkWidget  *share_write;
  GtkWidget  *share_guest;

  /* Current share name */
  gchar      *old_name;
};

THUNARX_DEFINE_TYPE (TspAdminEditor, tsp_admin_editor, GTK_TYPE_DIALOG);

static void
tsp_admin_editor_class_init (TspAdminEditorClass *klass)
{
  GtkDialogClass *gtkdialog_class;
  GObjectClass   *gobject_class;

  gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->finalize = tsp_admin_editor_finalize;

  gtkdialog_class = GTK_DIALOG_CLASS (klass);
  gtkdialog_class->response = tsp_admin_editor_response;
}

static void
tsp_admin_editor_init (TspAdminEditor *editor)
{
  GtkWidget *vbox;
  GtkWidget *table;
  GtkWidget *widget;
  gboolean   guest_ok = FALSE;

  vbox = GTK_WIDGET (GTK_DIALOG (editor)->vbox);

  /* Dialog setup */
  gtk_window_set_title (GTK_WINDOW (editor), _("Thunar - Add a share"));
  gtk_dialog_set_has_separator (GTK_DIALOG (editor), FALSE);
  gtk_window_set_destroy_with_parent (GTK_WINDOW (editor), TRUE);
  gtk_window_set_modal (GTK_WINDOW (editor), TRUE);
  gtk_window_set_position (GTK_WINDOW (editor), GTK_WIN_POS_CENTER_ON_PARENT);
  gtk_window_set_type_hint (GTK_WINDOW (editor), GDK_WINDOW_TYPE_HINT_DIALOG);

  /* Dialog responses and buttons */
  gtk_dialog_add_button (GTK_DIALOG (editor), GTK_STOCK_OK, GTK_RESPONSE_OK);
  gtk_dialog_set_default_response (GTK_DIALOG (editor), GTK_RESPONSE_CLOSE);

  /* Main container */
  table = gtk_table_new (5, 2, FALSE);
  gtk_table_set_col_spacings (GTK_TABLE (table), 5);
  gtk_box_pack_start (GTK_BOX(vbox), table, TRUE, TRUE, 0);

  /* Folder chooser */
  editor->share_folder_label = gtk_label_new (_("Folder:"));
  gtk_misc_set_padding (GTK_MISC (editor->share_folder_label), 3, 0);
  gtk_misc_set_alignment (GTK_MISC (editor->share_folder_label), 0.0, 0.50);
  gtk_table_attach (GTK_TABLE (table), editor->share_folder_label, 0, 1, 0, 1, GTK_EXPAND|GTK_FILL, GTK_FILL, 0, 5);

  editor->share_folder = gtk_file_chooser_button_new (NULL, GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER);
  gtk_table_attach (GTK_TABLE (table), editor->share_folder, 1, 2, 0, 1, GTK_EXPAND|GTK_FILL, GTK_FILL, 0, 5);

  /* Name */
  widget = gtk_label_new (_("Share name:"));
  gtk_misc_set_padding (GTK_MISC (widget), 3, 0);
  gtk_misc_set_alignment (GTK_MISC (widget), 0.0, 0.50);
  gtk_table_attach (GTK_TABLE (table), widget, 0, 1, 1, 2, GTK_EXPAND|GTK_FILL, GTK_FILL, 0, 5);

  editor->share_name = gtk_entry_new ();
  gtk_table_attach (GTK_TABLE (table), editor->share_name, 1, 2, 1, 2, GTK_EXPAND|GTK_FILL, GTK_FILL, 0, 5);

  /* Write access */
  editor->share_write = gtk_check_button_new_with_label (_("Allow others users to write in this folder"));
  gtk_table_attach (GTK_TABLE (table), editor->share_write, 0, 2, 2, 3, GTK_EXPAND|GTK_FILL, GTK_FILL, 0, 5);

  /* Guests access */
  editor->share_guest = gtk_check_button_new_with_label (_("Allow Guest access"));
  gtk_table_attach (GTK_TABLE (table), editor->share_guest, 0, 2, 3, 4, GTK_EXPAND|GTK_FILL, GTK_FILL, 0, 5);

  /* Comments */
  widget = gtk_label_new (_("Comments:"));
  gtk_misc_set_padding (GTK_MISC (widget), 3, 0);
  gtk_misc_set_alignment (GTK_MISC (widget), 0.0, 0.50);
  gtk_table_attach (GTK_TABLE (table), widget, 0, 1, 4, 5, GTK_EXPAND|GTK_FILL, GTK_FILL, 0, 5);

  editor->share_comments = gtk_entry_new ();
  gtk_table_attach (GTK_TABLE (table), editor->share_comments, 1, 2, 4, 5, GTK_EXPAND|GTK_FILL, GTK_FILL, 0, 5);

  /* Show all */
  gtk_widget_show_all (table);

  /* Check if guest access is supported */
  shares_supports_guest_ok (&guest_ok, NULL);
  if (!guest_ok){
    gtk_widget_hide (editor->share_guest);
  }
}

static void
tsp_admin_editor_finalize (GObject *object)
{
  TspAdminEditor *editor = TSP_ADMIN_EDITOR (object);

  /* Cleanup... */
  if (editor->old_name != NULL)
    g_free (editor->old_name);

  (*G_OBJECT_CLASS (tsp_admin_editor_parent_class)->finalize) (object);
}

static void
tsp_admin_editor_response (GtkDialog *dialog,
                           gint       response)
{
  if (response == GTK_RESPONSE_CLOSE){

    gtk_widget_destroy (GTK_WIDGET (dialog));

  } else if (response == GTK_RESPONSE_OK){

    if (tsp_admin_editor_save_changes (TSP_ADMIN_EDITOR (dialog))){
      gtk_widget_destroy (GTK_WIDGET (dialog));
    }

  } else if (GTK_DIALOG_CLASS (tsp_admin_editor_parent_class)->response != NULL){

    (*GTK_DIALOG_CLASS (tsp_admin_editor_parent_class)->response) (dialog, response);

  }
}

static gboolean
tsp_admin_editor_save_changes (TspAdminEditor *editor)
{
  const gchar *comments;
  const gchar *name;
  ShareInfo   *share_info;
  gchar       *local_file;
  gboolean     is_writable;
  gboolean     guests_ok;

  local_file = gtk_file_chooser_get_current_folder (GTK_FILE_CHOOSER (editor->share_folder));
  comments = gtk_entry_get_text (GTK_ENTRY (editor->share_comments));
  name = gtk_entry_get_text (GTK_ENTRY (editor->share_name));
  is_writable = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (editor->share_write));
  guests_ok = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (editor->share_guest));

  share_info = libshares_shares_share (local_file,
                                       name,
                                       comments,
                                       is_writable,
                                       guests_ok,
                                       editor->old_name);

  g_free (local_file);

  if (share_info)
  {
    GtkWindow *manager;

    shares_free_share_info (share_info);

    /* Reload share list */
    manager = gtk_window_get_transient_for (GTK_WINDOW (editor));
    
    if (manager != NULL && TSP_ADMIN_IS_MANAGER (manager))
      tsp_admin_manager_reload_shares (TSP_ADMIN_MANAGER (manager));

    return TRUE;
  }

  return FALSE;
}

static void
tsp_admin_editor_set_path (TspAdminEditor *editor,
                           const char     *path)
{
  ShareInfo *share_info;
  gboolean   result;
  GError    *error = NULL;

  /* Dialog changes */
  gtk_widget_hide (editor->share_folder);
  gtk_widget_hide (editor->share_folder_label);
  gtk_window_set_title (GTK_WINDOW (editor), _("Thunar - Edit share"));

  /* Load share info */
  result = shares_get_share_info_for_path (path, &share_info, &error);

  /* Check error */
  if (!result)
  {
    libshares_show_error (_("There was an error while listing shares"), error->message);
    g_error_free (error);
  }

  if (share_info)
  {
    editor->old_name = g_strdup (share_info->share_name);

    gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (editor->share_folder), path);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (editor->share_write), share_info->is_writable);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (editor->share_guest), share_info->guest_ok);
    gtk_entry_set_text (GTK_ENTRY (editor->share_name), share_info->share_name);
    gtk_entry_set_text (GTK_ENTRY (editor->share_comments), share_info->comment);

    shares_free_share_info (share_info);
  }
}

GtkWidget *
tsp_admin_editor_new (TspAdminManager *manager)
{
  GtkWidget *dialog;

  dialog = g_object_new (TSP_ADMIN_TYPE_EDITOR, NULL);

  if (manager != NULL)
    gtk_window_set_transient_for (GTK_WINDOW (dialog), GTK_WINDOW (manager));

  return dialog;
}

GtkWidget *
tsp_admin_editor_new_with_path (TspAdminManager *manager,
                                const char      *path)
{
  GtkWidget *dialog = tsp_admin_editor_new (manager);

  if (path != NULL)
    tsp_admin_editor_set_path (TSP_ADMIN_EDITOR (dialog), path);

  return dialog;
}
