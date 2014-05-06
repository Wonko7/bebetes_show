#include <gtk/gtk.h>
#include <stdlib.h>
#include <string.h>
#include "taskbrowser.h"
#include "lib_bbs_behavior.h"
#include "loader.h"
#include "divers.h"
#include "limits.h"

/* dimensions icones */
#define X 100
#define Y 140
#define DX 42
#define DY 29
#define HY 26
#define ESPACE 60
#define LX 32
#define LY 110


/*
**   au lieu d'essayer de comprendre mon code
**   vous pouvez aussi regarder ce tuto : 
**   http://scentric.net/tutorial/treeview-tutorial.html
*/

/* les tab. des libs :  */
extern behavior * stimuli;
extern int nb_sti;
extern agent * agents;
extern int nb_ag;
extern primitive * primitives;
extern int nb_prim;
extern operator * operators;
extern int nb_arg;
extern argument * arguments;
extern int nb_op;
extern void (*remove_task)();

int selected_ag = 0;
int selected_op = 0;
int selected_p = 0;
int selected_sti = 0;
int selected_i = 0;
int selected_w = 0;
int selected_t = 0;
/*spin button t_m*/

GtkWidget * sti_su = 0;
GtkWidget * sti_add_to = 0;

GtkWidget *da_act_su = 0;
GtkWidget *da_act = 0;
GtkWidget *te_arg,*te_w,*te_m,*te_i,*te_wc;
GtkWidget *lab_int=0, *lab_ext, *fb_int;

extern char * chemin;
  

/* tb */
extern GtkWidget *onglet_taskbrowser;

enum
  {                /***********************************/
    COL_NAME = 0,  /* nom du champ                    */
    COL_ID,        /* id du champ... p sur fonction ? */
    NUM_COLS       /* nombre de colones               */
  };               /***********************************/



int id_ag=-1,id_st=-1,id_op=-1,id_pr=-1;




/********************************************** 
 **  ARGUMENTS  ******************************* 
 **********************************************/

int ref_arg(char * arg)
{
  int i=0;

  if (!strcmp(arg,"Aucun"))
    return(-1);

  while (strcmp(arg,arguments[i].name))
    i++;

  if (i>=nb_arg)
    {
      return(-1);
      caca("Arg pas trouvé !!");
    }
  return(i);
}


/**************************************************** 
 **  GESTION DES LISTES : INIT+EVENEMENTS+VIEW  *****
 ****************************************************/

/********************************************** 
 **  AGENTS  ********************************** 
 **********************************************/



static GtkTreeModel * create_and_fill_agents ()
{
  GtkListStore  *store;
  GtkTreeIter    iter;
  int i;

  store = gtk_list_store_new (NUM_COLS, G_TYPE_STRING, G_TYPE_UINT);
  
  for (i=0;i<nb_ag;i++) 
    {
      gtk_list_store_append (store, &iter);
      gtk_list_store_set (store, &iter,COL_NAME,agents[i].name_ag,COL_ID,agents[i].ag,-1);
    }
  
  return GTK_TREE_MODEL (store);
}

gboolean view_sel_agents (GtkTreeSelection *selection UNUSED,
			  GtkTreeModel     *model,
			  GtkTreePath      *path,
			  gboolean          path_currently_selected,
			  gpointer          userdata UNUSED)
{
  GtkTreeIter iter;
  
  if (gtk_tree_model_get_iter(model, &iter, path))
    {
      gint id;
      gtk_tree_model_get(model, &iter, COL_ID, &id, -1);
      
      if (!path_currently_selected && id != id_ag)
	{
	  selected_sti = -1;
	  id_ag = id;
	  selected_ag = id;
	  recreate_sti ();
	  draw_task ();
	}
    }

  return 1;
}

