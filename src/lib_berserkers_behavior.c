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

#define SPEED 3.0

#define Add_courbe(x,y) stats[x].valeurs.ordonnee[current_stat] = y
#define Add_histo(x,y) spush(x,y)
#define Add_courbe_safe(x,y) add_courbe_stat(x,y)
#define Add_histo_safe(x,y) add_histo_stat(x,y)

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
    BERSERK1_NOTHING_TO_DO=0,
    BERSERK2_NOTHING_TO_DO,
    JOUEUR_NOTHING_TO_DO,
    GOSSE_BERSERK1_NOTHING_TO_DO,
    GOSSE_BERSERK2_NOTHING_TO_DO,
    FEMME_NOTHING_TO_DO,
    /*berserk1*/
    BERSERK1_BERSERK2,
    BERSERK1_HEALTH,
    BERSERK1_PRESENCE,
    BERSERK1_FEMME,
    BERSERK1_FEMME_PORTEE,
    BERSERK1_GOSSE,
    BERSERK1_GOSSE_BERSERK2,
    /*berserk2*/
    BERSERK2_BERSERK1,
    BERSERK2_HEALTH,
    BERSERK2_PRESENCE,
    BERSERK2_FEMME,
    BERSERK2_FEMME_PORTEE,
    BERSERK2_GOSSE,
    BERSERK2_GOSSE_BERSERK1,
    /*joueur*/
    JOUEUR_MARCHER,
    JOUEUR_FRAPPER,
    /*berserk1 gosse*/
    GOSSE_BERSERK1_HEALTH,
    GOSSE_BERSERK1_PRESENCE,
    GOSSE_BERSERK1_GRANDIR,
    /*berserk2 gosse*/
    GOSSE_BERSERK2_HEALTH,
    GOSSE_BERSERK2_PRESENCE,
    GOSSE_BERSERK2_GRANDIR,
    /*femme*/
    FEMME_PRESENCE,
    NB_STIMULI
  };



/* Agents */
enum 
  {
    BERSERK1=0,
    BERSERK2,
    JOUEUR,
    GOSSE_BERSERK1,
    GOSSE_BERSERK2,
    FEMME,
    NB_AG
  };



/* Objets */
enum 
  {
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
    DIE,
    IS_HEALTHY,
    IS_PRESENCE,
    FOLLOW_S,
    STRIKE,
    PICK_UP,
    PUT_DOWN,
    IS_GRANDIR,
    MATURE,
    IS_HAVE_SEX,
    LAY_GOSSE,
    J_AVANCER,
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
    IA_AV,
    IA_FR,
    NB_IA
  };



/* sol */
enum 
  {
    SOL=0,                   
    MUR,
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
  (*c)[SOL].texture = Load_image("snow.tga");
  (*c)[SOL].model = NULL;
  (*c)[SOL].objet = NULL;
  (*c)[SOL].accessible = 2;   

  (*c)[MUR].name = "Mur";
  (*c)[MUR].texture = Load_image("fourmiliere.bmp");
  (*c)[MUR].model = "cube";
  (*c)[MUR].objet = load_objet("cube",0.25);
  (*c)[MUR].accessible = 0;

}

void free_case (ccase * cases, int nb_case UNUSED)
{
  free(cases);
}


/********************************************** 
 **  INIT OPERATORS  ************************** 
 **********************************************/

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


enum
  {
    AN_RIEN=0,
    AN_RUN,
    AN_ATTACK,
    AN_DIE
  };
#define DR 1
#define FR 11
#define DA 12
#define FA 25
#define DD 26
#define FD 41

static inline void get_frame(t_agent ag, int anim)
{
  ag->ac_tps -= tps * ag->speed / 3.0;
  ag->ac_tps += tps * SPEED / 50.0;

  if (anim == AN_RIEN)
    {
      ag->step = 0;
      return;
    }
  if (anim == AN_RUN)
    {
      if (ag->step > FR)
	ag->step = DR;
      else 
	if (ag->step < DR)
	  ag->step = DR;
      return;
    }
  if (anim == AN_ATTACK)
    {
      if (ag->step > FA)
	ag->step = DA;
      else 
	if (ag->step < DA)
	  ag->step = DA;
      return;
    }
  if (anim == AN_DIE)
    {
      if (ag->step > FD)
	ag->step = DD;
      else 
	if (ag->step < DD)
	  ag->step = DD;
      return;
    }
}



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


