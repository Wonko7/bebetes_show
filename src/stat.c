#include <math.h>
#include <glib.h>
#include <stdio.h>
#include <stdlib.h>
#include "loader.h"
#include "camera.h"
#include "map.h"
#include "divers.h"
#include <float.h>
#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <errno.h>
#include <ctype.h>
#include "gtk+extra/gtkplot.h"
#include "gtk+extra/gtkplotdata.h"
#include "gtk+extra/gtkplotbar.h"
#include "gtk+extra/gtkplotcanvas.h"
#include "gtk+extra/gtkplotprint.h"
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "stat.h"
#include "lib_bbs_behavior.h"
#include "gtk+extra/gtkplotcanvasplot.h"

#define BUFF 30
#define NB_COLORS 26

GtkPlotData *dataset;

int npoints=20;
int baise_moi_fort = 0;
double nan_constant = 0.0;// / 0;
double inf_constant = 3.0;//(1.0 / 0.0);


extern char* save_dir;
extern char* save;
extern int nb_ag;
extern graphe * stats;
extern int nb_stats;
extern t_agent liste_agent;
extern t_objet liste_objet;
extern t_map carte;

int mycycle = 42;

extern void *(*stat_ag)(t_agent agent);
extern void *(*stat_obj)(t_objet objet);
extern void *(*stat_map)(t_case * kase);

typedef struct s_histo {
  double bmin;
  double bmax;
  double bomin;
  double bomax;
  int date;
  struct s_histo * prev;
  struct s_histo * next;
  my_plot * courbe;
} my_histo;


my_plot * big_courbes = NULL;
my_histo * big_histos = NULL;

GtkWidget *scroll_w = NULL, *combo_stat, *scroll1, *scroll2, *combo_color, *change_mode;
GtkWidget *canvas, *bbox, *label,* sx, * sy;
GtkWidget *bplot,* gtkcycle, *moyenne, *ecart, *variance, *foufoune, *fbox;
GtkPlotData * dd = NULL;


double * gabscisse = NULL;

int width=400,height=400;
int twidth=400,theight=400;
int mode = 0;
double bmin = 0., bmax = 10.; 
double bomin = 0., bomax = 10.; 
int precision = 1;
int increment = 0;

/*Pour savoir jusqu'ou le données ont été sauvegardées dans le fichier final*/
int flag_save=0;

/*Indique quelle est la prochaine case du tableau stat_nb_agents il faut remplir*/
int current_stat=0;

/* Indique a quel cycle correspond le debut des stats */
int start_cycle = 0;

/* Pour le big_stat */
int longueur = 0;


/* en chantier, peut etre un truc genial :D */

typedef double (*fptr)(double,double);

typedef struct s_calc {
  fptr op;
  struct s_calc * fg;
  struct s_calc * fd;
} a_calc;
 
a_calc * operation;
double inconnue;
int pos = 0;
int lexp = 5;
const char * op;

/* parseur de fonctions */

double gadd (double x, double y)
{
  return(x+y);
}

double gsub (double x, double y)
{
  return(x-y);
}

double gmul (double x, double y)
{
  return(x*y);
}

double gdiv (double x, double y)
{
  return(x/y);
}

double gln (double x, double y UNUSED)
{
  return(log(x));
}

double gexp (double x, double y UNUSED)
{
  return(exp(x));
}

double gcos (double x, double y UNUSED)
{
  return(cos(x));
}

double gsin (double x, double y UNUSED)
{
  return(sin(x));
}

double gtan (double x, double y UNUSED)
{
  return(tan(x));
}

double gpow (double x, double y)
{
  return(pow(x,y));
}

double applier (a_calc * node)
{
  if (!node)
    return(0);
  if (node->op)
    return((double)(node->op)(applier(node->fg),applier(node->fd)));
  else
    if (!node->fd)
      return(*((double *)node->fg));
    else
      return(inconnue);
}


gdouble function (GtkPlot *plot UNUSED, GtkPlotData *data UNUSED, gdouble x, gboolean *err UNUSED)
{
 gdouble y;
 *err = FALSE;
 inconnue = x;
 y = applier(operation);

 if (y>bomax) y = bomax;	else if (y<bomin) y = bomin;

 return y;
}

fptr findop (char c)
{
  if (c == '+') return (&gadd);
  if (c == '-') return (&gsub);
  if (c == '*') return (&gmul);
  if (c == '/') return (&gdiv);
  if (c == '^') return (&gpow);
  return (&gadd);
}

a_calc * read_expr ()
{
  a_calc * res;
  
  res = malloc(sizeof(a_calc));
  
  if (isdigit(op[pos]))
    {
      res->op = NULL;
      res->fd = NULL;
      res->fg = malloc(sizeof(double));
      sscanf(&op[pos],"%lg",(double *)res->fg);
      while (isdigit(op[pos])||(op[pos]==','))
	pos++;
      return(res);
    }
  if (op[pos]=='x')
    {
      res->op = NULL;
      res->fd = (a_calc *) 1;
      res->fg = NULL;
      pos++;
      return(res);
    }
  if (op[pos]=='e')
    {
      res->op = (&gexp);
      pos += 4;
      res->fg = read_expr();
      res->fd = NULL;
      pos++;
      return (res);
    }

  if (op[pos]=='c')
    {
      res->op = (&gcos);
      pos += 4;
      res->fg = read_expr();
      res->fd = NULL;
      pos++;
      return (res);
    }


  if (op[pos]=='s')
    {
      res->op = (&gsin);
      pos += 4;
      res->fg = read_expr();
      res->fd = NULL;
      pos++;
      return (res);
    }


  if (op[pos]=='t')
    {
      res->op = (&gtan);
      pos += 4;
      res->fg = read_expr();
      res->fd = NULL;
      pos++;
      return (res);
    }

  if (op[pos]=='l')
    {
      res->op = (&gln);
      pos += 3;
      res->fg = read_expr();
      res->fd = NULL;
      pos++;
      return (res);
    }
  if (op[pos]=='(')
    {
      pos++;
      res->fg = read_expr();
      while (op[pos] == ' ')
	pos++;
      res->op = findop(op[pos]);
      pos++;
      res->fd = read_expr();
      pos++;
      return(res);
    }
  if (op[pos] == ' ')
    {
      pos++;
      return(read_expr());
    }
  pos = 0;
  return(NULL);
}

typedef struct {
  char * name;
  GdkColor color;
} my_color;

my_color *colors=NULL;
int nb_colors=NB_COLORS;
GdkColor grey, steel;

int ins_data;

typedef struct s_avl {
  double cle;
  double nbr;
  struct s_avl * fg;
  struct s_avl * fd;
  int deseq;
} t_avl;

t_avl * rd (t_avl * noeud)
{
  t_avl * tmp = noeud->fg->fd;

  noeud->fg->fd = noeud;
  noeud = noeud->fg;
  noeud->fd->fg = tmp;
  noeud->deseq = 0;
  noeud->fd->deseq = 1;
  return(noeud);
}

t_avl * rg (t_avl * noeud)
{
  t_avl * tmp = noeud->fd->fg;

  noeud->fd->fg = noeud;
  noeud = noeud->fd;
  noeud->fg->fd = tmp;
  noeud->deseq = 0;
  noeud->fg->deseq = -1;
  return(noeud);
}
  
t_avl * rgd (t_avl * noeud)
{
  t_avl * tmp = noeud->fg->fd;

  noeud->fg->fd = tmp->fg;
  tmp->fg = noeud->fg;
  noeud->fg = tmp->fd;
  noeud->deseq = (- tmp->deseq - 1) / 2;
  tmp->fd = noeud;
  tmp->fg->deseq = (1 - tmp->deseq) / 2;
  tmp->deseq = 0;
  return(tmp);
} 

t_avl * rdg (t_avl * noeud)
{
  t_avl * tmp = noeud->fd->fg;

  noeud->fd->fg = tmp->fd;
  tmp->fd = noeud->fd;
  noeud->fd = tmp->fg;
  noeud->deseq = (1 - tmp->deseq) / 2;
  tmp->fg = noeud;
  tmp->fd->deseq = (- tmp->deseq - 1) / 2;
  tmp->deseq = 0;
  return(tmp);
} 

int add_avl (t_avl * pere, double n)
{
  int sens = (n <= pere->cle),add;
  t_avl * tmp = (sens)?pere->fg:pere->fd;
  

  if (!tmp)
    {
      tmp = malloc(sizeof(t_avl));
      tmp->cle = n;
      tmp->fg = NULL;
      tmp->fd = NULL;
      tmp->nbr = 1;
      tmp->deseq = 0;
      ins_data = 1;
      if (sens)
	{
	  pere->fg = tmp;
	  pere->deseq++;
	  return(pere->fd == 0);
	}
      else
	{
	  pere->fd = tmp;
	  pere->deseq--;
	  return(pere->fg == 0);
	}
    }
  else
    if (tmp->cle == n)
      tmp->nbr += 1;
    else
      {
	if (sens)
	  {
	    add = add_avl(tmp,n);
	    pere->deseq += add;
	    if (tmp->deseq == 2)
	      {
		if (tmp->fg->deseq == 1)
		  pere->fg = rd(tmp);
		else
		  pere->fg = rgd(tmp);
		return(0);
	      }
	    else
	      return(add);
	  }
	else
	  {
	    add = add_avl(tmp,n);
	    pere->deseq -= add;
	    if (tmp->deseq == -2)
	      {
		if (tmp->fd->deseq == -1)
		  pere->fd = rg(tmp);
		else
		  pere->fd = rdg(tmp);
		return(0);
	      }
	    else
	      return(add);
	  }
      }
  return(0);
}

