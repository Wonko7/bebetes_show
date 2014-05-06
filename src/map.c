#include "lib_bbs_behavior.h"
#include "loader.h"
#include "camera.h"
#include "map.h"
#include <stdlib.h>
#include <stdio.h>
#include "divers.h"
#include <math.h>
#include <dlfcn.h>

#include <GL/gl.h>
#include <GL/glu.h>

#include <gtk/gtk.h>
#include "info.h"
#include "remote_control.h"

extern t_map carte;
extern struct camera cam;
extern int gHeight,gWidth;
extern t_texture tex_index;
extern void * lib;
extern ccase * cases;
extern int options_lumiere;
extern iaction *iactions;
extern int selected_iaction;

int afficher_stimuli = 0;
int stimulus_selectionne = 0;
float max_sti = 0.;
float min_sti = 0.;
extern float options_profondeur;
extern int sel_x,sel_y;
extern int pick_selected;
int ultraHeight, ultraWidth;
picked clicked;
int cdepth;
extern unsigned long tps_tot;
extern float tps_tot_ac;
extern int cy_depr;
extern float cycle;

extern struct camera cam;
t_draw lag_drw = 0, lag_queue;

void init_map(t_map carte)
{
  int i,j;
  for(i=0;i < carte.x_max; i++)
    {
      for(j=0;j < carte.y_max; j++)
	{
	  carte.tab[i][j].objets = NULL;
	  carte.tab[i][j].agents = NULL;
	  carte.tab[i][j].signaux = NULL;
	  carte.tab[i][j].nbr_ag = 0;
	  carte.tab[i][j].nbr_obj = 0;
	  carte.tab[i][j].type = 0;
	  carte.tab[i][j].etat = 1;
	}
    }
}


t_map create_map(int x, int y)
{
  t_map tmp;
  int i;
 
  tmp.tab = malloc(x * sizeof(t_case *));
  for (i=0;i < x;i++)
    tmp.tab[i] = malloc(y * sizeof(t_case));

  tmp.x_max = x;
  tmp.y_max = y;
  init_map(tmp);

  return tmp;
}

void fonction_qui_fait_autant_de_choses_que_julien_dans_le_projet ()
{
   int en, plus, il, prend, de, la, place, pour, rien;
   en++;
   plus++;
   il++;
   prend++;
   de++;
   la++;
   place++;
   pour++;
   rien++;
}

void free_map()
{
  int i,j;
  t_signal tmp,prec;

  for (i=0;i < carte.x_max;i++)
    {
      for (j=0;j < carte.y_max;j++)
	{
	  if (carte.tab[i][j].agents)
	    free(carte.tab[i][j].agents);

	  tmp=carte.tab[i][j].signaux;
	  while(tmp)
	    {
	      prec=tmp;
	      tmp=tmp->next;
	      if(prec)
		{
		  free(prec);
		  prec=NULL;
		}
	    }
	  
	  
	}
      free(carte.tab[i]);
    }

  free(carte.tab);
  carte.tab = NULL;
  carte.x_max = 0;
  carte.y_max = 0;
}

float ultra_find_x (float x1, float y1, float x2, float y2) /* prend 2 points et renvoie la coord en x du point ou la droite crois le plan xz */
{
  y1 += (y1 == y2)?(1.0):(0.0);
  return(((y2 - y1) * x1 - (x2 - x1) * y1)/(y2-y1));
}



void draw_stim(int i, int j,int type, int force)
{
  glColor3f(0.1*type,0.1*force,1.0);
  glBegin(GL_QUADS);
  glVertex3f(i * TAILLE_CASE, 4.0f, j * TAILLE_CASE);  
  glVertex3f((i+1) * TAILLE_CASE, 4.0f, j * TAILLE_CASE);  
  glVertex3f((i+1) * TAILLE_CASE, 4.0f, (j+1) * TAILLE_CASE);  
  glVertex3f(i * TAILLE_CASE, 4.0f, (j+1) * TAILLE_CASE);   
  glEnd();
}



