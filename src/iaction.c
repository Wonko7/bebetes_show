#include <stdio.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include "lib_bbs_behavior.h"
#include "iaction.h"

extern iaction *iactions;
extern int nb_iaction;
extern int selected_iaction;

int set_state(GtkWidget *w, int *i)
{
  if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(w)))
    selected_iaction = *i;
  return(1);
}

GtkWidget * create_iaction()
{
  GtkWidget *vbox, *sc, *rad;
  GtkRadioButton *group;
  int i; 

  /*
  gtk_radio_button_new_with_label (GSList *group,
				   const gchar *label);
  GtkWidget*  gtk_radio_button_new_with_label_from_widget
    radio2 = gtk_radio_button_new_with_label_from_widget (GTK_RADIO_BUTTON (radio1),
							  "I'm the second radio button.");
  */
  selected_iaction = 0;
  vbox = gtk_vbox_new(0,2);  

  group = GTK_RADIO_BUTTON (rad = gtk_radio_button_new_with_label (0,iactions[0].name_ia));
  gtk_box_pack_start(GTK_BOX (vbox), rad, 0, 0, 0);  
  g_signal_connect (G_OBJECT(rad), "toggled",G_CALLBACK (set_state), &(iactions[0].i));

  for (i = 1; i < nb_iaction; i++)
    {
      rad = gtk_radio_button_new_with_label_from_widget (group,iactions[i].name_ia);
      gtk_box_pack_start(GTK_BOX (vbox), rad, 0, 0,0);  
      g_signal_connect (G_OBJECT(rad), "toggled",G_CALLBACK (set_state), &(iactions[i].i));
    }

  sc = gtk_scrolled_window_new(NULL,NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sc),GTK_POLICY_AUTOMATIC,GTK_POLICY_AUTOMATIC );
  gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW (sc), vbox);

  return(sc);
}


GtkWidget * create_info()
{
  GtkWidget *vbox, *sc;
  GtkWidget *lab;
  

  vbox = gtk_vbox_new(0,2);  
  lab = gtk_label_new ("\n- Clique gauche : Selection d'une fourmi\n- Clique droit : Rotation de la camera\n- Clique du milieu : Zoom ou Deplacement sur la carte\n- F3 : Alterner le fonctionnement du clique du milieu \n- F4 : Alterner entre le mode fenetre et le mode plein ecran");
  gtk_container_add (GTK_CONTAINER (vbox), lab);


  sc = gtk_scrolled_window_new(NULL,NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sc),GTK_POLICY_AUTOMATIC,GTK_POLICY_AUTOMATIC );
  gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW (sc), vbox);

  return(sc);
}


/* alexandre... t'es un putain de connard */
/* alexandre... t'es un putain de connard */
/* alexandre... t'es un putain de connard */
/* alexandre... t'es un putain de connard */
