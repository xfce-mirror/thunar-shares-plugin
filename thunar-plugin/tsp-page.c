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

#include <gtk/gtk.h>
#include <thunar-vfs/thunar-vfs.h>

#include <libshares/shares.h>
#include <libshares/libshares-xml.h>
#include <libshares/libshares-util.h>

#include "tsp-page.h"

#define XML_FILE "thunar-page.xml"

/* Property identifiers */
enum
{
	PROP_0,
	PROP_FILE,
};

static void tsp_page_class_init    (TspPageClass    *klass);
static void tsp_page_init          (TspPage         *tsp_page);
static void tsp_page_finalize      (GObject         *object);
static void tsp_page_get_property  (GObject         *object,
                                    guint            prop_id,
                                    GValue          *value,
                                    GParamSpec      *pspec);
static void tsp_page_set_property  (GObject         *object,
                                    guint            prop_id,
                                    const GValue    *value,
                                    GParamSpec      *pspec);
static void tsp_page_file_changed  (ThunarxFileInfo *file,
                                    TspPage         *tsp_page);
static void tsp_page_share_toggled (GtkToggleButton *togglebutton,
                                    TspPage         *tsp_page);
static void tsp_page_write_toggled (GtkToggleButton *togglebutton,
                                    TspPage         *tsp_page);
static void tsp_page_guest_toggled (GtkToggleButton *togglebutton,
                                    TspPage         *tsp_page);
static void tsp_page_apply_clicked (GtkButton       *button,
                                    TspPage         *tsp_page);
static void tsp_page_name_changed  (GtkEditable     *editable,
                                    TspPage         *tsp_page);
static void tsp_page_cmt_changed   (GtkEditable     *editable,
                                    TspPage         *tsp_page);
static void tsp_page_sensibility   (TspPage         *tsp_page,
                                    gboolean         sens);

static gboolean tsp_check_changes  (TspPage         *page);

static void     tsp_update_default (TspPage         *page,
                                    ShareInfo       *info);

struct _TspPageClass
{
	ThunarxPropertyPageClass __parent__;
};

struct _TspPage
{
	ThunarxPropertyPage __parent__;
	ThunarxFileInfo    *file;

	GtkWidget          *cb_share_folder;
	GtkWidget          *entry_share_name;
	GtkWidget          *cb_share_write;
	GtkWidget          *entry_share_comments;
	GtkWidget          *label_comments;
	GtkWidget          *label_name;
	GtkWidget          *button_apply;
	GtkWidget          *cb_share_guest;
	GtkWidget          *label_status;

	/* Default values for the file */
	gboolean            is_shared;
	gboolean            can_write;
	gboolean            can_guests;
	gchar              *share_name;
	gchar              *share_comment;
};

/* implements the tsp_page_get_type() and tsp_page_register_type() functions */
THUNARX_DEFINE_TYPE (TspPage, tsp_page, THUNARX_TYPE_PROPERTY_PAGE);

static void
tsp_page_class_init (TspPageClass *klass)
{
	GObjectClass *gobject_class;

	gobject_class = G_OBJECT_CLASS (klass);
	gobject_class->finalize = tsp_page_finalize;
	gobject_class->get_property = tsp_page_get_property;
	gobject_class->set_property = tsp_page_set_property;

	/**
	* TspPage:file:
	*
	* The ThunarxFileInfo displayed by this TspPage.
	**/
	g_object_class_install_property (gobject_class,
                                     PROP_FILE,
                                     g_param_spec_object ("file", "file", "file",
                                                          THUNARX_TYPE_FILE_INFO,
                                                          G_PARAM_READWRITE));
}

static void
tsp_page_init (TspPage *tsp_page)
{
	GtkBuilder *ui;
	GtkWidget  *vbox;

	/* Grab the widgets */
	ui = tsp_xml_get_file (XML_FILE, 
				"cb_share_folder", &tsp_page->cb_share_folder,
				"label_name", &tsp_page->label_name,
				"entry_share_name", &tsp_page->entry_share_name,
				"cb_share_write", &tsp_page->cb_share_write,
				"label_comments", &tsp_page->label_comments,
				"entry_share_comments", &tsp_page->entry_share_comments,
				"button_apply", &tsp_page->button_apply,
				"cb_share_guest", &tsp_page->cb_share_guest,
				"label_status", &tsp_page->label_status,
				"vbox1", &vbox,
				NULL);

	/* Connect the signals */
	tsp_xml_connect (ui, tsp_page,
				"cb_share_folder", "toggled", tsp_page_share_toggled,
				"cb_share_write", "toggled", tsp_page_write_toggled,
				"cb_share_guest", "toggled", tsp_page_guest_toggled,
				"entry_share_comments", "changed", tsp_page_cmt_changed,
				"entry_share_name", "changed", tsp_page_name_changed,
				"button_apply", "clicked", tsp_page_apply_clicked,
				NULL);

	gtk_container_add (GTK_CONTAINER (tsp_page), vbox);
	gtk_container_set_border_width (GTK_CONTAINER (tsp_page), 5);
	gtk_widget_show_all (vbox);

	g_object_unref (ui);
}