void draw_case (int i, int j)
{
  float k=0.;
  int c;

  if (afficher_stimuli)
    {
      t_signal proutchman=NULL;
      proutchman = carte.tab[i][j].signaux;
      while (proutchman)
	{

	  if (proutchman->type == stimulus_selectionne && 
	      (!cy_depr ||
	      (double) proutchman->date + ((double) cy_depr * (double) cycle) > (double) tps_tot))
	    {
	      k = proutchman->force;
	      break;
	    }
	  proutchman = proutchman->next;
	}
      k -= min_sti;
      if (k > 0.)
	glColor3f(((float)max_sti - (float)k) / (float)max_sti,((float)max_sti - (float)k)/(float)max_sti,1.0);
      else
	if ((i+j) % 2)
	  glColor3f(0.8,0.8,0.8);
	else
	  glColor3f(0.9,0.9,0.9);
      glDisable(GL_LIGHTING);
      glBegin(GL_QUADS);
      glVertex3f(i * TAILLE_CASE,0.0,j * TAILLE_CASE);
      glVertex3f(i * TAILLE_CASE,0.0,(j + 1) * TAILLE_CASE);
      glVertex3f((i + 1) * TAILLE_CASE,0.0,(j + 1) * TAILLE_CASE);
      glVertex3f((i + 1) * TAILLE_CASE,0.0,j * TAILLE_CASE);
      glEnd();
      if (options_lumiere)
	glEnable(GL_LIGHTING);
    }
  else
    {
      glColor3f(1.0f,1.0f,1.0f);
      glEnable(GL_TEXTURE_2D);
      glBindTexture(GL_TEXTURE_2D,*(cases[carte.tab[i][j].type].texture) );
      glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
      glBegin(GL_QUADS);
      glTexCoord2f(0.0f, 0.0f);
      glVertex3f(i * TAILLE_CASE,0.0,j * TAILLE_CASE);
      glTexCoord2f(1.0f, 0.0f);
      glVertex3f(i * TAILLE_CASE,0.0,(j + 1) * TAILLE_CASE);
      glTexCoord2f(1.0f, 1.0f);
      glVertex3f((i + 1) * TAILLE_CASE,0.0,(j + 1) * TAILLE_CASE);
      glTexCoord2f(0.0f, 1.0f);
      glVertex3f((i + 1) * TAILLE_CASE,0.0,j * TAILLE_CASE);
      glEnd();
      glDisable(GL_TEXTURE_2D); 
    }
  /*
  for (k = 0;k < carte.tab[i][j].nbr_ag;k++)
    draw_agent(carte.tab[i][j].agents[k],0);
  */

  for (c = 0;c < carte.tab[i][j].nbr_ag;c++)
    draw_agent(carte.tab[i][j].agents[c],0);

  for (c = 0;c < carte.tab[i][j].nbr_obj;c++)
    draw_objet(carte.tab[i][j].objets[c],0);
    
  if (cases[carte.tab[i][j].type].model)
    {
      cases[carte.tab[i][j].type].objet->x = (i + 0.5) * TAILLE_CASE;
      cases[carte.tab[i][j].type].objet->z = (j + 0.5) * TAILLE_CASE;
      draw_objet(cases[carte.tab[i][j].type].objet,0);
    }
}

