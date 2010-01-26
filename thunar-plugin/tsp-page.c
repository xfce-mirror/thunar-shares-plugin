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

#include <glib/gi18n-lib.h>
#include <gtk/gtk.h>

#include <libshares/shares.h>
#include <libshares/libshares-util.h>

#include "tsp-page.h"

/* Property identifiers */
enum
{
  PROP_0,
  PROP_FILE,
};

static void tsp_page_class_init    (TspPageClass    *klass);
static void tsp_page_init          (TspPage         *page);
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
                                    gpointer         user_data);
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
static void tsp_page_set_error     (TspPage         *tsp_page,
                                    const char      *msg);
static void tsp_page_reset_error   (TspPage         *tsp_page);

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

  gboolean            supports_guest;
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
tsp_page_init (TspPage *page)
{
  GtkWidget *vbox1;
  GtkWidget *hbox1;
  GtkWidget *widget;

  /* Main container */
  vbox1 = gtk_vbox_new (FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (vbox1), 5);

  /* Header */
  hbox1 = gtk_hbox_new (FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vbox1), hbox1, FALSE, TRUE, 5);

  widget = gtk_image_new ();
  gtk_image_set_from_icon_name (GTK_IMAGE (widget), "gnome-fs-share", GTK_ICON_SIZE_DIALOG);
  gtk_misc_set_padding (GTK_MISC (widget), 13, 0);
  gtk_misc_set_alignment (GTK_MISC (widget), 0.0, 0.50);
  gtk_box_pack_start (GTK_BOX (hbox1), widget, FALSE, FALSE, 0);

  widget = gtk_label_new (_("<b>Folder Sharing</b>"));
  gtk_label_set_use_markup (GTK_LABEL(widget), TRUE);
  gtk_misc_set_alignment (GTK_MISC (widget), 0.0f, 0.50f);
  gtk_box_pack_start (GTK_BOX (hbox1), widget, FALSE, TRUE, 5);

  /* Horizontal separator */
  widget = gtk_hseparator_new ();
  gtk_box_pack_start (GTK_BOX (vbox1), widget, FALSE, TRUE, 5);

  /* Share check button */
  page->cb_share_folder = gtk_check_button_new_with_label (_("Share this folder"));
  g_signal_connect (G_OBJECT (page->cb_share_folder), "toggled", G_CALLBACK (tsp_page_share_toggled), page);
  gtk_box_pack_start (GTK_BOX (vbox1), page->cb_share_folder, FALSE, FALSE, 5);

  /* Share name */
  hbox1 = gtk_hbox_new (FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vbox1), hbox1, FALSE, FALSE, 5);

  page->label_name = gtk_label_new (_("Share Name:"));
  gtk_misc_set_padding (GTK_MISC (page->label_name), 5, 0);
  gtk_misc_set_alignment (GTK_MISC (page->label_name), 0.0f, 0.50f);
  gtk_box_pack_start (GTK_BOX (hbox1), page->label_name, FALSE, FALSE, 0);

  page->entry_share_name = gtk_entry_new ();
  g_signal_connect (G_OBJECT (page->entry_share_name), "changed", G_CALLBACK (tsp_page_name_changed), page);
  gtk_box_pack_start (GTK_BOX (hbox1), page->entry_share_name, TRUE, TRUE, 0);

  /* Write access */
  page->cb_share_write = gtk_check_button_new_with_label (_("Allow others users to write in this folder"));
  g_signal_connect (G_OBJECT (page->cb_share_write), "toggled", G_CALLBACK (tsp_page_write_toggled), page);
  gtk_box_pack_start (GTK_BOX (vbox1), page->cb_share_write, FALSE, FALSE, 5);

  /* Guest access */
  page->cb_share_guest = gtk_check_button_new_with_label (_("Allow Guest access"));
  g_signal_connect (G_OBJECT (page->cb_share_guest), "toggled", G_CALLBACK (tsp_page_guest_toggled), page);
  gtk_box_pack_start (GTK_BOX (vbox1), page->cb_share_guest, FALSE, FALSE, 5);

  /* Share comments */
  hbox1 = gtk_hbox_new (FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vbox1), hbox1, FALSE, FALSE, 5);

  page->label_comments = gtk_label_new (_("Comments:"));
  gtk_misc_set_padding (GTK_MISC (page->label_comments), 5, 0);
  gtk_misc_set_alignment (GTK_MISC (page->label_comments), 0.0f, 0.50f);
  gtk_box_pack_start (GTK_BOX (hbox1), page->label_comments, FALSE, FALSE, 0);

  page->entry_share_comments = gtk_entry_new ();
  g_signal_connect (G_OBJECT (page->entry_share_comments), "changed", G_CALLBACK (tsp_page_cmt_changed), page);
  gtk_box_pack_start (GTK_BOX (hbox1), page->entry_share_comments, TRUE, TRUE, 0);

  /* Apply button */
  hbox1 = gtk_hbutton_box_new ();
  gtk_box_set_spacing (GTK_BOX (hbox1), 5);
  gtk_hbutton_box_set_layout_default (GTK_BUTTONBOX_END);
  gtk_box_pack_start (GTK_BOX (vbox1), hbox1, TRUE, TRUE, 7);

  page->button_apply = gtk_button_new_from_stock (GTK_STOCK_APPLY);
  g_signal_connect (G_OBJECT (page->button_apply), "clicked", G_CALLBACK (tsp_page_apply_clicked), page);
  gtk_box_pack_end (GTK_BOX (hbox1), page->button_apply, FALSE, FALSE, 0);

  /* Status label */
  page->label_status = gtk_label_new (NULL);
  gtk_label_set_use_markup (GTK_LABEL (page->label_status), TRUE);
  gtk_label_set_line_wrap (GTK_LABEL (page->label_status), TRUE);
  gtk_misc_set_alignment (GTK_MISC (page->label_status ), 0.0f, 0.50f);
  gtk_misc_set_padding (GTK_MISC (page->label_status), 5, 0);
  gtk_box_pack_start (GTK_BOX (vbox1), page->label_status, FALSE, FALSE, 5);

  /* Add to the dialog */
  gtk_container_add (GTK_CONTAINER (page), vbox1);
  gtk_container_set_border_width (GTK_CONTAINER (page), 5);
  gtk_widget_show_all (vbox1);

  /* Check if guest access is supported */
  shares_supports_guest_ok (&page->supports_guest, NULL);

  if (!page->supports_guest){
    gtk_widget_set_sensitive(page->cb_share_guest, FALSE);
  }
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

  /* Check if we already use this file */
  if (G_UNLIKELY (tsp_page->file == file))
    return;

  /* Disconnect from the previous file (if any) */
  if (G_LIKELY (tsp_page->file != NULL))
  {
    g_signal_handlers_disconnect_by_func (G_OBJECT (tsp_page->file), tsp_page_file_changed, tsp_page);
    g_object_unref (G_OBJECT (tsp_page->file));
  }

  /* Assign the value */
  tsp_page->file = file;

  /* Connect to the new file (if any) */
  if (G_LIKELY (file != NULL))
  {
    /* Take a reference on the info file */
    g_object_ref (G_OBJECT (tsp_page->file));
    tsp_page_file_changed (file, tsp_page);

    g_signal_connect (G_OBJECT (file), "changed", G_CALLBACK (tsp_page_file_changed), tsp_page);
  }
}

