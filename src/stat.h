enum
  {
    COURBE = 1,
    HISTOGRAMME
  };
GtkWidget * statify ();
void end_stat();
void charger_stats();
void save_stats();
void maj_stat  (guint page_num);
void add_courbe_stat (int graph, double value);
void add_histo_stat (int graph, double value);
void free_plots ();
void init_stats_var ();
void init_colors ();
void free_colors ();
void free_stats ();
void spush (int n, double value);

typedef struct {
  GtkPlotData *dataset;
  int size;
  double *ordonnee;
  double *abscisse;
  double amin;
  double amax;
  double omin;
  double omax;
  int color;
} my_plot;

typedef struct 
{
  char * abscisse;
  char * ordonnee;
  char * fichier;
  my_plot valeurs;
  int type;
} graphe;