/* DEPRECATED quad tree pour picking hehe
void draw_case_pic (int iinf, int isup, int jinf, int jsup)
{
  int k;
  int imed = (iinf + isup) / 2;
  int jmed = (jinf + jsup) / 2;
  GLubyte col[]={255,255,255,1}

  glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

  if ((imed == iinf) && (jmed == jinf))
    {
      caca("got it");
      return;
    }
  
  glBegin(GL_QUADS);
  glColor3ub(255,0,0); force rouge! 
  glVertex3f(iinf * TAILLE_CASE,0.0,jinf * TAILLE_CASE);
  glVertex3f(iinf * TAILLE_CASE,0.0,jmed * TAILLE_CASE);
  glVertex3f(imed * TAILLE_CASE,0.0,jmed * TAILLE_CASE);
  glVertex3f(imed * TAILLE_CASE,0.0,jinf * TAILLE_CASE);

  glColor3ub(0,255,0); force verte! 
  glVertex3f(imed * TAILLE_CASE,0.0,jinf * TAILLE_CASE);
  glVertex3f(imed * TAILLE_CASE,0.0,jmed * TAILLE_CASE);
  glVertex3f(isup * TAILLE_CASE,0.0,jmed * TAILLE_CASE);
  glVertex3f(isup * TAILLE_CASE,0.0,jinf * TAILLE_CASE);

  glColor3ub(0,0,255); force bleue!
  glVertex3f(iinf * TAILLE_CASE,0.0,jmed * TAILLE_CASE);
  glVertex3f(iinf * TAILLE_CASE,0.0,jsup * TAILLE_CASE);
  glVertex3f(imed * TAILLE_CASE,0.0,jsup * TAILLE_CASE);
  glVertex3f(imed * TAILLE_CASE,0.0,jmed * TAILLE_CASE);

  glColor3ub(255,168,231); force rose!
  glVertex3f(imed * TAILLE_CASE,0.0,jmed * TAILLE_CASE);
  glVertex3f(imed * TAILLE_CASE,0.0,jsup * TAILLE_CASE);
  glVertex3f(isup * TAILLE_CASE,0.0,jsup * TAILLE_CASE);
  glVertex3f(isup * TAILLE_CASE,0.0,jmed * TAILLE_CASE);

  glReadPixels(sel_x,sel_y,1,1,GL_RGBA,GL_UNSIGNED_BYTE,&col);

  if (col[1] == 168)
    draw_case_pic(imed,isup,jmed,jsup);
  else
    if (col[0] == 255)
      draw_case_pic(iinf,imed,jinf,jmed);
    else
      if (col[2] == 255)
	draw_case_pic(iinf,imed,jmed,jsup);
      else
	draw_case_pic(imed,isup,jinf,jmed);

  glEnd();
  

}  
*/
/* DEPRECATED
static inline void rgb_update ()
{
  red++;
  if (!red)
    green++;
}
*/
int find_clicked (int xinf,int xsup,int zinf, int zsup)
{
  int i,j,k;
  int red = cdepth, green = cdepth;
  GLubyte col[]={255,255,255,1};

  glDisable(GL_LIGHTING);

  if (cos(cam.alpha / RAD) < 0.0)
    {
      if (sin(cam.alpha / RAD) > 0.0)
	{
	  for (i = xinf;i <= xsup;i++)
	    {
	      for (j = zinf;j <= zsup;j++)
		{
		  for (k = 0;k < carte.tab[i][j].nbr_ag;k++)
		    {
		      draw_agent(carte.tab[i][j].agents[k],red);
		      red += cdepth;
		    }

		  if (cases[carte.tab[i][j].type].model)
		    {
		      cases[carte.tab[i][j].type].objet->x = (i + 0.5) * TAILLE_CASE;
		      cases[carte.tab[i][j].type].objet->z = (j + 0.5) * TAILLE_CASE;
		      draw_objet(cases[carte.tab[i][j].type].objet,254);
		    }		  
		  
		  
		  for (k = 0;k < carte.tab[i][j].nbr_obj;k++)
		    {
		      draw_objet(carte.tab[i][j].objets[k],green);
		      green += cdepth;
		    }
		  glReadPixels(sel_x,sel_y,1,1,GL_RGBA,GL_UNSIGNED_BYTE,&col);
		  if (!col[2])
		    {
		      clicked.x = i;
		      clicked.y = j;
		      if (col[1] > 42)
			{
			  clicked.type = IA_CASE;
			  clicked.clicked = &(carte.tab[i][j]);
			  return(1);
			}
		      else
			if (col[0])
			  {
			    clicked.type = IA_AGENT;
			    clicked.clicked = carte.tab[i][j].agents[col[0]/cdepth - 1];
			    return(1);
			  }
			else
			  {
			    clicked.type = IA_OBJ;
			    clicked.clicked = carte.tab[i][j].objets[col[1]/cdepth  - 1];
			    return(1);
			  }
		    }
		  red = green = cdepth;
		}
	    }
	}
      else
	{
	  for (i = xsup;i >= xinf;i--)
	    {
	      for (j = zinf;j <= zsup;j++)
		{
		  for (k = 0;k < carte.tab[i][j].nbr_ag;k++)
		    {
		      draw_agent(carte.tab[i][j].agents[k],red);
		      red += cdepth;
		    }

		  if (cases[carte.tab[i][j].type].model)
		    {
		      cases[carte.tab[i][j].type].objet->x = (i + 0.5) * TAILLE_CASE;
		      cases[carte.tab[i][j].type].objet->z = (j + 0.5) * TAILLE_CASE;
		      draw_objet(cases[carte.tab[i][j].type].objet,254);
		    }		  
		  
		  
		  for (k = 0;k < carte.tab[i][j].nbr_obj;k++)
		    {
		      draw_objet(carte.tab[i][j].objets[k],green);
		      green += cdepth;
		    }
		  glReadPixels(sel_x,sel_y,1,1,GL_RGBA,GL_UNSIGNED_BYTE,&col);
		  if (!col[2])
		    {
		      clicked.x = i;
		      clicked.y = j;
		      if (col[1] > 42)
			{
			  clicked.type = IA_CASE;
			  clicked.clicked = &(carte.tab[i][j]);
			  return(1);
			}
		      else
			if (col[0])
			  {
			    clicked.type = IA_AGENT;
			    clicked.clicked = carte.tab[i][j].agents[col[0]/cdepth  - 1];
			    return(1);
			  }
			else
			  {
			    clicked.type = IA_OBJ;
			    clicked.clicked = carte.tab[i][j].objets[col[1]/cdepth  - 1];
			    return(1);
			  }
		    }
		  red = green = cdepth;
		}
	    }
	}
    }
  else
    {
      if (sin(cam.alpha / RAD) < 0.0)
	{
	  for (i = xsup;i >= xinf;i--)
	    {
	      for (j = zsup;j >= zinf;j--)
		{
		  for (k = 0;k < carte.tab[i][j].nbr_ag;k++)
		    {
		      draw_agent(carte.tab[i][j].agents[k],red);
		      red += cdepth;
		    }
		  
		  if (cases[carte.tab[i][j].type].model)
		    {
		      cases[carte.tab[i][j].type].objet->x = (i + 0.5) * TAILLE_CASE;
		      cases[carte.tab[i][j].type].objet->z = (j + 0.5) * TAILLE_CASE;
		      draw_objet(cases[carte.tab[i][j].type].objet,254);
		    }		  
		  
		  
		  for (k = 0;k < carte.tab[i][j].nbr_obj;k++)
		    {
		      draw_objet(carte.tab[i][j].objets[k],green);
		      green += cdepth;
		    }
		  glReadPixels(sel_x,sel_y,1,1,GL_RGBA,GL_UNSIGNED_BYTE,&col);
		  if (!col[2])
		    {		  
		      clicked.x = i;
		      clicked.y = j;
		      if (col[1] > 42)
			{

			  clicked.type = IA_CASE;
			  clicked.clicked = &(carte.tab[i][j]);
			  return(1);
			}
		      else
			if (col[0])
			  {
			    clicked.type = IA_AGENT;
			    clicked.clicked = carte.tab[i][j].agents[col[0]/cdepth  - 1];
			    return(1);
			  }
			else
			  {
			    clicked.type = IA_OBJ;
			    clicked.clicked = carte.tab[i][j].objets[col[1]/cdepth  - 1];
			    return(1);
			  }
		    }
		  red = green = cdepth;
		}
	    }
	}
      else
	{
	  for (i = xinf;i <= xsup;i++)
	    {
	      for (j = zsup;j >= zinf;j--)
		{
		  for (k = 0;k < carte.tab[i][j].nbr_ag;k++)
		    {
		      draw_agent(carte.tab[i][j].agents[k],red);
		      red += cdepth;
		    }

		  if (cases[carte.tab[i][j].type].model)
		    {
		      cases[carte.tab[i][j].type].objet->x = (i + 0.5) * TAILLE_CASE;
		      cases[carte.tab[i][j].type].objet->z = (j + 0.5) * TAILLE_CASE;
		      draw_objet(cases[carte.tab[i][j].type].objet,254);
		    }		  
		  
		  
		  for (k = 0;k < carte.tab[i][j].nbr_obj;k++)
		    {
		      draw_objet(carte.tab[i][j].objets[k],green);
		      green += cdepth;
		    }
		  glReadPixels(sel_x,sel_y,1,1,GL_RGBA,GL_UNSIGNED_BYTE,&col);
		  if (!col[2])
		    {
		      clicked.x = i;
		      clicked.y = j;
		      if (col[1] > 42)
			{
			  clicked.type = IA_CASE;
			  clicked.clicked = &(carte.tab[i][j]);
			  return(1);
			}
		      else
			if (col[0])
			  {
			    clicked.type = IA_AGENT;
			    clicked.clicked = carte.tab[i][j].agents[col[0]/cdepth  - 1];
			    return(1);
			  }
			else
			  {
			    clicked.type = IA_OBJ;
			    clicked.clicked = carte.tab[i][j].objets[col[1]/cdepth  - 1];
			    return(1);
			  }
		    }
		  red = green = cdepth;
		}
	    }
	}
    }	      
  
  return(0);
}