static GtkWidget * create_view_and_model_agents (void)
{
  GtkCellRenderer     *renderer;
  GtkTreeModel        *model;
  GtkWidget           *view,*sc;
  GtkTreePath         *path;
  /* Setup the selection handler */
  GtkTreeSelection *selection;

  view = gtk_tree_view_new ();

  /* --- Column #1 --- */

  renderer = gtk_cell_renderer_text_new ();
  gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (view),-1,"Agents",renderer,"text", COL_NAME,NULL);

  model = create_and_fill_agents ();

  gtk_tree_view_set_model (GTK_TREE_VIEW (view), model);

  /* The tree view has acquired its own reference to the
   *  model, so we can drop ours. That way the model will
   *  be freed automatically when the tree view is destroyed */

  /* evenement selection */
  selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(view));
  path = gtk_tree_path_new_from_string("0");
  gtk_tree_selection_select_path (selection,path);
  free(path);
  gtk_tree_selection_set_select_function(selection, view_sel_agents, NULL, NULL);
  gtk_tree_selection_set_mode(selection,GTK_SELECTION_BROWSE);

  g_object_unref (model);
  sc = gtk_scrolled_window_new(NULL,NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sc),GTK_POLICY_AUTOMATIC,GTK_POLICY_AUTOMATIC );
  gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW (sc), view);

  return sc;
}


/********************************************** 
 **  STIMULI  ********************************* 
 **********************************************/



static GtkTreeModel * create_and_fill_stimuli ()
{
  GtkListStore  *store;
  GtkTreeIter    iter;
  int i;

  store = gtk_list_store_new (NUM_COLS, G_TYPE_STRING, G_TYPE_UINT);
  
  for (i=0;i<nb_sti;i++) 
    {
      if (stimuli[i].ag == selected_ag)
	{
	  gtk_list_store_append (store, &iter);
	  gtk_list_store_set (store, &iter,COL_NAME,stimuli[i].name_s,COL_ID,i,-1);
	}
    }
  
  return GTK_TREE_MODEL (store);
}

gboolean view_sel_stimuli (GtkTreeSelection *selection UNUSED,
		     GtkTreeModel     *model,
		     GtkTreePath      *path,
		     gboolean          path_currently_selected,
		     gpointer          userdata UNUSED)
{
  GtkTreeIter iter;
  gint id;

  
  if (gtk_tree_model_get_iter(model, &iter, path))
    {
      gtk_tree_model_get(model, &iter, COL_ID, &id, -1);
      if (selected_sti == -1 || (!path_currently_selected && (id != id_st)))
	{
	  id_st = id;
	  selected_sti = id;
	  draw_task();
	}
    }
  return TRUE;
}

static GtkWidget * create_view_and_model_stimuli (void)
{
  GtkCellRenderer     *renderer;
  GtkTreeModel        *model;
  GtkWidget           *view,*sc;
  /* Setup the selection handler */
  GtkTreeSelection    *selection;
  GtkTreePath         *path;

  view = gtk_tree_view_new ();

  /* --- Column #1 --- */

  renderer = gtk_cell_renderer_text_new ();
  gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (view),-1,"Stimuli",renderer,"text", COL_NAME,NULL);

  model = create_and_fill_stimuli ();

  gtk_tree_view_set_model (GTK_TREE_VIEW (view), model);

  /* The tree view has acquired its own reference to the
   *  model, so we can drop ours. That way the model will
   *  be freed automatically when the tree view is destroyed */

  /* evenement selection */
  selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(view));
  path = gtk_tree_path_new_from_string("0");
  gtk_tree_selection_select_path (selection,path);
  free(path);
  gtk_tree_selection_set_select_function(selection, view_sel_stimuli, NULL, NULL);
  gtk_tree_selection_set_mode(selection,GTK_SELECTION_BROWSE);

  g_object_unref (model);
  sc = gtk_scrolled_window_new(NULL,NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sc),GTK_POLICY_AUTOMATIC,GTK_POLICY_AUTOMATIC );
  gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW (sc), view);

  sti_su = sc;
  return sc;
}



/********************************************** 
 **  OPERATEURS  ****************************** 
 **********************************************/


static GtkTreeModel * create_and_fill_ops ()
{
  GtkListStore  *store;
  GtkTreeIter    iter;
  int i;

  store = gtk_list_store_new (NUM_COLS, G_TYPE_STRING, G_TYPE_UINT);
  
  for (i=0;i<nb_op;i++) 
    {
      gtk_list_store_append (store, &iter);
      gtk_list_store_set (store, &iter,COL_NAME,operators[i].name_op,COL_ID,i,-1);
    }
  
  return GTK_TREE_MODEL (store);
}

