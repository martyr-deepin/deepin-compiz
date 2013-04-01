/*
 * Copyright © 2006 Novell, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * Author: David Reveman <davidr@novell.com>
 *
 * 2D Mode: Copyright © 2010 Sam Spilsbury <smspillaz@gmail.com>
 * Frames Management: Copright © 2011 Canonical Ltd.
 *        Authored By: Sam Spilsbury <sam.spilsbury@canonical.com>
 */

#ifndef _GTK_WINDOW_DECORATOR_H
#define _GTK_WINDOW_DECORATOR_H
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "decoration.h"

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/cursorfont.h>
#include <X11/extensions/Xrender.h>
#include <X11/Xregion.h>

#ifdef HAVE_GTK_2_24

#ifndef GDK_DISABLE_DEPRECATED
#define GDK_DISABLE_DEPRECATED
#endif

#define create_foreign_window(xid)						       \
    gdk_x11_window_foreign_new_for_display (gdk_display_get_default (),	       \
					    xid)
#else

#define create_foreign_window(xid)						       \
    gdk_window_foreign_new (xid)

#ifdef GDK_DISABLE_DEPRECATED
#undef GDK_DISABLE_DEPRECATED
#endif

#endif

#ifndef GTK_DISABLE_DEPRECATED
#define GTK_DISABLE_DEPRECATED
#endif

#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include <gdk/gdk.h>

#ifdef USE_GCONF
#include <gconf/gconf-client.h>
#endif

#ifdef USE_DBUS_GLIB
#define DBUS_API_SUBJECT_TO_CHANGE
#include <dbus/dbus.h>
#include <dbus/dbus-glib-lowlevel.h>
#endif

#define WNCK_I_KNOW_THIS_IS_UNSTABLE
#include <libwnck/libwnck.h>
#include <libwnck/window-action-menu.h>

#ifndef HAVE_LIBWNCK_2_19_4
#define wnck_window_get_client_window_geometry wnck_window_get_geometry
#endif

#include <cairo.h>
#include <cairo-xlib.h>

#if CAIRO_VERSION < CAIRO_VERSION_ENCODE(1, 1, 0)
#define CAIRO_EXTEND_PAD CAIRO_EXTEND_NONE
#endif

#include <pango/pango-context.h>
#include <pango/pangocairo.h>

#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <limits.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>

#include <libintl.h>
#define _(x)  gettext (x)
#define N_(x) x

#ifdef USE_METACITY
#include <metacity-private/theme.h>
#endif

#include "gwd-settings-interface.h"

#define METACITY_GCONF_DIR "/apps/metacity/general"
#define MUTTER_GCONF_DIR "/apps/mutter/general"

#define COMPIZ_USE_SYSTEM_FONT_KEY		    \
METACITY_GCONF_DIR "/titlebar_uses_system_font"
		    
#define COMPIZ_TITLEBAR_FONT_KEY	\
METACITY_GCONF_DIR "/titlebar_font"

#define COMPIZ_DOUBLE_CLICK_TITLEBAR_KEY	       \
METACITY_GCONF_DIR "/action_double_click_titlebar"

#define COMPIZ_MIDDLE_CLICK_TITLEBAR_KEY	       \
METACITY_GCONF_DIR "/action_middle_click_titlebar"

#define COMPIZ_RIGHT_CLICK_TITLEBAR_KEY	       \
METACITY_GCONF_DIR "/action_right_click_titlebar"

#define MUTTER_DRAGGABLE_BORDER_WIDTH_KEY \
MUTTER_GCONF_DIR "/draggable_border_width"

#define MUTTER_ATTACH_MODAL_DIALOGS_KEY \
MUTTER_GCONF_DIR "/attach_modal_dialogs"

#define META_THEME_KEY		\
METACITY_GCONF_DIR "/theme"

#define META_BUTTON_LAYOUT_KEY		\
METACITY_GCONF_DIR "/button_layout"

