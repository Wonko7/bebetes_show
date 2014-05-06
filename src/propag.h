/*        DEPRECATED       *\
void propagate_stimulus(int x, int y, int stim, float puiss, float decrement);
void depropagate_stimulus(int x, int y, int stim, float puiss, float decrement);
void mega_propagate(int x, int y, int nb, int * stim, float puiss, float decrement);
void mega_depropagate(int x, int y, int nb, int * stim, float puiss, float decrement);
void deprop_int(t_agent agent);
*/

int free_tab_stim_int();
int init_tab_stim_int();

int ejac(int sens, int xx, int yy, int sti, float fforce, float decr);
int eponge(int sens, int xx, int yy, int sti, float fforce, float decr);
int deprop_int(t_agent a, float d);
int deprop_mega(int x, int y, int nb, int * stim, float d);
int prop_mega(int x, int y, int nb, int * stim, float force, float d);

#define prop_e(x, y, sti, f, d) ejac(8, x, y, sti, f, d)
#define deprop_e(x, y, sti, f, d) eponge(8, x, y, sti, f, d)

/*pour mes tests :
#define propagate_stimulus(x,y,sti,f,d) ejac(8, x, y, sti, f, d)
#define depropagate_stimulus(x,y,sti,f,d) eponge(8, x, y, sti, f, d)
#define mega_propagate(x,y,nb,s,f,d) prop_mega(x,y,nb,s,f,d)
#define deprop_int(a) deprop_mega(a,1.0)
*/

/* alexandre... t'es un putain de connard */
