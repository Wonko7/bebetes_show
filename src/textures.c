#include <GL/glut.h>    
#include <GL/gl.h>     
#include <GL/glu.h>	
#include <unistd.h>
#include "iloaders.h"
#include <string.h>

int realized=0;

GLuint glTexLoad (char * file)
{
  if ((file[strlen(file) - 3] == 't') && (file[strlen(file) - 2] == 'g') && (file[strlen(file) - 1] == 'a')) 
    return(loadTGATexture(file));
  if ((file[strlen(file) - 3] == 'b') && (file[strlen(file) - 2] == 'm') && (file[strlen(file) - 1] == 'p')) 
    return(loadBMPTexture(file));
  return(0);
}

