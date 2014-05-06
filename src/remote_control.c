#include <gtk/gtk.h>
#include <stdlib.h>
#include "lib_bbs_behavior.h"
#include <math.h>
#include "loader.h"
#include "divers.h"
#include "remote_control.h"
#include "f.h"
#include "camera.h"
#include "float.h"

extern char * chemin;
int remote_control_opened = -1;
GtkWidget *win;  
extern struct camera cam;
/* var globales utilisées dans traiter fourmi ? */
extern double tps;
extern double tps_cam;

int camzp=0;
int camzm=0;
int camap=0;
int camam=0;
int cambp=0;
int cambm=0;
int camxp=0;
int camxm=0;
int camyp=0;
int camym=0;
int cacai=0;
float delta=FLT_MAX;

void set_cam_delta()
{
  delta=FLT_MAX;
}

int destroy2( GtkWidget *widget UNUSED,
	       gpointer   data UNUSED )
{
  remote_control_opened = 0;
  gtk_widget_hide_all (win);
  return 1;
}

gint delete_event2( GtkWidget *widget,
                   GdkEvent  *event UNUSED,
		   gpointer   data )
{
  destroy2(widget,data);
  return 1;
}

/* ULTRA DEPRECATED
   on a pas idee de faire du code aussi moche...
   je le laisse pour les archives MOUHAHAHAHAHAHA 

void camzp_p ()
{     
  camzp = 1;
}
void camzp_r ()
{     
  camzp = 0;
}

void camzm_p ()
{
  camzm = 1;
}
void camzm_r ()
{
  camzm = 0;
}

void camap_p ()
{     
  camap = 1;
}
void camap_r ()
{     
  camap = 0;
}


void camam_p ()
{
  camam = 1;
}
void camam_r ()
{
  camam = 0;
}

void cambp_p ()
{     
  cambp = 1;
}
void cambp_r ()
{     
  cambp = 0;
}

void cambm_p ()
{
  cambm = 1;
}
void cambm_r ()
{
  cambm = 0;
}

void camxp_p ()
{     
  camxp = 1;
}
void camxp_r ()
{     
  camxp = 0;
}

void camxm_p ()
{
  camxm = 1;
}
void camxm_r ()
{
  camxm = 0;
}

void camyp_p ()
{     
  camyp = 1;
}

void camyp_r ()
{     
  camyp = 0;
}

void camym_p ()
{
  camym = 1;
}
void camym_r ()
{
  camym = 0;
}
*/


gboolean set_one (GtkWidget * widget UNUSED, GdkEventButton *event UNUSED, gpointer data)
{
  * (int *) data = 1;
  return(0);
} 

gboolean set_zero (GtkWidget * widget UNUSED, GdkEventButton *event UNUSED, gpointer data)
{
  * (int *) data = 0;
  return(0);
} 
  
void modif_cam_remote_ctl ()
{
  /*float t, yg, xg;*/
  float delta2;
  cam.alpha = (int) (cam.alpha + tps_cam*VITR*(camap - camam) +360)%360;
  cam.beta = (int) (cam.beta + tps_cam*VITR*(cambp - cambm) +360)%360;

  cam.zoom += tps_cam*VITZ*(camzp - camzm);
  if (camxp || camxm || camyp || camym)
   { 
      cam.follow_agent = 0;
      cam.reached = 1;
      cam.xgoto = cam.x+tps_cam*(abs(cam.zoom/10) + 1)*VITXY*((camym-camyp)*my_cos(cam.alpha)+(camxm-camxp)*my_cos(cam.alpha + 90));
      cam.ygoto = cam.y+tps_cam*(abs(cam.zoom/10) + 1)*VITXY*((camym-camyp)*my_sin(cam.alpha)+(camxm-camxp)*my_sin(cam.alpha + 90));
    }
   
  if (cam.follow_agent)
    {
      cam.xgoto = (*cam.follow_agent).x;
      cam.ygoto = (*cam.follow_agent).z;
    }
  if (cam.x != cam.xgoto || cam.y != cam.ygoto)
    {
      if (cam.reached)
	{	
	  cam.x = cam.xgoto;
	  cam.y = cam.ygoto;
	}
      else 
	{
	  /*printf("x%f xg%f y%f yg%f\n",cam.x,cam.xgoto,cam.y,cam.ygoto);*/

	  /*if (mfabs(cam.xgoto - cam.x) < 0.4)
	    {
	      cam.x = cam.xgoto;
	      if (mfabs(cam.ygoto - cam.y) < 0.8)
		cam.y = cam.ygoto;
	    }
	  if (mfabs(cam.ygoto - cam.y) < 0.4)
	     {
	       cam.y = cam.ygoto;
	       if (mfabs(cam.xgoto - cam.x) < 0.8)
		 cam.x = cam.xgoto;
	     }
	  
	  t = cam.xgoto - cam.x;
	  if ((t > -0.4) && (t < 0))
	    t = -0.4;
	  else
	    { 
	      if ((t > 0) && (t < 0.4))
		t = 0.4;
	    }
	  //t += (t>0 ? 1.0 : -1.0)*tps;
	  cam.x += VIT_CAM_MV*tps*(t);
	  t = cam.ygoto - cam.y;
	  if ((t>-0.4) && (t<0))
	    t=-0.4;
	  else
	    { 
	      if ((t>0) && (t<0.4))
		t=0.4;
	    }
	  //t += (t>0 ? 1.0 : -1.0)*tps;
	  cam.y += VIT_CAM_MV*tps*(t);*/
	  /*	    }

	    cam.x += (cam.xgoto - cam.x) / 64;
	    cam.y += (cam.ygoto - cam.y) / 64;
 */


	  if (tps_cam)
	    {
	      delta2 = (cam.xgoto - cam.x) * (cam.xgoto - cam.x) + (cam.ygoto - cam.y) * (cam.ygoto - cam.y);
	      if (delta2 >= delta)
		{
		  cam.x = cam.xgoto;
		  cam.y = cam.ygoto;
		}
	      else
		{
		  cam.x += (cam.xgoto - cam.x) * tps_cam;
		  cam.y += (cam.ygoto - cam.y) * tps_cam;
		}
	      delta = delta2;
	      }
	  if ((mfabs(cam.xgoto-cam.x) <= 0.01) && (mfabs(cam.ygoto-cam.y) <= 0.01))
	    {
	      delta = FLT_MAX;
	      cam.reached = 1;
	    }
	}
    }
}

