#include <gtk/gtk.h>
#include "lib_bbs_behavior.h"
#include "loader.h"
#include "camera.h"
#include "divers.h"
#include "f.h"
#include "map.h"

extern struct camera cam;
extern t_map carte;
extern GtkWidget *fakebox_lf;

GtkWidget *favlist, *lfav = 0, *nom_lf = 0;
GtkListStore *store;
GtkTreeModel *gmodel;
GtkTreePath *gpath;
GtkTreeIter giter;

int i=0;

enum
  {                /***********************************/
    COL_NAME = 0,  /* nom du champ                    */
    COL_ID,        /* id du champ... p sur fonction ? */
    COL_IS_DEL,    /* droit de suppr l'element ?      */
    COL_X,         /* x                               */
    COL_Y,         /* y                               */
    NUM_COLS       /* nombre de colones               */
  };               /***********************************/



int id_fav = -1;


/**************************************************** 
 **  GESTION DES LISTES : INIT+EVENEMENTS+VIEW  *****
 ****************************************************/



/********************************************** 
 **  LISTE FAV  ******************************* 
 **********************************************/



static GtkTreeModel * create_and_fill_fav ()
{
  GtkTreeIter    iter;


  store = gtk_list_store_new (NUM_COLS, G_TYPE_STRING, G_TYPE_UINT, G_TYPE_UINT, G_TYPE_FLOAT,G_TYPE_FLOAT);
  

  /* load fichier instead */
  gtk_list_store_append (store, &iter);
  gtk_list_store_set (store, &iter,
		      COL_NAME,"Centre",
		      COL_ID,i++,
		      COL_IS_DEL,0,
		      COL_X,(float) (carte.x_max/2*TAILLE_CASE),
		      COL_Y,(float) (carte.y_max/2*TAILLE_CASE),
		      -1);
  gtk_list_store_append (store, &iter);
  gtk_list_store_set (store, &iter,
		      COL_NAME,"Bas droite",
		      COL_ID,i++,
		      COL_IS_DEL,1,
		      COL_X,(float) (carte.x_max*TAILLE_CASE),
		      COL_Y,(float) (carte.y_max*TAILLE_CASE),
		      -1);
  gtk_list_store_append (store, &iter);
  gtk_list_store_set (store, &iter,
		      COL_NAME,"Bas gauche",
		      COL_ID,i++,
		      COL_IS_DEL,1,
		      COL_X,0.0,
		      COL_Y,(float) (carte.y_max*TAILLE_CASE),
		      -1);
  gtk_list_store_append (store, &iter);
  gtk_list_store_set (store, &iter,
		      COL_NAME,"Haut droite",
		      COL_ID,i++,
		      COL_IS_DEL,1,
		      COL_X,(float) (carte.x_max*TAILLE_CASE),
		      COL_Y,0.0,
		      -1);
  gtk_list_store_append (store, &iter);
  gtk_list_store_set (store, &iter,
		      COL_NAME,"Haut gauche",
		      COL_ID,i++,
		      COL_IS_DEL,1,
		      COL_X,0.0,
		      COL_Y,0.0,
		      -1);

  
  
  return GTK_TREE_MODEL (store);
}

gboolean view_sel_fav (GtkTreeSelection *selection UNUSED,
		       GtkTreeModel     *model,
		       GtkTreePath      *path,
		       gboolean          path_currently_selected,
		       gpointer          userdata UNUSED)
{
  GtkTreeIter iter;
  
  if (gtk_tree_model_get_iter(model, &iter, path))
    {
      gchar *name;
      gint id;
      gfloat x,y;
      gtk_tree_model_get(model, &iter, COL_NAME, &name, -1);
      gtk_tree_model_get(model, &iter, COL_ID, &id, -1);
      gtk_tree_model_get(model, &iter, COL_X, &x, -1);
      gtk_tree_model_get(model, &iter, COL_Y, &y, -1);
      if (!path_currently_selected && id != id_fav)
	{
	  cam.follow_agent = 0;
	  cam.reached = 0;
	  cam.xgoto = x;
	  cam.ygoto = y;
	  id_fav = id;
	  gmodel = model;
	  gpath = path;
	  giter = iter;
	}
      g_free(name);
    }
  return(1);
}

