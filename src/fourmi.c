#include<gtk/gtk.h>
#include<stdlib.h>

/** gl *****************************************************/

#include <gdk/gdkkeysyms.h>
#include <gtk/gtkgl.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <math.h>
#include <unistd.h> 

#include "lib_bbs_behavior.h"
#include "loader.h"
#include "camera.h"
#include "divers.h"
#include "textures.h"
#include "fourmi.h"
#include "f.h"
#include "map.h"
#include "discrimination.h"
#include "propag.h"

#include "gtk+extra/gtkplot.h"
#include "gtk+extra/gtkplotdata.h"
#include "gtk+extra/gtkplotbar.h"
#include "gtk+extra/gtkplotcanvas.h"
#include "gtk+extra/gtkplotprint.h"
#include "stat.h"
#define RATIO_SPEED_OGL_OBJ 0.5
#define max(x,y) ((x>y)?x:y)

extern int stepbystep;
extern int move;
extern t_agent liste_agent;
extern t_objet liste_objet;
extern t_agent temp_agent;
extern t_objet temp_objet;
extern struct camera cam;
extern int show_billboard;
extern t_map carte;
extern ccase *cases;
extern behavior *stimuli;
extern float cycle;

extern t_draw lag_drw;
extern t_draw lag_queue;

extern void *(*user_ta)(t_agent agent);
extern void *(*user_to)(t_objet objet);
extern void *(*user_cycle_ta)(t_agent agent);
extern void *(*user_cycle_to)(t_objet objet);
extern void *(*user_cycle_gena)(t_agent agent);
extern void *(*user_cycle_geno)(t_objet objet);

/*
** var glob, c'est le delta-t sert pour 
** tout ce qui est en fct de temps
*/
double tps;

/*derniere stat calculees*/
double tps_stat=0;
int do_stat = 0;

/* fait avancer sur le terrain & la frame obj */
int avancer_ant (t_agent liste_agent)
{
  float ac_vitesse=0.025;

  (*liste_agent).ac_tps += tps*(((*liste_agent).speed)/RAP_ANIM);
  if ((*liste_agent).ac_tps > ac_vitesse)
    {
      (*liste_agent).step = ((*liste_agent).step + 1 ) % (*(*liste_agent).model).frames;
      (*liste_agent).ac_tps = 0;
    }
  /* je ne gere pas y, si qqun s'ennuis... be my guest 
  (*liste_agent).x -= (*liste_agent).speed * tps * (my_cos((*liste_agent).ry));
  (*liste_agent).z -= (*liste_agent).speed * tps * (my_sin((*liste_agent).ry));
  */
  return(1);
}

void traiter_fourmi()
{
  tps = get_tps();
  t_agent tmp = liste_agent, *darth_privious;
  darth_privious = &liste_agent;
  int px,pz,x,y;

  tps_stat += tps;
  do_stat = tps_stat > CYCLE;
  
  while (tmp)
    { 
      tmp->tps += tps;
      if (tmp->tps > CYCLE)
	{
	  user_cycle_ta(tmp);	  
	  tmp->age++;
	  /*deprop_int(tmp,1.0);******************************************************/
	  tmp->tps = 0.0;
	  discriminer(tmp);
	}

      if (do_stat)
	user_cycle_gena(tmp);
      action_agent(tmp);
      avancer_ant(tmp);
      user_ta(tmp);

      if (tmp->age < 0)
	{
	  if (tmp == cam.follow_agent)
	    cam.follow_agent = NULL;
	  del_agent(tmp->case_x,tmp->case_y,tmp);
	  *darth_privious = tmp->next;
	  free_agent(tmp);
	  tmp = *darth_privious;
	}
      else
	if (tmp->is_caddie)
	  {
	    darth_privious = &(tmp->next);/**/
	    tmp = tmp->next;
	  }
	else
	  {
	    x = (int) (tmp->x / TAILLE_CASE);
	    y = (int) (tmp->z / TAILLE_CASE);
	    px = x != tmp->case_x;
	    pz = y != tmp->case_y;
	    /*if (!tmp->is_caddie && (px || pz))*/
	    if (px || pz)
	      {
		if (in_map(x,y))
		  {
		    if (!cases[carte.tab[x][y].type].accessible)
		      {		  /* cases non accessible */
			if (px)
			  {
			    if (tmp->case_x > x)
			      tmp->x -= tmp->x - (tmp->case_x) * TAILLE_CASE;
			    else
			      tmp->x -= tmp->x - (tmp->case_x + 1) * TAILLE_CASE;
			  }
			if (pz)
			  {
			    if (tmp->case_y > y)
			      tmp->z -= tmp->z - (tmp->case_y) * TAILLE_CASE;
			    else
			      tmp->z -= tmp->z - (tmp->case_y + 1) * TAILLE_CASE;
			  }
		      }
		    else
		      {
			/*deprop_int(tmp,1.0); ******************************************************/
			del_agent(tmp->case_x,tmp->case_y,tmp);
			add_agent((int)(tmp->x / TAILLE_CASE), (int)(tmp->z / TAILLE_CASE),tmp);
		      }
		  }
	      }
	    darth_privious = &(tmp->next);
	    /*darth_privious = &tmp;*/
	    tmp = (*tmp).next;
	  }
    }
  *darth_privious = temp_agent;
  temp_agent = NULL;
  user_ta(NULL);
  if (do_stat)
    user_cycle_gena(NULL);
  /*
  p = lag_drw;
  while (p)
    {
      draw_agent(p->ag,0);
      lag_drw = p;
      p = p->n;
      free(lag_drw);
    }
  lag_drw = 0;
  lag_queue = 0;
  */
} 

