#include <stdlib.h>
#include <gtk/gtk.h>
#include <math.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include "options.h"
#include "loader.h"
#include "divers.h"
#include "camera.h"
#include "map.h"
#include "loading.h"

GtkWidget
  *spin_autosave,
  *spin_profondeur,
  *mipmap,
  *lumiere,
  *w_load,
  *w_suppr,
  *opt_gcc;

int
  options_mipmap=1,
  options_autosave=10,
  options_lumiere=1,
  options_w_load = 1,
  options_w_suppr = 1,
  out = -69,
  first_time = 1;

char 
  *lib_sel = 0,
  *sim_sel = 0,
  *options_gcc = 0;

extern int realized,gHeight,gWidth;
extern float ratio;
float options_profondeur=500.0;
extern char *chemin;
extern char *save_dir;
extern GtkWidget *window;
GtkWidget *fbox11 = 0, *fbox22 = 0, *onglet_sv = 0, *onglet_op = 0;



/*fakeboxing*/
void refresh_sv()
{
  if (onglet_sv)
    gtk_widget_destroy(onglet_sv);
  create_loading();
  if (onglet_op)
    gtk_widget_destroy(onglet_op);
  create_options();
  gtk_widget_show_all(onglet_op);
  gtk_widget_show_all(onglet_sv);
}



/**************************************************
 *  LOAD/SAVE FICHIERS  *************************** 
 **************************************************/



int write_options()
{
  FILE *s;
  char *t1;


  start_locale();

  t1 = my_concat(save_dir,"/../options.bbs");
  if (!(s  = fopen(t1,"w")))
    {
      caca(t1);
      caca("l'ouvrirure du fichier nix marcher");
      free(t1);
      end_locale();
      return(0);
    }

  fprintf(s,"** Fichier d'options bebetes_show **\n");

  fprintf(s,"mipmap:%i\n",options_mipmap);
  fprintf(s,"profondeur:%f\n",options_profondeur);
  fprintf(s,"autosave:%i\n",options_autosave);
  fprintf(s,"lumiere:%i\n",options_lumiere);
  fprintf(s,"warning_load:%i\n",options_w_load);
  fprintf(s,"warning_suppr:%i\n",options_w_suppr);
  fprintf(s,"%s\n",options_gcc);

  free(t1);
  fclose(s);
  end_locale();

  return(1);
}

int read_options()
{
  FILE *s;
  char *t1, c;
  int i = 0;

  start_locale();

  t1 = my_concat(save_dir,"/../options.bbs");
  if (!(s  = fopen(t1,"r")))
    {
      options_gcc = my_concat("gcc `pkg-config --cflags gtk+-2.0` `pkg-config --cflags gtkglext-1.0` -fPIC -shared -nostartfiles -o", "");
      free(t1);
      end_locale();
      return(1);
    }

  fscanf(s,"** Fichier d'options bebetes_show **\n");

  fscanf(s,"mipmap:%i\n",&options_mipmap);
  fscanf(s,"profondeur:%f\n",&options_profondeur);
  fscanf(s,"autosave:%i\n",&options_autosave);
  fscanf(s,"lumiere:%i\n",&options_lumiere);
  fscanf(s,"warning_load:%i\n",&options_w_load);
  fscanf(s,"warning_suppr:%i\n",&options_w_suppr);

  c = getc(s);
  while (c != '\n' && c != EOF)
    {
      options_gcc = realloc(options_gcc, (i + 2) * (sizeof (char)));
      options_gcc[i] = c;
      options_gcc[i + 1] = 0;
      i++;
      c = getc(s);
    }

  if (!i)
    options_gcc = my_concat("gcc `pkg-config --cflags gtk+-2.0` `pkg-config --cflags gtkglext-1.0` -fPIC -shared -nostartfiles -o", "");

  free(t1);
  fclose(s);
  end_locale();

  return(1);
}



/**************************************************
 *  GTK JULIEN  *********************************** 
 **************************************************/



void set_options()
{
  gtk_entry_set_text(GTK_ENTRY (opt_gcc), options_gcc);
  gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin_autosave),options_autosave);
  gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin_profondeur),options_profondeur);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(mipmap),options_mipmap);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(lumiere),options_lumiere);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(w_load),options_w_load);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(w_suppr),options_w_suppr);
 }

