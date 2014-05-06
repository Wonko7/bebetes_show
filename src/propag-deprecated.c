/*
** les vieux propags, au cas ou... 
**
** /!\ WARNING : ne pas inclure se fichier,
**     copiez les fcts que vous voulez
**     faire revivre dans propag.c
**
**                         GayLord
*/



#define MARQUE -47

void propagate_stimulus(int x, int y, int stim, float puiss, float decrement)
{
  t_signal list, prec;
  float tmp;

  if((x>=0)&&(x<carte.x_max)&&(y>=0)&&(y<carte.y_max))
    {
      if(cases[carte.tab[x][y].type].accessible)
	{
	  list=carte.tab[x][y].signaux;
	  prec=NULL;
	  while(list&&(list->type != stim))
	    {
	      prec=list;
	      list=list->next;
	    }
	  if(!list)
	    {
	      list=malloc(sizeof(t_signal));
	      list->type=stim;
	      list->next=NULL;    
	      list->force = puiss-1;      
	      if(!prec)
		(carte.tab[x][y]).signaux=list;
	      else
		prec->next=list;
	    }
	   if(list->force < puiss)
	    {
	      list->force = puiss;
	      
	      tmp=puiss-decrement;
	      
	      if(tmp>0)
		{
		  if (in_map(x-1,y-1) && cases[carte.tab[x-1][y-1].type].accessible && ((in_map(x-1,y) && in_map(x,y-1) && (cases[carte.tab[x][y-1].type].accessible || cases[carte.tab[x-1][y].type].accessible))))
		    propagate_stimulus(x-1,y-1,stim,tmp,decrement);

		  propagate_stimulus(x-1,y,stim,tmp,decrement);

		  if (in_map(x-1,y+1) && cases[carte.tab[x-1][y+1].type].accessible &&((in_map(x-1,y) && in_map(x,y+1) && (cases[carte.tab[x][y+1].type].accessible || cases[carte.tab[x-1][y].type].accessible))))
		    propagate_stimulus(x-1,y+1,stim,tmp,decrement);

		  propagate_stimulus(x,y+1,stim,tmp,decrement);
		  propagate_stimulus(x,y-1,stim,tmp,decrement);

		  if (in_map(x+1,y-1) && cases[carte.tab[x+1][y-1].type].accessible && ((in_map(x+1,y) && in_map(x,y-1) && (cases[carte.tab[x][y-1].type].accessible || cases[carte.tab[x+1][y].type].accessible))))
		    propagate_stimulus(x+1,y-1,stim,tmp,decrement);

		  propagate_stimulus(x+1,y,stim,tmp,decrement);

		  if (in_map(x+1,y+1) && cases[carte.tab[x+1][y+1].type].accessible && ((in_map(x+1,y) && in_map(x,y+1) && (cases[carte.tab[x][y+1].type].accessible || cases[carte.tab[x+1][y].type].accessible))))
		    propagate_stimulus(x+1,y+1,stim,tmp,decrement);
		}
	    }
	}
    }
}



void depropagate_stimulus(int x, int y, int stim, float puiss, float decrement)
{
  t_signal list, prec;
  int deprop;
  float tmp;

  if((x>=0)&&(x<carte.x_max)&&(y>=0)&&(y<carte.y_max))
    {
      deprop = 0;
      if(cases[carte.tab[x][y].type].accessible)
	{
	  list=carte.tab[x][y].signaux;
	  prec=NULL;
	  while(list&&(list->type != stim))
	    {
	      prec=list;
	      list=list->next;
	    }
	  if(list&&(list->force<=puiss))
	    {  
	      deprop=1;
	      if(!prec)
		(carte.tab[x][y]).signaux=list->next;
	      else
		prec->next=list->next;
	      free(list);
	      list=NULL;
	    }
	   if(deprop)
	    { 
	      tmp=puiss-decrement;
	      
	      if(tmp>0)
		{
		  if (in_map(x-1,y-1) && cases[carte.tab[x-1][y-1].type].accessible && ((in_map(x-1,y) && in_map(x,y-1) && (cases[carte.tab[x][y-1].type].accessible || cases[carte.tab[x-1][y].type].accessible))))
		    depropagate_stimulus(x-1,y-1,stim,tmp,decrement);

		  depropagate_stimulus(x-1,y,stim,tmp,decrement);

		  if (in_map(x-1,y+1) && cases[carte.tab[x-1][y+1].type].accessible && ((in_map(x-1,y) && in_map(x,y+1) && (cases[carte.tab[x][y+1].type].accessible || cases[carte.tab[x-1][y].type].accessible))))
		    depropagate_stimulus(x-1,y+1,stim,tmp,decrement);

		  depropagate_stimulus(x,y+1,stim,tmp,decrement);
		  depropagate_stimulus(x,y-1,stim,tmp,decrement);

		  if (in_map(x+1,y-1) && cases[carte.tab[x+1][y-1].type].accessible && ((in_map(x+1,y) && in_map(x,y-1) && (cases[carte.tab[x][y-1].type].accessible || cases[carte.tab[x+1][y].type].accessible))))
		    depropagate_stimulus(x+1,y-1,stim,tmp,decrement);

		  depropagate_stimulus(x+1,y,stim,tmp,decrement);

		  if (in_map(x+1,y+1) && cases[carte.tab[x+1][y+1].type].accessible &&((in_map(x+1,y) && in_map(x,y+1) && (cases[carte.tab[x][y+1].type].accessible || cases[carte.tab[x+1][y].type].accessible))))
		    depropagate_stimulus(x+1,y+1,stim,tmp,decrement);
		}
	    }
	}
    }
}


