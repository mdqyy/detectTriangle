#ifndef __DETECTION_H__
#define __DETECTION_H__

#include <iostream>
#include <cv.h>
#include <constants.h>
#include <boost/shared_ptr.hpp>
#include <settings.h>
#include <vector>
#include <opencv2/highgui/highgui.hpp>
#include <classifier.h>
#include <list>
struct Contact
{
	cv::Point   position;
	cv::Point   dims;
	double      score;
	int         hits;
	int         misses;
	bool        valid;
};

typedef std::vector<Contact> ContactList;
typedef std::list<Contact>   TrackTable;

class Detection 
{
public:
   // Constructor
   Detection(Settings const & settings, cv::Size const & frameSize);

   // Destructor
   ~Detection(){}

   // Getter for the contact list
   ContactList const & getDetections() const { return m_list; }

   // Process a frame of data
   virtual void processFrame(cv::Mat const & rgb, cv::Mat const & depth)=0;

   ClassifierPtr const & classifier() const {return m_classifier;}

   void setTrackBox(Contact const & box);

   double overlap(Contact const & box);

   void trainNegative(cv::Mat const & integral);

   void tracking(bool const & tracking) {m_tracking = tracking;}


protected:
   Settings        m_settings;       // Detector settings
   ContactList     m_list;           // Contact List
   ClassifierPtr   m_classifier;     // Classifier
   Contact         m_track;          // Top detection

   bool            m_tracking;
   cv::Point       m_boxSize;
   cv::Point       m_size;
};

typedef boost::shared_ptr<Detection> DetectionPtr;
#endif
