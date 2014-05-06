#include <stdio.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include <math.h>
#include "lib_bbs_behavior.h"
#include "loader.h"
#include "camera.h"

#include "loading.h"
#include "map.h"
#include "divers.h"
#include "propag.h"

#define SPEED 2.0
extern ccase * cases;

/***********************************************
 **  Voici la liste actuelle des problemes:   **
 **   - loading des icones du tb, il faut en  **
 **     faire une commune pour OpenGL et GTK  **
 **                                           **
 **  Pour le loading de primitives, l'image   **
 **  n'est jamais dans le rep de sauv?        **
 **                                           **
 **  Regler le probleme des images tga        **
 ***********************************************/



extern char * chemin;
extern t_map carte;

/********************************************** 
 **  INIT LISTES D'ELEMENTS  ****************** 
 **********************************************/

/*
**  Voici la liste des stimuli des ouvrieres.
**  + un copier/coller pour la reine. 
**
**  y'a des stimuli en plus pour la reine ? et
**  y'a quoi pour cocons et larves ? 
**  rien en externe, et cure et hungry en interne ?
**
**                     GayLord.
*/

/* Stimuli */
enum 
  {
    /* Worker : */
    NOTHING_TO_DO=0,
    SORTIR,
    NB_STIMULI
  };

/* Agents */
enum 
  {
    G=0,
    NB_AG
  };

/* Objets */
enum 
  {
    CHAMEAU=0,
    NB_OBJ
  };

/* Operators */
enum
  {
    EXEC=0,
    EXEC_TERM,
    SI,
    SI_TERM,
    TANT_QUE,
    TANT_QUE_TERM,
    NB_OP
  };

/*Rajouter marcher en random qaux primitives?*/


/* Primitives */
enum
  {
    RANDOM_WALK=0,
    FOLLOW,
    DIE,
    NB_PRIM
  };

/* types pour arg */
enum 
  {
    AG=0,
    STI
  };
/* types pour iaction */
enum
  {
    PICKING=0,
    TUER,
    NB_IA
  };
/**********************************************
 **  CHOIX DES IMAGES  ************************
 **********************************************
 **  Les textures  des cases sont a mettre   **
 **  dans le dossier textures.               **
 **  Les icones sont a mettre dans le        **
 **  dossier tb_icones.                      **
 **********************************************/

/* SOL */
enum 
  {
    SOL,
    MUR,
    CHEM,
    INV,
    NB_CASES
  };


void init_cases (ccase ** c,int * nb_case)
{
  *nb_case = NB_CASES;
  *c = malloc(NB_CASES * (sizeof (ccase)));

  (*c)[SOL].name = "Sol";
  (*c)[SOL].texture = Load_image("sable.bmp");
  (*c)[SOL].model = NULL;
  (*c)[SOL].objet = NULL;
  (*c)[SOL].accessible = 1;

  (*c)[MUR].name = "Mur";
  (*c)[MUR].texture = Load_image("herbe.bmp");
  (*c)[MUR].model = "cubbe";
  (*c)[MUR].objet = load_objet("cubbe",0.25);
  (*c)[MUR].accessible = 0;

  (*c)[CHEM].name = "Chemin";
  (*c)[CHEM].texture = Load_image("copper.bmp");
  (*c)[CHEM].model = NULL;
  (*c)[CHEM].objet = NULL;
  (*c)[CHEM].accessible = 1;

  (*c)[INV].name = "mur invisible";
  (*c)[INV].texture = Load_image("sable.bmp");
  (*c)[INV].model = 0;
  (*c)[INV].objet = 0;
  (*c)[INV].accessible = 0;

}

void free_case (ccase * cases, int nb_case UNUSED)
{
  free(cases);
}

/* UNDEPRECATED c'est le grand retour de primoutives! la liste des images du billboarding */
int * primoutives[NB_PRIM];

void init_primoutives ()
{
  primoutives[RANDOM_WALK] = Load_image("brick001.bmp");
  primoutives[FOLLOW] = Load_image("brick001.bmp");

}

/* DEPRECATED mais jle garde quand meme au cas ou...
int * primoutives[NB_PRIM];
void init_icones (char * chemin)
{
  primoutives[MATURE] = Load_image(chemin,"mature.png");
  primoutives[DIE] = Load_image(chemin,"mort.png");
  primoutives[CURE] = Load_image(chemin,"heal.png");
  primoutives[HAS_FOOD] = Load_image(chemin,"hasfood.png");
  primoutives[EAT] = Load_image(chemin,"eat.png");
  primoutives[PICK_UP] = Load_image(chemin,"pickup.png");
  primoutives[PUT_DOWN] = Load_image(chemin,"putdown.png");
  primoutives[FOLLOW_S] = Load_image(chemin,"follow.png");
  primoutives[FLEE_S] = Load_image(chemin,"flee.png");
  primoutives[KILL] = Load_image(chemin,"kill.png");
} 
*/

