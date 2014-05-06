/* Loader de .lbo "Loulou Berserker Object" by Goulagman */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <math.h>
#include <gtk/gtk.h>
#include "lib_bbs_behavior.h"
#include "loader.h"
#include "divers.h"
#include "textures.h"
#include "loading.h"

#include <locale.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>


/* loading.c pour textures */
extern char * save_dir;
extern char * save ;
extern int realized;

float max = 0.0;   /* pour le sommet le plus haut */

extern char *chemin;
extern t_texture tex_index;
extern t_list_mod list_mod;

t_vertex * create_vertex_list(int vertices)
{
  t_vertex * vertex;
  
  vertex = malloc(vertices * sizeof(t_vertex));
  if ( !vertex )
    perror("create_vertex_list : memory allocation error");
  return(vertex);
}

t_normal * create_normal_list(int normals)
{
  t_normal * normal;
  
  normal = malloc(normals * sizeof(t_normal));
  if ( !normal )
    perror("create_normal_list : memory allocation error");
  return(normal);
}

t_triangle * create_triangle_list(int triangles)
{
  t_triangle * triangle;
  
  triangle = malloc(triangles * sizeof(t_triangle));
  if ( !triangle )
    perror("create_triangle_list : memory allocation error");
  return(triangle);
}

t_mesh * create_mesh_list(int meshes)
{
  t_mesh * mesh;

  mesh = malloc(meshes * sizeof(t_mesh));
  if ( !mesh )
    perror("create_mesh_list : memory allocation error");
  return(mesh);
}

/* DEPRECATED
GLuint * create_GLuint_list(int textures)
{
  GLuint * texture;
  
  texture = malloc(textures * sizeof(GLuint));
  if ( !texture )
    perror("create_GLuint_list : memory allocation error");
  return(texture);
}
*/

char * uconcat(char * str1,char * str2,char * str3)
{
  int i,j,k;
  char * tmp;

  i = (str1)?strlen(str1):0;
  j = (str2)?strlen(str2):0;
  k = (str3)?strlen(str3):0;
  tmp = malloc((i + j + k + 1) * sizeof(char));
  memcpy(tmp,str1,i * sizeof(char));
  memcpy(tmp + i,str2,j * sizeof(char));
  memcpy(tmp + i + j,str3,k * sizeof(char));
  tmp[i+j+k] = 0;
  return(tmp);
}

/* met la norme du vecteur a 1 */
void normalize (t_normal * nor)
{
  float norm;

  norm = (float)sqrt((*nor).x * (*nor).x + (*nor).y * (*nor).y + (*nor).z * (*nor).z);
  norm = norm ? norm : 1.0;
  (*nor).x /= norm;
  (*nor).y /= norm;
  (*nor).z /= norm;
}

/* produit vectoriel */
t_normal p_vect (t_normal u, t_normal v)
{
  t_normal w;

  w.x = u.y * v.z - u.z * v.y;
  w.y = u.z * v.x - u.x * v.z;
  w.z = u.x * v.y - u.y * v.x;

  return(w);
}


/* compare 2 strings */

/* load une texture dans l'index */

GLuint * Load_Texture (char * name)
{
  t_texture tmp;
  int l=1;

   /* on verifie que la texture n'existe pas deja */
  tmp = tex_index;
  while (tmp && (l = strcmp((*tmp).name,name)))
    tmp = (*tmp).next;
 
  if (!l)
    return(&(*tmp).id);

  tmp = malloc(sizeof(struct s_texture));
  l = strlen(name);
  (*tmp).name = malloc((l + 1) * sizeof(char));
  memcpy((*tmp).name,name,l+1);
  (*tmp).next = tex_index;
  (*tmp).id = (realized)?glTexLoad(name):0;
  tex_index = tmp;

  return(&((*tmp).id));
}

void reload_textures ()
{
  t_texture tmp=tex_index;
  
   while(tmp)
    {
      tmp->id = glTexLoad(tmp->name);
      tmp=tmp->next;
    }
}

int intlen (int n)
{
  int i=0;

  while (n != 0)
    {
      n /= 10;
      i++;
    }
  return(i);
}

