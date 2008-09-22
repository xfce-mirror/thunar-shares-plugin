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
#include <libshares/libshares-util.h>

#include "tsp-admin.h"
#include "tsp-admin-editor.h"

#define SHARES_ICON_NAME "gnome-fs-share"

enum {
  TSP_MANAGER_COL_ICON = 0,
  TSP_MANAGER_COL_TXT,
  TSP_MANAGER_COL_PATH,
  TSP_MANAGER_N_COLS
};

static void tsp_admin_manager_class_init (TspAdminManagerClass  *klass);
static void tsp_admin_manager_init       (TspAdminManager       *manager);
static void tsp_admin_manager_finalize   (GObject               *object);

static void tsp_admin_manager_response   (GtkDialog             *dialog,
                                          gint                   response);

static void tsp_admin_manager_select_cb  (GtkTreeSelection      *selection,
                                          TspAdminManager       *manager);

static void tsp_admin_manager_add_cb     (GtkButton             *button,
                                          TspAdminManager       *manager);
static void tsp_admin_manager_edit_cb    (GtkButton             *button,
                                          TspAdminManager       *manager);
static void tsp_admin_manager_remove_cb  (GtkButton             *button,
                                          TspAdminManager       *manager);

static void tsp_admin_manager_foreach    (gpointer               data,
                                          gpointer               user_data);

struct _TspAdminManagerClass
{
  GtkDialogClass __parent__;
};

struct _TspAdminManager
{
  GtkDialog   __parent__;
  
  GtkWidget  *share_list;
  GtkWidget  *share_edit;
  GtkWidget  *share_remove;
};

THUNARX_DEFINE_TYPE (TspAdminManager, tsp_admin_manager, GTK_TYPE_DIALOG);

static void
tsp_admin_manager_class_init (TspAdminManagerClass *klass)
{
  GtkDialogClass *gtkdialog_class;
  GObjectClass   *gobject_class;

  gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->finalize = tsp_admin_manager_finalize;

  gtkdialog_class = GTK_DIALOG_CLASS (klass);
  gtkdialog_class->response = tsp_admin_manager_response;
}

