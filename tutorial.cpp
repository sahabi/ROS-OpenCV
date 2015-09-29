#include <iostream>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <stdio.h>
#include <stdlib.h>
using namespace cv;
using namespace std;
RNG rng(12345);

Mat thresh_callback(Mat imgThresholded, Mat imgTmp )
{
 vector<vector<Point> > contours;
 vector<Vec4i> hierarchy;
 Mat imgLines = Mat::zeros( imgTmp.size(), CV_8UC3 );;
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

  /// Show in a window
  namedWindow( "Contours", CV_WINDOW_AUTOSIZE );
  imshow( "Contours", imgLines );
  return imgLines;
}


 int main( int argc, char** argv )
 {
    VideoCapture cap("/home/sahabi/Downloads/Animation.mp4"); //capture the video from web cam

    if ( !cap.isOpened() )  // if not success, exit program
    {
         cout << "Cannot open the web cam" << endl;
         return -1;
    }

    namedWindow("Control", CV_WINDOW_AUTOSIZE); //create a window called "Control"

  int iLowH = 0;
 int iHighH = 40;

  int iLowS = 155; 
 int iHighS = 255;

  int iLowV = 106;
 int iHighV = 255;

  //Create trackbars in "Control" window
 createTrackbar("LowH", "Control", &iLowH, 179); //Hue (0 - 179)
 createTrackbar("HighH", "Control", &iHighH, 179);

  createTrackbar("LowS", "Control", &iLowS, 255); //Saturation (0 - 255)
 createTrackbar("HighS", "Control", &iHighS, 255);

  createTrackbar("LowV", "Control", &iLowV, 255);//Value (0 - 255)
 createTrackbar("HighV", "Control", &iHighV, 255);

  int iLastX = -1; 
 int iLastY = -1;

  //Capture a temporary image from the camera
 Mat imgTmp;
 cap.read(imgTmp); 

  //Create a black image with the size as the camera output
 Mat imgLines = Mat::zeros( imgTmp.size(), CV_8UC3 );
 waitKey(1000);
    while (true)
    {
        Mat imgOriginal;

        bool bSuccess = cap.read(imgOriginal); // read a new frame from video

         if (!bSuccess) //if not success, break loop
        {
             cout << "Cannot read a frame from video stream" << endl;
             break;
        }

    Mat imgHSV;

   cvtColor(imgOriginal, imgHSV, COLOR_BGR2HSV); //Convert the captured frame from BGR to HSV
 
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
    line(imgLines, Point(posX, posY), Point(iLastX, iLastY), Scalar(0,255,0), 2);
    //rectangle(imgLines, Point(posX, posY), Point(iLastX, iLastY), Scalar(0,0,255));
   }

    iLastX = posX;
   iLastY = posY;
  }

   imshow("Thresholded Image", imgThresholded); //show the thresholded image
  imgLines2 = thresh_callback(imgThresholded,imgTmp);
   imgOriginal = imgOriginal + imgLines + imgLines2;
  imshow("Original", imgOriginal); //show the original image

        if (waitKey(100) == 27) //wait for 'esc' key press for 30ms. If 'esc' key is pressed, break loop
       {
            cout << "esc key is pressed by user" << endl;
            break; 
       }
       
    }

  waitKey(0);
  return 0;
}


/*

  /// Detect edges using Threshold



  /// Draw polygonal contour + bonding rects + circles

}



*/