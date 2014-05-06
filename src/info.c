#include <stdio.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include "loader.h"
#include "camera.h"
#include "lib_bbs_behavior.h"
#include "divers.h"
#include "map.h"

extern agent *agents;
extern behavior *stimuli;
extern objet *objets;
extern ccase *cases;
extern int nb_sti, nb_ag;
GtkWidget *fb = 0, *info = 0;

/*
#define seph(a) tmp = gtk_vbox_new(0,0); gtk_container_add (GTK_CONTAINER (tmp), gtk_hseparator_new());  gtk_box_pack_start (GTK_BOX (a), tmp,0,0,0)

#define sepv(a) tmp = gtk_vbox_new(0,0); gtk_container_add (GTK_CONTAINER (tmp), gtk_vseparator_new());  gtk_box_pack_start (GTK_BOX (a), tmp,0,0,0)
*/

#define seph(a) gtk_box_pack_start(GTK_BOX (a), gtk_hseparator_new(), 0,0,0)
#define sepv(a) gtk_box_pack_start(GTK_BOX (a), gtk_vseparator_new(), 0,0,0)


GtkWidget *draw_info(picked *c)
{
  GtkWidget *vbox,*hbox,*c0,*c1,*c2,*c3,*sc,*lab;
  char *t,*t1,*t2;
  int i;

  vbox = gtk_vbox_new(0,5);

  if (c->type == IA_AGENT)
    {
      if (((t_agent) c->clicked)->type >= nb_ag || ((t_agent) c->clicked)->type < 0)
	{
	  printf("/!\\ merdoiment fatal : type d'agent érroné :\n %i", ((t_agent) c->clicked)->type);
	  fflush(stdout);
	  ((t_agent) c->clicked)->type = 0;
	}
      t1 = mega_cct(2,"Agent : ",agents[((t_agent) c->clicked)->type].name_ag);
      lab = gtk_label_new(t1);
      free(t1);
      gtk_box_pack_start (GTK_BOX (vbox), lab,0,0,0);
      seph(vbox);

      t = itos(((t_agent) c->clicked)->age);
      t1 = mega_cct(2,"Age : ",t);
      lab = gtk_label_new(t1);
      free(t);
      free(t1);
      gtk_box_pack_start (GTK_BOX (vbox), lab, 0,0,0);
      seph(vbox);

      hbox = gtk_hbox_new(0,2);
      c0 = gtk_vbox_new(0,2);  
      c1 = gtk_vbox_new(0,2);  
      c2 = gtk_vbox_new(0,2);  
      c3 = gtk_vbox_new(0,2);  
      gtk_box_pack_start (GTK_BOX (vbox), hbox, 0,0,0);
      gtk_container_add (GTK_CONTAINER (hbox), c0);
      sepv(hbox);
      gtk_container_add (GTK_CONTAINER (hbox), c1);
      sepv(hbox);
      gtk_container_add (GTK_CONTAINER (hbox), c2);
      sepv(hbox);
      gtk_container_add (GTK_CONTAINER (hbox), c3);
      gtk_container_add (GTK_CONTAINER (c0), gtk_label_new("Stimuli"));
      seph(c0);
      gtk_container_add (GTK_CONTAINER (c1), gtk_label_new("Poids"));
      seph(c1);
      gtk_container_add (GTK_CONTAINER (c2), gtk_label_new("Seuil"));
      seph(c2);
      gtk_container_add (GTK_CONTAINER (c3), gtk_label_new("Force"));
      seph(c3);

      for(i=0;i<nb_sti;i++)
	{
	  if (((t_agent) c->clicked)->type == stimuli[i].ag)
	    {
	      t  = ftos(((t_agent) c->clicked)->stimuli[i].poids);
	      t1 = ftos(((t_agent) c->clicked)->stimuli[i].seuil);
	      t2 = ftos(((t_agent) c->clicked)->stimuli[i].force);
	      gtk_container_add (GTK_CONTAINER (c0), gtk_label_new(stimuli[i].name_s));
	      gtk_container_add (GTK_CONTAINER (c1), gtk_label_new(t));
	      gtk_container_add (GTK_CONTAINER (c2), gtk_label_new(t1));
	      gtk_container_add (GTK_CONTAINER (c3), gtk_label_new(t2));
	      free(t);
	      free(t1);
	      free(t2);
	    }
	}
    }
  else
    if (c->type == IA_OBJ)
      {
	t1 = mega_cct(2,"Objet : ",objets[((t_objet) c->clicked)->type].name_obj);
	lab = gtk_label_new(t1);
	free(t1);
	gtk_box_pack_start (GTK_BOX (vbox), lab, 0,0,0);

	seph(vbox);

	t = ftos(((t_objet) c->clicked)->valeur);
	t1 = mega_cct(2,"Valeur : ",t);
	lab = gtk_label_new(t1);
	free(t);
	free(t1);
	gtk_box_pack_start (GTK_BOX (vbox), lab, 0,0,0);
      }
    else
      if (c->type == IA_CASE)
	{
	  t1 = mega_cct(2,"Case : ",cases[((t_case *) c->clicked)->type].name);
	  lab = gtk_label_new(t1);
	  free(t1);
	  gtk_box_pack_start (GTK_BOX (vbox), lab, 0,0,0);
	}


  sc = gtk_scrolled_window_new(NULL,NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sc),GTK_POLICY_AUTOMATIC,GTK_POLICY_AUTOMATIC );
  gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW (sc), vbox);
  return(sc);
}

GtkWidget *redraw_info(picked *c)
{
  if (fb == 0)
    fb = gtk_vbox_new(0,0);
  if (info != 0)
    gtk_widget_destroy(info);
  info = draw_info(c);
  gtk_container_add (GTK_CONTAINER (fb),info);
  gtk_widget_show_all(fb);
  return(fb);
}

GtkWidget *ccreate_info(picked *c)
{
  fb = gtk_vbox_new(0,0);
  info = draw_info(c);
  gtk_container_add (GTK_CONTAINER (fb),info);
  gtk_widget_show_all(fb);
  return(fb);
}
/* alexandre... t'es un putain de connard */
/* alexandre... t'es un putain de connard */
/* alexandre... t'es un putain de connard */
/* alexandre... t'es un putain de connard */