#define GCONF_DIR "/apps/gwd"

#define USE_META_THEME_KEY	    \
GCONF_DIR "/use_metacity_theme"

#define META_THEME_OPACITY_KEY	        \
GCONF_DIR "/metacity_theme_opacity"

#define META_THEME_SHADE_OPACITY_KEY	      \
GCONF_DIR "/metacity_theme_shade_opacity"

#define META_THEME_ACTIVE_OPACITY_KEY	       \
GCONF_DIR "/metacity_theme_active_opacity"

#define META_THEME_ACTIVE_SHADE_OPACITY_KEY          \
GCONF_DIR "/metacity_theme_active_shade_opacity"

#define BLUR_TYPE_KEY	   \
GCONF_DIR "/blur_type"

#define WHEEL_ACTION_KEY   \
GCONF_DIR "/mouse_wheel_action"

#define USE_TOOLTIPS_KEY \
GCONF_DIR "/use_tooltips"

#define DBUS_DEST       "org.freedesktop.compiz"
#define DBUS_PATH       "/org/freedesktop/compiz/decor/screen0"
#define DBUS_INTERFACE  "org.freedesktop.compiz"
#define DBUS_METHOD_GET "get"

extern const float STROKE_ALPHA;

extern const unsigned short ICON_SPACE;

extern const float DOUBLE_CLICK_DISTANCE;

#define WM_MOVERESIZE_SIZE_TOPLEFT      0
#define WM_MOVERESIZE_SIZE_TOP          1
#define WM_MOVERESIZE_SIZE_TOPRIGHT     2
#define WM_MOVERESIZE_SIZE_RIGHT        3
#define WM_MOVERESIZE_SIZE_BOTTOMRIGHT  4
#define WM_MOVERESIZE_SIZE_BOTTOM       5
#define WM_MOVERESIZE_SIZE_BOTTOMLEFT   6
#define WM_MOVERESIZE_SIZE_LEFT         7
#define WM_MOVERESIZE_MOVE              8
#define WM_MOVERESIZE_SIZE_KEYBOARD     9
#define WM_MOVERESIZE_MOVE_KEYBOARD    10

extern const float	    SHADOW_RADIUS;
extern const float	    SHADOW_OPACITY;
extern const unsigned short SHADOW_OFFSET_X;
extern const unsigned short SHADOW_OFFSET_Y;
#define SHADOW_COLOR_RED   0x0000
#define SHADOW_COLOR_GREEN 0x0000
#define SHADOW_COLOR_BLUE  0x0000

extern const float META_OPACITY;
#define META_SHADE_OPACITY	    TRUE;
extern const float META_ACTIVE_OPACITY;
#define META_ACTIVE_SHADE_OPACITY   TRUE;

#define META_MAXIMIZED (WNCK_WINDOW_STATE_MAXIMIZED_HORIZONTALLY | \
WNCK_WINDOW_STATE_MAXIMIZED_VERTICALLY)

#define CMDLINE_OPACITY              (1 << 0)
#define CMDLINE_OPACITY_SHADE        (1 << 1)
#define CMDLINE_ACTIVE_OPACITY       (1 << 2)
#define CMDLINE_ACTIVE_OPACITY_SHADE (1 << 3)
#define CMDLINE_BLUR                 (1 << 4)
#define CMDLINE_THEME                (1 << 5)

#define MWM_HINTS_DECORATIONS (1L << 1)

#define MWM_DECOR_ALL      (1L << 0)
#define MWM_DECOR_BORDER   (1L << 1)
#define MWM_DECOR_HANDLE   (1L << 2)
#define MWM_DECOR_TITLE    (1L << 3)
#define MWM_DECOR_MENU     (1L << 4)
#define MWM_DECOR_MINIMIZE (1L << 5)
#define MWM_DECOR_MAXIMIZE (1L << 6)

