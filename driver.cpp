#include "driver.h"

driver::driver(int const & w, int const & h)
    : GLSobel(w,h),
      m_frameCount(0),
      m_filterOrange(false),
      m_showDepth(false),
      m_findTriangles(false)
{
    
    // Start the freenect device
    device.reset(&freenect.createDevice<Triangles>(0));
    device->startVideo();
    device->startDepth();

    // Initialize the dual frame and the summation frame
    dualFrame = Mat::zeros(Size(2*m_iWidth,m_iHeight),CV_8UC3);
    sumFrame = Mat::zeros(Size(m_iWidth,m_iHeight),CV_32FC3);
    if(!device->getVideo(rgbFrame))
    {
       rgbFrame = Mat::zeros(Size(m_iWidth,m_iHeight),CV_8UC3);
    }
    rgbFrame.copyTo(frame);
    rgbFrame.copyTo(orangeFrame);

    // Create a 2d texture for the input
    createImgTexture();
        
    // Create a 2d texture for the output
    createOutTexture();

}

// Update
void driver::update()
{   
   // Get the RGB frame and copy to frame for processing
   if( m_findTriangles )
   { 
      // Scale the accumulated data
      double alpha = 1.0/static_cast<double>(FRAMES_PER_STACK);
      sumFrame.convertTo(frame,CV_8UC3,alpha);
  
      // Reset everything
      m_frameCount = 0;
      m_findTriangles = false;
      Mat zeroFrame = Mat::zeros(Size(m_iWidth,m_iHeight),CV_32FC3);
      zeroFrame.copyTo(sumFrame);

      // Get just the orange in HSV
      Filters::filterOrange(frame);

      // Run the edge detection
      shader();

      // Get the opencv frame
      runDetect();
   }
   showStatus();
   transferToTexture(dualFrame,m_outputTex);
   
}

void driver::accumulate()
{
   if( device->getVideo(rgbFrame) )
   {
      m_frameCount++;
      Mat floatFrame = Mat::zeros(Size(m_iWidth,m_iHeight),CV_32FC3);
      Mat tmpFrame;
      rgbFrame.copyTo(tmpFrame);
      tmpFrame.convertTo(floatFrame,CV_32FC3);
      cv::accumulate(floatFrame,sumFrame);
   }
   if( m_frameCount == FRAMES_PER_STACK ) m_findTriangles = true;
   update();
}

void driver::showStatus()
{
    // copy into the output
    Rect leftROI(    Point(0,0),frame.size());
    Rect rightROI( Point(640,0),frame.size());
    Mat  leftSide  = dualFrame(leftROI);
    Mat  rightSide = dualFrame(rightROI);
    rgbFrame.copyTo(leftSide);
    orangeFrame.copyTo(rightSide);

}

void driver::runDetect()
{
    // Set the frame we will be doing computations on
    device->setOwnMat(frame);  // This should be the sobel output

    // Initalize the orangeFrame
    rgbFrame.copyTo(orangeFrame);

    // Copy depth into orange, or orange into orange 
    if( m_showDepth ) device->depthViewColor(orangeFrame);
    if( m_filterOrange ) Filters::filterOrange(orangeFrame);

    // Run the detection process
    device->contourImg();
    vector<Point> cMass = device->getDetectCM();
  
    // Output the detections
    if( device->foundTarget() )
    {
       for(unsigned int dIdx = 1; dIdx < cMass.size(); dIdx++)
       {  
          circle(orangeFrame, cMass[dIdx], 60, Scalar(0,0,255),5);
       }
       circle(orangeFrame, cMass[0], 60, Scalar(255,0,0),5);
    }

}

// Create the input texture
void driver::createImgTexture()
{
    // create a texture
    glGenTextures(2,m_iTexture);
    setupTexture(m_iTexture[0], m_iWidth, m_iHeight); // Read texture
    transferToTexture(frame,m_iTexture[0]);
    setupTexture(m_iTexture[1], m_iWidth, m_iHeight); // Read texture
    transferToTexture(frame,m_iTexture[1]);

}
// Create the output texture
void driver::createOutTexture()
{
    // create a texture
    glGenTextures(1, &m_outputTex);
    setupTexture(m_outputTex, 2*m_iWidth, m_iHeight); // Read texture
    transferToTexture(dualFrame, m_outputTex);
}

// Set the kinect angle
void driver::setTilt(double &tiltAngle)
{
   device->setTiltDegrees(tiltAngle);
}

// Set the depth flag
void driver::setDepthFlag()
{
   m_showDepth = !m_showDepth;
   if( m_showDepth && m_filterOrange ) setOrangeFlag();
}

// Set the orange flag
void driver::setOrangeFlag()
{
   m_filterOrange = !m_filterOrange;
   if( m_showDepth && m_filterOrange ) setDepthFlag();
}

// Reset the flags
void driver::resetFlags()
{
   m_filterOrange = false;
   m_showDepth    = false;
}