int get_options_w_load()
{
  return(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(w_load))); 
}

int get_options_w_suppr()
{
  return(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(w_suppr))); 
}

int get_options_profondeur()
{
  return(gtk_spin_button_get_value_as_float(GTK_SPIN_BUTTON(spin_profondeur)));
}

int get_options_mipmap()
{
  return(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(mipmap))); 
}

int get_options_autosave()
{
  return(gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spin_autosave)));
}

int get_options_lumiere()
{
  return(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(lumiere))); 
}

void set_var_profondeur()
{
  options_profondeur=get_options_profondeur();
  if (realized)
    {
      glMatrixMode(GL_PROJECTION);
      glLoadIdentity();
      
      gluPerspective(FOVY,ratio,0.1f,options_profondeur);
      gHeight = 2.0 * tan(FOVY / 2.0) * options_profondeur;
      gWidth = 2.0 * tan(FOVY / 2.0) * ratio * options_profondeur ;
      
      glMatrixMode(GL_MODELVIEW);
    }
}

void set_var_mipmap()
{
  options_mipmap=get_options_mipmap();
  if (options_mipmap)
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  else
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
}

void set_var_autosave()
{
  options_autosave=get_options_autosave();
  if (out != -69)
    g_source_remove(out);
  if (options_autosave)
    out = g_timeout_add((guint32) options_autosave * 60000,(GtkFunction) save_auto,(gpointer) NULL);
  else
    out = -69;
}

void set_var_lumiere()
{
  options_lumiere=get_options_lumiere();
  if (options_lumiere)
    glEnable(GL_LIGHTING);
  else
    glDisable(GL_LIGHTING);
}


void set_var_w_load()
{
  options_w_load = get_options_w_load();
}

void set_var_w_suppr()
{
  options_w_suppr = get_options_w_suppr();
}

void set_var_opt_gcc()
{
  const char *s;

  s = gtk_entry_get_text(GTK_ENTRY (opt_gcc));
  if (options_gcc)
    free(options_gcc);
  options_gcc = my_concat2("", s);
}

