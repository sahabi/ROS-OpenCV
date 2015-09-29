#include <ros/ros.h>
#include <image_transport/image_transport.h>
#include <opencv2/highgui/highgui.hpp>
#include <cv_bridge/cv_bridge.h>

int main(int argc, char **argv)
{
  ros::init(argc, argv, "image_publisher");
  ros::NodeHandle nh;
  image_transport::ImageTransport it(nh);
  image_transport::Publisher pub = it.advertise("camera/image", 1);
  cv::VideoCapture cap("/home/sahabi/Downloads/Animation.mp4");
  cv::Mat imgTmp;
  cap.read(imgTmp); 

  cv::waitKey(30);
      while (nh.ok())
      {
          cv::Mat imgOriginal;

          bool bSuccess = cap.read(imgOriginal); // read a new frame from video

           if (!bSuccess) //if not success, break loop
          {
               std::cout << "Cannot read a frame from video stream" << std::endl;
               break;
          }

          sensor_msgs::ImagePtr msg = cv_bridge::CvImage(std_msgs::Header(), "bgr8", imgOriginal).toImageMsg();

          ros::Rate loop_rate(5);
          pub.publish(msg);
          ros::spinOnce();
          loop_rate.sleep();
    
  }
}