#define PROP_MOTIF_WM_HINT_ELEMENTS 3

typedef struct _GWDSettingsWritable GWDSettingsWritable;
typedef struct _GWDSettingsNotified GWDSettingsNotified;

typedef struct {
unsigned long flags;
unsigned long functions;
unsigned long decorations;
} MwmHints;

extern gboolean minimal;

#define SWITCHER_SPACE 40

extern GWDSettingsNotified *notified;
extern GWDSettings	   *settings;
extern GWDSettingsWritable *writable;

extern gdouble decoration_alpha;
#ifdef USE_METACITY
extern MetaButtonLayout   meta_button_layout;
extern gboolean	          meta_button_layout_set;
#endif

extern Atom frame_input_window_atom;
extern Atom frame_output_window_atom;
extern Atom win_decor_atom;
extern Atom win_blur_decor_atom;
extern Atom wm_move_resize_atom;
extern Atom restack_window_atom;
extern Atom select_window_atom;
extern Atom mwm_hints_atom;
extern Atom switcher_fg_atom;
extern Atom compiz_shadow_info_atom;
extern Atom compiz_shadow_color_atom;
extern Atom toolkit_action_atom;
extern Atom toolkit_action_window_menu_atom;
extern Atom toolkit_action_force_quit_dialog_atom;
extern Atom net_wm_state_atom;
extern Atom net_wm_state_modal_atom;
extern Atom decor_request_atom;
extern Atom decor_pending_atom;
extern Atom decor_delete_pixmap_atom;

extern Time dm_sn_timestamp;

#define C(name) { 0, XC_ ## name }

struct _cursor {
    Cursor	 cursor;
    unsigned int shape;
};

extern struct _cursor cursor[3][3];

#define BUTTON_CLOSE   0
#define BUTTON_MAX     1
#define BUTTON_MIN     2
#define BUTTON_MENU    3
#define BUTTON_SHADE   4
#define BUTTON_ABOVE   5
#define BUTTON_STICK   6
#define BUTTON_UNSHADE 7
#define BUTTON_UNABOVE 8
#define BUTTON_UNSTICK 9
#define BUTTON_NUM     10

struct _pos {
    int x, y, w, h;
    int xw, yh, ww, hh, yth, hth;
};

extern struct _pos pos[3][3], bpos[];

typedef struct _decor_color {
    double r;
    double g;
    double b;
} decor_color_t;


#define IN_EVENT_WINDOW      (1 << 0)
#define PRESSED_EVENT_WINDOW (1 << 1)

typedef struct _decor_event {
    guint time;
    guint window;
    guint x;
    guint y;
    guint x_root;
    guint y_root;
    guint button;
} decor_event;

typedef enum _decor_event_type {
    GButtonPress = 1,
    GButtonRelease,
    GEnterNotify,
    GLeaveNotify,
    GMotionNotify
} decor_event_type;

typedef void (*event_callback) (WnckWindow       *win,
				decor_event      *gtkwd_event,
				decor_event_type gtkwd_type);

typedef struct {
    Window         window;
    Box            pos;
    event_callback callback;
} event_window;

typedef struct _decor_frame decor_frame_t;
typedef struct _decor_shadow_info decor_shadow_info_t;

struct _decor_shadow_info
{
    decor_frame_t *frame;
    unsigned int  state;
    gboolean      active;
};

typedef void (*frame_update_shadow_proc) (Display		 *display,
					  Screen		 *screen,
					  decor_frame_t		 *frame,
					  decor_shadow_t	  **shadow_normal,
					  decor_context_t	  *context_normal,
					  decor_shadow_t	  **shadow_max,
					  decor_context_t	  *context_max,
					  decor_shadow_info_t    *info,
					  decor_shadow_options_t *opt_shadow,
					  decor_shadow_options_t *opt_no_shadow);

typedef decor_frame_t * (*create_frame_proc) (const gchar *);
typedef void (*destroy_frame_proc) (decor_frame_t *);

