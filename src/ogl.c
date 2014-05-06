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
#include "loading.h"
#include "propag.h"
#include "discrimination.h"

#include "remote_control.h"
#include "map.h"                         

#define  _GNU_SOURCE
#include <dlfcn.h>

/**** Globals **************************/

extern GtkWidget *window;
extern GtkWidget *drawing_area;
extern GtkWidget *fs_t1,*fs_t2;

extern agent *agents;
extern objet *objets;

extern behavior * stimuli;
extern int nb_sti;
extern int options_lumiere;
extern float options_profondeur;
extern int connard;

extern double tps;
int depl=0;

int gHeight = 0;
int gWidth = 0;
int ultraHeight = 0, ultraWidth = 0;
float ratio;

/* bool redessiner */
int ramakicaca=0;


extern float pi;

/* gestion souris */;
extern int beginX, beginY;

/* donnees cam */
extern struct camera cam;

int stix=0,stiy=0,stif=0;

t_agent liste_agent = 0;
t_objet liste_objet = 0;

/* bool annimation */
int move=1;

/* global du billboard */
extern int show_billboard;


extern t_map carte;

extern primitive *primitives;
extern ccase *cases;
extern int nb_case;
extern void *(*free_case)();
extern void * lib;
int tour = 0;
extern int options_mipmap;
extern int cdepth;

/* sert pour faire avancer frame par frame */
int stepbystep=0;

/* bool activation lumiere */
int togglelightv=1;
int togglefs=0;

t_texture tex_index=NULL;
t_list_mod list_mod=0;
GLuint ground;

/* globale d'icones de primitive */
extern int ** prim_ico;

extern int realized;

/* chemin de l'exe */
extern char *chemin;

/* picking */
int pick_selected = 0;
int sel_x = 0,sel_y = 0;



GLfloat LightAmbient[]= { 1.0f, 1.0f, 1.0f, 0.1f };
GLfloat LightDiffuse[]= { 1.0f, 1.0f, 1.0f, 1.0f };	
GLfloat LightPosition[]= { 0.0f, 5.0f, 0.0f, 1.0f };


extern int mode_debug;

/* loading.c pour textures */
extern char * save_dir;
extern char * save ;

int afficher_ogl = 1;
extern int ogl_tps_loading;




void init_cam () 
{
  cam.zoom = -20.0f;
  cam.alpha = 0;
  cam.beta = 34.0f;
  cam.x = carte.x_max * TAILLE_CASE / 2;
  cam.y = carte.y_max * TAILLE_CASE / 2;
  cam.xgoto = cam.x;
  cam.ygoto = cam.y;
  cam.follow_agent = 0;
  cam.reached = 0;
}


/* h = hauteur au dessus de l'objet, l = 2 * longueur du cote du carre */
void draw_square (t_agent agent) 
{
  brick b;
  int i;
  float l = LARGEUR_BB ;

  if (agent->tache < 0)
    /* return ;*/ agent->tache = agent->type;


  glRotatef(-(*agent).ry,0.0f,-1.0f,0.0f);
  glRotatef((*agent).rx,-1.0f,0.0f,0.0f);
  glRotatef((*agent).rz,0.0f,0.0f,-1.0f);	  
 
  /* glTranslatef((*agent).x,(*agent).y,(*agent).z); */
  glRotatef(-cam.alpha,0.0f,0.1f,0.0f);
  /* glTranslatef(0.0f,h,0.0f); */ 
  glRotatef(-cam.beta,1.0f,0.0f,0.0f);

  glColor3f(1.0f,1.0f,1.0f);
  glEnable(GL_TEXTURE_2D);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);	

  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);

  /*
      /!\ en_cours est un int maintenant.
	  old:
              glBindTexture( GL_TEXTURE_2D,* prim_ico[obj->en_cours->p]);

	  new: merde faut parcourir la liste ca me pompe. dsl... je suis
	       obligé... et n'oubliez pas de rien afficher dans le cas 
	       du pointeur nul.

                         GayLord
  */

  b = stimuli[agent->tache].task.brick;
  i = 1;
 
  while (b && i<agent->en_cours)
    {
      b = b->next;
      i++;
    }

  if (b)
    {
      /*printf("%i\n",primoutives[b->p]);*/
      glBindTexture(GL_TEXTURE_2D,*primitives[b->p].bboard);

      glBegin(GL_QUADS);
      glTexCoord2f(0.0f,1.0f);
      glVertex3f(-l,l,0.0);
      glTexCoord2f(0.0f,0.0f);
      glVertex3f(-l,-l,0.0); 
      glTexCoord2f(1.0f,0.0f);
      glVertex3f(l,-l,0.0);
      glTexCoord2f(1.0f,1.0f);
      glVertex3f(l,l,0.0); 
      glEnd(); 
    }
  glDisable(GL_TEXTURE_2D);
  glDisable(GL_BLEND);  
    

  glRotatef(cam.beta,1.0f,0.0f,0.0f);
  /*glTranslatef(0.0f,-h,0.0f);*/ 
  glRotatef(cam.alpha,0.0f,0.1f,0.0f);
  glRotatef((*agent).rz,0.0f,0.0f,1.0f);
  glRotatef((*agent).rx,1.0f,0.0f,0.0f);
  glRotatef(-(*agent).ry,0.0f,1.0f,0.0f);
 
  /*  glTranslatef(-(*agent).x,-(*agent).y,-(*agent).z); */

}


