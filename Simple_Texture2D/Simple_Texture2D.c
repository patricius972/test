//
// Book:      OpenGL(R) ES 2.0 Programming Guide
// Authors:   Aaftab Munshi, Dan Ginsburg, Dave Shreiner
// ISBN-10:   0321502795
// ISBN-13:   9780321502797
// Publisher: Addison-Wesley Professional
// URLs:      http://safari.informit.com/9780321563835
//            http://www.opengles-book.com
//

// Simple_Texture2D.c
//
//    This is a simple example that draws a quad with a 2D
//    texture image. The purpose of this example is to demonstrate 
//    the basics of 2D texturing
//
#include <stdlib.h>
#include "esUtil.h"
#include <EGL/eglext.h>
#include <GLES2/gl2ext.h>
#include <ump/ump_ref_drv.h>

typedef struct
{
   // Handle to a program object
   GLuint programObject;

   // Attribute locations
   GLint  positionLoc;
   GLint  texCoordLoc;

   // Sampler location
   GLint samplerLoc;

   // Texture handle
   GLuint textureId;

} UserData;
/* EGLImage extension functions. */
PFNEGLCREATEIMAGEKHRPROC _eglCreateImageKHR = NULL;
PFNEGLDESTROYIMAGEKHRPROC _eglDestroyImageKHR = NULL;
PFNGLEGLIMAGETARGETTEXTURE2DOESPROC _glEGLImageTargetTexture2DOES = NULL;
    GLubyte * pixels;
GLenum eTexTarget = GL_TEXTURE_2D;
ump_handle buffer_h;

///
// Create a simple 2x2 texture image with four different colors
//
GLuint CreateSimpleTexture2D( )
{
   // Texture object handle
   GLuint textureId;
   int i;
   // 2x2 Image, 3 bytes per pixel (R, G, B)
   GLubyte pix[4*3] =
   {  
      255,   0,   0, // Red
        0, 255,   0, // Green
        0,   0, 255, // Blue
      255, 255,   0  // Yellow
   };
   for (i=0;i<12;i++)
	   pixels[i]=pix[i];
		  

   // Use tightly packed data
   glPixelStorei ( GL_UNPACK_ALIGNMENT, 1 );

   // Generate a texture object
   glGenTextures ( 1, &textureId );

   // Bind the texture object
   glBindTexture ( GL_TEXTURE_2D, textureId );

   // Load the texture
   glTexImage2D ( GL_TEXTURE_2D, 0, GL_RGB, 2, 2, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels );

   // Set the filtering mode
   glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
   glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );

   return textureId;

}


