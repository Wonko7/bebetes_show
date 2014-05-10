#include <stdio.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#define  _GNU_SOURCE
#include <dlfcn.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

#include "lib_bbs_behavior.h"
#include "loader.h"
#include "camera.h"
#include "divers.h"
#include "map.h"
#include "taskbrowser.h"
#include "f.h"
#include "map_editor.h"
#include "gtk+extra/gtkplot.h"
#include "gtk+extra/gtkplotdata.h"
#include "gtk+extra/gtkplotbar.h"
#include "gtk+extra/gtkplotcanvas.h"
#include "gtk+extra/gtkplotprint.h"
#include "stat.h"
#include "propag.h"

enum
  {                /***********************************/
    COL_NAME = 0,  /* nom du champ                    */
    COL_ID,        /* id du champ... p sur fonction ? */
    NUM_COLS       /* nombre de colones               */
  };               /***********************************/

extern t_agent liste_agent;
extern t_objet liste_objet;


void *(*user_ta)(t_agent agent);
void *(*user_to)(t_objet objet);
void *(*user_cycle_ta)(t_agent agent);
void *(*user_cycle_to)(t_objet objet);
void *(*user_cycle_gena)(t_agent agent);
void *(*user_cycle_geno)(t_objet objet);
void *(*stat_ag)(t_agent agent);
void *(*stat_obj)(t_objet objet);
void *(*stat_map)(t_case * kase);


/****** THINGS TO LOAD *********************/

extern void * lib;
extern behavior *stimuli;
extern agent *agents;
extern operator *operators;
extern primitive *primitives;
extern argument *arguments;
extern objet *objets;
extern ccase *cases;
extern iaction *iactions;
extern graphe *stats;
extern int nb_sti, nb_ag, nb_prim, nb_op, nb_arg, nb_case, nb_obj, nb_iaction, nb_stats;
extern float cycle;
extern unsigned long tps_tot;
extern float tps_tot_ac;
extern int cy_depr;


extern void *(*free_op)();
extern void *(*free_p)();
extern void *(*free_ag)();
extern void *(*free_objs)();
extern void *(*free_sti)();
extern void *(*free_arg)();
extern void *(*free_case)();
extern void *(*free_iaction)();

extern void *(*remove_task)();

/*******************************************/

extern t_texture tex_index;
extern int ground;
extern t_list_mod list_mod;
extern t_map carte;
extern GtkWidget *fs_t1;
extern GtkWidget *note_fucking_book;

/*******************************************/

extern int options_w_load;
extern int options_w_suppr;

/*******************************************/

extern struct camera cam;
extern int realized;
extern char *chemin;
char *save_dir = 0;
char *save = 0;
/**/
int id_sa;
char * selected_sa=0;
/**/
GtkWidget *t_inp, *fakebox, *list;
/**/
int selected_new_lib = 0;
char *selected_lib = 0;
char *default_lib = "lib_bbs_behavior.so";
/**/
extern GtkWidget *fbox22, *fbox11, *onglet_sv, *onglet_op;

/**************************************************
 *  ECRIRE BITE A BITE...  ou pas en fait... ******
 **************************************************/


int is_bbs_lib(char *s)
{
  char *e = "_behavior.so";
  int i, j = 12;

  for (i = 0; s[i] != 0; i++)
    ;


  /*for (j = i; s[j] == e[12 - i + j] && i - j < 13; j--)*/
  while (i >= 0 && j >= 0)
    {
      if (s[i] != e[j])
	return 0;
      i--;
      j--;
    }

  return 1;
}


/**************************************************
 *  OBJ 3D  ***************************************
 **************************************************/


int reload_obj3D()
{
  int i;

  for(i=0; i<nb_ag; i++)
    agents[i].mod = create_3Dobj(agents[i].nom_mod);

  for(i=0; i<nb_obj; i++)
    objets[i].mod = create_3Dobj(objets[i].nom_mod);

  for (i=0; i < nb_case;i++)
    if (cases[i].model)
      cases[i].objet->model = create_3Dobj(cases[i].model);

  return(1);
}


/**************************************************
 *  TEXTURES  *************************************
 **************************************************/

GLuint * Load_image(char * name)
{
  GLuint * texture_id;
  FILE * f;
  char *t3,*t7;

  t3 = mega_cct(5,save_dir,"/",save,"/textures/",name);
  t7 = mega_cct(3,chemin,"/../share/bebetes_show/textures/",name);

  if ((f = fopen(t3,"r")))
    {
      fclose(f);
      texture_id = Load_Texture(t3);
    }
  else
    texture_id = Load_Texture(t7);

  free(t3);
  free(t7);

  return(texture_id);
}


