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

#include "tsp-prefs-provider.h"
#include "tsp-admin.h"

static void   tsp_preferences_class_init     (TspPreferencesClass              *klass);
static void   tsp_preferences_provider_init  (ThunarxPreferencesProviderIface  *iface);
static void   tsp_preferences_init           (TspPreferences                   *tsp_preferences);

static void   tsp_preferences_finalize       (GObject                          *object);

static GList* tsp_preferences_get_actions    (ThunarxPreferencesProvider       *provider,
                                              GtkWidget                        *window);

static void   tsp_shares_admin_activated     (GtkWindow                        *window);

struct _TspPreferencesClass
{
	GObjectClass  __parent__;
};

struct _TspPreferences
{
	GObject       __parent__;
};

THUNARX_DEFINE_TYPE_WITH_CODE (TspPreferences,
                               tsp_preferences,
                               G_TYPE_OBJECT,
                               THUNARX_IMPLEMENT_INTERFACE (THUNARX_TYPE_PREFERENCES_PROVIDER,
                                                            tsp_preferences_provider_init));

static void
tsp_preferences_class_init (TspPreferencesClass *klass)
{
	GObjectClass *gobject_class;

	gobject_class = G_OBJECT_CLASS (klass);
	gobject_class->finalize = tsp_preferences_finalize;
}

static void
tsp_preferences_init (TspPreferences *tsp_preferences)
{
	/* Bleh.. */
}

static void
tsp_preferences_provider_init (ThunarxPreferencesProviderIface *iface)
{
	 iface->get_actions = tsp_preferences_get_actions;
}

static void
tsp_preferences_finalize (GObject *object)
{
	//TspPreferences *tsp_preferences = TSP_PREFERENCES (object);
 
	(*G_OBJECT_CLASS (tsp_preferences_parent_class)->finalize) (object);
}

static GList*
tsp_preferences_get_actions (ThunarxPreferencesProvider *provider,
                             GtkWidget                  *window)
{
	GtkAction *action;
	GClosure  *closure;

	action = gtk_action_new ("Tsp::thunar-shares-admin",
							_("Manage shared folders..."),
							_("Add, edit and remove shared folders"),
							NULL);

	g_object_set (G_OBJECT (action),
				  "icon-name", "gnome-fs-share",
				  NULL);

	closure = g_cclosure_new_object_swap (
				G_CALLBACK (tsp_shares_admin_activated), G_OBJECT (window));

	g_signal_connect_closure (G_OBJECT (action), "activate", closure, TRUE);

	return g_list_prepend (NULL, action);
}


static void
tsp_shares_admin_activated (GtkWindow *window)
{
	tsp_admin_dialog_show (window);
}