/***************************************************************************************************BALISE***/
t_frame load_frame (char * model, int numframe)
{
 
  /* 3D object made by Loulou for Berserkers with love */
  t_frame frame;
  FILE * loulou;
  char row[256];
  char c;
  char * tmp,*t2;
  int i,j,n1,n2,n3;
  int vertices,normals,triangles,meshes,textures;
  float ramakiprout;
  t_mesh * mesh;
  t_vertex * vertex;
  t_normal * gl_normal,u,v;
  t_triangle * triangle;
  GLuint ** texture;

  /* File open */
  tmp = malloc((intlen(numframe) + 1) * sizeof(char));
  sprintf(tmp,"%i",numframe);

  /*t1=my_concat(chemin,"/../share/bebetes_show/obj3D/");*/
  t2 = uconcat(model,tmp,".lbo");
  loulou = fopen(t2,"r");
  if (!loulou)
    perror("load frame : 3D Object not found");
  /*   
       ah ben bravo, c'est du joli !
       je me suis permi de free-er tes strings aussi... 

       ouais ben jtemmerde enculeur de vaches! :p 
  */

  free(t2);
  free(tmp);

 
  /* On vire les lignes inutiles du debut */
  fgets(row,255,loulou); /* header */
  fgets(row,255,loulou); /* empty */
  fgets(row,255,loulou); /* total frames */
  fgets(row,255,loulou); /* current frame */
  fgets(row,255,loulou); /* empty */
  fgets(row,9,loulou);   /* reads "Meshes: " */

  fscanf(loulou,"%i",&meshes);
  c = fgetc(loulou);

  mesh = create_mesh_list(meshes); 
 
  /* reading name of mesh */
  c = fgetc(loulou);
  
  /* creating frames */
  
  frame.meshes = meshes;
  
  for (j=0;j<meshes;j++)
    {
      c = fgetc(loulou);
      
      
      i=0;
      
      while ((c = fgetc(loulou)) != 34)
	{
	  row[i] = c;
	  i++;
	}
      mesh[j].name = malloc((i + 1) * sizeof(char));
      memcpy(mesh[j].name,row,i * sizeof(char));
      mesh[j].name[i] = 0;
      c = fgetc(loulou);
      
      fscanf(loulou,"%i",&mesh[j].flags);
      c = fgetc(loulou);
      fscanf(loulou,"%i",&mesh[j].material);
      
      
      
      /* reading vertices */
      
      fscanf(loulou,"%i",&vertices);
      c = fgetc(loulou);
      
      vertex = create_vertex_list(vertices);
      for(i=0;i<vertices;i++)
	{
	  fscanf(loulou,"%i %f %f %f %f %f %i",
		 &vertex[i].flags,&vertex[i].x,
		 &vertex[i].y,&vertex[i].z,
		 &vertex[i].u,&vertex[i].v,
		 &vertex[i].bone_index );
	  if (vertex[i].y > max)
	    max = vertex[i].y;
	  c = fgetc(loulou);
	}

      
      /* reading normals */
      fscanf(loulou,"%i",&normals);
      c = fgetc(loulou);
      
      for(i=0;i<normals;i++)
	{
	  fscanf(loulou,"%f %f %f",
		 &ramakiprout,&ramakiprout,
		 &ramakiprout); 
	  c = fgetc(loulou); 
	}
      
      /* reading triangles */
      
      fscanf(loulou,"%i",&triangles);
      c = fgetc(loulou);
      
      triangle = create_triangle_list(triangles);
      gl_normal = create_normal_list(triangles);
      
      for(i=0;i<triangles;i++)
	{
	  fscanf(loulou,"%i %i %i %i %i %i %i %i",
		 &triangle[i].flags,&triangle[i].vertex_1,
		 &triangle[i].vertex_2,&triangle[i].vertex_3,
		 &n1,&n2,
		 &n3,&triangle[i].s_group);
	  u.x = vertex[triangle[i].vertex_1].x - vertex[triangle[i].vertex_2].x;
	  u.y = vertex[triangle[i].vertex_1].y - vertex[triangle[i].vertex_2].y;
	  u.z = vertex[triangle[i].vertex_1].z - vertex[triangle[i].vertex_2].z;
	  v.x = vertex[triangle[i].vertex_2].x - vertex[triangle[i].vertex_3].x;
	  v.y = vertex[triangle[i].vertex_2].y - vertex[triangle[i].vertex_3].y;
	  v.z = vertex[triangle[i].vertex_2].z - vertex[triangle[i].vertex_3].z;
	  
	  u = p_vect(u,v);
	  normalize(&u);
	  triangle[i].normal = u;
	  
	  
	  c = fgetc(loulou);
	}
      c = fgetc(loulou);
      mesh[j].vertex = vertex;
      mesh[j].triangle = triangle;
      mesh[j].vertices = vertices;
      mesh[j].triangles = triangles;
      free(gl_normal);
      
    }
  
  frame.mesh = mesh;

  /* maintenant on load les textures! */
  fgets(row,255,loulou);
  fgets(row,11,loulou); /* reads "Materials: " */
  fscanf(loulou,"%i",&textures);
  c = fgetc(loulou);
  c = fgetc(loulou);
  /* On va faire sans
     texture = create_GLuint_list(textures); */
  /* a free-er */
  texture = malloc(textures * sizeof(GLuint *));
  for(i=0;i<textures;i++)
    {
      fgets(row,255,loulou);  /* name of the material */
      fgets(row,255,loulou);  /* Ambient */
      fgets(row,255,loulou);  /* Diffuse */
      fgets(row,255,loulou);  /* Specular */
      fgets(row,255,loulou);  /* Emission */
      fgets(row,255,loulou);  /* Shininess */
      fgets(row,255,loulou);  /* Transparency */

      c = getc(loulou); /* reads " */
      c = getc(loulou); /* reads . */
      c = getc(loulou); /* reads / */
      j=0;
      
      while ((c = fgetc(loulou)) != 34)
	{
	  row[j] = c;
	  j++;
	}
      tmp = malloc((j + 1) * sizeof(char));
      memcpy(tmp,row,j * sizeof(char));
      tmp[j] = 0;
      texture[i] = Load_image(tmp);
      free(tmp);
      
      c = getc(loulou); /* reads \n */
      fgets(row,255,loulou);  /* Alpha map */
      fgets(row,255,loulou);  /* empty */
    }
  
  frame.texture = texture;

  fclose(loulou);
  
  return(frame);
}

