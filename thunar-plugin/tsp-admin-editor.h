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

#include <thunarx/thunarx.h>
#include "tsp-admin.h"

G_BEGIN_DECLS;

typedef struct _TspAdminEditorClass       TspAdminEditorClass;
typedef struct _TspAdminEditor            TspAdminEditor;

#define TSP_ADMIN_TYPE_EDITOR             (tsp_admin_editor_get_type ())
#define TSP_ADMIN_EDITOR(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), TSP_ADMIN_TYPE_EDITOR, TspAdminEditor))
#define TSP_ADMIN_EDITOR_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), TSP_ADMIN_TYPE_EDITOR, TspAdminEditorClass))
#define TSP_ADMIN_IS_EDITOR(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TSP_ADMIN_TYPE_EDITOR))
#define TSP_ADMIN_IS_EDITOR_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), TSP_ADMIN_TYPE_EDITOR))
#define TSP_ADMIN_EDITOR_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), TSP_ADMIN_TYPE_EDITOR, TspAdminEditorClass))

GType   tsp_admin_editor_get_type         (void) G_GNUC_CONST G_GNUC_INTERNAL;
void    tsp_admin_editor_register_type    (ThunarxProviderPlugin *plugin) G_GNUC_INTERNAL;

GtkWidget *tsp_admin_editor_new           (TspAdminManager *manager);

GtkWidget *tsp_admin_editor_new_with_path (TspAdminManager *manager, const char *path);

G_END_DECLS;

#endif /* !__TSP_ADMIN_EDITOR_H__ */