/* int Load_icone(char * where, char * name) */
/* DEPRECATED, CONNARD
int Load_icone(char * name)
{
  int texture_id, l;
  FILE * f;
  char * t;

  t = my_concat("",name);
  //  l = strlen(t);
  // t[l-3] = 'b';
  // t[l-2] = 'm';
  // t[l-1] = 'p';
  texture_id = Load_Texture(t);
  free(t);
  return(texture_id);
}
bon elle je la laisse par nostalgie...*/

/**************************************************
 *  DYN LIBS  *************************************
 **************************************************/


void free_libs()
{
  if (stimuli)
    free_sti(stimuli);
  if (operators)
    free_op(operators,nb_op);
  if (primitives)
    free_p(primitives,nb_prim);
  if (agents)
    free_ag(agents,nb_ag);
  if (objets)
    free_objs(objets,nb_obj);
  if (arguments)
    free_arg(arguments,nb_arg);
  if (cases)
    free_case(cases,nb_case);
  if (iactions)
    free_iaction(iactions,nb_iaction);
  if (stats)
    free_stats();

  if (lib)
    dlclose(lib);
}

void load_dlib(char * where)
{
  char * tmp_str;
  void * (*init_stimuli)();
  void * (*init_agents)();
  void * (*init_objets)();
  void * (*init_operators)();
  void * (*init_primitives)();
  void * (*init_arg)();
  void * (*init_cases)();
  void * (*init_iaction)();
  void * (*init_stats)();

  /************************************
   *  reload the dl  ******************
   ************************************/

  free_libs();
  lib = dlopen(where,RTLD_NOW);

  if ((tmp_str = dlerror()))
    puts(tmp_str);

  /************************************
   *  reload our stuff  ***************
   ************************************/

  *(void **) (&init_stimuli) = dlsym(lib,"init_stimuli");
  *(void **) (&init_agents) = dlsym(lib,"init_agents");
  *(void **) (&init_objets) = dlsym(lib,"init_objets");
  *(void **) (&init_operators) = dlsym(lib,"init_operators");
  *(void **) (&init_primitives) = dlsym(lib,"init_primitives");
  *(void **) (&init_cases) = dlsym(lib,"init_cases");
  *(void **) (&init_arg) = dlsym(lib,"init_arg");
  *(void **) (&init_iaction) = dlsym(lib,"init_iaction");
  *(void **) (&init_stats) = dlsym(lib,"init_stats");

  *(void **) (&user_ta) = dlsym(lib,"user_ta");
  *(void **) (&user_to) = dlsym(lib,"user_to");
  *(void **) (&user_cycle_ta) = dlsym(lib,"user_cycle_ta");
  *(void **) (&user_cycle_to) = dlsym(lib,"user_cycle_to");
  *(void **) (&user_cycle_gena) = dlsym(lib,"user_cycle_gena");
  *(void **) (&user_cycle_geno) = dlsym(lib,"user_cycle_geno");
  *(void **) (&stat_ag) = dlsym(lib,"stat_ag");
  *(void **) (&stat_obj) = dlsym(lib,"stat_obj");
  *(void **) (&stat_map) = dlsym(lib,"stat_map");

  *(void **) (&free_op) = dlsym(lib,"free_op");
  *(void **) (&free_ag) = dlsym(lib,"free_ag");
  *(void **) (&free_objs) = dlsym(lib,"free_objs");
  *(void **) (&free_p) = dlsym(lib,"free_p");
  *(void **) (&free_sti) = dlsym(lib,"free_sti");
  *(void **) (&free_arg) = dlsym(lib,"free_arg");
  *(void **) (&free_case) = dlsym(lib,"free_case");
  *(void **) (&remove_task) = dlsym(lib,"remove_task2");
  *(void **) (&free_iaction) = dlsym(lib,"free_iaction");

  init_stimuli(&stimuli,&nb_sti,&cycle,&cy_depr);
  init_agents(&agents,&nb_ag);
  init_objets(&objets,&nb_obj);
  init_operators(&operators,&nb_op);
  init_primitives(&primitives,&nb_prim);
  init_cases(&cases,&nb_case);
  init_arg(&arguments,stimuli,agents,objets,&nb_arg,nb_ag,nb_sti,nb_obj);
  init_iaction(&iactions,&nb_iaction);
  init_stats(&stats,&nb_stats);
  init_stats_var();

  if ((tmp_str = dlerror()))
    puts(tmp_str);
}

/**************************************************
 *  SAUVEGARDE DE CETTE SALOPE DE MAP  ************
 **************************************************/

