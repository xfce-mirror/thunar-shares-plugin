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

#ifndef __TSP_PAGE_H__
#define __TSP_PAGE_H__

#include <thunarx/thunarx.h>

G_BEGIN_DECLS;

typedef struct _TspPageClass TspPageClass;
typedef struct _TspPage      TspPage;

#define TSP_TYPE_PAGE                   (tsp_page_get_type ())
#define TSP_PAGE(obj)                   (G_TYPE_CHECK_INSTANCE_CAST ((obj), TSP_TYPE_PAGE, TspPage))
#define TSP_PAGE_CLASS(klass)           (G_TYPE_CHECK_CLASS_CAST ((klass), TSP_TYPE_PAGE, TspPageClass))
#define TSP_IS_PAGE(obj)                (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TSP_TYPE_PAGE))
#define TSP_IS_PAGE_CLASS(klass)        (G_TYPE_CHECK_CLASS_TYPE ((klass), TSP_TYPE_PAGE))
#define TSP_PAGE_GET_CLASS(obj)         (G_TYPE_INSTANCE_GET_CLASS ((obj), TSP_TYPE_PAGE, TspPageClass))

/* these two functions are implemented automatically by the THUNARX_DEFINE_TYPE macro */
GType            tsp_page_get_type      (void) G_GNUC_CONST G_GNUC_INTERNAL;
void             tsp_page_register_type (ThunarxProviderPlugin *plugin) G_GNUC_INTERNAL;

GtkWidget       *tsp_page_new           (ThunarxFileInfo       *file) G_GNUC_INTERNAL G_GNUC_MALLOC;

ThunarxFileInfo *tsp_page_get_file      (TspPage               *tsp_page) G_GNUC_INTERNAL;
void             tsp_page_set_file      (TspPage               *tsp_page,
                                         ThunarxFileInfo       *file) G_GNUC_INTERNAL;

G_END_DECLS;

#endif /* !__TSP_PAGE_H__ */
