#include "lib_bbs_behavior.h"
#include "loader.h"
#include <stdlib.h>
#include <stdio.h>
#include "camera.h"
#include "map.h"
#include "divers.h"
#include <math.h>

extern t_map carte;
extern behavior *stimuli;
extern operator *operators;
extern primitive *primitives;
extern argument *arguments;

extern int nb_sti;

extern unsigned long tps_tot;
extern float tps_tot_ac;
extern int cy_depr;
extern float cycle;

/*ceci est une constante magique, c'est pas super,
mais on va dire que c'est temporaire :)           */
#define DECREMENT_TACHE 0.5


/*****************TO DO***********************
 **nothing to do, enlever codage en dur; OK **
 **enchainement des actions;             OK **
 **interruption                    à tester **
 **seuils;                       presque OK **
 **renforcement                  presque OK **
 **stimuli internes              en cours   **
 *********************************************/ 


/*appeller quand on change d'activité*/
void end_tache(t_agent agent)
{
  float tmp;

  if(agent)
    {
      if(agent->tache > agent->type)
	{
	  tmp =  fabs(0.5-(agent->na/agent->naa));
	  agent->stimuli[agent->tache].poids +=stimuli[agent->tache].task.weight_change * tmp;
	}
      if(agent->na <= 0)
	agent->stimuli[agent->tache].seuil += agent->naa;
      else
	agent->stimuli[agent->tache].seuil += agent->naa/agent->na;
      agent->na=0;
      agent->tache = agent->type;
      agent->en_cours=0;
      agent->naa=0;
    }
}





void discriminer(t_agent agent)
{
  t_signal tmp, stimulus=NULL; /*stimulus est le plus fort stimulus rencontré*/
  brick tmp2=NULL;
  int i;
  float nai,nai_max; /*nai_max est le niveau d'activité du stimulus le plus fort rencontré*/
  /*lecture des stimuli detectables sur la case*/
  if(agent)
    {
      /*on teste les stimuli internes a l'agent*/
      for (i=0;i<nb_sti;i++)
	{
	  if ((stimuli[i].interne) && (stimuli[i].ag == agent->type))
	    {
	      tmp2=stimuli[i].task.brick;
	      if (tmp2)
		{
		  operators[tmp2->op].op(agent,primitives[tmp2->p].p,tmp2->arg);
		  agent->en_cours--;
		}
	      /*bien mettre les seuils à jours dans les primitives*/
	    }
	  /*test sur les stimuli captés*/
	}
      tmp=carte.tab[agent->case_x][agent->case_y].signaux;
      while(tmp)
	{
	  if ((stimuli[tmp->type].ag == agent->type) && (tmp->type != agent->tache) &&
	      (!cy_depr ||
	      (double) tmp->date + ((double) cy_depr * (double) cycle) > (double) tps_tot))
	    {
	      nai= (tmp->force) * (agent->stimuli[tmp->type].poids);
	      if ((nai >= agent->stimuli[tmp->type].seuil) && (nai > agent->na))
		{
		  if (!stimulus)
		    {
		      stimulus=malloc(sizeof(struct s_signal));
		      stimulus->type=tmp->type;
		      stimulus->force=tmp->force;
		      nai_max=nai;
		    }
		  else 
		    {
		      if(nai>nai_max)
			{
			  if(agent->na > 0)
			    agent->stimuli[stimulus->type].seuil -= (nai/agent->na);
			  else
			    agent->stimuli[stimulus->type].seuil -= nai;
		                                                                 	  if(agent->stimuli[stimulus->type].seuil < 0)
			    agent->stimuli[stimulus->type].seuil = 0;
			  nai_max=nai;
			  stimulus->type = tmp->type;
			  stimulus->force = tmp->force;
			}
		      else
			{
			  if(agent->na > 0)
			    agent->stimuli[tmp->type].seuil-=(nai/agent->na);
			  else
			    agent->stimuli[tmp->type].seuil -= nai;
			  if(agent->stimuli[tmp->type].seuil < 0)
			    agent->stimuli[tmp->type].seuil = 0;
			}
		    }
		}
	      else
		{
		  if(agent->na > 0)
		    agent->stimuli[tmp->type].seuil-=(nai/agent->na);
		  else
		    agent->stimuli[tmp->type].seuil -= nai;
		  if(agent->stimuli[tmp->type].seuil < 0)
		    agent->stimuli[tmp->type].seuil = 0;
		}
	    }
	  tmp=tmp->next;
	}
  
      /*on va se debarasser de l'ancienne tache si on en a une nouvelle*/
      if (stimulus)
	{   
	  if(agent->tache > -1)
	    {
	      tmp2=stimuli[agent->tache].task.interruption;
	      if(tmp2)                                             
		operators[tmp2->op].op(agent,primitives[tmp2->p].p,tmp2->arg);
	    }
	  end_tache(agent);
	  agent->tache=stimulus->type;
	  agent->naa=nai_max;
	  agent->na=nai_max;
	  if(stimuli[stimulus->type].interne)
	    agent->en_cours = 2;
	  else
	    agent->en_cours = 1;
	  free(stimulus);
	  stimulus = NULL;
	}
      else
	{
	  if ((agent->na>0)&& !(stimuli[agent->tache].interne))
	    agent->na -= DECREMENT_TACHE * agent->naa;
	}
      /*   printf("na:%f\n",agent->na);
	   printf("tache:%i\n",agent->tache);*/
    }
}




                                                      

int action_agent(t_agent agent)
{
  brick tmp;
  int i;

  if(agent&&(agent->en_cours == -2)&&(agent->tache>=0))
    {/* REMY : ai-je bien fait ?
      tmp=stimuli[agent->tache].task.interruption;
      operators[tmp->op].op(agent,primitives[tmp->p].p,tmp->arg);
      return 2;
     */
      tmp=stimuli[agent->tache].task.interruption;
      if (tmp)
	operators[tmp->op].op(agent,primitives[tmp->p].p,tmp->arg);
      return 2;
    }
  if(agent&&(agent->en_cours)&&(agent->tache>=0))
    {
      tmp=stimuli[agent->tache].task.brick;
      i = agent->en_cours;
      while(tmp&&(i>1))
	{
	  tmp=tmp->next;
	  i--;
	}
      if(tmp)
	{
	  operators[tmp->op].op(agent,primitives[tmp->p].p,tmp->arg);
	  return 1;
	}
      else
	{
	  /*if(!stimuli[agent->tache].interne)
	    {
	      caca("la derniere action de la chaine n'était pas terminale");
	    }
	  else
	    {
	     caca("test pour les stimuli internes");
	     }*/
	  agent->en_cours = 0;
	  agent->tache = 0;
	  agent->naa = 0;
	  agent->na = 0;
	}
    }
    else
    {
      tmp=stimuli[agent->type].task.brick;
      if(tmp)
	operators[tmp->op].op(agent,primitives[tmp->p].p,tmp->arg);
    }
  return 0;
}

/* alexandre... t'es un putain de connard */
/* alexandre... t'es un putain de connard */
/* alexandre... t'es un putain de connard */
/* alexandre... t'es un putain de connard */