int write_map(char * dsave)
{
  FILE *s;
  char *t1;
  int i,j;
  t_signal si;

  start_locale();

  t1 = my_concat(dsave,"/map/map.bbs");
  if (!(s  = fopen(t1,"w")))
    {
      caca(t1);
      caca("l'ouvrirure du fichier nix marcher");
      free(t1);
      end_locale();
      return(0);
    }

  fprintf(s,"X:%i\n",carte.x_max);
  fprintf(s,"Y:%i\n",carte.y_max);
  fprintf(s,"tps: tot:%lu ac:%f\n", tps_tot, tps_tot_ac);

  for (i=0; i<carte.x_max; i++)
    for (j=0; j<carte.y_max; j++)
      {
	/*case*/
	fprintf(s,"etat:%i type:%i\n",carte.tab[i][j].etat,carte.tab[i][j].type);
	/*l-chainee signaux*/
	si = carte.tab[i][j].signaux;
	while (si)
	  {
	    fprintf(s,"type:%i force:%f date:%lu next:%i\n",si->type,si->force,si->date,1);
	    si = si->next;
	  }
	fprintf(s,"type:%i force:%f date:%lu next:%i\n",0,0.0,0L,0);
      }

  free(t1);
  fclose(s);
  end_locale();

  return(1);
}

int read_map(char * dsave)
{
  FILE *s;
  char *t1;
  int i,j,x,y,next;
  t_signal *sp;
  struct s_signal sii;

  start_locale();

  t1 = my_concat(dsave,"/map/map.bbs");
  if (!(s  = fopen(t1,"r")))
    {
      free(t1);
      end_locale();
      create_map(0,0);
      tps_tot = 0;
      tps_tot_ac = 0.0;
      return(0);
    }

  fscanf(s,"X:%i\n",&x);
  fscanf(s,"Y:%i\n",&y);
  fscanf(s,"tps: tot:%lu ac:%f\n", &tps_tot, &tps_tot_ac);

  carte = create_map(x,y);

  for (i=0; i<x; i++)
    for (j=0; j<y; j++)
      {
	/*case*/
	fscanf(s,"etat:%i type:%i\n",&(carte.tab[i][j].etat),&(carte.tab[i][j].type));
	/*l-chainee signaux*/
	fscanf(s,"type:%i force:%f date:%lu next:%i\n",&(sii.type),&(sii.force),&(sii.date),&(next));
	sp = &(carte.tab[i][j].signaux);
	*sp = 0;

	while (next == 1)
	  {
	    *sp = malloc(sizeof (struct s_signal));
	    **sp = sii;
	    (*sp)->next = 0;
	    sp = &((*sp)->next);
	    fscanf(s,"type:%i force:%f date:%lu next:%i\n",&(sii.type),&(sii.force),&(sii.date),&(next));
	  }

      }

  free(t1);
  fclose(s);
  end_locale();

  return(1);
}

/**************************************************
 *  SAUVEGARDE OBJETS  ****************************
 **************************************************/


static inline t_objet mk_obj(FILE *s, int is_c)
{
  t_objet po;
  struct s_objet o;

  fscanf(s,"cx:%i cy:%i x:%f y:%f z:%f rx:%f ry:%f rz:%f ratio:%f val:%f step:%i speed:%i type:%i tps:%lg\n", &(o.case_x), &(o.case_y), &(o.x), &(o.y), &(o.z), &(o.rx), &(o.ry), &(o.rz), &(o.ratio), &(o.valeur), &(o.step), &(o.speed), &(o.type),&(o.tps));
  po = creer_objet(objets[o.type].nom_mod, o.case_x, o.case_y, o.rx, o.ry, o.rz, o.ratio, o.type,!is_c);
  po->z = o.z;
  po->y = o.y;
  po->x = o.x;
  po->valeur = o.valeur;
  po->step = o.step;
  po->speed = o.speed;
  po->tps = o.tps;

  return(po);
}

static inline void wr_obj(FILE *s, t_objet o)
{
  fprintf(s,"cx:%i cy:%i x:%f y:%f z:%f rx:%f ry:%f rz:%f ratio:%f val:%f step:%i speed:%i type:%i tps:%g\n",o->case_x,o->case_y,o->x,o->y,o->z,o->rx,o->ry,o->rz,o->ratio,o->valeur,o->step,o->speed,o->type,o->tps);
}

int write_obj(char * dsave)
{
  FILE *s;
  char *t1;
  t_objet o;
  int j;

  start_locale();

  t1 = my_concat(dsave,"/objets/objets.bbs");
  if (!(s  = fopen(t1,"w")))
    {
      caca(t1);
      caca("l'ouvrirure du fichier nix marcher");
      free(t1);
      end_locale();
      return(0);
    }

  o = liste_objet;
  j = 0;
  while (o)
    {
      if (o->is_on_map)
	j++;
      o = o->next;
    }

  o = liste_objet;
  fprintf(s,"nb_obj:%i\n",j);

  while (o)
    {
      if (o->is_on_map)
	wr_obj(s,o);
      o = o->next;
    }

  free(t1);
  fclose(s);
  end_locale();

  return(1);
}

