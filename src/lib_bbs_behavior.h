/*
**   ¿ OLA QUE TAL ? 
**
**                     GayLord.
**
**   PORQUE TE VAS !
**
**                     Goulagman.
**
*/

typedef struct
{
  char * name_ia;
  int (*ia)();
  int i;
} iaction;

enum
  {
    IA_CASE=0,
    IA_AGENT,
    IA_OBJ
  };

/* moi aussi je t'aime!

                Goulag */
typedef struct n_brick * brick;
struct n_brick
{
  char * name_p;
  char * name_op;
  char * icone_p;
  char * icone_op;
  int  p;
  int  op;
  int  arg;
  int type_op;
  brick next;
};

typedef struct 
{
  float threshold;
  float weight;
  float increment;
  float weight_change;
  float force;
  brick brick;
  brick interruption;
} task;

typedef struct 
{
  char * name_s;
  int ag;
  int interne;
  /* émission 
     interne, 
     reponse 
     externe. 
     je 
     suis 
     un 
     oeuf
     ...   */
  /*       */
  /*   |   */
  /*   |   */
  /*   V   */
  int em_int;
  task task;
} behavior;

typedef struct 
{
  char * name_ag;
  int ag;
  float ratio;
  char * nom_mod;
  void * mod;
} agent;

typedef struct 
{
  char * name_obj;
  int obj;
  float ratio;
  char * nom_mod;
  void * mod;
  float crea_taux;
} objet;

typedef struct 
{
  int (*op)();
  char * name_op;
  int type_op;
  char * icone_op;
} operator;

typedef struct 
{
  int (*p)();
  char * name_p;
  char * icone_p;
  int * bboard;
} primitive;

typedef struct 
{
  int id;                 /* num du truc dans le tab  */
  int type;               /* c'est le type:ag ou sti  */
  char * name;            /* pour gtk                 */
} argument;

/* ATTENTION, POUR DES RAISON DE CACAPROUTAGE, DEPLACE DANS LOADER.H
typedef struct
{
  int * texture;
  char * name;
  void * model;
  int accessible;
} ccase;
*/

void remove_task2 (task t);

void init_icones (char * chemin);
void init_textures (char * chemin);

typedef struct {
  int x,y;
} x_et_y;

x_et_y find_strong_stimulus(x_et_y *tabu, int max, int stim);



