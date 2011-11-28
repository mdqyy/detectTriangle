#include <engine.h>

Engine::Engine(Settings const & settings) :
m_settings(settings),
m_reInit(false),
m_initialized(false),
m_confidence(1.),
m_filterToggle(0)
{
	// Initialize the bounding box
	m_box.set = false;

	// Create the kinect device
    m_device.reset(&freenect.createDevice<KinectDevice>(0));
    m_device->startVideo();
    m_device->startDepth();

    // Initialize the dual frame and the summation frame
    m_out = Mat::zeros(Size(1280,480),CV_8UC3);
    if(!m_device->getVideo(m_rgb))
       m_rgb = Mat::zeros(Size(640,480),CV_8UC3);

    if(!m_device->getDepth(m_depthRaw))
    {
       m_depthRaw = Mat::zeros(Size(640,480),CV_16UC1);
       m_depth    = Mat::zeros(Size(640,480),CV_8UC3);
    } else m_device->depthViewColor(m_depth);

    // Create the detection
    m_detector.reset( new SlidingWindow(m_settings,m_rgb.size()) );
    //m_detector.reset( new Triangles(m_settings,m_rgb.size()));
    // Create the display module
    m_display.reset( new Display(1280,480) );

    update();

}

void Engine::setBox( BoundingBox const & box )
{
	m_box     = box;
	m_box.set = true;
	m_reInit  = true;
}
void Engine::setOutput()
{
    // copy into the output
    Rect leftROI(    Point(0,0),m_rgb.size());
    Rect rightROI( Point(640,0),m_rgb.size());
    Mat  leftSide  = m_out(leftROI);
    Mat  rightSide = m_out(rightROI);

    // Get the list of outputs and copy them onto the left side
    // Output the detections
    //Points::iterator contacts = m_events.begin();

    m_rgb.copyTo(leftSide);

    if( m_box.set )
    {
		cv::rectangle(leftSide,m_contact.position,m_contact.position + m_contact.dims,
				      Scalar(0,0,255),5);
    }

    switch(m_filterToggle){
        case DEPTH:
           m_depth.copyTo(rightSide);
           break;
        case CONTOURS:
           m_rgb.copyTo(rightSide);
           Filters::binaryFilter(rightSide,m_mask);
           rightSide.setTo(cv::Scalar(255,255,255),m_mask);
           break;
        case ORANGE:
           m_rgb.copyTo(rightSide);
           Filters::filterOrange(rightSide);
           break;
        default: break;
    }


}
void Engine::topTriangle()
{
   // Run the detector
   m_detector->processFrame(m_rgb,m_depthRaw);

   // Get the greatest detected patch confidence
   ContactList contacts = m_detector->getDetections();
   if( !contacts.empty() )
   {
      double maxScore = 0;
      size_t maxIdx   = 0;
      for( size_t idx = 0; idx < contacts.size(); ++idx)
      {
         if( contacts[idx].score > maxScore)
         {
            maxScore = contacts[idx].score;
            maxIdx   = idx;
         }
      }

      m_contact       = contacts[maxIdx];
      m_box.lowerLeft = m_contact.position;
      m_box.size      = m_contact.dims;
      m_box.set       = true;
      m_reInit        = true;
   }
}