void avl_to_array ()
{
  my_plot * link;
  t_avl * tmp, **pile;
  int i,tete,pos=0;
  double omin = 0.,omax = 10.;

  for (i=0;i<nb_stats;i++)
    {
      if (stats[i].type != COURBE)
	{
	  link = &stats[i].valeurs;
	  tete = 0;
	  pos = 0;
	  omin = 0.;
	  omax = 10.;
 
	  tmp = ((t_avl *)(link->abscisse))->fg;
	  free((t_avl *)(link->abscisse));

	  if (!tmp)
	    {
	      link->color = 0;
	      
	      link->abscisse = NULL;
	      link->ordonnee = NULL;
	      return;
	    }
	  pile = malloc(link->color * sizeof(t_avl *));
	  link->color = 0;

	  omin = omax = tmp->nbr;
	      
	  pile[0] = tmp;
	  link->abscisse = malloc(link->size * sizeof(double));
	  link->ordonnee = malloc(link->size * sizeof(double));
	  
	  while(tete + 1)
	    {
	      tmp = pile[tete];
	      if (tmp->nbr > 0)
		{
		  if (tmp->fg)
		    {
		      tete++;
		      pile[tete] = tmp->fg;
		    }
		  tmp->nbr *= -1;
		}
	      else
		{
		  if (tmp->fd)
		    pile[tete] = tmp->fd;
		  else
		    tete--;
		  
		  link->abscisse[pos] = tmp->cle;
		  link->ordonnee[pos] = -tmp->nbr;
		  
		  if (-tmp->nbr > omax)
		    omax = -tmp->nbr;
		  else if (-tmp->nbr < omin)
		    omin = -tmp->nbr;
		  
		  pos++;
		  free(tmp);
		}
	      link->amin = link->abscisse[0];
	      link->amax = link->abscisse[link->size - 1];
	      link->omin = omin;
	      link->omax = omax;
	    }
	  free(pile);
	}
    }
}

void init_avl ()
{
  int i;
  t_avl * avl;

  for (i=0;i<nb_stats;i++)
    if (stats[i].type != COURBE)
      {
	(stats[i].valeurs.abscisse) = malloc(sizeof(t_avl));
	avl = (t_avl *)(stats[i].valeurs.abscisse);
	avl->fg = NULL;
	avl->fd = NULL;
	avl->cle = inf_constant;
	stats[i].valeurs.color = 0;
	stats[i].valeurs.size = 0;
      }
}

void init_stats_var ()
{
  int i;

  for (i=0;i<nb_stats;i++)
    {
      stats[i].valeurs.abscisse = NULL;
      stats[i].valeurs.dataset = NULL;
      if (stats[i].type == COURBE)
	stats[i].valeurs.ordonnee = calloc(npoints, sizeof(double));
      else
	stats[i].valeurs.ordonnee = NULL;
      stats[i].valeurs.size = 0;
    }
}

void spush (int n, double value)
{
  int i;
  my_plot * link = &stats[n].valeurs;
  ((t_avl *)(link->abscisse))->deseq = 0;
  ins_data = 0;
  i = add_avl((t_avl *)link->abscisse,value);
  link->color += abs(i);
  link->size += ins_data;
}
      
void init_colors () 
{
  int i=0;
  colors = malloc(NB_COLORS * sizeof(my_color));

  colors[0].name = "red";
  colors[1].name = "blue";
  colors[2].name = "black";
  colors[3].name = "brown";
  colors[4].name = "chocolate";
  colors[5].name = "coral";
  colors[6].name = "cyan";
  colors[7].name = "deep pink";
  colors[8].name = "deep sky blue";
  colors[9].name = "firebrick";
  colors[10].name = "blue violet";
  colors[11].name = "gray";
  colors[12].name = "khaki";
  colors[13].name = "magenta";
  colors[14].name = "maroon";
  colors[15].name = "orange";
  colors[16].name = "orchid";
  colors[17].name = "peru";
  colors[18].name = "pink";
  colors[19].name = "purple";
  colors[20].name = "royal blue";
  colors[21].name = "salmon";
  colors[22].name = "gold";
  colors[23].name = "sienna";
  colors[24].name = "turquoise";
  colors[25].name = "violet";

  while (i < nb_colors)
    {
      gdk_color_parse(colors[i].name, &colors[i].color);
      if (gdk_colormap_alloc_color(gdk_colormap_get_system(), &colors[i].color,FALSE,TRUE))
	i++;
      else
	{
	  colors[i] = colors[i+1];
	  nb_colors--;
	}
    }

  gdk_color_parse("grey90", &grey);
  gdk_color_alloc(gdk_colormap_get_system(), &grey); 
  gdk_color_parse("steel blue", &steel);
  gdk_color_alloc(gdk_colormap_get_system(), &steel); 

  colors = realloc(colors,nb_colors * sizeof(my_color));
  if (!nb_colors)
    colors = NULL;
}

void free_colors ()
{
  int i;
  
  if (colors)
    {
      for (i=0;i<nb_colors;i++)
	gdk_colormap_free_colors(gdk_colormap_get_system(), &colors[i].color,1);
      free(colors);
      colors = NULL;
      nb_colors = 0;
    }

  gdk_colormap_free_colors(gdk_colormap_get_system(), &grey,1);
  gdk_colormap_free_colors(gdk_colormap_get_system(), &steel,1);
}
/* pour l'instant, exit! UNDEPRECATED */
void free_big ()
{
  int i;
  my_histo * kk = big_histos,* tmp;
  my_plot * link;

  dd = NULL;

  if (gabscisse)
    {
      free(gabscisse);
      gabscisse = NULL;
    }

  if (kk)
    while (kk->next)
      {
	link = kk->courbe;
	for (i=0;i<nb_stats;i++)
	  {
	    if (link[i].color)
	      {
		if (mode)
		  free(link[i].abscisse);
		free(link[i].ordonnee);
	      }
	  }
	tmp = kk;
	kk = kk->next;
	free(tmp);
      }
  free(kk);
  big_histos = NULL;
  
}

/* a appeler ^^ */
void free_histo ()
{
  int i;
  my_plot * link;

  for (i=0;i<nb_stats;i++)
    {
      link = &(stats[i].valeurs);
      if (link->dataset)
	gtk_widget_unref(GTK_WIDGET(link->dataset));
      if (stats[i].type == HISTOGRAMME)
	{
	  
	  if (link->abscisse)
	    {
	      free(link->abscisse);
	      link->abscisse = NULL;
	    }
	  if (link->ordonnee)
	    {
	      free(link->ordonnee);
	      link->ordonnee = NULL;
	    }
	}
    }
}

void free_stats ()
{
  int i;
  
  for (i=0;i<nb_stats;i++)
    {
      if (stats[i].type != COURBE)
	if (stats[i].valeurs.abscisse)
	  {
	    free(stats[i].valeurs.abscisse);
	    stats[i].valeurs.abscisse = NULL;
	  }
      if (stats[i].valeurs.ordonnee)
	{
	  free(stats[i].valeurs.ordonnee);
	  stats[i].valeurs.ordonnee = NULL;
	}
    }
  
  free_big ();
  free(stats);
  stats = NULL;
}

void add_courbe_stat (int graph, double value)
{
  if (value == nan_constant)
    value = 0.;
  else
    if (value == inf_constant)
      value = DBL_MAX;
    else
      if (value == -inf_constant)
	value = -DBL_MAX;
  stats[graph].valeurs.ordonnee[current_stat] = value;
}


void add_histo_stat (int graph, double value)
{
  if (value == nan_constant)
    value = 0.;
  else
    if (value == inf_constant)
      value = DBL_MAX;
    else
      if (value == -inf_constant)
	value = -DBL_MAX;
  spush(graph,value);
}

/*Charge en mémoire les npoints/2 dernières valeurs du fichier de sauvegarde des stats sur le nb d'agents A appeller au chargement*/
void charger_stats()
{
  char * nom_fich;
  FILE * f_stat;
  struct stat buf;
  int i,j;

  if (!save)
    return;

  start_locale();

  for (j=0;j<nb_stats;j++)
    {
      if (stats[j].type == COURBE)
	{
	  nom_fich= mega_cct(6,save_dir,"/",save,"/statistiques/",stats[j].fichier,".tmp");
	  f_stat=fopen(nom_fich,"w");
	  fclose(f_stat);
	  free(nom_fich);
	 
 	  nom_fich= mega_cct(6,save_dir,"/",save,"/statistiques/",stats[j].fichier,".bbs");
	  f_stat=fopen(nom_fich,"r");
	  if (!f_stat)
	    {
	      start_cycle = 0;
	      current_stat = 0;
	      continue;
	    }
	  
	  if (j == 0)
	    {
	      stat(nom_fich,&buf);
	      i = buf.st_size / 14;
	      current_stat = (i < npoints/2)?i:npoints/2;
	      flag_save = current_stat;
	      start_cycle = i - current_stat;
	    }
	  fseek(f_stat,-current_stat*14,SEEK_END);
	  
	  for (i=0;i<current_stat;i++)
	    fscanf(f_stat,"%lg\n", &(stats[j].valeurs.ordonnee[i]));
	  
	  fclose(f_stat);
	  free(nom_fich);
	}
    }
  end_locale();
}