struct _decor_frame {
    decor_extents_t win_extents;
    decor_extents_t max_win_extents;
    int		    titlebar_height;
    int		    max_titlebar_height;
    decor_shadow_t *border_shadow_active;
    decor_shadow_t *border_shadow_inactive;
    decor_shadow_t *border_no_shadow;
    decor_shadow_t *max_border_shadow_active;
    decor_shadow_t *max_border_shadow_inactive;
    decor_shadow_t *max_border_no_shadow;
    decor_context_t window_context_active;
    decor_context_t window_context_inactive;
    decor_context_t window_context_no_shadow;
    decor_context_t max_window_context_active;
    decor_context_t max_window_context_inactive;
    decor_context_t max_window_context_no_shadow;
    PangoFontDescription *titlebar_font;
    PangoContext	 *pango_context;
    GtkWidget	         *style_window_rgba;
    GtkWidget		 *style_window_rgb;
    gint		 text_height;
    gchar		 *type;

    frame_update_shadow_proc update_shadow;
    gint		refcount;
};

typedef struct _decor {
    WnckWindow	      *win;
    decor_frame_t     *frame;
    event_window      event_windows[3][3];
    event_window      button_windows[BUTTON_NUM];
    Box		      *last_pos_entered;
    guint	      button_states[BUTTON_NUM];
    GdkPixmap	      *pixmap;
    GdkPixmap	      *buffer_pixmap;
    GdkWindow	      *frame_window;
    GtkWidget         *decor_window;
    GtkWidget	      *decor_event_box;
    GtkWidget         *decor_image;
    GHashTable        *old_pixmaps;
    cairo_t	      *cr;
    decor_layout_t    border_layout;
    decor_context_t   *context;
    decor_shadow_t    *shadow;
    Picture	      picture;
    gint	      button_width;
    gint	      width;
    gint	      height;
    gint	      client_width;
    gint	      client_height;
    gboolean	      decorated;
    gboolean	      active;
    PangoLayout	      *layout;
    gchar	      *name;
    cairo_pattern_t   *icon;
    GdkPixmap	      *icon_pixmap;
    GdkPixbuf	      *icon_pixbuf;
    WnckWindowState   state;
    WnckWindowActions actions;
    XID		      prop_xid;
    GtkWidget	      *force_quit_dialog;
    Bool	      created;
    void	      (*draw) (struct _decor *d);
} decor_t;

#define WINDOW_TYPE_FRAMES_NUM 5
typedef struct _default_frame_references
{
    char     *name;
    decor_t  *d;
} default_frame_references_t;

extern default_frame_references_t default_frames[WINDOW_TYPE_FRAMES_NUM * 2];
const gchar * window_type_frames[WINDOW_TYPE_FRAMES_NUM];

void     (*theme_get_shadow)		    (decor_frame_t *d,
					     decor_shadow_options_t *,
					     gboolean);

void     (*theme_draw_window_decoration)    (decor_t *d);
gboolean (*theme_calc_decoration_size)      (decor_t *d,
					     int     client_width,
					     int     client_height,
					     int     text_width,
					     int     *width,
					     int     *height);
void     (*theme_update_border_extents)     (decor_frame_t *frame);
void     (*theme_get_event_window_position) (decor_t *d,

					     gint    i,
					     gint    j,
					     gint    width,
					     gint    height,
					     gint    *x,
					     gint    *y,
					     gint    *w,
					     gint    *h);
gboolean (*theme_get_button_position)       (decor_t *d,
					     gint    i,
					     gint    width,
					     gint    height,
					     gint    *x,
					     gint    *y,
					     gint    *w,
					     gint    *h);
gfloat (*theme_get_title_scale)		    (decor_frame_t *frame);

extern char *program_name;

/* list of all decorations */
extern GHashTable    *frame_table;
extern GHashTable    *destroyed_pixmaps_table;