void draw_objet (t_objet objet,int pic)
{
  int meshes,triangles,i,j;
  t_vertex * vertex;
  t_triangle * triangle;
  float x_init,y_init,z_init,ratio;
  
  if (objet != NULL)
    {
      x_init = (*objet).x;
      y_init = (*objet).y;
      z_init = (*objet).z;
      ratio = (*objet).ratio;
      
      /*   ce que j'ai rajouté, a vous de voir si ca vous convient.  *
       *   c'est pour se mettre la ou doit etre dessiné l'objet.       */
      
      
      glTranslatef(x_init,y_init,z_init);
      glRotatef((*objet).rz,0.0f,0.0f,1.0f);
      glRotatef((*objet).rx,1.0f,0.0f,0.0f);
      glRotatef(-(*objet).ry,0.0f,1.0f,0.0f);

      meshes = (*(*objet).model).frame[(*objet).step].meshes;
      for (j=0;j<meshes;j++)
	{
	  triangles = (*(*objet).model).frame[(*objet).step].mesh[j].triangles;
	  vertex = (*(*objet).model).frame[(*objet).step].mesh[j].vertex;
	  triangle = (*(*objet).model).frame[(*objet).step].mesh[j].triangle;



	  if (((*(*objet).model).frame[(*objet).step].mesh[j].material + 1) && !pic)
	    {
	      glColor3f(1.0,1.0,1.0);
	      glEnable(GL_TEXTURE_2D);
	      glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);

	      glBindTexture( GL_TEXTURE_2D,*(*(*objet).model).frame[(*objet).step].texture[(*(*objet).model).frame[(*objet).step].mesh[j].material]  );
	      glBegin(GL_TRIANGLES);
	      for (i=0;i<triangles;i++)
		{
		  
		  glNormal3f(triangle[i].normal.x,triangle[i].normal.y,triangle[i].normal.z);
		  glTexCoord2f(vertex[triangle[i].vertex_1].u, vertex[triangle[i].vertex_1].v);
		  glVertex3f(ratio * vertex[triangle[i].vertex_1].x,ratio * vertex[triangle[i].vertex_1].y,ratio * vertex[triangle[i].vertex_1].z);
		  glTexCoord2f(vertex[triangle[i].vertex_2].u, vertex[triangle[i].vertex_2].v);
		  glVertex3f(ratio * vertex[triangle[i].vertex_2].x,ratio * vertex[triangle[i].vertex_2].y,ratio * vertex[triangle[i].vertex_2].z);
		  glTexCoord2f(vertex[triangle[i].vertex_3].u, vertex[triangle[i].vertex_3].v);
		  glVertex3f(ratio * vertex[triangle[i].vertex_3].x,ratio * vertex[triangle[i].vertex_3].y,ratio * vertex[triangle[i].vertex_3].z);
		}
	      glEnd();
	      glDisable(GL_TEXTURE_2D);
	    }
	  else
	    {
	      glColor3f(0.2f,0.1f,0.1f); 
	      if (pic)
		glColor3ub(0,pic,0); /* limite le nombre d'objets a environ 254 */
	      

	      glBegin(GL_TRIANGLES);
	      for (i=0;i<triangles;i++)
		{
		  
		  glNormal3f(triangle[i].normal.x,triangle[i].normal.y,triangle[i].normal.z);
		  glVertex3f(ratio * vertex[triangle[i].vertex_1].x,ratio * vertex[triangle[i].vertex_1].y,ratio * vertex[triangle[i].vertex_1].z);
		  glVertex3f(ratio * vertex[triangle[i].vertex_2].x,ratio * vertex[triangle[i].vertex_2].y,ratio * vertex[triangle[i].vertex_2].z);
		  glVertex3f(ratio * vertex[triangle[i].vertex_3].x,ratio * vertex[triangle[i].vertex_3].y,ratio * vertex[triangle[i].vertex_3].z);
		}
     
	      glEnd();
	    }
	}
      glRotatef(-(*objet).ry,0.0f,-1.0f,0.0f);
      glRotatef((*objet).rx,-1.0f,0.0f,0.0f);
      glRotatef((*objet).rz,0.0f,0.0f,-1.0f);	  
      glTranslatef(-x_init,-y_init,-z_init);
    }
}


