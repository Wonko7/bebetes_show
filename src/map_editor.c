#include <stdlib.h>
#include <gtk/gtk.h>
#include "map_editor.h"
#include "lib_bbs_behavior.h"
#include "loader.h"
#include "camera.h"
#include "map.h"
#include "gtk+extra/gtkplot.h"
#include "gtk+extra/gtkplotdata.h"
#include "gtk+extra/gtkplotbar.h"
#include "gtk+extra/gtkplotcanvas.h"
#include "gtk+extra/gtkplotprint.h"
#include "stat.h"
#include "divers.h"
#include "pango/pango.h"
#include "list_fav.h"

#define LARGEUR_CASE 30
#define HAUTEUR_CASE 30

extern t_map carte;
extern char* chemin;
extern GtkWindow *window;
extern int nb_ag, nb_case,nb_obj;
extern agent * agents;
extern ccase * cases;
extern objet * objets;

GtkWidget 
  *scroll,
  *im_map = 0,
  *radio_supp_ag,
  *radio_supp_obj,
  *x_spin,
  *y_spin,
  *align; 

GtkWidget  **radio_agents,
  **radio_case,
  **radio_objets;

GdkPixmap *pix_map,
  *pix_terrain,
  *pix_obstacle,
  *pix_fourmiliere,
  *pix_border,
  **buff_case,
  **buff_agents,
  **buff_objets,
  **buff_num;



GdkGC* Gc;

int last_clic_x =-1;
int last_clic_y =-1;

void create_new_map()
{
  int i,j;
  if (carte.x_max != 0)
    gtk_caca("Pour creer une nouvelle carte, il faut d'abord creer un nouvelle partie");
  else
    {
      carte= create_map(gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(x_spin)),gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(y_spin)));
      pix_map = gdk_pixmap_new (gdk_screen_get_root_window(window->screen),carte.x_max*LARGEUR_CASE+carte.x_max, carte.y_max*HAUTEUR_CASE+carte.y_max, -1 );
      for (i=0;i<carte.x_max;i++)
	{
	  for (j=0;j<carte.y_max;j++)
	    {
	      gdk_draw_pixmap ((GdkDrawable*)pix_map, Gc, (GdkDrawable*) pix_border,0,0,i*LARGEUR_CASE+i,j*HAUTEUR_CASE+j,-1,-1);
	      colorier(i,j);
	    }
	}
      if (im_map)
	gtk_widget_destroy(im_map);
      im_map=gtk_image_new_from_pixmap (pix_map,NULL);
      gtk_container_add (GTK_CONTAINER(align),im_map);
      gtk_widget_queue_draw(GTK_WIDGET(scroll));
      /*
      for (i=0;i<carte.x_max;i++)
	{
	  for (j=0;j<carte.y_max;j++)
	    {
	      colorier(i,j);
	    }
	}
      */
      gtk_widget_show_all(scroll);
      refresh_lfav();
    }
}

void delete_ag (int x, int y)
{
  int i;

  for(i=0;i<carte.tab[x][y].nbr_ag;i++)
    carte.tab[x][y].agents[i]->age *= (carte.tab[x][y].agents[i]->age > 0)?-1:1;

  free(carte.tab[x][y].agents);
  carte.tab[x][y].agents = NULL;
  carte.tab[x][y].nbr_ag = 0;
}

void delete_obj (int x, int y)
{
  int i;

  for(i=0;i<carte.tab[x][y].nbr_obj;i++)
    carte.tab[x][y].objets[i]->valeur *= (carte.tab[x][y].objets[i]->valeur > 0)?-1:1;

  free(carte.tab[x][y].objets);
  carte.tab[x][y].objets = NULL;
  carte.tab[x][y].nbr_obj = 0;
}