/* action menu */
extern GtkWidget     *action_menu;
extern gboolean      action_menu_mapped;
extern decor_color_t _title_color[2];
extern gint	     double_click_timeout;


/* tooltip */
extern GtkWidget     *tip_window;
extern GtkWidget     *tip_label;
extern GTimeVal	     tooltip_last_popdown;
extern gint	     tooltip_timer_tag;

extern GSList *draw_list;
extern guint  draw_idle_id;

/* switcher */
extern Window     switcher_selected_window;
extern GtkWidget  *switcher_label;
extern decor_t    *switcher_window;

extern XRenderPictFormat *xformat_rgba;
extern XRenderPictFormat *xformat_rgb;

/* gtk-window-decorator.c */

double
dist (double x1, double y1,
      double x2, double y2);

/* frames.c */

void
initialize_decorations ();

void
update_frames_border_extents (gpointer key,
			      gpointer value,
			      gpointer user_data);

decor_frame_t *
gwd_get_decor_frame (const gchar *);

decor_frame_t *
gwd_decor_frame_ref (decor_frame_t *);

decor_frame_t *
gwd_decor_frame_unref (decor_frame_t *);

void
gwd_frames_foreach (GHFunc   foreach_func,
		    gpointer user_data);

void
gwd_process_frames (GHFunc	foreach_func,
		    const gchar	*frame_keys[],
		    gint	frame_keys_num,
		    gpointer	user_data);

decor_frame_t *
decor_frame_new (const gchar *type);

void
decor_frame_destroy (decor_frame_t *);

/* decorator.c */

void
frame_update_shadow (decor_frame_t	    *frame,
		     decor_shadow_info_t    *info,
		     decor_shadow_options_t *opt_shadow,
		     decor_shadow_options_t *opt_no_shadow);

void
frame_update_titlebar_font (decor_frame_t *frame);

void
bare_frame_update_shadow (Display		  *xdisplay,
			   Screen		  *screen,
			   decor_frame_t	  *frame,
			   decor_shadow_t	  **shadow_normal,
			   decor_context_t	  *context_normal,
			   decor_shadow_t	  **shadow_max,
			   decor_context_t	  *context_max,
			   decor_shadow_info_t    *info,
			   decor_shadow_options_t *opt_shadow,
			   decor_shadow_options_t *opt_no_shadow);

void
decor_frame_update_shadow (Display		  *xdisplay,
			   Screen		  *screen,
			   decor_frame_t	  *frame,
			   decor_shadow_t	  **shadow_normal,
			   decor_context_t	  *context_normal,
			   decor_shadow_t	  **shadow_max,
			   decor_context_t	  *context_max,
			   decor_shadow_info_t    *info,
			   decor_shadow_options_t *opt_shadow,
			   decor_shadow_options_t *opt_no_shadow);

decor_frame_t *
create_normal_frame (const gchar *type);

void
destroy_normal_frame ();

decor_frame_t *
create_bare_frame (const gchar *type);

void
destroy_bare_frame ();

/* Don't use directly */
gboolean
update_window_decoration_size (WnckWindow *win);

gboolean
request_update_window_decoration_size (WnckWindow *win);

void
update_window_decoration_name (WnckWindow *win);

gint
max_window_name_width (WnckWindow *win);

unsigned int
populate_frame_type (decor_t *d);

unsigned int
populate_frame_state (decor_t *d);

unsigned int
populate_frame_actions (decor_t *d);

void
update_default_decorations (GdkScreen *screen);

void
update_window_decoration_state (WnckWindow *win);

void
update_window_decoration_actions (WnckWindow *win);

void
update_window_decoration_icon (WnckWindow *win);

void
update_event_windows (WnckWindow *win);

int
update_shadow (void);

void
update_titlebar_font ();

void
update_window_decoration_name (WnckWindow *win);

void
update_window_decoration (WnckWindow *win);