gboolean view_sel_ops (GtkTreeSelection *selection UNUSED,
		       GtkTreeModel     *model,
		       GtkTreePath      *path,
		       gboolean          path_currently_selected,
		       gpointer          userdata UNUSED)
{
  GtkTreeIter iter;
  
  if (gtk_tree_model_get_iter(model, &iter, path))
    {
      gint id;
      gtk_tree_model_get(model, &iter, COL_ID, &id, -1);
      
      if (!path_currently_selected && id != id_op)
	{
	  id_op = id;
	  selected_op = id;
	}
    }

  return TRUE;
}

static GtkWidget * create_view_and_model_ops (void)
{
  GtkCellRenderer     *renderer;
  GtkTreeModel        *model;
  GtkWidget           *view,*sc;
  /* Setup the selection handler */
  GtkTreeSelection *selection;
  GtkTreePath         *path;

  view = gtk_tree_view_new ();

  /* --- Column #1 --- */

  renderer = gtk_cell_renderer_text_new ();
  gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (view),-1,"Operateurs",renderer,"text", COL_NAME,NULL);

  model = create_and_fill_ops ();

  gtk_tree_view_set_model (GTK_TREE_VIEW (view), model);

  /* The tree view has acquired its own reference to the
   *  model, so we can drop ours. That way the model will
   *  be freed automatically when the tree view is destroyed */


  /* evenement selection */
  selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(view));
  path = gtk_tree_path_new_from_string("0");
  gtk_tree_selection_select_path (selection,path);
  free(path);
  gtk_tree_selection_set_select_function(selection, view_sel_ops, NULL, NULL);
  gtk_tree_selection_set_mode(selection,GTK_SELECTION_BROWSE);

  g_object_unref (model);

  sc = gtk_scrolled_window_new(NULL,NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sc),GTK_POLICY_AUTOMATIC,GTK_POLICY_AUTOMATIC );
  gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW (sc), view);
  return sc;

}



/********************************************** 
 **  PRIMITIVES  ****************************** 
 **********************************************/



static GtkTreeModel * create_and_fill_primitives ()
{
  GtkListStore  *store;
  GtkTreeIter    iter;
  int i;

  store = gtk_list_store_new (NUM_COLS, G_TYPE_STRING, G_TYPE_UINT);
  
  for (i=0;i<nb_prim;i++) 
    {
      gtk_list_store_append (store, &iter);
      gtk_list_store_set (store, &iter,COL_NAME,primitives[i].name_p,COL_ID,i,-1);
    }
  
  return GTK_TREE_MODEL (store);
}

gboolean view_sel_primitives (GtkTreeSelection *selection UNUSED,
		     GtkTreeModel     *model,
		     GtkTreePath      *path,
		     gboolean          path_currently_selected,
		     gpointer          userdata UNUSED)
{
  GtkTreeIter iter;
  
  if (gtk_tree_model_get_iter(model, &iter, path))
    {
      gint id;
      gtk_tree_model_get(model, &iter, COL_ID, &id, -1);
      
      if (!path_currently_selected && id != id_pr)
	{
	  id_pr = id;
	  selected_p = id;
	}
    }

  return TRUE;
}

static GtkWidget * create_view_and_model_primitives (void)
{
  GtkCellRenderer     *renderer;
  GtkTreeModel        *model;
  GtkWidget           *view,*sc;
  /* Setup the selection handler */
  GtkTreeSelection *selection;
  GtkTreePath         *path;

  view = gtk_tree_view_new ();

  /* --- Column #1 --- */

  renderer = gtk_cell_renderer_text_new ();
  gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (view),-1,"Primitives",renderer,"text", COL_NAME,NULL);

  model = create_and_fill_primitives ();

  gtk_tree_view_set_model (GTK_TREE_VIEW (view), model);

  /* The tree view has acquired its own reference to the
   *  model, so we can drop ours. That way the model will
   *  be freed automatically when the tree view is destroyed */


  /* evenement selection */
  selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(view));
  path = gtk_tree_path_new_from_string("0");
  gtk_tree_selection_select_path (selection,path);
  free(path);
  gtk_tree_selection_set_select_function(selection, view_sel_primitives, NULL, NULL);
  gtk_tree_selection_set_mode(selection,GTK_SELECTION_BROWSE);

  g_object_unref (model);

  sc = gtk_scrolled_window_new(NULL,NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sc),GTK_POLICY_AUTOMATIC,GTK_POLICY_AUTOMATIC );
  gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW (sc), view);
  return sc;  
}