/********************************************** 
 **  INIT OPERATORS  ************************** 
 **********************************************/
int op_exemple (t_agent agent UNUSED,int (*p)() UNUSED,void * arg UNUSED)
{
  puts("Op appellé");
  
  return(1);
}

int executer(t_agent agent,int (*p)(),void * arg)
{
  p(agent,arg);
  if (agent->tache)
    (agent->en_cours)++;
  return 1;
}

int executer_terminal(t_agent agent,int (*p)(),void * arg)
{
  p(agent,arg);
  agent->tache = -1;       /*  -1 pour l'action basique, marcher en random */
  agent->en_cours = 0;
  return 1;
}

int si(t_agent agent,int (*p)(),void * arg)
{
  if(p(agent,arg))
    {
      (agent->en_cours)++;
      return 1;
    }
  else
    {
      agent->en_cours = -2; /*-2 représente l'interruption*/
      return 0;
    }
}

int si_terminal(t_agent agent,int (*p)(),void * arg)
{
  return executer_terminal(agent,p,arg);
}


int tant_que(t_agent agent,int (*p)(),void * arg)
{
  if(p(agent,arg))
    {
      (agent->en_cours)++;
      return 1;
    }
  return 0;
}

int tant_que_terminal(t_agent agent, int (*p)(),void * arg)
{
  if(p(agent,arg))
    {
      agent->en_cours = 0;
      agent->tache = -1;
      return 1;
    }
  return 0;
}


/********************************************** 
 **  INIT PRIMITIVES  ************************* 
 **********************************************/
 /**********************************************
 **  toi utilisateur pas touche cette        **
 **  fonction toi pas comprendre             **
 **  -------                                 **
 **  Je nie formellement mon implication     **
 **  dans toute ecriture de fonction moche   **
 **  et incomprehensible.       Goulag.      **
 **********************************************/

static inline int my_rand(int n)
{
  return(1 + (int) (n * (rand() / (RAND_MAX + 1.0))));
}


static inline int my_angle (float x) 
{
  if (my_sin(x) > 0.38268)
    return(1);
  if (my_sin(x) < -0.38268)
    return(-1);
  return(0);
}

int autour (int x,int y,float rx,x_et_y *tabu) /* tabu de 9 pointeurs sur t_case */
{
  int i,j=1;
  
  tabu[0].x = x;
  tabu[0].y = y;
  if (cases[carte.tab[x + my_angle(rx + 90)][y + my_angle(rx)].type].accessible)
    {
      tabu[1].x = x + my_angle(rx + 90);
      tabu[1].y = y + my_angle(rx);
      j++;
    }
  for (i=1;i <= 3;i++)
    {
      if (cases[carte.tab[x + my_angle(rx + 90 - i * 45)][y + my_angle(rx + i * 45)].type].accessible)
	{
	  tabu[j].x = x + my_angle(rx + 90 - i * 45);
	  tabu[j].y = y + my_angle(rx + i * 45);
	  j++;
	}
      if (cases[carte.tab[x + my_angle(rx + 90 - i * 45)][y + my_angle(rx - i * 45)].type].accessible)
	{
	  tabu[j].x = x + my_angle(rx + 90 - i * 45);
	  tabu[j].y = y + my_angle(rx - i * 45);
	  j++;
	}
    }
  if (cases[carte.tab[x + my_angle(rx + 90)][y + my_angle(rx)].type].accessible)
    {
      tabu[j].x = x + my_angle(rx + 270);
      tabu[j].y = y + my_angle(rx + 180);
      j++;
    }
  return j;
}  

int p_exemple (void * arg UNUSED)
{
  puts("p appellé");
  
  return(1);
}

int random_walk  (t_agent agent UNUSED,int arg UNUSED)
{
  /*
  agent->speed = SPEED;
  

  if (((int)(agent->x / TAILLE_CASE) == (int)(agent->xgoto / TAILLE_CASE)) && ((int) (agent->z / TAILLE_CASE) == (int)(agent->zgoto / TAILLE_CASE)))
    {
      tabu = malloc(9*sizeof(x_et_y));
      i = autour(agent->case_x,agent->case_y,agent->ry,tabu);
      if (i-1)
	{
	  j = my_rand(t[i-1]) - 5;
	  for (k=1; k<i;k++)
	    if (j <= (t[k]-5))
	      break;
	  piou = tabu[k];
	  agent->xgoto = (piou.x + 0.5) * TAILLE_CASE;
	  agent->zgoto = (piou.y + 0.5) * TAILLE_CASE;
	}
	free(tabu);
    }
  */
   return(0);
}