static GtkWidget * create_view_and_model_fav (void)
{
  GtkCellRenderer     *renderer;
  GtkTreeModel        *model;
  GtkWidget           *view,*sc;
  /* Setup the selection handler */
  GtkTreeSelection *selection;

  view = gtk_tree_view_new ();
  favlist = view;
  /* --- Column #1 --- */

  renderer = gtk_cell_renderer_text_new ();
  gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (view),-1,"Points Favoris"
					       ,renderer,"text", COL_NAME,NULL);  
  model = create_and_fill_fav ();
  gtk_tree_view_set_model (GTK_TREE_VIEW (view), model);

  /* The tree view has acquired its own reference to the
   * model, so we can drop ours. That way the model will
   * be freed automatically when the tree view is destroyed */


  /* evenement selection */
  selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(view));
  gtk_tree_selection_set_select_function(selection, view_sel_fav, NULL, NULL);
  gtk_tree_selection_set_mode(selection,GTK_SELECTION_BROWSE);

  g_object_unref (model);
  sc = gtk_scrolled_window_new(NULL,NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sc),GTK_POLICY_AUTOMATIC,GTK_POLICY_AUTOMATIC );
  gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW (sc), view);
  return sc;
}

void elt_remove ()
{
  gfloat is=0;
      

  if (giter.stamp != -69)
    gtk_tree_model_get(gmodel, &giter, COL_IS_DEL, &is, -1);
  if (is)
    {
      gtk_list_store_remove (store,&giter);
      giter.stamp = -69;
    }
}

void elt_add (GtkWidget *w UNUSED, gpointer lfav UNUSED)
{
  GtkTreeIter iter;
  const gchar *s;
  
  if (nom_lf)
    {
      s = gtk_entry_get_text(GTK_ENTRY (nom_lf));
      
      gtk_list_store_append (store, &iter);
      gtk_list_store_set (store, &iter,
			  COL_NAME,s,
			  COL_ID,i++,
			  COL_IS_DEL,1,
			  COL_X,cam.x,
			  COL_Y,cam.y,
			  -1);
      gtk_entry_set_text(GTK_ENTRY (nom_lf), "");

    
      /*  gtk_widget_queue_draw(GTK_WIDGET (favlist));*/
      gtk_widget_hide_all(GTK_WIDGET (favlist));
      gtk_widget_show_all(GTK_WIDGET (favlist));
    }
}


GtkWidget * create_lfav ()
{
  GtkWidget 
    *badd, *bdel,
    *vbbox, *hbox;

  /* init boxes */
  hbox = gtk_hbox_new(0,2);
  vbbox = gtk_vbox_new(0,2);;

  /* init list */
  giter.stamp = -69; /* init pour suppr */
  lfav =  create_view_and_model_fav ();

  /* init buttons */
  bdel = gtk_button_new_with_label("Supprimer");
  g_signal_connect (bdel, "clicked",G_CALLBACK (elt_remove), lfav);

  badd = gtk_button_new_with_label("Ajouter");
  g_signal_connect (badd, "clicked",G_CALLBACK (elt_add), lfav);

  /* init lfav */
  nom_lf = gtk_entry_new();

  gtk_container_add (GTK_CONTAINER (hbox), lfav);
  /*gtk_container_add (GTK_CONTAINER (hbox), vbbox);*/
  gtk_box_pack_start(GTK_BOX (hbox),vbbox,FALSE,FALSE,0);
  /*gtk_container_add (GTK_CONTAINER (vbbox), badd);
    gtk_container_add (GTK_CONTAINER (vbbox), bdel);*/
  gtk_box_pack_start(GTK_BOX (vbbox),nom_lf,0,0,0);
  gtk_box_pack_start(GTK_BOX (vbbox),badd,0,0,0);
  gtk_box_pack_start(GTK_BOX (vbbox),bdel,0,0,0);

  return(lfav = hbox);
}

void refresh_lfav()
{
  if (lfav)
    {
      gtk_widget_destroy(lfav);
      lfav = create_lfav();
      gtk_container_add (GTK_CONTAINER (fakebox_lf),lfav);
      gtk_widget_show_all(fakebox_lf);
    }
}
/* alexandre... t'es un putain de connard */
/* alexandre... t'es un putain de connard */
/* alexandre... t'es un putain de connard */
/* alexandre... t'es un putain de connard */