/*Copie le fichier temporaire nb d'agents et les données courantes dans le final et clear le temporaire A appeler lors de la sauvegarde*/
void save_stats()
{
  char *fich_src, *fich_dst;
  FILE *src,*dst;
  int tmp,i,j,size;
  t_agent tmpag = liste_agent;
  my_plot * link;

  if (!save)
    return;

  start_locale();

  init_avl ();
  
  while (tmpag)
    {
      stat_ag(tmpag);
      tmpag = tmpag->next;
    }
  
  avl_to_array ();


  for (j=0;j<nb_stats;j++)  
    {
      fich_dst= mega_cct(6,save_dir,"/",save,"/statistiques/",stats[j].fichier,".bbs");
      
      if (stats[j].type == COURBE)
	{
	  fich_src= mega_cct(6,save_dir,"/",save,"/statistiques/",stats[j].fichier,".tmp");

	  src=fopen(fich_src,"r");
	  dst=fopen(fich_dst,"a");
	  if (src)
	    {
	      while((tmp=fgetc(src))!= EOF)
		fputc(tmp,dst);
	      fclose(src);
	      src=fopen(fich_src,"w");
	      fclose(src);  
	    }
	  
	  for (i=flag_save;i<current_stat;i++)
	    fprintf(dst,"%+.6e\n",stats[j].valeurs.ordonnee[i]);
	  
	  free(fich_src);
	  free(fich_dst);
	  fclose(dst);
	}
      else
	{
	  dst=fopen(fich_dst,"a");
	  link = &stats[j].valeurs;
	  size = link->size;

	  fprintf(dst,"%i;%i;%g;%g;%g;%g\n",mycycle,size,link->amin,link->amax,link->omin,link->omax);

	  for (i=0;i<size;i++)
	    fprintf(dst,"%+.6e;%+.6e\n",link->abscisse[i],link->ordonnee[i]);
	  
	  free(fich_dst);
	  fclose(dst);

	  /* penser a free-er tout le bazar */
	  free(link->abscisse);
	  free(link->ordonnee);

	}
    }
  end_locale();
  flag_save = current_stat;
}

/*Quand on a fini de mettre a jour les stats, on avance dans le tableau et on ecrit dans le fichier temporaire si necessaire*/
void end_stat()
{
  int i,j,errsv;
  FILE * file_stat;
  char * file_path,* tmp;

  current_stat++;
  if (current_stat == npoints)
    {
      start_locale();

      for (j=0;j<nb_stats;j++)
	{
	  if ((save) && (stats[j].type == COURBE))
	    {
	      file_path = mega_cct(6,save_dir,"/",save,"/statistiques/",stats[j].fichier,".tmp");
	      file_stat = fopen(file_path,"a");
	      errsv = errno;
	      if (!file_stat)
		{
		  tmp = mega_cct(3,"Le fichier de stats ",file_path," ne peut etre cree");
		  caca(tmp);
		  printf("Code d'erreur: %i\n",errsv);
		  free(file_path);
		  free(tmp);
		  continue ;
		}
	      free(file_path);
	      for (i = flag_save;i < npoints; i++)
		fprintf(file_stat,"%+.6e\n",stats[j].valeurs.ordonnee[i]);
	      fclose(file_stat);
	      memcpy(stats[j].valeurs.ordonnee,stats[j].valeurs.ordonnee + npoints/2,(npoints/2)*sizeof(double));
	    }
	}
      flag_save= npoints/2;
      current_stat= npoints / 2;
      start_cycle += npoints/2;
      end_locale();
    }
}

void resized ()
{
  int i,nx,ny;
  double * tx,* ty,tioup;
  my_plot * link;

  if (mode)
    {
      double tmin = gtk_range_get_value(GTK_RANGE(scroll1));
      double tmax = gtk_range_get_value(GTK_RANGE(scroll2));
      
      
      gtk_plot_axis_set_ticks(gtk_plot_get_axis(GTK_PLOT(bplot), GTK_PLOT_AXIS_LEFT),MAX(floor((double)(bomax - bomin)/height*30),1.),1);      
	    
	    /*      gtk_plot_axis_set_ticks(gtk_plot_get_axis(link->dataset->plot, GTK_PLOT_AXIS_RIGHT),MAX(floor((double)(link->omax - link->omin)/height*30),1.),1);*/
	    
	    gtk_plot_set_range(GTK_PLOT(bplot),tmin ,tmax,bomin,bomax + ((bomin==bomax)?1:0));
	    
	    gtk_plot_axis_set_ticks(gtk_plot_get_axis(GTK_PLOT(bplot), GTK_PLOT_AXIS_BOTTOM),MAX(floor((double)(tmax - tmin)/width*50),1.),1);
    }
  else
    for (i=0;i<nb_stats;i++)
      {
	link = &(stats[i].valeurs);
	tx = gtk_plot_data_get_x(link->dataset,&nx);
	ty = gtk_plot_data_get_y(link->dataset,&ny);

	if (!tx || !ty)
		return;
	
	gtk_plot_axis_set_ticks(gtk_plot_get_axis(link->dataset->plot, GTK_PLOT_AXIS_LEFT),MAX(floor((double)(link->omax - link->omin)/height*30),1.),1);      
	
	/*      gtk_plot_axis_set_ticks(gtk_plot_get_axis(link->dataset->plot, GTK_PLOT_AXIS_RIGHT),MAX(floor((double)(link->omax - link->omin)/height*30),1.),1);*/
	
	tioup = (nx)?((tx[0] == tx[nx-1])?tx[0]+1:tx[nx-1]):10.;
	
	gtk_plot_set_range(link->dataset->plot,(nx)?tx[0]:0. ,tioup,link->omin,link->omax + ((link->omin == link->omax)?1:0));
	
	gtk_plot_axis_set_ticks(gtk_plot_get_axis(link->dataset->plot, GTK_PLOT_AXIS_BOTTOM),MAX(floor((double)((nx)?tioup-tx[0]:10.)/width*50),1.),1);
	
      }
}



GtkPlotData * create_dataset (int n)
{
  GtkPlotData * dataset;
  GdkColor * line, * point;
  if (mode)
    line = point = &(colors[big_courbes[n].color - 1].color);
  else
    {
      point = &(colors[0].color);
      line = &(colors[1].color);
    }

  dataset = GTK_PLOT_DATA(gtk_plot_data_new());
  /* on essaie de construire un graph */
  if (stats[n].type-1)
    {
      gtk_plot_data_set_symbol(dataset,
			       GTK_PLOT_SYMBOL_SQUARE,
			       GTK_PLOT_SYMBOL_OPAQUE,
			       7,1.5, line, point);      
      gtk_plot_data_set_line_attributes(dataset,GTK_PLOT_LINE_NONE, 0, 0, 1, line);
    }
  else
    {
      gtk_plot_data_set_symbol(dataset,
			       GTK_PLOT_SYMBOL_CIRCLE,
			       GTK_PLOT_SYMBOL_OPAQUE,
			       5,1, line, point);      
      gtk_plot_data_set_line_attributes(dataset,GTK_PLOT_LINE_SOLID, 0, 0, 1, line);
    }
  
  gtk_plot_data_set_points(dataset,NULL,NULL, NULL, NULL,0);
  gtk_widget_ref(GTK_WIDGET(dataset));
  
  return(dataset);
}

void  rescale (GtkRange *range UNUSED, gpointer  user_data UNUSED)
{
  GtkWidget *view = gtk_bin_get_child(GTK_BIN(scroll_w));
  int diff,amin,amax;
  
  if (view && baise_moi_fort) 
    {
      int i;

      double tmin = gtk_range_get_value(GTK_RANGE(scroll1));
      double tmax = gtk_range_get_value(GTK_RANGE(scroll2));
      if (mode)
	{
	  gtk_range_set_range(GTK_RANGE(scroll1),bmin, (tmax + tmin) / 2 - 0.5);
	  gtk_range_set_range(GTK_RANGE(scroll2),(tmax + tmin) / 2 + 0.5,bmax);
	      
	  if (mode == 1)
	    {
	      for (i=0;i<nb_stats;i++)
		if (big_courbes[i].color)
		  {
		    amin = big_courbes[i].amin;
		    amax = big_courbes[i].amax;
		    diff = amax - amin;
		    
		    gtk_plot_data_set_points(big_courbes[i].dataset,&((big_courbes[i].abscisse)[(int)(ceil(tmin) - amin)]), &((big_courbes[i].ordonnee)[(int)(ceil(tmin) - amin)]), NULL, NULL,MAX((int)(floor(tmax) - ceil(tmin)),0.));
		  }
	    }
	  else
	    {
	      int min,max;
	      for (i=0;i<nb_stats;i++)
		if (big_courbes[i].color)
		  {
		    min = 0;
		    max = big_courbes[i].size - 1;
		    
		    while (big_courbes[i].abscisse[min] < tmin)
		      min++;
		    
		    while (big_courbes[i].abscisse[max] > tmax)
		      max--;
		    
		    gtk_plot_data_set_points(big_courbes[i].dataset,&((big_courbes[i].abscisse)[min]), &((big_courbes[i].ordonnee)[min]), NULL, NULL,MAX(max-min+1,0.));
		  }
	    }
	  resized();
	}
      else
	{
	  gtk_range_set_range(GTK_RANGE(scroll1),0., (tmax + tmin) / 2 - 0.5);
	  gtk_range_set_range(GTK_RANGE(scroll2),(tmax + tmin) / 2 + 0.5,100.);
	  for (i=0;i<nb_stats;i++)
	    {
	      amin = ceil(stats[i].valeurs.size * tmin / 100);
	      amax = floor(stats[i].valeurs.size * tmax / 100);
	      diff = amax - amin;
	      if (!diff)
		diff = 1;
	      
	      gtk_plot_data_set_points(stats[i].valeurs.dataset,&((stats[i].valeurs.abscisse)[amin]), &((stats[i].valeurs.ordonnee)[amin]), NULL, NULL,diff);
	    }
	  resized();
	}
      gtk_plot_canvas_paint(GTK_PLOT_CANVAS(canvas));
      gtk_plot_canvas_refresh(GTK_PLOT_CANVAS(canvas));  
    }
}