int die (t_agent agent,int arg UNUSED)
{
  agent->age *= (agent->age > 0)?-1:1;
  return(1);
}

int kill (t_agent agent,int arg UNUSED) /* dans le cas de kill, l'agent est la cible */
{
  agent->age *= (agent->age > 0)?-1:1;
  return 1;
}

int put_down (t_agent agent UNUSED,int arg UNUSED)
{
  /*move_truc(&(agent->caddie),&(carte.tab[agent->case_x][agent->case_y].objets));*/
  return 1;
}

int follow (t_agent agent, int arg)
{
  int i,j,i_max,j_max;
  
  /* caca("ici");*/

  agent->speed = 8.0;
  i=agent->case_x;
  j=agent->case_y;
  if (find_strong_stimulus(i,j,arg,&i_max,&j_max))
    {
    carte.tab[i_max][j_max].type = CHEM;
      if ((i != i_max) || (j != j_max))
	{
	  agent->xgoto = (i_max + 0.5) * TAILLE_CASE;
	  agent->zgoto = (j_max + 0.5) * TAILLE_CASE;
	  /*	  carte.tab[l=(int) (agent->xgoto / TAILLE_CASE)][k=(int) (agent->zgoto / TAILLE_CASE)].type = CHEM;*/
	  /*	  if (cases[carte.tab[l][k].type].accessible)
		  printf("type:%s acc:%i\n",cases[carte.tab[l][k].type].name,cases[carte.tab[l][k].type].accessible);*/
	  return(0);
	}
      else
	return(1);
    }
  else
    {
      /* marcher en random */
      return(0);
    }
}

int mature  (t_agent agent,int arg UNUSED)
{
  agent->ratio *= 2.0;
  return(1);
}

/********************************************** 
 **  INIT TABLEAU  **************************** 
 **********************************************/


void init_stimuli (behavior ** s,int *nb_sti, float *cycle)
{
  int i;

  *nb_sti = NB_STIMULI;
  *cycle = 7.5;
  *s = malloc(NB_STIMULI * (sizeof (behavior)));

  /* Worker : */
  (*s)[NOTHING_TO_DO].name_s = "Nothing to do";
  (*s)[NOTHING_TO_DO].ag = G;
  (*s)[SORTIR].name_s = "Sortir";
  (*s)[SORTIR].ag = G;

  for (i=0; i<NB_STIMULI; i++)
    {
      (*s)[i].task.brick = 0;
      (*s)[i].task.interruption = 0;
      (*s)[i].task.weight = 0;
      (*s)[i].task.weight_change = 0;
      (*s)[i].task.increment = 0;
      (*s)[i].task.threshold = 0;
    }
}


void remove_task2 (task t)
{
  brick b,bn;

      b = t.brick;
      while (b)
	{
	  bn = (*b).next;
	  free(b);
	  b = bn;
	}
      if (t.interruption)
	free(t.interruption);
}

void free_sti (behavior * stimuli)
{
  int i;

  for (i=0; i<NB_STIMULI; i++)
      remove_task2(stimuli[i].task);

  free(stimuli);
}


void init_agents (agent ** a,int * nb_ag)
{
  int i;
  
  *nb_ag = NB_AG;
  *a = malloc(NB_AG * (sizeof (agent)));

  (*a)[G].name_ag = "Gars Perdu"; 
  (*a)[G].ag = G;
  (*a)[G].ratio = 0.075;
  (*a)[G].nom_mod = "fourmi";

  for (i=0; i<NB_AG; i++)
    (*a)[i].mod = 0;
}

void init_objets (objet ** o,int * nb_obj)
{
  int i;
  
  *nb_obj = NB_OBJ;
  *o = malloc(NB_OBJ * (sizeof (objet)));

  (*o)[CHAMEAU].name_obj = "Chameau"; 
  (*o)[CHAMEAU].obj = CHAMEAU;
  (*o)[CHAMEAU].ratio = 0.05;
  (*o)[CHAMEAU].nom_mod = "chameau";
  
  for (i=0; i<NB_OBJ; i++)
    (*o)[i].mod = 0;
}

