/* millisecondes : */
#define RAFR 50
#define UNUSED  __attribute__((unused))

/* Limites cam */
#define BETA_SUP 85
#define BETA_INF 1
#define ZOOM_SUP -5
#define ZOOM_INF -50
#define RAD 57.2957795
#define my_cos(x) cos((x) / RAD)
#define my_sin(x) sin((x) / RAD)



/* 
**    ** debug, a supprimer dans la version finale  **
**
**    on appelle caca avec une string ou 0 en parametre.
**    elle a un puts pour flusher.
**
**    si b est vrai arse affiche le message et quitte le prog.     
*/

#define caca(s) {printf("/!\\ Attention : %s:%i: ",__FILE__,__LINE__);{if (s) puts(s); else puts(" ");}}
#define caca_(s) printf("\033[22;31m/!\\\033[22;34m Attention : %s:%i: \033[22;37m",__FILE__,__LINE__);{if (s) puts(s); else puts(" ");}
#define arse(b,s) if (b) {printf("/!\\ Oops : %s:%i: ",__FILE__,__LINE__);  if (s) puts(s); else puts(" "); exit(1); }
#define in_map(i,j) (i>=0 && j>=0 && i<carte.x_max && j<carte.y_max)
/*attention pour se servir de gtk_caca faut avoir extern window*/
void gtk_caca(char *s);
int gtk_caca_q(char *s);

char *mega_cct (int i,...);
char *my_concat (char *s1, char *s2);
char *my_concat2 (const char *s1, const char *s2);
char * itos (int i);
char * ftos (float f);
void trouver_chemin(int argc, char *argv[]);
double get_tps();
int majgtktime () ;

float mfabs (float e);
void start_locale();
void end_locale();
float my_min(float x, float y);
float my_max(float x, float y);
void get_case (t_agent agent,int * i, int * j);
void move_truc (t_objet * objet, t_objet * list_agent);
int supprec (char *d);
int cp_file(char *src, char *dest);
int create_save_dir();
int cp_rec(char *src, char *dest);
/* alexandre... t'es un putain de connard */