void transformer_case(int x, int y)
{
  int i;
  for (i=0; i< nb_case; i++)
    {
      if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(radio_case[i])))
	{
	  carte.tab[x][y].etat=i;
	  carte.tab[x][y].type=i;
	}
    }
  for (i=0; i< nb_ag; i++)
    {
      if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(radio_agents[i])))
	{
	  creer_agent(agents[i].nom_mod,x,y,0.0,(1 + (int) (360 * (rand() / (RAND_MAX + 1.0)))),0.0,agents[i].ratio,i,1);
	}
    }
  for (i=0; i< nb_obj; i++)
    {
      if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(radio_objets[i])))
	{
	  creer_objet(objets[i].nom_mod,x,y,0,0,0,objets[i].ratio,i,1);
	}
    }


  if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(radio_supp_ag)))
    delete_ag(x,y);

  if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(radio_supp_obj)))
    delete_obj(x,y);
  
  colorier(x,y);
  gtk_widget_queue_draw(GTK_WIDGET(scroll));
}

gboolean map_click (GtkWidget      *widget UNUSED,
		    GdkEventButton *event,
		    gpointer        data UNUSED)
{
  GtkAdjustment *horizontal, *vertical;
  int clic_x, clic_y,i,j,increment;

  horizontal=gtk_scrolled_window_get_hadjustment ((GtkScrolledWindow *)scroll);
  vertical  =gtk_scrolled_window_get_vadjustment ((GtkScrolledWindow *)scroll);
  clic_x=(int)((event->x + horizontal->value)/(LARGEUR_CASE+1));
  clic_y=(int)((event->y + vertical->value)/(HAUTEUR_CASE+1));

  if (clic_x >= carte.x_max || clic_y >= carte.y_max || clic_x < 0 || clic_y < 0)
    return(1);

  if (event->button == 3)
    {
      if (last_clic_x == -1)
	{
	  last_clic_x=clic_x;
	  last_clic_y=clic_y;
	}
      else
	{
	  if ((last_clic_x != clic_x) && (last_clic_y != clic_y))
	    {
	      for (i=((clic_x<last_clic_x)?clic_x:last_clic_x); i<=((clic_x>last_clic_x)?clic_x:last_clic_x);i++)
		{
		  for (j=((clic_y<last_clic_y)?clic_y:last_clic_y); j<=((clic_y>last_clic_y)?clic_y:last_clic_y);j++)
		    {
		      transformer_case(i,j);
		    }
		}
	      last_clic_x=-1;
	      last_clic_y=-1;
	      gtk_image_set_from_pixmap (GTK_IMAGE(im_map),pix_map,NULL);
	    }
	  else
	    {
	      if(last_clic_x==clic_x)
		{
		  increment=(last_clic_y <clic_y) ? 1:-1;
		  i=last_clic_y;
		  while(1)
		    {
		      transformer_case(clic_x,i);
		      if (i==clic_y) break;
		      i=i+increment;
		    }
		}
	      else
		{
		  increment=(last_clic_x <clic_x) ? 1:-1;
		  i=last_clic_x;
		  while(1)
		    {
		      transformer_case(i,clic_y);
		      if (i==clic_x) break;
		      i=i+increment;
		    }
		}
	      last_clic_x=-1;
	      last_clic_y=-1;
	      gtk_image_set_from_pixmap (GTK_IMAGE(im_map),pix_map,NULL);
	    }
	}

    }
  if (event->button == 1)
    {
      transformer_case(clic_x,clic_y);
      gtk_image_set_from_pixmap (GTK_IMAGE(im_map),pix_map,NULL);
    }
  return TRUE;
}