void
queue_decor_draw (decor_t *d);

void
copy_to_front_buffer (decor_t *d);

/* wnck.c*/

const gchar *
get_frame_type (WnckWindow *win);

void
decorations_changed (WnckScreen *screen);

void
connect_screen (WnckScreen *screen);

void
window_opened (WnckScreen *screen,
	       WnckWindow *window);

void
window_closed (WnckScreen *screen,
	       WnckWindow *window);

void
add_frame_window (WnckWindow *win,
		  Window     frame,
		  Bool	     mode);

void
remove_frame_window (WnckWindow *win);

void
restack_window (WnckWindow *win,
		int	   stack_mode);

void
connect_window (WnckWindow *win);


/* blur.c */

void
decor_update_blur_property (decor_t *d,
			    int     width,
			    int     height,
			    Region  top_region,
			    int     top_offset,
			    Region  bottom_region,
			    int     bottom_offset,
			    Region  left_region,
			    int     left_offset,
			    Region  right_region,
			    int     right_offset);

/* decorprops.c */

void
decor_update_window_property (decor_t *d);

void
decor_update_switcher_property (decor_t *d);

/* cairo.c */

#define CORNER_TOPLEFT     (1 << 0)
#define CORNER_TOPRIGHT    (1 << 1)
#define CORNER_BOTTOMRIGHT (1 << 2)
#define CORNER_BOTTOMLEFT  (1 << 3)

#define SHADE_LEFT   (1 << 0)
#define SHADE_RIGHT  (1 << 1)
#define SHADE_TOP    (1 << 2)
#define SHADE_BOTTOM (1 << 3)

void
draw_shadow_background (decor_t		*d,
			cairo_t		*cr,
			decor_shadow_t  *s,
			decor_context_t *c);

void
draw_window_decoration (decor_t *d);

void
fill_rounded_rectangle (cairo_t       *cr,
			double        x,
			double        y,
			double        w,
			double        h,
			double	      radius,
			int	      corner,
			decor_color_t *c0,
			double        alpha0,
			decor_color_t *c1,
			double	      alpha1,
			int	      gravity);

void
rounded_rectangle (cairo_t *cr,
		   double  x,
		   double  y,
		   double  w,
		   double  h,
		   double  radius,
		   int	   corner);

gboolean
calc_decoration_size (decor_t *d,
		      gint    w,
		      gint    h,
		      gint    name_width,
		      gint    *width,
		      gint    *height);

void
update_border_extents ();

gboolean
get_button_position (decor_t *d,
		     gint    i,
		     gint    width,
		     gint    height,
		     gint    *x,
		     gint    *y,
		     gint    *w,
		     gint    *h);

void
get_event_window_position (decor_t *d,
			   gint    i,
			   gint    j,
			   gint    width,
			   gint    height,
			   gint    *x,
			   gint    *y,
			   gint    *w,
			   gint    *h);

gfloat
get_title_scale (decor_frame_t *frame);

void
cairo_get_shadow (decor_frame_t *, decor_shadow_options_t *opts, gboolean active);

/* gdk.c */

void
gdk_cairo_set_source_color_alpha (cairo_t  *cr,
				  GdkColor *color,
				  double   alpha);

GdkWindow *
create_gdk_window (Window xframe);

GdkColormap *
get_colormap_for_drawable (GdkDrawable *d);

XRenderPictFormat *
get_format_for_drawable (decor_t *d, GdkDrawable *drawable);

GdkPixmap *
create_pixmap (int	 w,
	       int	 h,
	       GtkWidget *parent_style_window);

GdkPixmap *
pixmap_new_from_pixbuf (GdkPixbuf *pixbuf, GtkWidget *parent);

/* metacity.c */
#ifdef USE_METACITY

MetaFrameType
meta_get_frame_type_for_decor_type (const gchar *frame_type);

void
meta_draw_window_decoration (decor_t *d);