int read_obj(char * dsave)
{
  FILE *s;
  char *t1;
  int i,j;

  start_locale();

  t1 = my_concat(dsave,"/objets/objets.bbs");
  if (!(s  = fopen(t1,"r")))
    {
      free(t1);
      end_locale();
      return(0);
    }

  fscanf(s,"nb_obj:%i\n",&j);

  for (i=0; i<j; i++)
    mk_obj(s,0);

  free(t1);
  fclose(s);
  end_locale();

  return(1);
}

/**************************************************
 *  SAUVEGARDE AGENTS  ****************************
 **************************************************/

t_agent mk_ag(FILE *s, int is_c)
{
  struct s_agent an;
  t_agent an2;
  int test, ii;

  fscanf(s,"x:%f y:%f z:%f xgoto:%f ygoto:%f zgoto:%f rx:%f ry:%f rz:%f cx:%i cy:%i ci:%i ratio:%f step:%i tps:%f speed:%f naa:%f na:%f age:%i en_cours:%i tache:%i tps:%lg type:%i\n",&(an.x),&(an.y),&(an.z),&(an.xgoto),&(an.ygoto),&(an.zgoto),&(an.rx),&(an.ry),&(an.rz),&(an.case_x),&(an.case_y),&(an.case_index),&(an.ratio),&(an.step),&(an.ac_tps),&(an.speed),&(an.naa),&(an.na),&(an.age),&(an.en_cours),&(an.tache),&(an.tps),&(an.type));

  an2 = creer_agent(agents[an.type].nom_mod,an.case_x,an.case_y,an.rx,an.ry,an.rz,an.ratio,an.type,!is_c);
  an2->x = an.x;
  an2->y = an.y;
  an2->z = an.z;
  an2->xgoto = an.xgoto;
  an2->ygoto = an.ygoto;
  an2->zgoto = an.zgoto;
  an2->case_x = an.case_x;
  an2->case_y = an.case_y;
  an2->ratio = agents[an.type].ratio;
  an2->step = an.step;
  an2->ac_tps = an.ac_tps;
  an2->speed = an.speed;
  an2->naa = an.naa;
  an2->na = an.na;
  an2->age = an.age;
  an2->en_cours = an.en_cours;
  an2->tache = an.tache;
  an2->tps = an.tps;
  an2->type = an.type;

  for (ii=0; ii<nb_sti; ii++)
    fscanf(s,"p:%f s:%f f:%f\n",&(an2->stimuli[ii].poids),&(an2->stimuli[ii].seuil),&(an2->stimuli[ii].force));

  fscanf(s,"is_cad_ag:%i\n",&test);

  if (test == 1)
    {
      an2->caddie = mk_ag(s,1);
      an2->is_caddie_ag = 1;
    }
  else
    if (!test)
      {
	an2->caddie = mk_obj(s,1);
	an2->is_caddie_ag = 0;
      }
  return(an2);
}

void wr_ag(FILE *s, t_agent an)
{
  int ii;

  /* données ag */
  fprintf(s,"x:%f y:%f z:%f xgoto:%f ygoto:%f zgoto:%f rx:%f ry:%f rz:%f cx:%i cy:%i ci:%i ratio:%f step:%i tps:%f speed:%f naa:%f na:%f age:%i en_cours:%i tache:%i tps:%g type:%i\n",an->x,an->y,an->z,an->xgoto,an->ygoto,an->zgoto,an->rx,an->ry,an->rz,an->case_x,an->case_y,an->case_index,an->ratio,an->step,an->ac_tps,an->speed,an->naa,an->na,an->age,an->en_cours,an->tache,an->tps,an->type);

  /* tab sti */
  for (ii=0; ii<nb_sti; ii++)
    fprintf(s,"p:%f s:%f f:%f\n",an->stimuli[ii].poids,an->stimuli[ii].seuil,an->stimuli[ii].force);

  /* caddie */
  if (an->caddie)
    {
      fprintf(s,"is_cad_ag:%i\n",an->is_caddie_ag);
      if (an->is_caddie_ag)
	wr_ag(s,(t_agent) an->caddie);
      else
	wr_obj(s,(t_objet) an->caddie);
    }
  else
    fprintf(s,"is_cad_ag:%i\n",69);
}

int write_ag(char * dsave)
{
  FILE *s;
  char *t1;
  int j;
  t_agent an;

  start_locale();

  t1 = my_concat(dsave,"/agents/agents.bbs");
  if (!(s  = fopen(t1,"w")))
    {
      caca(t1);
      caca("l'ouvrirure du fichier nix marcher");
      free(t1);
      end_locale();
      return(0);
    }

  an = liste_agent;
  j = 0;
  while (an)
    {
      if (!an->is_caddie)
	j++;
      an = an->next;
    }

  an = liste_agent;
  fprintf(s,"nb_ag:%i\n",j);
  fprintf(s,"nb_sti:%i\n",nb_sti);

  while (an)
    {
      if (!an->is_caddie)
	wr_ag(s,an);
      an = an->next;
    }

  free(t1);
  fclose(s);
  end_locale();

  return(1);
}