void colorier(int i,int j)
{
  gdk_draw_pixmap ((GdkDrawable*)pix_map, Gc, buff_case[carte.tab[i][j].type],0,0,i*LARGEUR_CASE+i,j*HAUTEUR_CASE+j,-1,-1);
  if (carte.tab[i][j].agents!=NULL)
    {
      gdk_draw_pixmap ((GdkDrawable*)pix_map, Gc, buff_agents[carte.tab[i][j].agents[0]->type],0,0,i*LARGEUR_CASE+i+2,j*HAUTEUR_CASE+j+2,-1,-1);
      if (carte.tab[i][j].nbr_ag <= 9)
	gdk_draw_pixmap ((GdkDrawable*)pix_map, Gc, buff_num[carte.tab[i][j].nbr_ag],0,0,i*LARGEUR_CASE+i+2,j*HAUTEUR_CASE+j+15,-1,-1);
      else
	gdk_draw_pixmap ((GdkDrawable*)pix_map, Gc, buff_num[10],0,0,i*LARGEUR_CASE+i+2,j*HAUTEUR_CASE+j+15,-1,-1);
    }
  if (carte.tab[i][j].objets!=NULL)
    {
      gdk_draw_pixmap ((GdkDrawable*)pix_map, Gc, buff_objets[carte.tab[i][j].objets[0]->type],0,0,i*LARGEUR_CASE+i+15,j*HAUTEUR_CASE+j+2,-1,-1);
      if (carte.tab[i][j].nbr_obj<=9)
	gdk_draw_pixmap ((GdkDrawable*)pix_map, Gc, buff_num[carte.tab[i][j].nbr_obj],0,0,i*LARGEUR_CASE+i+15,j*HAUTEUR_CASE+j+15,-1,-1);
      else
	gdk_draw_pixmap ((GdkDrawable*)pix_map, Gc, buff_num[10],0,0,i*LARGEUR_CASE+i+15,j*HAUTEUR_CASE+j+15,-1,-1);
    }
}

void free_editeur()
{
  free(radio_agents);
  free(buff_agents);
  free(radio_case);
  free(buff_case);
  free(radio_objets);
  free(buff_objets);
  free(buff_num);
}

