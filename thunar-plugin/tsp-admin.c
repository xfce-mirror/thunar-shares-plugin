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

#include <libshares/shares.h>
#include <libshares/libshares-xml.h>
#include <libshares/libshares-util.h>

#include "tsp-admin.h"
#include "tsp-admin-editor.h"

#define XML_FILE "admin-dialog.xml"

#define SHARES_ICON_NAME "gnome-fs-share"

enum {
	TSP_ADMIN_COL_ICON = 0,
	TSP_ADMIN_COL_TXT,
	TSP_ADMIN_COL_PATH
};

typedef struct {
	GtkWidget  *dialog;

	GtkWidget  *share_list;
	GtkWidget  *share_edit;
	GtkWidget  *share_remove;
} TspAdmin;

static void admin_response_cb    (GtkWidget  *widget,
                                  gint        response,
                                  TspAdmin   *admin);

static void admin_destroy_cb     (GtkWidget  *widget,
                                  TspAdmin   *admin);

static void admin_selection_cb   (GtkTreeSelection *selection,
                                  TspAdmin         *admin);

static void admin_add_cb         (GtkButton  *button,
                                  TspAdmin   *admin);
static void admin_edit_cb        (GtkButton  *button,
                                  TspAdmin   *admin);
static void admin_remove_cb      (GtkButton  *button,
                                  TspAdmin   *admin);

static void admin_shares_foreach (gpointer    data,
                                  gpointer    user_data);

static void
admin_response_cb (GtkWidget *widget,
                   gint       response,
                   TspAdmin  *admin)
{
	if (response == GTK_RESPONSE_OK)
	{
		/* Bleh.. */
	}
	gtk_widget_destroy (widget);
}

static void
admin_destroy_cb (GtkWidget *widget,
                  TspAdmin  *admin)
{
	g_free (admin);
}

/* Adding a shared folder */
static void
admin_add_cb (GtkButton *button,
              TspAdmin  *admin)
{
	tsp_admin_editor_dialog_show (GTK_WINDOW (admin->dialog), NULL, admin);
}

/* Editing a shared folder */
static void
admin_edit_cb (GtkButton *button,
               TspAdmin  *admin)
{
	GtkTreeSelection *selection;
	GtkTreeModel     *model;
	GtkTreeIter       iter;
	gchar            *path;

	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (admin->share_list));
	if (gtk_tree_selection_get_selected (selection, &model, &iter))
	{
		gtk_tree_model_get (model, &iter, TSP_ADMIN_COL_PATH, &path, -1);

		/* Run the editor dialog */
		tsp_admin_editor_dialog_show (GTK_WINDOW (admin->dialog), path, admin);

		g_free (path);
	}
}

/* Removing a shared folder */
static void
admin_remove_cb (GtkButton *button,
                 TspAdmin  *admin)
{
	GtkTreeSelection *selection;
	GtkTreeModel     *model;
	GtkTreeIter       iter;
	gchar            *path;

	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (admin->share_list));
	if (gtk_tree_selection_get_selected (selection, &model, &iter))
	{
		gtk_tree_model_get (model, &iter, TSP_ADMIN_COL_PATH, &path, -1);
		
		/* Remove from shares */
		if (tsp_shares_unshare (path)){
			gtk_list_store_remove (GTK_LIST_STORE (model), &iter);
		}

		g_free (path);
	}
}

/* Changing the folder selection */
static void
admin_selection_cb (GtkTreeSelection *selection,
                    TspAdmin         *admin)
{
	GtkTreeModel *model;
	GtkTreeIter   iter;
	gboolean      has_selection;

	has_selection = gtk_tree_selection_get_selected (selection, &model, &iter);

	/* Update buttons sensitivity */
	gtk_widget_set_sensitive (admin->share_edit, has_selection);
	gtk_widget_set_sensitive (admin->share_remove, has_selection);
}

/* Add a share to the list view */
static void
admin_shares_foreach (gpointer  data,
                      gpointer  user_data)
{
	GtkListStore *store = GTK_LIST_STORE (user_data);
	GtkTreeIter   iter;
	ShareInfo    *share = (ShareInfo *)data;
	gchar        *text_string;

	gtk_list_store_append (store, &iter);

	text_string = g_strdup_printf ("<b>%s</b>\n<small>%s</small>",
									share->share_name, share->path);

	gtk_list_store_set (store, &iter,
						TSP_ADMIN_COL_ICON, SHARES_ICON_NAME,
						TSP_ADMIN_COL_TXT, text_string,
						TSP_ADMIN_COL_PATH, share->path,
						-1);

	g_free (text_string);
}

/* (Re)loads the shares list */
void
tsp_admin_load_shares (gpointer data)
{
	TspAdmin *admin = (TspAdmin *)data;
	GSList   *info_list = NULL;

	shares_get_share_info_list (&info_list, NULL);

	if (info_list)
	{
		GtkTreeModel *model;

		/* Reset the list */
		model = gtk_tree_view_get_model (GTK_TREE_VIEW (admin->share_list));
		gtk_list_store_clear (GTK_LIST_STORE (model));

		g_slist_foreach (info_list, admin_shares_foreach, model);

		shares_free_share_info_list (info_list);
	}
}

/* Displays the main admin dialog */
void
tsp_admin_dialog_show (GtkWindow *parent)
{
	GtkTreeSelection *selection;
	static TspAdmin  *admin;
	GtkBuilder       *ui;

	if (admin)
	{
		/* Bring the dialog to front */
		gtk_window_present (GTK_WINDOW (admin->dialog));
		return;
	}

	admin = g_new0 (TspAdmin, 1);

	/* Get widgets */
	ui = tsp_xml_get_file (XML_FILE,
						"shares_admin", &admin->dialog,
						 "share_list", &admin->share_list,
						 "share_edit", &admin->share_edit,
						 "share_remove", &admin->share_remove,
						 NULL);

	/* Connect signals */
	tsp_xml_connect (ui, admin,
						"shares_admin", "destroy", admin_destroy_cb,
						"shares_admin", "response", admin_response_cb,
						"share_add", "clicked", admin_add_cb,
						"share_edit", "clicked", admin_edit_cb,
						"share_remove", "clicked", admin_remove_cb,
						NULL);

	g_object_unref (ui);

	g_object_add_weak_pointer (G_OBJECT (admin->dialog), (gpointer) &admin);

	/* Disable edit and remove buttons */
	gtk_widget_set_sensitive (admin->share_edit, FALSE);
	gtk_widget_set_sensitive (admin->share_remove, FALSE);

	/* Load current shares */
	tsp_admin_load_shares (admin);

	/* Setup selection changes */
	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (admin->share_list));
	gtk_tree_selection_set_mode (selection, GTK_SELECTION_SINGLE);
	g_signal_connect (G_OBJECT (selection), "changed", G_CALLBACK (admin_selection_cb), admin);

	/* Show the dialog */
	gtk_window_set_transient_for (GTK_WINDOW (admin->dialog), parent);
	gtk_widget_show (admin->dialog);
}