void draw_agent (t_agent agent,int pic)
{
  int meshes, triangles, i, j;
  t_vertex * vertex;
  t_triangle * triangle;
  float x_init, y_init, z_init, ratio, h = 0.0;
  //  glEnable( GL_COLOR_MATERIAL );
  if (agent != NULL)
    {
      x_init = (*agent).x;
      y_init = (*agent).y;
      z_init = (*agent).z;
      ratio = (*agent).ratio;

      glTranslatef(x_init,y_init,z_init);
      glRotatef((*agent).rz,0.0f,0.0f,1.0f);
      glRotatef((*agent).rx,1.0f,0.0f,0.0f);
      glRotatef(-(*agent).ry,0.0f,1.0f,0.0f);


      /*printf("%f %f %f\n", x_init, y_init, z_init);*/

      meshes = (*(*agent).model).frame[(*agent).step].meshes;
      for (j=0;j<meshes;j++)
	{
	  /*
	  triangles = (*(*agent).model).frame[(*agent).step].mesh[j].triangles;
	  vertex = (*(*agent).model).frame[(*agent).step].mesh[j].vertex;
	  triangle = (*(*agent).model).frame[(*agent).step].mesh[j].triangle;
	  */


	  triangles = ((t_3Dobj *)(agents[agent->type].mod))->frame[(*agent).step].mesh[j].triangles;
	  vertex = ((t_3Dobj *)(agents[agent->type].mod))->frame[(*agent).step].mesh[j].vertex;
	  triangle = ((t_3Dobj *)(agents[agent->type].mod))->frame[(*agent).step].mesh[j].triangle;

	  /*   ce que j'ai rajouté, a vous de voir si ca vous convient.  *
	   *   c'est pour se mettre la ou doit etre dessiné l'agent.       */




	  if (((*(*agent).model).frame[(*agent).step].mesh[j].material + 1) && !pic)
	    {
	      glColor3f(1.0,1.0,1.0);
	      glEnable(GL_TEXTURE_2D);
	      glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);

	      glBindTexture( GL_TEXTURE_2D,*(*(*agent).model).frame[(*agent).step].texture[(*(*agent).model).frame[(*agent).step].mesh[j].material]  );
	      glBegin(GL_TRIANGLES);
	      for (i=0;i<triangles;i++)
		{
		  
		  glNormal3f(triangle[i].normal.x,triangle[i].normal.y,triangle[i].normal.z);
		  glTexCoord2f(vertex[triangle[i].vertex_1].u, vertex[triangle[i].vertex_1].v);
		  glVertex3f(ratio * vertex[triangle[i].vertex_1].x,ratio * vertex[triangle[i].vertex_1].y,ratio * vertex[triangle[i].vertex_1].z);
		  glTexCoord2f(vertex[triangle[i].vertex_2].u, vertex[triangle[i].vertex_2].v);
		  glVertex3f(ratio * vertex[triangle[i].vertex_2].x,ratio * vertex[triangle[i].vertex_2].y,ratio * vertex[triangle[i].vertex_2].z);
		  glTexCoord2f(vertex[triangle[i].vertex_3].u, vertex[triangle[i].vertex_3].v);
		  glVertex3f(ratio * vertex[triangle[i].vertex_3].x,ratio * vertex[triangle[i].vertex_3].y,ratio * vertex[triangle[i].vertex_3].z);
		}
	      glEnd();
	      glDisable(GL_TEXTURE_2D);
	    }
	  else
	    {
	      if (connard)
		glColor3f(1.0f,0.5f,0.8f); 
	      else
		glColor3f(0.2f,0.1f,0.1f); 
	      if (pic)
		glColor3ub(pic,0,0); /* limite le nombre d'objets a environ 254 */
	      
	      glBegin(GL_TRIANGLES);
	      for (i=0;i<triangles;i++)
		{
		  glNormal3f(triangle[i].normal.x,triangle[i].normal.y,triangle[i].normal.z);
		  glVertex3f(ratio * vertex[triangle[i].vertex_1].x,ratio * vertex[triangle[i].vertex_1].y,ratio * vertex[triangle[i].vertex_1].z);
		  glVertex3f(ratio * vertex[triangle[i].vertex_2].x,ratio * vertex[triangle[i].vertex_2].y,ratio * vertex[triangle[i].vertex_2].z);
		  glVertex3f(ratio * vertex[triangle[i].vertex_3].x,ratio * vertex[triangle[i].vertex_3].y,ratio * vertex[triangle[i].vertex_3].z);
		}
     
	      glEnd();
	    }
	  //glDisable( GL_COLOR_MATERIAL );	  
	}
      h =  agent->ratio * agent->model->max;
      glTranslatef(0.0,h,0.0);
      if (agent->caddie)
	{
	  if (agent->is_caddie_ag)
	    {
	      ((t_agent) agent->caddie)->x = 0;
	      ((t_agent) agent->caddie)->y = 0;
	      ((t_agent) agent->caddie)->z = 0;
	      h += HAUTEUR_OBJ;
	      glTranslatef(0.0,HAUTEUR_OBJ,0.0);
	      draw_agent((t_agent) agent->caddie,0);
	      h += ((t_agent)agent->caddie)->ratio * ((t_agent)agent->caddie)->model->max;
	      glTranslatef(0.0,((t_agent)agent->caddie)->ratio * ((t_agent)agent->caddie)->model->max,0.0);
	    }
	  else
	    {
	      ((t_objet) agent->caddie)->x = 0;
	      ((t_objet) agent->caddie)->y = 0;
	      ((t_objet) agent->caddie)->z = 0;
	      h += HAUTEUR_OBJ;
	      glTranslatef(0.0,HAUTEUR_OBJ,0.0);
	      draw_objet((t_objet) agent->caddie,0);
	      h += ((t_objet)agent->caddie)->ratio * ((t_objet)agent->caddie)->model->max;
	      glTranslatef(0.0,((t_objet)agent->caddie)->ratio * ((t_objet)agent->caddie)->model->max,0.0);
	    }
	}
      h += HAUTEUR_BB;
      glTranslatef(0.0,HAUTEUR_BB,0.0);
      if (show_billboard)
	draw_square(agent);
      else
	if (cam.follow_agent == agent)
	  draw_square(cam.follow_agent);  /* optimisable */

      glTranslatef(0.0,-h,0.0);

      glRotatef(-(*agent).ry,0.0f,-1.0f,0.0f);
      glRotatef((*agent).rx,-1.0f,0.0f,0.0f);
      glRotatef((*agent).rz,0.0f,0.0f,-1.0f);	  
      glTranslatef(-x_init,-y_init,-z_init);

      
    }
  //  glDisable( GL_COLOR_MATERIAL );	  
}
  

