/* vim: set ts=4 sw=8 noet ai nocindent syntax=c: */
/*
 * Copyright (C) 2002 Imendio AB
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

#ifndef __LIBSHARES_XML_H__
#define __LIBSHARES_XML_H__

#include <gtk/gtkbuilder.h>

G_BEGIN_DECLS

GtkBuilder *tsp_xml_get_file (const gchar *filename,
                              const gchar *first_widget,
                              ...);

void        tsp_xml_connect  (GtkBuilder  *gui,
                              gpointer     user_data,
                              gchar       *first_widget,
                              ...);
G_END_DECLS

#endif /*  __LIBSHARES_XML_H__ */