static void
tsp_page_finalize (GObject *object)
{
	TspPage *tsp_page = TSP_PAGE (object);

	/* disconnect from the file */
	tsp_page_set_file (tsp_page, NULL);

	/* Free some memory */
	if (!G_STR_EMPTY (tsp_page->share_name))
		g_free (tsp_page->share_name);

	if (!G_STR_EMPTY (tsp_page->share_comment))
		g_free (tsp_page->share_comment);

	(*G_OBJECT_CLASS (tsp_page_parent_class)->finalize) (object);
}

static void
tsp_page_get_property (GObject    *object,
                       guint       prop_id,
                       GValue     *value,
                       GParamSpec *pspec)
{
	TspPage *tsp_page = TSP_PAGE (object);

	switch (prop_id)
	{
		case PROP_FILE:
			g_value_set_object (value, tsp_page_get_file (tsp_page));
		break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
tsp_page_set_property (GObject      *object,
                       guint         prop_id,
                       const GValue *value,
                       GParamSpec   *pspec)
{
	TspPage *tsp_page = TSP_PAGE (object);

	switch (prop_id)
	{
		case PROP_FILE:
			tsp_page_set_file (tsp_page, g_value_get_object (value));
		break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

GtkWidget*
tsp_page_new (ThunarxFileInfo *file)
{
	TspPage *page = g_object_new (TSP_TYPE_PAGE, "file", file, NULL);

	thunarx_property_page_set_label (THUNARX_PROPERTY_PAGE (page), _("Share"));

	return GTK_WIDGET (page);
}

ThunarxFileInfo*
tsp_page_get_file (TspPage *tsp_page)
{
	g_return_val_if_fail (TSP_IS_PAGE (tsp_page), NULL);
	return tsp_page->file;
}

void
tsp_page_set_file (TspPage         *tsp_page,
                   ThunarxFileInfo *file)
{
	g_return_if_fail (TSP_IS_PAGE (tsp_page));
	g_return_if_fail (file == NULL || THUNARX_IS_FILE_INFO (file));

	if (tsp_page->file != NULL)
	{
		g_signal_handlers_disconnect_by_func (tsp_page->file, tsp_page_file_changed, tsp_page);
		g_object_unref (G_OBJECT (tsp_page->file));
	}

	tsp_page->file = file;

	if (file != NULL)
	{
		g_object_ref (file);
		tsp_page_file_changed (file, tsp_page);
		g_signal_connect (file, "changed", G_CALLBACK (tsp_page_file_changed), tsp_page);
	}

	g_object_notify (G_OBJECT (tsp_page), "file");
}

/* File changed */
static void
tsp_page_file_changed (ThunarxFileInfo *file,
                       TspPage         *tsp_page)
{
	ShareInfo   *share_info;
	gboolean     result;
	gchar       *uri, *file_local;

	/* Load share info */
	uri = thunarx_file_info_get_uri (tsp_page->file);
	file_local = g_filename_from_uri (uri, NULL, NULL);

	result = shares_get_share_info_for_path (file_local, &share_info, NULL);
	if (share_info)
	{
		tsp_page_sensibility (tsp_page, TRUE);

		gtk_toggle_button_set_active (
				GTK_TOGGLE_BUTTON (tsp_page->cb_share_folder), TRUE);
		gtk_toggle_button_set_active (
				GTK_TOGGLE_BUTTON (tsp_page->cb_share_write), share_info->is_writable);
		gtk_toggle_button_set_active (
				GTK_TOGGLE_BUTTON (tsp_page->cb_share_guest), share_info->guest_ok);
		gtk_entry_set_text (
				GTK_ENTRY (tsp_page->entry_share_name), share_info->share_name);
		gtk_entry_set_text (
				GTK_ENTRY (tsp_page->entry_share_comments), share_info->comment);

		tsp_update_default (tsp_page, share_info);
		shares_free_share_info (share_info);
	} else {
		tsp_update_default (tsp_page, NULL);
		tsp_page_sensibility (tsp_page, FALSE);
	}

	g_free (uri);
	g_free (file_local);
}

/* Share name changed */
static void
tsp_page_name_changed  (GtkEditable  *editable,
                        TspPage      *tsp_page)
{
	tsp_check_changes (tsp_page);
}

/* Share comment changed */
static void
tsp_page_cmt_changed   (GtkEditable  *editable,
                        TspPage      *tsp_page)
{
	tsp_check_changes (tsp_page);
}

/* Allow users to write in the folder toggled */
static void
tsp_page_write_toggled (GtkToggleButton *togglebutton,
                        TspPage         *tsp_page)
{
	tsp_check_changes (tsp_page);
}

/* Allow guests access toggled */
static void
tsp_page_guest_toggled (GtkToggleButton *togglebutton,
                        TspPage         *tsp_page)
{
	tsp_check_changes (tsp_page);
}

/* Share folder toggled */
static void
tsp_page_share_toggled (GtkToggleButton *togglebutton,
                        TspPage         *tsp_page)
{
	gboolean share = gtk_toggle_button_get_active (togglebutton);

	/* Toggle sharing */
	tsp_page_sensibility (tsp_page, share);

	/* Sharing (?) */
	if (share)
	{
		const gchar *defauln;
		gchar       *name_utf8;
		gchar       *name;

		/* Check if we alredy had a name for it */
		defauln = gtk_entry_get_text (GTK_ENTRY (tsp_page->entry_share_name));
		if (G_STR_EMPTY (defauln))
		{
			name = thunarx_file_info_get_name (tsp_page->file);
			name_utf8 = g_filename_display_name (name);

			/* First sharename is the filename */
			gtk_entry_set_text (GTK_ENTRY (tsp_page->entry_share_name), name_utf8);
			
			g_free (name_utf8);
			g_free (name);
		}
	}

	tsp_check_changes (tsp_page);
}

/* Apply button clicked */
static void
tsp_page_apply_clicked (GtkButton *button,
                        TspPage   *tsp_page)
{
	gboolean share;
	gchar   *local_file;

	share = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (tsp_page->cb_share_folder));
	local_file = tsp_get_local_file (tsp_page->file);

	if (share){
		const gchar *comments;
		const gchar *name;
		ShareInfo   *share_info;
		gboolean     is_writable;
		gboolean     guests_ok;

		/* Share file */
		comments = gtk_entry_get_text (GTK_ENTRY (tsp_page->entry_share_comments));
		name = gtk_entry_get_text (GTK_ENTRY (tsp_page->entry_share_name));
		is_writable = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (tsp_page->cb_share_write));
		guests_ok = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (tsp_page->cb_share_guest));

		share_info = tsp_shares_share (local_file, name, comments,
		 							   is_writable, guests_ok);
		if (share_info)
		{
			tsp_update_default (tsp_page, share_info);
			shares_free_share_info (share_info);
		}
	} else {
		/* Un-share file */
		if (tsp_shares_unshare (local_file)){
			tsp_update_default (tsp_page, NULL);
		}
	}
	g_free (local_file);
}

/* Disable ore enable widgets when 'share folder' is toggled */
static void
tsp_page_sensibility  (TspPage  *tsp_page,
                       gboolean  sens)
{
	gtk_widget_set_sensitive (GTK_WIDGET (tsp_page->entry_share_name), sens);
	gtk_widget_set_sensitive (GTK_WIDGET (tsp_page->cb_share_write), sens);
	gtk_widget_set_sensitive (GTK_WIDGET (tsp_page->entry_share_comments), sens);
	gtk_widget_set_sensitive (GTK_WIDGET (tsp_page->label_name), sens);
	gtk_widget_set_sensitive (GTK_WIDGET (tsp_page->label_comments), sens);
	gtk_widget_set_sensitive (GTK_WIDGET (tsp_page->label_comments), sens);
	gtk_widget_set_sensitive (GTK_WIDGET (tsp_page->cb_share_guest), sens);
}

/* Checks if the settings of the current file has been changed */
static
gboolean tsp_check_changes (TspPage *page)
{
	const gchar *name, *comment;
	gboolean     shared, guests, write;
	gboolean     result = FALSE;

	/* Get the info */
	shared = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (page->cb_share_folder));
	guests = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (page->cb_share_guest));
	write = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (page->cb_share_write));
	name = gtk_entry_get_text (GTK_ENTRY (page->entry_share_name));
	comment = gtk_entry_get_text (GTK_ENTRY (page->entry_share_comments));
	
	/* Shared? */
	if (page->is_shared != shared){
		result = TRUE;
	} else if (!page->is_shared && !shared){
		result = FALSE;
	} else if ((page->can_write != write) ||
				(page->can_guests != guests) ||
				(!tsp_str_equal (page->share_comment, comment)) ||
				(!tsp_str_equal (page->share_name, name))){
		result = TRUE;
	}

	/* Update button */
	gtk_widget_set_sensitive (page->button_apply, result);

	return result;
}

/* Sets the default settings for the current file */
static void
tsp_update_default (TspPage   *page,
                    ShareInfo *info)
{
	if (!G_STR_EMPTY (page->share_name))
		g_free (page->share_name);

	if (!G_STR_EMPTY (page->share_comment))
		g_free (page->share_comment);

	page->share_name = NULL;
	page->share_comment = NULL;

	if (!info){
		/* Not shared info */
		page->is_shared = FALSE;
		page->can_write = FALSE;
		page->can_guests = FALSE;
	} else {
		/* Shared info */
		page->is_shared = TRUE;
		page->can_write = info->is_writable;
		page->can_guests = info->guest_ok;

		if (!G_STR_EMPTY (info->share_name))
			page->share_name = g_strdup (info->share_name);

		if (!G_STR_EMPTY (info->comment))
			page->share_comment = g_strdup (info->comment);
	}

	/* Disable button */
	gtk_widget_set_sensitive (page->button_apply, FALSE);
}
