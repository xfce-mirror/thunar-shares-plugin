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

#ifndef __TSP_PREFERENCES_H__
#define __TSP_PREFERENCES_H__

#include <thunarx/thunarx.h>

G_BEGIN_DECLS;

typedef struct _TspPreferencesClass      TspPreferencesClass;
typedef struct _TspPreferences           TspPreferences;

#define TSP_TYPE_PREFERENCES             (tsp_preferences_get_type ())
#define TSP_PREFERENCES(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), TSP_TYPE_PREFERENCES, TspPreferences))
#define TSP_PREFERENCES_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), TSP_TYPE_PREFERENCES, TspPreferencesClass))
#define TSP_IS_PREFERENCES(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TSP_TYPE_PREFERENCES))
#define TSP_IS_PREFERENCES_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), TSP_TYPE_PREFERENCES))
#define TSP_PREFERENCES_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), TSP_TYPE_PREFERENCES, TspPreferencesClass))

GType tsp_preferences_get_type           (void) G_GNUC_CONST G_GNUC_INTERNAL;
void  tsp_preferences_register_type      (ThunarxProviderPlugin *plugin) G_GNUC_INTERNAL;

G_END_DECLS;

#endif /* __TSP_PREFERENCES_H__ */
