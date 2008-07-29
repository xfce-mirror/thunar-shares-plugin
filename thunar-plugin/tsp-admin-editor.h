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

#ifndef __TSP_ADMIN_EDITOR_H__
#define __TSP_ADMIN_EDITOR_H__

#include <gtk/gtkwindow.h>

G_BEGIN_DECLS;

void tsp_admin_editor_dialog_show (GtkWindow   *parent,
                                   const gchar *path,
                                   gpointer     admin);

G_END_DECLS;

#endif /* __TSP_ADMIN_EDITOR_H__ */