double * load_graphe_file (int graph_index)
{
  double *valeurs = 0;
  char *nom_fich = 0;
  FILE *bbs = 0, *tmp = 0;
  struct stat buf;
  char * name = stats[graph_index].fichier;
  double omin,omax;
  int i, t_bbs = 0, t_tmp = 0;
  int longueur;
  
  omin = omax = stats[graph_index].valeurs.ordonnee[0];

  nom_fich = mega_cct(6,save_dir,"/",save,"/statistiques/",name,".bbs");
  bbs = fopen(nom_fich,"r");
  if (bbs)
    {
      stat(nom_fich,&buf);
      t_bbs += buf.st_size / 14;
    }
  
  free(nom_fich);
  nom_fich = mega_cct(6,save_dir,"/",save,"/statistiques/",name,".tmp");
  tmp = fopen(nom_fich,"r");
  if (tmp)
    {
      stat(nom_fich,&buf);
      t_tmp += buf.st_size / 14;
    }
  
  big_courbes[graph_index].amax = big_courbes[graph_index].size = longueur = (t_bbs + t_tmp + current_stat - flag_save);
  
  valeurs = malloc(longueur * sizeof(double));
  
  start_locale();
  
  for (i=0;i<t_bbs;i++)
    {
      fscanf(bbs,"%lg\n", &valeurs[i]);
      if (valeurs[i] < omin)
	omin = valeurs[i];
      else
	if (valeurs[i] > omax)
	  omax = valeurs[i];
    }
  
  for (i=t_bbs;i<(t_bbs + t_tmp);i++)
    {
      fscanf(tmp,"%lg\n", &valeurs[i]);
      if (valeurs[i] < omin)
	omin = valeurs[i];
      else
	if (valeurs[i] > omax)
	  omax = valeurs[i];
    }
  
  end_locale();
  
  if (bbs)
    fclose(bbs);
  if (tmp)
    fclose(tmp);
  
  for (i=t_bbs+t_tmp;i<longueur;i++)
    {
      valeurs[i] = stats[graph_index].valeurs.ordonnee[i - t_bbs - t_tmp + flag_save];
      if (valeurs[i] < omin)
	omin = valeurs[i];
      else
	if (valeurs[i] > omax)
	  omax = valeurs[i];
    }
  
  big_courbes[graph_index].omin = omin;
  big_courbes[graph_index].omax = omax;
  big_courbes[graph_index].amin = 0.;

  
  return(valeurs);
}

void set_cycle(int cc)
{
  int l = 0;
  int ff = cc;
  while (ff)
    {
      l++;
      ff /= 10;
    }
  char tt[l+1];
  snprintf(tt,l+1,"%i",cc);
  tt[l] = '\0';
  gtk_label_set_text(GTK_LABEL(gtkcycle),tt);
}


void load_histo_file (int graph_index, int color, int allocate)
{
  my_plot * link;
  my_histo * tmp = big_histos,* prev = NULL;
  double tmin,tmax,tomin,tomax;
  char * nom_fich= mega_cct(6,save_dir,"/",save,"/statistiques/",stats[graph_index].fichier,".bbs");      
  FILE * fich = fopen(nom_fich,"r");
  free(nom_fich);
  int date,nbr,i;


  if (fich)
    while (fscanf(fich,"%i;%i;%lg;%lg;%lg;%lg\n",&date,&nbr,&tmin,&tmax,&tomin,&tomax) != EOF)
      {
	if (allocate)
	  {
	    tmp = malloc(sizeof(my_histo));
	    tmp->prev = prev; 
	    tmp->next = NULL;
	    if (prev)
	      prev->next = tmp;
	    else
	      big_histos = tmp;
	    link = tmp->courbe = malloc(nb_stats * sizeof(my_plot));
	    for (i=0;i<nb_stats;i++)
	      link[i].color = 0;
	    prev = tmp;
	  }
	else
	  link = tmp->courbe;

	tmp->date = date;
	link[graph_index].abscisse = malloc(nbr * sizeof(double));
	link[graph_index].ordonnee = malloc(nbr * sizeof(double));
	link[graph_index].size = nbr;

	if (!allocate)
	  {
	    link[graph_index].amin = tmin;
	    if (tmin < tmp->bmin)
	      tmp->bmin = tmin; 
	    link[graph_index].amax = tmax;
	    if (tmax > tmp->bmax)
	      tmp->bmax = tmax; 
	    link[graph_index].omin = tomin;
	    if (tomin < tmp->bomin)
	      tmp->bomin = tomin; 
	    link[graph_index].omax = tomax;
	    if (tomax > tmp->bomax)
	      tmp->bomax = tomax; 
	    tmp = tmp->next;
	  }
	else
	  {
	    tmp->bmin = link[graph_index].amin = tmin;
	    tmp->bmax = link[graph_index].amax = tmax;
	    tmp->bomin = link[graph_index].omin = tomin;
	    tmp->bomax = link[graph_index].omax = tomax;
	  }
	
	link[graph_index].color = color;
	
	for (i=0;i<nbr;i++)
	  fscanf(fich,"%lg;%lg\n",&(link[graph_index].abscisse[i]),&(link[graph_index].ordonnee[i]));
	
	big_courbes = link;
	  /*
	link[graph_index].dataset = create_dataset(graph_index);
	  */

	link[graph_index].dataset = stats[graph_index].valeurs.dataset;
	
      }
  if (allocate)
    {
      tmp = malloc(sizeof(my_histo));
      tmp->prev = prev; 
      tmp->next = NULL;
      if (prev)
	prev->next = tmp;
      else
	big_histos = tmp;
      link = tmp->courbe = malloc(nb_stats * sizeof(my_plot));
      for (i=0;i<nb_stats;i++)
	link[i].color = 0;
      
      bmin = tmp->bmin = link[graph_index].amin = stats[graph_index].valeurs.amin;
      bmax = tmp->bmax = link[graph_index].amax = stats[graph_index].valeurs.amax;
      bomin = tmp->bomin = link[graph_index].omin = stats[graph_index].valeurs.omin;
      bomax = tmp->bomax = link[graph_index].omax = stats[graph_index].valeurs.omax;
    }
  else
    {
      tmin = stats[graph_index].valeurs.amin;
      tmax = stats[graph_index].valeurs.amax;
      tomin = stats[graph_index].valeurs.omin;
      tomax = stats[graph_index].valeurs.omax;
      link = tmp->courbe;
      link[graph_index].amin = tmin;
      if (tmin < tmp->bmin)
	tmp->bmin = tmin; 
      link[graph_index].amax = tmax;
      if (tmax > tmp->bmax)
	tmp->bmax = tmax; 
      link[graph_index].omin = tomin;
      if (tomin < tmp->bomin)
	tmp->bomin = tomin; 
      link[graph_index].omax = tomax;
      if (tomax > tmp->bomax)
	tmp->bomax = tomax; 
    }


  link[graph_index].abscisse = stats[graph_index].valeurs.abscisse;
  link[graph_index].ordonnee = stats[graph_index].valeurs.ordonnee;
  
  baise_moi_fort = 0;
  gtk_range_set_range(GTK_RANGE(scroll1),bmin, (bmax + bmin) / 2 - 0.5);
  gtk_range_set_range(GTK_RANGE(scroll2),(bmax + bmin) / 2 + 0.5,bmax);
  gtk_range_set_value(GTK_RANGE(scroll1),bmin);
  gtk_range_set_value(GTK_RANGE(scroll2),bmax);
  baise_moi_fort = 1;

  tmp->date = mycycle;
  
  link[graph_index].color = color;
  link[graph_index].size = stats[graph_index].valeurs.size;
  
  big_courbes = link;
  link[graph_index].dataset = stats[graph_index].valeurs.dataset;

  set_cycle(mycycle);
}


void add_big_graphe (int ref)
{
  my_plot * link = &big_courbes[ref];
  /* a remplacer pour etre independant des structures
  GtkPlot * plot = (GTK_PLOT_CANVAS_PLOT(GTK_PLOT_CANVAS(canvas)->childs->data))->plot;
  oh yeah je l'ai fait c'est trop classe */

  int n = gtk_combo_box_get_active(GTK_COMBO_BOX(combo_color));

  if (stats[ref].type == HISTOGRAMME)
    {
      load_histo_file(ref, n, 0);
      gtk_plot_data_set_symbol(link->dataset,
			       GTK_PLOT_SYMBOL_SQUARE,
			       GTK_PLOT_SYMBOL_OPAQUE,
			       7,1.5, &colors[n-1].color,  &colors[n-1].color);
      big_courbes = big_histos->courbe;
      if (big_courbes[ref].amin < bmin)
	bmin = big_courbes[ref].amin;
      if (big_courbes[ref].amax > bmax)
	bmax = big_courbes[ref].amax;
      if (big_courbes[ref].omin < bomin)
	bomin = big_courbes[ref].omin;
      if (big_courbes[ref].omax > bomax)
	bomax = big_courbes[ref].omax;
    }
  else
    {
      
      link->ordonnee = load_graphe_file(ref);
      link->abscisse = gabscisse;
      
      link->color = n;  
      link->dataset = create_dataset (ref);
      
      
      if (link->amin < bmin)
	bmin = link->amin;
      
      if (link->amax > bmax)
	bmax = link->amax;
      
      if (link->omin < bomin)
	bomin = link->omin;
      
      if (link->omax > bomax)
	bomax = link->omax;
      
    }
  gtk_plot_add_data(GTK_PLOT(bplot), link->dataset);
  gtk_widget_show(GTK_WIDGET(link->dataset));
  
  gtk_plot_data_draw_points(link->dataset, 1);
  rescale(NULL,NULL);
  
}

