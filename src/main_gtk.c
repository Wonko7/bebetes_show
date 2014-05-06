#include<gtk/gtk.h>
#include<stdlib.h>

/** gl *****************************************************/

#include <gdk/gdkkeysyms.h>
#include <gtk/gtkgl.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <math.h>
#include <unistd.h> 
#include <string.h>

#define  _GNU_SOURCE
#include <dlfcn.h>
#include "lib_bbs_behavior.h"
#include "f.h"
#include "divers.h"

#ifndef _LOADER_H
#include "loader.h"
#endif
#include "camera.h"
#include "map.h"
#include "map_editor.h"
#include "textures.h"
#include "taskbrowser.h"
#include "remote_control.h"
#include "list_fav.h"
#include "options.h"
#include "loading.h"
#include "iaction.h"
#include "propag.h"
#include "info.h"
#include "gtk+extra/gtkplot.h"
#include "gtk+extra/gtkplotdata.h"
#include "gtk+extra/gtkplotbar.h"
#include "gtk+extra/gtkplotcanvas.h"
#include "gtk+extra/gtkplotprint.h"
#include "stat.h"
/* inutile ? */
/*int presse = 0;*/

/**** Globals **************************/

/* 3 spinners ctrl cam */
GtkWidget *window;
GtkWidget *drawing_area;
GtkWidget *fs_t1,*fs_t2;
GtkWidget *note_fucking_book=0;
GtkWidget *fakebox_lf;

/* tb */
GtkWidget *onglet_taskbrowser;

/* bool redessiner 
   int ramakicaca=0;*/

/* gestion souris */
int beginX, beginY;

/* donnees cam */
struct camera cam;

extern t_texture tex_index;
t_map carte;

extern t_agent liste_agent;
extern t_objet liste_objet;
/* bool annimation */
extern int move;

/* faire avancer l'herbe */
float herbe=0.0f;

/* chemin de l'exe */
extern char *chemin;

/* on affiche le billboarding? */
int show_billboard = 0;

int vitesse_simulation = 1;
extern int afficher_stimuli;
extern int stimulus_selectionne;
extern int afficher_ogl;

int connard;
int abdoul_special_dedicace = 0;

void init_connard ()
{
  /*
  char * s = getlogin ();
  connard = (!strcmp(s,"underscore"));
  connard = connard || (!strcmp(s,"Underscore"));
  connard = connard || (!strcmp(s,"root"));
  */
}
    

/****** lib dyn ****************************/
void * lib=0;

behavior *stimuli=0;
agent *agents=0;
objet *objets=0;
operator *operators=0;
primitive *primitives=0;
argument *arguments=0;
ccase *cases=0;
iaction *iactions=0;
graphe * stats = 0;
int selected_iaction = 0;

int nb_sti, nb_ag, nb_obj, nb_prim, nb_op, nb_arg, nb_case, nb_iaction, nb_stats;
float cycle = 7.5;

void *(*free_op)();
void *(*free_p)();
void *(*free_ag)();
void *(*free_objs)();
void *(*free_sti)();
void *(*free_arg)();
void *(*free_case)();
void *(*free_iaction)();

void *(*remove_task)();

extern void *(*deprup)();
extern int nb_stats;

/*******************************************/


/*
**  Lorsque cette variable est a 1 on a acces
**  aux choses qu'on interdit a l'utilisateur.
**  faudra l'utiliser pour le droit a la 
**  desactivation de la lumiere etc. 
**  pour l'instant elle sert juste pour la cam.
**  on change mode (debug, ut) avec F12.
*/
int mode_debug=0;

void set_stimuli(GtkWidget *combo_stimuli)
{
  stimulus_selectionne=(gtk_combo_box_get_active(GTK_COMBO_BOX(combo_stimuli)));
}

void set_vitesse(GtkWidget *combo_vitesse)
{
  int sel;

  sel = gtk_combo_box_get_active(GTK_COMBO_BOX(combo_vitesse));
  
  if  (!sel)
    vitesse_simulation = 0;
  else
    if  (sel == 1)
      vitesse_simulation = 1;
    else
      if  (sel == 2)
	vitesse_simulation = 2;
      else
	if  (sel == 3)
	  vitesse_simulation = 4;
	else
	  if  (sel == 4)
	    vitesse_simulation = 8;
	  else
	    vitesse_simulation = -1;
}

void aff_ogl_callback (GtkWidget *widget)
{
  if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget))) 
    {
      afficher_ogl = 1;
    }
  else
    {
      afficher_ogl = 0;
    }
}

void aff_stimuli_callback (GtkWidget *widget)
{
  if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget))) 
    {
      afficher_stimuli = 1;
    }
  else
    {
      afficher_stimuli = 0;
    }
}