int create_remote_control ( GtkWidget *widget UNUSED,
			    GdkEvent  *event UNUSED,
			    gpointer   data  UNUSED)
{
  if (remote_control_opened == -1)
    {
      GtkWidget *table;
      GtkWidget *buts, *dingbat;
      char * tmp_str;

      /* create window */

      win=gtk_window_new(GTK_WINDOW_TOPLEVEL); 
      gtk_window_set_title (GTK_WINDOW (win), "Remote Control");
      /*gtk_window_set_default_size(GTK_WINDOW (win),285,171);*/
      /*gtk_container_set_border_width (GTK_CONTAINER (win), 10);*/
      g_signal_connect (G_OBJECT (win), "delete_event",
			G_CALLBACK (delete_event2), NULL);
      g_signal_connect (G_OBJECT (win), "destroy",
			G_CALLBACK (destroy2), win);
      gtk_window_set_keep_above (GTK_WINDOW (win),1);



      /* create buttons */


      table = gtk_table_new (3, 5, 1);
      gtk_container_add (GTK_CONTAINER (win), table);      

      /* up */
      buts=gtk_button_new();
      tmp_str = my_concat(chemin,"/../share/bebetes_show/remote_control/ht.png");
      dingbat = gtk_image_new_from_file(tmp_str);
      free(tmp_str);
      gtk_container_add (GTK_CONTAINER (buts),dingbat);
      gtk_table_attach_defaults (GTK_TABLE (table), buts,1,2,0,1);
      g_signal_connect (buts, "button-press-event",G_CALLBACK (set_one), &camxp);
      g_signal_connect (buts, "button-release-event",G_CALLBACK (set_zero), &camxp);
      
      /* down */
      buts=gtk_button_new();
      tmp_str = my_concat(chemin,"/../share/bebetes_show/remote_control/bs.png");
      dingbat = gtk_image_new_from_file(tmp_str);
      free(tmp_str);
      gtk_container_add (GTK_CONTAINER (buts),dingbat);
      gtk_table_attach_defaults (GTK_TABLE (table), buts,1,2,2,3);
      g_signal_connect (buts, "button-press-event",G_CALLBACK (set_one), &camxm);
      g_signal_connect (buts, "button-release-event",G_CALLBACK (set_zero), &camxm);

      /* home */
      buts=gtk_button_new();
      tmp_str = my_concat(chemin,"/../share/bebetes_show/remote_control/home.png");
      dingbat = gtk_image_new_from_file(tmp_str);
      free(tmp_str);
      gtk_container_add (GTK_CONTAINER (buts),dingbat);
      gtk_table_attach_defaults (GTK_TABLE (table), buts,1,2,1,2);
      g_signal_connect (buts, "clicked",G_CALLBACK (init_cam), NULL);

      /* left */
      buts=gtk_button_new();
      tmp_str = my_concat(chemin,"/../share/bebetes_show/remote_control/gc.png");
      dingbat = gtk_image_new_from_file(tmp_str);
      free(tmp_str);
      gtk_container_add (GTK_CONTAINER (buts),dingbat);
      gtk_table_attach_defaults (GTK_TABLE (table), buts,0,1,1,2);
      g_signal_connect (buts, "button-press-event",G_CALLBACK (set_one), &camyp);
      g_signal_connect (buts, "button-release-event",G_CALLBACK (set_zero), &camyp);

      /* right */
      buts=gtk_button_new();
      tmp_str = my_concat(chemin,"/../share/bebetes_show/remote_control/dr.png");
      dingbat = gtk_image_new_from_file(tmp_str);
      free(tmp_str);
      gtk_container_add (GTK_CONTAINER (buts),dingbat);
      gtk_table_attach_defaults (GTK_TABLE (table), buts,2,3,1,2);
      g_signal_connect (buts, "button-press-event",G_CALLBACK (set_one), &camym);
      g_signal_connect (buts, "button-release-event",G_CALLBACK (set_zero), &camym);


      /* + */
      buts=gtk_button_new();
      tmp_str = my_concat(chemin,"/../share/bebetes_show/remote_control/zoom.png");
      dingbat = gtk_image_new_from_file(tmp_str);
      free(tmp_str);
      gtk_container_add (GTK_CONTAINER (buts),dingbat);
      gtk_table_attach_defaults (GTK_TABLE (table), buts,3,4,0,1);
      g_signal_connect (buts, "button-press-event",G_CALLBACK (set_one), &camzp);
      g_signal_connect (buts, "button-release-event",G_CALLBACK (set_zero), &camzp);

      /* - */
      buts=gtk_button_new();
      tmp_str = my_concat(chemin,"/../share/bebetes_show/remote_control/dezoom.png");
      dingbat = gtk_image_new_from_file(tmp_str);
      free(tmp_str);
      gtk_container_add (GTK_CONTAINER (buts),dingbat);
      gtk_table_attach_defaults (GTK_TABLE (table), buts,3,4,1,2);
      g_signal_connect (buts, "button-press-event",G_CALLBACK (set_one),&camzm);
      g_signal_connect (buts, "button-release-event",G_CALLBACK (set_zero), &camzm);

      /* b+ */
      buts=gtk_button_new();
      tmp_str = my_concat(chemin,"/../share/bebetes_show/remote_control/bplus.png");
      dingbat = gtk_image_new_from_file(tmp_str);
      free(tmp_str);
      gtk_container_add (GTK_CONTAINER (buts),dingbat);
      gtk_table_attach_defaults (GTK_TABLE (table), buts,4,5,0,1);
      g_signal_connect (buts, "button-press-event",G_CALLBACK (set_one), &cambp);
      g_signal_connect (buts, "button-release-event",G_CALLBACK (set_zero), &cambp);

      /* b- */
      buts=gtk_button_new();
      tmp_str = my_concat(chemin,"/../share/bebetes_show/remote_control/bmoins.png");
      dingbat = gtk_image_new_from_file(tmp_str);
      free(tmp_str);
      gtk_container_add (GTK_CONTAINER (buts),dingbat);
      gtk_table_attach_defaults (GTK_TABLE (table), buts,4,5,1,2);
      g_signal_connect (buts, "button-press-event",G_CALLBACK (set_one), &cambm);
      g_signal_connect (buts, "button-release-event",G_CALLBACK (set_zero), &cambm);

      /* a+ */
      buts=gtk_button_new();
      tmp_str = my_concat(chemin,"/../share/bebetes_show/remote_control/aplus.png");
      dingbat = gtk_image_new_from_file(tmp_str);
      free(tmp_str);
      gtk_container_add (GTK_CONTAINER (buts),dingbat);
      gtk_table_attach_defaults (GTK_TABLE (table), buts,3,4,2,3);
      g_signal_connect (buts, "button-press-event",G_CALLBACK (set_one), &camap);
      g_signal_connect (buts, "button-release-event",G_CALLBACK (set_zero), &camap);

      /* a- */
      buts=gtk_button_new();
      tmp_str = my_concat(chemin,"/../share/bebetes_show/remote_control/amoins.png");
      dingbat = gtk_image_new_from_file(tmp_str);
      free(tmp_str);
      gtk_container_add (GTK_CONTAINER (buts),dingbat);
      gtk_table_attach_defaults (GTK_TABLE (table), buts,4,5,2,3);
      g_signal_connect (buts, "button-press-event",G_CALLBACK (set_one), &camam);
      g_signal_connect (buts, "button-release-event",G_CALLBACK (set_zero), &camam);


      gtk_widget_show_all (win);
      remote_control_opened = 1;
      return 1;
    }

  if (!remote_control_opened)
    {
      gtk_window_set_keep_above (GTK_WINDOW (win),1);
      gtk_widget_show_all (win);
      remote_control_opened = 1;
      return 1;
    }
  return 1;
}
/* alexandre... t'es un putain de connard */
/* alexandre... t'es un putain de connard */
/* alexandre... t'es un putain de connard */
/* alexandre... t'es un putain de connard */