/* DEPRECATED
   void draw_ground ()
   {
   int i,j;

   glColor3f(1.0f,1.0f,1.0f);
   glEnable(GL_TEXTURE_2D);

   for (i=0;i<carte.x_max;i++)
   for (j=0;j<carte.y_max;j++)
   {
   glBindTexture( GL_TEXTURE_2D, *carte.tab[i][j].texture );
   glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
   glBegin(GL_QUADS);
   glTexCoord2f(0.0f, 0.0f); glVertex3f(i * TAILLE_CASE, 0.0f, j * TAILLE_CASE);  
   glTexCoord2f(1.0f, 0.0f); glVertex3f((i+1) * TAILLE_CASE, 0.0f, j * TAILLE_CASE);  
   glTexCoord2f(1.0f, 1.0f); glVertex3f((i+1) * TAILLE_CASE, 0.0f, (j+1) * TAILLE_CASE);  
   glTexCoord2f(0.0f, 1.0f); glVertex3f(i * TAILLE_CASE, 0.0f, (j+1) * TAILLE_CASE);   
   glEnd(); 

   }
   glDisable(GL_TEXTURE_2D);  
   }
*/

void drawt()
{
  glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
  glLoadIdentity();

  /* restriction cam */
  if (!mode_debug)
    {
      if (cam.beta<=BETA_INF || cam.beta > 200)
	cam.beta=BETA_INF;
      /*      if (cam.zoom<ZOOM_INF)
	      cam.zoom=ZOOM_INF;*/
      
      if (cam.beta>BETA_SUP)
	cam.beta=BETA_SUP;
      if (cam.zoom>ZOOM_SUP)
	cam.zoom=ZOOM_SUP;
    }

  modif_cam_remote_ctl();



  glTranslatef(0.0f,0.0f,cam.zoom);
  glRotatef(cam.beta,1.0f,0.0f,0.0f);
  glRotatef(cam.alpha,0.0f,0.1f,0.0f);
  glTranslatef(-1.0*cam.x,0.0f,-1.0*cam.y);


  /*
    glTranslatef(0.0f,0.0f,cam.zoom);
    glRotatef(cam.beta,1.0f,0.0f,0.0f);
    glRotatef(cam.alpha,0.0f,0.1f,0.0f);
  */
}

