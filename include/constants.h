#ifndef __CONSTANTS_H__
#define __CONSTANTS_H__

#include <cv.h>
#include <vector>

struct BoundingBox
{
	cv::Point lowerLeft;
	cv::Point size;
	bool      set;
};

// Feature ordering
//	double area;
//	double perimeter;
//	double hue;
//	double saturation;
//	double value;
//	double angle;
typedef std::vector<double> Feature;

typedef std::vector<cv::Point2f> Points;
//
// The constants used to convert from pixel,depth to XYZ
// can be found at
// nicolas.burrus.name/index.php/Research/KinectCalibration
//
namespace Kinect
{

   // PI
   static const double  fPi    =  3.14159265;

   // Constants for RGB conversion
   static const double fx_rgb =  5.2921508098293293e+02;
   static const double fy_rgb =  5.2556393630057437e+02;
   static const double cx_rgb =  3.2894272028759258e+02;
   static const double cy_rgb =  2.6748068171871557e+02;
   static const double k1_rgb =  2.6451622333009589e-01;
   static const double k2_rgb = -8.3990749424620825e-01;
   static const double p1_rgb = -1.9922302173693159e-03;
   static const double p2_rgb =  1.4371995932897616e-03;
   static const double k3_rgb =  9.1192465078713847e-01;

   // Constants for depth conversion
   static const double fx_d   =  5.9421434211923247e+02;
   static const double fy_d   =  5.9104053696870778e+02;
   static const double cx_d   =  3.3930780975300314e+02;
   static const double cy_d   =  2.4273913761751615e+02;
   static const double k1_d   = -2.6386489753128833e-01;
   static const double k2_d   =  9.9966832163729757e-01;
   static const double p1_d   = -7.6275862143610667e-04;
   static const double p2_d   =  5.0350940090814270e-03;
   static const double k3_d   = -1.3053628089976321e+00;

}

#endif