void
meta_get_decoration_geometry (decor_t		*d,
			      MetaTheme	        *theme,
			      MetaFrameFlags    *flags,
			      MetaFrameGeometry *fgeom,
			      MetaButtonLayout  *button_layout,
			      MetaFrameType     frame_type,
			      GdkRectangle      *clip);

void
meta_calc_button_size (decor_t *d);

gboolean
meta_calc_decoration_size (decor_t *d,
			   gint    w,
			   gint    h,
			   gint    name_width,
			   gint    *width,
			   gint    *height);

gboolean
meta_get_button_position (decor_t *d,
			  gint    i,
			  gint	  width,
			  gint	  height,
			  gint    *x,
			  gint    *y,
			  gint    *w,
			  gint    *h);

gboolean
meta_button_present (MetaButtonLayout   *button_layout,
		     MetaButtonFunction function);

void
meta_get_event_window_position (decor_t *d,
				gint    i,
				gint    j,
				gint	width,
				gint	height,
				gint    *x,
				gint    *y,
				gint    *w,
				gint    *h);

gfloat
meta_get_title_scale (decor_frame_t *);

void
meta_update_border_extents ();

void
meta_update_button_layout (const char *value);

void
meta_get_shadow (decor_frame_t *, decor_shadow_options_t *opts, gboolean active);

#endif
/* switcher.c */

#define SWITCHER_ALPHA 0xa0a0

void
switcher_frame_update_shadow (Display		  *xdisplay,
			      Screen		  *screen,
			      decor_frame_t	  *frame,
			      decor_shadow_t	  **shadow_normal,
			      decor_context_t	  *context_normal,
			      decor_shadow_t	  **shadow_max,
			      decor_context_t	  *context_max,
			      decor_shadow_info_t    *info,
			      decor_shadow_options_t *opt_shadow,
			      decor_shadow_options_t *opt_no_shadow);

decor_frame_t *
create_switcher_frame (const gchar *);

void
destroy_switcher_frame ();

void
draw_switcher_decoration (decor_t *d);

gboolean
update_switcher_window (Window     popup,
			Window     selected);

decor_t *
switcher_window_opened (Window popup, Window selected);

void
switcher_window_closed ();

/* events.c */

void
move_resize_window (WnckWindow *win,
		    int	       direction,
		    decor_event *gtkwd_event);

void
common_button_event (WnckWindow *win,
		     decor_event *gtkwd_event,
		     decor_event_type gtkwd_type,
		     int	button,
		     int	max,
		     char	*tooltip);

void
close_button_event (WnckWindow *win,
		    decor_event *gtkwd_event,
		    decor_event_type gtkwd_type);

void
max_button_event (WnckWindow *win,
		  decor_event *gtkwd_event,
		  decor_event_type gtkwd_type);

void
min_button_event (WnckWindow *win,
		  decor_event *gtkwd_event,
		  decor_event_type gtkwd_type);

void
menu_button_event (WnckWindow *win,
		   decor_event *gtkwd_event,
		   decor_event_type gtkwd_type);

void
shade_button_event (WnckWindow *win,
		    decor_event *gtkwd_event,
		    decor_event_type gtkwd_type);

void
above_button_event (WnckWindow *win,
		    decor_event *gtkwd_event,
		    decor_event_type gtkwd_type);

void
stick_button_event (WnckWindow *win,
		    decor_event *gtkwd_event,
		    decor_event_type gtkwd_type);
void
unshade_button_event (WnckWindow *win,
		      decor_event *gtkwd_event,
		      decor_event_type gtkwd_type);

void
unabove_button_event (WnckWindow *win,
		      decor_event *gtkwd_event,
		      decor_event_type gtkwd_type);

void
unstick_button_event (WnckWindow *win,
		      decor_event *gtkwd_event,
		      decor_event_type gtkwd_type);