GtkWidget* create_ops()
{
  GtkWidget
    *v_box_options,
    *h_box_tmp,
    *label,
    *f1, *f2, *tmp, *tmp2,
    *b;

  if (first_time)
    read_options();
  first_time = 0;

  if (options_autosave)
    out = g_timeout_add((guint32) options_autosave * 60000,(GtkFunction) save_auto,(gpointer) NULL);  

  v_box_options = gtk_vbox_new(0,20);
  h_box_tmp=gtk_vbox_new(0,2);

  f1 = gtk_frame_new("Options Generales");
  gtk_container_add(GTK_CONTAINER (v_box_options),f1);

  f2 = gtk_frame_new("Options Graphiques");
  gtk_container_add(GTK_CONTAINER (v_box_options),f2);

  tmp = gtk_vbox_new(0,5);
  gtk_container_add(GTK_CONTAINER (f1),tmp);
  f1 = tmp;
  tmp = gtk_vbox_new(0,5);
  gtk_container_add(GTK_CONTAINER (f2),tmp);
  f2 = tmp;
  gtk_container_set_border_width (GTK_CONTAINER (f1), 10);
  gtk_container_set_border_width (GTK_CONTAINER (f2), 10);

  label = gtk_label_new("Sauvegarde automatique (0 pour desactiver) : ");
  spin_autosave = gtk_spin_button_new_with_range(0,60,1); 
  tmp = gtk_hbox_new(0,5);
  gtk_box_pack_start(GTK_BOX (tmp),label,FALSE,TRUE,0);
  gtk_box_pack_start(GTK_BOX (tmp),spin_autosave,FALSE,FALSE,0);
  gtk_box_pack_start(GTK_BOX (f1),tmp,FALSE,TRUE,0);

  w_load = gtk_check_button_new_with_label("Demander confirmation avant de charger une simulation");
  gtk_box_pack_start(GTK_BOX (f1),w_load,FALSE,FALSE,0);

  w_suppr = gtk_check_button_new_with_label("Demander confirmation avant de supprimer une simulation");
  gtk_box_pack_start(GTK_BOX (f1),w_suppr,FALSE,FALSE,0);

  b = gtk_button_new_with_label("Appliquer"); 
  g_signal_connect (b, "clicked", G_CALLBACK (set_var_opt_gcc), NULL);  tmp = gtk_vbox_new(0, 5);
  tmp2 = gtk_hbox_new(0, 5);
  label = gtk_label_new("Ligne de compilation pour les bibliotheques dynamiques :");
  opt_gcc = gtk_entry_new();
  gtk_box_pack_start(GTK_BOX (tmp), label,FALSE,TRUE,0);
  //gtk_box_pack_start(GTK_BOX (tmp2), opt_gcc,FALSE,FALSE,0);
  gtk_container_add(GTK_CONTAINER (tmp2), opt_gcc);
  gtk_box_pack_start(GTK_BOX (tmp2), b,FALSE,FALSE,0);
  gtk_box_pack_start(GTK_BOX (tmp), tmp2, FALSE, FALSE,0);
  gtk_box_pack_start(GTK_BOX (f1), tmp, FALSE, FALSE,0);



  /*h_box_tmp = gtk_hbox_new(0,2);
    gtk_box_pack_start(GTK_BOX (h_box_tmp),mipmap,FALSE,FALSE,0);*/

  label = gtk_label_new("Profondeur d'affichage : ");
  spin_profondeur = gtk_spin_button_new_with_range(10,100000,10); 
  tmp = gtk_hbox_new(0,5);
  gtk_box_pack_start(GTK_BOX (tmp),label,FALSE,TRUE,0);
  gtk_box_pack_start(GTK_BOX (tmp),spin_profondeur,FALSE,FALSE,0);
  gtk_box_pack_start(GTK_BOX (f2),tmp,FALSE,TRUE,0);

  /*mipmap = gtk_check_button_new_with_label("Mipmap (optimisation des textures, desactiver si absence d'acceleration materielle)");*/
  mipmap = gtk_check_button_new_with_label("Mipmap (desactiver si absence d'acceleration materielle)");
  h_box_tmp = gtk_hbox_new(0,2);
  gtk_box_pack_start(GTK_BOX (h_box_tmp),mipmap,FALSE,FALSE,0);
  gtk_box_pack_start(GTK_BOX (f2),h_box_tmp,FALSE,FALSE,0);

  lumiere = gtk_check_button_new_with_label("Lumiere");
  h_box_tmp=gtk_hbox_new(0,2);
  gtk_box_pack_start(GTK_BOX (h_box_tmp),lumiere,FALSE,FALSE,0);
  gtk_box_pack_start(GTK_BOX (f2),h_box_tmp,FALSE,FALSE,0);

  g_signal_connect (G_OBJECT (spin_profondeur), "value-changed", G_CALLBACK (set_var_profondeur),NULL);
  g_signal_connect (G_OBJECT (spin_autosave), "value-changed", G_CALLBACK (set_var_autosave),NULL);
  g_signal_connect (G_OBJECT (mipmap), "toggled", G_CALLBACK (set_var_mipmap),NULL);
  g_signal_connect (G_OBJECT (lumiere), "toggled", G_CALLBACK (set_var_lumiere),NULL);
  g_signal_connect (G_OBJECT (w_load), "toggled", G_CALLBACK (set_var_w_load),NULL);
  g_signal_connect (G_OBJECT (w_suppr), "toggled", G_CALLBACK (set_var_w_suppr),NULL);
 
  set_options();
  return(v_box_options);
}
/***************************/


/**************************************************
 *  GTK STUFF  ************************************ 
 **************************************************/


enum
  {                /***********************************/
    COL_NAME = 0,  /* nom du champ                    */
    COL_ID,        /* id du champ... p sur fonction ? */
    NUM_COLS       /* nombre de colones               */
  };               /***********************************/