void bill_board_callback (GtkWidget *widget, gpointer actif)
{
  if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget))) 
    {
      * (int *) actif = 1;
    }
  else
    {
      * (int *) actif = 0;
    }
}


extern t_texture tex_index;

gint delete_event( GtkWidget *widget UNUSED,
                   GdkEvent  *event UNUSED,
		   gpointer   data UNUSED )
{
  gtk_main_quit ();
  return FALSE;
}

void destroy( GtkWidget *widget UNUSED,
              gpointer   data UNUSED)
{
  static int fin = 5;
  
  if (connard)
    fin--;
  else
    fin -=5;

  if (!fin)
    {
      gtk_widget_destroy(widget);
      gtk_main_quit ();
    }
}

/*
void set_current_texture(GtkWidget * w, GtkFileSelection *choix_texture)
{
  glDeleteTextures(1,&(*tex_index).id);
  (*tex_index).id = Load_Texture(gtk_file_selection_get_filename(GTK_FILE_SELECTION(choix_texture)));
  gtk_widget_destroy(GTK_WIDGET (choix_texture));
};


void menu_texture(GtkWidget *frame1)
{
  GtkWidget *choix_texture;
  char *c;
  choix_texture=gtk_file_selection_new("Texture a charger");
  g_signal_connect(G_OBJECT(GTK_FILE_SELECTION(choix_texture)->ok_button),
                   "clicked",G_CALLBACK(set_current_texture),(gpointer)choix_texture);
  g_signal_connect_swapped(G_OBJECT(GTK_FILE_SELECTION(choix_texture)->cancel_button),
			   "clicked",G_CALLBACK(gtk_widget_destroy),(gpointer)choix_texture);
  c=my_concat(chemin,"/../share/bebetes_show/textures/");
  gtk_file_selection_set_filename(GTK_FILE_SELECTION (choix_texture),c);
  free(c);
  gtk_widget_show(choix_texture);
};
*/