/***************************************************************************************************BALISE***/
t_3Dobj * create_3Dobj(char * model)
{
  t_3Dobj * obj;
  int i=1, numframe=0;
  t_list_mod tmp;
  FILE * loulou,*f;    /* pour voir le nombre total de frames */
  char row[256];
  char *t1,*t2,*t3,*t4,*t5,*t6;

  t1 = my_concat("",save_dir);
  t2 = my_concat(t1,"/");
  t3 = save ? my_concat(t2,save) : my_concat(t2,"");
  t4 = my_concat(t3,"/obj_3D/");
  t5 = my_concat(t4,model);
  t6 = my_concat(t5,"1.lbo");

  max = 0.0;

  if ((f = fopen(t6,"r")))
    fclose(f);  
  else
    {
      free(t5);
      free(t6);
      free(t1);
      t1 = my_concat(chemin,"/../share/bebetes_show/obj3D/");
      t5 = my_concat(t1,model);
      t6 = my_concat(t5,"1.lbo");
    }

  tmp = list_mod;

  while ((tmp) && (i = strcmp((*tmp).name,model)))
    {
      tmp = (*tmp).next;
    }
  if (!i)
    return((*tmp).model);

  /*    File open */
  loulou = fopen(t6,"r");
  if (!loulou)
    perror("load frame : 3D Object not found");

  /* On vire les lignes inutiles du debut */
  fgets(row,255,loulou); /* header */
  fgets(row,255,loulou); /* empty */
  fgets(row,8,loulou);   /* reads "Frames: " */
  fscanf(loulou,"%i",&numframe);
  fclose(loulou);


  obj = malloc(sizeof(t_3Dobj));
  (*obj).frame = malloc(numframe * sizeof(t_frame));

  start_locale();
  for (i=1;i<=numframe;i++)
    (*obj).frame[i-1] = load_frame(t5,i);
  end_locale();

  (*obj).frames = numframe;
  obj->max = max;

  free(t1);
  free(t2);
  free(t3);
  free(t4);
  free(t5);
  free(t6);
  
  tmp = malloc(sizeof (struct s_list_mod));
  tmp->name = model;
  tmp->model = obj;
  tmp->next = list_mod;
  list_mod = tmp;
  
  return(obj);
}


void free_mesh(t_mesh * mesh)
{
  free((*mesh).triangle);
  free((*mesh).vertex);
  free((*mesh).name);
}

void free_frame(t_frame * frame) 
{
  int i;

  for (i=0;i<(*frame).meshes;i++)
    free_mesh((*frame).mesh + i);
  free((*frame).mesh);
  free((*frame).texture);
}

void free_3Dobj(t_3Dobj * model)
{
  int i;
  for (i=0;i<(*model).frames;i++)
    free_frame((*model).frame + i);
  free((*model).frame);
  free(model);
}

void free_list_mod ()
{
  t_list_mod tmp,n;


  /*
  if (list_mod != NULL)
    {
      free_list_mod((*list_mod).next);
      free_3Dobj((*list_mod).model);
      free ((*list_mod).name);
      free(list_mod);
      list_mod = 0;
    }
  */

  n = list_mod;
  while (n)
    {
      free_3Dobj(n->model);
      tmp = n;
      n = n->next;
      free(tmp);
    }
  list_mod = 0;
}


void free_textures()
{
  t_texture tmp,n;

  n = tex_index;
  while(n)
    {
      free((*n).name);
      glDeleteTextures(1,&(*n).id);
      tmp = n;
      n = n->next;
      free(tmp);
    }
  tex_index = 0;
}