int read_ag(char * dsave)
{
  FILE *s;
  char *t1;
  int i,sti,tot;
  t_agent an;


  start_locale();

  an = malloc(sizeof (struct s_agent));

  t1 = my_concat(dsave,"/agents/agents.bbs");
  if (!(s  = fopen(t1,"r")))
    {
      free(t1);
      free(an);
      end_locale();
      return(0);
    }


  fscanf(s,"nb_ag:%i\n",&tot);
  fscanf(s,"nb_sti:%i\n",&sti);

  if (sti != nb_sti)
    {
      caca("Conflit de versions entre le tableau de stimuli de la library et celui enregistré dans le fichier");
      gtk_caca("Conflit de versions entre le tableau de stimuli de la library et celui enregistre dans le fichier");
    }

  for (i=0; i<tot; i++)
    mk_ag(s,0);

  free(an);
  free(t1);
  fclose(s);
  end_locale();

  return(1);
}


/**************************************************
 *  LOAD/SAVE BEHAVIOR  ***************************
 **************************************************/

int write_sti(char * save)
{
  char *t1;
  FILE *s;
  int i;
  brick b;

  start_locale();

  t1 = my_concat(save,"/behavior/behavior.bbs");
  if (!(s  = fopen(t1,"w+")))
    {
      caca(t1);
      caca("l'ouvrirure du fichier nix marcher");
      free(t1);
      end_locale();
      return(0);
    }

  fprintf(s,"%i\n",nb_sti);

  for (i=0; i<nb_sti; i++)
    {
      fprintf(s,"w:%f i:%f t:%f c:%f\n",stimuli[i].task.weight,stimuli[i].task.increment,stimuli[i].task.threshold,stimuli[i].task.weight_change);

      fprintf(s,"#interruption:\n");
      if (stimuli[i].task.interruption)
	fprintf(s,"p:%i op:%i a:%i\n",stimuli[i].task.interruption->p,stimuli[i].task.interruption->op,stimuli[i].task.interruption->arg);
      else
	fprintf(s,"p:%i op:%i a:%i\n",-1,-1,-1);

      fprintf(s,"#brick:\n");
      b = stimuli[i].task.brick;
      if (!b)
	fprintf(s,"p:%i op:%i a:%i n:%i\n",-1,-1,-1,0);
      while (b)
	{
	  fprintf(s,"p:%i op:%i a:%i n:%i\n",(*b).p,(*b).op,(*b).arg,(*b).next ? 1 : 0);
	  b = (*b).next;
	}
    }

  fclose(s);
  free(t1);
  end_locale();
  return(1);
}

brick create_brick(int p, int op, int a)
{
  brick b;

 if (p>nb_prim || op>nb_op || a>nb_arg)
    {
      caca("Conflit de versions entre le tableau de stimuli de la library et celui enregistré dans le fichier");
      gtk_caca("Conflit de versions entre le tableau de stimuli de la library et celui enregistre dans le fichier");
    }

  b = malloc(sizeof (struct n_brick));
  (*b).op = op;
  (*b).p = p;
  (*b).icone_op = operators[op].icone_op;
  (*b).icone_p = primitives[p].icone_p;
  (*b).name_op = operators[op].name_op;
  (*b).name_p = primitives[p].name_p;
  (*b).type_op = operators[op].type_op;
  (*b).arg = a;
  (*b).next=0;

  return(b);
}

/* WARNING : il faut que stimuli soit freeé. */
int read_sti(char * save)
{
  char *t1;
  FILE *s;
  int j,im,p,op,n,a;
  float w,i,t,wc;
  brick *b;

  start_locale();

  t1 = my_concat(save,"/behavior/behavior.bbs");
  if (!(s  = fopen(t1,"r")))
    {
      free(t1);
      end_locale();
      return(0);
    }

  fscanf(s,"%i\n",&im);

  if (im != nb_sti)
    {
      gtk_caca("Conflit de versions entre le tableau de stimuli de la library et celui enregistre dans le fichier");
      caca("Conflit de versions entre le tableau de stimuli de la library et celui enregistré dans le fichier");
      caca(t1);
      printf("Nombres d'elements : Lib:%i Fich:%i\n",im,nb_sti);
      im = im > nb_sti ? nb_sti : im;
    }

  for (j=0; j<im; j++)
    {

      fscanf(s,"w:%f i:%f t:%f c:%f\n",&w,&i,&t,&wc);
      stimuli[j].task.weight = w;
      stimuli[j].task.increment = i;
      stimuli[j].task.threshold = t;
      stimuli[j].task.weight_change = wc;

      fscanf(s,"#interruption:\n");
      fscanf(s,"p:%i op:%i a:%i\n",&p,&op,&a);

      if (p == -1)
	stimuli[j].task.interruption = 0;
      else
	stimuli[j].task.interruption = create_brick(p,op,a);

      fscanf(s,"#brick:\n");
      stimuli[j].task.brick = 0;
      b = &(stimuli[j].task.brick);
      do
	{
	  n=0;
	  fscanf(s,"p:%i op:%i a:%i n:%i\n",&p,&op,&a,&n);
	  if (p != -1)
	    {
	      *b = create_brick(p,op,a);
	      b = &(*(*b)).next;
	    }
	}
      while (n);
    }

  fclose(s);
  free(t1);
  end_locale();
  return(1);
}

