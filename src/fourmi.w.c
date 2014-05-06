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

#define RATIO_SPEED_OGL_OBJ 0.5
#define max(x,y) ((x>y)?x:y)
extern int stepbystep;
extern int move;
extern t_agent liste_agent;
extern t_objet liste_objet;
extern struct camera cam;
extern int show_billboard;
extern t_map carte;
extern ccase *cases;
extern behavior *stimuli;
extern float cycle;

extern void *(*user_ta)(t_agent agent);
extern void *(*user_to)(t_objet objet);

/*
** var glob, c'est le delta-t sert pour 
** tout ce qui est en fct de temps
*/
double tps;



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
  /*t_signal sg;*/
  tps = get_tps();
  t_agent tmp = liste_agent, *darth_privious;
  darth_privious = &liste_agent;
  int px,pz,x,y;

  while (tmp)
    { 
      tmp->tps += tps;
      if (tmp->tps > CYCLE)
	{
	  tmp->tps = 0.0;
	  discriminer(tmp);
	}
      action_agent(tmp);

      tmp->x += my_cos(tmp->ry) * tmp->speed * tps;
      tmp->z += my_sin(tmp->ry) * tmp->speed * tps;
      avancer_ant(tmp);
      tmp->speed = 0.0;
      
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
	{
	  x = (int) (tmp->x / TAILLE_CASE);
	  y = (int) (tmp->z / TAILLE_CASE);
	  px = x != tmp->case_x;
	  pz = y != tmp->case_y;
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
		      del_agent(tmp->case_x,tmp->case_y,tmp);
		      add_agent((int)(tmp->x / TAILLE_CASE), (int)(tmp->z / TAILLE_CASE),tmp);
		    }
		}
	    }
	  darth_privious = &(tmp->next);
	  tmp = (*tmp).next;
	}
    }
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
      user_to(tmp);
      if (tmp->valeur < 0.0)
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
		      del_objet(tmp->case_x,tmp->case_y,tmp);
		      add_objet((int)(tmp->x / TAILLE_CASE), (int)(tmp->z / TAILLE_CASE),tmp);
		    }
		}
	    }
	  darth_privious = &(tmp->next);
	  tmp = (*tmp).next;
	}
    }
}

/* alexandre... t'es un putain de connard */
/* alexandre... t'es un putain de connard */
/* alexandre... t'es un putain de connard */
/* alexandre... t'es un putain de connard */