/*init*/
void realize (GtkWidget *widget, gpointer data UNUSED)
{
  char *t1,*t2,*t3,*t4,*t5;
  /* void * (*init_cases)(); */

  realized = 1;

  GdkGLContext *glcontext = gtk_widget_get_gl_context (widget);
  GdkGLDrawable *gldrawable = gtk_widget_get_gl_drawable (widget);
  int Height = widget->allocation.height, Width = widget->allocation.width;

  /*** OpenGL BEGIN ***/
  if (!gdk_gl_drawable_gl_begin (gldrawable, glcontext))
    return;

  /* init agent fourmi */
  /* a supprimer !!!*/
  
  /* init_libs */
  t1 = my_concat(chemin,save_dir);
  t4 = my_concat(t1,"/");
  t5 = my_concat(t4,save);
  t2 = my_concat(t5,"/lib/lib_bbs_behavior.so");
  t3 = my_concat(chemin,"/../lib/lib_bbs_behavior.so");
  
  /*  *(void **) (&init_cases) = dlsym(lib,"init_cases"); */

  reload_textures();

  /*
    if (cases)
    free_case(cases,nb_case);

  if ((f = fopen(t2,"r")))
    {
      fclose(f);
      init_cases(&cases,&nb_case,t2);
    }
  else
    init_cases(&cases,&nb_case,t3);
  */

  free(t1);
  free(t2);
  free(t3);
  free(t4);
  free(t5);
  
  /* Qu'est ce qu'il fou la se connard ? il aurait du sauter y'a longtemps !! */
  /* hop c'est ce qu'il fait */
  /* reload_obj3D(); */

  /*  evb_load();*/

  glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
  glClearDepth(1.0);			
  glDepthFunc(GL_LESS);			
  glEnable(GL_DEPTH_TEST);		
  glShadeModel(GL_SMOOTH); 
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();			

  /* if (options_mipmap)
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  else
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  */

  gHeight = 2.0 * tan(FOVY / 2.0) * options_profondeur;
  gWidth = 2.0 * tan(FOVY / 2.0) * (Width / Height) * options_profondeur ;

  ratio = (GLfloat)Width/(GLfloat)Height;

  gluPerspective(FOVY,ratio,0.1f,options_profondeur);
  
  glMatrixMode(GL_MODELVIEW);

  gdk_gl_drawable_gl_end (gldrawable);
  
    glColorMaterial(GL_FRONT,GL_AMBIENT_AND_DIFFUSE);
  //glColorMaterial(GL_FRONT,GL_AMBIENT);
  glEnable( GL_COLOR_MATERIAL );
  //  GL_EMISSION, GL_AMBIENT, GL_DIFFUSE, GL_SPECULAR,  and
  //         GL_AMBIENT_AND_DIFFUSE.  
  //glColorMaterial(GL_BACK ,GL_SPECULAR);
  //glEnable( GL_COLOR_MATERIAL );  


  glGetIntegerv(GL_BLUE_BITS,&cdepth);
  cdepth = 256 / (1 << (cdepth -1));
  cdepth = (cdepth)?(cdepth):1;

  glLightfv(GL_LIGHT1, GL_DIFFUSE, LightDiffuse);      
  glLightfv(GL_LIGHT1, GL_AMBIENT, LightAmbient);  
  glLightfv(GL_LIGHT1, GL_POSITION,LightPosition);

  glEnable(GL_LIGHT1);
  glEnable(GL_LIGHTING);
}

