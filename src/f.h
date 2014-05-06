
#ifndef _LOADER_H
#include "loader.h"
#endif


/* ogl *************************************************/


void init_cam ();
void drawt();
/*init*/
void realize (GtkWidget *widget, gpointer data);
/*reshape*/
gboolean configure_event (GtkWidget *widget, GdkEventConfigure *event, gpointer data);
/*draw*/
gboolean expose_event (GtkWidget *widget, GdkEventExpose *event, gpointer data);
gboolean redraw(GtkWidget * widget);

/*mouse*/
gboolean motion_notify_event (GtkWidget      *widget,
				     GdkEventMotion *event,
				     gpointer        data);
gboolean button_press_event (GtkWidget      *widget,
				    GdkEventButton *event,
				    gpointer        data);
/*kb*/
gboolean key_press_event (GtkWidget *widget, GdkEventKey *event, gpointer data);

/* gtk->gl */
void majpos (gpointer data);
void toggleanim ();



/* main_gtk **********/

void init_gtk_nfs ();

gint delete_event( GtkWidget *widget,
                   GdkEvent  *event,
		   gpointer   data );

void destroy( GtkWidget *widget,
              gpointer   data );
/* alexandre... t'es un putain de connard */
