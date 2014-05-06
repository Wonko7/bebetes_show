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
    CAR_NOTHING_TO_DO=0,
    FT_NOTHING_TO_DO,
    
    CAR_ACCIDENT,
    /*CAR_FIRE,*/
    /*CAR_IMP,*/
    
    FT_ACCIDENT,
    /*FT_FIRE,*/
    /*FT_IMP,*/

    NB_STIMULI
  };

/* Agents */
enum 
  {
    CAR=0,
    FT,
    NB_AG
  };

/* Objets */
enum 
  {
    /*FIRE,*/
    FR=0,
    FV,
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
    RANDOM_ROULE=0,
    PICK_UP,
    PUT_DOWN,
    FOLLOW_S,
    FLEE_S,
    IS_ACC,
    STOP,
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
    IA_ACC,
    /*IA_FIRE,*/
    IA_SUPPR,
    NB_IA
  };

/* SOL */
enum 
  {
    CITY=0,                   /* par defaut */
    NORTH,
    SOUTH,
    EST,
    WEST,
    RP,
    NB_CASES
  };

/* les caractéristiques des agents /
enum
  {
    CA_HEALTH,
    CA_BOUFFE,
    NB_CA
  };

/ les caractéristiques des objets /
enum
  {
    NB_CO
  };
*/
int is_accident(t_agent ag, int arg UNUSED);
int put_down(t_agent agent, int arg UNUSED);

/********************************************** 
 **  CASES  *********************************** 
 **********************************************/