void init_gtk_nfs ()
{
  int i;

  GtkWidget *notebook;
  GtkWidget *table,*table2;
  GtkWidget *frame1,*frame2;
  GtkWidget *label1, *label2, *label3, *label4, *label5;
  GtkWidget *button,*but_remote_control;
  GtkWidget *box;
  GtkWidget *lfav;
  GtkWidget *principale,*statistoque;
  GtkWidget *editeur;
  GtkWidget *button_box;
  GtkWidget *bill_board;
  GtkWidget *options;
  GtkWidget *combo_vitesse;
  GtkWidget *combo_stimuli;
  GtkWidget *aff_stimuli;
  GtkWidget *aff_ogl;
  GtkWidget *h_box_options;
  GtkWidget *hb_ia;
  picked c;

  /*pour infos agent*/
  c.type = 5;


  /* gl */
  GdkGLConfig *glconfig;
  /* ** */  


  /*fakebox_nb = gtk_vbox_new(0,0);*/
  fakebox_lf = gtk_vbox_new(0,0);

  /* onglet tb */

  gtk_container_set_border_width (GTK_CONTAINER (window), 10);

  table = gtk_vbox_new(0,2);
  gtk_container_add (GTK_CONTAINER (window), table);
  fs_t1 = table;

  table2 = gtk_table_new (2, 2, 1);
  gtk_container_set_border_width (GTK_CONTAINER (table2), 15);
  
  note_fucking_book = notebook = gtk_notebook_new ();
  gtk_notebook_set_tab_pos (GTK_NOTEBOOK (notebook), GTK_POS_TOP);
  gtk_container_add (GTK_CONTAINER (table),notebook);
  gtk_box_set_child_packing(GTK_BOX (table),notebook,1,1,2,GTK_PACK_START);
  g_signal_connect_after (G_OBJECT (notebook), "switch-page", G_CALLBACK (redraw_me), NULL);
  

  frame1 = gtk_frame_new ("frame 1");
  gtk_container_set_border_width (GTK_CONTAINER (frame1), 5);

  frame2 = gtk_frame_new ("frame 2");
  gtk_container_set_border_width (GTK_CONTAINER (frame2), 5);

  /* TEMPS */
  /*g_timeout_add((guint32) RAFR,(GtkFunction) majgtktime,(gpointer) NULL);*/


  /*
      a rajouter : 
      -onglet principale (ouvrir, sauvegarder, nvx +logo) (juju)
      -onglet tb onglet_taskbrowser (w)
      -onglet aide (qui veut...)
   */


  label1 = gtk_label_new ("Sauvegardes");
  principale = create_loading();
  gtk_notebook_append_page(GTK_NOTEBOOK(notebook),principale,label1);

  /** Simu ****************************************************/
  label1 = gtk_label_new ("Simulation");
  /************************************************************/

  /** Options *************************************************/
  label2 = gtk_label_new ("Options");
  options = create_options();
  /************************************************************/

  /** TaskBrowser *********************************************/
  label3 = gtk_label_new ("Comportement");
  create_tb();
  /************************************************************/

  /** Editeur de carte ****************************************/
  label4 = gtk_label_new ("Editeur de carte");
  editeur = create_editeur();
  /************************************************************/

  /** Statistiques ********************************************/
  label5 = gtk_label_new ("Statistiques");
  statistoque = statify();
  /************************************************************/

  if (abdoul_special_dedicace)
    {
      gtk_notebook_append_page(GTK_NOTEBOOK(notebook),table2,label1);
      gtk_notebook_append_page(GTK_NOTEBOOK(notebook),options,label2);
      gtk_notebook_append_page(GTK_NOTEBOOK(notebook),onglet_taskbrowser,label3);
      gtk_notebook_append_page(GTK_NOTEBOOK(notebook),editeur,label4);
      if (nb_stats) 
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook),statistoque,label5);
    }
  else
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook),options,label2);

  abdoul_special_dedicace = 1;
  /************************************************************/



  /** gl *****************************************************/
  glconfig = gdk_gl_config_new_by_mode (GDK_GL_MODE_RGB    |
                                        GDK_GL_MODE_DEPTH  |
                                        GDK_GL_MODE_DOUBLE);
  if (glconfig == NULL)
    exit (1);

  /* Drawing area for drawing OpenGL scene. */
  drawing_area = gtk_drawing_area_new ();
  gtk_container_set_focus_child(GTK_CONTAINER (table2),drawing_area);



  /* Set OpenGL-capability to the widget. */
  gtk_widget_set_gl_capability (drawing_area,glconfig,NULL,TRUE,GDK_GL_RGBA_TYPE);

  /*init*/
  g_signal_connect_after (G_OBJECT (drawing_area), "realize",G_CALLBACK (realize), NULL);
  /*reshape*/
  g_signal_connect (G_OBJECT (drawing_area), "configure_event",G_CALLBACK (configure_event), NULL);
  /*draw*/
  g_signal_connect (G_OBJECT (drawing_area), "expose_event",G_CALLBACK (expose_event), NULL);



  /*sig sourie kb*/

  gtk_widget_add_events (drawing_area,
			 GDK_BUTTON1_MOTION_MASK    |
			 GDK_BUTTON2_MOTION_MASK    |
			 GDK_BUTTON3_MOTION_MASK    |
			 GDK_BUTTON_PRESS_MASK      |
			 GDK_VISIBILITY_NOTIFY_MASK);
  g_signal_connect (G_OBJECT (drawing_area), "motion_notify_event",G_CALLBACK (motion_notify_event), NULL);
  g_signal_connect (G_OBJECT (drawing_area), "button_press_event", G_CALLBACK (button_press_event), NULL);
  g_signal_connect_swapped (G_OBJECT (window), "key_press_event", G_CALLBACK (key_press_event), drawing_area);


  /** ** *****************************************************/

  fs_t2 = gtk_vbox_new(0,0);
  gtk_container_add (GTK_CONTAINER (fs_t2), drawing_area); 
  gtk_table_attach_defaults (GTK_TABLE (table2), fs_t2, 1, 2, 0, 2);


  box = gtk_vbox_new(0,2);
  gtk_table_attach_defaults (GTK_TABLE (table2), box, 0, 1, 0, 1);

  lfav = create_lfav();
  gtk_container_add (GTK_CONTAINER (fakebox_lf),lfav);
  gtk_box_pack_start(GTK_BOX (box),fakebox_lf,1,1,0);

  but_remote_control=gtk_button_new_with_label("Telecommande");
  gtk_box_pack_start(GTK_BOX (box),but_remote_control,FALSE,FALSE,0);
  g_signal_connect (but_remote_control, "clicked",G_CALLBACK (create_remote_control), NULL);

  h_box_options=gtk_hbox_new(1,2);
  gtk_box_pack_start(GTK_BOX (box),h_box_options,FALSE,FALSE,0);  

  aff_ogl = gtk_check_button_new_with_label("Afficher la simulation");
  gtk_box_pack_start(GTK_BOX (h_box_options),aff_ogl,TRUE,TRUE,0);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON (aff_ogl),afficher_ogl);
  g_signal_connect (G_OBJECT(aff_ogl), "toggled",G_CALLBACK (aff_ogl_callback), NULL);

  combo_vitesse=gtk_combo_box_new_text ();
  gtk_box_pack_start(GTK_BOX (h_box_options),combo_vitesse,TRUE,FALSE,0);
  gtk_combo_box_append_text(GTK_COMBO_BOX(combo_vitesse),"STOP");
  gtk_combo_box_append_text(GTK_COMBO_BOX(combo_vitesse),"1x");
  gtk_combo_box_append_text(GTK_COMBO_BOX(combo_vitesse),"2x");
  gtk_combo_box_append_text(GTK_COMBO_BOX(combo_vitesse),"4x");
  gtk_combo_box_append_text(GTK_COMBO_BOX(combo_vitesse),"8x");
  gtk_combo_box_append_text(GTK_COMBO_BOX(combo_vitesse),"max");
  gtk_combo_box_set_active (GTK_COMBO_BOX(combo_vitesse),1);
  g_signal_connect (combo_vitesse, "changed",G_CALLBACK (set_vitesse), NULL);

  
  h_box_options=gtk_hbox_new(0,2);
  gtk_box_pack_start(GTK_BOX (box),h_box_options,FALSE,FALSE,0);

  aff_stimuli = gtk_check_button_new_with_label("Afficher le stimulus selectionne");
  gtk_box_pack_start(GTK_BOX (h_box_options),aff_stimuli,FALSE,FALSE,0);
  g_signal_connect (G_OBJECT(aff_stimuli), "toggled",G_CALLBACK (aff_stimuli_callback), NULL);

  combo_stimuli=gtk_combo_box_new_text ();
  gtk_box_pack_start(GTK_BOX (h_box_options),combo_stimuli,TRUE,FALSE,0);
  for (i=0;i<nb_sti;i++)
    gtk_combo_box_append_text(GTK_COMBO_BOX(combo_stimuli),stimuli[i].name_s);
  g_signal_connect (combo_stimuli, "changed",G_CALLBACK (set_stimuli), NULL);


  h_box_options=gtk_hbox_new(0,2);
  gtk_box_pack_start(GTK_BOX (box),h_box_options,FALSE,FALSE,0);

  bill_board = gtk_check_button_new_with_label("Montrer les taches en cours");
  gtk_box_pack_start(GTK_BOX (h_box_options),bill_board,FALSE,FALSE,0);
  g_signal_connect (G_OBJECT(bill_board), "toggled",G_CALLBACK (bill_board_callback), (gpointer) &show_billboard);

  /*info + iactions*/
  hb_ia = gtk_hbox_new(0,5);
  gtk_table_attach_defaults (GTK_TABLE (table2), hb_ia, 0, 1, 1, 2);
  gtk_container_add (GTK_CONTAINER (hb_ia), ccreate_info(&c));
  gtk_container_add (GTK_CONTAINER (hb_ia), create_iaction());

  /*boutton quitter*/
  button_box=gtk_hbutton_box_new();
  gtk_container_add (GTK_CONTAINER (table), button_box);
  gtk_box_set_child_packing(GTK_BOX (table),button_box,0,0,2,GTK_PACK_START);
  button=gtk_button_new_with_label("Quitter");
  g_signal_connect (button, "clicked",G_CALLBACK (destroy), NULL);
  gtk_container_add (GTK_CONTAINER (button_box), button);

  /** Aide : a ajouter en dernier ****************************/
  /*
  label1 = gtk_label_new ("Aide");
  create_tb();  
  gtk_notebook_append_page(GTK_NOTEBOOK(notebook),lab,label1);
  */
  /** *************************** ****************************/

  gtk_widget_show_all (window);
}




