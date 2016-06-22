/** \file
  Copyright (c) CSIRO ICT Robotics Centre
  \brief  View Video Stream using SDL - faster for Colour video 
  \author Cedric.Pradalier@csiro.au
  \warning This program requires SDL
 */

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
#include "CanonDriver.h"
#include <opencv2/opencv.hpp>
#include <cv_bridge/cv_bridge.h>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>



using namespace std;
using namespace boost;
// #include "canon_vbc50i/libCanon/CanonProjection.h"
using namespace std;
using namespace cv;

#if 0
#warning Saving path specific to eddie-ph
#define RECORDING_DEST "/scratch/stream"
#else
#define RECORDING_DEST "/tmp"
#endif

canon_vbm42::CanonParamsConfig currentConfig;
boost::recursive_mutex param_mutex;
CanonDriver * driver = NULL;

void callback(canon_vbm42::CanonParamsConfig &config, uint32_t level)
{
    cout<<"In callback"<<endl;
    if (config.night_mode != currentConfig.night_mode) {
		driver->setNightMode(config.night_mode);
    }
    /*if (config.fps != currentConfig.fps) {
		driver->setFrameRate(config.fps);
    }*/
    bool pos_change = false;
    if (config.pan_ang != currentConfig.pan_ang) {
        pos_change = true;
    }
    if (config.tilt_ang != currentConfig.tilt_ang) {
        pos_change = true;
    }
    if (config.zoom_ang != currentConfig.zoom_ang) {
        pos_change = true;
    }
	if (config.pan_speed != currentConfig.pan_speed) 
		driver->setPanSpeed(currentConfig.pan_ang,config.pan_speed);
	if (config.tilt_speed != currentConfig.tilt_speed) 
		driver->setTiltSpeed(currentConfig.tilt_ang,config.tilt_speed);
	if (config.zoom_speed != currentConfig.zoom_speed) 
		driver->setZoomSpeed(currentConfig.zoom_ang,config.zoom_speed);

    if (pos_change) {
        driver->moveTo(config.pan_ang,config.tilt_ang,config.zoom_ang);
	cout<<"Moving..."<<endl;
    }
#if 0

	// if ((local.width != buffer.width) || 
	// 		(local.height != buffer.height))
	// 	driver->setImageSize(local.width,local.height);

	if (config.focus_mode != currentConfig.focus_mode) {
        switch(config.focus_mode) {
            case 0:
                driver->setFocusMode(CanonDriver::FMAuto, CanonDriver::FMUndef);
                break;
            case 1:
                driver->setFocusMode(CanonDriver::FMManual, CanonDriver::FMManualFar);
                break;
            case 2:
                driver->setFocusMode(CanonDriver::FMManual, CanonDriver::FMManualNear);
                break;
            case 3:
                driver->setFocusMode(CanonDriver::FMAutoDomes, CanonDriver::FMUndef);
                break;
            case 4:
                driver->setFocusMode(CanonDriver::FMInfinity, CanonDriver::FMUndef);
                break;
        }
	}

    if (config.autoexp != currentConfig.autoexp) {
        driver->setAutoExposure("auto");
        printf("Switched to auto exposure\n");
	} else if ((config.aperture != currentConfig.aperture) ||
				(config.inv_shutter != currentConfig.inv_shutter) ||
				(config.gain != currentConfig.gain)) {
        driver->setExposureParameters(config.aperture,
                config.inv_shutter,config.gain);
        printf("Changed exposure parameters\n");
	}

	if (config.digital_zoom != currentConfig.digital_zoom) {
		driver->setDigitalZoom(config.digital_zoom);
	}
	if (config.pause != currentConfig.pause) {
		driver->setVideoReceptionPauseStatus(config.pause);
	}
	if (config.record != currentConfig.record) {
        driver->setRecordingDestination(config.record_dir.c_str());
		driver->setVideoRecordingMode(config.record);
		if (config.record) {
			fprintf(stderr,"Warning: video recording started\n");
		}
	}
    currentConfig = config;
#endif
}

int main(int argc, char* argv[]) 
{
	if (argc < 2) {
		return 1;
	}

    currentConfig.hostname = argv[1];
    currentConfig.subsampling = 0;
    //currentConfig.fps = 10;

    ros::init(argc, argv, "canon_vbm42");
    ros::NodeHandle nh("~");
    ros::Publisher ptzpub = nh.advertise<canon_vbm42::PTZ>("ptz", 1);
    image_transport::ImageTransport it(nh);
    image_transport::Publisher image_pub = it.advertise("image",1);
    int capt;


    char* loginf = NULL;
    if (argc >= 3) {
	    loginf = argv[2];
    }
    double p=11.08,t=-20.07,z=41;
    /**** Initialising Canon Driver *****/
    CanonDriver canon(currentConfig.hostname.c_str());
    driver = &canon;
	 if (!canon.connect()) {
		    return 1;
	    }
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
    canon.getCurrentPos(&p,&t,&z);
    currentConfig.pan_ang = p;
    currentConfig.tilt_ang = t;
    currentConfig.zoom_ang = z;
    dynamic_reconfigure::Server<canon_vbm42::CanonParamsConfig> srv(param_mutex,nh);
    dynamic_reconfigure::Server<canon_vbm42::CanonParamsConfig>::CallbackType f = boost::bind(&callback, _1, _2);
    srv.setCallback(f);
    srv.updateConfig(currentConfig);
    

    while (ros::ok()) {
	    
	canon_vbm42::PTZ ptz;
	canon.getCurrentPos(&p,&t,&z);
	ptz.pan = p;
	ptz.tilt = t;
	ptz.zoom = z;
	ptzpub.publish(ptz);

	switch(capt)
	{
	  case 1: canon.point(1);
	    break;
	  case 2: canon.point(2);
	    break;
	  case 4: canon.point(4);
	    break;
	  case 5: canon.point(5);
	    break;
	  default : cout<<"default"<<endl;
	}
	cap.read(frame);
	if(!frame.empty()){
	  //cout<<"envoi"<<endl;
	    msg = cv_bridge::CvImage(std_msgs::Header(),"bgr8",frame).toImageMsg();
	    image_pub.publish(msg);
	}
	ros::spinOnce();
	
    }

	    canon.disconnect();
    driver = NULL;

    
    printf("\n");
    return 0;
}