/* CHANGEMENT! : accessible : 2 pour on peut y aller et y creer des objets aleatoirement, 1 : on peut seulement y aller , 0 : ni l un ni l autre */
void init_cases (ccase ** c,int * nb_case)
{
  *nb_case = NB_CASES;
  *c = malloc(NB_CASES * (sizeof (ccase)));

  (*c)[NORTH].name = "Nord";
  (*c)[NORTH].texture = Load_image("road.tga");
  (*c)[NORTH].model = NULL;
  (*c)[NORTH].objet = NULL;
  (*c)[NORTH].accessible = 2;   

  (*c)[SOUTH].name = "Sud";
  (*c)[SOUTH].texture = Load_image("road.tga");
  (*c)[SOUTH].model = NULL;
  (*c)[SOUTH].objet = NULL;
  (*c)[SOUTH].accessible = 2;   

  (*c)[EST].name = "Est";
  (*c)[EST].texture = Load_image("road.tga");
  (*c)[EST].model = NULL;
  (*c)[EST].objet = NULL;
  (*c)[EST].accessible = 2;   

  (*c)[WEST].name = "Ouest";
  (*c)[WEST].texture = Load_image("road.tga");
  (*c)[WEST].model = NULL;
  (*c)[WEST].objet = NULL;
  (*c)[WEST].accessible = 2;   

  (*c)[RP].name = "Carrefour";
  (*c)[RP].texture = Load_image("road.tga");
  (*c)[RP].model = NULL;
  (*c)[RP].objet = NULL;
  (*c)[RP].accessible = 2;   

  (*c)[CITY].name = "Immeuble";
  (*c)[CITY].texture = Load_image("bouffe.bmp");
  (*c)[CITY].model = 0;/*"bouffe";*/
  (*c)[CITY].objet = 0;/*load_objet("bouffe",0.25);*/
  (*c)[CITY].accessible = 0;
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
  if (p(agent, arg))
    {
      agent->na = 0;
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
      caca("free_op, truc louche");
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
 **  Je nie formellement mon implication     **
 **  dans toute ecriture de fonction moche   **
 **  et incomprehensible.       Goulag.      **
 **********************************************/


static inline void ft_pd(t_agent ag)
{
  int i;

  if (ag->caddie && in_map(ag->case_x, ag->case_y) && ag->type == FT)
    {
      for (i = 0; i < carte.tab[ag->case_x][ag->case_y].nbr_ag; i++)
	if (carte.tab[ag->case_x][ag->case_y].agents[i]->type == CAR)
	  return;
      put_down(ag, 1);
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
static inline int mm_cos (int x) 
{
  while (x > 360)
    x -= 360;
  while (x < 0)
    x += 360;

  if (x == 90 || x == 270)
    return(0);
  if ((x >= 0 && x <= 90) || (x >= 270 && x <= 360))
    return(1);
  else
    return(-1);
}

static inline int mm_sin (int x) 
{
  while (x > 360)
    x -= 360;
  while (x < 0)
    x += 360;

  if (x == 0 || x == 180 || x == 360)
    return(0);
  if (x >= 0 && x <= 180)
    return(1);
  else
    return(-1);
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

static inline int gros_arrondi(float x)
{
  float a;

  a = x;
  a *= -1;

  while (a > 360.0)
    a -= 360.0;
  while (a < 0.0)
    a += 360.0;
  
  if (a >= 315.0 || a < 45.0)
    return 0;
  if (a >= 45.00 && a < 135.0)
    return 90;
  if (a >= 135.0 && a < 225.0)
    return 180.0;
  return 270;
}

/* putain, tabu-se... */

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
  /*
  if (tabu2[max - 1] == force)
    cpt--;
  */
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

static inline int sens_road(int x, int y)
{
  t_case c;

  if (!in_map(x,y))
    return -2;

  c = carte.tab[x][y];

  if (c.type == NORTH)
    return 90;
  if (c.type == EST)  
    return 0;
  if (c.type == WEST)
    return 180;
  if (c.type == SOUTH)
    return 270;
  if (c.type == RP)
    return -1;
  return(-5);
}

typedef struct n_pts * lpts;
struct n_pts 
{
  int x;
  int y;
  lpts n;
};
static inline void set_goto(t_agent ag)
{
  int x, y, sx, sy, a, b, sens, nd = 0, nb = 0, pos, tmp, apb;
  lpts t,n,q;

  t = q = malloc(sizeof(struct n_pts));
  q->x = -1;
  q->y = -1;
  q->n = 0;

  x = ag->case_x;
  y = ag->case_y;
  sens = sens_road(x, y);

  if (sens == -2 || sens == -5)
    {
      ag->xgoto = 0;
      ag->ygoto = 0;
      ag->x = 0.1;
      ag->z = 0.1;
      return;
    }

  if (sens_road(x, y) != -1) 
    {
      ag->xgoto = (ag->case_x + (mm_cos(sens)) + 0.5) * TAILLE_CASE;
      ag->zgoto = (ag->case_y - (mm_sin(sens)) + 0.5) * TAILLE_CASE;
      return;
    }

  a = gros_arrondi(ag->ry);
  b = -90;

  while (b < 500)
    {
      while (sens_road(x, y) == -1)
	{
	  apb = (a + b);
	  sx = x + mm_cos(apb - 90);
	  sy = y - mm_sin(apb - 90);
	  sens = sens_road(sx, sy);
	  tmp = apb - sens;
	  while (tmp > 360)
	    tmp -= 360;
	  while (tmp < 0)
	    tmp += 360;

	  if (sens >= 0 && tmp == 90)
	    {
	      n = malloc(sizeof (struct n_pts));
	      q->n = n;
	      q = n;
	      q->n = 0;
	      q->x = sx;
	      q->y = sy;
	      nb++;
	      if (sens == a - 180 || sens - 360 == a - 180)
		nd++;
	    }

	  if (x == ag->case_x && y == ag->case_y && b >= 270)
	    goto on_sort;
	  x += mm_cos(apb);
	  y -= mm_sin(apb);
	}
      x -= mm_cos(apb);
      y += mm_sin(apb);
      b += 90;
    }

 on_sort:

  if (my_rand(20) == 18)
    pos = nb - nd + my_rand(nd);
  else
    pos = my_rand(nb - nd);
  n = t;
  while (n)
    {
      if (pos == 0)
	{
	  ag->xgoto = (n->x + 0.5) * TAILLE_CASE ;
	  ag->zgoto = (n->y + 0.5) * TAILLE_CASE ;
	}
      q = n;
      n = n->n;
      free(q);
      pos--;
    }
}

#define NBCP 3
static inline int is_voiture(int x, int y, int a, int tout_droit)
{
  t_case c;
  int ss, t, i, is_feu = 0;;

  ss = sens_road(x, y);
  t = a + ss;
  while (t > 360)
    t -= 360;

  if (ss >= -1)
    {    
      c = carte.tab[x][y];

      if (c.nbr_obj)
	for (i = 0; i < c.nbr_obj; i++)
	  if (c.objets[i]->type == FR)
	    is_feu = 1;
    }

  if (ss < -1 || ((t != 90 || is_feu) && !tout_droit))
    return 0;


  if (is_feu) 
    return 2;


  if (c.nbr_ag == 0)
    return 1;

  for (i = 0; i < c.nbr_ag; i++)
    if (c.agents[i]->speed == SPEED)
      return 2;

  return 3;
}
static inline void goto_case(t_agent ag)
{
  int a, ss, sx, sy, i;

  ag->speed = SPEED;

  if (sens_road(ag->xgoto / TAILLE_CASE, ag->zgoto / TAILLE_CASE) < -1)
    {
      ag->speed = 0;
      set_goto(ag);
      return;
    }

  a = gros_arrondi(ag->ry);
  sx = ag->case_x + mm_cos(a - 90) + mm_cos(a);
  sy = ag->case_y - mm_sin(a - 90) - mm_sin(a);
  
  i = 0;
  ss = 1;
  while (ag->speed && ss && i < NBCP)
    {
      ss = is_voiture(sx, sy, a, 0); 
     if (ss == 2 || ss == 3)
	{
	  ag->speed = 0;
	  return;
	}
      sx += mm_cos(a - 90);
      sy -= mm_sin(a - 90);
      i++;
    }

  sx = ag->case_x + mm_cos(a);
  sy = ag->case_y - mm_sin(a);

  ss = is_voiture(sx, sy, a, 1);
  if (ss == 2 || ss == 0)
    {
      ag->speed = 0;
    }
  if (ss == 3)
    {
      ag->speed = 0;
      if (sens_road(ag->case_x, ag->case_y) == -1)
	set_goto(ag);
    }

  if (ag->type == CAR && is_accident(ag, -69))
    ag->speed = 0;
  
  ft_pd(ag);
}

/**** PRIMITIVES ELLES MEMES : *******/


int random_roule(t_agent agent, int arg UNUSED)
{
  /*int i,j,k,t[]={20,30,30,20,20,15,15,10,5};*/
  /*  for (i = 7; i>=0; i--)
    printf("%i ",t[i] += t[i+1]);  caca("");  */
  /*int i,j,k,t[]={165, 145, 115, 85, 65, 45, 30, 15, 5};
    x_et_y tabu[9],piou;*/

  /*agent->speed = SPEED;*/
  if (((int)(agent->x / TAILLE_CASE) == (int)(agent->xgoto / TAILLE_CASE)) && ((int) (agent->z / TAILLE_CASE) == (int)(agent->zgoto / TAILLE_CASE)))
    set_goto(agent);
  goto_case(agent);
  
  return(0);
}

int follow(t_agent agent, int arg)
{
  int i;
  x_et_y tabu[9],nef;
  
  agent->speed = SPEED;
  ft_pd(agent);
  if (((int)(agent->x / TAILLE_CASE) == (int)(agent->xgoto / TAILLE_CASE)) && ((int) (agent->z / TAILLE_CASE) == (int)(agent->zgoto / TAILLE_CASE)))
    {
      i = autour(agent->case_x,agent->case_y,agent->ry,tabu);
      nef = find_strong_stimulus(tabu,i,arguments[arg].id);
      if (nef.x == carte.x_max)
	return 1;//(random_roule(agent,arguments[arg].id));
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
	return(random_roule(agent,arguments[arg].id));
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

int is_accident(t_agent ag, int arg UNUSED)
{
  int x, y, i, b;
  t_case c;
  
  x = ag->case_x;
  y = ag->case_y;
  if (in_map(x, y) && !ag->is_caddie)
    {
      b = 0;
      c = carte.tab[x][y];
      i = 0;
      while (i < c.nbr_ag && !b)
	{
	  if (c.agents[i] != ag && c.agents[i]->type == CAR)
	    b = 1;
	  i++;
	}

      if (!(ag->stimuli[CAR_ACCIDENT].force) && b && my_rand(5) == 2)
	ag->stimuli[CAR_ACCIDENT].force = 50.0;
      if (!b)
	ag->stimuli[CAR_ACCIDENT].force = 0.0;

      if (ag->stimuli[CAR_ACCIDENT].force)
	{
	  if (arg != -69)
	    {
	      ag->stimuli[CAR_ACCIDENT].force += 0.5;
	      prop_e(x, y, CAR_ACCIDENT, ag->stimuli[CAR_ACCIDENT].force, 50.0);
	      prop_e(x, y, FT_ACCIDENT, ag->stimuli[CAR_ACCIDENT].force, 1.0);
	    }
	  return 1;
	}
    }
  return 0;
}

int die(t_agent agent, int arg UNUSED)
{
  put_down(agent, 1);
  agent->age *= (agent->age > 0)?-1:1;
  return 1;
}

int stop(t_agent ag, int arg UNUSED)
{
  ag->speed = 0;
  return 1;
}


/****  Le tableau et free  *********/

void init_primitives(primitive ** p,int * nb_p)
{
  char * s[NB_PRIM],*t1,*t2;
  int i;

  *nb_p = NB_PRIM;
  *p = malloc(NB_PRIM * (sizeof (primitive)));

  (*p)[RANDOM_ROULE].name_p = "Se promener";
  (*p)[RANDOM_ROULE].p = random_roule;
  (*p)[RANDOM_ROULE].bboard = Load_image("random_roule.tga");
  s[RANDOM_ROULE] = "random_roule.png";

  (*p)[FOLLOW_S].name_p = "Suivre stimulus";
  (*p)[FOLLOW_S].p = follow;
  (*p)[FOLLOW_S].bboard = Load_image("follow.tga");
  s[FOLLOW_S] = "p_follow.png";

  (*p)[FLEE_S].name_p = "Fuir stimulus";
  (*p)[FLEE_S].p = flee;
  (*p)[FLEE_S].bboard = Load_image("flee.tga");
  s[FLEE_S] = "p_flee.png";

  (*p)[PICK_UP].name_p = "Ramasser";
  (*p)[PICK_UP].p = pick_up;
  (*p)[PICK_UP].bboard = Load_image("pickup.tga");
  s[PICK_UP] = "p_pickup.png";

  (*p)[PUT_DOWN].name_p = "Poser";
  (*p)[PUT_DOWN].p = put_down;
  (*p)[PUT_DOWN].bboard = Load_image("putdown.tga");
  s[PUT_DOWN] = "p_putdown.png";
  /*
  (*p)[STOP_FIRE].name_p = "Fuir stimulus";
  (*p)[STOP_FIRE].p = p_exemple;
  s[STOP_FIRE] = "p_flee.png";
  */
  (*p)[IS_ACC].name_p = "Collision";
  (*p)[IS_ACC].p = is_accident;
  (*p)[IS_ACC].bboard = Load_image("accident.tga");
  s[IS_ACC] = "accident.png";

  (*p)[STOP].name_p = "Stop";
  (*p)[STOP].p = stop;
  (*p)[STOP].bboard = Load_image("stop.tga");
  s[STOP] = "stop.png";


  for (i=0; i<NB_PRIM; i++)
    {
      t1 = my_concat("/../share/bebetes_show/tb_icones/",s[i]);
      t2 = my_concat(chemin,t1);
      (*p)[i].icone_p = t2;
      free(t1);
    }
}

void free_p(primitive * op, int nb_op)
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
  *cycle = 1.5;
  *nombre_cycles_depr = 1;
  *s = malloc(NB_STIMULI * (sizeof (behavior)));

  (*s)[CAR_NOTHING_TO_DO].name_s = "Voiture: Rien a faire";
  (*s)[CAR_NOTHING_TO_DO].ag = CAR;
  (*s)[CAR_NOTHING_TO_DO].interne = 0;
  (*s)[CAR_NOTHING_TO_DO].em_int = -1;
  (*s)[CAR_NOTHING_TO_DO].task.force = INUTILISE;

  (*s)[FT_NOTHING_TO_DO].name_s = "Depanneuse: Rien a faire";
  (*s)[FT_NOTHING_TO_DO].ag = FT;
  (*s)[FT_NOTHING_TO_DO].interne = 0;
  (*s)[FT_NOTHING_TO_DO].em_int = -1;
  (*s)[FT_NOTHING_TO_DO].task.force = INUTILISE;

  (*s)[CAR_ACCIDENT].name_s = "Voiture: Accident";
  (*s)[CAR_ACCIDENT].ag = CAR;
  (*s)[CAR_ACCIDENT].interne = 1;
  (*s)[CAR_ACCIDENT].em_int = -1;
  (*s)[CAR_ACCIDENT].task.force = INUTILISE;
  /*(*s)[CAR_FIRE].name_s = "Voiture: Feu";
  (*s)[CAR_FIRE].ag = CAR;
  (*s)[CAR_FIRE].interne = 0;
  (*s)[CAR_FIRE].em_int = -1;
  (*s)[CAR_FIRE].task.force = INUTILISE;
  (*s)[CAR_IMP].name_s = "Voiture: Impatience";
  (*s)[CAR_IMP].ag = CAR;
  (*s)[CAR_IMP].interne = 0;
  (*s)[CAR_IMP].em_int = -1;
  (*s)[CAR_IMP].task.force = INUTILISE;
  */
  (*s)[FT_ACCIDENT].name_s = "Depanneuse: Accident";
  (*s)[FT_ACCIDENT].ag = FT;
  (*s)[FT_ACCIDENT].interne = 0;
  (*s)[FT_ACCIDENT].em_int = -1;
  (*s)[FT_ACCIDENT].task.force = INUTILISE;
  /*(*s)[FT_FIRE].name_s = "Depanneuse Feu";
  (*s)[FT_FIRE].ag = FT;
  (*s)[FT_FIRE].interne = 0;
  (*s)[FT_FIRE].em_int = -1;
  (*s)[FT_FIRE].task.force = INUTILISE;
  (*s)[FT_IMP].name_s = "Depanneuse: Impatience";
  (*s)[FT_IMP].ag = FT;
  (*s)[FT_IMP].interne = 0;
  (*s)[FT_IMP].em_int = -1;
  (*s)[FT_IMP].task.force = INUTILISE;
  */

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

  (*a)[CAR].name_ag = "Voiture"; 
  (*a)[CAR].ag = CAR;
  (*a)[CAR].ratio = 0.05;
  (*a)[CAR].nom_mod = "voiture";
  (*a)[FT].name_ag = "Depanneuse"; 
  (*a)[FT].ag = FT;
  (*a)[FT].ratio = 0.05;
  (*a)[FT].nom_mod = "ambulance";
  
  for (i=0; i<NB_AG; i++)
    (*a)[i].mod = 0;
}

void free_ag(agent * ag, int nb_ag UNUSED)
{
  free(ag);
}
/*
void make_agent(t_agent a)
{
  
}
*/

/********************************************** 
 **  OBJETS  ********************************** 
 **********************************************/

void init_objets(objet ** o,int * nb_obj)
{
  int i;
  
  *nb_obj = NB_OBJ;
  *o = malloc(NB_OBJ * (sizeof (objet)));
  /*
  (*o)[FIRE].name_obj = "Fire"; 
  (*o)[FIRE].obj = FIRE;
  (*o)[FIRE].ratio = 0.05;
  (*o)[FIRE].nom_mod = "bouffe";
  (*o)[FIRE].crea_taux = 0.05;
  */

  (*o)[FR].name_obj = "Feu Rouge";
  (*o)[FR].obj = FR;
  (*o)[FR].ratio = 0.025;
  (*o)[FR].nom_mod = "frouge";
  (*o)[FR].crea_taux = 0.0;

  (*o)[FV].name_obj = "Feu Vert"; 
  (*o)[FV].obj = FV;
  (*o)[FV].ratio = 0.025;
  (*o)[FV].nom_mod = "fvert";
  (*o)[FV].crea_taux = 0.0;


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

int ia_suppr(picked * clicked)
{
  if (clicked->type == IA_AGENT)
    die((t_agent) clicked->clicked, 1);

  return(1);
}

int ia_info(picked * clicked)
{
  int x, y;
  t_agent ag;
  if (clicked->type == IA_AGENT)
    {
      ag = clicked->clicked;
      x = (int) ag->xgoto / TAILLE_CASE;
      y = (int) ag->zgoto / TAILLE_CASE;
      printf("Info agent --- --- ---\n  x:%f \n  z:%f\n  xg:%f\n  zg:%f\n  xgc:%i\n  zgc:%i\n  xc:%i\n  yc:%i\n ---- \n  ry:%f--- --- ---\n",
	     ag->x, ag->z, ag->xgoto, ag->zgoto, x, y, ag->case_x, ag->case_y, ag->ry);
    }

  return(1);
}


/****  Le tableau et free  *********/

void init_iaction (iaction **ia,int *nb_ia)
{
  *nb_ia = NB_IA;
  *ia = malloc(NB_IA * (sizeof (iaction)));

  (*ia)[PICKING].name_ia = "Se deplacer";
  (*ia)[PICKING].ia = suivre;;
  (*ia)[PICKING].i = 0;
  
  (*ia)[IA_ACC].name_ia = "Creer un accident";
  (*ia)[IA_ACC].ia = ia_info;
  (*ia)[IA_ACC].i = 1;
  /*
  (*ia)[IA_FIRE].name_ia = "Mettre le feu";
  (*ia)[IA_FIRE].ia = ia_mat;
  (*ia)[IA_FIRE].i = 2;
  */
  (*ia)[IA_SUPPR].name_ia = "Supprimer";
  (*ia)[IA_SUPPR].ia = ia_suppr;
  (*ia)[IA_SUPPR].i = 2;
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
    STAT_V,
    STAT_F,
    NB_STATS
  };


void init_stats (graphe **stats,int *nb_stats)
{
  *nb_stats = NB_STATS;
  *stats = malloc(NB_STATS * (sizeof (graphe)));

  (*stats)[STAT_AGENTS].abscisse = "Cycles";
  (*stats)[STAT_AGENTS].ordonnee = "Nombre agents";
  (*stats)[STAT_AGENTS].fichier = "nb_agents";
  (*stats)[STAT_AGENTS].type = COURBE;

  (*stats)[STAT_V].abscisse = "Cycles";
  (*stats)[STAT_V].ordonnee = "Nombre voitures";
  (*stats)[STAT_V].fichier = "nb_voit";
  (*stats)[STAT_V].type = COURBE;

  (*stats)[STAT_F].abscisse = "Cycles";
  (*stats)[STAT_F].ordonnee = "Nombre depanneuses";
  (*stats)[STAT_F].fichier = "nb_pomp";
  (*stats)[STAT_F].type = COURBE;
}

/********************************************** 
 **  ACTIONS SUR OBJS ET AGENTS  **************
 **********************************************/

/* Pour les action executees a chaque tour definies par l'utilisateur */
void user_ta (t_agent agent)
{
  float dx,dz,/*angle,s,*/ d1, d2, an;
  int ss, a = 0, t = 666, agcx, agcy, sx, sy, sss;
  static float drad = 180.0 / M_PI;

  if (agent != NULL)
    {
      agcx = (int) agent->xgoto / TAILLE_CASE;
      agcy = (int) agent->zgoto / TAILLE_CASE;

      if ((sss = sens_road((int) agent->x / TAILLE_CASE, (int) agent->z / TAILLE_CASE)) == -1)
	{
	  a = gros_arrondi(agent->ry);
	  ss = sens_road(agent->xgoto / TAILLE_CASE, agent->zgoto / TAILLE_CASE);
	  t = ss - a;
	  while (t > 360)
	    t -= 360;
	  while (t < 0)
	    t += 360;

	  sx = (agent->case_x + mm_cos(a) + 0.5) * TAILLE_CASE;
	  sy = (agent->case_y - mm_sin(a) + 0.5) * TAILLE_CASE;

	  d1 = fabsf((agent->case_x + 0.5) * TAILLE_CASE - agent->xgoto) +
	    fabsf((agent->case_y + 0.5) * TAILLE_CASE - agent->zgoto);
	  d2 = fabsf(sx - agent->xgoto) + fabsf(sy - agent->zgoto);
	}
      if ((t == 90 || t == 270)
	  && d2 <= d1
	  && agcx != agent->case_x 
	  && agcy != agent->case_y)
	{
	  agent->ry = (float) a * -1;
	}
      else
	{
	  dx = (agent->xgoto - agent->x);// * -1.0;
	  dz = (agent->zgoto - agent->z);// * -1.0;

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
	      if (sss >= 0 && sss != gros_arrondi(agent->ry) && agent->type != FT)
		{
		  agent->ry = sss;
		  set_goto(agent);
		}
	    }
	  /*	  else
		  agent->ry = gros_arrondi(agent->ry);*/
	  /*
	  
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
	      printf("%f %f\n", agent->ry,  angle);*/


	}
      /* avancer : */
      agent->x += my_cos(agent->ry) * agent->speed * tps;
      agent->z += my_sin(agent->ry) * agent->speed * tps;
      agent->speed = 0.0;
    }
}

#define CF 20.0
void user_to (t_objet o)
{
  if (o && o->valeur > CF)
    {
      o->valeur = CF;
      o->tps = 0.0;
    }
}

void user_cycle_ta (t_agent agent UNUSED)
{
}

void user_cycle_to (t_objet o)
{
  int type, x, y;

  if (o)
    {
      o->valeur--;
      if (o->valeur == 0)
	{
	  type = o->type;
	  x = o->case_x;
	  y = o->case_y;
	  o->valeur = -1.0;
	  if (type == FR)
	    creer_objet(objets[FV].nom_mod, x, y, 0.,0.,0., objets[FV].ratio, FV, 1);
	  else
	    creer_objet(objets[FR].nom_mod, x, y, 0.,0.,0., objets[FR].ratio, FR, 1);
	}
    }
  /*float force;*/
  /*int t1[] = {ANT_FOOD,QUEEN_FOOD};
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
  /pas sur de devoir le repropager a chaque cycle, ou alors on change sa valeur (alternance jour/nuit ...)/
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
  */
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
      Add_courbe(STAT_AGENTS, (double) tab_count[NB_AG]);
      Add_courbe(STAT_V, (double) tab_count[CAR]);
      Add_courbe(STAT_F, (double) tab_count[FT]);
      for (i = 0;i < NB_AG + 1;i++)
	tab_count[i] = 0;
    }
}

void user_cycle_geno (t_objet objet UNUSED)
{
  /*static int compteur[NB_OBJ] = {0};
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
	}*/
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
