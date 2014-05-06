/*
 * jpeg.c -- jpeg texture loader
 * last modification: feb. 9, 2006
 *
 * Copyright (c) 2005-2006 David HENRY
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * gcc -Wall -ansi -L/usr/X11R6/lib -lGL -lGLU -lglut -ljpeg jpeg.c -o jpeg
 */

#include <GL/glut.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <jpeglib.h>

/* Microsoft Visual C++ */
#ifdef _MSC_VER
#pragma comment (lib, "libjpeg.lib")
#endif	/* _MSC_VER */


/* OpenGL texture info */
typedef struct
{
  GLsizei width;
  GLsizei height;

  GLenum format;
  GLint internalFormat;
  GLuint id;

  GLubyte *texels;

} gl_texture_t;


/* texture Id */
GLuint texId;


gl_texture_t *
ReadJPEGFromFile (const char *filename)
{
  gl_texture_t *texinfo = NULL;
  FILE *fp = NULL;
  struct jpeg_decompress_struct cinfo;
  struct jpeg_error_mgr jerr;
  JSAMPROW j;
  int i;

  /* open image file */
  fp = fopen (filename, "rb");
  if (!fp)
    {
      fprintf (stderr, "error: couldn't open \"%s\"!\n", filename);
      return NULL;
    }

  /* create and configure decompressor */
  jpeg_create_decompress (&cinfo);
  cinfo.err = jpeg_std_error (&jerr);
  jpeg_stdio_src (&cinfo, fp);

  /*
   * NOTE: this is the simplest "readJpegFile" function. There
   * is no advanced error handling.  It would be a good idea to
   * setup an error manager with a setjmp/longjmp mechanism.
   * In this function, if an error occurs during reading the JPEG
   * file, the libjpeg abords the program.
   * See jpeg_mem.c (or RTFM) for an advanced error handling which
   * prevent this kind of behavior (http://tfc.duke.free.fr)
   */

  /* read header and prepare for decompression */
  jpeg_read_header (&cinfo, TRUE);
  jpeg_start_decompress (&cinfo);

  /* initialize image's member variables */
  texinfo = (gl_texture_t *)malloc (sizeof (gl_texture_t));
  texinfo->width = cinfo.image_width;
  texinfo->height = cinfo.image_height;
  texinfo->internalFormat = cinfo.num_components;

  if (cinfo.num_components == 1)
    texinfo->format = GL_LUMINANCE;
  else
    texinfo->format = GL_RGB;

  texinfo->texels = (GLubyte *)malloc (sizeof (GLubyte) * texinfo->width
			       * texinfo->height * texinfo->internalFormat);

  /* extract each scanline of the image */
  for (i = 0; i < texinfo->height; ++i)
    {
      j = (texinfo->texels +
	((texinfo->height - (i + 1)) * texinfo->width * texinfo->internalFormat));
      jpeg_read_scanlines (&cinfo, &j, 1);
    }

  /* finish decompression and release memory */
  jpeg_finish_decompress (&cinfo);
  jpeg_destroy_decompress (&cinfo);

  fclose (fp);
  return texinfo;
}


GLuint
loadJPEGTexture (const char *filename)
{
  gl_texture_t *jpeg_tex = NULL;
  GLuint tex_id = 0;

  jpeg_tex = ReadJPEGFromFile (filename);

  if (jpeg_tex && jpeg_tex->texels)
    {
      /* generate texture */
      glGenTextures (1, &jpeg_tex->id);
      glBindTexture (GL_TEXTURE_2D, jpeg_tex->id);

      /* setup some parameters for texture filters and mipmapping */
      /*glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);*/
      glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

      /*
      glTexImage2D (GL_TEXTURE_2D, 0, jpeg_tex->internalFormat,
		    jpeg_tex->width, jpeg_tex->height, 0, jpeg_tex->format,
		    GL_UNSIGNED_BYTE, jpeg_tex->texels);
      */
     
      gluBuild2DMipmaps (GL_TEXTURE_2D, jpeg_tex->internalFormat,
			 jpeg_tex->width, jpeg_tex->height,
			 jpeg_tex->format, GL_UNSIGNED_BYTE, jpeg_tex->texels);
     

      tex_id = jpeg_tex->id;

      /* OpenGL has its own copy of texture data */
      free (jpeg_tex->texels);
      free (jpeg_tex);
    }

  return tex_id;
}


/* alexandre... t'es un putain de connard */
/* alexandre... t'es un putain de connard */
/* alexandre... t'es un putain de connard */
/* alexandre... t'es un putain de connard */