/* File changed */
static void
tsp_page_file_changed (ThunarxFileInfo *file,
                       gpointer         user_data)
{
  ShareInfo   *share_info;
  gboolean     result;
  TspPage     *tsp_page;
  GError      *error = NULL;
  gchar       *uri, *file_local;

  tsp_page = TSP_PAGE (user_data);

  tsp_page->file = file; 

  /* Load share info */
  uri = thunarx_file_info_get_uri (tsp_page->file);
  file_local = g_filename_from_uri (uri, NULL, NULL);

  result = shares_get_share_info_for_path (file_local, &share_info, &error);

  /* Free some memory */
  g_free (uri);
  g_free (file_local);

  /* Check folder owner */
  if (!libshares_check_owner (tsp_page->file))
  {
    tsp_page_set_error (tsp_page, _("You are not the owner of the folder."));

    return;
  }

  /* Check error */
  if (!result)
  {
    tsp_page_set_error (tsp_page, _("You may need to install Samba, check your "
                                    "user permissions(usershares group) and re-login."
                                    "\n<b>More info:</b> <u>http://thunar-shares.googlecode.com/</u>"));
 
    g_error_free (error);
    
    return;
  }

  /* Reset error */
  tsp_page_reset_error (tsp_page);

  /* Check if shared */
  if (share_info)
  {
    tsp_page_sensibility (tsp_page, TRUE);

    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (tsp_page->cb_share_folder), TRUE);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (tsp_page->cb_share_write), share_info->is_writable);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (tsp_page->cb_share_guest), share_info->guest_ok);
    gtk_entry_set_text (GTK_ENTRY (tsp_page->entry_share_name), share_info->share_name);
    gtk_entry_set_text (GTK_ENTRY (tsp_page->entry_share_comments), share_info->comment);

    tsp_update_default (tsp_page, share_info);
    shares_free_share_info (share_info);
  } else {
    tsp_update_default (tsp_page, NULL);
    tsp_page_sensibility (tsp_page, FALSE);
  }
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
  local_file = libshares_get_local_file (tsp_page->file);

  if (share)
  {
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

    share_info = libshares_shares_share (local_file, name, comments,
                                         is_writable, guests_ok,
                                         tsp_page->share_name);
    if (share_info != NULL)
    {
      tsp_update_default (tsp_page, share_info);
      shares_free_share_info (share_info);

      thunarx_file_info_changed (tsp_page->file);
    }
  } else {
    /* Un-share the folder */
    if (libshares_shares_unshare (local_file))
    {
      tsp_update_default (tsp_page, NULL);

      thunarx_file_info_changed (tsp_page->file);
    }
  }

  g_free (local_file);
}