static GtkTreeModel * create_and_fill_save (char *s, int mid)
{
  DIR *d;
  struct dirent *d1;
  /*char * s;*/
  GtkListStore  *store;
  GtkTreeIter    iter;

  /*s = my_concat("",save_dir);*/

  store = gtk_list_store_new (NUM_COLS, G_TYPE_STRING, G_TYPE_UINT);
  

  d = opendir(s);
  if (!d)
    {
      caca(s);
      caca("Ouvrirture le du repertoire pas arrivé");
      return(0);
    }
  
  readdir(d);
  readdir(d);

  while ((d1 = readdir(d)))
    {
      if (is_bbs_lib(d1->d_name) || mid != 2)
	{
	  gtk_list_store_append (store, &iter);
	  gtk_list_store_set (store, &iter,COL_NAME,(*d1).d_name,COL_ID,mid,-1);
	}
    }
  closedir(d);

  return GTK_TREE_MODEL (store);
}

gboolean view_sel_save2 (GtkTreeSelection * selection UNUSED,
			  GtkTreeModel     *model,
			  GtkTreePath      *path,
			  gboolean          path_currently_selected,
			  gpointer userdata UNUSED)
{
  GtkTreeIter iter;
  gchar *name;
  gint id;
  
  if (gtk_tree_model_get_iter(model, &iter, path))
    {
      gtk_tree_model_get(model, &iter, COL_NAME, &name, -1);
      gtk_tree_model_get(model, &iter, COL_ID, &id, -1);
      if (!path_currently_selected)
	{
	  if (id == 1) /*sim*/
	    {
	      if (sim_sel)
		free(sim_sel);
	      sim_sel = my_concat("", name);
	    }
	  else         /*lib*/
	    {
	      if (lib_sel)
		free(lib_sel);
	      lib_sel = my_concat("", name);
	    }
	}
      g_free(name);
    }

  return 1;
}

static GtkWidget * create_view_and_model_save (char *w, int mid)
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
  gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (view),-1,"Sauvegardes",renderer,"text", COL_NAME,NULL);

  model = create_and_fill_save (w, mid);

  gtk_tree_view_set_model (GTK_TREE_VIEW (view), model);

  /* The tree view has acquired its own reference to the
   *  model, so we can drop ours. That way the model will
   *  be freed automatically when the tree view is destroyed */

  /* evenement selection */
  selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(view));
  path = gtk_tree_path_new_from_string("0");
  gtk_tree_selection_select_path (selection,path);
  free(path);
  gtk_tree_selection_set_select_function(selection, view_sel_save2, NULL, NULL);
  gtk_tree_selection_set_mode(selection,GTK_SELECTION_BROWSE);

  g_object_unref (model);
  sc = gtk_scrolled_window_new(NULL,NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sc),GTK_POLICY_AUTOMATIC,GTK_POLICY_AUTOMATIC );
  gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW (sc), view);

  return(sc);
}

