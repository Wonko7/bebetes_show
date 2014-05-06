#define _GNU_SOURCE
#include <gtk/gtk.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

#include <math.h>
#include <string.h>

#include <time.h>

#include <locale.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/times.h>


#include <dirent.h>
#include <pwd.h>

#include "lib_bbs_behavior.h"
#include "loader.h"
#include "camera.h"
#include "divers.h"
#include "map.h"

char *chemin;
long tps_date=0,gtktime=0;
double tcos[360];
float pi;
extern GtkWidget *window;  
extern GtkWidget *note_fucking_book;  
extern int vitesse_simulation;
int caca_q_rep;
extern char *save_dir;
int ogl_tps_loading = 0;
/*tps_total d'une partie*/
unsigned long tps_tot = 0;
float tps_tot_ac = 0.0;
int cy_depr = 0;
double cpu_time = 0.0;
double moy [] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
double tps_cam = 0.0;
int moyhead = 0;


/* locale */
char *remy, * remy2, * remy3;

/*
  ici on met toutes les petites fonctions qui 
  servent partout, genre concat...
*/

void gtk_caca(char *s)
{
  GtkWidget *dialog;
  dialog = gtk_message_dialog_new (GTK_WINDOW (window),
				   GTK_DIALOG_DESTROY_WITH_PARENT,
				   GTK_MESSAGE_ERROR,
				   GTK_BUTTONS_CLOSE,
				   s);
  gtk_dialog_run (GTK_DIALOG (dialog));
  gtk_widget_destroy (dialog);
}

int caca_q(GtkDialog *dialog UNUSED,
	   gint       arg1,
	   gpointer   user_data)
{
  caca_q_rep = arg1;

  fflush(stdout);
  gtk_widget_destroy(user_data);
  return(1);
}

int gtk_caca_q(char *s)
{
  GtkWidget *dialog;
  dialog = gtk_message_dialog_new (GTK_WINDOW (window),
				   GTK_DIALOG_DESTROY_WITH_PARENT,
				   GTK_MESSAGE_QUESTION,
				   GTK_BUTTONS_NONE,
				   s);
  gtk_dialog_add_button(GTK_DIALOG (dialog),"Oui",1);
  gtk_dialog_add_button(GTK_DIALOG (dialog),"Non",0);
  g_signal_connect_swapped (dialog, "response",
			    G_CALLBACK (caca_q),
			    dialog);
  gtk_dialog_run (GTK_DIALOG (dialog));
  /*  g_signal_connect (dialog, "response",
			    G_CALLBACK (caca_q),
			    dialog);*/
  /*
  gtk_widget_destroy (dialog);
  */
  return(caca_q_rep);
}

double get_tps()
{
  /*long t;*/
  double res, mm;
  int i;
  static int je_suis_moche = 40;

  double cpu_old;
  struct tms cpu;

  cpu_old = (double) cpu_time;
  times(&cpu);
  cpu_time = (double) cpu.tms_utime;

  res = ((double) cpu_time - (double) cpu_old) * 100000.0 / CLOCKS_PER_SEC; 
  /* * (double) vitesse_simulation;*/

  /*  if (!vitesse_simulation)
    res = 0;
  else
    if (vitesse_simulation == -1)
      res = (double) RAFR / 1000.0;
    else
    res = ((double) cpu_time - (double) cpu_old) * 100000.0 / CLOCKS_PER_SEC  * (double) vitesse_simulation;*/

  tps_tot_ac += res;

  if (tps_tot_ac > 1.0)
    {
      tps_tot += (int) tps_tot_ac;
      tps_tot_ac = 0.0;
    }

  /* moy */
  if (moyhead == 40)
    moyhead = 0;
  moy[moyhead++] = res;
  mm = 0.0;

  for (i = 0; i < 40; i++)
    mm += moy[i];

  tps_cam = mm / 40.0;
  if (!vitesse_simulation || je_suis_moche)
    {
      mm = 0.0;
      if (je_suis_moche)
	{
	  je_suis_moche--;
	  if (!je_suis_moche)
	    ogl_tps_loading = 1;
	}
    }
  else
    if (vitesse_simulation == -1)
      mm = (double) RAFR / 1000.0;
    else
      mm /= (double) 40.0 / (double) vitesse_simulation;      

        /*printf("%g %g %g %g\n", cpu_time / CLOCKS_PER_SEC, cpu_old / CLOCKS_PER_SEC, cpu_time - cpu_old, res);*/


  /*
  t = gtktime;
  
  if (!vitesse_simulation)
    res = 0;
  else
    if (vitesse_simulation == -1)
      res = (double) RAFR / 1000.0;
    else
      res = (double) (t-tps_date) / 1000.0 * vitesse_simulation;

  tps_date = t;
  tps_tot_ac += res;

  if (tps_tot_ac > 1.0)
    {
      tps_tot += (int) tps_tot_ac;
      tps_tot_ac = 0.0;
    }
  */

  /*
  **    printf("Cycles CPUs %li - %li - res : %g",t,CLOCKS_PER_SEC,res);
  **    puts("");
  */

  return mm;
} 

