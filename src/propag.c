#include <gtk/gtk.h>
#include <stdlib.h>
#include"lib_bbs_behavior.h"
#include "loader.h"
#include "camera.h"
#include "map.h"
#include "divers.h"
#include "loading.h"
#include "propag.h"
#define __USE_ISOC99
#define __USE_XOPEN
#include <math.h>
#undef  __USE_ISOC99
#undef  __USE_XOPEN

#define is_float(f) ((f != NAN) && (f != INFINITY))

extern ccase * cases;
t_map carte;
extern behavior *stimuli;
extern agent *agents;
extern int nb_sti, nb_ag;
extern float cycle;
extern unsigned long tps_tot;
extern float tps_tot_ac;
extern int cy_depr;
int **stim_kkt = 0;


/********************************************** 
 **  TABLEAUX DE STI_INT A DEPROPAGER  ******** 
 **********************************************/

int init_tab_stim_int()
{
  int i,j,k;

  stim_kkt = malloc(nb_ag * sizeof (int *));

  for (i = 0; i < nb_ag; i++)
    {
      k = 0;
      for (j = 0; j < nb_sti; j++)
	if ((stimuli[j].ag == i && stimuli[j].interne) || stimuli[j].em_int == i)
	  k++;

      stim_kkt[i] = malloc((k + 1) * sizeof (int *));
      stim_kkt[i][0] = k;

      k = 1;
      j = 0;
      while (k <= stim_kkt[i][0])
	{
	  if ((stimuli[j].ag == i && stimuli[j].interne) || stimuli[j].em_int == i)
	    {
	      stim_kkt[i][k] = j;
	      k++;
	    }
	  j++;
	}
    }
  return(1);
}

int free_tab_stim_int()
{
  int i;
  
  if (!stim_kkt)
    return(1);

  for (i = 0; i < nb_ag; i++)
    free(stim_kkt[i]);

  free(stim_kkt);
  stim_kkt = 0;

  return(0);
}


/********************************************** 
 **  PROPAGATION ÉJACULATOIRE  **************** 
 **********************************************/

enum
  {
    N,
    NE,
    E,
    SE,
    S,
    SO,
    O,
    NO,
    ALL
  };

static inline int traiter_case(int x, int y, int sti, float force)
{
  t_signal list = 0, l, prec = 0;

  if (!(in_map(x,y)) || !cases[carte.tab[x][y].type].accessible || force <= 0.0 || !is_float(force))
    return(0);

  l = carte.tab[x][y].signaux;

  while (l)
    {
      if (cy_depr && (double) l->date <= (double) tps_tot - ((double) cy_depr * (double) cycle))
	{
	  if (prec)
	    {
	      prec->next = l->next;
	      free(l);
	      l = prec->next;
	    }
	  else
	    {
	      carte.tab[x][y].signaux = l->next;
	      free(l);
	      l = carte.tab[x][y].signaux;
	    }
	}
      else
	{
	  if (l->type == sti)
	    list = l;
	  prec = l;
	  l = l->next;
	}
    }

  if (list && list->force >/*=*/ force)
    return(0);

  if (!list)
    {
      list = malloc(sizeof (struct s_signal));
      list->type = sti;
      list->next = 0;
      if (!prec)
	carte.tab[x][y].signaux = list;
      else
	prec->next = list;
    }
  list->force = force;
  list->date = tps_tot;

  return(1);
}

#define is_obs(x,y,xx,yy) (in_map(x, y) && in_map(xx, yy) && !cases[carte.tab[x][y].type].accessible && cases[carte.tab[xx][yy].type].accessible)
#define is_diag(x,y,xx,yy) ((in_map(x, y) && in_map(xx, yy) && (cases[carte.tab[x][y].type].accessible || cases[carte.tab[xx][yy].type].accessible)) || !(in_map(x,y) && in_map(xx,yy)))

