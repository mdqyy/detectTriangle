#include <twobitbp.h>
#include <highgui.h>
TwoBitBP::TwoBitBP( cv::Point2f const & scale )
{
	// Generate randome scales between minScale and maxScale
	// This assumes the scale variable is ordered as
	// scale.x = minScale
	// scale.y = maxScale
	m_scale.x = (scale.y - scale.x)*static_cast<float>(rand()) /
			    static_cast<float>(RAND_MAX) + scale.x;
	m_scale.y = (scale.y - scale.x)*static_cast<float>(rand()) /
			    static_cast<float>(RAND_MAX) + scale.x;

	// Generate random position between 0 and width/heigh
	m_position.x = (1.0f - m_scale.x)*static_cast<float>(rand()) /
			       static_cast<float>(RAND_MAX);
	m_position.y = (1.0f - m_scale.y)*static_cast<float>(rand()) /
			       static_cast<float>(RAND_MAX);

}

float TwoBitBP::pixelSum(cv::Mat     const & image,
		                 cv::Point   const & pos,
		                 cv::Point   const & size)
{

	// Make sure we don't exceed the image size
	cv::Size imgSize = image.size() - cv::Size(1,1);
	if( pos.x < 0 || pos.x + size.x >= imgSize.width   ||
		pos.y < 0 || pos.y + size.y >= imgSize.height  ||
		size.x < 0 || size.y < 0 ) return 0.0f;

	// The input image should be an integral image so that
	// it's easy to calculate the summed area of four points
	// sum = I(x,y) + I(x+w,y+h) - I(x+w,y) - I(x,y+h)
	float iA = image.at<float>(pos.y,pos.x);
    float iB = image.at<float>(pos.y + size.y, pos.x + size.x);
    float iC = image.at<float>(pos.y, pos.x + size.x);
    float iD = image.at<float>(pos.y + size.y, pos.x);

    return iA + iB - iC - iD;
}



int TwoBitBP::test(cv::Mat   const & image,
		           cv::Point const & patchPt,
		           cv::Point const & patchDims)
{
	// Cast the input dimensions as floats
	cv::Point2f floatDims;
	floatDims.x = static_cast<float>(patchDims.x);
	floatDims.y = static_cast<float>(patchDims.y);

	// Compute the properties of the test rectangles relative to the
	// size and position of the patch
	cv::Point pos;
	cv::Point scale;
	pos.x   = static_cast<int>(m_position.x*floatDims.x) + patchPt.x + 1;
	pos.y   = static_cast<int>(m_position.y*floatDims.y) + patchPt.y;
	scale.x = static_cast<int>(m_scale.x*floatDims.x*0.5f);
	scale.y = static_cast<int>(m_scale.y*floatDims.y*0.5f);

	// Double the height
	scale.y *= 2;
	float left     = pixelSum(image,pos,scale);
	float right    = pixelSum(image,pos + cv::Point(scale.x,0),scale );

	// Double the width and undo the height scaling
	scale.x *= 2;
	scale.y /= 2;
	float top      = pixelSum(image,pos,scale);
	float bottom   = pixelSum(image,pos + cv::Point(0,scale.y),scale );

    return (left > right ? 0 : 2) + ( top > bottom ? 0 : 1);

}