/*reshape*/
gboolean configure_event (GtkWidget *widget, GdkEventConfigure *event UNUSED, gpointer data UNUSED)
{
  GdkGLContext *glcontext = gtk_widget_get_gl_context (widget);
  GdkGLDrawable *gldrawable = gtk_widget_get_gl_drawable (widget);
  int Height=widget->allocation.height ,Width=widget->allocation.width;

  /*** OpenGL BEGIN ***/
  if (!gdk_gl_drawable_gl_begin (gldrawable, glcontext))
    return FALSE;

  if (Height==0)	
    Height=1;

  glViewport(0, 0, Width, Height);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  gHeight = 2.0 * tan(FOVY / 2.0) * options_profondeur;
  gWidth = 2.0 * tan(FOVY / 2.0) * ((float)Width / (float)Height) * options_profondeur;

  ultraHeight = Height;
  ultraWidth = Width;

  gluPerspective(FOVY,(GLfloat)Width/(GLfloat)Height,0.1f,options_profondeur);
  glMatrixMode(GL_MODELVIEW);
  gdk_gl_drawable_gl_end (gldrawable);
  /*** OpenGL END ***/

  if (!ramakicaca)
    {
      ramakicaca++;
      gtk_widget_queue_resize(widget);
    }
  else
    ramakicaca--;

  return TRUE;
}

void picking ()
{
  t_agent tmp=liste_agent,pic=0;
  GLubyte col[]={255,255,255,1},mem[]={255,255,255};
  
  glDisable(GL_LIGHTING);
  
  while (tmp)
    {
      draw_agent(tmp,1);
      glReadPixels(sel_x,sel_y,1,1,GL_RGBA,GL_UNSIGNED_BYTE,&col);
      if ((col[0] != mem[0]) || (col[1] != mem[1]))
	pic = tmp;
      mem[0] = col[0];
      mem[1] = col[1];
      tmp = (*tmp).next;
    }
  if (pic)
    {
      cam.follow_agent = pic;
      cam.reached = 0;	  
    }
  
  if (options_lumiere)
    glEnable(GL_LIGHTING);
  glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
  pick_selected = 0;
}


/*draw*/
gboolean expose_event (GtkWidget *widget, GdkEventExpose *event UNUSED, gpointer data UNUSED)
{

  GdkGLContext *glcontext = gtk_widget_get_gl_context (widget);
  GdkGLDrawable *gldrawable = gtk_widget_get_gl_drawable (widget);
      
      
  /*** OpenGL BEGIN ***/
  if (!gdk_gl_drawable_gl_begin (gldrawable, glcontext))
    return FALSE;


  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  if (afficher_ogl && ogl_tps_loading)
    {
      drawt();  
      draw_scene();
    }
  //traiter_fourmi();
  gdk_gl_drawable_swap_buffers (gldrawable);
  gdk_gl_drawable_gl_end (gldrawable);
  /*** OpenGL END ***/
      
  gtk_widget_queue_draw(GTK_WIDGET(widget)); /* demande son rafraichissement immédiat */

  traiter_fourmi();
  traiter_objet();
  /*
  tmp = liste_agent;
  while(tmp)
    {
      action_agent(tmp);
      tmp=tmp->next;
    }
  */
  return TRUE;
}

gboolean redraw(GtkWidget * widget) /* le widget à rafraîchir */
{
  gtk_widget_queue_draw(GTK_WIDGET(widget)); /* demande son rafraichissement immédiat */
  return 1;
}


void togglelight ()
{
  if (togglelightv)
    glEnable(GL_LIGHTING);
  else
    glDisable(GL_LIGHTING);
  togglelightv = !togglelightv;
}