int ejac(int sens, int xx, int yy, int sti, float fforce, float decr)
{
  int y = yy, x = xx;
  float force = fforce;

  if (!traiter_case(x, y, sti, force))
    return(0);

  if (sens == N || sens == NE || sens == NO || sens == ALL)
    {
      y = yy/* + 1*/;
      force = fforce/* - decr*/;
      while (traiter_case(x,y,sti,force))
	{
	  force -= decr;
	  if (is_obs(x + 1, y, x, y + 1))
	    ejac(NE, x + 1, y + 1, sti, force, decr);
	  if (is_obs(x - 1, y, x, y + 1))
	    ejac(NO, x - 1, y + 1, sti, force, decr);
	  y++;
	}
    }

  if (sens == S || sens == SE || sens == SO || sens == ALL)
    {
      y = yy/* - 1*/;
      force = fforce/* - decr*/;
      while (traiter_case(x,y,sti,force))
	{
	  force -= decr;
	  if (is_obs(x + 1, y, x, y - 1))
	    ejac(SE, x + 1, y - 1, sti, force, decr);
	  if (is_obs(x - 1, y, x, y - 1))
	    ejac(SO, x - 1, y - 1, sti, force, decr);
	  y--;
	}
    }

  if (sens == E || sens == SE || sens == NE || sens == ALL)
    {
      y = yy;
      x = xx/* + 1*/;
      force = fforce/* - decr*/;
      while (traiter_case(x,y,sti,force))
	{
	  force -= decr;
	  if (is_obs(x, y - 1, x + 1, y))
	    ejac(SE, x + 1, y - 1, sti, force, decr);
	  if (is_obs(x, y + 1, x + 1, y))
	    ejac(NE, x + 1, y + 1, sti, force, decr);
	  x++;
	}
    }

  if (sens == O || sens == SO || sens == NO || sens == ALL)
    {
      y = yy;
      x = xx/* - 1*/;
      force = fforce/* - decr*/;
      while (traiter_case(x,y,sti,force))
	{
	  force -= decr;
	  if (is_obs(x, y - 1, x - 1, y))
	    ejac(SO, x - 1, y - 1, sti, force, decr);
	  if (is_obs(x, y + 1, x - 1, y))
	    ejac(NO, x - 1, y + 1, sti, force, decr);
	  x--;
	}
    }

  if (sens == NE || sens == ALL)
    {
      y = yy/* + 1*/;
      x = xx/* + 1*/;
      force = fforce/* - decr*/;
      while (traiter_case(x,y,sti,force) && is_diag(x + 1, y, x, y + 1))
	{
	  force -= decr;
	  ejac(N, x, y + 1, sti, force, decr);
	  ejac(E, x + 1, y, sti, force, decr);
	  if (is_obs(x + 1, y, x, y + 1) || is_obs(x, y + 1, x + 1, y))
	    {
	      ejac(NE, x + 1, y + 1, sti, force, decr);
	      break;
	    }
	  x++;
	  y++;
	}
    }

  if (sens == SE || sens == ALL)
    {
      y = yy/* - 1*/;
      x = xx/* + 1*/;
      force = fforce/* - decr*/;
      while (traiter_case(x,y,sti,force) && is_diag(x + 1, y, x, y - 1))
	{
	  force -= decr;
	  ejac(S, x, y - 1, sti, force, decr);
	  ejac(E, x + 1, y, sti, force, decr);
	  if (is_obs(x + 1, y, x, y - 1) || is_obs(x, y - 1, x + 1, y))
	    {
	      ejac(SE, x + 1, y - 1, sti, force, decr);
	      break;
	    }
	  x++;
	  y--;
	}
    }

  if (sens == SO || sens == ALL)
    {
      y = yy/* - 1*/;
      x = xx/* - 1*/;
      force = fforce/* - decr*/;
      while (traiter_case(x,y,sti,force) && is_diag(x - 1, y, x, y - 1))
	{
	  force -= decr;
	  ejac(S, x, y - 1, sti, force, decr);
	  ejac(O, x - 1, y, sti, force, decr);
	  if (is_obs(x - 1, y, x, y - 1) || is_obs(x, y - 1, x - 1, y))
	    {
	      ejac(SO, x - 1, y - 1, sti, force, decr);
	      break;
	    }
	  x--;
	  y--;
	}
    }

  if (sens == NO || sens == ALL)
    {
      y = yy/* + 1*/;
      x = xx/* - 1*/;
      force = fforce/* - decr*/;
      while (traiter_case(x,y,sti,force) && is_diag(x - 1, y, x, y + 1))
	{
	  force -= decr;
	  ejac(N, x, y + 1, sti, force, decr);
	  ejac(O, x - 1, y, sti, force, decr);
	  if (is_obs(x - 1, y, x, y + 1) || is_obs(x, y + 1, x - 1, y))
	    {
	      ejac(NO, x - 1, y + 1, sti, force, decr);
	      break;
	    }
	  x--;
	  y++;
	}
    }

  return(1);  
}