void init_operators (operator ** op,int * nb_op)
{
  char * s[NB_OP],*t1,*t2;
  int i;

  *nb_op = NB_OP;
  *op = malloc(NB_OP * (sizeof (operator)));

    (*op)[EXEC].name_op = "Executer";
    (*op)[EXEC].op = executer;
    (*op)[EXEC].type_op = 1;
    s[EXEC] = "exec.png";
    (*op)[EXEC_TERM].name_op = "Executer (terminal)";
    (*op)[EXEC_TERM].op = executer_terminal;
    (*op)[EXEC_TERM].type_op = 0;
    s[EXEC_TERM] = "exec term.png";
    (*op)[SI].name_op = "Si";
    (*op)[SI].op = si;
    (*op)[SI].type_op = 1;
    s[SI] = "si.png";
    (*op)[SI_TERM].name_op = "Si (terminal)";
    (*op)[SI_TERM].op = si_terminal;
    (*op)[SI_TERM].type_op = 0;
    s[SI_TERM] = "si term.png";
    (*op)[TANT_QUE].name_op = "Tant que";
    (*op)[TANT_QUE].op = tant_que;
    (*op)[TANT_QUE].type_op = 1;
    s[TANT_QUE] = "tant queue.png";
    (*op)[TANT_QUE_TERM].name_op = "Tant que (terminal)";
    (*op)[TANT_QUE_TERM].op = tant_que_terminal;
    (*op)[TANT_QUE_TERM].type_op = 0;
    s[TANT_QUE_TERM] = "tant queue term.png";

    for (i=0; i<NB_OP; i++)
      {
	t1 = my_concat("/../share/bebetes_show/tb_icones/",s[i]);
	t2 = my_concat(chemin,t1);
	(*op)[i].icone_op = t2;
	free(t1);
      }
}



void free_op (operator * op,int nb_op)
{
  int i;
  int j=0;

  for (i=0; i<nb_op; i++)
    {
      if (op[i].icone_op)
	{
	  free(op[i].icone_op);
	  j++;
	}
    }
  if (j!=i)
    {
      caca("free_p, truc louche");
    }
  free(op);
}

void free_p (primitive * op,int nb_op)
{
  int i;
  int j=0;

  for (i=0; i<nb_op; i++)
    {
      if (op[i].icone_p)
	{
	  free(op[i].icone_p);
	  j++;
	}
    }
  if (j!=i)
    {
      caca("free_op, truc louche");
    }
  free(op);
}

void free_ag (agent * ag,int nb_ag UNUSED)
{
  free(ag);
}
void free_objs (objet * ob,int nb_obj UNUSED)
{
  free(ob);
}


void init_primitives (primitive ** p,int * nb_p)
{
  char * s[NB_PRIM],*t1,*t2;
  int i;

  *nb_p = NB_PRIM;
  *p = malloc(NB_PRIM * (sizeof (primitive)));

  (*p)[RANDOM_WALK].name_p = "Random walk";
  (*p)[RANDOM_WALK].p = random_walk;
  s[RANDOM_WALK] = "p_mort.png";
  (*p)[FOLLOW].name_p = "Follow";
  (*p)[FOLLOW].p = follow;
  s[FOLLOW] = "p_follow.png";
  (*p)[DIE].name_p = "Die";
  (*p)[DIE].p = die;
  s[DIE] = "p_mort.png";

  for (i=0; i<NB_PRIM; i++)
    {
      t1 = my_concat("/../share/bebetes_show/tb_icones/",s[i]);
      t2 = my_concat(chemin,t1);
      (*p)[i].icone_p = t2;
      free(t1);
    }
}



void init_arg (argument ** arg, behavior * sti, agent * ag, int * nb_arg, int nb_ag, int nb_sti)
{
  int i;
  
  *nb_arg = nb_ag + nb_sti;
  *arg = malloc(*nb_arg * (sizeof (argument)));

  for (i=0; i<nb_ag; i++)
    {
      (*arg)[i].name = ag[i].name_ag;
      (*arg)[i].id = i;
      (*arg)[i].type = AG;
    }

  for (i=0; i<nb_sti; i++)
    {
      (*arg)[i+nb_ag].name = sti[i].name_s;
      (*arg)[i+nb_ag].id = i;
      (*arg)[i+nb_ag].type = STI;
    }
}

void free_arg (argument * a,int nb_arg UNUSED)
{
  free(a);
}
/* iactions) */

int ia_tuer(void *p, int t, int x UNUSED, int y UNUSED)
{
  if (t == IA_AGENT)
    kill (p,2);

  return(1);
}

void init_iaction (iaction **ia,int *nb_ia)
{
  *nb_ia = NB_IA;
  *ia = malloc(NB_IA * (sizeof (iaction)));

  (*ia)[PICKING].name_ia = "Suivre une fourmi";
  (*ia)[PICKING].ia = suivre;
  (*ia)[PICKING].i = 0;
  
  (*ia)[TUER].name_ia = "Tuer une fourmi";
  (*ia)[TUER].ia = ia_tuer;
  (*ia)[TUER].i = 1;
}

void free_iaction (iaction *ia,int nb_ia UNUSED)
{
  free(ia);
}
/* alexandre... t'es un putain de connard */
/* alexandre... t'es un putain de connard */
/* alexandre... t'es un putain de connard */
/* alexandre... t'es un putain de connard */
