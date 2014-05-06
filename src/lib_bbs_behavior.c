#include <stdio.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#define __USE_ISOC99
#define __USE_XOPEN
#include <math.h>
#undef  __USE_ISOC99
#undef  __USE_XOPEN
#include "gtk+extra/gtkplot.h"
#include "gtk+extra/gtkplotdata.h"
#include "gtk+extra/gtkplotbar.h"
#include "gtk+extra/gtkplotcanvas.h"
#include "gtk+extra/gtkplotprint.h"
#include "stat.h"

#include "lib_bbs_behavior.h"
#include "loader.h"
#include "camera.h"

#include "limits.h"
#include "loading.h"
#include "map.h"
#include "divers.h"
#include "discrimination.h"
#include "propag.h"

#define SPEED 1.0
#define FMAX_HUM 300.0

#define Add_courbe(x,y) stats[x].valeurs.ordonnee[current_stat] = y
#define Add_histo(x,y) spush(x,y)
#define Add_courbe_safe(x,y) add_courbe_stat(x,y)
#define Add_histo_safe(x,y) add_histo_stat(x,y)
#define Nom_abscisse(x) (*stats)[x].abscisse
#define Nom_ordonnee(x) (*stats)[x].ordonnee
#define Nom_fichier(x) (*stats)[x].fichier
#define Type(x) (*stats)[x].type
#define Statistiques void init_stats (graphe **stats,int *nb_stats)\
{\
  *nb_stats = NB_STATS;\
  *stats = malloc(NB_STATS * (sizeof (graphe)));
#define Fin }

extern objet *objets;
extern int nb_obj;
extern behavior *stimuli;
extern ccase * cases;
extern argument *arguments;
extern agent * agents;
extern graphe * stats;

extern int npoints;
extern char * chemin;
extern t_map carte;
extern int current_stat;
extern double tps;

/********************************************** 
 **  LISTES D'ELEMENTS  *********************** 
 **********************************************/
/* Stimuli */
enum 
  {
    EGG_NOTHING_TO_DO=0,
    LARVA_NOTHING_TO_DO,
    COCOON_NOTHING_TO_DO,
    ANT_NOTHING_TO_DO,
    QUEEN_NOTHING_TO_DO,

    /* Egg : */
    EGG_HUMIDITY,
    EGG_LIGHT,
    EGG_EGG,
    EGG_CURE_EGG,
    MATURING_EGG,

    /* Larva : */
    LARVA_HUMIDITY,
    LARVA_LIGHT,
    LARVA_LARVA,
    LARVA_CURE_LARVA,
    LARVA_HUNGRY_LARVA,
    MATURING_LARVA,

    /* Cocoon : */
    COCOON_HUMIDITY,
    COCOON_LIGHT,
    COCOON_COCOON,
    COCOON_CURE_COCOON,
    MATURING_COCOON,
    
    /* Worker : */
    ANT_HUMIDITY,
    ANT_LIGHT,
    ANT_EGG,
    ANT_CURE_EGG,
    ANT_LARVA,
    ANT_CURE_LARVA,
    ANT_HUNGRY_LARVA,
    ANT_COCOON,
    ANT_CURE_COCOON,
    ANT_CURE_ANT,
    ANT_HUNGRY_ANT,
    ANT_KILL_EGG,
    ANT_KILL_LARVA,
    ANT_FOOD,
    MATURING_ANT,

    /* Queen : */
    QUEEN_HUMIDITY,
    QUEEN_LIGHT,
    QUEEN_EGG,
    QUEEN_CURE_EGG,
    QUEEN_LARVA,
    QUEEN_CURE_LARVA,
    QUEEN_HUNGRY_LARVA,
    QUEEN_COCOON,
    QUEEN_CURE_COCOON,
    QUEEN_CURE_QUEEN,
    QUEEN_HUNGRY_QUEEN,
    QUEEN_KILL_EGG,
    QUEEN_KILL_LARVA,
    QUEEN_FOOD,
    QUEEN_LAY_EGGS,
    MATURING_QUEEN,

    NB_STIMULI
  };

/* Agents */
enum 
  {
    EGG=0,
    LARVA,
    COCOON,
    ANT,
    QUEEN,
    NB_AG
  };

/* Objets */
enum 
  {
    NOURRITURE=0,
    LUM,
    HUM,
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

/* Primitives */
enum
  {
    RANDOM_WALK=0,
    MATURE,
    DIE,
    /**/    PROPAGATE,
    DEPROPAGATE,
    CURE,
    IS_HEALTHY,
    HAS_FOOD,
    IS_GRANDIR,
    IS_PRESENCE,
    EAT,
    PICK_UP,
    PUT_DOWN,
    FOLLOW_S,
    FLEE_S,
    KILL,
    IS_PONDRE,
    PONDRE,
    NB_PRIM
  };

/* types pour arg */
enum 
  {
    AG=0,
    OBJ,
    STI
  };

/* types pour iaction */
enum
  {
    PICKING=0,
    TUER,
    IA_BF,
    IA_OF,
    IA_MAT,
    NB_IA
  };

/* SOL */
enum 
  {
    SOL=0,                   /* par defaut */
    FOURMILIERE,
    MUR,
    INV,
    NB_CASES
  };




/********************************************** 
 **  CASES  *********************************** 
 **********************************************/

/* CHANGEMENT! : accessible : 2 pour on peut y aller et y creer des objets aleatoirement, 1 : on peut seulement y aller , 0 : ni l un ni l autre */
void init_cases (ccase ** c,int * nb_case)
{
  *nb_case = NB_CASES;
  *c = malloc(NB_CASES * (sizeof (ccase)));

  (*c)[SOL].name = "Sol";
  (*c)[SOL].texture = Load_image("sol.bmp");
  (*c)[SOL].model = NULL;
  (*c)[SOL].objet = NULL;
  (*c)[SOL].accessible = 2;   

  (*c)[FOURMILIERE].name = "Fourmiliere";
  (*c)[FOURMILIERE].texture = Load_image("fourmiliere.bmp");
  (*c)[FOURMILIERE].model = NULL;
  (*c)[FOURMILIERE].objet = NULL;
  (*c)[FOURMILIERE].accessible = 1;

  (*c)[MUR].name = "Mur";
  (*c)[MUR].texture = Load_image("fourmiliere.bmp");
  (*c)[MUR].model = "cube";
  (*c)[MUR].objet = load_objet("cube",0.25);
  (*c)[MUR].accessible = 0;

  (*c)[INV].name = "mur invisible";
  (*c)[INV].texture = Load_image("sol.bmp");
  (*c)[INV].model = 0;
  (*c)[INV].objet = 0;
  (*c)[INV].accessible = 0;

}

void free_case (ccase * cases, int nb_case UNUSED)
{
  free(cases);
}


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
  if (agent->tache != agent->type)
    (agent->en_cours)++;
  return 1;
}

int executer_terminal(t_agent agent,int (*p)(),void * arg)
{
  agent->na=0;
  end_tache(agent);
  agent->tache = agent->type;       
  p(agent,arg);                                    
  return 1;
}

int si(t_agent agent,int (*p)(),void * arg)
{
  if (p(agent,arg))
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
  if (p(agent,arg))
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
      agent->na=0;
      end_tache(agent);
      agent->tache = agent->type;
      return 1;
    }
  return 0;
}

/****  Le tableau et free  *********/

