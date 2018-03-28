#include "vision.hpp"
#include "GripPipeline.h"
using namespace std;


//Set up constants
cv::RNG rng(12345);
cv::Scalar MY_RED (0, 0, 255);
cv::Scalar MY_BLUE (255, 0, 0);
cv::Scalar MY_GREEN (0, 255, 0);
cv::Scalar MY_PURPLE (255, 0, 255);
cv::Scalar GUIDE_DOT(255,255,0);
cv::Point TEST_POINT(120,120);

bool hasInit = false;
grip::GripPipeline pipeline;

//utility functions
void copyPointData (const cv::Point &pSource, cv::Point &pTarget) {
    pTarget.x = pSource.x;
    pTarget.y = pSource.y;
}

inline int getHue (cv::Mat &img, int r, int c) {
    return img.at<cv::Vec3b>(r, c)[0];
}

inline int getSat (cv::Mat &img, int r, int c) {
    return img.at<cv::Vec3b>(r, c)[1];
}

inline int getVal (cv::Mat &img, int r, int c) {
    return img.at<cv::Vec3b>(r, c)[2];
}

void drawPoint (cv::Mat &img, cv::Point &p, cv::Scalar &color) {
    cv::circle(img, p, 4, color, 4);
}

//checks for contour validity
bool is_valid (contour_type &contour) {
    bool valid = true; //start out assuming its valid, disprove this later

    //find bounding rect & convex hull
    cv::Rect rect = cv::boundingRect(contour);
    contour_type hull;
    cv::convexHull(contour, hull);

    double totalArea = (RES_X * RES_Y);

    //calculate relevant ratios & values
    double area = cv::contourArea(contour) / totalArea;
    //double perim = cv::arcLength(hull, true);

    double convex_area = cv::contourArea(hull) / totalArea;

    double width = rect.width, height = rect.height;

    double area_rat = area / convex_area;
    double rect_rat = height / width;

  //check ratios & values for validity
    if (area < MIN_AREA || area > MAX_AREA) valid = false;
    if (area_rat < MIN_AREA_RAT || area_rat > MAX_AREA_RAT) valid = false;
    if (rect_rat < MIN_RECT_RAT || rect_rat > MAX_RECT_RAT) valid = false;
    if (width < MIN_WIDTH || width > MAX_WIDTH) valid = false;
    if (height < MIN_HEIGHT || height > MAX_HEIGHT) valid = false;

    return valid;
}

void init() {
   pipeline = grip::GripPipeline();
}

VisionResultsPackage calculate(const cv::Mat &bgr, cv::Mat &processedImage){
   if(!hasInit) {
      init();
      hasInit = true;
   } 
   //ui64 time_began = millis_since_epoch();

    processedImage = bgr.clone();
    pipeline.Process(processedImage);
    vector<vector<cv::Point>> conts = *pipeline.GetFilterContoursOutput();
    cv::Rect biggestRect;
    double largestArea = -1;
    for(vector<cv::Point> cont : conts) {
         cv::Rect rect = cv::boundingRect( cv::Mat(cont));
         cv::rectangle(processedImage, rect.tl(), rect.br(), cv::Scalar(0, 255, 0), 2, 8, 0);
        if(rect.width * rect.height > largestArea) {
            largestArea = rect.width * rect.height;
            biggestRect = rect;
        }
    }

    VisionResultsPackage res;
    if(&biggestRect != NULL) {
        cv::rectangle(processedImage, biggestRect.tl(), biggestRect.br(), cv::Scalar(255, 0, 0));    
        res.width = biggestRect.width;
        res.height = biggestRect.height;
        res.x = biggestRect.x;
        res.y = biggestRect.y;
    } else {
       res.width = -1;
       res.height = -1;
       res.x = -1;
       res.y = -1;
    }
    
   


   // drawOnImage (processedImage, res);
    return res;
}

void drawOnImage (cv::Mat &img, VisionResultsPackage info) {
    //draw the 4 corners on the image
}

VisionResultsPackage processingFailurePackage(ui64 time) {
    VisionResultsPackage failureResult;
    failureResult.timestamp = time;
    

    return failureResult;
}