int random_walk(t_agent agent, int arg UNUSED)
{
  int i,j,k,t[]={165, 145, 115, 85, 65, 45, 30, 15, 5};
  x_et_y tabu[9],piou;

  if(!agent->is_caddie)
    {
      agent->speed = SPEED / 2.0;
      if (((int)(agent->x / TAILLE_CASE) == (int)(agent->xgoto / TAILLE_CASE)) && ((int) (agent->z / TAILLE_CASE) == (int)(agent->zgoto / TAILLE_CASE)))
	{
	  i = autour(agent->case_x,agent->case_y,agent->ry,tabu);
	  if (i-1)
	    {
	      j = my_rand(t[0]);
	      for (k = i - 1; k >= 0; k--)
		if (t[k] >= j)
		  break;
	      piou = tabu[k];
	      agent->xgoto = (piou.x + 0.5) * TAILLE_CASE;
	  agent->zgoto = (piou.y + 0.5) * TAILLE_CASE;
	    }
	}
    }
  return(0);
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

  return 1;
}


int die(t_agent agent, int arg UNUSED)
{ 
  int an;

  an = agent->step == FD;
  get_frame(agent, AN_DIE);
  if (an)
    {
      put_down(agent,1);
      agent->age *= (agent->age > 0)?-1:1;
      return 1;
    }
  return 0;
}