void Engine::update()
{

    if(m_device->getVideo(m_rgb) && m_device->getDepth(m_depthRaw))
    {
       m_device->depthViewColor(m_depth);

       // Create a grayscale image
       cv::cvtColor(m_rgb,m_gray,CV_BGR2GRAY);
     //  if( !m_reInit && !m_initialized ) topTriangle();

       if( m_reInit )
       {
    	   tldInitialize();
       }
       else if( m_initialized )
       {
    	   tldUpdate();
       } else
       {
    	   m_tracker.setPrevious(m_gray);
       }


       setOutput();
    }
    m_display->update(m_out);
}
void Engine::tldInitialize()
{

   // Calculate the integral image
   cv::integral(m_gray,m_integral,CV_32F);

   // If we have already initialized, reset everything
   if( m_initialized ) m_detector->classifier()->restart();

   // Train the classifer on the bounding box and some warps of it
   m_detector->classifier()->train(m_integral,m_box.lowerLeft, m_box.size,1);
   m_detector->classifier()->warpTrainPositive(m_integral,m_box.lowerLeft,m_box.size);

   // Set the contact information
   m_contact.position = m_box.lowerLeft;
   m_contact.dims     = m_box.size;
   m_contact.score    = 1.;
   m_contact.valid    = true;
   m_confidence       = 1.;

   // Set the track box in the detector and train the negative patches
   m_detector->setTrackBox(m_contact);
   m_detector->trainNegative(m_integral);

   double score = m_detector->classifier()->classify(m_integral,
	                                                 m_contact.position,
		                                             m_contact.dims);
   std::cout << score << ",";
   std::cout << m_contact.position.x << ",";
   std::cout << m_contact.position.x << ",";
   std::cout << m_contact.dims.x << ",";
   std::cout << m_contact.dims.y << std::endl;

   // Set flags
   m_reInit = false;
   m_initialized = true;
}
void Engine::tldUpdate()
{
   // Calculate the integral image
   cv::integral(m_gray,m_integral,CV_32F);

   // Train the classifier based on user selection
   Contact newContact;

   if( m_confidence > 0.1)
   {
      // Run the tracker
      m_tracker.update(m_gray,m_contact,newContact);
      newContact = m_contact;
      // Determine the score for the new contact
      newContact.score = m_detector->classifier()->classify(m_integral,
	                                                        newContact.position,
		                                                    newContact.dims);
      std::cout << newContact.score << ",";
      std::cout << newContact.position.x << ",";
      std::cout << newContact.position.x << ",";
      std::cout << newContact.dims.x << ",";
      std::cout << newContact.dims.y << std::endl;
      m_detector->setTrackBox(newContact);
      m_detector->processFrame(m_integral,m_depthRaw);
      //m_detector->processFrame(m_rgb,m_depthRaw);
      m_detector->tracking(true);
      m_box.set = true;
   } else
   {
      m_detector->setTrackBox(newContact);
      m_detector->processFrame(m_integral,m_depthRaw);
      //m_detector->processFrame(m_rgb,m_depthRaw);
      m_detector->tracking(false);
      m_tracker.setPrevious(m_gray);
      newContact.position = cv::Point(0,0);
      newContact.dims     = cv::Point(0,0);
      newContact.score = 0.1;
      m_box.set = false;
   }

   // Get the greatest detected patch confidence
   ContactList contacts = m_detector->getDetections();
   double maxScore = 0;
   size_t maxIdx   = 0;
   for( size_t idx = 0; idx < contacts.size(); ++idx)
   {
#if 0
	  contacts[idx].score = m_detector->classifier()->classify(m_integral,
	   														   contacts[idx].position,
	   														   contacts[idx].dims);
#endif
      if( contacts[idx].score > maxScore)
      {
         maxScore = contacts[idx].score;
         maxIdx   = idx;
      }
   }

   // If we found a stronger detection, use it
   // If our original detection was valid, train with it
   if( maxScore > newContact.score && maxScore > 0.7)
   {
      newContact = contacts[maxIdx];
   }
   else if( newContact.score > maxScore && m_confidence > 0.8)
   {
      // Loop over the detections
      for( size_t idx = 0; idx < contacts.size(); ++idx)
      {
         if( contacts[idx].valid )
         {
            m_detector->classifier()->train(m_integral,
							                contacts[idx].position,
							                contacts[idx].dims,1);

         } else if ( !contacts[idx].valid && contacts[idx].score > 0.5)
         {

            m_detector->classifier()->train(m_integral,
							                contacts[idx].position,
							                contacts[idx].dims,0);

         }
      }
   }

   // set confidence
   m_confidence    = newContact.score;
   m_box.lowerLeft = newContact.position;
   m_box.size      = newContact.dims;
   m_contact       = newContact;

}
