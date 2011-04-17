#include "libfreenect.h"

#include "kinectdevice.h"
#include "driver.h"
#include "triangles.h"
#include <iostream>
#include <vector>
#include <cmath>
#include <pthread.h>
#include <cv.h>
#include <cxcore.h>
#include <highgui.h>

// OpenGL stuff
#include <GL/glew.h>
#include <GL/glut.h>


using namespace cv;
using namespace std;

// Global driver
driver *g_Driver;
double freenect_angle=0;

// GLUT display function
void display()
{
    g_Driver->update();  // update the scene and run the edge detect filter
    g_Driver->display(); // display the results
    glutSwapBuffers();
 
}

// GLUT Idle function
void idle(void) {
    glutPostRedisplay();
}

// Keyboard
void keyboard( unsigned char key, int /*x*/, int /*y*/) 
{
    switch(key){ 
        case 'w':
           freenect_angle++;
           if (freenect_angle > 30){
              freenect_angle = 30;
           }
           g_Driver->setTilt(freenect_angle);
           break; 
        case 'x': 
           freenect_angle--;
           if (freenect_angle < -30){
              freenect_angle = -30;
           }
           g_Driver->setTilt(freenect_angle);
           break;
        case 's': 
           freenect_angle = 0;
           g_Driver->setTilt(freenect_angle);
           break;
        case 'd':
           g_Driver->setDepthFlag();
           break;
        case 'f':
           g_Driver->setOrangeFlag();
           break;
        case 'e':
           g_Driver->resetFlags();
           break;
        case 27:
           exit (0);
	   break;
        default: break;
    }
    glutPostRedisplay();
}

// GLUT reshape function
void reshape(int w, int h)
{
    if (h == 0) h = 1;
    
    glViewport(0, 0, w, h);
    
    // GPGPU CONCEPT 3b: One-to-one Pixel to Texel Mapping: An Orthographic 
    //                   Projection.
    // This code sets the projection matrix to orthographic with a range of 
    // [-1,1] in the X and Y dimensions. This allows a trivial mapping of 
    // pixels to texels.
    glMatrixMode(GL_PROJECTION);    
    glLoadIdentity();               
    gluOrtho2D(-1, 1, -1, 1);       
    glMatrixMode(GL_MODELVIEW);     
    glLoadIdentity();               
}

void initialize()
{
   glewInit();

   // Ensure we have the necessary OpenGL Shading Language extensions.
   if (glewGetExtension("GL_ARB_fragment_shader")      != GL_TRUE ||
       glewGetExtension("GL_ARB_vertex_shader")        != GL_TRUE ||
       glewGetExtension("GL_ARB_shader_objects")       != GL_TRUE ||
       glewGetExtension("GL_ARB_shading_language_100") != GL_TRUE)
   {
       fprintf(stderr, "Driver does not support OpenGL Shading Language\n");
       exit(1);
   }

   g_Driver = new driver(640,480);
    

}

// initGL
void initGL( int *argc, char** argv)
{
    glutInit(argc,argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
    glutInitWindowSize(1280,480);
    glutCreateWindow("Orange Triangle Detection");

}

int main(int argc, char **argv) {


    //Initialize OpenGl
    initGL(&argc, argv);
  
    glutIdleFunc(idle);
    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutReshapeFunc(reshape);

    initialize();

    glutMainLoop();

    // Shut down
    return 0;
}
