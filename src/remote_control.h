                           /**************************/
#define VITZ 50.0          /* cases ogl par secondes */
#define VITR 150           /* degres par secondes    */
#define VITXY 10            /* cases ogl par secondes */
#define VITXY_MOUSE 3
#define VIT_CAM_MV 2.0     /* cases ogl par secondes */
#define VIT_CAM_MIN 0.5    /* vitesse minimum        */                       
                           /**************************/


int create_remote_control ( GtkWidget *widget,
			    GdkEvent  *event,
			    gpointer   data );
void modif_cam_remote_ctl ();
void set_cam_delta();
/* alexandre... t'es un putain de connard */