void
handle_title_button_event (WnckWindow   *win,
			   int          action,
			   decor_event *gtkwd_event);

void
handle_mouse_wheel_title_event (WnckWindow   *win,
				unsigned int button);

void
title_event (WnckWindow       *win,
	     decor_event      *gtkwd_event,
	     decor_event_type gtkwd_type);

void
frame_common_event (WnckWindow       *win,
		    int              direction,
		    decor_event      *gtkwd_event,
		    decor_event_type gtkwd_type);

void
top_left_event (WnckWindow       *win,
		decor_event      *gtkwd_event,
		decor_event_type gtkwd_type);

void
top_event (WnckWindow       *win,
	   decor_event      *gtkwd_event,
	   decor_event_type gtkwd_type);

void
top_right_event (WnckWindow       *win,
		 decor_event      *gtkwd_event,
		 decor_event_type gtkwd_type);

void
left_event (WnckWindow       *win,
	    decor_event      *gtkwd_event,
	    decor_event_type gtkwd_type);
void
right_event (WnckWindow       *win,
	     decor_event      *gtkwd_event,
	     decor_event_type gtkwd_type);

void
bottom_left_event (WnckWindow *win,
		   decor_event *gtkwd_event,
		   decor_event_type gtkwd_type);

void
bottom_event (WnckWindow *win,
	      decor_event *gtkwd_event,
	      decor_event_type gtkwd_type);
void
bottom_right_event (WnckWindow *win,
		    decor_event *gtkwd_event,
		    decor_event_type gtkwd_type);

void
frame_window_realized (GtkWidget *widget,
		       gpointer  data);

event_callback
find_event_callback_for_point (decor_t *d,
			       int     x,
			       int     y,
			       Bool    *enter,
			       Bool    *leave,
			       BoxPtr  *entered_box);

event_callback
find_leave_event_callback (decor_t *d);

void
frame_handle_button_press (GtkWidget      *widget,
			   GdkEventButton *event,
			   gpointer       user_data);

void
frame_handle_button_release (GtkWidget      *widget,
			     GdkEventButton *event,
			     gpointer       user_data);

void
frame_handle_motion (GtkWidget      *widget,
		     GdkEventMotion *event,
		     gpointer       user_data);

GdkFilterReturn
selection_event_filter_func (GdkXEvent *gdkxevent,
			     GdkEvent  *event,
			     gpointer  data);

GdkFilterReturn
event_filter_func (GdkXEvent *gdkxevent,
		   GdkEvent  *event,
		   gpointer  data);

/* tooltip.c */

gboolean
create_tooltip_window (void);

void
handle_tooltip_event (WnckWindow *win,
		      decor_event *gtkwd_event,
		      decor_event_type   gtkwd_type,
		      guint	 state,
		      const char *tip);

/* forcequit.c */

void
show_force_quit_dialog (WnckWindow *win,
			Time        timestamp);

void
hide_force_quit_dialog (WnckWindow *win);

/* actionmenu.c */

void
action_menu_map (WnckWindow *win,
		 long	     button,
		 Time	     time);

/* util.c */

double
square (double x);

double
dist (double x1, double y1,
      double x2, double y2);

void
shade (const decor_color_t *a,
       decor_color_t	   *b,
       float		   k);

gboolean
get_window_prop (Window xwindow,
		 Atom   atom,
		 Window *val);

unsigned int
get_mwm_prop (Window xwindow);


/* style.c */

void
update_style (GtkWidget *widget);

void
style_changed (GtkWidget *widget, void *user_data /* PangoContext */);

/* settings.c */

void
set_frame_scale (decor_frame_t *frame,
		 const gchar *font_str);

void
set_frames_scales (gpointer key,
		   gpointer value,
		   gpointer user_data);

gboolean
init_settings (GWDSettingsWritable *writable, WnckScreen *screen);

void
fini_settings ();

gboolean
gwd_process_decor_shadow_property_update ();

#endif