GtkWidget * create_editeur()
{
  GtkWidget *h_box_principale,
    *v_box_droite,
    *h_box_boutons,
    *v_box_boutons1,
    *v_box_boutons2,
    *v_box_boutons3,
    *h_box_newmap,
    *map_event,
    *label,
    *button_newmap;

  GtkObject *x_adj, *y_adj;

  GSList *group = NULL;

  gchar *str_border;

  int i,j;
  gchar *nom_agent,*type_case, *nom_objet;
  radio_agents=malloc(nb_ag*sizeof(GtkWidget*));
  buff_agents=malloc(nb_ag*sizeof(GdkPixmap*));
  radio_case=malloc((nb_case*sizeof(GtkWidget*)));
  buff_case=malloc(nb_case*sizeof(GdkPixmap*));
  radio_objets=malloc((nb_obj*sizeof(GtkWidget*)));
  buff_objets=malloc(nb_obj*sizeof(GdkPixmap*));
  buff_num=malloc(11*sizeof(GdkPixmap*));

  scroll = gtk_scrolled_window_new(NULL,NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scroll),GTK_POLICY_AUTOMATIC,GTK_POLICY_AUTOMATIC );

  h_box_principale = gtk_hbox_new(0,2);
  gtk_container_set_border_width (GTK_CONTAINER (h_box_principale), 10);
  h_box_boutons  = gtk_hbox_new(0,20);
  v_box_boutons1 = gtk_vbox_new(0,2);
  v_box_boutons2 = gtk_vbox_new(0,2);
  v_box_boutons3 = gtk_vbox_new(0,2);
  v_box_droite   = gtk_vbox_new(0,2);

  /*INTERFACE CREATION D'UNE MAP*/
  label = gtk_label_new("Creation d'une nouvelle carte");
  gtk_box_pack_start(GTK_BOX (v_box_droite),label,FALSE,FALSE,0);

  h_box_newmap   = gtk_hbox_new(0,2);
  label = gtk_label_new("Largeur  : ");
  x_adj = gtk_adjustment_new (0, 1, 100000, 1,10, 0);
  x_spin=gtk_spin_button_new((GtkAdjustment *)x_adj,0,0);
  gtk_spin_button_set_wrap( GTK_SPIN_BUTTON (x_spin),0 );
  gtk_box_pack_start(GTK_BOX (h_box_newmap),label,TRUE,TRUE,0);
  gtk_box_pack_start(GTK_BOX (h_box_newmap),x_spin,TRUE,TRUE,0);
  gtk_box_pack_start(GTK_BOX (v_box_droite),h_box_newmap,FALSE,FALSE,0);


  h_box_newmap   = gtk_hbox_new(0,2);
  label = gtk_label_new("Hauteur : ");
  y_adj = gtk_adjustment_new (0, 1, 100000, 1,10, 0);
  y_spin=gtk_spin_button_new((GtkAdjustment *)y_adj,0,0);
  gtk_spin_button_set_wrap( GTK_SPIN_BUTTON (y_spin),0 );
  button_newmap=gtk_button_new_with_label ("Creer carte");
  g_signal_connect (G_OBJECT (button_newmap), "clicked", G_CALLBACK (create_new_map),NULL);
  gtk_box_pack_start(GTK_BOX (h_box_newmap),label,TRUE,TRUE,0);
  gtk_box_pack_start(GTK_BOX (h_box_newmap),y_spin,TRUE,TRUE,0);
  gtk_box_pack_start(GTK_BOX (v_box_droite),h_box_newmap,FALSE,FALSE,0);
  gtk_box_pack_start(GTK_BOX (v_box_droite),button_newmap,FALSE,FALSE,0);
  gtk_box_pack_start(GTK_BOX (v_box_droite),gtk_hseparator_new(),FALSE,FALSE,0);


  /*BOUTONS RADIO*/
  /*Types de case*/
  label = gtk_label_new("Type de case");
  gtk_box_pack_start(GTK_BOX (v_box_boutons1),label,FALSE,FALSE,0);
  group=NULL;
  for(i=0;i<nb_case;i++)
    {
      type_case=cases[i].name;
      radio_case[i] = gtk_radio_button_new_with_label (group,type_case );
      group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (radio_case[i]));
      gtk_box_pack_start(GTK_BOX (v_box_boutons1),radio_case[i],FALSE,FALSE,0);
      type_case=mega_cct(4,chemin,"/../share/bebetes_show/editeur/",type_case,".xpm");
      buff_case[i]= gdk_pixmap_create_from_xpm (gdk_screen_get_root_window(window->screen),NULL,NULL,type_case);
      free(type_case);
    }
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (radio_case[0]), TRUE);

  /*Types d'agent*/
  label = gtk_label_new("Type d'agent");
  gtk_box_pack_start(GTK_BOX (v_box_boutons2),label,FALSE,FALSE,0);

   for(i=0;i<nb_ag;i++)
    {
      nom_agent=agents[i].name_ag;
      radio_agents[i] = gtk_radio_button_new_with_label (group,nom_agent );
      group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (radio_agents[i]));
      gtk_box_pack_start(GTK_BOX (v_box_boutons2),radio_agents[i],FALSE,FALSE,0);
      nom_agent=mega_cct(4,chemin,"/../share/bebetes_show/editeur/",nom_agent,".xpm");
      buff_agents[i]= gdk_pixmap_create_from_xpm (gdk_screen_get_root_window(window->screen),NULL,NULL,nom_agent);
      free(nom_agent);
    }
   
 /*Types d'objets*/
   
  label = gtk_label_new("Type d'objet");
  gtk_box_pack_start(GTK_BOX (v_box_boutons3),label,FALSE,FALSE,0);

   for(i=0;i<nb_obj;i++)
    {
      nom_objet=objets[i].name_obj;
      radio_objets[i] = gtk_radio_button_new_with_label (group,nom_objet );
      group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (radio_objets[i]));
      gtk_box_pack_start(GTK_BOX (v_box_boutons3),radio_objets[i],FALSE,FALSE,0);
      nom_objet=mega_cct(4,chemin,"/../share/bebetes_show/editeur/",nom_objet,".xpm");
      buff_objets[i]= gdk_pixmap_create_from_xpm (gdk_screen_get_root_window(window->screen),NULL,NULL,nom_objet);
      free(nom_objet);
    }
   

   radio_supp_ag=gtk_radio_button_new_with_label (group,"Supprimer agents");  
   group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (radio_supp_ag));
   gtk_box_pack_start(GTK_BOX (v_box_boutons2),radio_supp_ag,FALSE,FALSE,0);

   radio_supp_obj=gtk_radio_button_new_with_label (group,"Supprimer objets");
   gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (radio_supp_obj), TRUE);
   gtk_box_pack_start(GTK_BOX (v_box_boutons3),radio_supp_obj,FALSE,FALSE,0);

  /*INITIALISATION DES BUFFERS CHIFFRES POUR COPIER*/
   for (i=0;i<=9;i++)
     {
       nom_objet=my_concat(my_concat(chemin,"/../share/bebetes_show/editeur/"),my_concat(itos(i),".xpm"));
       buff_num[i]= gdk_pixmap_create_from_xpm (gdk_screen_get_root_window(window->screen),NULL,NULL,nom_objet);
     }
   nom_objet=my_concat(my_concat(chemin,"/../share/bebetes_show/editeur/"),"+.xpm");
   buff_num[10]= gdk_pixmap_create_from_xpm (gdk_screen_get_root_window(window->screen),NULL,NULL,nom_objet);
   
  if (carte.x_max != 0)
    pix_map        = gdk_pixmap_new (gdk_screen_get_root_window(window->screen),carte.x_max*LARGEUR_CASE+carte.x_max, carte.y_max*HAUTEUR_CASE+carte.y_max, -1 );

  Gc=gdk_gc_new  (gdk_screen_get_root_window(window->screen));

  str_border=      my_concat(chemin,"/../share/bebetes_show/editeur/border.xpm");
  pix_border =     gdk_pixmap_create_from_xpm (gdk_screen_get_root_window(window->screen),NULL,NULL,str_border);

  /*PARCOURS DE LA MAP ET CREATION DU BUFFER PRINCIPAL*/
  for (i=0;i<carte.x_max;i++)
    {
      for (j=0;j<carte.y_max;j++)
	{
	  gdk_draw_pixmap ((GdkDrawable*)pix_map, Gc, (GdkDrawable*) pix_border,0,0,i*LARGEUR_CASE+i,j*HAUTEUR_CASE+j,-1,-1);
	  colorier(i,j);
	}
    }

  /*AFFICHAGE DU BUFFER PRINCIPAL*/
  im_map=gtk_image_new_from_pixmap (pix_map,NULL);
  map_event=gtk_event_box_new();
  g_signal_connect (G_OBJECT (map_event), "button_press_event", G_CALLBACK (map_click),NULL);

  align = GTK_WIDGET(gtk_alignment_new(0,0,0,0));

  gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW (scroll), align);
  gtk_container_add (GTK_CONTAINER(align),im_map);

  gtk_scrolled_window_set_placement  (GTK_SCROLLED_WINDOW (scroll),GTK_CORNER_TOP_LEFT);
  gtk_container_add (GTK_CONTAINER (map_event), scroll);
  gtk_box_pack_start(GTK_BOX (h_box_principale),map_event,TRUE,TRUE,0);
  gtk_box_pack_start(GTK_BOX (h_box_boutons),v_box_boutons1,FALSE,FALSE,0);
  gtk_box_pack_start(GTK_BOX (h_box_boutons),v_box_boutons2,FALSE,FALSE,0);
  gtk_box_pack_start(GTK_BOX (h_box_boutons),v_box_boutons3,FALSE,FALSE,0);
  gtk_box_pack_start(GTK_BOX (v_box_droite),h_box_boutons,FALSE,FALSE,0);
  gtk_box_pack_start(GTK_BOX (h_box_principale),v_box_droite,FALSE,FALSE,0);
  return(h_box_principale);
}

void refresh_map()
{
  if (!carte.x_max)
    {
      gtk_widget_destroy(im_map);
      im_map = 0;
    }
  gtk_widget_queue_draw(GTK_WIDGET(scroll));
}

void redraw_me (GtkNotebook *nb UNUSED, GtkNotebookPage *page UNUSED, int p, gpointer data UNUSED)
{
  int i,j;

   maj_stat(p);

  if (p == 4 && carte.x_max != 0)
    {
      
      for (i=0;i<carte.x_max;i++)
	{
	  for (j=0;j<carte.y_max;j++)
	    {
	      gdk_draw_pixmap ((GdkDrawable*)pix_map, Gc, (GdkDrawable*) pix_border,0,0,i*LARGEUR_CASE+i,j*HAUTEUR_CASE+j,-1,-1);
	      colorier(i,j);
	    }
	}
      
    }
}