/**************************************************
 *  SAUVEGARDE DE LA CAMERA  **********************
 **************************************************/

int write_cam(char * dsave)
{
  FILE *s;
  char *t1;

  start_locale();

  t1 = my_concat(dsave,"/cam.bbs");
  if (!(s  = fopen(t1,"w")))
    {
      caca(t1);
      caca("l'ouvrirure du fichier nix marcher");
      free(t1);
      end_locale();
      return(0);
    }

  fprintf(s,"zoom:%f\n",cam.zoom);
  fprintf(s,"alpha:%f\n",cam.alpha);
  fprintf(s,"beta:%f\n",cam.beta);
  fprintf(s,"x:%f\n",cam.x);
  fprintf(s,"y:%f\n",cam.y);
  fprintf(s,"xgoto:%f\n",cam.xgoto);
  fprintf(s,"ygoto:%f\n",cam.ygoto);

  free(t1);
  fclose(s);
  end_locale();

  return(1);
}

int read_cam(char * dsave)
{
  FILE *s;
  char *t1;

  start_locale();

  t1 = my_concat(dsave,"/cam.bbs");
  if (!(s  = fopen(t1,"r")))
    {
      free(t1);
      end_locale();
      init_cam();
      return(0);
    }

  fscanf(s,"zoom:%f\n",&cam.zoom);
  fscanf(s,"alpha:%f\n",&cam.alpha);
  fscanf(s,"beta:%f\n",&cam.beta);
  fscanf(s,"x:%f\n",&cam.x);
  fscanf(s,"y:%f\n",&cam.y);
  fscanf(s,"xgoto:%f\n",&cam.xgoto);
  fscanf(s,"ygoto:%f\n",&cam.ygoto);

  cam.follow_agent = 0;
  cam.reached = 0;

  free(t1);
  fclose(s);
  end_locale();

  return(1);
}

/**************************************************
 *  GTK STUFF  ************************************
 **************************************************/


static GtkTreeModel * create_and_fill_save ()
{
  int i=5;
  DIR *d;
  struct dirent *d1;
  char * s;
  GtkListStore  *store;
  GtkTreeIter    iter;

  s = my_concat("",save_dir);

  store = gtk_list_store_new (NUM_COLS, G_TYPE_STRING, G_TYPE_UINT);


  d = opendir(s);
  if (!d)
    {
      caca(s);
      free(s);
      caca("Ouvrirture le du repertoire pas arrivé");
      return(0);
    }

  while ((d1 = readdir(d)))
  {
	  if (!strcmp(d1->d_name, "..") || !strcmp(d1->d_name, "."))
		  continue;

	  gtk_list_store_append (store, &iter);
	  gtk_list_store_set (store, &iter,COL_NAME,(*d1).d_name,COL_ID,i,-1);
  }

  closedir(d);
  free(s);

  return GTK_TREE_MODEL (store);
}

gboolean view_sel_save (GtkTreeSelection * selection UNUSED,
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
	  free(selected_sa);
	  selected_sa = my_concat("",name);
	}
      g_free(name);
    }

  return 1;
}

static GtkWidget * create_view_and_model_save (void)
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

  model = create_and_fill_save ();

  gtk_tree_view_set_model (GTK_TREE_VIEW (view), model);

  /* The tree view has acquired its own reference to the
   *  model, so we can drop ours. That way the model will
   *  be freed automatically when the tree view is destroyed */

  /* evenement selection */
  selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(view));
  path = gtk_tree_path_new_from_string("0");
  gtk_tree_selection_select_path (selection,path);
  //free(path);
  gtk_tree_path_free(path);
  gtk_tree_selection_set_select_function(selection, view_sel_save, NULL, NULL);
  gtk_tree_selection_set_mode(selection,GTK_SELECTION_BROWSE);

  g_object_unref (model);
  sc = gtk_scrolled_window_new(NULL,NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sc),GTK_POLICY_AUTOMATIC,GTK_POLICY_AUTOMATIC );
  gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW (sc), view);

  return(sc);
}


void redraw_list()
{
  static int en_cours = 0;

  if (!en_cours)
    {
      en_cours = 1;
      gtk_widget_destroy(list);
      list = create_view_and_model_save();
      gtk_container_add (GTK_CONTAINER (fakebox), list);
      gtk_widget_show_all(fakebox);
      en_cours = 0;
    }
}

