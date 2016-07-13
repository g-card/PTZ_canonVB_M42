#include <sys/time.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <ros/ros.h>
#include <sensor_msgs/Image.h>
#include <image_transport/image_transport.h>
#include <dynamic_reconfigure/server.h>
#include <canon_vbm42/CanonParamsConfig.h>
#include <canon_vbm42/PTZ.h>
#include <string>
#include <vector>
#include "libCanon/CanonDriver.h"
#include <opencv2/opencv.hpp>
#include <cv_bridge/cv_bridge.h>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include "capteurs.h"
#include "homog.cpp"

using namespace std;
using namespace boost;
using namespace std;
using namespace cv;


canon_vbm42::CanonParamsConfig currentConfig;
boost::recursive_mutex param_mutex;
CanonDriver * driver = NULL;

void ptzCallback(canon_vbm42::PTZ ptz)
{
    cout<<"In callback"<<endl;
    bool pos_change = false;
    float current_pan,current_tilt,current_zoom;
    driver->getCurrentPos(&current_pan,&current_tilt,&current_zoom);
    if (ptz.pan != current_pan && ptz.tilt != current_tilt && ptz.zoom != current_zoom) {

	  driver->moveTo(ptz.pan,ptz.tilt,ptz.zoom);
    }
}
int main(int argc, char* argv[]) 
{
	if (argc < 2) {
		return 1;
	}

    currentConfig.hostname = argv[1];
    currentConfig.subsampling = 0;
    ros::init(argc, argv, "canon_vbm42");
    ros::NodeHandle nh("~");
    Capteurs Capteurs(nh);
    ros::Subscriber ptzsub=nh.subscribe<canon_vbm42::PTZ>("/cmd_camera/cmd_ptz",1,ptzCallback);
    ros::Publisher ptzpub = nh.advertise<canon_vbm42::PTZ>("ptz_pos", 1);
    image_transport::ImageTransport it(nh);
    image_transport::Publisher image_pub = it.advertise("image",1);
    int capt;


    char* loginf = NULL;
    if (argc >= 3) {
	    loginf = argv[2];
    }
    float p=11,t=-20,z=41;
    float valp,valt,valz;
    /**** Initialising Canon Driver *****/
    CanonDriver canon(currentConfig.hostname.c_str());
    driver = &canon;
	 if (!driver->connect()) {
		    return 1;
	    }
    driver->moveTo(p,t,z);//centering
    cout<<"Init done"<<endl;
    const std::string videoStreamAddress = "http://"+lexical_cast <string>(argv[1])+"/-wvhttp-01-/video.cgi?.mjpg"; 
    cv::VideoCapture cap(videoStreamAddress);
    if(!cap.isOpened()){
      //error in opening the video input
      cout << "Unable to open video file: " << videoStreamAddress << endl;
      return -1;
    }
    cout<<"Video stream opened"<<endl;
    Mat frame;
    sensor_msgs::ImagePtr msg;
    driver->getCurrentPos(&p,&t,&z);
    currentConfig.pan_ang = p;
    currentConfig.tilt_ang = t;
    currentConfig.zoom_ang = z;
    ros::Rate loop_rate(20);
    //dynamic_reconfigure::Server<canon_vbm42::CanonParamsConfig> srv(param_mutex,nh);
    //dynamic_reconfigure::Server<canon_vbm42::CanonParamsConfig>::CallbackType f = boost::bind(&callback, _1, _2);
    //srv.setCallback(f);
    //srv.updateConfig(currentConfig);
    

    while (ros::ok()) {
	    
	canon_vbm42::PTZ ptz;
	driver->getCurrentPos(&p,&t,&z);
	//cout<<p<<","<<t<<","<<z<<endl;
	ptz.pan = p;
	ptz.tilt = t;
	ptz.zoom = z;
	ptzpub.publish(ptz);
	cap>>frame;
	if(!frame.empty()){
	    msg = cv_bridge::CvImage(std_msgs::Header(),"bgr8",frame).toImageMsg();
	    image_pub.publish(msg);
	}
	ros::spinOnce();
	loop_rate.sleep();
	
    }

    driver = NULL;

    
    printf("\n");
    return 0;
}
