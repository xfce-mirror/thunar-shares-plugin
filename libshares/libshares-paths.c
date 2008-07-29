/* vim: set ts=4 sw=8 noet ai nocindent syntax=c: */
/*
 * Copyright (C) 2006-2007 Imendio AB
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
 * Authors: Richard Hult <richard@imendio.com>
 *          Brian Pepple <bpepple@fedoraproject.org>
 *          Daniel Morales <daniel@daniel.com.uy>
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "libshares-paths.h"

#define THUNARSHARES "thunar-shares"

gchar *
tsp_paths_get_xml_path (const gchar *filename)
{
	return g_build_filename (DATADIR, THUNARSHARES, filename, NULL);
}

gchar *
tsp_paths_get_image_path (const gchar *filename)
{
	return g_build_filename (DATADIR, THUNARSHARES, filename, NULL);
}

gchar *
tsp_paths_get_locale_path ()
{
	return g_strdup (LOCALEDIR);
}