void draw_scene ()
{
  float x,y,z,l;
  float x1,y1,z1,x2,y2,z2,x3,y3,z3,x4,y4,z4,db,da;
  float coa, cob, sia, sib;
  t_2Dplot c[4];
  int i=0,j=0;
  int zinf,zsup,xinf,xsup;
  max_sti = 0.;
  min_sti = 1./0.;
  GLubyte col[]={255,255,255,1};

  sia = sin(cam.alpha / RAD);
  sib = sin(cam.beta / RAD);
  coa = cos(cam.alpha / RAD);
  cob = cos(cam.beta / RAD);

  l = cam.zoom * cos(1 / 57.2957795 * cam.beta);

  x = cam.x + l * sin(1 / 57.2957795 * cam.alpha) ;
  y = - cam.zoom * sin(1 / 57.2957795 * cam.beta);
  z = cam.y - l * cos(1 / 57.2957795 * cam.alpha);

  float zellagui = 0.75;    /* constante empirique tres moche qu'on comprend pas... */

  db = gHeight / 2.0 / options_profondeur * zellagui;
  da = gWidth / 2.0 / options_profondeur * zellagui;

  x1 = x - options_profondeur * (-sia * cob - sia * sib * db + da * coa);
  x2 = x - options_profondeur * (-sia * cob - sia * sib * db + (-da) * coa);
  x3 = x - options_profondeur * (-sia * cob - sia * sib * (-db) + (-da) * coa);
  x4 = x - options_profondeur * (-sia * cob - sia * sib * (-db) + da * coa);

  y1 = y + options_profondeur * (-sib + db * cob);
  y2 = y + options_profondeur * (-sib + db * cob);
  y3 = y + options_profondeur * (-sib + (-db) * cob);
  y4 = y + options_profondeur * (-sib + (-db) * cob);

  z1 = z + options_profondeur * (-coa * cob - coa * sib * db - sia * da);
  z2 = z + options_profondeur * (-coa * cob - coa * sib * db - sia * (-da));
  z3 = z + options_profondeur * (-coa * cob - coa * sib * (-db) - sia * (-da));
  z4 = z + options_profondeur * (-coa * cob - coa * sib * (-db) - sia * da);

  if (y1 * y3 <= 0)
    {
      c[0].x = ultra_find_x(x1,y1,x4,y4);
      c[0].y = ultra_find_x(z1,y1,z4,y4);
      c[1].x = ultra_find_x(x2,y2,x3,y3);
      c[1].y = ultra_find_x(z2,y2,z3,y3);
      i += 2;
      j++;
    }
  if (cam.beta <= 90)
    {
      if (y * y1 <= 0)
	{
	  c[i].x = ultra_find_x(x,y,x1,y1);
	  c[i].y = ultra_find_x(z,y,z1,y1);
	  i++;
	  c[i].x = ultra_find_x(x,y,x2,y2);
	  c[i].y = ultra_find_x(z,y,z2,y2);
	  i++;
	  j++;
	}
      if (y * y3 <= 0)
	{
	  c[i].x = ultra_find_x(x,y,x3,y3);
	  c[i].y = ultra_find_x(z,y,z3,y3);
	  i++;
	  c[i].x = ultra_find_x(x,y,x4,y4);
	  c[i].y = ultra_find_x(z,y,z4,y4);
	  i++;
	  j++;
	}
    }
  else
    {
      if (y * y3 <= 0)
	{
	  c[i].x = ultra_find_x(x,y,x3,y3);
	  c[i].y = ultra_find_x(z,y,z3,y3);
	  i++;
	  c[i].x = ultra_find_x(x,y,x4,y4);
	  c[i].y = ultra_find_x(z,y,z4,y4);
	  i++;
	  j++;
	}
       if (y * y1 <= 0)
	{
	  c[i].x = ultra_find_x(x,y,x1,y1);
	  c[i].y = ultra_find_x(z,y,z1,y1);
	  i++;

	  c[i].x = ultra_find_x(x,y,x2,y2);
	  c[i].y = ultra_find_x(z,y,z2,y2);
	  i++;
	  j++;
	}
    }

  if (!j)
    {
      return;
    }
  


  zinf = my_min(my_min(c[0].y,c[1].y),my_min(c[2].y,c[3].y)) / TAILLE_CASE;
  zsup = my_max(my_max(c[0].y,c[1].y),my_max(c[2].y,c[3].y)) / TAILLE_CASE;
  xinf = my_min(my_min(c[0].x,c[1].x),my_min(c[2].x,c[3].x)) / TAILLE_CASE;
  xsup = my_max(my_max(c[0].x,c[1].x),my_max(c[2].x,c[3].x)) / TAILLE_CASE;

  /*
  zinf--;
  xinf--;
  zsup++;
  xsup++;
  */
  if (zinf < 0)
    zinf = 0;
  if (xinf < 0)
    xinf = 0;
  if (zsup >= carte.y_max)
    zsup = carte.y_max - 1;
  if (xsup >= carte.x_max)
    xsup = carte.x_max - 1;

  
  if (pick_selected)
    {
      /* mes fonctions sont immondes mais personne regardera jamais mon code euh! tralalilalere! */
     
      /* attention! parcours inverse ici pour le picking */
      /* ici on dessinera les objets parce que c'est cool les objets */
      
      /* FAIRE DECREMENTATION POUR LES COULEURS */

      
      /* et ici si on a pas trouve d'objet clicke, on va faire une interpolation pour savoir ou on clicke */
      /* on verfie d'abord qu'on a pas clicke dans le vent... il faudrait dessiner un carre ^^ */

      if (!find_clicked (xinf,xsup,zinf, zsup))
	{
	  float propx, propy,px,py,pz,ux,uy;

	  glColor3f(0.0,0.0,1.0);
	  glBegin(GL_QUADS);
	  glVertex3f(xinf * TAILLE_CASE,0.0,zinf * TAILLE_CASE);
	  glVertex3f(xinf * TAILLE_CASE,0.0,(zsup+1) * TAILLE_CASE);
	  glVertex3f((xsup+1) * TAILLE_CASE,0.0,(zsup+1) * TAILLE_CASE);
	  glVertex3f((xsup+1) * TAILLE_CASE,0.0,zinf * TAILLE_CASE);
	  glEnd();

	  glReadPixels(sel_x,sel_y,1,1,GL_RGBA,GL_UNSIGNED_BYTE,&col);

	  if (!col[0])
	    {
	      
	      propx = (float)sel_x / (float) ultraWidth;
	      propy = 1.0 - (float)sel_y / (float) ultraHeight;
	      
	      px = x1 * (1 - propx - propy) + x2 * propx + x4 * propy;
	      py = y1 * (1 - propx - propy) + y2 * propx + y4 * propy;
	      pz = z1 * (1 - propx - propy) + z2 * propx + z4 * propy;
	      
	      ux = ultra_find_x(x,y,px,py);
	      uy = ultra_find_x(z,y,pz,py);
	      
	      i = (int)(ux / TAILLE_CASE);
	      j = (int)(uy / TAILLE_CASE);
	      
	      if ((i >= 0) && (i < carte.x_max) && (j >= 0) && (j < carte.y_max))
		{
		  clicked.type = IA_CASE;
		  clicked.clicked = &(carte.tab[i][j]);
		  clicked.x = i;
		  clicked.y = j;
		  redraw_info(&clicked); /**/
		  iactions[selected_iaction].ia(&clicked);
		}
	    }
	}
      else
	{
	  redraw_info(&clicked); /**/
	  iactions[selected_iaction].ia(&clicked);
	}
      
      
      if (options_lumiere)
	glEnable(GL_LIGHTING);
      glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
      pick_selected = 0;
    }
  
  
  
  if (afficher_stimuli)
    {
      t_signal proutchman=NULL;
      
      for (i = xinf;i <= xsup;i++)
	for (j = zinf;j <= zsup;j++)
	  {
	    if (cases[carte.tab[i][j].type].accessible)
	      {
		proutchman = carte.tab[i][j].signaux;
		while (proutchman)
		  {
		    if (proutchman->type == stimulus_selectionne)
		      {		      
			if (proutchman->force > max_sti)
			  {
			    max_sti = proutchman->force;
			    break;
			  }
			else
			  if (proutchman->force < min_sti)
			    {
			      min_sti = proutchman->force;
			      break;
			    }
		      }
		    proutchman = proutchman->next;
		  }
	      }
	  }
      if (min_sti == max_sti)
	min_sti *= 0.5;
      min_sti -= 0.1 * (max_sti - min_sti);
      max_sti -= min_sti;
    }
  
    
  if (cos(cam.alpha / RAD) < 0.0)
    {
      if (sin(cam.alpha / RAD) > 0.0)
	{
	  for (i = xsup;i >= xinf;i--)
	    {
	      for (j = zsup;j >= zinf;j--)
		{
		  draw_case(i,j);
		}
	    }
	}
      else
	{
	  for (i = xinf;i <= xsup;i++)
	    {
	      for (j = zsup;j >= zinf;j--)
		{
		  draw_case(i,j);
		}
	    }
	}
    }
  else
    {
      if (sin(cam.alpha / RAD) < 0.0)
	{
	  for (i = xinf;i <= xsup;i++)
	    {
	      for (j = zinf;j <= zsup;j++)
		{
		  draw_case(i,j);
		}
	    }
	}
      else
	{
	  for (i = xsup;i >= xinf;i--)
	    {
	      for (j = zinf;j <= zsup;j++)
		{
		  draw_case(i,j);
		}
	    }
	}
    }

    /*
  glColor3f(0.0,0.0,1.0);
  glBegin(GL_QUADS);
  glVertex3f(x1,y1,z1);
  glVertex3f(x2,y2,z2);
  glVertex3f(x3,y3,z3);
  glVertex3f(x4,y4,z4);
  glEnd();


 

    glColor3f(0.0,0.0,1.0);
  glBegin(GL_QUADS);
  glVertex3f(xinf,0.2,zinf);
  glVertex3f(xinf,0.2,zsup);
  glVertex3f(xsup,0.2,zsup);
  glVertex3f(xsup,0.2,zinf);
  glEnd();



  glBegin(GL_QUADS);
  glColor3f(1.0,0.0,0.0);
  glVertex3f(c[0].x,0.1,c[0].y);
  glColor3f(0.0,1.0,0.0);
  glVertex3f(c[1].x,0.1,c[1].y);
  glColor3f(0.0,0.0,1.0);
  glVertex3f(c[2].x,0.1,c[2].y);
  glColor3f(1.0,1.0,0.0);
  glVertex3f(c[3].x,0.1,c[3].y);
  glEnd();



  glColor3f(1.0,0.0,0.0);
  glBegin(GL_QUADS);
  glVertex3f(c[0].x-1.0,0.1,c[0].y-1.0);
  glVertex3f(c[0].x-1.0,0.1,c[0].y+1.0);
  glVertex3f(c[0].x+1.0,0.1,c[0].y+1.0);
  glVertex3f(c[0].x+1.0,0.1,c[0].y-1.0);
  glEnd();

  glColor3f(1.0,0.0,0.0);
  glBegin(GL_QUADS);
  glVertex3f(c[1].x-1.0,0.1,c[1].y-1.0);
  glVertex3f(c[1].x-1.0,0.1,c[1].y+1.0);
  glVertex3f(c[1].x+1.0,0.1,c[1].y+1.0);
  glVertex3f(c[1].x+1.0,0.1,c[1].y-1.0);
  glEnd();

  glColor3f(1.0,0.0,0.0);
  glBegin(GL_QUADS);
  glVertex3f(c[2].x-1.0,0.1,c[2].y-1.0);
  glVertex3f(c[2].x-1.0,0.1,c[2].y+1.0);
  glVertex3f(c[2].x+1.0,0.1,c[2].y+1.0);
  glVertex3f(c[2].x+1.0,0.1,c[2].y-1.0);
  glEnd();
  
  glColor3f(1.0,0.0,0.0);
  glBegin(GL_QUADS);
  glVertex3f(c[3].x-1.0,0.1,c[3].y-1.0);
  glVertex3f(c[3].x-1.0,0.1,c[3].y+1.0);
  glVertex3f(c[3].x+1.0,0.1,c[3].y+1.0);
  glVertex3f(c[3].x+1.0,0.1,c[3].y-1.0);
  glEnd();

    */

}