/**/
char *get_file(char *title)
{
  static GtkWidget *dialog = 0;
  char *res = 0;
  char *filename;

  if (!dialog)
    dialog = gtk_file_chooser_dialog_new(title,
					 GTK_WINDOW (window),	
					 GTK_FILE_CHOOSER_ACTION_OPEN,
					 GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
					 GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
					 NULL);
  gtk_widget_show(dialog);
  if (gtk_dialog_run(GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
    {
      filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER (dialog));
      res = my_concat("", filename);
      g_free(filename);
    }
  gtk_widget_hide(dialog);
  return res;
}

int evb_li()
{
  char *file, *com;

  file = get_file("Choisir le fichier a importer :");
  if (file)
    {
      com = mega_cct(16, 
		     "sh", " ", chemin, "/../share/bebetes_show/imp_bib.sh", " ", 
		     file, " ", 
		     chemin, "/../lib/", " ", 
		     chemin, "/../share/bebetes_show/lib_data/", " ",
		     "\"", options_gcc, "\"");
      system(com);
      free(com);
      free(file);
    }
  refresh_sv();
  return 1;
}

int evb_le()
{
  char *file, *com, *rep;
  int j, i = 0;

  file = get_file("Choisir le fichier source C :");
  if (file && lib_sel)
    {
      rep = my_concat("", file);
      while (file[i]) 
	i++;
      for (j = i; file[j] != '/'; j--)
	rep[j] = 0;

      com = mega_cct(13,
		     "sh", " ", chemin, "/../share/bebetes_show/exp_bib.sh", " ", 
		     lib_sel, " ", 
		     file, " ", 
		     chemin, "/../share/bebetes_show/lib_data/", " ",
		     rep);
      system(com);
      free(com);
      free(file);
      free(rep);
    }
  return 1;
}

int evb_si()
{
  char *file, *com;

  file = get_file("Choisir le fichier source C :");
  if (file)
    {
      com = mega_cct(9,
		     "sh", " ", chemin, "/../share/bebetes_show/imp_sim.sh", " ", 
		     file, " ",
		     chemin, "/../lib/");

      system(com);
      free(com);
      free(file);
    }
  refresh_sv();
  return 1;
}

int evb_se()
{
  char *com;

  if (sim_sel)
    {
      com = mega_cct(6,
		     "sh", " ", chemin, "/../share/bebetes_show/exp_sim.sh", " ", 
		     sim_sel);
      system(com);
      free(com);
    }
  return 1;
}



/**/


GtkWidget* create_options()
{
  GtkWidget *hb, *vb, *ops, *f1, *f2, *l1, *l2, *tmp, *bb1, *li, *le, *si, *se;
  char *s;

  hb = gtk_hbox_new(1, 10);
  gtk_container_set_border_width (GTK_CONTAINER (hb), 10);
  vb = gtk_vbox_new(0, 20);

  f1 = gtk_frame_new("Simulations");
  f2 = gtk_frame_new("Bibliotheques");
  gtk_container_add(GTK_CONTAINER (vb), f1);
  gtk_container_add(GTK_CONTAINER (vb), f2);
  tmp = gtk_hbox_new(0, 5);
  gtk_container_add(GTK_CONTAINER (f1), tmp);
  f1 = tmp;
  tmp = gtk_hbox_new(0, 5);
  gtk_container_add(GTK_CONTAINER (f2), tmp);
  f2 = tmp;
  gtk_container_set_border_width (GTK_CONTAINER (f1), 10);
  gtk_container_set_border_width (GTK_CONTAINER (f2), 10);

  ops = create_ops();

  gtk_container_add(GTK_CONTAINER (hb), ops);
  gtk_container_add(GTK_CONTAINER (hb), vb);

  s = my_concat("",save_dir);
  l1 = create_view_and_model_save (s, 1);
  free(s);
  s = mega_cct(2,chemin,"/../lib");
  l2 = create_view_and_model_save (s, 2);
  free(s);
  gtk_container_add(GTK_CONTAINER (f1), l1);
  gtk_container_add(GTK_CONTAINER (f2), l2);

  bb1 = gtk_vbox_new(0, 5);
  si = gtk_button_new_with_label("Importer");
  se = gtk_button_new_with_label("Exporter");
  gtk_box_pack_start(GTK_BOX (bb1), si, FALSE, FALSE, 10);  
  gtk_box_pack_start(GTK_BOX (bb1), se, FALSE, FALSE, 10);  
  gtk_box_pack_start(GTK_BOX (f1), bb1, FALSE, FALSE, 10);

  bb1 = gtk_vbox_new(0, 5);
  li = gtk_button_new_with_label("Importer");
  le = gtk_button_new_with_label("Exporter");
  gtk_box_pack_start(GTK_BOX (bb1), li, FALSE, FALSE, 10);  
  gtk_box_pack_start(GTK_BOX (bb1), le, FALSE, FALSE, 10);  
  gtk_box_pack_start(GTK_BOX (f2), bb1, FALSE, FALSE, 10);

  g_signal_connect (si, "clicked", G_CALLBACK (evb_si), NULL);
  g_signal_connect (li, "clicked", G_CALLBACK (evb_li), NULL);
  g_signal_connect (se, "clicked", G_CALLBACK (evb_se), NULL);
  g_signal_connect (le, "clicked", G_CALLBACK (evb_le), NULL);
  
  if (!fbox11)
    fbox11 = gtk_hbox_new(0, 0);
  onglet_op = hb;
  gtk_container_add(GTK_CONTAINER (fbox11), hb);
  return fbox11;
}

