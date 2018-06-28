/*
  Project: OpenCogER
  File: FingersCountTest.cpp
  Author: Dagim Sisay <dagiopia@gmail.com>
  License: AGPL
  Date: March, 2018
*/

#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <string>

#include "sense/vision/CamCapture.hpp"
#include "sense/vision/ITColor2Gray.hpp"
#include "sense/vision/ITEqualizeHist.hpp"
#include "sense/vision/ITDetectHand.hpp"
#include "sense/vision/FingersCount.hpp"

#define SCALE 0.1
   
int main(int argc, char **argv)
{    
    CamCapture cc("c", 320, 240, 0, 20);
    if( !cc.isOk()) { fprintf(stderr, "ERROR: Camera Opening : %s\n", 
                             cc.getState().c_str()); return -1; }
    ITColor2Gray c2g("c2g1");
    ITEqualizeHist eh("eh1");
    ITDetectHand dh("dh1");
    FingersCount fc(true);

    cv::Mat frame, img, img2;
    std::vector<cv::Rect> hands;

    while (true)
    {
        frame = cc.getCurrentFrame();
        img = eh.Transform(c2g.Transform(frame));
	cv::threshold(img, img, 70, 255, CV_THRESH_BINARY_INV | CV_THRESH_OTSU);
	hands = dh.Transform(img);
	for (size_t i = 0 ; i < hands.size(); i++)
	{
            img = cv::Mat(img, cv::Rect(hands[i].x - hands[i].x*SCALE,
	                                hands[i].y - hands[i].y*SCALE,
					hands[i].width + hands[i].width*SCALE,
					hands[i].height + hands[i].height*SCALE));
	    cv::putText(frame, std::to_string(fc.num_fingers(img)), cv::Point(20, 20), 
	              cv::FONT_HERSHEY_COMPLEX_SMALL, 1.5, CV_RGB(50, 50, 50), 2,
		      CV_AA);
	    cv::cvtColor(img, img2, COLOR_GRAY2BGR);
	    for(size_t j = 0 ; j < fc.f_tips.size(); j++)
	        cv::circle(img2, fc.f_tips[j], 2, CV_RGB(255, 0, 0), 2);
	    cv::imshow("hand", img2);
            
	}
	cv::imshow("Fingers Count", frame);
	if((int)'q' == cv::waitKey(10))
	    break;
    }// while true
    
    return 0;
}
