#include <GL/gl.h>  /* pour le type GLuint */
#define _LOADER_H

#define HAUTEUR_BB 2.0
#define HAUTEUR_OBJ 0.5
#define LARGEUR_BB 1.0

extern int nb_sti;
typedef struct
{
  int flags;
  float x;
  float y;
  float z;
  float u;
  float v;
  int bone_index;
} t_vertex;

typedef struct
{
  float x;
  float y;
  float z;
} t_normal;

typedef struct
{
  int flags;
  int vertex_1;
  int vertex_2;
  int vertex_3;
  t_normal normal;
  int s_group;
} t_triangle;

typedef struct
{
  char * name;
  int flags;
  int material;
  t_vertex * vertex;
  t_triangle * triangle;
  int vertices;
  int triangles;
} t_mesh;

typedef struct
{
  t_mesh * mesh;
  int meshes;
  GLuint ** texture;
  int textures;
} t_frame;

typedef struct
{
  t_frame * frame;
  int frames;
  float max;
} t_3Dobj;

/* for texturizing! */
typedef struct s_texture * t_texture;

struct s_texture
{
  GLuint id;
  char * name;
  t_texture next;
};


typedef struct s_list_mod * t_list_mod;

struct s_list_mod
{
  char * name;
  t_3Dobj * model;
  t_list_mod next;
};
  


void free_textures();
void free_list_mod ();




/* objets! */

typedef struct s_objet * t_objet;

struct s_objet
{
  float x;
  float y;
  float z;
  float rx;     /* rotation axe x            */
  float ry;     /* rotation axe y            */
  float rz;     /* rotation axw z            */
  int case_x,case_y,case_index; /* pour la gestion des cases (sisi) */ 
  float ratio;
  float valeur; /* selon l'objet. ce peut etre la valeure nutritive par ex... */
  double tps;
  t_3Dobj * model;
  int is_caddie;
  int step;
  int speed;
  int type;
  int is_on_map;
  t_objet next;
};

struct stimuli_agent
{
  float poids;
  float seuil;
  float force;
};

typedef struct s_agent * t_agent;
 
struct s_agent
{
  float x;
  float y;
  float z;
  float xgoto;
  float ygoto;
  float zgoto;
  float rx;                  /* rotation axe x            */
  float ry;                  /* rotation axe y            */
  float rz;                  /* rotation axe z            */
  int case_x,case_y,case_index; /* pour la gestion des cases (sisi) */ 
  float ratio;               /* ratio du modele 3d*/
  int   step;                /* numéro de frame actuelle dans l'animation de la fourmi*/
  float ac_tps;              /* acu pour changer de frame */
  float speed;               /* cases ogl par seconde ?   */
  t_3Dobj * model;           /* pointeur vers le modele 3d */
  void *caddie;             /* objets portes par la fourmi */
  int is_caddie_ag;
  int is_caddie;
  struct stimuli_agent *stimuli; /*tableau représentant les poids et les seuils des stimuli pr cet agent*/
  float naa;                   /*niveau auquel a ete activee la tache actuelle*/
  float na;                    /*niveau de la tache actuelle*/ 
  int age;                   /* si c'est negatif, c'est mourru */
  int en_cours;            /*tache en cours*/
  int tache;
  double tps;                /*temps d'execution de la tache en cours*/
  int type;                  /*type de l'agent*/
  t_agent next;                /*agent suivant*/
};


t_3Dobj * create_3Dobj(char * model);
t_agent creer_agent(char * model,int x, int y, float rx,float ry,float rz,float ratio,int type, int add);
void free_agent(t_agent agent);
void free_agents();

t_objet creer_objet(char * model,int x, int y, float rx,float ry,float rz,float ratio,int type,int add);
void free_objets();
void free_objet(t_objet objet);
t_objet load_objet(char * model,float ratio);

/* ogl.c :  */
void draw_agent (t_agent caca,int pic);
void draw_objet (t_objet obj,int pic);
void dust_to_dust (t_agent amen);

/* on le laisse la en attendant */
void draw_ground ();
char * uconcat(char * str1,char * str2,char * str3);
GLuint * Load_Texture (char * name);
void reload_textures ();

typedef struct
{
  int * texture;
  char * name;
  char * model;
  t_objet objet;
  int accessible;
} ccase;
/* alexandre... t'es un putain de connard */