static void
tsp_admin_manager_init (TspAdminManager *manager)
{
  GtkWidget *vbox;
  GtkWidget *hbox;
  GtkWidget *widget;
  GtkWidget *buttonbox;
  GtkListStore *liststore;
  GtkCellRenderer *renderer;
  GtkTreeViewColumn *column;
  GtkTreeSelection *selection;

  vbox = GTK_WIDGET (GTK_DIALOG (manager)->vbox);

  /* Dialog setup */
  gtk_window_set_title (GTK_WINDOW (manager), _("Thunar - Manage Shared Folders"));
  gtk_dialog_set_has_separator (GTK_DIALOG (manager), FALSE);
  gtk_window_set_destroy_with_parent (GTK_WINDOW (manager), TRUE);
  gtk_window_set_modal (GTK_WINDOW (manager), TRUE);
  gtk_window_set_position (GTK_WINDOW (manager), GTK_WIN_POS_CENTER_ON_PARENT);
  gtk_window_set_type_hint (GTK_WINDOW (manager), GDK_WINDOW_TYPE_HINT_DIALOG);
  gtk_window_set_default_size (GTK_WINDOW (manager), 500, 280);
  gtk_container_set_border_width (GTK_CONTAINER (manager), 5);

  /* Dialog response and buttons */
  gtk_dialog_add_button (GTK_DIALOG (manager), GTK_STOCK_CLOSE, GTK_RESPONSE_CLOSE);
  gtk_dialog_set_default_response (GTK_DIALOG (manager), GTK_RESPONSE_CLOSE);
  
  hbox = gtk_hbox_new (FALSE, 10);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, TRUE, TRUE, 5);
  
  /* List view */
  widget = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (widget), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (widget), GTK_SHADOW_IN);
  gtk_box_pack_start (GTK_BOX (hbox), widget, TRUE, TRUE, 0);

  manager->share_list = gtk_tree_view_new ();
  gtk_tree_view_set_enable_search (GTK_TREE_VIEW (manager->share_list), FALSE);
  gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (manager->share_list), FALSE);
  gtk_container_add (GTK_CONTAINER (widget), manager->share_list);
  
  /* List model */
  liststore = gtk_list_store_new (TSP_MANAGER_N_COLS, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
  gtk_tree_view_set_model (GTK_TREE_VIEW (manager->share_list), GTK_TREE_MODEL (liststore));

  /* Tree view columns and renders */
  column = gtk_tree_view_column_new ();
  gtk_tree_view_column_set_expand (column, TRUE);
  gtk_tree_view_column_set_resizable (column, FALSE);
  gtk_tree_view_append_column (GTK_TREE_VIEW (manager->share_list), column);
  
  renderer = gtk_cell_renderer_pixbuf_new ();
  g_object_set (G_OBJECT (renderer), "xpad", 2, "ypad", 2, "stock-size", GTK_ICON_SIZE_DND, NULL);
  gtk_tree_view_column_pack_start (column, renderer, FALSE);
  gtk_tree_view_column_set_attributes (column, renderer, "icon-name", TSP_MANAGER_COL_ICON, NULL);
  
  renderer = gtk_cell_renderer_text_new ();
  gtk_tree_view_column_pack_start (column, renderer, TRUE);
  gtk_tree_view_column_set_attributes (column, renderer, "markup", TSP_MANAGER_COL_TXT, NULL);

  /* Setup selection changes */
  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (manager->share_list));
  gtk_tree_selection_set_mode (selection, GTK_SELECTION_SINGLE);
  g_signal_connect (G_OBJECT (selection), "changed", G_CALLBACK (tsp_admin_manager_select_cb), manager);

  /* Buttons */
  buttonbox = gtk_vbutton_box_new ();
  gtk_box_set_spacing (GTK_BOX (buttonbox), 5);
  gtk_button_box_set_layout (GTK_BUTTON_BOX (buttonbox), GTK_BUTTONBOX_START);
  gtk_box_pack_start (GTK_BOX (hbox), buttonbox, FALSE, FALSE, 0);
  
  widget = gtk_button_new_from_stock (GTK_STOCK_ADD);
  g_signal_connect (G_OBJECT (widget), "clicked", G_CALLBACK (tsp_admin_manager_add_cb), manager);
  gtk_container_add (GTK_CONTAINER (buttonbox), widget);
  
  manager->share_edit = gtk_button_new_from_stock (GTK_STOCK_EDIT);
  g_signal_connect (G_OBJECT (manager->share_edit), "clicked", G_CALLBACK (tsp_admin_manager_edit_cb), manager);
  gtk_container_add (GTK_CONTAINER (buttonbox), manager->share_edit);
  
  manager->share_remove = gtk_button_new_from_stock (GTK_STOCK_REMOVE);
  g_signal_connect (G_OBJECT (manager->share_remove), "clicked", G_CALLBACK (tsp_admin_manager_remove_cb), manager);
  gtk_container_add (GTK_CONTAINER (buttonbox), manager->share_remove);
  
  /* Disable edit and remove buttons */
  gtk_widget_set_sensitive (manager->share_edit, FALSE);
  gtk_widget_set_sensitive (manager->share_remove, FALSE);
  
  /* Let's show all ;) */
  gtk_widget_show_all (vbox);

  /* Load shares */
  tsp_admin_manager_reload_shares (manager);
}

static void
tsp_admin_manager_finalize (GObject *object)
{
  /*TspAdminManager *manager = TSP_ADMIN_MANAGER (object);*/

  /* Cleanup... */

  (*G_OBJECT_CLASS (tsp_admin_manager_parent_class)->finalize) (object);
}

