#include <stdlib.h>
#include <stdio.h>
#include "lib_bbs_behavior.h"
#include "loader.h"
#include "camera.h"
#include "divers.h"
#include "map.h"
#include "discrimination.h"


extern t_agent liste_agent;
extern t_objet liste_objet;
t_agent temp_agent = NULL;
t_objet temp_objet = NULL;

extern int nb_sti, nb_obj;
extern t_map carte;
extern behavior * stimuli;

static inline float my_rand(float n)
{
  return(1 + (n * (rand() / (RAND_MAX + 1.0))));
}

t_agent creer_agent(char * model,int x, int y, float rx,float ry,float rz,float ratio,int type,int add)
{
  t_agent obj = malloc(sizeof (struct s_agent));
  int i;

  (*obj).x = obj->xgoto = (x + 0.5) * TAILLE_CASE;
  (*obj).y = obj->ygoto = 0;
  (*obj).z = obj->zgoto = (y + 0.5) * TAILLE_CASE;
  (*obj).rx = rx;
  (*obj).ry = ry;
  (*obj).rz = rz;
  (*obj).case_x = x;
  (*obj).case_y = y;
  (*obj).ratio = ratio;
  (*obj).step = 0;
  (*obj).model = create_3Dobj(model);
  (*obj).next = temp_agent;
  (*obj).speed = 0;
  (*obj).caddie = NULL;
  (*obj).is_caddie = !add;
  obj->stimuli = calloc(nb_sti,(sizeof (struct stimuli_agent)));

  for(i=0;i<nb_sti;i++)
    {
      obj->stimuli[i].poids = stimuli[i].task.weight;
      obj->stimuli[i].seuil = stimuli[i].task.threshold;
      obj->stimuli[i].force = stimuli[i].task.force;
    }

  obj->ac_tps = 0.0;
  obj->tps = my_rand(CYCLE);
  obj->age = 1;
  obj->na = 1;
  obj->naa = 0;
  obj->en_cours = 0;
  obj->tache = -1;
  obj->type = type;
  /*obj->ag = type;*/ /* doublon?*//*on dirait*/

  temp_agent = obj;
  if (add)
    add_agent(obj->case_x,obj->case_y,obj);
  
  return(obj);
}

void free_agent (t_agent agent)
{
  if (agent->stimuli)
    free(agent->stimuli);
  free(agent);
}

void free_agents()
{
  t_agent tmp,n;


  if (temp_agent)
    n = temp_agent;
  else
    n = liste_agent;
  while (n)
    {
      tmp = n;
      n = n->next; 
      free_agent(tmp);
    }
  temp_agent = liste_agent = 0;
}


t_objet creer_objet(char * model,int x, int y, float rx,float ry,float rz,float ratio,int type,int add)
{
  t_objet obj;

  obj = malloc(sizeof (struct s_objet));

  /*if (type < -1 || type >= nb_obj)
    {
      type = 0;
      free(type);
      free("zizi");
      free(5);
    }
  */

  (*obj).x = (x + 0.5) * TAILLE_CASE;
  (*obj).y = 0;
  (*obj).z = (y + 0.5) * TAILLE_CASE;
  (*obj).rx = rx;
  (*obj).ry = ry;
  (*obj).rz = rz;
  (*obj).case_x = x;
  (*obj).case_y = y;
  (*obj).ratio = ratio;
  (*obj).step = 0;
  (*obj).model = create_3Dobj(model);
  (*obj).speed = 0;
  (*obj).next = temp_objet;
  (*obj).is_on_map = add;
  (*obj).type = type;
  (*obj).valeur = 300.;
  (*obj).is_caddie = !add;
  obj->tps = my_rand(CYCLE);
  
  temp_objet = obj;

  if (add)
    add_objet(obj->case_x,obj->case_y,obj);

  return(obj);
}

void free_objet(t_objet objet)
{
  free(objet);
}

void free_objets()
{
  t_objet tmp, n;

  if (temp_objet)
    n = temp_objet;
  else
    n = liste_objet;

  while (n)
    {
      tmp = n;
      n = n->next; 
      free_objet(tmp);
    }
  temp_objet = liste_objet = 0;
}

t_objet load_objet(char * model,float ratio)
{
  return(creer_objet(model,0,0,0.0,0.0,0.0,ratio,-1,0));
}
/* alexandre... t'es un putain de connard */
/* alexandre... t'es un putain de connard */
/* alexandre... t'es un putain de connard */
/* alexandre... t'es un putain de connard */