GtkWidget * create_plot(int n)
{
  GtkWidget * plot;
  /* plot */
  plot = gtk_plot_new(NULL);
  
  gtk_plot_set_ticks(GTK_PLOT(plot), GTK_PLOT_AXIS_X, 1, 1);
  gtk_plot_set_ticks(GTK_PLOT(plot), GTK_PLOT_AXIS_Y, 1, 1);
  
  /* labels */
  /* labels style
     GTK_PLOT_LABEL_EXP  
     GTK_PLOT_LABEL_POW    
     GTK_PLOT_LABEL_FLOAT
  */
  /* axis */
  
  
  gtk_plot_axis_set_visible(gtk_plot_get_axis(GTK_PLOT(plot), GTK_PLOT_AXIS_LEFT), TRUE);
  
  gtk_plot_axis_set_ticks_length(gtk_plot_get_axis(GTK_PLOT(plot), GTK_PLOT_AXIS_LEFT), -7);
  gtk_plot_axis_set_ticks_length(gtk_plot_get_axis(GTK_PLOT(plot), GTK_PLOT_AXIS_RIGHT), -7);
  gtk_plot_axis_set_ticks_length(gtk_plot_get_axis(GTK_PLOT(plot), GTK_PLOT_AXIS_TOP), 0);
  
  gtk_plot_axis_show_labels(gtk_plot_get_axis(GTK_PLOT(plot), GTK_PLOT_AXIS_TOP),0);
  gtk_plot_axis_hide_title(gtk_plot_get_axis(GTK_PLOT(plot), GTK_PLOT_AXIS_TOP));
  gtk_plot_axis_hide_title(gtk_plot_get_axis(GTK_PLOT(plot), GTK_PLOT_AXIS_RIGHT));
  
  /*  if (stats[n].type == 1) */
  gtk_plot_axis_show_labels(gtk_plot_get_axis(GTK_PLOT(plot), GTK_PLOT_AXIS_RIGHT),0);
  
  /*
    gtk_plot_axis_move_title (gtk_plot_get_axis(GTK_PLOT(plot), GTK_PLOT_AXIS_LEFT),90,0.0001 * twidth + 0.02,0.41); 	
    gtk_plot_axis_move_title (gtk_plot_get_axis(GTK_PLOT(plot), GTK_PLOT_AXIS_BOTTOM),0,0.46,-0.0001 * theight + 0.85); 	
  */

  gtk_plot_axis_move_title (gtk_plot_get_axis(GTK_PLOT(plot), GTK_PLOT_AXIS_LEFT),90,0.0001 * (twidth / width * 400) + 0.02,0.41); 	
  gtk_plot_axis_move_title (gtk_plot_get_axis(GTK_PLOT(plot), GTK_PLOT_AXIS_BOTTOM),0,0.46,0.7 + ((float)height / (float)theight) * 0.1); 	
  
  gtk_plot_axis_set_labels_style(gtk_plot_get_axis(GTK_PLOT(plot), GTK_PLOT_AXIS_BOTTOM), GTK_PLOT_LABEL_FLOAT, 0);
  gtk_plot_axis_set_labels_style(gtk_plot_get_axis(GTK_PLOT(plot), GTK_PLOT_AXIS_LEFT), GTK_PLOT_LABEL_FLOAT, 0);
  gtk_plot_axis_set_labels_style(gtk_plot_get_axis(GTK_PLOT(plot), GTK_PLOT_AXIS_RIGHT), GTK_PLOT_LABEL_FLOAT, 0);
  
  gtk_plot_axis_set_title(gtk_plot_get_axis(GTK_PLOT(plot), GTK_PLOT_AXIS_LEFT), stats[n].ordonnee );
  gtk_plot_axis_set_title(gtk_plot_get_axis(GTK_PLOT(plot), GTK_PLOT_AXIS_BOTTOM), stats[n].abscisse);
  
  /* legends */
  gtk_plot_hide_legends(GTK_PLOT(plot));
  
  /* grid */
  gtk_plot_grids_set_visible(GTK_PLOT(plot),TRUE,FALSE,TRUE,FALSE);
  gtk_plot_major_vgrid_set_attributes(GTK_PLOT(plot),GTK_PLOT_LINE_SOLID,1.,&grey);
  gtk_plot_major_hgrid_set_attributes(GTK_PLOT(plot),GTK_PLOT_LINE_SOLID,1.,&grey);
  gtk_widget_show(plot);
  
  return(plot);
}

void maj_min_max (int n)
{
  double amin,amax,omin,omax;
  my_plot * link = &stats[n].valeurs;
  int numpoints = link->size;
  double * valeurs = link->ordonnee,* abscisse = link->abscisse;
  int i;

  omin=0.;
  omax=10.;

  if (numpoints)
    {
      omin=omax=link->ordonnee[0];
      for (i=0;i<numpoints;i++)
	{
	  if (omin > valeurs[i])
	    omin = valeurs[i];
	  if (omax < valeurs[i])
	    omax = valeurs[i];
	}
      if (omin == omax)
	omax++;
    }

  link->omin = omin;
  link->omax = omax;
  
  if (stats[n].type == 2)
    {
      amin = 0.;
      amax = 10.;
      
      if (numpoints)
	{
	  amin = amax = abscisse[0];
	  for (i=0;i<numpoints;i++)
	    {
	      if (amin > abscisse[i])
		amin = abscisse[i];
	      if (amax < abscisse[i])
		amax = abscisse[i];
	    }
	  if (amin == amax)
	    amax++;
	}
    }
  else
    {
      amin = start_cycle;
      amax = amin + current_stat;
    }
  
  link->amin = amin;
  link->amax = amax;
}
  

GtkPlotData * create_graph (int n)
{
  GtkPlotData * dataset;
  GtkWidget *plot;
  my_plot * link = &stats[n].valeurs;

  
  plot = create_plot(n);

  link->color =  gtk_combo_box_get_active(GTK_COMBO_BOX(combo_color)) - 1;
  
  dataset = create_dataset(n);

  gtk_plot_add_data(GTK_PLOT(plot), dataset);

  gtk_plot_data_draw_points(dataset, 1);
  
  
  gtk_widget_show(GTK_WIDGET(dataset));

  return(dataset);
}

void graphe_change (GtkComboBox *widget, gpointer user_data UNUSED)
{
  int n = gtk_combo_box_get_active(GTK_COMBO_BOX(widget));
  
  if (mode)
    {
      baise_moi_fort = 0;
      gtk_combo_box_set_active(GTK_COMBO_BOX(combo_color),big_courbes[n].color);
      baise_moi_fort = 1;
      
    }
  else
    if (stats[n].valeurs.size == 0)
      gtk_widget_set_sensitive(change_mode,0);
    else
      gtk_widget_set_sensitive(change_mode,1);
}



void color_change (GtkComboBox *widget, gpointer user_data UNUSED)
{
  int n = gtk_combo_box_get_active(GTK_COMBO_BOX(combo_stat));
  int c = gtk_combo_box_get_active(GTK_COMBO_BOX(widget)) - 1;
  my_plot * link = &big_courbes[n];

  if (stats[n].type != mode)
    return;

  if (baise_moi_fort)
    {
      if (c!= -1)
	{
	  if (link->color)
	    {
	      link->color = c+1;
	      if (stats[n].type-1)
		{
		  my_histo * tmp = big_histos;

		  while (tmp)
		    {
		      link = &(tmp->courbe[n]);
			gtk_plot_data_set_symbol(link->dataset,
						 GTK_PLOT_SYMBOL_SQUARE,
						 GTK_PLOT_SYMBOL_OPAQUE,
						 7,1.5, &colors[c].color, &colors[c].color);
		      gtk_plot_data_set_line_attributes(link->dataset,GTK_PLOT_LINE_NONE, 0, 0, 1, &colors[c].color );
		      tmp = tmp->next;
		    }
		}
	      else
		{
		  gtk_plot_data_set_symbol(link->dataset,
					   GTK_PLOT_SYMBOL_CIRCLE,
					   GTK_PLOT_SYMBOL_OPAQUE,
					   5, 1, &colors[c].color, &colors[c].color);
		  gtk_plot_data_set_line_attributes(link->dataset,
						    GTK_PLOT_LINE_SOLID,
						    0, 0, 1, &colors[c].color );
		}
	      gtk_plot_canvas_paint(GTK_PLOT_CANVAS(canvas));
	      gtk_plot_canvas_refresh(GTK_PLOT_CANVAS(canvas));  
	    }
	  else
	    {
	      add_big_graphe (n);
	    }
	}
      else
	{
	  if (link->color)
	    {
	      int i,cpt = 0;
	      for (i=0;i<nb_stats;i++)
		{
		  if ((big_courbes[i].color) && (i !=n))
		    {
		      if (!cpt)
			{
			  bomin = big_courbes[i].omin;
			  bomax = big_courbes[i].omax;
			  cpt ++;
			  continue;
			}
		      cpt++;
		      if (big_courbes[i].omin < bomin)
			bomin = big_courbes[i].omin;
		      if (big_courbes[i].omax > bomax)
			bomax = big_courbes[i].omax;
		    }
		}
	      if (cpt)
		{
		  if (mode -1)
		    {
		      double tmin,tmax,tomin,tomax;
		      my_histo * pp;
		      gtk_plot_remove_data(GTK_PLOT(bplot),link->dataset);
		      cpt = 0;
		      
		      pp = big_histos;
		      while (pp)
			{
			  for (i=0;i<nb_stats;i++)
			    {
			      if ((big_courbes[i].color) && (i !=n))
				{
				  if (!cpt)
				    {
				      tmin = big_courbes[i].amin;
				      tmax = big_courbes[i].amax;
				      tomin = big_courbes[i].omin;
				      tomax = big_courbes[i].omax;
				      cpt ++;
				      continue;
				    }
				  cpt++;
				  
				  if (big_courbes[i].amin < tmin)
				    tmin = big_courbes[i].amin;
				  if (big_courbes[i].amax > tmax)
				    tmax = big_courbes[i].amax;
				  if (big_courbes[i].omin < tomin)
				    tomin = big_courbes[i].omin;
				  if (big_courbes[i].omax > tomax)
				    tomax = big_courbes[i].omax;
				}
			    }
			  pp->bmin = tmin;
			  pp->bmax = tmax;
			  pp->bomin = tomin;
			  pp->bomax = tomax;
			  if (pp->courbe == big_courbes)
			    {
			      bmin = tmin;
			      bmax = tmax;
			      bomin = tomin;
			      bomax = tomax;
			    }
			  free((pp->courbe)[n].abscisse);
			  free((pp->courbe)[n].ordonnee);
			  pp = pp->next;
			}
		      rescale(NULL,NULL);
		    }


		  else
		    {
		      free(link->ordonnee);
		      link->color = 0;
		      gtk_plot_remove_data(link->dataset->plot,link->dataset);
		      rescale(NULL,NULL);
		    }
		}



	      else
		gtk_combo_box_set_active(GTK_COMBO_BOX(combo_color),link->color );		  
	    }
	}
    }
}