void init_operators (operator ** op,int * nb_op)
{
  char * s[NB_OP],*t1,*t2;
  int i;

  *nb_op = NB_OP;
  *op = malloc(NB_OP * (sizeof (operator)));
  
  (*op)[EXEC].name_op = "Executer";
  (*op)[EXEC].op = executer;
  (*op)[EXEC].type_op = 1;
  s[EXEC] = "p_exec.png";
  (*op)[EXEC_TERM].name_op = "Executer (terminal)";
  (*op)[EXEC_TERM].op = executer_terminal;
  (*op)[EXEC_TERM].type_op = 0;
  s[EXEC_TERM] = "p_exec term.png";
  (*op)[SI].name_op = "Si";
  (*op)[SI].op = si;
  (*op)[SI].type_op = 1;
  s[SI] = "p_si.png";
  (*op)[SI_TERM].name_op = "Si (terminal)";
  (*op)[SI_TERM].op = si_terminal;
  (*op)[SI_TERM].type_op = 0;
  s[SI_TERM] = "p_si term.png";
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


/********************************************** 
 **  INIT PRIMITIVES  ************************* 
 **********************************************/

/**** FCT POUR LES PRIIMITIVES *******/

/**********************************************
 **  toi utilisateur pas touche ces          **
 **  fonctions toi pas comprendre            **
 **  -------                                 **
 **  Nous nions formellement notre           **
 **  implication dans toute ecriture de      **
 **  fonction moche et incomprehensible.     **
 **                       Goulag & Gaylord.  **
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
static inline int mm_cos (float x) 
{
  if (x == 90 || x == 270)
    return(0);
  if ((x >= 0 && x <= 90) || (x >= 270 && x <= 360))
    return(1);
  else
    return(-1);
}
static inline int mm_sin (float x) 
{
  if (x == 0 || x == 180)
    return(0);
  if (x >= 0 && x <= 180)
    return(1);
  else
    return(-1);
}
static inline int pos_rd (int x)
{
  switch (x)
    {
    case 0:
      return(0);
    case 1:
      return(1);
    case 2:
      return(3);
    case 3:
      return(5);
    case 4:
      return(7);
    case 5:
      return(6);
    case 6:
      return(4);
    case 7:
      return(2);
    }
  return(8);
}
static inline int arondi_ag (int x)
{
  if (x >= 337 || x < 22)
    return(0);
  if (x >= 22 && x < 67)
    return(45);
  if (x >= 67 && x < 112)
    return(90);
  if (x >= 112 && x < 157)
    return(135);
  if (x >= 157 && x < 202)
    return(180);
  if (x >= 202 && x < 247)
    return(225);
  if (x >= 247 && x < 292)
    return(270);
  return(315);
}

/* putain, tabu-se... */
int autour(int x,int y,float rx,x_et_y *tabu) /* tabu de 9 pointeurs sur t_case */
{
  int mx,my,t,ti,tj,k,h,i,j = 0;
  
  mx = arondi_ag(rx);
  my = (mx - 45 + 360) % 360;
  for (i = 0; i < 9; i++)
    {
      tabu[i].x = carte.x_max;
      tabu[i].y = carte.y_max;
    }
  /* les 8 cases autours */
  for (i = 0; i < 4; i++)
    {
      /* coins */
      ti = mm_cos(mx) + x;
      tj = mm_sin(mx) + y;
      if (in_map(ti,tj) && cases[carte.tab[ti][tj].type].accessible && 
	  ((ti == x || tj == y) ||
	   (in_map(ti,y) && in_map(x,tj) && 
	    (cases[carte.tab[x][tj].type].accessible || cases[carte.tab[ti][y].type].accessible))))
	{
	  t = pos_rd(j);
	  tabu[t].x = ti;
	  tabu[t].y = tj;
	}
      j++;
      /* aretes */
      ti = mm_cos(my) + x;
      tj = mm_sin(my) + y;
      if (in_map(ti,tj) && cases[carte.tab[ti][tj].type].accessible && 
	  ((ti == x || tj == y) ||
	   (in_map(ti,y) && in_map(x,tj) && 
	    (cases[carte.tab[x][tj].type].accessible || cases[carte.tab[ti][y].type].accessible))))
	{
	  t = pos_rd(j);
	  tabu[t].x = ti;
	  tabu[t].y = tj;
	}
      j++;
      mx = (mx + 90 + 360) % 360;
      my = (my - 90 + 360) % 360;
    }
  /* case centrale */
  if (in_map(x,y) && cases[carte.tab[x][y].type].accessible)
    {
      t = pos_rd(j);
      tabu[j].x = x;
      tabu[j].y = y;
    }

  j = 0; 
  while (j < 9)
    {
      i = 0;
      while ((i + j) < 9 && tabu[i + j].x == carte.x_max)
	i++;
      k = i + j;
      h = 0;
      if (i)
	while ((k + h) < 9 && (tabu[k].x != carte.x_max))
	  {
	    tabu[j+h] = tabu[k+h];
	    tabu[k+h].x = carte.x_max;
	    h++;
	  }
      j = j + (h ? (h - 1):(1));
    }

  j = 0;
  for (i = 0; i < 9; i++)
    if (in_map(tabu[i].x,tabu[i].y))
      j++;

  return(j);
}  

/*trouve la case possédant le stimulus donné avec la plus grande force*/
/*retourne 0 si aucune case ne possede le stimulus donné, sinon 1*/

static inline float force_stim(int i,int j,int stim)
{
  t_signal list;

  list = carte.tab[i][j].signaux;
  while (list && (list->type != stim))
    list = list->next;
  if (list)
    return(list->force);
  else 
    return(0.0);
}

x_et_y find_strong_stimulus(x_et_y *tabu, int max, int stim)
{
  int i, cpt = 1, n, j;
  float tabu2[9], tmp = 0.0, force = 0.0;
  x_et_y res;

  for (i = 0; i < max; i++)
    if ((tmp = force_stim(tabu[i].x,tabu[i].y,stim)) > force)
      {
	cpt = 1;
	force = tmp;
	tabu2[i] = force;
      }
    else
      if (tmp == force)
	{
	  cpt++;
	  tabu2[i] = force;
	}
      else
	tabu2[i] = 0.0;
  
  if (!force)
    {
      res.x = carte.x_max;
      return(res);
    }
  if (tabu2[max - 1] == force)
    {
      res.x = tabu[max - 1].x;
      res.y = tabu[max - 1].y;
      return(res);
    }

  n = my_rand(cpt) - 1;
  j = 0;
  i = 0;
  while (tabu2[i] != force && i<max)
    i++;
  while (j<n && i<max)
    {
      i++;
      if (tabu2[i] == force)
	j++;
    }

  res.x = tabu[i].x;
  res.y = tabu[i].y;
  return(res);
}

x_et_y find_weak_stimulus(x_et_y *tabu, int max, int stim)
{
  int i, cpt = 1, n, j;
  float tabu2[9], tmp = 0.0, force = INFINITY;
  x_et_y res;

  for (i = 0; i < max; i++)
    if ((tmp = force_stim(tabu[i].x,tabu[i].y,stim)) < force)
      {
	cpt = 1;
	force = tmp;
	tabu2[i] = force;
      }
    else
      if (tmp == force)
	{
	  cpt++;
	  tabu2[i] = force;
	}
      else
	tabu2[i] = INFINITY;
  
  if (max == cpt)
    {
      res.x = carte.x_max;
      return(res);
    }
  /* si case act est hors sti, ou le min */
  if (!tabu2[max - 1] || tabu2[max - 1] == force)
    {
      res.x = tabu[max - 1].x;
      res.y = tabu[max - 1].y;
      return(res);
    }
  n = my_rand(cpt) - 1;
  j = 0;
  i = 0;
  while (tabu2[i] != force && i<max)
    i++;
  while (j<n && i<max)
    {
      i++;
      if (tabu2[i] == force)
	j++;
    }
  
  res.x = tabu[i].x;
  res.y = tabu[i].y;
  return(res);
}

/**** PRIMITIVES ELLES MEMES : *******/

int p_exemple(void * arg UNUSED)
{
  puts("p appellé");
  
  return(1);
}

int random_walk(t_agent agent, int arg UNUSED)
{
  int i,j,k,t[]={165, 145, 115, 85, 65, 45, 30, 15, 5};
  x_et_y tabu[9],piou;

  agent->speed = SPEED;
  if (((int)(agent->x / TAILLE_CASE) == (int)(agent->xgoto / TAILLE_CASE)) && ((int) (agent->z / TAILLE_CASE) == (int)(agent->zgoto / TAILLE_CASE)))
    {
      i = autour(agent->case_x,agent->case_y,agent->ry,tabu);
      if (i-1)
	{
	  j = my_rand(t[0]);
	  for (k = i - 1; k >= 0; k--)
	    if (t[k] >= j)
	      break;
	  /*if (k>0 && t[k] == t[k-1] && my_rand(2) == 2)
	    k--;*/
	  piou = tabu[k];
	  agent->xgoto = (piou.x + 0.5) * TAILLE_CASE;
	  agent->zgoto = (piou.y + 0.5) * TAILLE_CASE;
	}
    }
  return(0);
}


int propagate(t_agent agent, int arg)
{
  prop_e(agent->case_x,agent->case_y,arguments[arg].id,10,1);

  return(1);
}

int depropagate(t_agent agent, int arg)
{
  t_signal tmp;
  
  tmp = carte.tab[agent->case_x][agent->case_y].signaux;
  while (tmp && (tmp->type != arguments[arg].id))
    tmp = tmp->next;
  if (tmp)
    deprop_e(agent->case_x,agent->case_y,arguments[arg].id,tmp->force,(int) stimuli[arguments[arg].id].task.increment);

  return(1);
}

int put_down(t_agent agent, int arg UNUSED)
{
  if (agent->caddie)
    {
      if (!agent->is_caddie_ag)
	{
	  ((t_objet) agent->caddie)->x = agent->x;
	  ((t_objet) agent->caddie)->z = agent->z;
	  ((t_objet) agent->caddie)->is_caddie = 0;
	  add_objet(agent->case_x,agent->case_y,(t_objet) agent->caddie);
	}
      else 
	{
	  ((t_agent) agent->caddie)->x = agent->x;
	  ((t_agent) agent->caddie)->z = agent->z;
	  ((t_agent) agent->caddie)->is_caddie = 0;
	  add_agent(agent->case_x,agent->case_y,(t_agent) agent->caddie);
	}
    }
  agent->caddie = 0;

  return(1);
}

int die(t_agent agent, int arg UNUSED)
{

  if (agent->type == EGG || agent->type == LARVA || agent->type == COCOON)
    creer_objet(objets[NOURRITURE].nom_mod, agent->case_x, agent->case_y, 0.,0.,0.,objets[NOURRITURE].ratio, NOURRITURE,1);
  put_down(agent, 1);
 
  agent->age *= (agent->age > 0)?-1:1;
  return(1);
}

int mkill(t_agent agent, int arg) 
{
  t_case c;
  int i;

  c = carte.tab[agent->case_x][agent->case_y];
  i = 0;
  while (i < c.nbr_ag) 
   {
     if (arguments[arg].id == c.agents[i]->type || arg == -1)
       {
	 c.agents[i]->age *= (c.agents[i]->age > 0)?-1:1;	 
	 i = c.nbr_ag + 5;
       }
     i++;
   }

  return(1);
}

int pick_up(t_agent agent, int arg)
{
  t_case c;
  int i, stim;
  t_agent ca;
  t_objet co;

  c = carte.tab[agent->case_x][agent->case_y];
  i = 0;

  if (!agent->caddie)
    {
      if (arguments[arg].type == OBJ)
	{
	  if (!c.nbr_obj)
	    return(0);
	  co = c.objets[0];
	  for (i = 0; i < c.nbr_obj; i++)
	    if ((arguments[arg].id == c.objets[i]->type) &&
		((co->valeur < c.objets[i]->valeur) ||
		 (co->type != arguments[arg].id)))
	      co = c.objets[i];
	       
	  if (arguments[arg].id == co->type)
	    {
	      if (co->type == NOURRITURE)
		agent->caddie = co;
	      agent->is_caddie_ag = 0;
	      ((t_objet) agent->caddie)->is_caddie = 1;
	      del_objet(agent->case_x,agent->case_y,co);
	      return(1);
	    }
	}
      else
	if (arguments[arg].type == AG)
	  {
	    stim = 0;
	    if (agent->tache == ANT_EGG || agent->tache == QUEEN_EGG)
	      stim = EGG_EGG;
	    else
	      {
		if (agent->tache == ANT_LARVA || agent->tache == QUEEN_LARVA)
		  stim = LARVA_LARVA;
		else
		  {
		    if (agent->tache == ANT_COCOON || agent->tache == QUEEN_COCOON)
		      stim = COCOON_COCOON;
		    else
		      {
			if (agent->tache == ANT_CURE_EGG || agent->tache == QUEEN_CURE_EGG)
			  stim = EGG_CURE_EGG;
			else
			  {
			    if (agent->tache == ANT_CURE_LARVA || agent->tache == QUEEN_CURE_LARVA)
			      stim = LARVA_CURE_LARVA;
			    else
			      {
				if (agent->tache == ANT_CURE_COCOON || agent->tache == QUEEN_CURE_COCOON)
				  stim = COCOON_CURE_COCOON;
				else
				  {
				    if (agent->tache == ANT_HUNGRY_LARVA || agent->tache == QUEEN_HUNGRY_LARVA)
				      stim = LARVA_HUNGRY_LARVA;
				  }
			      }
			  }
		      }
		  }
	      }
	  if (!c.nbr_ag)
	    return(0);
	    ca = c.agents[0];
	    for (i = 0; i < c.nbr_ag; i++)
	      if ((arguments[arg].id == c.agents[i]->type && agent != c.agents[i]) && 
		  ((ca->stimuli[stim].force < c.agents[i]->stimuli[stim].force) || 
		   (ca->type != arguments[arg].id || ca == agent)))
		ca = c.agents[i];
	    if (arguments[arg].id == ca->type && agent != ca)
	      {
		agent->caddie = ca;
		agent->is_caddie_ag = 1;
		ca->is_caddie = 1;
		del_agent(agent->case_x,agent->case_y,ca);

		if (ca->type == EGG)
		  {
		    ca->stimuli[EGG_EGG].force -= 5.0;
		    if (ca->stimuli[EGG_EGG].force < 0.0)
		      ca->stimuli[EGG_EGG].force = 0.0;
		  }
		else
		  {
		    if (ca->type == LARVA)
		      {
			ca->stimuli[LARVA_LARVA].force -= 5.0;
			if (ca->stimuli[LARVA_LARVA].force < 0.0)
			  ca->stimuli[LARVA_LARVA].force = 0.0;
		      }
		    else
		      if (ca->type == COCOON)
			{
			  ca->stimuli[COCOON_COCOON].force -= 5.0;
			  if (ca->stimuli[COCOON_COCOON].force < 0.0)
			    ca->stimuli[COCOON_COCOON].force = 0.0;
			}
		  }
		return(1);
	      }
	  }
    }
  return(1);
}

int follow(t_agent agent, int arg)
{
  int i;
  x_et_y tabu[9],nef;
  
  agent->speed = SPEED;
  if (((int)(agent->x / TAILLE_CASE) == (int)(agent->xgoto / TAILLE_CASE)) && ((int) (agent->z / TAILLE_CASE) == (int)(agent->zgoto / TAILLE_CASE)))
    {
      i = autour(agent->case_x,agent->case_y,agent->ry,tabu);
      nef = find_strong_stimulus(tabu,i,arguments[arg].id);
      if (nef.x == carte.x_max)
	return(random_walk(agent,arguments[arg].id));
      if (nef.x != agent->case_x || nef.y != agent->case_y)
	{
	  agent->xgoto = (nef.x + 0.5) * TAILLE_CASE;
	  agent->zgoto = (nef.y + 0.5) * TAILLE_CASE;
	  return(0);
	}
      return(1);
    }
  return(0);
}


int flee(t_agent agent, int arg)
{
  int i;
  x_et_y tabu[9],nef;
  
  agent->speed = SPEED;
  if (((int)(agent->x / TAILLE_CASE) == (int)(agent->xgoto / TAILLE_CASE)) && ((int) (agent->z / TAILLE_CASE) == (int)(agent->zgoto / TAILLE_CASE)))
    {
      i = autour(agent->case_x,agent->case_y,agent->ry,tabu);
      nef = find_weak_stimulus(tabu,i,arguments[arg].id);
      if (nef.x == carte.x_max)
	return(random_walk(agent,arguments[arg].id));
      if (nef.x != agent->case_x || nef.y != agent->case_y)
	{
	  agent->xgoto = (nef.x + 0.5) * TAILLE_CASE;
	  agent->zgoto = (nef.y + 0.5) * TAILLE_CASE;
	  return(0);
	}
      return(1);
    }
  return(0);
}

int mature(t_agent agent,int arg UNUSED)
{
  if (agent->type == EGG)
    {
      if (agent->stimuli[EGG_CURE_EGG].force > 10.69)
	{
	  agent->stimuli[MATURING_LARVA].force = agent->stimuli[MATURING_EGG].force;
	  agent->type = LARVA;
	  agent->ratio = agents[LARVA].ratio;
	  agent->model = create_3Dobj(agents[LARVA].nom_mod);
	}
      else
	die(agent,1);
      return(1);
    }

  if (agent->type == LARVA)
    {
      if (agent->stimuli[LARVA_CURE_LARVA].force > 10.69 && agent->stimuli[LARVA_HUNGRY_LARVA].force > 10.69)
	{
	  agent->stimuli[MATURING_COCOON].force = agent->stimuli[MATURING_LARVA].force;
	  agent->type = COCOON;
	  agent->ratio = agents[COCOON].ratio;
	  agent->model = create_3Dobj(agents[COCOON].nom_mod); 
	}
      else
	die(agent,1);
      return(1);
    }

  if (agent->type == COCOON)
    {
      if (agent->stimuli[COCOON_CURE_COCOON].force > 10.69)
	{
	  if (my_rand(12) == 7)
	    {
	      agent->stimuli[MATURING_QUEEN].force = agent->stimuli[MATURING_COCOON].force;
	      agent->type = QUEEN;
	      agent->ratio = agents[QUEEN].ratio;
	      agent->model = create_3Dobj(agents[QUEEN].nom_mod); 
	    }
	  else
	    {
	      agent->stimuli[MATURING_ANT].force = agent->stimuli[MATURING_COCOON].force;
	      agent->type = ANT;
	      agent->ratio = agents[ANT].ratio;
	      agent->model = create_3Dobj(agents[ANT].nom_mod); 
	    }
	}
      else
	die(agent,1);
      return(1);
    }

  die(agent,1);
  return(1);
}

int manger(t_agent agent, int arg)
{
  t_case c;
  int i, manger_ag;

  if (agent->type == LARVA)
    manger_ag = LARVA_HUNGRY_LARVA; 
  else
    if (agent->type == ANT)
      manger_ag = ANT_HUNGRY_ANT;
    else
      if (agent->type == QUEEN)
	manger_ag = QUEEN_HUNGRY_QUEEN;

  c = carte.tab[agent->case_x][agent->case_y];
  i = 0;
  if (arguments[arg].type == OBJ && agent->type != EGG && agent->type != COCOON)
    while (i < c.nbr_obj) 
      {
	if (arguments[arg].id == c.objets[i]->type)
	  {
	    agent->stimuli[manger_ag].force++;
	    c.objets[i]->valeur--;
	    if (agent->stimuli[manger_ag].force >= stimuli[manger_ag].task.force)
	      {
		agent->stimuli[manger_ag].force = stimuli[manger_ag].task.force;
		return(1);
	      }
	    if (c.objets[i]->valeur < 0.0)
	      return(1);
	    i = c.nbr_obj + 5;
	  }
	i++;
      }

  return(!i);
}

int soigner(t_agent agent, int arg)
{
  t_case c;
  int i, cure;
  float min = FLT_MAX;

  t_agent a_min = 0;

  if (arguments[arg].type != AG)
    {
      if (agent->type == ANT)
	cure = ANT_CURE_ANT;
      else
	cure = QUEEN_CURE_QUEEN;
      
      agent->stimuli[cure].force++;
      if (agent->stimuli[cure].force > stimuli[cure].task.force)
	{
	  agent->stimuli[cure].force = stimuli[cure].task.force;
	  return(1);
	}
      return(0);
    }

  if (agents[arguments[arg].id].ag == EGG)
    cure = EGG_CURE_EGG;
  else
    if (agents[arguments[arg].id].ag == LARVA)
      cure = LARVA_CURE_LARVA;
    else
      if (agents[arguments[arg].id].ag == COCOON)
	cure = COCOON_CURE_COCOON;
      else
	if (agents[arguments[arg].id].ag == ANT)
	  cure = ANT_CURE_ANT;
	else
	  if (agents[arguments[arg].id].ag == QUEEN)
	    cure = QUEEN_CURE_QUEEN;

  c = carte.tab[agent->case_x][agent->case_y];      
  for (i = 0; i < c.nbr_ag; i++)
    if (arguments[arg].id == c.agents[i]->type && min > c.agents[i]->stimuli[cure].force)
      {
	a_min = c.agents[i];
	min = c.agents[i]->stimuli[cure].force;
      }
	
  if (a_min)
    {
      a_min->stimuli[cure].force+=5.0;
      if (a_min->stimuli[cure].force > stimuli[cure].task.force)
	{
	  a_min->stimuli[cure].force = stimuli[cure].task.force;
	  return(1);
	}
      return(0);
    }

  return(1);
}

int pondre(t_agent agent, int arg UNUSED)
{
  creer_agent(agents[EGG].nom_mod, agent->case_x, agent->case_y, 0.0, 0.0, 0.0, agents[EGG].ratio, EGG, 1);
  return(1);
}

int is_pondre(t_agent agent, int arg UNUSED)
{
  float h, mx = -1.0, force, t1[] = {QUEEN_EGG, QUEEN_LARVA, QUEEN_COCOON, QUEEN_HUNGRY_LARVA, QUEEN_CURE_EGG, QUEEN_CURE_LARVA, QUEEN_CURE_COCOON};
  int i;

  if (!in_map(agent->case_x, agent->case_y))
    return(0);

  h = force_stim(agent->case_x, agent->case_y, QUEEN_HUMIDITY);
  for (i = 0; i < 7; i++)
    {
      force = force_stim(agent->case_x, agent->case_y, t1[i]);
      if (force > mx)
	mx = force;
    }

  if (mx <= 0.01 && (FMAX_HUM - h) / 0.5 <= 1.00)
    force = 50.0 / (mx + ((FMAX_HUM - h) / 0.5) + 0.2);
  else
    force = 0.0;

  prop_e(agent->case_x, agent->case_y, QUEEN_LAY_EGGS, force, force);
  
  return(1);
}

int has_food(t_agent agent, int arg UNUSED)
{
  float bouffe;
  float oeuf;
  float larve;
  float force;
  int t1[] = {ANT_HUNGRY_LARVA, QUEEN_HUNGRY_LARVA};
  /*  int t2[] = {ANT_KILL_EGG, ANT_KILL_LARVA}; */

  if (agent->type == LARVA)
    {
      if ((agent->stimuli[LARVA_HUNGRY_LARVA].force -= 0.2) < 0.0)
	die(agent,1);
      else
	{
	  agent->stimuli[LARVA_HUNGRY_LARVA].seuil = agent->stimuli[LARVA_HUNGRY_LARVA].force/2.0 ;
	  if (!agent->is_caddie)
	    {
	      force = (stimuli[LARVA_HUNGRY_LARVA].task.force - agent->stimuli[LARVA_HUNGRY_LARVA].force) / 20.0;
	      force *= force / 5.0;

	      prop_e(agent->case_x, agent->case_y,LARVA_HUNGRY_LARVA,force * 4.0,force * 4.0);
	      prop_mega(agent->case_x, agent->case_y,2,t1,force,0.5);
	    }
	}
      return(1);
    }
  if (agent->type == ANT)
    {
      if ((agent->stimuli[ANT_HUNGRY_ANT].force -= 0.5) < 0.0)
	die(agent,1);
      else
	{
	  agent->stimuli[ANT_HUNGRY_ANT].seuil = agent->stimuli[ANT_HUNGRY_ANT].force/2.0;
	  if (!agent->is_caddie)
	    {
	      force = (stimuli[ANT_HUNGRY_ANT].task.force - agent->stimuli[ANT_HUNGRY_ANT].force) / 20.0;
	      force *= force*(3.0/4.0);

	      prop_e(agent->case_x, agent->case_y,ANT_HUNGRY_ANT,force,force);
	    }
	}
      return(1);
    }
  if (agent->type == QUEEN)
    {
      if ((agent->stimuli[QUEEN_HUNGRY_QUEEN].force -= 0.5) < 0.0)
	die(agent,1);
      else
	{
	  agent->stimuli[QUEEN_HUNGRY_QUEEN].seuil = agent->stimuli[QUEEN_HUNGRY_QUEEN].force /*3.0*/ /2.0;
	  if (!agent->is_caddie)
	    {
	      force = (stimuli[QUEEN_HUNGRY_QUEEN].task.force - agent->stimuli[QUEEN_HUNGRY_QUEEN].force) / 20.0;
	      force *= force;
	      
	      prop_e(agent->case_x, agent->case_y,QUEEN_HUNGRY_QUEEN,force,force);
	      bouffe = force_stim(agent->case_x,agent->case_y,QUEEN_FOOD); 
	      oeuf = force_stim(agent->case_x,agent->case_y,QUEEN_EGG);
	      larve = force_stim(agent->case_x,agent->case_y,QUEEN_LARVA);
	      
	      /*  if(agent->stimuli[QUEEN_HUNGRY_QUEEN].force <= 20)
		{
		  caca("oulala, je dois manger un oeuf");
		  if(2*oeuf > bouffe)
		      {
			caca("beuuuuuuuuuuuhhhhhh");
			prop_e(agent->case_x, agent->case_y,QUEEN_KILL_EGG,force,force);
			agent->stimuli[QUEEN_KILL_EGG].seuil = agent->stimuli[QUEEN_HUNGRY_QUEEN].force;
		      }
		      
		      if(larve > bouffe)
		      {
		      prop_e(agent->case_x, agent->case_y,QUEEN_KILL_LARVA,force,force); 
			agent->stimuli[QUEEN_KILL_LARVA].seuil = 0;
			}
		  
			}*/
	    }
	  return(1);
	}
    }
  return(1);
}

int is_healthy(t_agent agent, int arg UNUSED)
{
  float force;
  int t1[] = {ANT_CURE_EGG, QUEEN_CURE_EGG};
  int t2[] = {ANT_CURE_LARVA, QUEEN_CURE_LARVA};
  int t3[] = {ANT_CURE_COCOON, QUEEN_CURE_COCOON};
  
  if (agent->type == EGG)
    {
      if ((agent->stimuli[EGG_CURE_EGG].force -= 0.5) < 0.0)
	die(agent,1);
      else
	{
	  agent->stimuli[EGG_CURE_EGG].seuil = agent->stimuli[EGG_CURE_EGG].force;
	  if (!agent->is_caddie)
	    {
	      force = (stimuli[EGG_CURE_EGG].task.force - agent->stimuli[EGG_CURE_EGG].force) / 20.0;
	      force *= force/3.0;
	      prop_mega(agent->case_x, agent->case_y,2,t1,force,0.2);
	      prop_e(agent->case_x, agent->case_y,EGG_CURE_EGG,force,force);
	    }
	}
      return(1);
    }
  if (agent->type == LARVA)
    {
      if ((agent->stimuli[LARVA_CURE_LARVA].force -= 0.7) < 0.0)
	die(agent,1);
      else
	{
	  agent->stimuli[LARVA_CURE_LARVA].seuil = agent->stimuli[LARVA_CURE_LARVA].force;
	  if (!agent->is_caddie)
	    {
	      force = (stimuli[LARVA_CURE_LARVA].task.force - agent->stimuli[LARVA_CURE_LARVA].force) / 20.0;
	      force *= force/3.0;
	      prop_mega(agent->case_x, agent->case_y,2,t2,force,0.2);
	      prop_e(agent->case_x, agent->case_y,LARVA_CURE_LARVA,force,force);	    }
	}
      return(1);
    }
  if (agent->type == COCOON)
    {
      if ((agent->stimuli[COCOON_CURE_COCOON].force -= 0.5) < 0.0)
	die(agent,1);
      else
	{
	  agent->stimuli[COCOON_CURE_COCOON].seuil = agent->stimuli[COCOON_CURE_COCOON].force;
	  if (!agent->is_caddie)
	    {
	      force = (stimuli[COCOON_CURE_COCOON].task.force - agent->stimuli[COCOON_CURE_COCOON].force) / 20.0;
	      force *= force/3.0;
	      prop_mega(agent->case_x, agent->case_y,2,t3,force,0.2);
	      prop_e(agent->case_x, agent->case_y,COCOON_CURE_COCOON,force,force);	    }
	}
      return(1);
    }
  if (agent->type == ANT)
    {
      if ((agent->stimuli[ANT_CURE_ANT].force -= 1.0) < 0.0)
	die(agent,1);
      else
	{
	  agent->stimuli[ANT_CURE_ANT].seuil = agent->stimuli[ANT_CURE_ANT].force/2.0;
	  if (!agent->is_caddie)
	    {
	      force = (stimuli[ANT_CURE_ANT].task.force / agent->stimuli[ANT_CURE_ANT].force) * 5.0;
	      prop_e(agent->case_x, agent->case_y,ANT_CURE_ANT,force,force);
	    }
	}
      return(1);
    }
  if (agent->type == QUEEN)
    {
      if ((agent->stimuli[QUEEN_CURE_QUEEN].force -= 1.0) < 0.0)
	die(agent,1);
      else
	{
	  agent->stimuli[QUEEN_CURE_QUEEN].seuil = agent->stimuli[QUEEN_CURE_QUEEN].force/2.0;
	  if (!agent->is_caddie)
	    {
	      force = (stimuli[QUEEN_CURE_QUEEN].task.force / agent->stimuli[QUEEN_CURE_QUEEN].force) * 5.0;
	      prop_e(agent->case_x, agent->case_y,QUEEN_CURE_QUEEN,force,force);	    }
	}
      return(1);
    }

  return(1);
}

int is_grandir(t_agent agent, int arg UNUSED)
{
  float force;
 
  if (agent->type == EGG)
    {
      agent->stimuli[MATURING_EGG].force += 1.0;
      if (agent->stimuli[MATURING_EGG].force >= stimuli[MATURING_LARVA].task.force)
	{
	  agent->stimuli[MATURING_EGG].seuil = 0.0; 
	  if (!agent->is_caddie)
	    {
	      force = agent->stimuli[MATURING_EGG].force;
	      prop_e(agent->case_x, agent->case_y,MATURING_EGG,force,force);
	    }
	}
      else
	agent->stimuli[MATURING_EGG].seuil = 5000.0;
      return(1);
    }
  if (agent->type == LARVA)
    {
      agent->stimuli[MATURING_LARVA].force += 1.0;
      if (agent->stimuli[MATURING_LARVA].force >= stimuli[MATURING_COCOON].task.force)
	{
	  agent->stimuli[MATURING_LARVA].seuil = 0.0;
	  if (!agent->is_caddie)
	    {
	  force = agent->stimuli[MATURING_LARVA].force;
	  prop_e(agent->case_x, agent->case_y,MATURING_LARVA,force,force);
	    }
	}
      else
	agent->stimuli[MATURING_LARVA].seuil = 5000.0;
      return(1);
    }
  if (agent->type == COCOON)
    {
      agent->stimuli[MATURING_COCOON].force += 1.0;
      if (agent->stimuli[MATURING_COCOON].force >= stimuli[MATURING_ANT].task.force)
	{
	  agent->stimuli[MATURING_COCOON].seuil = 0.0;
	  if (!agent->is_caddie)
	    {
	  force = agent->stimuli[MATURING_COCOON].force;
	  prop_e(agent->case_x, agent->case_y,MATURING_COCOON,force,force);
	    }
	}
      else
	agent->stimuli[MATURING_COCOON].seuil = 5000.0;
      return(1);
    }
  if (agent->type == ANT)
    {
      force = agent->stimuli[MATURING_ANT].force += 1.0;
      if (agent->stimuli[MATURING_ANT].force >= 3318.0) /*&& my_rand((int) force * 3) > 60)*/
	{
	  agent->stimuli[MATURING_ANT].seuil = 0.0;
	  if (!agent->is_caddie)
	    {
	      force = agent->stimuli[MATURING_ANT].force;
	      prop_e(agent->case_x, agent->case_y,MATURING_ANT,force,force);
	    }
	}
      else
	agent->stimuli[MATURING_ANT].seuil = 5000.0;
      return(1);
    }
  if (agent->type == QUEEN)
    {
      agent->stimuli[MATURING_QUEEN].force += 1.0;
      if (agent->stimuli[MATURING_QUEEN].force >= 9954)
	{
	  agent->stimuli[MATURING_QUEEN].seuil = 0.0; 
	  if (!agent->is_caddie)
	    {
	      force = agent->stimuli[MATURING_QUEEN].force;
	      prop_e(agent->case_x, agent->case_y,MATURING_QUEEN,force,force);
	    }
	}
      else
	agent->stimuli[MATURING_QUEEN].seuil = 5000.0;
      return(1);
    }

  return(1);
}

int is_presence(t_agent agent, int arg UNUSED)
{
  float force, lum, hum;
  int t1[] = {ANT_EGG, QUEEN_EGG};
  int t2[] = {ANT_LARVA, QUEEN_LARVA};
  int t3[] = {ANT_COCOON, QUEEN_COCOON};
  
  if (agent->type == EGG)
    {
      lum = force_stim(agent->case_x, agent->case_y, EGG_LIGHT);
      hum = force_stim(agent->case_x, agent->case_y, EGG_HUMIDITY);
      agent->stimuli[EGG_EGG].force += fmaxf(0.1 / (hum ? hum : 1.0) , 0.05) + 0.3 * lum;
      if(agent->stimuli[EGG_EGG].force < 0.0)
	agent->stimuli[EGG_EGG].force = 0.0;
      if (!agent->is_caddie)
	{
	  force = agent->stimuli[EGG_EGG].force/5.0;
	  prop_mega(agent->case_x,agent->case_y,2,t1,force,0.5);
	}
      return(1);
    }
  if (agent->type == LARVA)
    {
      lum = force_stim(agent->case_x, agent->case_y, LARVA_LIGHT);
      hum = force_stim(agent->case_x, agent->case_y, LARVA_HUMIDITY);
      agent->stimuli[LARVA_LARVA].force += fmaxf(0.1 / (hum ? hum : 1.0) , 0.01) + 0.03 * lum;
      if(agent->stimuli[LARVA_LARVA].force < 0.0)
	agent->stimuli[LARVA_LARVA].force = 0.0;
      if (!agent->is_caddie)
	{
	  force = agent->stimuli[LARVA_LARVA].force/5.0;
	  prop_mega(agent->case_x,agent->case_y,2,t2,force,0.5);
	}
      return(1);
    }
  if (agent->type == COCOON)
    {
      lum = force_stim(agent->case_x, agent->case_y, COCOON_LIGHT);
      hum = force_stim(agent->case_x, agent->case_y, COCOON_HUMIDITY);
      agent->stimuli[COCOON_COCOON].force += fmaxf(0.1 / (hum ? hum : 1.0) , 0.01) + 0.03 * lum;
      if(agent->stimuli[COCOON_COCOON].force < 0.0)
	agent->stimuli[COCOON_COCOON].force = 0.0;
      if (!agent->is_caddie)
	{
	  force = agent->stimuli[COCOON_COCOON].force/5.0;
	  prop_mega(agent->case_x,agent->case_y,2,t3,force,0.5);
	}
      return(1);
    }

  return(1);
}

/****  Le tableau et free  *********/

void init_primitives(primitive ** p,int * nb_p)
{
  char * s[NB_PRIM],*t1,*t2;
  int i;

  *nb_p = NB_PRIM;
  *p = malloc(NB_PRIM * (sizeof (primitive)));

  (*p)[RANDOM_WALK].name_p = "Se promener";
  (*p)[RANDOM_WALK].p = random_walk;
  (*p)[RANDOM_WALK].bboard = Load_image("random.tga");
  s[RANDOM_WALK] = "p_random.png";

  (*p)[MATURE].name_p = "Muer";
  (*p)[MATURE].p = mature;
  (*p)[MATURE].bboard = Load_image("mature.tga");  
  s[MATURE] = "p_mature.png";

  (*p)[DIE].name_p = "Mourir";
  (*p)[DIE].p = die;
  (*p)[DIE].bboard = Load_image("mort.tga");
  s[DIE] = "p_mort.png";

  (*p)[PROPAGATE].name_p = "Propager";
  (*p)[PROPAGATE].p = propagate;
  (*p)[PROPAGATE].bboard = Load_image("prop.tga");
  s[PROPAGATE] = "p_prop.png";

  (*p)[DEPROPAGATE].name_p = "Depropagager";
  (*p)[DEPROPAGATE].p = depropagate;
  (*p)[DEPROPAGATE].bboard = Load_image("deprop.tga");
  s[DEPROPAGATE] = "p_deprop.png";

  (*p)[CURE].name_p = "Soigner";
  (*p)[CURE].p = soigner;
  (*p)[CURE].bboard = Load_image("heal.tga");
  s[CURE] = "p_heal.png";

  (*p)[HAS_FOOD].name_p = "Niveau Faim";
  (*p)[HAS_FOOD].p = has_food;
  (*p)[HAS_FOOD].bboard = Load_image("hasfood.tga");
  s[HAS_FOOD] = "p_hasfood.png";

  (*p)[IS_HEALTHY].name_p = "Niveau Sante";
  (*p)[IS_HEALTHY].p = is_healthy;
  (*p)[IS_HEALTHY].bboard = Load_image("sante.tga");
  s[IS_HEALTHY] = "sante.png";

  (*p)[IS_GRANDIR].name_p = "Niveau Age";
  (*p)[IS_GRANDIR].p = is_grandir;
  (*p)[IS_GRANDIR].bboard = Load_image("vieillir.tga");
  s[IS_GRANDIR] = "vieillir.png";

  (*p)[IS_PRESENCE].name_p = "Niveau Confort";
  (*p)[IS_PRESENCE].p = is_presence;
  (*p)[IS_PRESENCE].bboard = Load_image("confort.tga");
  s[IS_PRESENCE] = "confort.png";

  (*p)[EAT].name_p = "Manger";
  (*p)[EAT].p = manger;
  (*p)[EAT].bboard = Load_image("eat.tga");
  s[EAT] = "p_eat.png";

  (*p)[PICK_UP].name_p = "Ramasser";
  (*p)[PICK_UP].p = pick_up;
  (*p)[PICK_UP].bboard = Load_image("pickup.tga");
  s[PICK_UP] = "p_pickup.png";

  (*p)[PUT_DOWN].name_p = "Poser";
  (*p)[PUT_DOWN].p = put_down;
  (*p)[PUT_DOWN].bboard = Load_image("putdown.tga");
  s[PUT_DOWN] = "p_putdown.png";

  (*p)[FOLLOW_S].name_p = "Suivre stimulus";
  (*p)[FOLLOW_S].p = follow;
  (*p)[FOLLOW_S].bboard = Load_image("follow.tga");
  s[FOLLOW_S] = "p_follow.png";

  (*p)[FLEE_S].name_p = "Fuir stimulus";
  (*p)[FLEE_S].p = flee;
  (*p)[FLEE_S].bboard = Load_image("flee.tga");
  s[FLEE_S] = "p_flee.png";

  (*p)[KILL].name_p = "Tuer";
  (*p)[KILL].p = mkill;
  (*p)[KILL].bboard = Load_image("kill.tga");
  s[KILL] = "p_kill.png";

  (*p)[PONDRE].name_p = "Pondre";
  (*p)[PONDRE].p = pondre;
  s[PONDRE] = "p_pondre.png";
  (*p)[PONDRE].bboard = Load_image("p_pondre.tga");


  (*p)[IS_PONDRE].name_p = "Niveux ponte";
  (*p)[IS_PONDRE].p = is_pondre;
  s[IS_PONDRE] = "p_niveau_ponte.png";
  (*p)[IS_PONDRE].bboard = Load_image("p_niveau_ponte.tga");



  for (i=0; i<NB_PRIM; i++)
    {
      t1 = my_concat("/../share/bebetes_show/tb_icones/",s[i]);
      t2 = my_concat(chemin,t1);
      (*p)[i].icone_p = t2;
      free(t1);
    }
}

void free_p(primitive * op,int nb_op)
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


/********************************************** 
 **  STIMULI  ********************************* 
 **********************************************/

#define INUTILISE 50.0

void init_stimuli(behavior ** s,int *nb_sti, float *cycle, int *nombre_cycles_depr)
{
  int i;

  *nb_sti = NB_STIMULI;
  *cycle = 7.5;
  *nombre_cycles_depr = 1;
  *s = malloc(NB_STIMULI * (sizeof (behavior)));

  (*s)[EGG_NOTHING_TO_DO].name_s = "Oeuf: Rien a faire";
  (*s)[EGG_NOTHING_TO_DO].ag = EGG;
  (*s)[EGG_NOTHING_TO_DO].interne = 0;
  (*s)[EGG_NOTHING_TO_DO].em_int = -1;
  (*s)[EGG_NOTHING_TO_DO].task.force = INUTILISE;
  (*s)[LARVA_NOTHING_TO_DO].name_s = "Larve: Rien a faire";
  (*s)[LARVA_NOTHING_TO_DO].ag = LARVA;
  (*s)[LARVA_NOTHING_TO_DO].interne = 0;
  (*s)[LARVA_NOTHING_TO_DO].em_int = -1;
  (*s)[LARVA_NOTHING_TO_DO].task.force = INUTILISE;
  (*s)[COCOON_NOTHING_TO_DO].name_s = "Cocon: Rien a faire";
  (*s)[COCOON_NOTHING_TO_DO].ag = COCOON;
  (*s)[COCOON_NOTHING_TO_DO].interne = 0;
  (*s)[COCOON_NOTHING_TO_DO].em_int = -1;
  (*s)[COCOON_NOTHING_TO_DO].task.force = INUTILISE;
  (*s)[ANT_NOTHING_TO_DO].name_s = "Ouvriere: Rien a faire";
  (*s)[ANT_NOTHING_TO_DO].ag = ANT;
  (*s)[ANT_NOTHING_TO_DO].interne = 0;
  (*s)[ANT_NOTHING_TO_DO].em_int = -1;
  (*s)[ANT_NOTHING_TO_DO].task.force = INUTILISE;
  (*s)[QUEEN_NOTHING_TO_DO].name_s = "Reine: Rien a faire";
  (*s)[QUEEN_NOTHING_TO_DO].ag = QUEEN;
  (*s)[QUEEN_NOTHING_TO_DO].interne = 0;
  (*s)[QUEEN_NOTHING_TO_DO].em_int = -1;
  (*s)[QUEEN_NOTHING_TO_DO].task.force = INUTILISE;

  (*s)[EGG_HUMIDITY].name_s = "Oeuf: Humidite";
  (*s)[EGG_HUMIDITY].ag = EGG;
  (*s)[EGG_HUMIDITY].interne = 0;
  (*s)[EGG_HUMIDITY].em_int = -1;
  (*s)[EGG_HUMIDITY].task.force = INUTILISE;
  (*s)[EGG_LIGHT].name_s = "Oeuf: Lumiere";
  (*s)[EGG_LIGHT].ag = EGG;
  (*s)[EGG_LIGHT].interne = 0;
  (*s)[EGG_LIGHT].em_int = -1;
  (*s)[EGG_LIGHT].task.force = INUTILISE;
  (*s)[EGG_EGG].name_s = "Oeuf: Confort";
  (*s)[EGG_EGG].ag = EGG;
  (*s)[EGG_EGG].interne = 1;
  (*s)[EGG_EGG].em_int = -1;
  (*s)[EGG_EGG].task.force = 10.0;
  (*s)[EGG_CURE_EGG].name_s = "Oeuf: Sante";
  (*s)[EGG_CURE_EGG].ag = EGG;
  (*s)[EGG_CURE_EGG].interne = 1;
  (*s)[EGG_CURE_EGG].em_int = -1; /* ? */
  (*s)[EGG_CURE_EGG].task.force = 80.0;
  (*s)[MATURING_EGG].name_s = "Oeuf:  Mue";
  (*s)[MATURING_EGG].ag = EGG;
  (*s)[MATURING_EGG].interne = 1;
  (*s)[MATURING_EGG].em_int = -1;
  (*s)[MATURING_EGG].task.force = 0.0;

  (*s)[LARVA_HUMIDITY].name_s = "Larve: Humidite";
  (*s)[LARVA_HUMIDITY].ag = LARVA;
  (*s)[LARVA_HUMIDITY].interne = 0;
  (*s)[LARVA_HUMIDITY].em_int = -1;
  (*s)[LARVA_HUMIDITY].task.force = INUTILISE;
  (*s)[LARVA_LIGHT].name_s = "Larve: Lumiere";
  (*s)[LARVA_LIGHT].ag = LARVA;
  (*s)[LARVA_LIGHT].interne = 0;
  (*s)[LARVA_LIGHT].em_int = -1;
  (*s)[LARVA_LIGHT].task.force = INUTILISE;
  (*s)[LARVA_LARVA].name_s = "Larve: Confort";
  (*s)[LARVA_LARVA].ag = LARVA;
  (*s)[LARVA_LARVA].interne = 1;
  (*s)[LARVA_LARVA].em_int = -1; 
  (*s)[LARVA_LARVA].task.force = 5.0;
  (*s)[LARVA_CURE_LARVA].name_s = "Larve: Sante";
  (*s)[LARVA_CURE_LARVA].ag = LARVA;
  (*s)[LARVA_CURE_LARVA].interne = 1;
  (*s)[LARVA_CURE_LARVA].em_int = -1; 
  (*s)[LARVA_CURE_LARVA].task.force = 80.0;
  (*s)[LARVA_HUNGRY_LARVA].name_s = "Larve: Faim";
  (*s)[LARVA_HUNGRY_LARVA].ag = LARVA;
  (*s)[LARVA_HUNGRY_LARVA].interne = 1;
  (*s)[LARVA_HUNGRY_LARVA].em_int = -1;
  (*s)[LARVA_HUNGRY_LARVA].task.force = 80.0;
  (*s)[MATURING_LARVA].name_s = "Larve: Mue";
  (*s)[MATURING_LARVA].ag = LARVA;
  (*s)[MATURING_LARVA].interne = 1;
  (*s)[MATURING_LARVA].em_int = -1; 
  (*s)[MATURING_LARVA].task.force = 200.0;

  (*s)[COCOON_HUMIDITY].name_s = "Cocon: Humidite";
  (*s)[COCOON_HUMIDITY].ag = COCOON;
  (*s)[COCOON_HUMIDITY].interne = 0;
  (*s)[COCOON_HUMIDITY].em_int = -1;
  (*s)[COCOON_HUMIDITY].task.force = INUTILISE;
  (*s)[COCOON_LIGHT].name_s = "Cocon: Lumiere";
  (*s)[COCOON_LIGHT].ag = COCOON;
  (*s)[COCOON_LIGHT].interne = 0;
  (*s)[COCOON_LIGHT].em_int = -1;
  (*s)[COCOON_LIGHT].task.force = INUTILISE;
  (*s)[COCOON_COCOON].name_s = "Cocon: Confort";
  (*s)[COCOON_COCOON].ag = COCOON;
  (*s)[COCOON_COCOON].interne = 1;
  (*s)[COCOON_COCOON].em_int = -1; 
  (*s)[COCOON_COCOON].task.force = 10.0;
  (*s)[COCOON_CURE_COCOON].name_s = "Cocon: Sante";
  (*s)[COCOON_CURE_COCOON].ag = COCOON;
  (*s)[COCOON_CURE_COCOON].interne = 1;
  (*s)[COCOON_CURE_COCOON].em_int = -1;
  (*s)[COCOON_CURE_COCOON].task.force = 80.0;
  (*s)[MATURING_COCOON].name_s = "Cocon: Mue";
  (*s)[MATURING_COCOON].ag = COCOON;
  (*s)[MATURING_COCOON].interne = 1;
  (*s)[MATURING_COCOON].em_int = -1;
  (*s)[MATURING_COCOON].task.force = 367.0;

  (*s)[ANT_HUMIDITY].name_s = "Ouvriere: Humidite";
  (*s)[ANT_HUMIDITY].ag = ANT;
  (*s)[ANT_HUMIDITY].interne = 0;
  (*s)[ANT_HUMIDITY].em_int = -1;
  (*s)[ANT_HUMIDITY].task.force = INUTILISE;
  (*s)[ANT_LIGHT].name_s = "Ouvriere: Lumiere";
  (*s)[ANT_LIGHT].ag = ANT;
  (*s)[ANT_LIGHT].interne = 0;
  (*s)[ANT_LIGHT].em_int = -1;
  (*s)[ANT_LIGHT].task.force = INUTILISE;

  (*s)[ANT_EGG].name_s = "Ouvriere: Oeuf";
  (*s)[ANT_EGG].ag = ANT;
  (*s)[ANT_EGG].interne = 0;
  (*s)[ANT_EGG].em_int = EGG;
  (*s)[ANT_EGG].task.force = INUTILISE;
  (*s)[ANT_CURE_EGG].name_s = "Ouvriere: Soin oeuf";
  (*s)[ANT_CURE_EGG].ag = ANT;
  (*s)[ANT_CURE_EGG].interne = 0;
  (*s)[ANT_CURE_EGG].em_int = EGG;
  (*s)[ANT_CURE_EGG].task.force = INUTILISE;
  (*s)[ANT_LARVA].name_s = "Ouvriere: Larve";
  (*s)[ANT_LARVA].ag = ANT;
  (*s)[ANT_LARVA].interne = 0;
  (*s)[ANT_LARVA].em_int = LARVA;
  (*s)[ANT_LARVA].task.force = INUTILISE;
  (*s)[ANT_CURE_LARVA].name_s = "Ouvriere: Soin larve";
  (*s)[ANT_CURE_LARVA].ag = ANT;
  (*s)[ANT_CURE_LARVA].interne = 0;
  (*s)[ANT_CURE_LARVA].em_int = LARVA;
  (*s)[ANT_CURE_LARVA].task.force = INUTILISE;
  (*s)[ANT_HUNGRY_LARVA].name_s = "Ouvriere: Nourrir larve";
  (*s)[ANT_HUNGRY_LARVA].ag = ANT;
  (*s)[ANT_HUNGRY_LARVA].interne = 0;
  (*s)[ANT_HUNGRY_LARVA].em_int = LARVA;
  (*s)[ANT_HUNGRY_LARVA].task.force = INUTILISE;
  (*s)[ANT_COCOON].name_s = "Ouvriere: Cocon";
  (*s)[ANT_COCOON].ag = ANT;
  (*s)[ANT_COCOON].interne = 0;
  (*s)[ANT_COCOON].em_int = COCOON;
  (*s)[ANT_COCOON].task.force = INUTILISE;
  (*s)[ANT_CURE_COCOON].name_s = "Ouvriere: Soin cocon";
  (*s)[ANT_CURE_COCOON].ag = ANT;
  (*s)[ANT_CURE_COCOON].interne = 0;
  (*s)[ANT_CURE_COCOON].em_int = COCOON;
  (*s)[ANT_CURE_COCOON].task.force = INUTILISE;
  (*s)[ANT_CURE_ANT].name_s = "Ouvriere: Sante";
  (*s)[ANT_CURE_ANT].ag = ANT;
  (*s)[ANT_CURE_ANT].interne = 1;
  (*s)[ANT_CURE_ANT].em_int = -1;
  (*s)[ANT_CURE_ANT].task.force = 80.0;
  (*s)[ANT_HUNGRY_ANT].name_s = "Ouvriere: Faim";
  (*s)[ANT_HUNGRY_ANT].ag = ANT;
  (*s)[ANT_HUNGRY_ANT].interne = 1;
  (*s)[ANT_HUNGRY_ANT].em_int = -1;
  (*s)[ANT_HUNGRY_ANT].task.force = 80.0;
  (*s)[ANT_KILL_EGG].name_s = "Ouvriere: Tuer oeuf";
  (*s)[ANT_KILL_EGG].ag = ANT;
  (*s)[ANT_KILL_EGG].interne = 1;
  (*s)[ANT_KILL_EGG].em_int = -1;
  (*s)[ANT_KILL_EGG].task.force = 50.0;
  (*s)[ANT_KILL_LARVA].name_s = "Ouvriere: Tuer larve";
  (*s)[ANT_KILL_LARVA].ag = ANT;
  (*s)[ANT_KILL_LARVA].interne = 1;
  (*s)[ANT_KILL_LARVA].em_int = -1;
  (*s)[ANT_KILL_LARVA].task.force = 50.0;
  (*s)[ANT_FOOD].name_s = "Ouvriere: Nourriture";
  (*s)[ANT_FOOD].ag = ANT;
  (*s)[ANT_FOOD].interne = 0;
  (*s)[ANT_FOOD].em_int = -1;
  (*s)[ANT_FOOD].task.force = 50.0;
  (*s)[MATURING_ANT].name_s = "Ouvriere: Mue";
  (*s)[MATURING_ANT].ag = ANT;
  (*s)[MATURING_ANT].interne = 1;
  (*s)[MATURING_ANT].em_int = -1;
  (*s)[MATURING_ANT].task.force = 630.0;

  (*s)[QUEEN_HUMIDITY].name_s = "Reine: Humidite";
  (*s)[QUEEN_HUMIDITY].ag = QUEEN;
  (*s)[QUEEN_HUMIDITY].interne = 0;
  (*s)[QUEEN_HUMIDITY].em_int = -1;
  (*s)[QUEEN_HUMIDITY].task.force = INUTILISE;
  (*s)[QUEEN_LIGHT].name_s = "Reine: Lumiere";
  (*s)[QUEEN_LIGHT].ag = QUEEN;
  (*s)[QUEEN_LIGHT].interne = 0;
  (*s)[QUEEN_LIGHT].em_int = -1;
  (*s)[QUEEN_LIGHT].task.force = INUTILISE;


  (*s)[QUEEN_EGG].name_s = "Reine: Oeuf";
  (*s)[QUEEN_EGG].ag = QUEEN;
  (*s)[QUEEN_EGG].interne = 0;
  (*s)[QUEEN_EGG].em_int = EGG;
  (*s)[QUEEN_EGG].task.force = INUTILISE;
  (*s)[QUEEN_CURE_EGG].name_s = "Reine: Soin oeuf";
  (*s)[QUEEN_CURE_EGG].ag = QUEEN;
  (*s)[QUEEN_CURE_EGG].interne = 0;
  (*s)[QUEEN_CURE_EGG].em_int = EGG;
  (*s)[QUEEN_CURE_EGG].task.force = INUTILISE;
  (*s)[QUEEN_LARVA].name_s = "Reine: Larve";
  (*s)[QUEEN_LARVA].ag = QUEEN;
  (*s)[QUEEN_LARVA].interne = 0;
  (*s)[QUEEN_LARVA].em_int = LARVA;
  (*s)[QUEEN_LARVA].task.force = INUTILISE;
  (*s)[QUEEN_CURE_LARVA].name_s = "Reine: Soin larve";
  (*s)[QUEEN_CURE_LARVA].ag = QUEEN;
  (*s)[QUEEN_CURE_LARVA].interne = 0;
  (*s)[QUEEN_CURE_LARVA].em_int = LARVA;
  (*s)[QUEEN_CURE_LARVA].task.force = INUTILISE;
  (*s)[QUEEN_HUNGRY_LARVA].name_s = "Reine: Nourrir larve";
  (*s)[QUEEN_HUNGRY_LARVA].ag = QUEEN;
  (*s)[QUEEN_HUNGRY_LARVA].interne = 0;
  (*s)[QUEEN_HUNGRY_LARVA].em_int = LARVA;
  (*s)[QUEEN_HUNGRY_LARVA].task.force = INUTILISE;
  (*s)[QUEEN_COCOON].name_s = "Reine: Cocon";
  (*s)[QUEEN_COCOON].ag = QUEEN;
  (*s)[QUEEN_COCOON].interne = 0;
  (*s)[QUEEN_COCOON].em_int = COCOON;
  (*s)[QUEEN_COCOON].task.force = INUTILISE;
  (*s)[QUEEN_CURE_COCOON].name_s = "Reine: Soin cocon";
  (*s)[QUEEN_CURE_COCOON].ag = QUEEN;
  (*s)[QUEEN_CURE_COCOON].interne = 0;
  (*s)[QUEEN_CURE_COCOON].em_int = COCOON;
  (*s)[QUEEN_CURE_COCOON].task.force = INUTILISE;
  (*s)[QUEEN_CURE_QUEEN].name_s = "Reine: Sante";
  (*s)[QUEEN_CURE_QUEEN].ag = QUEEN;
  (*s)[QUEEN_CURE_QUEEN].interne = 1;
  (*s)[QUEEN_CURE_QUEEN].em_int = -1;
  (*s)[QUEEN_CURE_QUEEN].task.force = 20.0;
  (*s)[QUEEN_HUNGRY_QUEEN].name_s = "Reine: Faim";
  (*s)[QUEEN_HUNGRY_QUEEN].ag = QUEEN;
  (*s)[QUEEN_HUNGRY_QUEEN].interne = 1;
  (*s)[QUEEN_HUNGRY_QUEEN].em_int = -1;
  (*s)[QUEEN_HUNGRY_QUEEN].task.force = 100.0;
  (*s)[QUEEN_KILL_EGG].name_s = "Reine: Tuer oeuf";
  (*s)[QUEEN_KILL_EGG].ag = QUEEN;
  (*s)[QUEEN_KILL_EGG].interne = 1;
  (*s)[QUEEN_KILL_EGG].em_int = -1;
  (*s)[QUEEN_KILL_EGG].task.force = 50.0;
  (*s)[QUEEN_KILL_LARVA].name_s = "Reine: Tuer larve";
  (*s)[QUEEN_KILL_LARVA].ag = QUEEN;
  (*s)[QUEEN_KILL_LARVA].interne = 1;
  (*s)[QUEEN_KILL_LARVA].em_int = -1;
  (*s)[QUEEN_KILL_LARVA].task.force = 50.0;
  (*s)[QUEEN_FOOD].name_s = "Reine: Nourriture";
  (*s)[QUEEN_FOOD].ag = QUEEN;
  (*s)[QUEEN_FOOD].interne = 0;
  (*s)[QUEEN_FOOD].em_int = -1;
  (*s)[QUEEN_FOOD].task.force = 50.0;
  (*s)[QUEEN_LAY_EGGS].name_s = "Reine: Pondre";
  (*s)[QUEEN_LAY_EGGS].ag = QUEEN;
  (*s)[QUEEN_LAY_EGGS].interne = 1;
  (*s)[QUEEN_LAY_EGGS].em_int = -1;
  (*s)[QUEEN_LAY_EGGS].task.force = 50.0;
  (*s)[MATURING_QUEEN].name_s = "Reine: Mue";
  (*s)[MATURING_QUEEN].ag = QUEEN;
  (*s)[MATURING_QUEEN].interne = 1;
  (*s)[MATURING_QUEEN].em_int = -1;
  (*s)[MATURING_QUEEN].task.force = 630.0;
  for (i = 0; i < NB_STIMULI; i++)
    {
      (*s)[i].task.brick = 0;
      (*s)[i].task.interruption = 0;
      (*s)[i].task.weight = 0;
      (*s)[i].task.weight_change = 0;
      (*s)[i].task.increment = 0;
      (*s)[i].task.threshold = 0;
    }
}

void remove_task2(task t)
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

void free_sti(behavior * stimuli)
{
  int i;

  for (i=0; i<NB_STIMULI; i++)
    remove_task2(stimuli[i].task);
  
  free(stimuli);
}


/********************************************** 
 **  AGENTS  ********************************** 
 **********************************************/

void init_agents(agent ** a,int * nb_ag)
{
  int i;
  
  *nb_ag = NB_AG;
  *a = malloc(NB_AG * (sizeof (agent)));

  (*a)[EGG].name_ag = "Oeuf"; 
  (*a)[EGG].ag = EGG;
  (*a)[EGG].ratio = 0.025;
  (*a)[EGG].nom_mod = "egg";
  (*a)[COCOON].name_ag = "Cocon"; 
  (*a)[COCOON].ag = COCOON;
  (*a)[COCOON].ratio = 0.025;
  (*a)[COCOON].nom_mod = "cocon";
  (*a)[LARVA].name_ag = "Larve"; 
  (*a)[LARVA].ag = LARVA;
  (*a)[LARVA].ratio = 0.05;
  (*a)[LARVA].nom_mod = "larva";
  (*a)[ANT].name_ag = "Ouvriere"; 
  (*a)[ANT].ag = ANT;
  (*a)[ANT].ratio = 0.05;
  (*a)[ANT].nom_mod = "fourmi";
  (*a)[QUEEN].name_ag = "Reine"; 
  (*a)[QUEEN].ag = QUEEN;
  (*a)[QUEEN].ratio = 0.1;
  (*a)[QUEEN].nom_mod = "fourmi";
  
  for (i=0; i<NB_AG; i++)
    (*a)[i].mod = 0;
}

void free_ag(agent * ag, int nb_ag UNUSED)
{
  free(ag);
}

/********************************************** 
 **  OBJETS  ********************************** 
 **********************************************/

void init_objets(objet ** o,int * nb_obj)
{
  int i;
  
  *nb_obj = NB_OBJ;
  *o = malloc(NB_OBJ * (sizeof (objet)));

  (*o)[NOURRITURE].name_obj = "Nourriture"; 
  (*o)[NOURRITURE].obj = NOURRITURE;
  (*o)[NOURRITURE].ratio = 0.05;
  (*o)[NOURRITURE].nom_mod = "bouffe";
  (*o)[NOURRITURE].crea_taux = 0.05;

  (*o)[LUM].name_obj = "Lumiere"; 
  (*o)[LUM].obj = LUM;
  (*o)[LUM].ratio = 0.05;
  (*o)[LUM].nom_mod = "lum";
  (*o)[LUM].crea_taux = 0.0;

  (*o)[HUM].name_obj = "Humidite"; 
  (*o)[HUM].obj = HUM;
  (*o)[HUM].ratio = 0.05;
  (*o)[HUM].nom_mod = "hum";
  (*o)[HUM].crea_taux = 0.0;
  
  for (i=0; i<NB_OBJ; i++)
    (*o)[i].mod = 0;
}

void free_objs(objet * ob,int nb_obj UNUSED)
{
  free(ob);
}

/********************************************** 
 **  ARGUMENTS  ******************************* 
 **********************************************/

void init_arg(argument ** arg, behavior * sti, agent * ag, objet * obj, int * nb_arg, int nb_ag, int nb_sti, int nb_obj)
{
  int i;
  
  *nb_arg = nb_ag + nb_sti + nb_obj;
  *arg = malloc((*nb_arg + 1) * (sizeof (argument)));

  (*arg)[0].name = "Aucun";
  (*arg)[0].id = -1;
  (*arg)[0].type = -69;

  (*arg)++;

  for (i = 0; i < nb_ag; i++)
    {
      (*arg)[i].name = ag[i].name_ag;
      (*arg)[i].id = i;
      (*arg)[i].type = AG;
    }
  for (i = 0; i < nb_obj; i++)
    {
      (*arg)[i+nb_ag].name = obj[i].name_obj;
      (*arg)[i+nb_ag].id = i;
      (*arg)[i+nb_ag].type = OBJ;
    }
  for (i = 0; i < nb_sti; i++)
    {
      (*arg)[i+nb_ag+nb_obj].name = sti[i].name_s;
      (*arg)[i+nb_ag+nb_obj].id = i;
      (*arg)[i+nb_ag+nb_obj].type = STI;
    }
}

void free_arg(argument * a,int nb_arg UNUSED)
{
  free(--a);
}


/********************************************** 
 **  INTERACTIONS  **************************** 
 **********************************************/

int ia_tuer(picked * clicked)
{
  if (clicked->type == IA_AGENT)
    die((t_agent) clicked->clicked,2);
  return(1);
}


int ia_bouffe(picked * clicked)
{
  creer_objet(objets[NOURRITURE].nom_mod, clicked->x, clicked->y, 0.,0.,0.,objets[NOURRITURE].ratio, NOURRITURE,1);

  return(1);
}

int ia_oeuf(picked * clicked)
{
  creer_agent(agents[EGG].nom_mod, clicked->x, clicked->y, 0.0, 0.0, 0.0, agents[EGG].ratio, EGG, 1);
  
  return(1);
}

int ia_mat(picked * clicked)
{
  if (clicked->type == IA_AGENT)
    mature((t_agent) clicked->clicked, 1);

  return(1);
}


/****  Le tableau et free  *********/

void init_iaction (iaction **ia,int *nb_ia)
{
  *nb_ia = NB_IA;
  *ia = malloc(NB_IA * (sizeof (iaction)));

  (*ia)[PICKING].name_ia = "Se deplacer";
  (*ia)[PICKING].ia = suivre;
  (*ia)[PICKING].i = 0;
  
  (*ia)[TUER].name_ia = "Tuer une fourmi";
  (*ia)[TUER].ia = ia_tuer;
  (*ia)[TUER].i = 1;

  (*ia)[IA_BF].name_ia = "Poser nourriture";
  (*ia)[IA_BF].ia = ia_bouffe;
  (*ia)[IA_BF].i = 2;

  (*ia)[IA_OF].name_ia = "Poser un oeuf";
  (*ia)[IA_OF].ia = ia_oeuf;
  (*ia)[IA_OF].i = 3;

  (*ia)[IA_MAT].name_ia = "Faire muer";
  (*ia)[IA_MAT].ia = ia_mat;
  (*ia)[IA_MAT].i = 4;
}

void free_iaction (iaction *ia,int nb_ia UNUSED)
{
  free(ia);
}

/**********************************************
 ** STATS *************************************
 **********************************************/


enum
  {
    STAT_AGENTS=0,
    STAT_EGG,
    STAT_COCOON,
    STAT_LARVA,
    STAT_ANT,
    STAT_QUEEN,
    STAT_AGE,
    STAT_TACHE,
    NB_STATS
  };

Statistiques

  Nom_abscisse(STAT_AGENTS) = "Cycles";
  Nom_ordonnee(STAT_AGENTS) = "Nombre d'agents";
  Nom_fichier(STAT_AGENTS) = "nb_agents";
  Type(STAT_AGENTS) = COURBE;

  Nom_abscisse(STAT_EGG) = "Cycles";
  Nom_ordonnee(STAT_EGG) = "Nombre d'oeufs";
  Nom_fichier(STAT_EGG) = "nb_oeuf";
  Type(STAT_EGG) = COURBE;

  Nom_abscisse(STAT_COCOON) = "Cycles";
  Nom_ordonnee(STAT_COCOON) = "Nombre de cocons";
  Nom_fichier(STAT_COCOON) = "nb_cocoon";
  Type(STAT_COCOON) = COURBE;

  Nom_abscisse(STAT_LARVA) = "Cycles";
  Nom_ordonnee(STAT_LARVA) = "Nombre de larves";
  Nom_fichier(STAT_LARVA) = "nb_larva";
  Type(STAT_LARVA) = COURBE;

  Nom_abscisse(STAT_ANT) = "Cycles";
  Nom_ordonnee(STAT_ANT) = "Nombre d'ouvrieres";
  Nom_fichier(STAT_ANT) = "nb_ant";
  Type(STAT_ANT) = COURBE;

  Nom_abscisse(STAT_QUEEN) = "Cycles";
  Nom_ordonnee(STAT_QUEEN) = "Nombre de reines";
  Nom_fichier(STAT_QUEEN) = "nb_queen";
  Type(STAT_QUEEN) = COURBE;

  Nom_abscisse(STAT_AGE) = "Age";
  Nom_ordonnee(STAT_AGE) = "Nombre d'agents";
  Nom_fichier(STAT_AGE) = "st_age";
  Type(STAT_AGE) = HISTOGRAMME;

  Nom_abscisse(STAT_TACHE) = "Tache";
  Nom_ordonnee(STAT_TACHE) = "Nombre d'agents";
  Nom_fichier(STAT_TACHE) = "st_tache";
  Type(STAT_TACHE) = HISTOGRAMME;

Fin

/********************************************** 
 **  ACTIONS SUR OBJS ET AGENTS  **************
 **********************************************/

/* Pour les action executees a chaque tour definies par l'utilisateur */
void user_ta (t_agent agent)
{
  if (agent != NULL)
    {
      float dx,dz,angle,s;
      
      if ((dx = agent->xgoto - agent->x) != 0.0)
	{
	  dz = agent->zgoto - agent->z;
	  s = sqrt(dx*dx+dz*dz);
	  if (s)
	    angle = RAD * acos(dx/s) * ((dz > 0.0)?1.0:(-1.0));
	  else
	    if ((dz = agent->zgoto - agent->z) != 0.0)
	      angle = 90 * ((dz > 0.0)?1.0:(-1.0));
	}
      else
	if ((dz = agent->zgoto - agent->z) != 0.0)
	  angle = 90 * ((dz > 0.0)?1.0:(-1.0));
  
      agent->ry = angle;

      /* avancer : */
      agent->x += my_cos(agent->ry) * agent->speed * tps;
      agent->z += my_sin(agent->ry) * agent->speed * tps;
      agent->speed = 0.0;
    }
}

void user_to (t_objet objet UNUSED)
{

}

void user_cycle_ta (t_agent agent UNUSED)
{
}

void user_cycle_to (t_objet objet UNUSED)
{
  float force;
  int t1[] = {ANT_FOOD,QUEEN_FOOD};
  int t2[] = {EGG_LIGHT,LARVA_LIGHT,COCOON_LIGHT,ANT_LIGHT,QUEEN_LIGHT};
  int t3[] = {EGG_HUMIDITY,LARVA_HUMIDITY,COCOON_HUMIDITY,ANT_HUMIDITY,QUEEN_HUMIDITY};
  
  if (!objet)
    return;
  if (objet->type == NOURRITURE)
    {
      objet->valeur -= 0.5;
      if (objet->valeur > 0.0 && !objet->is_caddie)
	{
	  force = objet->valeur / 20.0; 
	  prop_mega(objet->case_x,objet->case_y,2,t1,force,0.5);
	}
    }
  /*pas sur de devoir le repropager a chaque cycle, ou alors on change sa valeur (alternance jour/nuit ...)*/
  if (objet->type == LUM)
    {
      force = objet->valeur / 20.0; 
      prop_mega(objet->case_x,objet->case_y,5,t2,force,1.0);
    }

  
  if (objet->type == HUM)
    {
      force = objet->valeur;
      prop_mega(objet->case_x,objet->case_y,5,t3,force,0.5);
    }
}

void user_cycle_gena (t_agent agent)
{
  static int tab_count[NB_AG + 1]={0};
  int i;

  if (agent != NULL)
    {
      tab_count[NB_AG]++;
      tab_count[agent->type]++;
    }
  else
    {
      Add_courbe(STAT_AGENTS,(double) tab_count[NB_AG]);
      Add_courbe(STAT_EGG,(double) tab_count[EGG]);
      Add_courbe(STAT_COCOON, (double) tab_count[COCOON]);
      Add_courbe(STAT_LARVA, (double) tab_count[LARVA]);
      Add_courbe(STAT_ANT,(double) tab_count[ANT]);
      Add_courbe(STAT_QUEEN,(double)tab_count[QUEEN]);
      for (i = 0;i < NB_AG + 1;i++)
	tab_count[i] = 0;
    }
}

void user_cycle_geno (t_objet objet)
{
  static int compteur[NB_OBJ] = {0};
  int i,j,x,y,essais=0;
  double taux;
  
  if (objet != NULL)
    compteur[objet->type]++;
  else
    {
      for (i=0;i<nb_obj;i++)
	{
	  taux = ((double) compteur[i] / (double)carte.x_max / (double)carte.y_max );
	  if (taux < objets[i].crea_taux)
	    {
	      taux = (objets[i].crea_taux - taux) * carte.x_max * carte.y_max;
	      for (j=0;j<(int)taux;j++)
		{
		  while (essais < 5)
		    {
		      x = my_rand(carte.x_max)-1;
		      y = my_rand(carte.y_max)-1;
		    if (cases[carte.tab[x][y].type].accessible == 2)
		      {
			creer_objet(objets[i].nom_mod,x, y, 0.,0.,0.,objets[i].ratio,i,1);
			break;
		      }
		    essais++;
		    }
		}
	    }
	  compteur[i] = 0;
	}
    }
}

void stat_ag (t_agent agent)
{
  Add_histo(STAT_AGE,agent->age);
  Add_histo(STAT_TACHE,agent->tache);
}

void stat_obj (t_objet objet UNUSED)
{

}

void stat_map (t_case * kase UNUSED)
{

}
