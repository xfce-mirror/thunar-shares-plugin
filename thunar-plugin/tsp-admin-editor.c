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

#include <libshares/shares.h>
#include <libshares/libshares-xml.h>
#include <libshares/libshares-util.h>

#include "tsp-admin.h"
#include "tsp-admin-editor.h"

#define XML_FILE "admin-dialog-editor.xml"

typedef struct {
	GtkWidget  *dialog;

	GtkWidget  *share_folder;
	GtkWidget  *share_name;
	GtkWidget  *share_comments;
	GtkWidget  *share_write;
	GtkWidget  *share_guest;

	/* Widgets to hide, when edit */
	GtkWidget  *share_label1;
	GtkWidget  *share_hbox1;

	gpointer    admin;
} TspAdminEdit;

static void editor_response_cb (GtkWidget    *widget,
                                gint          response,
                                TspAdminEdit *editor);

static void editor_destroy_cb  (GtkWidget    *widget,
                                TspAdminEdit *editor);

static void editor_set_values  (const gchar  *path,
                                TspAdminEdit *editor);

static void
editor_response_cb (GtkWidget    *widget,
                    gint          response,
                    TspAdminEdit *editor)
{
	if (response == GTK_RESPONSE_OK)
	{
		/* Save changes */
		const gchar *comments;
		const gchar *name;
		ShareInfo   *share_info;
		gchar       *local_file;
		gboolean     is_writable;
		gboolean     guests_ok;

		/* Share file */
		local_file = gtk_file_chooser_get_current_folder (GTK_FILE_CHOOSER (editor->share_folder));
		comments = gtk_entry_get_text (GTK_ENTRY (editor->share_comments));
		name = gtk_entry_get_text (GTK_ENTRY (editor->share_name));
		is_writable = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (editor->share_write));
		guests_ok = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (editor->share_guest));

		share_info = tsp_shares_share (local_file, name, comments,
		 							   is_writable, guests_ok);
		if (share_info)
		{
			shares_free_share_info (share_info);
		} else {
			/* Dunno destroy the dialog */
			return;
		}

		/* Reload share list */
		tsp_admin_load_shares (editor->admin);

		g_free (local_file);
	}
	gtk_widget_destroy (widget);
}

static void
editor_destroy_cb (GtkWidget    *widget,
                   TspAdminEdit *editor)
{
	g_free (editor);
}

/* Displays the main admin dialog */
void
tsp_admin_editor_dialog_show (GtkWindow   *parent,
                              const gchar *path,
                              gpointer     admin)
{
	static TspAdminEdit *editor;
	GtkBuilder          *ui;

	if (editor)
	{
		/* Update values */
		editor_set_values (path, editor);

		/* Bring the dialog to front */
		gtk_window_present (GTK_WINDOW (editor->dialog));
		return;
	}

	editor = g_new0 (TspAdminEdit, 1);
	editor->admin = admin;

	/* Get widgets */
	ui = tsp_xml_get_file (XML_FILE,
						"shares_editor", &editor->dialog,
						 "share_folder", &editor->share_folder,
						 "share_name", &editor->share_name,
						 "share_comments", &editor->share_comments,
						 "share_write", &editor->share_write,
						 "share_guest", &editor->share_guest,
						 "share_hbox1", &editor->share_hbox1,
						 "share_label1", &editor->share_label1,
						  NULL);

	/* Connect signals */
	tsp_xml_connect (ui, editor,
						"shares_editor", "destroy", editor_destroy_cb,
						"shares_editor", "response", editor_response_cb,
						NULL);

	g_object_unref (ui);

	g_object_add_weak_pointer (G_OBJECT (editor->dialog), (gpointer) &editor);

	/* Update values */
	editor_set_values (path, editor);

	/* Show the dialog */
	gtk_window_set_transient_for (GTK_WINDOW (editor->dialog), parent);
	gtk_widget_show (editor->dialog);
}

/* Updates the dialog to edit/add actions */
static void
editor_set_values (const gchar  *path,
                   TspAdminEdit *editor)
{
	if (G_STR_EMPTY (path)){
		const gchar *home_dir;

		gtk_window_set_title (GTK_WINDOW (editor->dialog),
											_("Thunar - Add a share"));

		home_dir = g_get_home_dir ();

		/* Show folders chooser */
		gtk_widget_show (editor->share_hbox1);
		gtk_widget_show (editor->share_label1);

		/* Reset values */
		gtk_file_chooser_set_current_folder (
					GTK_FILE_CHOOSER (editor->share_folder), home_dir);
		gtk_toggle_button_set_active (
					GTK_TOGGLE_BUTTON (editor->share_write), FALSE);
		gtk_toggle_button_set_active (
					GTK_TOGGLE_BUTTON (editor->share_guest), FALSE);
		gtk_entry_set_text (
					GTK_ENTRY (editor->share_name), "");
		gtk_entry_set_text (
					GTK_ENTRY (editor->share_comments), "");

	} else {
		gtk_window_set_title (GTK_WINDOW (editor->dialog),
											_("Thunar - Edit share"));
		/* Load values */
		ShareInfo *share_info;
		gboolean   result;

		/* Hide folders chooser */
		gtk_widget_hide (editor->share_hbox1);
		gtk_widget_hide (editor->share_label1);

		result = shares_get_share_info_for_path (path, &share_info, NULL);
		if (share_info)
		{
			gtk_file_chooser_set_current_folder (
					GTK_FILE_CHOOSER (editor->share_folder), path);
			gtk_toggle_button_set_active (
					GTK_TOGGLE_BUTTON (editor->share_write), share_info->is_writable);
			gtk_toggle_button_set_active (
					GTK_TOGGLE_BUTTON (editor->share_guest), share_info->guest_ok);
			gtk_entry_set_text (
					GTK_ENTRY (editor->share_name), share_info->share_name);
			gtk_entry_set_text (
					GTK_ENTRY (editor->share_comments), share_info->comment);

			shares_free_share_info (share_info);
		}
	}
}