int loaddd()
{
  char *t2,*t3,*t4;
  DIR *dir;
  struct dirent *d1;

  fbox11 = fbox22 = onglet_sv = onglet_op = 0;
  free_textures();
  free_list_mod();
  free_agents();
  free_objets();
  liste_agent = 0;
  free_map();
  free_tab_stim_int();

  t2 = mega_cct(4,save_dir,"/",save,"/lib");
  t3 = my_concat(chemin,"/../lib/lib_bbs_behavior.so");

  dir = opendir(t2);
  if (!dir)
    {
      caca(t2);
      caca("loaddd : Ouvrirture le du repertoire pas arrivé");
      free(t2);
      free(t3);
      return(0);
    }

   d1 = readdir(dir);
   while (d1 && !is_bbs_lib(d1->d_name))
	   d1 = readdir(dir);
   if (d1)
   {
	   t4 = mega_cct(3,t2,"/",d1->d_name);
	   load_dlib(t4);
	   free(t4);
   }
   else
	   load_dlib(t3);
   closedir(dir);

  t4 = mega_cct(3,save_dir,"/",save);

  reload_obj3D();

  get_tps();
  read_map(t4);
  read_sti(t4);
  read_ag(t4);
  read_obj(t4);
  read_cam(t4);
  charger_stats();
  init_tab_stim_int();

  free(t2);
  free(t3);
  free(t4);

  realized = 0;
  gtk_widget_destroy(fs_t1);
  fs_t1 = 0;
  init_gtk_nfs();
  refresh_map();
  gtk_notebook_set_current_page(GTK_NOTEBOOK (note_fucking_book),1);

  return(1);
}


/* buts */
int evb_new ()
{
  char *t1,*t3,*src,*dest;
  const gchar *name;
  int i;
  char * c[7];


  c[0] = "textures";
  c[1] = "lib";
  c[2] = "obj_3D";
  c[3] = "behavior";
  c[4] = "agents";
  c[5] = "map";
  c[6] = "objets";
  c[7] = "statistiques";
  i = 7;

  name = gtk_entry_get_text(GTK_ENTRY (t_inp));

  if (!name[0])
    {
      gtk_caca("Il faut donner un nom a la sauvegarde");
      return(1);
    }

  t3 = mega_cct(3,save_dir,"/",name);

  if (mkdir(t3,00755))
    {
      gtk_caca("Le dossier existe deja, ou vous n'avez pas les droits d'ecritures");
      free(t3);
      return(1);
    }
  for (; i>=0; i--)
    {
      t1 = mega_cct(3,t3,"/",c[i]);
      mkdir(t1,00755);
      free(t1);
    }

  redraw_list();
  free(t3);

  if (selected_lib && strcmp(selected_lib,default_lib))
    {
      src = mega_cct(3,chemin,"/../lib/",selected_lib);
      dest = mega_cct(5,save_dir,"/",name,"/lib/",selected_lib);
      cp_file(src,dest);
      free(src);
      free(dest);

      src = mega_cct(3,chemin,"/../share/bebetes_show/lib_data/",selected_lib);
      dest = mega_cct(3,save_dir,"/",name);
      cp_rec(src,dest);

      free(src);
      free(dest);
    }

  if (save)
    free(save);
  save = my_concat2(name,"");

  /* fuck shit cunt ass suck */

  loaddd();

  draw_task();
  return(1);
}

int evb_load ()
{
  char *t;

  t = "Attention, si vous n'avez pas enregistre la simulation en cours, vous perdez les modifications que vous y avez apporte. Voulez vous continuer ?";

  if (!selected_sa[0])
    {
      gtk_caca("Aucune sauvegarde selectionne.");
      return(1);
    }

  if (options_w_load && !gtk_caca_q(t))
    return(1);

  save = selected_sa;
  loaddd();

  return(1);
}

int evb_save ()
{
  char *t4;

  if (!save)
    {
      gtk_caca("Aucune sauvegarde de chargee.");
      return(1);
    }

  t4 = mega_cct(3,save_dir,"/",save);

  write_map(t4);
  write_sti(t4);
  write_ag(t4);
  write_obj(t4);
  write_cam(t4);
  save_stats();

  free(t4);

  draw_task();
  return(1);
}

int evb_suppr ()
{
  char *t,*t2;

  if (!selected_sa || !selected_sa[0])
    {
      gtk_caca("Aucune simulation de selectionne !");
      return (1);
    }

  if (save && !strcmp(selected_sa,save))
    {
      gtk_caca("On ne peut pas supprimer une simulation qui est en cours");
      return (1);
    }

  t = mega_cct(3, "Etes vous sur de vouloir supprimer la simulation ", selected_sa, " ?");
  t2 = mega_cct(3,save_dir,"/",selected_sa);

  if (!options_w_suppr || gtk_caca_q(t))
    supprec(t2);

  free(t);
  free(t2);
  redraw_list();

  return(1);
}