/**************************************************** 
 **  LE RESTE : MISE EN PAGE + BOUTONS...  **********
 ****************************************************/

/********************************************** 
 **  LISTES TASK/BRICKS  ********************** 
 **********************************************/


/********************************************** 
 **  REFRESH STIMULI  ************************* 
 **********************************************/


void recreate_sti ()
{     

  static int en_cours = 0;

  if (sti_su && sti_add_to && !en_cours)
    {
      en_cours = 1;
      gtk_widget_destroy(sti_su);
      sti_su = create_view_and_model_stimuli();
      gtk_container_add (GTK_CONTAINER (sti_add_to), sti_su);
      gtk_widget_show_all(sti_add_to);
      en_cours = 0;
    }
}

/********************************************** 
 **  EVENEMENTS BOUTONS  ********************** 
 **********************************************/

/*
void evb_simulation () 
{
  puts("Evenement bouton 'simulation' cliqué");
}
*/

void evb_remove () 
{
  if (selected_sti == -1 || 
      (!stimuli[selected_sti].task.brick && !stimuli[selected_sti].task.interruption))
    gtk_caca("Aucune action a supprimer");
  else
    {
      remove_task (stimuli[selected_sti].task);
      stimuli[selected_sti].task.interruption = 0;
      stimuli[selected_sti].task.brick = 0;
      stimuli[selected_sti].task.weight = 0;
      stimuli[selected_sti].task.weight_change = 0;
      stimuli[selected_sti].task.threshold = 0;
      draw_task();
    }
}


void evb_add () 
{
  brick b,bn;
  int i=0;
  float selected_w,selected_i,selected_t,selected_wc;

  b = stimuli[selected_sti].task.brick;
  bn = b;
  get_witc(&selected_w,&selected_i,&selected_t,&selected_wc);

  if (selected_sti != -1)
    {
      while (bn)
	{
	  b = bn;
	  bn = (*bn).next;
	  i++;
	}
  
      if (b && !(*b).type_op)
	{
	  if (stimuli[selected_sti].task.interruption)
	    gtk_caca("L'action est deja complete, il faut la supprimer pour la recommencer");
	  else
	    {
	      if (operators[selected_op].type_op)
		gtk_caca("Pour l'interruption l'operateur doit etre terminal");
	      else
		{
		  bn = malloc(sizeof (struct n_brick));
		  (*bn).op = selected_op;
		  (*bn).name_op = operators[selected_op].name_op;
		  (*bn).type_op = operators[selected_op].type_op;
		  (*bn).icone_op = operators[selected_op].icone_op;
		  (*bn).p = selected_p;
		  (*bn).name_p = primitives[selected_p].name_p;
		  (*bn).icone_p = primitives[selected_p].icone_p;
		  (*bn).arg =  gtk_combo_box_get_active(GTK_COMBO_BOX(te_arg)) - 1;
		  (*bn).next = 0;
		  stimuli[selected_sti].task.interruption = bn;
		  stimuli[selected_sti].task.weight = selected_w;
		  stimuli[selected_sti].task.increment = selected_i;
		  stimuli[selected_sti].task.threshold = selected_t;
		  stimuli[selected_sti].task.weight_change = selected_wc;
		}
	    }
	}
      
      else 
	{
	  bn = malloc(sizeof (struct n_brick));
	  (*bn).op = selected_op;
	  (*bn).name_op = operators[selected_op].name_op;
	  (*bn).type_op = operators[selected_op].type_op;
	  (*bn).icone_op = operators[selected_op].icone_op;
	  (*bn).p = selected_p;
	  (*bn).name_p = primitives[selected_p].name_p;
	  (*bn).icone_p = primitives[selected_p].icone_p;
	  (*bn).arg =  gtk_combo_box_get_active(GTK_COMBO_BOX(te_arg)) - 1;
	  (*bn).next = 0;
	  stimuli[selected_sti].task.weight = selected_w;
	  stimuli[selected_sti].task.increment = selected_i;
	  stimuli[selected_sti].task.threshold = selected_t;
	  stimuli[selected_sti].task.weight_change = selected_wc;
	  
	  if (i)
	    (*b).next = bn;
	  else      
	    stimuli[selected_sti].task.brick = bn;
	}
    }
  else
    gtk_caca("On ne peut pas ajouter d'action si le stimulus n'existe pas");

 draw_task ();
}