/*mega_propagate propage tous les stimuli qui sont dans le tableau stim*/

void mega_propagate(int x, int y, int nb, int * stim, float puiss, float decrement)
{
  t_signal list, prec;
  int cpt,i;
  int stim_tmp[nb];
  float prop=0.0;
  
  caca("mega proop");
  if((x>=0)&&(x<carte.x_max)&&(y>=0)&&(y<carte.y_max))
    {
      for(i=0;i<nb;i++)
	stim_tmp[i]=stim[i];
      if(cases[carte.tab[x][y].type].accessible)
	{
	  list=carte.tab[x][y].signaux;
	  prec=NULL;
	  cpt = nb;
	  while(cpt && list)
	    {
	      for(i=0;i<nb;i++)
		{
		  if(stim_tmp[i]!=MARQUE && stim_tmp[i]==list->type)
		    {
		      if(list->force < puiss)
			{
			  list->force = puiss;
			  prop = puiss - decrement;
			}
		      stim_tmp[i]=MARQUE;
		      cpt--;
		      break;
		    }
		}
	      prec=list;
	      list=list->next;
	    }
	  i = 0;
	  while(cpt&&(i<nb))
	    {
	      if(stim_tmp[i]!=MARQUE)
		{
		  list=malloc(sizeof(t_signal));
		  list->type=stim[i];
		  list->force=puiss;
		  list->next=NULL;   
		  if(!prec)
		    (carte.tab[x][y]).signaux=list;
		  else
		    prec->next=list;
		  prec = list;
		  cpt--;
		  prop = puiss - decrement;
		}
	      i++;
	    }
	  if(prop>0.0)
	    {
	      if (in_map(x-1,y-1) && cases[carte.tab[x-1][y-1].type].accessible && ((in_map(x-1,y) && in_map(x,y-1) && (cases[carte.tab[x][y-1].type].accessible || cases[carte.tab[x-1][y].type].accessible))))
		mega_propagate(x-1,y-1,nb,stim,prop,decrement);
	      
	      mega_propagate(x-1,y,nb,stim,prop,decrement);
	      
	      if (in_map(x-1,y+1) && cases[carte.tab[x-1][y+1].type].accessible &&((in_map(x-1,y) && in_map(x,y+1) && (cases[carte.tab[x][y+1].type].accessible || cases[carte.tab[x-1][y].type].accessible))))
		mega_propagate(x-1,y+1,nb,stim,prop,decrement);
	      
	      mega_propagate(x,y+1,nb,stim,prop,decrement);
	      
	      mega_propagate(x,y-1,nb,stim,prop,decrement);
	      
	      if (in_map(x+1,y-1) && cases[carte.tab[x+1][y-1].type].accessible && ((in_map(x+1,y) && in_map(x,y-1) && (cases[carte.tab[x][y-1].type].accessible || cases[carte.tab[x+1][y].type].accessible))))
		mega_propagate(x+1,y-1,nb,stim,prop,decrement);
	      
	      mega_propagate(x+1,y,nb,stim,prop,decrement);
	      
	      if (in_map(x+1,y+1) && cases[carte.tab[x+1][y+1].type].accessible && ((in_map(x+1,y) && in_map(x,y+1) && (cases[carte.tab[x][y+1].type].accessible || cases[carte.tab[x+1][y].type].accessible))))
		mega_propagate(x+1,y+1,nb,stim,prop,decrement);
	    }
	}
    }
}



