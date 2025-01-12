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

#include <glib/gi18n-lib.h>

#include <thunarx/thunarx.h>

#include <libshares/libshares-util.h>

#include "tsp-provider.h"
#include "tsp-page.h"
#include "xfconf/xfconf.h"

#define THUNAR_SHARES_PLUGIN_PROP_SHOW_MENU_CONTRIBUTION "/show-menu-contribution"

static void     tsp_provider_finalize            (GObject                          *object);

static void     tsp_provider_page_provider_init  (ThunarxPropertyPageProviderIface *iface);
static GList   *tsp_provider_get_pages           (ThunarxPropertyPageProvider      *provider,
                                                  GList                            *files);
static void     tsp_provider_menu_provider_init  (ThunarxMenuProviderIface         *iface);
static GList   *tsp_provider_get_menu            (ThunarxMenuProvider      *provider,
                                                  GtkWidget                *window,
                                                  GList                    *files);

struct _TspProviderClass
{
  GObjectClass  __parent__;
};

struct _TspProvider
{
  GObject       __parent__;
};

THUNARX_DEFINE_TYPE_WITH_CODE (TspProvider,
                               tsp_provider,
                               G_TYPE_OBJECT,
                               THUNARX_IMPLEMENT_INTERFACE (THUNARX_TYPE_PROPERTY_PAGE_PROVIDER,
                                                            tsp_provider_page_provider_init)
			                         THUNARX_IMPLEMENT_INTERFACE (THUNARX_TYPE_MENU_PROVIDER,
                                                            tsp_provider_menu_provider_init));/* add a menu provider */

static void
tsp_provider_class_init (TspProviderClass *klass)
{
  GObjectClass *gobject_class;

  gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->finalize = tsp_provider_finalize;
}

static void
tsp_provider_init (TspProvider *tsp_provider)
{
  GError *error = NULL;

  /* initialize xfconf */
  if (!xfconf_init (&error))
  {
      g_warning (PACKAGE_NAME ": Failed to initialize Xfconf: %s", error->message);
      g_clear_error (&error);
  }
}

static void
tsp_provider_menu_provider_init (ThunarxMenuProviderIface *iface)
{
  /* add right click menu contribution */
  iface->get_file_menu_items = tsp_provider_get_menu;
}


static void
tsp_provider_finalize (GObject *object)
{
  (*G_OBJECT_CLASS (tsp_provider_parent_class)->finalize) (object);
}

static void
tsp_provider_page_provider_init (ThunarxPropertyPageProviderIface *iface)
{
  iface->get_pages = tsp_provider_get_pages;
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

/* show share property page callback function */
static void
share_this_folder_callback (GtkAction *action, gpointer user_data)
{
  GtkWidget * window;
  GtkWidget * tsp_page;
  
  tsp_page  = tsp_page_new (user_data);
  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);

  gtk_container_add (GTK_CONTAINER (window), tsp_page);  
  gtk_widget_show (tsp_page);
  gtk_widget_show (window);
}

static GList*
tsp_provider_get_menu (ThunarxMenuProvider *menu_provider,
                       GtkWidget           *window,
                       GList               *files)
{
  ThunarxMenuItem *item;
  ShareInfo       *share_info;
  GError          *error = NULL;
  gchar           *uri, *file_local;
  ThunarxFileInfo *file = files->data;
  XfconfChannel   *channel;

  /* only show for single folders */
  if (g_list_length (files) != 1)
    return NULL;


  channel = xfconf_channel_get ("thunar-shares-plugin");

  /* Create the channel, if it does not exist yet */
  if (!xfconf_channel_has_property (channel, THUNAR_SHARES_PLUGIN_PROP_SHOW_MENU_CONTRIBUTION))
    xfconf_channel_set_bool (channel, THUNAR_SHARES_PLUGIN_PROP_SHOW_MENU_CONTRIBUTION, TRUE);

  if (!xfconf_channel_get_bool (channel, THUNAR_SHARES_PLUGIN_PROP_SHOW_MENU_CONTRIBUTION, TRUE))
    return NULL;

  if (!libshares_is_shareable (file))
    return NULL;

  /* Only sharable when you are the owner  */
  if (!libshares_check_owner (file))
    return NULL;

  /* Load share info */
  uri = thunarx_file_info_get_uri (file);
  file_local = g_filename_from_uri (uri, NULL, NULL);
  g_free (uri);

  if (!shares_get_share_info_for_path (file_local, &share_info, &error))
  {
    g_error_free (error);
    g_free (file_local);
    return NULL;
  }

  /* Free some memory */
  g_free (file_local);

  /* This folder is already shared */
  if (share_info)
  {
    shares_free_share_info (share_info);
    return NULL;
  }

  item = thunarx_menu_item_new ("TspProvider::share",
                            _("Folder _Sharing"),
                            _("Share this folder"),
                            "folder-publicshare");

  /* set callback function */
  g_signal_connect (G_OBJECT (item), "activate", G_CALLBACK (share_this_folder_callback), file);

  return g_list_prepend (NULL, item);
}
