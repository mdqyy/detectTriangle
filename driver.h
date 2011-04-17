#ifndef __DRIVER_H__
#define __DRIVER_H__

#include <cv.h>
#include <highgui.h>
#include "libfreenect.h"
#include "kinectdevice.h"
#include "triangles.h"
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#define GLEW_STATIC 1
#include <GL/glew.h>
#include <GL/glut.h>


using namespace cv;

// This class encapsulates all of the GPGPU functionality of the example.
class driver
{
public: // methods
    driver(int w, int h);

    ~driver()
    {
       device->stopVideo();
       device->stopDepth();
    };

    // Pulls a frame and runs it through GLUT
    void update();

    // Eventually this will do histogram equalization
    // At the moment it does a sobel filter 
    void rgbNorm();

    // Display the results
    void display();

    // Run the triangle detection portion of the code
    void runDetect();

    // Set the kinect tilt angle
    void setTilt(double &tiltAngle);

    // Set whether or not you want to see the depth field
    void setDepthFlag();

    // Create a texture for the image we want to process
    void createImgTexture();
    
    // Create a texture for the output image
    void createOutTexture();

    // Initialize the shader program
    void initShader();

    // Initialize the Frame Buffer Object
    void initFBO();  
   
    // Setup the texture
    void setupTexture(const GLuint texID, int width, int height);

    // Transfer the image to a texture
    void transferToTexture(Mat &input, GLuint texID);
    
    // Transfer to texture to an image
    void transferFromTexture(Mat& output);
  
protected: // data
    // Note: inconsistent naming conventions cause I'm lazy
    int                _iWidth;
    int                _iHeight;          // The dimensions of our array
    
    unsigned int       _outputTex;        // Output texture
    unsigned int       _iTexture[2];      // The texture used as a data array
                                          // 0 - read
                                          // 1 - write 
    GLhandleARB        _programObject;    // the program used to update
    GLhandleARB        _fragmentShader;

    GLint              _texUnit;          // a parameter to the fragment program

    GLuint             _fbo;              // Frame buffer object

    Freenect::Freenect freenect;          // Freenect device
    Triangles*         device;            // The kinect device and 
                                          // triangle detection object
    Mat                frame;             // The filtered frame
    Mat                bigFrame;          // The output image 
                                          // (left rgb, right out)
    Mat                rgbFrame;          // The RGB frame
    bool               m_filterOrange;    // Show depth or orange?
                                      
};
#endif

