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

#include <thunarx/thunarx.h>
#include <thunar-vfs/thunar-vfs.h>

#include <libshares/shares.h>
#include <libshares/libshares-util.h>

#include "tsp-provider.h"
#include "tsp-page.h"
#include "tsp-admin.h"

static void     tsp_provider_class_init          (TspProviderClass                 *klass);
static void     tsp_provider_init                (TspProvider                      *tsp_provider);
static void     tsp_provider_finalize            (GObject                          *object);

static void     tsp_provider_page_provider_init  (ThunarxPropertyPageProviderIface *iface);
static void     tsp_provider_prefs_provider_init (ThunarxPreferencesProviderIface  *iface);
static void     tsp_provider_menu_provider_init  (ThunarxMenuProviderIface         *iface);

static GList   *tsp_provider_get_pages           (ThunarxPropertyPageProvider      *provider,
                                                  GList                            *files);

static GList   *tsp_provider_get_actions         (ThunarxPreferencesProvider       *provider,
                                                  GtkWidget                        *window);

static GList   *tsp_provider_get_file_actions    (ThunarxMenuProvider              *provider,
                                                  GtkWidget                        *window,
                                                  GList                            *files);

static void     tsp_provider_unshare_activated   (GtkAction                        *action,
                                                  GtkWindow                        *window);

static void     tsp_provider_prefs_activated     (GtkWindow                        *window);

struct _TspProviderClass
{
  GObjectClass  __parent__;
};

struct _TspProvider
{
  GObject       __parent__;
};

static GQuark tsp_action_folders_quark;

THUNARX_DEFINE_TYPE_WITH_CODE (TspProvider,
                               tsp_provider,
                               G_TYPE_OBJECT,
                               THUNARX_IMPLEMENT_INTERFACE (THUNARX_TYPE_PROPERTY_PAGE_PROVIDER,
                                                            tsp_provider_page_provider_init)
                               THUNARX_IMPLEMENT_INTERFACE (THUNARX_TYPE_PREFERENCES_PROVIDER,
                                                            tsp_provider_prefs_provider_init)
                               THUNARX_IMPLEMENT_INTERFACE (THUNARX_TYPE_MENU_PROVIDER,
                                                            tsp_provider_menu_provider_init));

static void
tsp_provider_class_init (TspProviderClass *klass)
{
  GObjectClass *gobject_class;

  /* Quark for folders and actions */
  tsp_action_folders_quark = g_quark_from_string ("tsp-action-folders");

  gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->finalize = tsp_provider_finalize;
}

static void
tsp_provider_init (TspProvider *tsp_provider)
{
  /* Bleh..! */
}

static void
tsp_provider_finalize (GObject *object)
{
  /*TspProvider *tsp_provider = TSP_PROVIDER (object);*/

  (*G_OBJECT_CLASS (tsp_provider_parent_class)->finalize) (object);
}

static void
tsp_provider_page_provider_init (ThunarxPropertyPageProviderIface *iface)
{
  iface->get_pages = tsp_provider_get_pages;
}

static void
tsp_provider_prefs_provider_init (ThunarxPreferencesProviderIface *iface)
{
  iface->get_actions = tsp_provider_get_actions;
}

static void
tsp_provider_menu_provider_init (ThunarxMenuProviderIface *iface)
{
  iface->get_file_actions = tsp_provider_get_file_actions;
}

static GList*
tsp_provider_get_pages (ThunarxPropertyPageProvider *property_page_provider,
                        GList                       *files)
{
  if (g_list_length (files) != 1){
    return NULL;
  } else if (!libshares_is_shareable (THUNARX_FILE_INFO (files->data))){
    return NULL;
  }

  return g_list_append (NULL, (gpointer)tsp_page_new (files->data));
}

static GList*
tsp_provider_get_actions (ThunarxPreferencesProvider *provider,
                          GtkWidget               *window)
{
  GtkAction *action;
  GClosure  *closure;

  action = gtk_action_new ("Tsp::thunar-shares-admin",
                           _("Manage shared folders..."),
                           _("Add, edit and remove shared folders"),
                           NULL);

  g_object_set (G_OBJECT (action), "icon-name", "gnome-fs-share", NULL);

  closure = g_cclosure_new_object_swap (
              G_CALLBACK (tsp_provider_prefs_activated), G_OBJECT (window));

  g_signal_connect_closure (G_OBJECT (action), "activate", closure, TRUE);

  return g_list_prepend (NULL, action);
}

static GList*
tsp_provider_get_file_actions (ThunarxMenuProvider *provider,
                               GtkWidget           *window,
                               GList               *files)
{
  GtkAction *action;
  gboolean   is_shared;
  GClosure  *closure;
  gchar     *folder_path;

  if (g_list_length (files) != 1){
    return NULL;
  } else if (!libshares_is_shareable (THUNARX_FILE_INFO (files->data))){
    return NULL;
  }

  folder_path = libshares_get_local_file (THUNARX_FILE_INFO (files->data));

  if (G_LIKELY (folder_path == NULL)){
    return NULL;
  }

  shares_get_path_is_shared (folder_path, &is_shared, NULL);

  if (!is_shared)
  {
    g_free (folder_path);
    return NULL;
  }

  action = gtk_action_new ("Tsp::thunar-shares-unshare",
                           _("Unshare..."),
                           _("Unshare the folder"),
                           NULL);

  g_object_set (G_OBJECT (action), "icon-name", "gnome-fs-share", NULL);
  
  g_object_set_qdata_full (G_OBJECT (action), tsp_action_folders_quark,
                           g_strdup (folder_path), (GDestroyNotify) g_free);

  closure = g_cclosure_new_object (G_CALLBACK (tsp_provider_unshare_activated), G_OBJECT (window));
  g_signal_connect_closure (G_OBJECT (action), "activate", closure, TRUE);

  g_free (folder_path);

  return g_list_prepend (NULL, action);
}

static void
tsp_provider_unshare_activated (GtkAction  *action,
                                GtkWindow  *window)
{
  gchar *folder;

  folder = g_object_get_qdata (G_OBJECT (action), tsp_action_folders_quark);

  if (folder == NULL)
    return;

  libshares_shares_unshare (folder);
}

static void
tsp_provider_prefs_activated (GtkWindow *window)
{
  GtkWidget *dialog;

  dialog = tsp_admin_manager_new (window);

  gtk_widget_show (dialog);
}
