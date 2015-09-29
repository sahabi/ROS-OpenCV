#include <ros/ros.h>
#include <image_transport/image_transport.h>
#include <opencv2/highgui/highgui.hpp>
#include "opencv2/imgproc/imgproc.hpp"
#include <cv_bridge/cv_bridge.h>
using namespace cv;
using namespace std;
int iLastX = -1; 
int iLastY = -1;
Mat imgLines5;
int ii = 0;
Mat thresh_callback(Mat imgThresholded, Mat imgTmp )
{
 vector<vector<Point> > contours;
 vector<Vec4i> hierarchy;
 //if (i == 0)
 //{
  Mat imgLines = Mat::zeros( imgTmp.size(), CV_8UC3 );

 //}
 findContours( imgThresholded, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0) );

  /// Approximate contours to polygons + get bounding rects and circles
  vector<vector<Point> > contours_poly( contours.size() );
  vector<Rect> boundRect( contours.size() );
  vector<Point2f>center( contours.size() );
  vector<float>radius( contours.size() );

  for( int i = 0; i < contours.size(); i++ )
     { approxPolyDP( Mat(contours[i]), contours_poly[i], 3, true );
       boundRect[i] = boundingRect( Mat(contours_poly[i]) );
     }
       //Mat drawing = Mat::zeros( imgThresholded.size(), CV_8UC3 );
  for( int i = 0; i< contours.size(); i++ )
     {
       Scalar color = Scalar(0,0, 255);
       rectangle( imgLines, boundRect[i].tl(), boundRect[i].br(), color, 2, 8, 0 );
     }
 if (ii == 0)
 {
  imgLines5 = Mat::zeros( imgTmp.size(), CV_8UC3 );
  ii = 1;
 }
  return imgLines;
}


void imageCallback(const sensor_msgs::ImageConstPtr& msg)
{
  Mat newimage = cv_bridge::toCvShare(msg, "bgr8")->image;
  int iLowH = 0;
  int iHighH = 40;

  int iLowS = 155; 
  int iHighS = 255;

  int iLowV = 106;
  int iHighV = 255;



  Mat imgHSV;

  cvtColor(newimage, imgHSV, COLOR_BGR2HSV); //Convert the captured frame from BGR to HSV
 
  Mat imgThresholded;
   Mat imgLines2;
   inRange(imgHSV, Scalar(iLowH, iLowS, iLowV), Scalar(iHighH, iHighS, iHighV), imgThresholded); //Threshold the image
      
  //morphological opening (removes small objects from the foreground)
  erode(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) );
  dilate( imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) ); 

   //morphological closing (removes small holes from the foreground)
  dilate(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) ); 
  erode(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) );

   //Calculate the moments of the thresholded image
  Moments oMoments = moments(imgThresholded);

   double dM01 = oMoments.m01;
  double dM10 = oMoments.m10;
  double dArea = oMoments.m00;

   // if the area <= 10000, I consider that the there are no object in the image and it's because of the noise, the area is not zero 
  if (dArea > 10000)
  {
   //calculate the position of the ball
   int posX = dM10 / dArea;
   int posY = dM01 / dArea;        
        
   if (iLastX >= 0 && iLastY >= 0 && posX >= 0 && posY >= 0)
   {
    //Draw a red line from the previous point to the current point
    line(imgLines5, Point(posX, posY), Point(iLastX, iLastY), Scalar(0,255,0), 2);
    //rectangle(imgLines, Point(posX, posY), Point(iLastX, iLastY), Scalar(0,0,255));
   }

  iLastX = posX;
  iLastY = posY;

  }
    imgLines2 = thresh_callback(imgThresholded,newimage);
    newimage = newimage + imgLines5 + imgLines2;

  try
  {
    imshow("view", newimage);
  }
  catch (cv_bridge::Exception& e)
  {
    ROS_ERROR("Could not convert from '%s' to 'bgr8'.", msg->encoding.c_str());
  }
}

int main(int argc, char **argv)
{
  ros::init(argc, argv, "image_listener");
  ros::NodeHandle nh;
  namedWindow("view");
  startWindowThread();
  image_transport::ImageTransport it(nh);
  image_transport::Subscriber sub = it.subscribe("camera/image", 1, imageCallback);
  ros::spin();
  destroyWindow("view");
}