gboolean stat_click (GtkWidget      *widget UNUSED,
		     GdkEventButton *event,
		     gpointer        data UNUSED)
{
  int clic_x, clic_y,i;

  if (!mode)
    {
      
      clic_x=(int)(event->x/width);
      clic_y=(int)(event->y/height);
      
      i = clic_x + 2 * clic_y;
      gtk_combo_box_set_active(GTK_COMBO_BOX(combo_stat),i);
    }
  return(1);
}  
  
gint select_item(GtkWidget *widget UNUSED, GdkEvent *event UNUSED, GtkPlotCanvasChild *child, 
		 gpointer data UNUSED )
{
  gint n = 0;
  gdouble *x = NULL, *y = NULL;
  char putu[9];
  
  if(GTK_IS_PLOT_CANVAS_PLOT(child))
    if (GTK_PLOT_CANVAS_PLOT(child)->pos == GTK_PLOT_CANVAS_PLOT_IN_DATA)
      {
        x = gtk_plot_data_get_x(GTK_PLOT_CANVAS_PLOT(child)->data, &n); 
        y = gtk_plot_data_get_y(GTK_PLOT_CANVAS_PLOT(child)->data, &n); 
        n = GTK_PLOT_CANVAS_PLOT(child)->datapoint;
	snprintf(putu,8,"%g",x[n]);
	putu[8]='\0';
	gtk_label_set_text(GTK_LABEL(sx),putu);
	snprintf(putu,8,"%g",y[n]);
	putu[8]='\0';
	gtk_label_set_text(GTK_LABEL(sy),putu);
      }
  
  return TRUE;
}


void maj_stat  (guint page_num) 
{
  canvas = NULL;
  static int gogo = 0;

  if (page_num == 5)
    {
      if (nb_stats)
	{
	  gogo = 1;
	  GtkWidget *plot;
	  GtkPlotCanvasChild *child;
	  double larg=(nb_stats<2)?nb_stats:2, haut=(nb_stats + 1)/2;
	  double coef_x=1./larg , coef_y=1./haut ;
	  double x = 0.,y = 0.;
	  int i,j;
	  t_agent tmp = liste_agent;
	  t_objet piou = liste_objet;
	  t_case ** tab;
	  
	  init_avl ();
	  
	  while (tmp)
	    {
	      stat_ag(tmp);
	      tmp = tmp->next;
	    }
	  
	  
	  while (piou)
	    {
	      stat_obj(piou);
	      piou = piou->next;
	    }
	  
	  tab = carte.tab;
	  for (i=0;i<carte.x_max;i++)
	    for (j=0;j<carte.y_max;j++)
	      stat_map(&(tab[i][j]));
	  
	  
	  
	  avl_to_array ();
	  
	  twidth = width * larg;
	  theight = height * haut;
	  
	  baise_moi_fort = 0;
	  
	  gtk_range_set_range(GTK_RANGE(scroll1),0., 49.5);
	  gtk_range_set_range(GTK_RANGE(scroll2),50.5,100.);
	  gtk_range_set_value(GTK_RANGE(scroll1),0.);
	  gtk_range_set_value(GTK_RANGE(scroll2),100.);
	  
	  baise_moi_fort = 1;
	  
	  gabscisse =malloc(current_stat * sizeof(double)) ;
	  
	  canvas = gtk_plot_canvas_new(twidth,theight,1.0);
	  GTK_PLOT_CANVAS_SET_FLAGS(GTK_PLOT_CANVAS(canvas), GTK_PLOT_CANVAS_CAN_SELECT_ITEM);	 
	  
	  
	  for (i=0;i<current_stat;i++)
	    gabscisse[i] = (double)start_cycle + i;
	  
	  
	  for (i=0;i<nb_stats;i++)
	    {
	      if (stats[i].type == COURBE)
		{
		  stats[i].valeurs.size = current_stat;
		  stats[i].valeurs.abscisse = gabscisse;
		  maj_min_max(i);
		  stats[i].valeurs.dataset = create_graph(i);
		}
	      else
		{
		  stats[i].valeurs.dataset = create_graph(i);
		}
	      
	      plot = GTK_WIDGET(stats[i].valeurs.dataset->plot);
	      
	      gtk_plot_data_set_points(stats[i].valeurs.dataset, stats[i].valeurs.abscisse, stats[i].valeurs.ordonnee, NULL, NULL, stats[i].valeurs.size);	  
	      child = gtk_plot_canvas_plot_new(GTK_PLOT(plot));
	      
	      gtk_plot_canvas_put_child(GTK_PLOT_CANVAS(canvas), child,coef_x * (x + 0.14) , coef_y * (y + 12./height), coef_x * ( x + 1. - 32./width),coef_y * ( y + 0.88));
	      
	      GTK_PLOT_CANVAS_PLOT(child)->flags |= GTK_PLOT_CANVAS_PLOT_SELECT_POINT;

	      x = (int)(x+1) % 2;
	      y += (x)?0:1;
	    }
	  
	  resized();
	  
	  
	  gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(scroll_w),canvas);
	  
	  gtk_widget_show(canvas);
	  
	  if (!current_stat)
	    {
	      gtk_widget_set_sensitive(scroll1,0);
	      gtk_widget_set_sensitive(scroll2,0);
	      gtk_widget_set_sensitive(change_mode,0);
	    }
	  else
	    {
	      gtk_widget_set_sensitive(scroll1,1);
	      gtk_widget_set_sensitive(scroll2,1);
	      gtk_widget_set_sensitive(change_mode,1);
	    }
	  gtk_widget_set_sensitive(combo_color,0);
	  gtk_widget_set_sensitive(bbox,0);
	  gtk_widget_set_sensitive(fbox,0);
	  

	  gtk_signal_connect(GTK_OBJECT(canvas), "select_item",
			     (GtkSignalFunc) select_item, NULL);

	  
	  gtk_plot_canvas_paint(GTK_PLOT_CANVAS(canvas));
	  gtk_plot_canvas_refresh(GTK_PLOT_CANVAS(canvas));  
	}  
      else
	{
	  gtk_widget_set_sensitive(bbox,0);
	  gtk_widget_set_sensitive(fbox,0);
	}
    }      
  else
    {
      mode = 0;
      if (gogo)
	{
	  gtk_widget_destroy (gtk_bin_get_child(GTK_BIN(gtk_bin_get_child(GTK_BIN(scroll_w)))));
	  gogo = 0;
	  if (canvas)
	    {
	      gtk_widget_destroy(canvas);
	      canvas = NULL;
	    }
	  if (gabscisse)
	    {
	      free(gabscisse);
	      gabscisse=0;
	    }
	  free_big ();
	  free_histo ();
	}
    } 
}