void traiter_objet()
{
  t_objet tmp = liste_objet, *darth_privious;
  darth_privious = &liste_objet;
  int px,pz,x,y;

  while (tmp)
    { 
      if (tmp->type == -1)
	{
	  darth_privious = &(tmp->next);
	  tmp = (*tmp).next;
	  continue;
	}

      tmp->tps += tps;
      if (tmp->tps > CYCLE)
	{
	  user_cycle_to(tmp);	  
	  tmp->tps = 0.0;
	}

      if (do_stat)
	user_cycle_geno(tmp);

      user_to(tmp);
      if (tmp->valeur < 0.0 && !tmp->is_caddie)
	{
	  del_objet(tmp->case_x,tmp->case_y,tmp);
	  *darth_privious = tmp->next;
	  free_objet(tmp);
	  tmp = *darth_privious;
	}
      else
	{
	  x = (int) (tmp->x / TAILLE_CASE);
	  y = (int) (tmp->z / TAILLE_CASE);
	  px = x != tmp->case_x;
	  pz = y != tmp->case_y;
	  if (!tmp->is_caddie && (px || pz))
	    {
	      if (in_map(x,y))
		{
		  if (!cases[carte.tab[x][y].type].accessible)
		    {		  /* cases non accessible */
		      if (px)
			{
			  if (tmp->case_x > x)
			    tmp->x -= tmp->x - (tmp->case_x) * TAILLE_CASE;
			  else
			    tmp->x -= tmp->x - (tmp->case_x + 1) * TAILLE_CASE;
			}
		      if (pz)
			{
			  if (tmp->case_y > y)
			    tmp->z -= tmp->z - (tmp->case_y) * TAILLE_CASE;
			  else
			    tmp->z -= tmp->z - (tmp->case_y + 1) * TAILLE_CASE;
			}
		    }
		  else
		    {
		      del_objet(tmp->case_x,tmp->case_y,tmp);
		      add_objet((int)(tmp->x / TAILLE_CASE), (int)(tmp->z / TAILLE_CASE),tmp);
		    }
		}
	      else
		{
		  del_objet(tmp->case_x,tmp->case_y,tmp);
		  add_objet((int)(tmp->x / TAILLE_CASE), (int)(tmp->z / TAILLE_CASE),tmp);
		} 
	    }
	  darth_privious = &(tmp->next);
	  tmp = (*tmp).next;
	}
    }
  user_to(NULL);

  *darth_privious = temp_objet;
  temp_objet = NULL;

  if (do_stat)
    user_cycle_geno(NULL);

  if (do_stat)
    {
      end_stat();
      tps_stat = 0.;
      do_stat = 0;
    }
} 