int majgtktime () 
{
  int sel;
  sel = gtk_notebook_get_current_page(GTK_NOTEBOOK (note_fucking_book));

  if (sel == 1)
    gtktime += (double) RAFR;

  return 1;
}

char *mega_cct (int i,...)
{
  va_list ap;
  char * res=NULL,* string;
  int l = 0,as;
  
  va_start(ap,i);

  for (;i>0;i--)
    {
      string = va_arg(ap,char *);
      if (!string)
	continue;
      as = strlen(string);
      res = realloc(res,(l+as+1) * sizeof(char));
      memcpy(&res[l],string,(as+1) * sizeof(char));
      l += as;
    }
  
  va_end(ap);
  return(res);
}  

/* faire un FREE sur la chaine apres utilisation */

char * my_concat2 (const char *s1, const char *s2)
{
  int i,j;
  char * tmp;

  i = s1 ? strlen(s1) : 0; 
  j = s2 ? strlen(s2) : 0; 

  tmp = malloc((i + j + 1) * sizeof(char));
  memcpy(tmp,s1,i * sizeof(char));
  memcpy(tmp + i,s2,j * sizeof(char));
  tmp[i+j] = 0;
  return(tmp);
}
char * my_concat (char *s1, char *s2)
{
  int i,j;
  char * tmp;

  i = s1 ? strlen(s1) : 0; 
  j = s2 ? strlen(s2) : 0; 

  tmp = malloc((i + j + 1) * sizeof(char));
  memcpy(tmp,s1,i * sizeof(char));
  memcpy(tmp + i,s2,j * sizeof(char));
  tmp[i+j] = 0;
  return(tmp);
}

/* j'aime pas me repeter, mais :
   faire un FREE sur la chaine apres utilisation 
   int to string :                               */
char * ftos (float f)
{
  char *s;

  asprintf(&s,"%.2f",f);
  return(s);
}

char * itos (int i) 
{
  char *s;

  asprintf(&s,"%i",i);
  return(s);
}


/* Ne pas regarder la source de cette fonction
   si vous avez mangé recemment...             */

void trouver_chemin(int argc UNUSED, char **argv)
{
  char c[256],t;
  FILE * f;
  int i;

  c[0]=0;
  strcat(c,"which ");
  strcat(c,argv[0]);
  strcat(c," >> /tmp/tmpbbs");
  system(c);
  
  f=fopen("/tmp/tmpbbs","r");

  t=getc(f);
  i=0;
  while(t!=EOF && t!='\n')
    {
      c[i]=t;
      i++;
      t=getc(f);
    }
  c[i]=0;

  while(c[i]!='/')
    i--;
  c[i]=0;
  i++;
  chemin=malloc((sizeof(char))*i);
  memcpy(chemin,c,i);
  fclose(f);
  system("rm /tmp/tmpbbs");
}

float mfabs (float e)
{
  if (e>0)
    return(e);
  else
    return(-1.0*e);
}

void start_locale()
{
  remy = setlocale(LC_ALL,NULL);
  remy3 = my_concat(remy,"");
  if (!strcmp(remy3,"C"))
    {
      free(remy3);
      remy3=0;
    }
  else  
    setlocale(LC_ALL,"POSIX");
}

void end_locale()
{
  if (remy3)
    {    
      setlocale(LC_ALL,remy3);
      free(remy3);
    }
}

float my_min (float x,float y)
{
  return((x>y)?y:x);
}
float my_max (float x,float y)
{
  return((x>y)?x:y);
}

/*void get_case (t_agent agent,int * i, int * j)
{
  *i = agent->x / TAILLE_CASE;
  *j = agent->z / TAILLE_CASE;
}*/


/* ca n'a rien a faire dans divers ca ! fuck... */
void move_truc (t_objet * objet,t_objet * list_agent)
{
  t_objet tmp;
  
  tmp = *list_agent;
  list_agent = objet;
  (*objet)->next = tmp;
  *objet = NULL;
}