void big_stat (int graph_index) 
{
  GtkPlotCanvasChild *child;
  int i,longueur;

  canvas=NULL;

  twidth = width;
  theight = height;

  canvas = gtk_plot_canvas_new(twidth,theight,1.0);
  GTK_PLOT_CANVAS_SET_FLAGS(GTK_PLOT_CANVAS(canvas), GTK_PLOT_CANVAS_CAN_SELECT_ITEM);	 

  if (stats[graph_index].type == COURBE)
    {
      big_courbes = malloc(nb_stats * sizeof(my_plot));

      for (i=0;i<nb_stats;i++)
	big_courbes[i].color = 0;

      big_courbes[graph_index].ordonnee = load_graphe_file(graph_index);

      longueur = big_courbes[graph_index].size;
      gabscisse = malloc(longueur * sizeof(double));
      
      bmin = big_courbes[graph_index].amin;
      bmax = big_courbes[graph_index].amax;
      bomin = big_courbes[graph_index].omin;
      bomax = big_courbes[graph_index].omax;

      baise_moi_fort = 0;
      gtk_range_set_range(GTK_RANGE(scroll1),bmin, (bmax + bmin) / 2 - 0.5);
      gtk_range_set_range(GTK_RANGE(scroll2),(bmax + bmin) / 2 + 0.5,bmax);
      gtk_range_set_value(GTK_RANGE(scroll1),bmin);
      gtk_range_set_value(GTK_RANGE(scroll2),bmax);
      baise_moi_fort = 1;

      for (i=0;i<longueur;i++)
	gabscisse[i] = i;
      
      big_courbes[graph_index].abscisse = gabscisse;
      big_courbes[graph_index].color = 1;

      big_courbes[graph_index].dataset = create_graph(graph_index);

      bplot = GTK_WIDGET(big_courbes[graph_index].dataset->plot);

      gtk_plot_data_set_points(big_courbes[graph_index].dataset, big_courbes[graph_index].abscisse, big_courbes[graph_index].ordonnee, NULL, NULL, big_courbes[graph_index].size);

      /*
	g_list->color = gtk_combo_box_get_active(GTK_COMBO_BOX(combo_color)) - 1;
	g_list->valeurs = valeurs;

	bmin = omin;
	bmax = omax;
      */
    }

  else
    {
      load_histo_file(graph_index,1,1);
      

      bplot = GTK_WIDGET(create_plot(graph_index));
      gtk_plot_add_data(GTK_PLOT(bplot),big_courbes[graph_index].dataset);
      /*
      gtk_plot_data_set_points(big_courbes[graph_index].dataset,big_courbes[graph_index].abscisse,big_courbes[graph_index].ordonnee, NULL, NULL,big_courbes[graph_index].size);
      */
      
      gtk_plot_data_draw_points( big_courbes[graph_index].dataset, 1);
      gtk_widget_show(GTK_WIDGET( big_courbes[graph_index].dataset));

    }
  
  
  
  
  child = gtk_plot_canvas_plot_new(GTK_PLOT(bplot));

  /* les 4 chiffres a la fin correspond a tx,ty,bx,by, les points top left et bottom right qui servent a delimiter l affichage du graphe */
  gtk_plot_canvas_put_child(GTK_PLOT_CANVAS(canvas), child, 0.14 , 12./height, 1. - 32./width, 0.88);

  GTK_PLOT_CANVAS_PLOT(child)->flags |= GTK_PLOT_CANVAS_PLOT_SELECT_POINT;
  

  gtk_signal_connect(GTK_OBJECT(canvas), "select_item",
		     (GtkSignalFunc) select_item, NULL);


  
  twidth = width;
  theight = height;
  
  gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(scroll_w),canvas);
  
  gtk_widget_show(canvas);
  
  rescale(NULL,NULL);
}

gboolean downsize (GtkWidget * widget UNUSED, GdkEventButton *event UNUSED, gpointer data UNUSED)
{
  GtkWidget * view = gtk_bin_get_child(GTK_BIN(scroll_w));
  int faire = (width<=300)||(height<300);
  int larg=twidth/width,haut=theight/height;
  
  width -= (faire)?0:30;
  height -= (faire)?0:30;
  twidth = larg * width;
  theight = haut * height;
  gtk_plot_canvas_set_size(GTK_PLOT_CANVAS(canvas),twidth,theight);

  gtk_widget_ref(canvas);
  gtk_widget_unparent (canvas);
  gtk_widget_ref(view);
  gtk_widget_unparent(view);
  gtk_widget_set_parent(canvas,view);
  gtk_widget_set_parent(view,scroll_w);

  gtk_plot_canvas_paint(GTK_PLOT_CANVAS(canvas));
  gtk_plot_canvas_refresh(GTK_PLOT_CANVAS(canvas));  
  gtk_widget_unref(canvas);
  gtk_widget_unref(view);

  return(1);
}

gboolean upsize (GtkWidget * widget UNUSED, GdkEventButton *event UNUSED, gpointer data UNUSED)
{
  GtkWidget * view = gtk_bin_get_child(GTK_BIN(scroll_w));
  int larg=twidth/width,haut=theight/height;

  width += 30 ;
  height += 30;
  twidth = larg * width;
  theight = haut * height;
  gtk_plot_canvas_set_size(GTK_PLOT_CANVAS(canvas),twidth,theight);

  gtk_widget_ref(canvas);
  gtk_widget_unparent (canvas);
  gtk_widget_ref(view);
  gtk_widget_unparent(view);
  
  gtk_widget_set_parent(canvas,view);
  gtk_widget_set_parent(view,scroll_w);
  
  gtk_plot_canvas_paint(GTK_PLOT_CANVAS(canvas));
  gtk_plot_canvas_refresh(GTK_PLOT_CANVAS(canvas));  
  gtk_widget_unref(canvas);
  gtk_widget_unref(view);

  return(1);
}




gboolean stat_changemode (GtkWidget * widget UNUSED, GdkEventButton *event UNUSED, gpointer data UNUSED)
{
  int n=gtk_combo_box_get_active(GTK_COMBO_BOX(combo_stat));

  if (!mode)
    {
      if (n>-1)
	{
	  gtk_widget_destroy (gtk_bin_get_child(GTK_BIN(gtk_bin_get_child(GTK_BIN(scroll_w)))));

	  if (gabscisse)
	    {
	      free(gabscisse);
	      gabscisse = NULL;
	    }

	  gtk_widget_set_sensitive(combo_color,1);
	  gtk_widget_set_sensitive(fbox,1);
	  baise_moi_fort = 0;
	  gtk_combo_box_set_active(GTK_COMBO_BOX(combo_color),1);
	  baise_moi_fort = 1;
	  mode = stats[n].type;
	  if (mode == HISTOGRAMME)gtk_widget_set_sensitive(bbox,1);
	  big_stat(n);
	}
    }
  else
    {
      /*      free_big(); */
      /*      gtk_widget_destroy (gtk_bin_get_child(GTK_BIN(scroll_w)));*/
      gtk_widget_destroy (gtk_bin_get_child(GTK_BIN(gtk_bin_get_child(GTK_BIN(scroll_w)))));
      mode = 0;
      maj_stat(5);
    }
  return(1);
}

gboolean down_date (GtkWidget * widget UNUSED, GdkEventButton *event UNUSED, gpointer data UNUSED)
{
  my_histo * pp = big_histos;
  int i;
  
  while (pp)
    {
      if (pp->courbe == big_courbes)
	break;
      pp = pp->next;
    }
  if (pp->prev)
    {
      pp = pp->prev;
      big_courbes = pp->courbe;
      
      for (i=0;i<nb_stats;i++)
	if (big_courbes[i].color)
	  gtk_plot_data_set_points(big_courbes[i].dataset, big_courbes[i].abscisse, big_courbes[i].ordonnee, NULL, NULL, big_courbes[i].size);

      set_cycle(pp->date);
      bmin = pp->bmin;
      bmax = pp->bmax;
      bomin = pp->bomin;
      bomax = pp->bomax;
      rescale(NULL,NULL);
    }
  
  
  return(1);
}

gboolean stat_export (GtkWidget * widget UNUSED, GdkEventButton *event UNUSED, gpointer data UNUSED)
{

  gtk_plot_canvas_export_ps_with_size(GTK_PLOT_CANVAS(canvas),"./stat.eps",GTK_PLOT_PORTRAIT,TRUE,0,twidth,theight);
  /* fonction automatique, le resultat est assez fun
     gtk_plot_canvas_export_ps(GTK_PLOT_CANVAS(canvas),"./stat.eps",GTK_PLOT_PORTRAIT,TRUE,GTK_PLOT_CUSTOM); */
  return(1);
}

gboolean mup_date (GtkWidget * widget UNUSED, GdkEventButton *event UNUSED, gpointer data UNUSED)
{
  my_histo * pp = big_histos;
  int i;
  
  while (pp)
    {
      if (pp->courbe == big_courbes)
	break;
      pp = pp->next;
    }
  if (pp->next)
    {
      pp = pp->next;
      big_courbes = pp->courbe;
      
      for (i=0;i<nb_stats;i++)
	if (big_courbes[i].color)
	  gtk_plot_data_set_points(big_courbes[i].dataset, big_courbes[i].abscisse, big_courbes[i].ordonnee, NULL, NULL, big_courbes[i].size);

      set_cycle(pp->date);
      bmin = pp->bmin;
      bmax = pp->bmax;
      bomin = pp->bomin;
      bomax = pp->bomax;
      rescale(NULL,NULL);
    }
  
  
  return(1);
}




gboolean show_fun (GtkWidget * widget UNUSED, GdkEventButton *event UNUSED, gpointer data UNUSED)
{
  op = gtk_entry_get_text(GTK_ENTRY(foufoune));
  operation = read_expr();
  
  if (!dd)
    gtk_plot_remove_data(GTK_PLOT(bplot),dd);
  dd = NULL;
      

  dd = gtk_plot_add_function(GTK_PLOT(bplot),(GtkPlotFunc)function);
  gtk_widget_show(GTK_WIDGET(dd));
  
  gtk_plot_data_set_line_attributes(dd,
				    GTK_PLOT_LINE_DASHED,
				    0, 0, 2, &steel);
  
  gtk_plot_add_data(GTK_PLOT(bplot),big_courbes[gtk_combo_box_get_active(GTK_COMBO_BOX(combo_stat))].dataset);
  gtk_widget_show(GTK_WIDGET(dd));
  gtk_plot_canvas_paint(GTK_PLOT_CANVAS(canvas));
  gtk_plot_canvas_refresh(GTK_PLOT_CANVAS(canvas));    

  return(1);
}


