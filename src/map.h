/*#include "loader.h"*/ 
#define TAILLE_CASE 10.0
#define FOVY 45.0                /*    pour savoir a quoi ca correspond   */

typedef struct s_signal * t_signal;
struct s_signal
{
  int type;
  float force;
  unsigned long date;
  t_signal  next;
};

typedef struct
{
  t_agent * agents;
  t_signal  signaux;
  t_objet * objets;
  int nbr_ag,nbr_obj;
  int etat;
  int type;
} t_case;

/*le type des cases de la map*/
/*typedef struct s_case t_case;*/


/*la map est un tableau statique de t_case*/
typedef struct
{
  int x_max, y_max;
  t_case ** tab;
} t_map;

typedef struct {
  float x,y;
} t_2Dplot;

void free_map();
t_map create_map(int x, int y);

void draw_scene ();

void add_agent (int x, int y, t_agent agent);
void del_agent (int x,int y, t_agent agent);
void add_objet (int x, int y, t_objet objet);
void del_objet (int x,int y, t_objet objet);
int suivre(picked * clicked);
/* alexandre... t'es un putain de connard */

typedef struct n_draw * t_draw;
struct n_draw
{
  t_agent ag;
  t_draw  n;
};