/********************************************** 
 **  DÉPROPAGATION ÉJACULATOIRE  ************** 
 **********************************************/

static inline int traiter_case2(int x, int y, int sti, float force, int ma_premiere_fois)
{
  t_signal list, prec;

  if (!ma_premiere_fois)
    return(1);

  if (!(in_map(x,y)) || !cases[carte.tab[x][y].type].accessible || force <= 0.0)
    return(0);

  list = carte.tab[x][y].signaux;
  prec = 0;
  
  while (list && (list->type != sti))
    {
      prec = list;
      list = list->next;
    }

  if (!list || list->force > force)
    return(0);

  if (!prec)
    carte.tab[x][y].signaux = list->next;
  else
    prec->next = list->next;
  free(list);

  return(1);
}

int eponge(int sens, int xx, int yy, int sti, float fforce, float decr)
{
  int y = yy, x = xx;
  float force = fforce;

  if (!traiter_case2(x, y, sti, force, 1))
    return(0);

  if (sens == N || sens == NE || sens == NO || sens == ALL)
    {
      y = yy/* + 1*/;
      force = fforce/* - decr*/;
      while (traiter_case2(x, y, sti, force, y != yy))
	{
	  force -= decr;
	  if (is_obs(x + 1, y, x, y + 1))
	    eponge(NE, x + 1, y + 1, sti, force, decr);
	  if (is_obs(x - 1, y, x, y + 1))
	    eponge(NO, x - 1, y + 1, sti, force, decr);
	  y++;
	}
    }
 
  if (sens == S || sens == SE || sens == SO || sens == ALL)
    {
      y = yy/* - 1*/;
      force = fforce/* - decr*/;
      while (traiter_case2(x, y, sti, force, y != yy))
	{
	  force -= decr;
	  if (is_obs(x + 1, y, x, y - 1))
	    eponge(SE, x + 1, y - 1, sti, force, decr);
	  if (is_obs(x - 1, y, x, y - 1))
	    eponge(SO, x - 1, y - 1, sti, force, decr);
	  y--;
	}
    }

  if (sens == E || sens == SE || sens == NE || sens == ALL)
    {
      y = yy;
      x = xx/* + 1*/;
      force = fforce/* - decr*/;
      while (traiter_case2(x, y, sti, force, x != xx))
	{
	  force -= decr;
	  if (is_obs(x, y - 1, x + 1, y))
	    eponge(SE, x + 1, y - 1, sti, force, decr);
	  if (is_obs(x, y + 1, x + 1, y))
	    eponge(NE, x + 1, y + 1, sti, force, decr);
	  x++;
	}
    }

  if (sens == O || sens == SO || sens == NO || sens == ALL)
    {
      y = yy;
      x = xx/* - 1*/;
      force = fforce/* - decr*/;
      while (traiter_case2(x, y, sti, force, x != xx))
	{
	  force -= decr;
	  if (is_obs(x, y - 1, x - 1, y))
	    eponge(SO, x - 1, y - 1, sti, force, decr);
	  if (is_obs(x, y + 1, x - 1, y))
	    eponge(NO, x - 1, y + 1, sti, force, decr);
	  x--;
	}
    }

  if (sens == NE || sens == ALL)
    {
      y = yy/* + 1*/;
      x = xx/* + 1*/;
      force = fforce/* - decr*/;
      while (traiter_case2(x, y, sti, force, y != yy) && is_diag(x + 1, y, x, y + 1))
	{
	  force -= decr;
	  eponge(N, x, y + 1, sti, force, decr);
	  eponge(E, x + 1, y, sti, force, decr);
	  if (is_obs(x + 1, y, x, y + 1) || is_obs(x, y + 1, x + 1, y))
	    {
	      eponge(NE, x + 1, y + 1, sti, force, decr);
	      break;
	    }
	  x++;
	  y++;
	}
    }

  if (sens == SE || sens == ALL)
    {
      y = yy/* - 1*/;
      x = xx/* + 1*/;
      force = fforce/* - decr*/;
      while (traiter_case2(x, y, sti, force, y != yy) && is_diag(x + 1, y, x, y - 1))
	{
	  force -= decr;
	  eponge(S, x, y - 1, sti, force, decr);
	  eponge(E, x + 1, y, sti, force, decr);
	  if (is_obs(x + 1, y, x, y - 1) || is_obs(x, y - 1, x + 1, y))
	    {
	      eponge(SE, x + 1, y - 1, sti, force, decr);
	      break;
	    }
	  x++;
	  y--;
	}
    }

  if (sens == SO || sens == ALL)
    {
      y = yy/* - 1*/;
      x = xx/* - 1*/;
      force = fforce/* - decr*/;
      while (traiter_case2(x, y, sti, force, y != yy) && is_diag(x - 1, y, x, y - 1))
	{
	  force -= decr;
	  eponge(S, x, y - 1, sti, force, decr);
	  eponge(O, x - 1, y, sti, force, decr);
	  if (is_obs(x - 1, y, x, y - 1) || is_obs(x, y - 1, x - 1, y))
	    {
	      eponge(SO, x - 1, y - 1, sti, force, decr);
	      break;
	    }
	  x--;
	  y--;
	}
    }

  if (sens == NO || sens == ALL)
    {
      y = yy/* + 1*/;
      x = xx/* - 1*/;
      force = fforce/* - decr*/;
      while (traiter_case2(x, y, sti, force, y != yy) && is_diag(x - 1, y, x, y + 1))
	{
	  force -= decr;
	  eponge(N, x, y + 1, sti, force, decr);
	  eponge(O, x - 1, y, sti, force, decr);
	  if (is_obs(x - 1, y, x, y + 1) || is_obs(x, y + 1, x - 1, y))
	    {
	      eponge(NO, x - 1, y + 1, sti, force, decr);
	      break;
	    }
	  x--;
	  y++;
	}
    }

  return(1);  
}