void add_agent (int x, int y, t_agent agent)
{
  t_case * cc = 0;

  if ((x>=0 && x<carte.x_max) && (y>=0 && y<carte.y_max))
    {
      cc = &(carte.tab[x][y]);
      agent->case_x = x;
      agent->case_y = y;
      agent->case_index = cc->nbr_ag;
      cc->nbr_ag++;
      cc->agents = realloc(cc->agents,cc->nbr_ag * sizeof(t_agent));
      (cc->agents)[agent->case_index] = agent;
    }
  else
    {
      printf("ag:%p x:%i y:%i x_max:%i y_max:%i\n",agent,x,y,carte.x_max,carte.y_max);
    }
}

void del_agent (int x,int y, t_agent agent)
{
  t_case * cc = 0;

  if ((x>=0 && x<carte.x_max) && (y>=0 && y<carte.y_max))
    {
      cc = &(carte.tab[x][y]);

      if (cc->nbr_ag <= agent->case_index || agent != cc->agents[agent->case_index])
	return;

      if (cc->nbr_ag == 1)
	{
	  free(cc->agents);
	  cc->agents = NULL;
	  cc->nbr_ag = 0;
	}
      else 
	{
	  cc->agents[agent->case_index] = cc->agents[--(cc->nbr_ag)];
	  cc->agents[agent->case_index]->case_index = agent->case_index;
	  realloc(cc->agents,cc->nbr_ag * sizeof(t_agent));
	}
    }
  else
    {
      printf("ag:%p x:%i y:%i x_max:%i y_max:%i\n",agent,x,y,carte.x_max,carte.y_max);
      caca("truc louche del_agent");
    }
}