int main(int argc, char *argv[])
{
  char * t1;

  init_connard ();

  /* chemin */
  trouver_chemin(argc, argv);

  /* save home ?*/
  create_save_dir();

  /** gl *****************************************************/
  gtk_init (&argc, &argv);
  gtk_gl_init (&argc, &argv);

  /** ** *****************************************************/

  window = gtk_window_new(GTK_WINDOW_TOPLEVEL); 
  gtk_window_set_title (GTK_WINDOW (window), "bbs");
  gtk_window_set_default_size(GTK_WINDOW (window),1000,600);
  g_signal_connect (G_OBJECT (window), "delete_event",
		    G_CALLBACK (delete_event), NULL);
  g_signal_connect (G_OBJECT (window), "destroy",
		    G_CALLBACK (destroy), window);


  /* init_things */
  t1 = my_concat(chemin,"/../lib/lib_bbs_behavior.so");
  init_colors();

  load_dlib(t1);
  free(t1);
  carte = create_map(0,0);
  init_cam();
  init_tab_stim_int();
  reload_obj3D();
  /* main truc */
  init_gtk_nfs ();
  gtk_main();

  write_options();

  free_tab_stim_int();
  free_colors ();
  free_stats ();
  free(chemin);
  free_textures();
  free_agents();
  free_objets();
  free_list_mod();
  free_map();
  free_libs();

  return(0);
}