/********************************************** 
 **  MEGA **PROPAGATION ÉJACULATOIRE  ********* 
 **********************************************/


static inline float force_stim(int i, int j, int stim)
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

int deprop_int(t_agent a, float z UNUSED)
{
  int max = stim_kkt[a->type][0], cpt = 1, i, x, y;
  float *t, d = 0.0, dmin;
  t_signal list;

  /*caca("deprop_int est DEPRECATED, connard");*/
  t = calloc(max, sizeof (float));

  if (!(in_map(a->case_x,a->case_y)))
    return(0);

  list = carte.tab[a->case_x][a->case_y].signaux;

  while (list && (cpt < max))
    {
      i = 1;
      while (i <= max)
	{
	  if (list->type == stim_kkt[a->type][i])
	    {
	      t[i-1] = list->force;
	      cpt++;
	      break;
	    }
	  i++;
	}
      list = list->next;
    }
 
  for (i = 1; i <= max; i++)
    {
      if (t[i-1])
	{
	  dmin = t[i-1];
	  for (x = 0; x < 3; x++)
	    for (y  = 0; y < 3; y++)
	      {
		if (in_map(a->case_x + x - 1, a->case_y + y - 1) && 
		    cases[carte.tab[a->case_x + x - 1][a->case_y + y - 1].type].accessible)
		  {
		    d = t[i-1] - force_stim(a->case_x + x - 1, a->case_y + y - 1, stim_kkt[a->type][i]);
		    if (d < dmin && d > 0)
		      dmin = d;
		  }
	      }
	  eponge(ALL, a->case_x, a->case_y, stim_kkt[a->type][i], t[i-1], d);	  
	}
    }

  /*
  for (i = 1; i <= max; i++)
  t[i-1] && eponge(ALL, a->case_x, a->case_y, stim_kkt[a->type][i], t[i-1], d);
  */

  free(t);
  return(1);
}

int prop_mega(int x, int y, int nb, int * stim, float force, float d)
{
  int i;

  for (i = 0; i < nb; i++)
    ejac(ALL, x, y, stim[i], force, d);

  return(1);
}

int deprop_mega(int x, int y, int nb, int * stim, float d)
{
  int cpt = 0, i;
  float *t;
  t_signal list, prec;

  if (!(in_map(x,y)))
    return(0);

  t = calloc(nb, sizeof (float));
  list = carte.tab[x][y].signaux;
  prec = 0;
  
  while (list && (cpt < nb))
    {
      i = 0;
      while (i < nb)
	{
	  if (list->type == stim[i])
	    {
	      t[i] = list->force;
	      cpt++;
	      break;
	    }
	  i++;
	}
      list = list->next;
    }

  for (i = 0; i < nb; i++)
    t[i] && eponge(ALL, x, y, stim[i], t[i], d);

  free(t);
  return(1);
}

/* alexandre... t'es un putain de connard */
/* alexandre... t'es un putain de connard */
/* alexandre... t'es un putain de connard */
/* alexandre... t'es un putain de connard */