int supprec (char *d)
{
  DIR *dir;
  char *f;
  struct dirent *d1;
  struct stat sf;

  dir = opendir(d);
  if (!dir)
    {
      caca(d);
      caca("supprec : Ouvrirture le du repertoire pas arrivé");
      return(0);
    }

  readdir(dir);
  readdir(dir);

  while ((d1 = readdir(dir)))
    {
      f = mega_cct(3,d,"/",d1->d_name);
      if (stat(f,&sf))
	{
	  caca(f);
	  caca("supprec : stat bug");
	  return(0);
	}
      if (S_ISDIR(sf.st_mode))
	{
	  if (!supprec(f))
	    return(0);
	}
      else
	unlink(f);
      free(f);
    }

  closedir(dir);
  rmdir(d);

  return(1);
}

#define T_BLOC 256
int cp_file(char *src, char *dest)
{
  FILE *fsrc, *fdest;
  int bloc[T_BLOC], i = T_BLOC, j = 1;
  

  start_locale();
  if (!(fsrc  = fopen(src,"r")))
    {
      caca(src);
      caca("cp_file: source n'existe pas");
      return(0);
    }

  if (!(fdest  = fopen(dest,"w")))
    {
      caca(dest);
      caca("cp_file: dest inouvrable");
      return(0);
    }

  
  while (i == T_BLOC)
    {
      j = i = 0;
      while ((i != EOF) && (j < T_BLOC))
	{
	  bloc[j] = i = getc(fsrc);
	  j++;
	}
      for (i = 0; i < j; i++)
	putc(bloc[i], fdest);
    }

  fclose(fsrc);
  fclose(fdest);
  end_locale();

  return(1);
}

/*
** /!\ _src_ ET _dest_ doivent exister. 
** (je pourrai faire dest mais ca fait des testes en plus)
** contrairement au cp unix, je copie le contenu de src
** dans dest (et non depuis le rep src dans dest)
*/
int cp_rec(char *src, char *dest)
{
  DIR *dir,*f;
  char *fsrc,*fdest;
  struct dirent *d1;
  struct stat sf;

  dir = opendir(src);
  if (!dir)
    {
      caca(src);
      caca("cp_cont : Ouvrirture le du repertoire pas arrivé");
      return(0);
    }

  readdir(dir);
  readdir(dir);

  while ((d1 = readdir(dir)))
    {
      fsrc = mega_cct(3,src,"/",d1->d_name);
      fdest = mega_cct(3,dest,"/",d1->d_name);

      if (stat(fsrc,&sf))
	{
	  caca(fsrc);
	  caca("cp_rec : stat bug");
	  return(0);
	}
      if (S_ISDIR(sf.st_mode))
	{
	  if ((f = opendir(fdest)))
	    closedir(f);
	  else 
	    mkdir(fdest,00755);
	  if (!cp_rec(fsrc,fdest))
	    return(0);
	}
      else
	cp_file(fsrc,fdest);
      free(fsrc);
      free(fdest);
    }
  closedir(dir);

  return(1);
}

int create_save_dir()
{
  char *c, *t, *fsrc, *fdest;
  struct passwd * p;
  DIR *d,*dd;

  p = getpwuid(getuid());
  c = mega_cct(2,p->pw_dir,"/.bebetes_show");
  save_dir = mega_cct(2,c,"/save");
  t = mega_cct(2,chemin,"/../share/bebetes_show/save");
  struct dirent *d1,*d2;

  /* exists */
  if ((d = opendir(save_dir)))
    {
      free(t);
      free(c);
      closedir(d);
      return(1);
    }

  /* else create it */
  if (mkdir(c,00755) || mkdir(save_dir,00755))
    {
      caca("la creation du dossier de sauvegarde a échoué");
      caca(c);
      free(t);
      free(c);
      return(0);
    }
  cp_rec(t,save_dir);

  /* les bonnes libs */
  d = opendir(t);
  readdir(d);
  readdir(d);

  while ((d1 = readdir(d)))
    {
      fsrc = mega_cct(4,t,"/",d1->d_name,"/lib");
      if ((dd = opendir(fsrc)))
	{
	  readdir(dd);
	  readdir(dd);
	  d2 = readdir(dd);
	  if (d2)
	    {
	      free(fsrc);
	      fsrc = mega_cct(3,chemin,"/../lib/",d2->d_name);
	      fdest = mega_cct(5,save_dir,"/",d1->d_name,"/lib/",d2->d_name);
	      cp_file(fsrc,fdest);
	      free(fdest);
	    }
	  closedir(dd);
	}
      free(fsrc);
    }

  closedir(d);
  free(c);
  free(t);

  return(1);
}

/* alexandre... t'es un putain de connard */
/* alexandre... t'es un putain de connard */
/* alexandre... t'es un putain de connard */
/* alexandre... t'es un putain de connard */