/*
void mega_depropagate(int x, int y, int nb, int * stim, float puiss, float decrement)
{
  t_signal list,prec;
  int j,tmp=0, cpt = nb;
  float deprop;
  
  if((x>=0)&&(x<carte.x_max)&&(y>=0)&&(y<carte.y_max))
    {
      if(cases[carte.tab[x][y].type].accessible)
	{
	  list=carte.tab[x][y].signaux;
	  prec=NULL;
	  while(cpt && list)
	    {
	      for(j=0;j<nb;j++)
		{
		  if((stim[j]==list->type)&&(list->force <= puiss))
		    {
		      if(prec)
			{
			  prec->next=list->next;
			  free(list);
			  list = prec->next;
			}
		      else
			{
			  carte.tab[x][y].signaux=list->next;
			  free(list);
			  list=carte.tab[x][y].signaux;
			}
		      cpt--;
		      tmp=1;
		      break;
		    }
		  else
		    if(j==nb-1)
		      {
			prec = list;
			list=list->next;
		      }
		}
	    }
	  if(tmp)
	    {
	      deprop=puiss-decrement;
	      
	      if (in_map(x-1,y-1) && cases[carte.tab[x-1][y-1].type].accessible && ((in_map(x-1,y) && in_map(x,y-1) && (cases[carte.tab[x][y-1].type].accessible || cases[carte.tab[x-1][y].type].accessible))))
		mega_depropagate(x-1,y-1,nb,stim,deprop,decrement);
	      
	      mega_depropagate(x-1,y,nb,stim,deprop,decrement);
	      
	      if (in_map(x-1,y+1) && cases[carte.tab[x-1][y+1].type].accessible && ((in_map(x-1,y) && in_map(x,y+1) && (cases[carte.tab[x][y+1].type].accessible || cases[carte.tab[x-1][y].type].accessible))))
		mega_depropagate(x-1,y+1,nb,stim,deprop,decrement);
	      
	      mega_depropagate(x,y+1,nb,stim,deprop,decrement);
	      mega_depropagate(x,y-1,nb,stim,deprop,decrement);
	      
	      if (in_map(x+1,y-1) && cases[carte.tab[x+1][y-1].type].accessible && ((in_map(x+1,y) && in_map(x,y-1) && (cases[carte.tab[x][y-1].type].accessible || cases[carte.tab[x+1][y].type].accessible))))
		mega_depropagate(x+1,y-1,nb,stim,deprop,decrement);
	      
	      mega_depropagate(x+1,y,nb,stim,deprop,decrement);
	      
	      if (in_map(x+1,y+1) && cases[carte.tab[x+1][y+1].type].accessible &&((in_map(x+1,y) && in_map(x,y+1) && (cases[carte.tab[x][y+1].type].accessible || cases[carte.tab[x+1][y].type].accessible))))
		mega_depropagate(x+1,y+1,nb,stim,deprop,decrement);
	    }
	}
    }
}  
*/


/*pour toi Alexandre*/
/*
void propagation_ejaculatoire(int x, int y, int nb, int * stim, float puiss, float decrement)
{
  t_signal list, prec;
  int cpt,i;
  float prop=0.0;
 
  if((x>=0)&&(x<carte.x_max)&&(y>=0)&&(y<carte.y_max))
    {
      if(cases[carte.tab[x][y].type].accessible)
	{
	  list=carte.tab[x][y].signaux;
	  prec=NULL;
	  cpt = nb;
	  while(cpt && list)
	    {
	      for(i=0;i<nb;i++)
		{
		  if(stim[i]==list->type)
		    {
		      if(list->force < puiss)
			{
			  list->force = puiss;
			  prop = puiss - decrement;
			}
		      stim[i]=MARQUE;
		      cpt--;
		    }
		}
	      prec=list;
	      list=list->next;
	    }
	  i = 0;
	  while(cpt&&(i<nb))
	    {
	      if(stim[i]!=MARQUE)
		{
		  list=malloc(sizeof(t_signal));
		  list->type=stim[i];
		  list->force=puiss;
		  list->next=NULL;   
		  if(!prec)
		    (carte.tab[x][y]).signaux=list;
		  else
		    prec->next=list;
		  prec = list;
		  cpt--;
		  prop = puiss - decrement;
		}
	      i++;
	    }
	  printf("prop:%f\n",prop);
	  if(prop>0.0)
	    {
	      if (in_map(x-1,y-1) && cases[carte.tab[x-1][y-1].type].accessible && ((in_map(x-1,y) && in_map(x,y-1) && (cases[carte.tab[x][y-1].type].accessible || cases[carte.tab[x-1][y].type].accessible))))
		mega_propagate(x-1,y-1,nb,stim,prop,decrement);

	      mega_propagate(x-1,y,nb,stim,prop,decrement);

		  if (in_map(x-1,y+1) && cases[carte.tab[x-1][y+1].type].accessible &&((in_map(x-1,y) && in_map(x,y+1) && (cases[carte.tab[x][y+1].type].accessible || cases[carte.tab[x-1][y].type].accessible))))
		    mega_propagate(x-1,y+1,nb,stim,prop,decrement);

		  mega_propagate(x,y+1,nb,stim,prop,decrement);
		  mega_propagate(x,y-1,nb,stim,prop,decrement);

		  if (in_map(x+1,y-1) && cases[carte.tab[x+1][y-1].type].accessible && ((in_map(x+1,y) && in_map(x,y-1) && (cases[carte.tab[x][y-1].type].accessible || cases[carte.tab[x+1][y].type].accessible))))
		    mega_propagate(x+1,y-1,nb,stim,prop,decrement);

		  mega_propagate(x+1,y,nb,stim,prop,decrement);

		  if (in_map(x+1,y+1) && cases[carte.tab[x+1][y+1].type].accessible && ((in_map(x+1,y) && in_map(x,y+1) && (cases[carte.tab[x][y+1].type].accessible || cases[carte.tab[x+1][y].type].accessible))))
		    mega_propagate(x+1,y+1,nb,stim,prop,decrement);
	    }
	}
    }
}
*/