void add_objet (int x, int y, t_objet objet)
{
  t_case * cc = 0;

  if ((x>=0 && x<carte.x_max) && (y>=0 && y<carte.y_max))
    {
      cc = &(carte.tab[x][y]);
      objet->case_x = x;
      objet->case_y = y;
      objet->case_index = cc->nbr_obj;
      cc->nbr_obj++;
      cc->objets = realloc(cc->objets,cc->nbr_obj * sizeof(t_objet));
      (cc->objets)[objet->case_index] = objet;
    }
  else
    {
      printf("ag:%p x:%i y:%i x_max:%i y_max:%i\n",objet,x,y,carte.x_max,carte.y_max);
      caca("truc louche add_objet");
    }
}

void del_objet (int x,int y, t_objet objet)
{
  t_case * cc = 0;

  if ((x>=0 && x<carte.x_max) && (y>=0 && y<carte.y_max))
    {
      cc = &(carte.tab[x][y]);

      if (cc->nbr_obj <= objet->case_index || objet != cc->objets[objet->case_index])
	return;

      if (cc->nbr_obj == 1)
	{
	  free(cc->objets);
	  cc->objets = NULL;
	  cc->nbr_obj = 0;
	}
      else 
	{
	  cc->objets[objet->case_index] = cc->objets[--(cc->nbr_obj)];
	  cc->objets[objet->case_index]->case_index = objet->case_index;
	  realloc(cc->objets,cc->nbr_obj * sizeof(t_objet));
	}
    }
  else
    {
      printf("ag:%p x:%i y:%i x_max:%i y_max:%i\n",objet,x,y,carte.x_max,carte.y_max);
      caca("truc louche del_objet");
    }
}

int suivre(picked * clicked)
{
  if (clicked->type == IA_CASE)
    {
      cam.xgoto = (clicked->x + 0.5) * TAILLE_CASE;
      cam.ygoto = (clicked->y + 0.5) * TAILLE_CASE;
      cam.follow_agent = NULL;
    }
  else
    {
      if (clicked->type == IA_AGENT)
	cam.follow_agent = clicked->clicked;
      else
	{
	  cam.xgoto = (*(t_objet)(clicked->clicked)).x;
	  cam.ygoto = (*(t_objet)(clicked->clicked)).z;
	  cam.follow_agent = NULL;
	}
    }
  cam.reached = 0;
  set_cam_delta();
  return(1);
}