static void
tsp_page_reset_error (TspPage *tsp_page)
{
  gtk_label_set_markup (GTK_LABEL (tsp_page->label_status), NULL);

  gtk_widget_set_sensitive (GTK_WIDGET (tsp_page), TRUE);
}

static void
tsp_page_set_error (TspPage    *tsp_page,
                    const char *msg)
{
  gchar *errormsg = g_strdup_printf ("<span color='red'>%s</span>", msg);

  gtk_label_set_markup (GTK_LABEL (tsp_page->label_status), errormsg);

  gtk_widget_set_sensitive (GTK_WIDGET (tsp_page), FALSE);
}


/* Disable ore enable widgets when 'share folder' is toggled */
static void
tsp_page_sensibility (TspPage  *tsp_page,
                      gboolean  sens)
{
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (tsp_page->cb_share_folder), sens);
  gtk_widget_set_sensitive (GTK_WIDGET (tsp_page->entry_share_name), sens);
  gtk_widget_set_sensitive (GTK_WIDGET (tsp_page->cb_share_write), sens);
  gtk_widget_set_sensitive (GTK_WIDGET (tsp_page->entry_share_comments), sens);
  gtk_widget_set_sensitive (GTK_WIDGET (tsp_page->label_name), sens);
  gtk_widget_set_sensitive (GTK_WIDGET (tsp_page->label_comments), sens);
  gtk_widget_set_sensitive (GTK_WIDGET (tsp_page->label_comments), sens);

  if (tsp_page->supports_guest){
    gtk_widget_set_sensitive (GTK_WIDGET (tsp_page->cb_share_guest), sens);
  }
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
            (!libshares_str_equal (page->share_comment, comment)) ||
            (!libshares_str_equal (page->share_name, name))){
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

  if (!info) {
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
