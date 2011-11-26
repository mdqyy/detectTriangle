#include <include/engine.h>
#include <include/settings.h>
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
Engine     *g_engine;
double      freenect_angle=0;
int         filter=0;
BoundingBox box;

// GLUT display function
void display()
{
    g_engine->update();
    glutSwapBuffers();
 
}

// GLUT Idle function
void idle(void) {
    glutPostRedisplay();
}
void mouse( int button, int state, int x, int y)
{
	if( button == GLUT_LEFT_BUTTON )
	{
		if( state == GLUT_DOWN )
		{
			box.set = false;
			if( x < 640 && y < 480 )
			{
				box.lowerLeft.x = x;
			    box.lowerLeft.y = 480 - y;
			    box.set = true;
			} // Only accept points on the left
		} else
		{
			if( x < 640 && y < 480  && box.set )
			{
				box.size.x = x - box.lowerLeft.x;
				box.size.y = (480 - y) - box.lowerLeft.y;

				// Swap points if negative
				if( box.size.x < 0)
				{
					box.size.x *= -1;
					box.lowerLeft.x = x;
				}
				if( box.size.y < 0)
				{
					box.size.y *= -1;
					box.lowerLeft.y = (480 - y);
				}

				g_engine->setBox(box);
			}
		}
	}
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
           g_engine->device()->setTiltDegrees(freenect_angle);
           break; 
        case 'x': 
           freenect_angle--;
           if (freenect_angle < -30){
              freenect_angle = -30;
           }
           g_engine->device()->setTiltDegrees(freenect_angle);
           break;
        case 's': 
           freenect_angle = 0;
           g_engine->device()->setTiltDegrees(freenect_angle);
           break;
        case 'f':
           if( filter == 2 ) filter = -1;
           g_engine->toggleFilter(++filter);
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

    if( argc < 1)
    {
   	   std::cout << "Usage: " << std::endl;
	   std::cout << "./orange filename" << std::endl;
	   return 0;
    }

    // Input file name
    std::string inputFile(argv[argc-1]);

    std::cout << " Running Triangle Detection " << std::endl;
    std::cout << " Processing Settings File: " << inputFile << std::endl;

    Settings inputSettings(inputFile);
    if( !inputSettings.readSettings() ) return -1;
    //Initialize OpenGl
    initGL(&argc, argv);
  
    glutIdleFunc(idle);
    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutMouseFunc(mouse);
    glutReshapeFunc(reshape);

    initialize();
    g_engine = new Engine(inputSettings);

    glutMainLoop();

    // Shut down
    return 0;
}