void togglefullscreen ()
{


  if (togglefs)
    {
      gtk_container_set_border_width (GTK_CONTAINER (window), 10);
      gtk_widget_show_all(window);      
      gtk_widget_reparent(drawing_area,fs_t2);
      gtk_window_unfullscreen(GTK_WINDOW (window));
    }
  else
    {
      gtk_container_set_border_width (GTK_CONTAINER (window), 0);
      gtk_widget_hide_all(fs_t1);
      gtk_widget_show(fs_t1);
      gtk_widget_reparent(drawing_area,fs_t1);
      gtk_widget_show(drawing_area);
      gtk_window_fullscreen(GTK_WINDOW (window));
    }
  
  togglefs = !togglefs;
}


/*mouse*/
gboolean motion_notify_event (GtkWidget      *widget,
			      GdkEventMotion *event,
			      gpointer        data UNUSED)
{

  if (event->state & GDK_BUTTON3_MASK)
    {
      cam.alpha= (int) (cam.alpha + event->x - beginX +360)%360;
      cam.beta= (int) (cam.beta + event->y - beginY +360)%360;
      redraw(widget);
    }

  if (event->state & GDK_BUTTON2_MASK)
    {
      if(!depl)
	{
	  cam.zoom-=(event->y - beginY)*.1;
	  /*redraw(widget);*/
	}
      else
	{
	  cam.follow_agent=0;
	  cam.reached = 1;
	  cam.xgoto=cam.x+tps*(abs(cam.zoom/10.0)+1.0)*VITXY_MOUSE*((beginX-event->x)*my_cos(cam.alpha)+(beginY-event->y)*my_cos(cam.alpha+90));
	  cam.ygoto=cam.y+tps*(abs(cam.zoom/10.0)+1.0)*VITXY_MOUSE*((beginX-event->x)*my_sin(cam.alpha)+(beginY-event->y)*my_sin(cam.alpha+90));
	}
      redraw(widget);
    }
  beginX = event->x;
  beginY = event->y;

  return TRUE;
}









gboolean button_press_event (GtkWidget      *widget,
			     GdkEventButton *event,
			     gpointer        data UNUSED)
{
  /* picking */
  if (event->button == 1)
    {
      pick_selected = 1;
      gtk_widget_get_pointer(widget,&sel_x,&sel_y);
      sel_y = (*widget).allocation.height - sel_y;
      beginX = event->x;
      beginY = event->y;
      return TRUE;
    }

  if (event->button == 2)
    {
      beginX = event->x;
      beginY = event->y;
      return TRUE;
    }

  if (event->button == 3)
    {
      beginX = event->x;
      beginY = event->y;
      return TRUE;
    }
  return 0;
}

static inline int my_rand(int n)
{
  return(1 + (int) (n * (rand() / (RAND_MAX + 1.0))));
}

/*kb*/
      int xxx=0;
      int yyy=0;
gboolean key_press_event (GtkWidget *widget UNUSED, GdkEventKey *event, gpointer data UNUSED)
{
  switch (event->keyval)
    {
    case GDK_F4:
      if (realized)
	togglefullscreen ();
      break;
      /* debug ogl */
    case  GDK_F3:
      if(depl)                          
	depl=0;
      else
	depl=1;
      break;
      /*
    case GDK_F1:  
      stix = my_rand(carte.x_max-2);
      stiy = my_rand(carte.y_max-2);
      stif = my_rand(10)+5;

      propagate_stimulus(stix,stiy,1,stif,1);
      break;
      
      case GDK_F2:
     depropagate_stimulus(stix,stiy,1,stif,1);
      break;

    case GDK_F6:  
      creer_objet("chameau",xxx++,yyy++,0.0,0.0,0.0,0.002,0,1);
      break;

    case GDK_F7:
      if (liste_agent)
	liste_agent->caddie = creer_objet("chameau",0,0,0.0,0.0,0.0,0.002,0,0);
      break;

   
    case GDK_F5:
      if(liste_agent)
	discriminer(liste_agent);
      break;

    case GDK_F9:
      propagate_stimulus(18,14,1,100,1);
      break;

    case GDK_F10:
      depropagate_stimulus(18,14,1,100,1);
      break;
      */

    case GDK_F12:
      mode_debug = !mode_debug;
      break;

    case GDK_Escape:
      gtk_main_quit ();
      /************/
      break;
    default:
      return FALSE;
    }

  return TRUE;
}



/* gtk->gl */

void toggleanim () 
{
  move = !move;
  if (move)
    stepbystep=0;
}

/** gl *****************************************************/