///
// Initialize the shader and program object
//
int Init ( ESContext *esContext )
{
   esContext->userData = malloc(sizeof(UserData));	
   UserData *userData = esContext->userData;
   GLbyte vShaderStr[] =  
      "attribute vec4 a_position;   \n"
      "attribute vec2 a_texCoord;   \n"
      "varying vec2 v_texCoord;     \n"
      "void main()                  \n"
      "{                            \n"
      "   gl_Position = a_position; \n"
      "   v_texCoord = a_texCoord;  \n"
      "}                            \n";
   
   GLbyte fShaderStr[] =  
      "precision mediump float;                            \n"
      "varying vec2 v_texCoord;                            \n"
      "uniform sampler2D s_texture;                        \n"
      "void main()                                         \n"
      "{                                                   \n"
      "  gl_FragColor = texture2D( s_texture, v_texCoord );\n"
      "}                                                   \n";
   /* Retrieve EGLImage functions dynamically as they are not delivered in the standard headers */
   _eglCreateImageKHR = (PFNEGLCREATEIMAGEKHRPROC) eglGetProcAddress("eglCreateImageKHR");
   _eglDestroyImageKHR = (PFNEGLDESTROYIMAGEKHRPROC) eglGetProcAddress("eglDestroyImageKHR");
   _glEGLImageTargetTexture2DOES = (PFNGLEGLIMAGETARGETTEXTURE2DOESPROC) eglGetProcAddress("glEGLImageTargetTexture2DOES");

   // Load the shaders and get a linked program object
   userData->programObject = esLoadProgram ( vShaderStr, fShaderStr );

   // Get the attribute locations
   userData->positionLoc = glGetAttribLocation ( userData->programObject, "a_position" );
   userData->texCoordLoc = glGetAttribLocation ( userData->programObject, "a_texCoord" );
   
   // Get the sampler location
   userData->samplerLoc = glGetUniformLocation ( userData->programObject, "s_texture" );

   // Load the texture
   buffer_h = ump_ref_drv_allocate(2*2*3,UMP_REF_DRV_CONSTRAINT_USE_CACHE);
   //pixels = (GLubyte*) ump_mapped_pointer_get(buffer_h);
   pixels = (GLubyte*) malloc(2*2*3);
   GLuint texture = CreateSimpleTexture2D ();
	/* [Create the EGLImage] */
    /* create the new EGLImageKHR */
    const EGLint attrs[] =
    {
        EGL_IMAGE_PRESERVED_KHR,    EGL_TRUE,
	 EGL_GL_TEXTURE_LEVEL_KHR ,0,
        EGL_NONE,                   EGL_NONE
    };

    /* Retrieve andorid native buffer in order to associate the Android buffer to the texture */
    EGLClientBuffer anb =(void*)texture;//buffer;//->getNativeBuffer();

    /* Create the EGLImageKHR, which will have the Android native buffer assigned to itself */
    EGLImageKHR pEGLImage = _eglCreateImageKHR(esContext->eglDisplay,esContext->eglContext, EGL_GL_TEXTURE_2D_KHR, (EGLClientBuffer)anb, attrs);


    /* If eglCreateImageKHR returned no image exit the app */
    if(pEGLImage == EGL_NO_IMAGE_KHR)
    {
        printf("[setupGraphics] eglCreateImage() failed \n");
        exit(1);
    }
	/* [Create the 2D texture] */
    // Create an OpenGL texture in the usual way. But do not call glTexImageXX functions!
    glGenTextures(1, &(userData->textureId));
    glBindTexture(eTexTarget, userData->textureId);
    glTexParameteri(eTexTarget, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(eTexTarget, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	/* [Create the 2D texture] */

	/* [Make the 2D texture the EGLImage] */
    /*
     * This is very important call, which makes the texture, which has been created above, to become EGLImage.
     * Since now on the buffer associated to the EGLImageKHR when updated, its content is immediately
     * visible on the texture.
     */
    _glEGLImageTargetTexture2DOES(eTexTarget, pEGLImage);

   glClearColor ( 0.0f, 0.0f, 0.0f, 0.0f );
   return GL_TRUE;
}

///
// Draw a triangle using the shader pair created in Init()
//
void Draw ( ESContext *esContext )
{
   UserData *userData = esContext->userData;
   GLfloat vVertices[] = { -0.5f,  0.5f, 0.0f,  // Position 0
                            0.0f,  0.0f,        // TexCoord 0 
                           -0.5f, -0.5f, 0.0f,  // Position 1
                            0.0f,  1.0f,        // TexCoord 1
                            0.5f, -0.5f, 0.0f,  // Position 2
                            1.0f,  1.0f,        // TexCoord 2
                            0.5f,  0.5f, 0.0f,  // Position 3
                            1.0f,  0.0f         // TexCoord 3
                         };
   GLushort indices[] = { 0, 1, 2, 0, 2, 3 };
      
   // Set the viewport
   glViewport ( 0, 0, esContext->width, esContext->height );
   
   // Clear the color buffer
   glClear ( GL_COLOR_BUFFER_BIT );

   // Use the program object
   glUseProgram ( userData->programObject );

   // Load the vertex position
   glVertexAttribPointer ( userData->positionLoc, 3, GL_FLOAT, 
                           GL_FALSE, 5 * sizeof(GLfloat), vVertices );
   // Load the texture coordinate
   glVertexAttribPointer ( userData->texCoordLoc, 2, GL_FLOAT,
                           GL_FALSE, 5 * sizeof(GLfloat), &vVertices[3] );

   glEnableVertexAttribArray ( userData->positionLoc );
   glEnableVertexAttribArray ( userData->texCoordLoc );

   // Bind the texture
   glActiveTexture ( GL_TEXTURE0 );
   glBindTexture ( GL_TEXTURE_2D, userData->textureId );

   // Set the sampler texture unit to 0
   glUniform1i ( userData->samplerLoc, 0 );

   glDrawElements ( GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices );

}

///
// Cleanup
//
void ShutDown ( ESContext *esContext )
{
   UserData *userData = esContext->userData;

   // Delete texture object
   glDeleteTextures ( 1, &userData->textureId );

   // Delete program object
   glDeleteProgram ( userData->programObject );
	
   free(esContext->userData);
}

int main ( int argc, char *argv[] )
{
   ESContext esContext;
   UserData  userData;

   esInitContext ( &esContext );
   esContext.userData = &userData;

   esCreateWindow ( &esContext, "Simple Texture 2D", 320, 240, ES_WINDOW_RGB );

   if ( !Init ( &esContext ) )
      return 0;

   esRegisterDrawFunc ( &esContext, Draw );

   esMainLoop ( &esContext );

   ShutDown ( &esContext );
}
