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

#include <glib/gi18n-lib.h>
#include <libshares/shares.h>

#include "tsp-provider.h"
#include "tsp-page.h"

G_MODULE_EXPORT void thunar_extension_initialize (ThunarxProviderPlugin *plugin);
G_MODULE_EXPORT void thunar_extension_shutdown   (void);
G_MODULE_EXPORT void thunar_extension_list_types (const GType          **types,
                                                  gint                  *n_types);

static GType type_list[1];

G_MODULE_EXPORT void
thunar_extension_initialize (ThunarxProviderPlugin *plugin)
{
	const gchar *mismatch;

	/* verify that the thunarx versions are compatible */
	mismatch = thunarx_check_version (THUNARX_MAJOR_VERSION, THUNARX_MINOR_VERSION, THUNARX_MICRO_VERSION);
	if (G_UNLIKELY (mismatch != NULL))
	{
		g_warning ("Version mismatch: %s", mismatch);
		return;
	}

	/* setup i18n support */
	bindtextdomain (GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
#ifdef HAVE_BIND_TEXTDOMAIN_CODESET
	bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
#endif

#ifdef G_ENABLE_DEBUG
	g_message ("Initializing thunar-shares-plugin extension");
#endif

	/* register the types provided by this plugin */
	tsp_page_register_type (plugin);
	tsp_provider_register_type (plugin);

	/* setup the plugin provider type list */
	type_list[0] = TSP_TYPE_PROVIDER;
}

G_MODULE_EXPORT void
thunar_extension_shutdown (void)
{
#ifdef G_ENABLE_DEBUG
	g_message ("Shutting down thunar-shares-plugin extension");
#endif

	/* Free all shares */
	free_all_shares ();
}

G_MODULE_EXPORT void
thunar_extension_list_types (const GType **types,
                             gint         *n_types)
{
	*types = type_list;
	*n_types = G_N_ELEMENTS (type_list);
}