/* ok, pas tres lisible celle ci... a refaire si j'ai du temps */
void draw_task ()
{
  static int en_cours2 = 0;
  GtkWidget * il1,*sc,*l1;
  brick b;
  char * t1;
  int i=10;

  en_cours2 = 1;
  if ((selected_sti != -1)/* && !en_cours2*/ && da_act && da_act_su && (b = stimuli[selected_sti].task.brick))
    {

      il1 = gtk_fixed_new();

      gtk_widget_destroy(da_act_su);
      da_act_su = gtk_hbox_new(0,20);

      sc = gtk_scrolled_window_new(NULL,NULL);
      gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sc),GTK_POLICY_AUTOMATIC,GTK_POLICY_AUTOMATIC );
      gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW (sc), il1);

      gtk_container_add (GTK_CONTAINER (da_act_su), sc);
      gtk_container_add (GTK_CONTAINER (da_act), da_act_su);

      if (b)
	{
	  t1 = my_concat(chemin,"/../share/bebetes_show/tb_icones/head.png");
	  gtk_fixed_put(GTK_FIXED (il1), gtk_image_new_from_file(t1),i,5);
	  i += HY;
	}

      while (b)
	{
	  gtk_fixed_put(GTK_FIXED (il1), gtk_image_new_from_file((*b).icone_op),i,5);
	  gtk_fixed_put(GTK_FIXED (il1), gtk_image_new_from_file((*b).icone_p),i+DX,5+DY);

	  if (b->arg != -1)
	    {
	      l1 = gtk_label_new (arguments[b->arg].name);
	      gtk_fixed_put(GTK_FIXED (il1), l1,i+LX,5+LY);
	    }

	  i += Y;
	  b = (*b).next;
	}

      if (stimuli[selected_sti].task.interruption)
	{
	  il1 = gtk_fixed_new();

	  t1 = my_concat(chemin,"/../share/bebetes_show/tb_icones/head.png");
	  gtk_fixed_put(GTK_FIXED (il1), gtk_image_new_from_file(t1),0,5);

	  gtk_fixed_put(GTK_FIXED (il1),
			gtk_image_new_from_file ((*(stimuli[selected_sti].task.interruption)).icone_op),
			HY,5);
	  gtk_fixed_put(GTK_FIXED (il1),
			gtk_image_new_from_file ((*(stimuli[selected_sti].task.interruption)).icone_p),
			HY+DX,5+DY);
	  if (stimuli[selected_sti].task.interruption->arg != -1)
	    {
	      l1 = gtk_label_new (arguments[stimuli[selected_sti].task.interruption->arg].name);
	      gtk_fixed_put(GTK_FIXED (il1), l1,LX+LX,5+LY);
	    }

	  gtk_box_pack_start(GTK_BOX (da_act_su),il1,FALSE,FALSE,0);        
	}

      set_witc(stimuli[selected_sti].task.weight,
	       stimuli[selected_sti].task.increment,
	       stimuli[selected_sti].task.threshold,
	       stimuli[selected_sti].task.weight_change,
	       stimuli[selected_sti].interne);
      
      gtk_widget_show_all(da_act);
    }
  else
    if (/*en_cours2 &&*/ da_act && da_act_su)
      {
	gtk_widget_destroy(da_act_su);
	da_act_su = gtk_hbox_new(0,0);
	gtk_container_add (GTK_CONTAINER (da_act), da_act_su);
	gtk_widget_show_all(da_act);
	set_witc(0,0,stimuli[selected_sti].task.force,0,stimuli[selected_sti].interne);
      }
  en_cours2 = 0;
}

GtkWidget * init_arg()
{
  int i;
  GtkWidget  *te_arg;
  te_arg=gtk_combo_box_new_text();

  gtk_combo_box_append_text((GtkComboBox *)te_arg,"Aucun");
  for (i=0; i<nb_arg; i++)
    gtk_combo_box_append_text((GtkComboBox *)te_arg,arguments[i].name);

  gtk_combo_box_set_active(GTK_COMBO_BOX(te_arg),0);
  return(te_arg);
}


void get_witc(float *w, float *i, float *t, float *wc)
{
  *w = gtk_spin_button_get_value(GTK_SPIN_BUTTON (te_w));
  *i = gtk_spin_button_get_value(GTK_SPIN_BUTTON (te_i));
  *t = gtk_spin_button_get_value(GTK_SPIN_BUTTON (te_m));
  *wc = gtk_spin_button_get_value(GTK_SPIN_BUTTON (te_wc));
}