/* permet de recalculer les valeurs importantes */
gboolean coconuts (GtkWidget * widget UNUSED, GdkEventButton *event UNUSED, gpointer data UNUSED)
{
  int n = gtk_combo_box_get_active(GTK_COMBO_BOX(combo_stat)),ny,i,teutal = 0;
  double moy=0,var=0;
  double * ty, * tx;
  char putu[9];

  if (n>-1)
    {
      if (stats[n].type)
	{
	  if (mode)
	    {
	      ty = gtk_plot_data_get_y(big_courbes[n].dataset,&ny);
	      tx = gtk_plot_data_get_x(big_courbes[n].dataset,&ny);
	    }
	  else
	    {
	      ty = gtk_plot_data_get_y(stats[n].valeurs.dataset,&ny);
	      tx = gtk_plot_data_get_x(stats[n].valeurs.dataset,&ny);
	    }
	  for (i=0;i<ny;i++)
	    {
	      moy += tx[i] * ty[i];
	      teutal += tx[i];
	    }
	  moy /= teutal;
	  for (i=0;i<ny;i++)
	    var += tx[i]*(ty[i]-moy)*(ty[i]-moy);
	  var /= teutal;
	}
      else
	{
	  if (mode)
	    ty = gtk_plot_data_get_y(big_courbes[n].dataset,&ny);
	  else
	    ty = gtk_plot_data_get_y(stats[n].valeurs.dataset,&ny);
	  
	  for (i=0;i<ny;i++)
	    moy += ty[i];
	  moy /= ny;
	  for (i=0;i<ny;i++)
	    var += (ty[i]-moy)*(ty[i]-moy);
	  var /= ny;
	}	  
    }
  snprintf(putu,8,"%g",moy);
  putu[8]='\0';
  gtk_label_set_text(GTK_LABEL(moyenne),putu);
  snprintf(putu,8,"%g",sqrt(var));
  putu[8]='\0';
  gtk_label_set_text(GTK_LABEL(ecart),putu);
  snprintf(putu,8,"%g",var);
  putu[8]='\0';
  gtk_label_set_text(GTK_LABEL(variance),putu);
  return(1);
}

GtkWidget * statify ()
{

  /* genre il est 4 heures du math et j;ai envie de faire du code propre avec les noms de var inspires des exemples! */
  GtkWidget *hbox, *vbox, *button, *scalebox, *lab, *fluxbox;
  char * tmp;
  int i;

  /* hertical box */
  hbox = gtk_hbox_new(FALSE,10);
  gtk_container_set_border_width(GTK_CONTAINER (hbox), 10);

  /* scrolled window */
  scroll_w = gtk_scrolled_window_new(NULL, NULL);
  gtk_container_set_border_width(GTK_CONTAINER (scroll_w), 0);
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll_w), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

  /*gtk_box_pack_start(GTK_BOX(hbox), scroll, TRUE, TRUE, 0);*/

  gtk_container_add(GTK_CONTAINER (hbox),GTK_WIDGET(scroll_w));
  vbox = gtk_vbox_new(FALSE,10);
  gtk_box_pack_start(GTK_BOX(hbox),vbox,FALSE,FALSE,0);

  bbox = gtk_hbox_new(TRUE,10);  

  button=gtk_button_new_with_label("-");
  gtk_box_pack_start(GTK_BOX (bbox),button,TRUE,TRUE,0); 
  g_signal_connect (button, "clicked",G_CALLBACK (downsize), NULL);

  button=gtk_button_new_with_label("+");
  gtk_box_pack_start(GTK_BOX (bbox),button,TRUE,TRUE,0); 
  g_signal_connect (button, "clicked",G_CALLBACK (upsize), NULL);

  gtk_box_pack_start(GTK_BOX (vbox),bbox,FALSE,FALSE,0); 

  combo_stat=gtk_combo_box_new_text ();
  gtk_box_pack_start(GTK_BOX (vbox),combo_stat,0,0,0); 

  /*  gtk_container_add(GTK_CONTAINER (vbox),combo_stat); */

  /*  big_stats = calloc(nb_stats, sizeof(void *));  */

  for (i=0;i<nb_stats;i++)
    {
      tmp = mega_cct(3,stats[i].ordonnee," par ",stats[i].abscisse);
      gtk_combo_box_append_text(GTK_COMBO_BOX(combo_stat),tmp);
      free(tmp);
    }

  g_signal_connect (combo_stat, "changed",G_CALLBACK (graphe_change), NULL);

  combo_color=gtk_combo_box_new_text ();
  gtk_box_pack_start(GTK_BOX (vbox),combo_color,0,0,0); 

  gtk_combo_box_append_text(GTK_COMBO_BOX(combo_color),"none");
  for (i=0;i<NB_COLORS;i++)
    gtk_combo_box_append_text(GTK_COMBO_BOX(combo_color),colors[i].name);
  
  g_signal_connect (combo_color, "changed",G_CALLBACK (color_change), NULL);

  change_mode=gtk_button_new_with_label("Changer de mode!");
  gtk_box_pack_start(GTK_BOX (vbox),change_mode,FALSE,FALSE,0); 
  g_signal_connect (change_mode, "clicked",G_CALLBACK (stat_changemode), NULL);
  g_signal_connect (scroll_w, "button_press_event",G_CALLBACK (stat_click), NULL);

  scalebox = gtk_vbox_new(FALSE,0);
  /*gtk_container_add(GTK_CONTAINER (vbox),scalebox);*/
  gtk_box_pack_start(GTK_BOX (vbox),scalebox,0,0,0);
  scroll1 = gtk_hscale_new_with_range(0,10,0.1);
  scroll2 =  gtk_hscale_new_with_range(10,100,0.1);
  lab = gtk_label_new("Minimum :");
  gtk_container_add(GTK_CONTAINER (scalebox),lab);
  gtk_container_add(GTK_CONTAINER (scalebox),scroll1);
  lab = gtk_label_new("Maximum :");
  gtk_container_add(GTK_CONTAINER (scalebox),lab);
  gtk_container_add(GTK_CONTAINER (scalebox),scroll2);


  button=gtk_button_new_with_label("Exporter stat.eps");
  gtk_box_pack_start(GTK_BOX (vbox),button,FALSE,FALSE,0); 
  g_signal_connect (button, "clicked",G_CALLBACK (stat_export), NULL);

  bbox = gtk_hbox_new(FALSE,10);  


  button=gtk_button_new_with_label("<");
  gtk_box_pack_start(GTK_BOX (bbox),button,FALSE,FALSE,0); 
  g_signal_connect (button, "clicked",G_CALLBACK (down_date), NULL);

  button=gtk_button_new_with_label(">");
  gtk_box_pack_start(GTK_BOX (bbox),button,FALSE,FALSE,0); 
  g_signal_connect (button, "clicked",G_CALLBACK (mup_date), NULL);
  
  label = gtk_label_new("Cycle :");
  gtk_box_pack_start(GTK_BOX (bbox),label,FALSE,FALSE,0); 

  gtkcycle = gtk_label_new("");
  gtk_box_pack_start(GTK_BOX (bbox),gtkcycle,FALSE,FALSE,0); 

  gtk_box_pack_start(GTK_BOX (vbox),bbox,FALSE,FALSE,0); 

  fluxbox = gtk_hbox_new(FALSE,10);  
  moyenne = label = gtk_label_new("Moyenne : ");
  gtk_box_pack_start(GTK_BOX (fluxbox),moyenne,FALSE,FALSE,0);
  moyenne = label = gtk_label_new("");
  gtk_box_pack_start(GTK_BOX (fluxbox),moyenne,FALSE,FALSE,0);
  gtk_box_pack_start(GTK_BOX (vbox),fluxbox,FALSE,FALSE,0); 

  fluxbox = gtk_hbox_new(FALSE,10);  
  ecart = label = gtk_label_new("Ecart-type : ");
  gtk_box_pack_start(GTK_BOX (fluxbox),ecart,FALSE,FALSE,0);
  ecart = label = gtk_label_new("");
  gtk_box_pack_start(GTK_BOX (fluxbox),ecart,FALSE,FALSE,0);
  gtk_box_pack_start(GTK_BOX (vbox),fluxbox,FALSE,FALSE,0); 

  fluxbox = gtk_hbox_new(FALSE,10);  
  variance = label = gtk_label_new("Variance : ");
  gtk_box_pack_start(GTK_BOX (fluxbox),variance,FALSE,FALSE,0);
  variance = label = gtk_label_new("");
  gtk_box_pack_start(GTK_BOX (fluxbox),variance,FALSE,FALSE,0);
  gtk_box_pack_start(GTK_BOX (vbox),fluxbox,FALSE,FALSE,0); 

  button=gtk_button_new_with_label("Recalculer");
  gtk_box_pack_start(GTK_BOX (vbox),button,FALSE,FALSE,0); 
  g_signal_connect (button, "clicked",G_CALLBACK (coconuts), NULL);

  fbox = gtk_hbox_new(FALSE,0);
  foufoune = gtk_entry_new();
  gtk_box_pack_start(GTK_BOX (fbox),foufoune,FALSE,FALSE,0); 
  button=gtk_button_new_with_label("Afficher");
  gtk_box_pack_start(GTK_BOX (fbox),button,FALSE,FALSE,0); 
  g_signal_connect (button, "clicked",G_CALLBACK (show_fun), NULL);
  gtk_box_pack_start(GTK_BOX (vbox),fbox,FALSE,FALSE,0); 


  fluxbox = gtk_hbox_new(FALSE,10);  
  sx = gtk_label_new("X : ");
  gtk_box_pack_start(GTK_BOX (fluxbox),sx,FALSE,FALSE,0);
  sx = gtk_label_new("");
  gtk_box_pack_start(GTK_BOX (fluxbox),sx,FALSE,FALSE,0);
  sy = gtk_label_new("Y : ");
  gtk_box_pack_start(GTK_BOX (fluxbox),sy,FALSE,FALSE,0);
  sy = gtk_label_new("");
  gtk_box_pack_start(GTK_BOX (fluxbox),sy,FALSE,FALSE,0);
  gtk_box_pack_start(GTK_BOX (vbox),fluxbox,FALSE,FALSE,0); 


  g_signal_connect (scroll1, "value-changed",G_CALLBACK (rescale), NULL);
  g_signal_connect (scroll2, "value-changed",G_CALLBACK (rescale), NULL);


  return(hbox);
}