static void
tsp_admin_manager_response (GtkDialog *dialog,
                            gint       response)
{
  if (response == GTK_RESPONSE_CLOSE){

    gtk_widget_destroy (GTK_WIDGET (dialog));

  } else if (GTK_DIALOG_CLASS (tsp_admin_manager_parent_class)->response != NULL){

    (*GTK_DIALOG_CLASS (tsp_admin_manager_parent_class)->response) (dialog, response);

  }
}

/* Adding a shared folder */
static void
tsp_admin_manager_add_cb (GtkButton        *button,
                          TspAdminManager  *manager)
{
  GtkWidget *dialog;

  dialog = tsp_admin_editor_new (manager);

  gtk_widget_show (dialog);
}

/* Editing a shared folder */
static void
tsp_admin_manager_edit_cb (GtkButton        *button,
                           TspAdminManager  *manager)
{
  GtkTreeSelection *selection;
  GtkWidget        *dialog;
  GtkTreeModel     *model;
  GtkTreeIter       iter;
  gchar            *path;

  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (manager->share_list));
  if (gtk_tree_selection_get_selected (selection, &model, &iter))
  {
    gtk_tree_model_get (model, &iter, TSP_MANAGER_COL_PATH, &path, -1);

    dialog = tsp_admin_editor_new_with_path (manager, path);

    gtk_widget_show (dialog);

    g_free (path);
  }
}

/* Removing a shared folder */
static void
tsp_admin_manager_remove_cb (GtkButton       *button,
                             TspAdminManager *manager)
{
  GtkTreeSelection *selection;
  GtkTreeModel     *model;
  GtkTreeIter       iter;
  gchar            *path;

  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (manager->share_list));
  if (gtk_tree_selection_get_selected (selection, &model, &iter))
  {
    gtk_tree_model_get (model, &iter, TSP_MANAGER_COL_PATH, &path, -1);

    /* Remove from shares */
    if (tsp_shares_unshare (path)){
      gtk_list_store_remove (GTK_LIST_STORE (model), &iter);
    }

    g_free (path);
  }
}

/* Selection of the list view */
static void
tsp_admin_manager_select_cb (GtkTreeSelection *selection,
                             TspAdminManager  *manager)
{
  GtkTreeModel *model;
  GtkTreeIter   iter;
  gboolean      has_selection;

  has_selection = gtk_tree_selection_get_selected (selection, &model, &iter);

  /* Update buttons sensitivity */
  gtk_widget_set_sensitive (manager->share_edit, has_selection);
  gtk_widget_set_sensitive (manager->share_remove, has_selection);
}

/* Add a share to the list view */
static void
tsp_admin_manager_foreach (gpointer  data,
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
                      TSP_MANAGER_COL_ICON, SHARES_ICON_NAME,
                      TSP_MANAGER_COL_TXT, text_string,
                      TSP_MANAGER_COL_PATH, share->path,
                      -1);

  g_free (text_string);
}

/* (Re)loads the shares list */
void
tsp_admin_manager_reload_shares (TspAdminManager *manager)
{
  gboolean result;
  GSList  *info_list = NULL;
  GError  *error = NULL;

  result = shares_get_share_info_list (&info_list, &error);

  /* Check error */
  if (!result)
  {
    tsp_show_error (_("There was an error while listing shares"), error->message);
    g_error_free (error);
  }

  if (info_list)
  {
    GtkTreeModel *model;

    /* Reset the list */
    model = gtk_tree_view_get_model (GTK_TREE_VIEW (manager->share_list));
    gtk_list_store_clear (GTK_LIST_STORE (model));

    g_slist_foreach (info_list, tsp_admin_manager_foreach, model);

    shares_free_share_info_list (info_list);
  }
}

GtkWidget *
tsp_admin_manager_new (GtkWindow *parent)
{
  GtkWidget *dialog;

  dialog = g_object_new (TSP_ADMIN_TYPE_MANAGER, NULL);

  if (parent != NULL)
    gtk_window_set_transient_for (GTK_WINDOW (dialog), GTK_WINDOW (parent));

  return dialog;
}
