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

#include <thunarx/thunarx.h>

#include <libshares/libshares-util.h>

#include "tsp-provider.h"
#include "tsp-page.h"

static void     tsp_provider_class_init          (TspProviderClass                 *klass);
static void     tsp_provider_init                (TspProvider                      *tsp_provider);
static void     tsp_provider_finalize            (GObject                          *object);

static void     tsp_provider_page_provider_init  (ThunarxPropertyPageProviderIface *iface);
static void     tsp_provider_prefs_provider_init (ThunarxPreferencesProviderIface  *iface);

static GList   *tsp_provider_get_pages           (ThunarxPropertyPageProvider      *provider,
                                                  GList                            *files);

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
                               THUNARX_IMPLEMENT_INTERFACE (THUNARX_TYPE_PREFERENCES_PROVIDER,
                                                            tsp_provider_prefs_provider_init));

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
  /* Bleh..! */
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

static void
tsp_provider_prefs_provider_init (ThunarxPreferencesProviderIface *iface)
{
  /* Bleh..! */
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