void mega_depropagate(int x, int y, int nb, int * stim, float puiss, float decrement)
{
  t_signal list,prec;
  int j,tmp=0, cpt = nb;
  float deprop;

  if((x>=0)&&(x<carte.x_max)&&(y>=0)&&(y<carte.y_max))
    {
      if(cases[carte.tab[x][y].type].accessible)
	{
	  list=carte.tab[x][y].signaux;
	  prec=NULL;
	  while(cpt && list)
	    {
	      for(j=1;j<=nb;j++)
		{
		  if((stim[j]==list->type)&&(list->force <= puiss))
		    {
		      if(prec)
			{
			  prec->next=list->next;
			  free(list);
			  list = prec->next;
			}
		      else
			{
			  carte.tab[x][y].signaux=list->next;
			  free(list);
			  list=carte.tab[x][y].signaux;
			}
		      cpt--;
		      tmp=1;
		      break;
		    }
		  else
		    if(j==nb)
		      {
			prec = list;
			list=list->next;
		      }
		}
	    }
	  if(tmp)
	    {
	      deprop=puiss-decrement;
	      
	      if (in_map(x-1,y-1) && cases[carte.tab[x-1][y-1].type].accessible && ((in_map(x-1,y) && in_map(x,y-1) && (cases[carte.tab[x][y-1].type].accessible || cases[carte.tab[x-1][y].type].accessible))))
		mega_depropagate(x-1,y-1,nb,stim,deprop,decrement);
	      
	      mega_depropagate(x-1,y,nb,stim,deprop,decrement);
	      
	      if (in_map(x-1,y+1) && cases[carte.tab[x-1][y+1].type].accessible && ((in_map(x-1,y) && in_map(x,y+1) && (cases[carte.tab[x][y+1].type].accessible || cases[carte.tab[x-1][y].type].accessible))))
		mega_depropagate(x-1,y+1,nb,stim,deprop,decrement);
	      
	      mega_depropagate(x,y+1,nb,stim,deprop,decrement);
	      mega_depropagate(x,y-1,nb,stim,deprop,decrement);
	      
	      if (in_map(x+1,y-1) && cases[carte.tab[x+1][y-1].type].accessible && ((in_map(x+1,y) && in_map(x,y-1) && (cases[carte.tab[x][y-1].type].accessible || cases[carte.tab[x+1][y].type].accessible))))
		mega_depropagate(x+1,y-1,nb,stim,deprop,decrement);
	      
	      mega_depropagate(x+1,y,nb,stim,deprop,decrement);
	      
	      if (in_map(x+1,y+1) && cases[carte.tab[x+1][y+1].type].accessible &&((in_map(x+1,y) && in_map(x,y+1) && (cases[carte.tab[x][y+1].type].accessible || cases[carte.tab[x+1][y].type].accessible))))
		mega_depropagate(x+1,y+1,nb,stim,deprop,decrement);
	    }
	}
    }
}  


void deprop_int(t_agent agent)
{
  int nb,i=1,j=1;
  float max=0.0;
  t_signal tmp;
  int *tab;

  caca("mega deproproop");  

  tab=stim_kkt[agent->type];
  nb = tab[0];
  tmp= carte.tab[agent->case_x][agent->case_y].signaux;
  while(tmp && (i<=nb))
    {
      for(j=1;j<=nb;j++)
	{
	  if(tmp->type == tab[j])
	    {
	      if(tmp->force > max)
		max = tmp->force;
	      i++;
	    }
	}
      tmp=tmp->next;
    } 
  mega_depropagate(agent->case_x,agent->case_y,nb,tab,max,1.0);
}