void set_witc(float w, float i, float t, float wc, int is_int)
{
  gtk_spin_button_set_value(GTK_SPIN_BUTTON (te_w),w);
  gtk_spin_button_set_value(GTK_SPIN_BUTTON (te_m),t);
  gtk_spin_button_set_value(GTK_SPIN_BUTTON (te_i),i);
  gtk_spin_button_set_value(GTK_SPIN_BUTTON (te_wc),wc);

  if (lab_int)
    gtk_widget_destroy(lab_int);
  if (is_int)
    lab_int = gtk_label_new ("Interne");
  else
    lab_int = gtk_label_new ("Externe");
  gtk_container_add (GTK_CONTAINER (fb_int), lab_int);
  gtk_widget_show_all(fb_int);
}

void create_tb ()
{  

  GtkWidget 
    *agents,*stimuli,*vb_para,*ops,*primitives,
    *hbutbox,*vb_d,*vb_g,*vb_m,*hb_m,*hb_m2,*hb_p1,*hb_p2,*hb_p3,*hb_p4,*fake_box,
    *but_rem,*but_add,
    *l1,*l2,*l3,*l4,*l5,*l6,
    *table;

  GtkObject *te_w_adj, *te_m_adj, *te_i_adj, *te_wc_adj;

  selected_ag = 0;
  selected_op = 0;
  selected_p = 0;
  selected_sti = 0;
  selected_i = 0;
  selected_w = 0;
  selected_t = 0;
  sti_su = 0;
  sti_add_to = 0;
  da_act_su = 0;
  da_act = 0;


  /* init table */
  table = gtk_table_new(1,4,1);

  /* init boxes */
  onglet_taskbrowser = gtk_hbox_new(0,2);
  vb_para = gtk_vbox_new(0,2);
  vb_d = gtk_vbox_new(0,2);
  vb_g = gtk_vbox_new(0,2);
  vb_m = gtk_vbox_new(0,2);
  hb_m = gtk_hbox_new(0,2);
  hb_m2 = gtk_hbox_new(0,2);
  hbutbox = gtk_hbutton_box_new();;

  hb_p1 = gtk_hbox_new(1,2);
  hb_p2 = gtk_hbox_new(1,2);
  hb_p3 = gtk_hbox_new(1,2);
  hb_p4 = gtk_hbox_new(1,2);
  fake_box = gtk_hbox_new(0,0);

  /* init listes */
  agents     =  create_view_and_model_agents ();
  ops        =  create_view_and_model_ops ();
  primitives =  create_view_and_model_primitives ();
  stimuli    =  create_view_and_model_stimuli ();

  /* init boutons */
  but_rem=gtk_button_new_with_label("Supprimer Tache");
  g_signal_connect (but_rem, "clicked",G_CALLBACK (evb_remove), NULL);

  but_add=gtk_button_new_with_label("Ajout Primitive");
  g_signal_connect (but_add, "clicked",G_CALLBACK (evb_add), NULL);
  
  /* init labels */
  l1 = gtk_label_new ("Parametres");
  l2 = gtk_label_new ("Poids");
  l3 = gtk_label_new ("Seuil");
  l4 = gtk_label_new ("Increment");
  l6 = gtk_label_new ("Renforcement");
  l5 = gtk_label_new ("Argument");
  lab_int = gtk_label_new ("Interne");
  lab_ext = gtk_label_new ("Externe");
  fb_int = gtk_vbox_new(0,2);
  
  /* init wit (spin buts) */
  te_w_adj = gtk_adjustment_new (0, 0, 100, 1,10, 0);
  te_w=gtk_spin_button_new((GtkAdjustment *)te_w_adj,0,0);
  gtk_spin_button_set_wrap( GTK_SPIN_BUTTON (te_w),0 );

  te_m_adj = gtk_adjustment_new (0, 0, INT_MAX, 1,10, 0);
  te_m=gtk_spin_button_new((GtkAdjustment *)te_m_adj,0,0);
  gtk_spin_button_set_wrap( GTK_SPIN_BUTTON (te_m),0 );

  te_i_adj = gtk_adjustment_new (0, 0, 100, 1,10, 0);
  te_i=gtk_spin_button_new((GtkAdjustment *)te_i_adj,0,0);
  gtk_spin_button_set_wrap( GTK_SPIN_BUTTON (te_i),0 );

  te_wc_adj = gtk_adjustment_new (0, -1, 1, 1,10, 0);
  te_wc = gtk_spin_button_new((GtkAdjustment *)te_wc_adj,0,0);
  gtk_spin_button_set_wrap( GTK_SPIN_BUTTON (te_wc),0 );

  
  te_arg=init_arg();

  /**/
  da_act = gtk_hbox_new(0,0);
  da_act_su = gtk_hbox_new(0,0);

  /******************/
  /* mise en page : */
  /******************/

  /* page : */
  gtk_container_add (GTK_CONTAINER (onglet_taskbrowser), table);  
  gtk_table_attach_defaults(GTK_TABLE (table),vb_g,0,1,0,1);
  gtk_table_attach_defaults(GTK_TABLE (table),vb_d,1,4,0,1);

  /** colonne de gauche ************/
  gtk_container_set_border_width (GTK_CONTAINER (vb_g), 10);
  gtk_container_add (GTK_CONTAINER (vb_g), agents);
  gtk_container_add (GTK_CONTAINER (fake_box), stimuli);
  gtk_container_add (GTK_CONTAINER (vb_g), fake_box);
  gtk_container_add (GTK_CONTAINER (vb_g), vb_para);
  sti_add_to = fake_box;

  /* parametres : */
  gtk_container_add (GTK_CONTAINER (vb_para), l1);
  gtk_container_add (GTK_CONTAINER (vb_para), hb_p1);
  gtk_container_add (GTK_CONTAINER (vb_para), hb_p2);
  /*gtk_container_add (GTK_CONTAINER (vb_para), hb_p3);*/
  gtk_container_add (GTK_CONTAINER (vb_para), hb_p4);
  gtk_container_add (GTK_CONTAINER (vb_para), fb_int);
  /*gtk_container_add (GTK_CONTAINER (fb_int), lab_int);
    gtk_container_add (GTK_CONTAINER (fb_int), lab_ext);*/


  gtk_container_add (GTK_CONTAINER (hb_p1), l2); 
  gtk_container_add (GTK_CONTAINER (hb_p1), te_w); 

  gtk_container_add (GTK_CONTAINER (hb_p2), l3); 
  gtk_container_add (GTK_CONTAINER (hb_p2), te_m); 
  
  gtk_container_add (GTK_CONTAINER (hb_p3), l4); 
  gtk_container_add (GTK_CONTAINER (hb_p3), te_i); 
  
  gtk_container_add (GTK_CONTAINER (hb_p4), l6); 
  gtk_container_add (GTK_CONTAINER (hb_p4), te_wc); 


  /* colonne de droite *************/
  gtk_container_set_border_width (GTK_CONTAINER (vb_d), 10);
  gtk_box_pack_start(GTK_BOX (vb_d),hbutbox,FALSE,FALSE,0);  
  gtk_container_add (GTK_CONTAINER (vb_d), hb_m);
  gtk_box_pack_start(GTK_BOX (vb_d),da_act,FALSE,FALSE,0);
  gtk_widget_set_size_request(da_act,-1,X+ESPACE);

  /* truc au centre */
  gtk_container_add (GTK_CONTAINER (hb_m), ops);
  gtk_container_add (GTK_CONTAINER (hb_m), vb_m);

  gtk_container_add (GTK_CONTAINER (vb_m), primitives);
  gtk_box_pack_start(GTK_BOX (vb_m),hb_m2,FALSE,FALSE,0);  

  gtk_container_add (GTK_CONTAINER (hb_m2), l5);
  gtk_container_add (GTK_CONTAINER (hb_m2), te_arg);

  /* boutons */
  gtk_container_add (GTK_CONTAINER (hbutbox), but_rem);
  gtk_container_add (GTK_CONTAINER (hbutbox), but_add);

  draw_task();
}



/**************************************************** 
 **  EDIT : ADD/REMOVE STUFF                    *****
 ****************************************************/

/* alexandre... t'es un putain de connard */
/* alexandre... t'es un putain de connard */
/* alexandre... t'es un putain de connard */
/* alexandre... t'es un putain de connard */