int save_auto ()
{
  char *t4;

  if (!save)
    return(1);

  t4 = mega_cct(3,save_dir,"/",save);

  write_map(t4);
  write_sti(t4);
  write_ag(t4);
  write_obj(t4);
  write_cam(t4);
  save_stats();

  free(t4);

  draw_task();
  return(1);
}

void set_selected_lib(GtkWidget *combo)
{
  selected_lib = gtk_combo_box_get_active_text(GTK_COMBO_BOX(combo));
}

GtkWidget *create_sel_lib ()
{
	int j = 0, i = 0;
	DIR *d;
	struct dirent *d1;
	char * s;
	GtkWidget *cb;

	s = my_concat(chemin,"/../lib");
	cb = gtk_combo_box_new_text ();

	d = opendir(s);
	if (!d)
	{
		caca(s);
		caca("Ouvrirture le du repertoire pas arrivé");
		return(0);
	}

	while ((d1 = readdir(d)))
	{
		if (!strcmp(d1->d_name,default_lib))
			j = i;
		if (is_bbs_lib(d1->d_name))
		{
			gtk_combo_box_append_text(GTK_COMBO_BOX(cb),d1->d_name);
			i++;
		}
	}

	gtk_combo_box_set_active (GTK_COMBO_BOX(cb),j);
	g_signal_connect (cb, "changed", G_CALLBACK (set_selected_lib), NULL);

	free(s);
	closedir(d);

	return(cb);
}


GtkWidget * create_loading()
{
  /*                           logo;  tout; */
  GtkWidget *h_box_principale, *v_bd, *v_bg, *h_bnew, *h_bce,
    *b_load, *b_save, *b_new, *b_suppr,
    *l_new, *logo, *lib_combo;
  char * tmp_str;

  selected_sa = malloc(sizeof (char));

  /* init objets : */
  h_box_principale = gtk_hbox_new(1,2);
  gtk_container_set_border_width (GTK_CONTAINER (h_box_principale), 10);
  v_bg = gtk_vbox_new(0,2);
  v_bd = gtk_vbox_new(0,2);
  h_bnew = gtk_hbox_new(0,2);
  h_bce = gtk_hbox_new(0,2);
  fakebox = gtk_hbox_new(0,0);
  list = create_view_and_model_save();
  gtk_container_add (GTK_CONTAINER (fakebox), list);
  lib_combo = create_sel_lib();


  tmp_str = my_concat(chemin,"/../share/bebetes_show/textures/logo/logo.jpg");
  logo = gtk_image_new_from_file(tmp_str);
  free(tmp_str);

  /*button*/
  b_new = gtk_button_new_with_label("Nouvelle Simulation");
  g_signal_connect (b_new, "clicked",G_CALLBACK (evb_new), NULL);
  b_save = gtk_button_new_with_label("Enregistrer");
  g_signal_connect (b_save, "clicked",G_CALLBACK (evb_save), NULL);
  b_load = gtk_button_new_with_label("Charger");
  g_signal_connect (b_load, "clicked",G_CALLBACK (evb_load), NULL);
  b_suppr = gtk_button_new_with_label("Supprimer");
  g_signal_connect (b_suppr, "clicked",G_CALLBACK (evb_suppr), NULL);

  l_new = gtk_label_new("Nom :");
  t_inp = gtk_entry_new();

  /* cacaté */
  gtk_container_add (GTK_CONTAINER (h_box_principale), v_bg);
  gtk_container_add (GTK_CONTAINER (h_box_principale), v_bd);

  /* a droite */
  gtk_container_add (GTK_CONTAINER (v_bd), logo);

  /* a gauche */
  gtk_box_pack_start(GTK_BOX (v_bg),h_bnew,FALSE,FALSE,10);
  gtk_box_pack_start(GTK_BOX (v_bg),fakebox,1,1,10);
  gtk_box_pack_start(GTK_BOX (v_bg),h_bce,FALSE,FALSE,10);

  /* en haut */
  gtk_container_add (GTK_CONTAINER (h_bnew), l_new);
  gtk_container_add (GTK_CONTAINER (h_bnew), t_inp);
  gtk_container_add (GTK_CONTAINER (h_bnew), lib_combo);
  gtk_container_add (GTK_CONTAINER (h_bnew), b_new);

  /* en bas...  */
  gtk_container_add (GTK_CONTAINER (h_bce), b_save);
  gtk_container_add (GTK_CONTAINER (h_bce), b_load);
  gtk_container_add (GTK_CONTAINER (h_bce), b_suppr);

  if (!fbox22)
    fbox22 = gtk_hbox_new(0, 0);
  onglet_sv = h_box_principale;
  gtk_container_add(GTK_CONTAINER (fbox22), h_box_principale);
  return fbox22;
}