/*utilisé par STRIKE*/
int mkill(t_agent agent, int arg) 
{
  t_case c;
  int i, an, combien;

  c = carte.tab[agent->case_x][agent->case_y];
  i = 0;

  get_frame(agent, AN_ATTACK);
  an = agent->step == FA;

  if (agent->type == JOUEUR)
    combien = 500;
  else
    combien = 10;

  for (i = 0; i < c.nbr_ag; i++)
    if ((arguments[arg].id == c.agents[i]->type || arg == -1) && c.agents[i] != agent)
      {
	if (arguments[arg].id == BERSERK1 || (arguments[arg].id == -1 && c.agents[i]->type == BERSERK1))
	  {
	    c.agents[i]->stimuli[BERSERK1_HEALTH].force -= combien;	 
	    return 0;
	  }
	if (arguments[arg].id == BERSERK2 || (arguments[arg].id == -1 && c.agents[i]->type == BERSERK2))
	  {
	    c.agents[i]->stimuli[BERSERK2_HEALTH].force -= combien;	 
	    return 0;
	  }
	if (arguments[arg].id == GOSSE_BERSERK1 || (arguments[arg].id == -1 && c.agents[i]->type == GOSSE_BERSERK1))
	  {
	    c.agents[i]->stimuli[GOSSE_BERSERK1_HEALTH].force -= combien;	 
	    return 0;
	  }
	if (arguments[arg].id == GOSSE_BERSERK2 || (arguments[arg].id == -1 && c.agents[i]->type == GOSSE_BERSERK2))
	  {
	    c.agents[i]->stimuli[GOSSE_BERSERK2_HEALTH].force -= combien;	 
	    return 0;
	  }
      }
  if (an)
    {
      agent->step = 0;
      return 1;
    }
  return 0;
}

  
int pick_up(t_agent agent, int arg)
{
  t_case c;
  int i;
  t_agent ca;
  
  c = carte.tab[agent->case_x][agent->case_y];
  i = 0;
  
  if (arguments[arg].type == AG && !agent->caddie)
    {
      if (!c.nbr_ag)
	return 1;
      ca = c.agents[0];
      for (i = 0; i < c.nbr_ag; i++)
	if (arguments[arg].id == c.agents[i]->type && agent != c.agents[i])
	  ca = c.agents[i];
      if (arguments[arg].id == ca->type && agent != ca)
	{
	  agent->caddie = ca;
	  agent->is_caddie_ag = 1;
	  ca->is_caddie = 1;
	  del_agent(agent->case_x,agent->case_y,ca);
	}
    }
  return 1;
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

int is_healthy(t_agent agent, int arg UNUSED)
{
  int i;

  if((agent->type == BERSERK1)&&(agent->stimuli[BERSERK1_HEALTH].force <= 0))
    {
      for(i=0;i<NB_STIMULI;i++)
	{
	  agent->stimuli[i].poids = 0;
	  agent->stimuli[i].seuil = 5000;
	}
      agent->stimuli[BERSERK1_HEALTH].poids = 5000;
      agent->stimuli[BERSERK1_HEALTH].seuil = 0;
      prop_e(agent->case_x,agent->case_y,BERSERK1_HEALTH,1,1);
      return 1;
    }
  if((agent->type == BERSERK2)&&(agent->stimuli[BERSERK2_HEALTH].force <= 0))
    {
      for(i=0;i<NB_STIMULI;i++)
	{
	  agent->stimuli[i].poids = 0;
	  agent->stimuli[i].seuil = 5000;
	}
      agent->stimuli[BERSERK2_HEALTH].poids = 5000;
      agent->stimuli[BERSERK2_HEALTH].seuil = 0;
      prop_e(agent->case_x,agent->case_y,BERSERK2_HEALTH,1,1);
      return 1;
    }
  if((agent->type == GOSSE_BERSERK1)&&(agent->stimuli[GOSSE_BERSERK1_HEALTH].force <= 0))
    {
      for(i=0;i<NB_STIMULI;i++)
	{
	  agent->stimuli[i].poids = 0;
	  agent->stimuli[i].seuil = 5000;
	}
      agent->stimuli[GOSSE_BERSERK1_HEALTH].poids = 5000;
      agent->stimuli[GOSSE_BERSERK1_HEALTH].seuil = 0;
      prop_e(agent->case_x,agent->case_y,GOSSE_BERSERK1_HEALTH,1,1);
      return 1;
    }
  if((agent->type == GOSSE_BERSERK2)&&(agent->stimuli[GOSSE_BERSERK2_HEALTH].force <= 0))
    {

      for(i=0;i<NB_STIMULI;i++)
	{
	  agent->stimuli[i].poids = 0;
	  agent->stimuli[i].seuil = 5000;
	}
      agent->stimuli[GOSSE_BERSERK2_HEALTH].poids = 5000;
      agent->stimuli[GOSSE_BERSERK2_HEALTH].seuil = 0;
      prop_e(agent->case_x,agent->case_y,GOSSE_BERSERK2_HEALTH,1,1);
    }
  
  return 1;
}


int is_presence(t_agent agent, int arg UNUSED)
{
  if(agent->type == BERSERK1)
    {
      prop_e(agent->case_x, agent->case_y,BERSERK2_BERSERK1,5,1);
      if(agent->caddie && agent->is_caddie_ag && (((t_agent)agent->caddie)->type == FEMME))
	prop_e(agent->case_x, agent->case_y,BERSERK2_FEMME_PORTEE,10,1);
      return 1;
    }
  if(agent->type == BERSERK2)
    {
      prop_e(agent->case_x, agent->case_y,BERSERK1_BERSERK2,5,1);
      if(agent->caddie && agent->is_caddie_ag && (((t_agent)agent->caddie)->type == FEMME))
	prop_e(agent->case_x, agent->case_y,BERSERK1_FEMME_PORTEE,10,1);
      return 1;
    }

  if(agent->type == GOSSE_BERSERK1)
    {
      prop_e(agent->case_x, agent->case_y,BERSERK2_GOSSE_BERSERK1,4,1);
      return 1;
    }

  if(agent->type == GOSSE_BERSERK2)
    {
      prop_e(agent->case_x, agent->case_y,BERSERK1_GOSSE_BERSERK2,4,1);
      return 1;
    }

  if((agent->type == FEMME)&&(!agent->is_caddie))
    {
      prop_e(agent->case_x, agent->case_y,BERSERK1_FEMME,6,1);
      prop_e(agent->case_x, agent->case_y,BERSERK2_FEMME,6,1);
    }
  return 1;
}

int is_have_sex(t_agent agent, int arg UNUSED)
{
  float force;

  if(agent->type == BERSERK1)
    {
      if((agent->caddie)&&((t_agent)agent->caddie)->type == FEMME)
	{
	  agent->stimuli[BERSERK1_GOSSE].force -= 1;
	  if(agent->stimuli[BERSERK1_GOSSE].force <= 1)
	    agent->stimuli[BERSERK1_GOSSE].force = 1;
	  if(agent->stimuli[BERSERK1_GOSSE].force <= 20)
	    {
	      force = 20/(agent->stimuli[BERSERK1_GOSSE].force * 2);
	      prop_e(agent->case_x, agent->case_y,BERSERK1_GOSSE,force,force);
	    }
	}
      return 1;
    }
  
  if(agent->type == BERSERK2)
    {
      if((agent->caddie)&&((t_agent)agent->caddie)->type == FEMME)
	{	
	  agent->stimuli[BERSERK2_GOSSE].force -= 1;
	  if(agent->stimuli[BERSERK2_GOSSE].force <= 1)
	    agent->stimuli[BERSERK2_GOSSE].force = 1;
	  if(agent->stimuli[BERSERK2_GOSSE].force <= 20)
	    {
	      force = 20/(agent->stimuli[BERSERK2_GOSSE].force * 2);
	      prop_e(agent->case_x, agent->case_y, BERSERK2_GOSSE,force,force);
	    }
	}
    }
  return 1;
}



int pondre(t_agent agent, int arg UNUSED)
{
  if(agent->caddie && ((t_agent)agent->caddie)->type == FEMME)
    {
      if(agent->type == BERSERK1)
	{
	  creer_agent(agents[GOSSE_BERSERK1].nom_mod,agent->case_x,agent->case_y,0,0,0,agents[GOSSE_BERSERK1].ratio,GOSSE_BERSERK1,1);
	  agent->stimuli[BERSERK1_GOSSE].force = stimuli[BERSERK1_GOSSE].task.force;
	  return 1;
	}
      
      if(agent->type == BERSERK2)
	{
	  //creer_agent("fourmi",agent->case_x,agent->case_y,0,0,0,0.05,GOSSE_BERSERK2,1);
	  creer_agent(agents[GOSSE_BERSERK2].nom_mod,agent->case_x,agent->case_y,0,0,0,agents[GOSSE_BERSERK2].ratio,GOSSE_BERSERK2,1);
	  agent->stimuli[BERSERK2_GOSSE].force = stimuli[BERSERK2_GOSSE].task.force;
	}
    }
  return 1;
}


int is_grandir(t_agent agent, int arg UNUSED)
{

  float force;
 
  if (agent->type == GOSSE_BERSERK1)
    {
      agent->stimuli[GOSSE_BERSERK1_GRANDIR].force -= 1.0;
      if (agent->stimuli[GOSSE_BERSERK1_GRANDIR].force <= 0)
	{
	  agent->stimuli[GOSSE_BERSERK1_GRANDIR].seuil = 0.0; 
	  if (!agent->is_caddie)
	    {
	      force = 40;
	      prop_e(agent->case_x, agent->case_y,GOSSE_BERSERK1_GRANDIR,force,force);
	    }
	}
      else
	agent->stimuli[GOSSE_BERSERK1_GRANDIR].seuil = 5000.0;
      return 1;
    }

    if (agent->type == GOSSE_BERSERK2)
    {
      agent->stimuli[GOSSE_BERSERK2_GRANDIR].force -= 1.0;
      if (agent->stimuli[GOSSE_BERSERK2_GRANDIR].force <= 0)
	{
	  agent->stimuli[GOSSE_BERSERK2_GRANDIR].seuil = 0.0; 
	  if (!agent->is_caddie)
	    {
	      force = 40;
	      prop_e(agent->case_x, agent->case_y,GOSSE_BERSERK2_GRANDIR,force,force);
	    }
	}
      else
	agent->stimuli[GOSSE_BERSERK2_GRANDIR].seuil = 5000.0;
    }
  return 1;
}


int mature(t_agent agent, int arg UNUSED)
{
  if(agent->type == GOSSE_BERSERK1)
    {
      agent->type = BERSERK1;
      agent->ratio = agents[BERSERK1].ratio;
      agent->model = create_3Dobj(agents[BERSERK1].nom_mod);
      return 1;
    }

  if(agent->type == GOSSE_BERSERK2)
    {
      agent->type = BERSERK2;
      agent->ratio = agents[BERSERK2].ratio;
      agent->model = create_3Dobj(agents[BERSERK2].nom_mod);
    }
  return 1;
}

int avancer(t_agent ag, int arg UNUSED)
{
  if (fabsf(ag->x - ag->xgoto) >= TAILLE_CASE / 2.0 ||
      fabsf(ag->z - ag->zgoto) >= TAILLE_CASE / 2.0)
    {
      ag->speed = SPEED;
      return 0;
    }
  ag->step = 0;
  ag->speed = 0.0;
  return 1;
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
  s[RANDOM_WALK] = "p_random.png";
  (*p)[RANDOM_WALK].bboard = Load_image("random.tga");

  (*p)[DIE].name_p = "Mourir";
  (*p)[DIE].p = die;
  s[DIE] = "p_mort.png";
  (*p)[DIE].bboard = Load_image("mort.tga");


  (*p)[PICK_UP].name_p = "Ramasser";
  (*p)[PICK_UP].p = pick_up;
  s[PICK_UP] = "p_pickup.png";              
  (*p)[PICK_UP].bboard = Load_image("pickup.tga");


  (*p)[PUT_DOWN].name_p = "Poser";
  (*p)[PUT_DOWN].p = put_down;
  s[PUT_DOWN] = "p_putdown.png";           
  (*p)[PUT_DOWN].bboard = Load_image("putdown.tga");


  (*p)[IS_HEALTHY].name_p = "Niveau Sante";
  (*p)[IS_HEALTHY].p = is_healthy;
  s[IS_HEALTHY] = "sante.png";
  (*p)[IS_HEALTHY].bboard = Load_image("sante.tga");    

  (*p)[IS_PRESENCE].name_p = "Presence";
  (*p)[IS_PRESENCE].p = is_presence;
  s[IS_PRESENCE] = "p_presence.png";
  (*p)[IS_PRESENCE].bboard = Load_image("confort.tga");

  (*p)[FOLLOW_S].name_p = "Suivre stimulus"; 
  (*p)[FOLLOW_S].p = follow;
  s[FOLLOW_S] = "p_follow.png";
  (*p)[FOLLOW_S].bboard = Load_image("follow.tga");

  (*p)[IS_GRANDIR].name_p = "Age";       
  (*p)[IS_GRANDIR].p = is_grandir;
  s[IS_GRANDIR] = "vieillir.png";             
  (*p)[IS_GRANDIR].bboard = Load_image("vieillir.tga"); 

  (*p)[MATURE].name_p = "Grandir"; 
  (*p)[MATURE].p = mature;
  s[MATURE] = "p_mature.png";                 
  (*p)[MATURE].bboard = Load_image("mature.tga");

  (*p)[STRIKE].name_p = "Frapper";
  (*p)[STRIKE].p = mkill;
  s[STRIKE] = "p_kill.png";
  (*p)[STRIKE].bboard = Load_image("kill.tga");

  (*p)[IS_HAVE_SEX].name_p = "Faire un enfant?";   
  (*p)[IS_HAVE_SEX].p = is_have_sex;
  s[IS_HAVE_SEX] = "p_heart.png";                
  (*p)[IS_HAVE_SEX].bboard = Load_image("p_heart.tga");      

  (*p)[LAY_GOSSE].name_p = "Se reproduire";
  (*p)[LAY_GOSSE].p = pondre;
  s[LAY_GOSSE] = "p_reproduction.png";                  
  (*p)[LAY_GOSSE].bboard = Load_image("p_reproduction.tga"); 

  (*p)[J_AVANCER].name_p = "Faire avancer joueur";
  (*p)[J_AVANCER].p = avancer;
  s[J_AVANCER] = "p_avancer.png";                
  (*p)[J_AVANCER].bboard = Load_image("p_avancer.tga");

  

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
    caca("free_op, truc louche");
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

  (*s)[BERSERK1_NOTHING_TO_DO].name_s = "Berserk1:Rien a faire";
  (*s)[BERSERK1_NOTHING_TO_DO].ag = BERSERK1;
  (*s)[BERSERK1_NOTHING_TO_DO].interne = 0;
  (*s)[BERSERK1_NOTHING_TO_DO].task.force = INUTILISE;

  (*s)[BERSERK2_NOTHING_TO_DO].name_s = "Berserk2:Rien a faire";
  (*s)[BERSERK2_NOTHING_TO_DO].ag = BERSERK2;
  (*s)[BERSERK2_NOTHING_TO_DO].interne = 0;
  (*s)[BERSERK2_NOTHING_TO_DO].task.force = INUTILISE;

  (*s)[JOUEUR_NOTHING_TO_DO].name_s = "Joueur:Rien a faire";
  (*s)[JOUEUR_NOTHING_TO_DO].ag = JOUEUR;
  (*s)[JOUEUR_NOTHING_TO_DO].interne = 0;
  (*s)[JOUEUR_NOTHING_TO_DO].task.force = INUTILISE;

  (*s)[JOUEUR_MARCHER].name_s = "Joueur:Marcher";
  (*s)[JOUEUR_MARCHER].ag = JOUEUR;
  (*s)[JOUEUR_MARCHER].interne = 0;
  (*s)[JOUEUR_MARCHER].task.force = INUTILISE;

  (*s)[JOUEUR_FRAPPER].name_s = "Joueur:Frapper";
  (*s)[JOUEUR_FRAPPER].ag = JOUEUR;
  (*s)[JOUEUR_FRAPPER].interne = 0;
  (*s)[JOUEUR_FRAPPER].task.force = INUTILISE;

  (*s)[GOSSE_BERSERK1_NOTHING_TO_DO].name_s = "Gosse1:Rien a faire";
  (*s)[GOSSE_BERSERK1_NOTHING_TO_DO].ag = GOSSE_BERSERK1;
  (*s)[GOSSE_BERSERK1_NOTHING_TO_DO].interne = 0;
  (*s)[GOSSE_BERSERK1_NOTHING_TO_DO].task.force = INUTILISE;

  (*s)[GOSSE_BERSERK2_NOTHING_TO_DO].name_s = "Gosse2:Rien a faire";
  (*s)[GOSSE_BERSERK2_NOTHING_TO_DO].ag = GOSSE_BERSERK2;
  (*s)[GOSSE_BERSERK2_NOTHING_TO_DO].interne = 0;
  (*s)[GOSSE_BERSERK2_NOTHING_TO_DO].task.force = INUTILISE;

  (*s)[FEMME_NOTHING_TO_DO].name_s = "Femme:Rien a faire";
  (*s)[FEMME_NOTHING_TO_DO].ag = FEMME;
  (*s)[FEMME_NOTHING_TO_DO].interne = 0;
  (*s)[FEMME_NOTHING_TO_DO].task.force = INUTILISE;

  (*s)[BERSERK1_BERSERK2].name_s = "Berserk1:Berserk2";
  (*s)[BERSERK1_BERSERK2].ag = BERSERK1;
  (*s)[BERSERK1_BERSERK2].interne = 0;
  (*s)[BERSERK1_BERSERK2].task.force = INUTILISE;

  (*s)[BERSERK2_BERSERK1].name_s = "Berserk2:Berserk1";
  (*s)[BERSERK2_BERSERK1].ag = BERSERK2;
  (*s)[BERSERK2_BERSERK1].interne = 0;
  (*s)[BERSERK2_BERSERK1].task.force = INUTILISE;

  (*s)[BERSERK1_HEALTH].name_s = "Berserk1:Sante";
  (*s)[BERSERK1_HEALTH].ag = BERSERK1;
  (*s)[BERSERK1_HEALTH].interne = 1;
  (*s)[BERSERK1_HEALTH].task.force = 150;

  (*s)[BERSERK2_HEALTH].name_s = "Berserk2:Sante";
  (*s)[BERSERK2_HEALTH].ag = BERSERK2;
  (*s)[BERSERK2_HEALTH].interne = 1;
  (*s)[BERSERK2_HEALTH].task.force = 150;

  (*s)[GOSSE_BERSERK1_HEALTH].name_s = "Gosse1:Sante";
  (*s)[GOSSE_BERSERK1_HEALTH].ag = GOSSE_BERSERK1;
  (*s)[GOSSE_BERSERK1_HEALTH].interne = 1;
  (*s)[GOSSE_BERSERK1_HEALTH].task.force = 100;

  (*s)[GOSSE_BERSERK2_HEALTH].name_s = "Gosse2:Sante";
  (*s)[GOSSE_BERSERK2_HEALTH].ag = GOSSE_BERSERK2;
  (*s)[GOSSE_BERSERK2_HEALTH].interne = 1;
  (*s)[GOSSE_BERSERK2_HEALTH].task.force = 100;


  (*s)[BERSERK1_PRESENCE].name_s = "Berserk1:Presence";
  (*s)[BERSERK1_PRESENCE].ag = BERSERK1;
  (*s)[BERSERK1_PRESENCE].interne = 1;
  (*s)[BERSERK1_PRESENCE].task.force = INUTILISE;


  (*s)[BERSERK2_PRESENCE].name_s = "Berserk2:Presence";
  (*s)[BERSERK2_PRESENCE].ag = BERSERK2;
  (*s)[BERSERK2_PRESENCE].interne = 1;
  (*s)[BERSERK2_PRESENCE].task.force = INUTILISE;

  (*s)[GOSSE_BERSERK1_PRESENCE].name_s = "Gosse1:Presence";
  (*s)[GOSSE_BERSERK1_PRESENCE].ag = GOSSE_BERSERK1;
  (*s)[GOSSE_BERSERK1_PRESENCE].interne = 1;
  (*s)[GOSSE_BERSERK1_PRESENCE].task.force = INUTILISE;

  (*s)[GOSSE_BERSERK2_PRESENCE].name_s = "Gosse2:Presence";
  (*s)[GOSSE_BERSERK2_PRESENCE].ag = GOSSE_BERSERK2;
  (*s)[GOSSE_BERSERK2_PRESENCE].interne = 1;
  (*s)[GOSSE_BERSERK2_PRESENCE].task.force = INUTILISE;

  (*s)[GOSSE_BERSERK1_GRANDIR].name_s = "Gosse1:Grandir";
  (*s)[GOSSE_BERSERK1_GRANDIR].ag = GOSSE_BERSERK1;
  (*s)[GOSSE_BERSERK1_GRANDIR].interne = 1;
  (*s)[GOSSE_BERSERK1_GRANDIR].task.force = 20;

  (*s)[GOSSE_BERSERK2_GRANDIR].name_s = "Gosse2:Grandir";
  (*s)[GOSSE_BERSERK2_GRANDIR].ag = GOSSE_BERSERK2;
  (*s)[GOSSE_BERSERK2_GRANDIR].interne = 1;
  (*s)[GOSSE_BERSERK2_GRANDIR].task.force = 20;


  (*s)[FEMME_PRESENCE].name_s = "Femme:Presence";
  (*s)[FEMME_PRESENCE].ag = FEMME;
  (*s)[FEMME_PRESENCE].interne = 1;
  (*s)[FEMME_PRESENCE].task.force = INUTILISE;

  (*s)[BERSERK1_FEMME].name_s = "Berserk1:Femme";
  (*s)[BERSERK1_FEMME].ag = BERSERK1;
  (*s)[BERSERK1_FEMME].interne = 0;
  (*s)[BERSERK1_FEMME].task.force = INUTILISE;

  (*s)[BERSERK2_FEMME].name_s = "Berserk2:Femme";
  (*s)[BERSERK2_FEMME].ag = BERSERK2;
  (*s)[BERSERK2_FEMME].interne = 0;
  (*s)[BERSERK2_FEMME].task.force = INUTILISE;

  (*s)[BERSERK1_FEMME_PORTEE].name_s = "Berserk1:Enlevement";
  (*s)[BERSERK1_FEMME_PORTEE].ag = BERSERK1;
  (*s)[BERSERK1_FEMME_PORTEE].interne = 0;
  (*s)[BERSERK1_FEMME_PORTEE].task.force = INUTILISE;

  (*s)[BERSERK2_FEMME_PORTEE].name_s = "Berserk2:Enlevement";
  (*s)[BERSERK2_FEMME_PORTEE].ag = BERSERK2;
  (*s)[BERSERK2_FEMME_PORTEE].interne = 0;
  (*s)[BERSERK2_FEMME_PORTEE].task.force = INUTILISE;

  (*s)[BERSERK1_GOSSE].name_s = "Berserk1:Reproduction";
  (*s)[BERSERK1_GOSSE].ag = BERSERK1;
  (*s)[BERSERK1_GOSSE].interne = 1;
  (*s)[BERSERK1_GOSSE].task.force = 50;

  (*s)[BERSERK2_GOSSE].name_s = "Berserk2:Reproduction";
  (*s)[BERSERK2_GOSSE].ag = BERSERK2;
  (*s)[BERSERK2_GOSSE].interne = 1;
  (*s)[BERSERK2_GOSSE].task.force = 50;

  (*s)[BERSERK1_GOSSE_BERSERK2].name_s = "Berserk1:Enfant2";
  (*s)[BERSERK1_GOSSE_BERSERK2].ag = BERSERK1;
  (*s)[BERSERK1_GOSSE_BERSERK2].interne = 0;
  (*s)[BERSERK1_GOSSE_BERSERK2].task.force = INUTILISE;

  (*s)[BERSERK2_GOSSE_BERSERK1].name_s = "Berserk2:Enfant1";
  (*s)[BERSERK2_GOSSE_BERSERK1].ag = BERSERK2;
  (*s)[BERSERK2_GOSSE_BERSERK1].interne = 0;
  (*s)[BERSERK2_GOSSE_BERSERK1].task.force = INUTILISE;


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

  (*a)[BERSERK1].name_ag = "Berserk1"; 
  (*a)[BERSERK1].ag = BERSERK1;
  (*a)[BERSERK1].ratio = 0.15;
  (*a)[BERSERK1].nom_mod = "red-berserk";

  (*a)[BERSERK2].name_ag = "Berserk2"; 
  (*a)[BERSERK2].ag = BERSERK2;
  (*a)[BERSERK2].ratio = 0.15;
  (*a)[BERSERK2].nom_mod = "wh-berserk";

  (*a)[JOUEUR].name_ag = "Joueur"; 
  (*a)[JOUEUR].ag = JOUEUR;
  (*a)[JOUEUR].ratio = 0.15;
  (*a)[JOUEUR].nom_mod = "bl-berserk";

  (*a)[GOSSE_BERSERK1].name_ag = "Enfant1"; 
  (*a)[GOSSE_BERSERK1].ag = GOSSE_BERSERK1;
  (*a)[GOSSE_BERSERK1].ratio = 0.05;
  (*a)[GOSSE_BERSERK1].nom_mod = "red-berserk";

  (*a)[GOSSE_BERSERK2].name_ag = "Enfant2"; 
  (*a)[GOSSE_BERSERK2].ag = GOSSE_BERSERK2;
  (*a)[GOSSE_BERSERK2].ratio = 0.05;
  (*a)[GOSSE_BERSERK2].nom_mod = "red-berserk";

  (*a)[FEMME].name_ag = "Femme"; 
  (*a)[FEMME].ag = FEMME;
  (*a)[FEMME].ratio = 0.15;
  (*a)[FEMME].nom_mod = "pk-berserk";

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

extern t_agent liste_agent;
static inline t_agent trouver_joueur()
{
  t_agent l;

  l = liste_agent;
  while (l)
    {
      if (l->type == JOUEUR)
	return l;
      l = l->next;
    }
  return 0;
}

int ia_marcher(picked *c)
{
  t_agent j;

  if ((j = trouver_joueur()))
    {
      if (c->clicked == j)
	{
	  j->en_cours = 1;
	  j->tache = JOUEUR_FRAPPER;
	  return 1;
	}
      j->xgoto = (c->x + 0.5) * TAILLE_CASE;
      j->zgoto = (c->y + 0.5) * TAILLE_CASE;
      j->en_cours = 1;
      j->tache = JOUEUR_MARCHER;
    }
  return 1;
}

int ia_frapper(picked *c UNUSED)
{
  t_agent j;

  if ((j = trouver_joueur()))
    {
      j->en_cours = 1;
      j->tache = JOUEUR_FRAPPER;
    }
  return 1;
}


/****  Le tableau et free  *********/

void init_iaction (iaction **ia,int *nb_ia)
{
  *nb_ia = NB_IA;
  *ia = malloc(NB_IA * (sizeof (iaction)));

  (*ia)[PICKING].name_ia = "Se deplacer";
  (*ia)[PICKING].ia = suivre;
  (*ia)[PICKING].i = 0;

  (*ia)[IA_AV].name_ia = "Marcher";
  (*ia)[IA_AV].ia = ia_marcher;
  (*ia)[IA_AV].i = 1;

  (*ia)[IA_FR].name_ia = "Frapper";
  (*ia)[IA_FR].ia = ia_frapper;
  (*ia)[IA_FR].i = 2;
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
    NB_STATS
  };


void init_stats (graphe **stats,int *nb_stats)
{
  *nb_stats = NB_STATS;
  *stats = malloc(NB_STATS * (sizeof (graphe)));
}


/********************************************** 
 **  ACTIONS SUR OBJS ET AGENTS  **************
 **********************************************/



/* Pour les action executees a chaque tour definies par l'utilisateur */
void user_ta (t_agent agent)
{
  static int cpt1=0,cpt2=0;
  static int jai_deja_annonce_la_victoire_donc_pas_besoin_de_le_refaire = 0;/*name by GayLord*/
  static float drad = 180.0 / M_PI;
  float dx,dz,an;

  
  if (agent != NULL)
    {
      dx = (agent->xgoto - agent->x);// * -1.0;
      dz = (agent->zgoto - agent->z);// * -1.0;
      agent->ry -= 90;

      if (dz + dx)// && (agcx != agent->case_x && agcy != agent->case_y))
	{
	  an = dx ? (atan(fabsf(dz) / fabsf(dx))) * drad : 90.0;
	  if (dx  <= 0)
	    {
	      if (dz <= 0)
		agent->ry = an + 180.0;
	      else
		agent->ry = an * -1.0 + 180.0;
	    }
	  else
	    {
	      if (dz <= 0)
		agent->ry = an * -1.0;
	      else
		agent->ry = an;
	    }
	}
      if (agent->speed)
	get_frame(agent, AN_RUN);
      /*else
	if (agent->en_cours <= FEMME_NOTHING_TO_DO)
	get_frame(agent, AN_RIEN);*/
      /* avancer : */
      agent->x += my_cos(agent->ry) * agent->speed * tps;
      agent->z += my_sin(agent->ry) * agent->speed * tps;
      agent->speed = 0.0;
      if(agent->type == BERSERK1)
	cpt1++;
      if(agent->type == BERSERK2)
	cpt2++;
      agent->ry += 90;
    }
  else
    {
      if((!jai_deja_annonce_la_victoire_donc_pas_besoin_de_le_refaire) && (cpt1 !=cpt2))
	{
	  if(cpt1 == 0)
	    {
	      jai_deja_annonce_la_victoire_donc_pas_besoin_de_le_refaire = 42;/*niéééé, je suis trop un rigolo >_< */
	      gtk_caca("L'equipe 2 a survecu");
	      return;
	    }
	  if(cpt2 == 0)
	    {
	      jai_deja_annonce_la_victoire_donc_pas_besoin_de_le_refaire = 1;
	      gtk_caca("L'equipe 1 a survecu");
	    }
	}
      cpt1=cpt2=0;

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
}

void user_cycle_gena (t_agent agent UNUSED)
{
}

void user_cycle_geno (t_objet objet UNUSED)
{
}


void stat_ag (t_agent agent UNUSED)
{

}

void stat_obj (t_objet objet UNUSED)
{

}


void stat_map (t_case * kase UNUSED)
{

}
