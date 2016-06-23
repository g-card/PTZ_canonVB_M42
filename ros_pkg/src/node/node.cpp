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


using namespace std;
using namespace boost;
using namespace std;
using namespace cv;


canon_vbm42::CanonParamsConfig currentConfig;
boost::recursive_mutex param_mutex;
CanonDriver * driver = NULL;
/*
void callback(canon_vbm42::CanonParamsConfig &config, uint32_t level)
{
    cout<<"In callback"<<endl;
    if (config.night_mode != currentConfig.night_mode) {
		driver->setNightMode(config.night_mode);
    }
    /*if (config.fps != currentConfig.fps) {
		driver->setFrameRate(config.fps);
    }
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
        //driver->moveTo(config.pan_ang,config.tilt_ang,config.zoom_ang);
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
}*/
 main(int argc, char* argv[]) 
{
	if (argc < 2) {
		return 1;
	}
    bool PSx[25],DxD[13],DxG[13],CPx[11],CPIx[9];
    	for(int i=0;i<25;i++) PSx[i]=0;
	for(int i=0;i<13;i++) DxD[i]=0;
	for(int i=0;i<13;i++) DxG[i]=0;
	for(int i=0;i<11;i++) CPx[i]=0;
	for(int i=0;i<9;i++) CPIx[i]=0;
    bool center=0,p2=0,p1=0,p4=0,p5=0;
    currentConfig.hostname = argv[1];
    currentConfig.subsampling = 0;
    //currentConfig.fps = 10;
    cout<<get_selfpath()<<endl;
    ros::init(argc, argv, "canon_vbm42");
    ros::NodeHandle nh("~");
    Capteurs Capteurs(nh);
    ros::Publisher ptzpub = nh.advertise<canon_vbm42::PTZ>("ptz", 1);
    image_transport::ImageTransport it(nh);
    image_transport::Publisher image_pub = it.advertise("image",1);
    int capt;


    char* loginf = NULL;
    if (argc >= 3) {
	    loginf = argv[2];
    }
    double p=11.08,t=-20.07,z=41;
    float valp,valt,valz;
    /**** Initialising Canon Driver *****/
    CanonDriver canon(currentConfig.hostname.c_str());
    driver = &canon;
	 if (!canon.connect()) {
		    return 1;
	    }
    driver->moveTo(16.6,-27.49,60.29);//centering
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
    ros::Rate loop_rate(5);
    //dynamic_reconfigure::Server<canon_vbm42::CanonParamsConfig> srv(param_mutex,nh);
    //dynamic_reconfigure::Server<canon_vbm42::CanonParamsConfig>::CallbackType f = boost::bind(&callback, _1, _2);
    //srv.setCallback(f);
    //srv.updateConfig(currentConfig);
    

    while (ros::ok()) {
	    
	canon_vbm42::PTZ ptz;
	canon.getCurrentPos(&p,&t,&z);
	cout<<p<<","<<t<<","<<z<<endl;
	ptz.pan = p;
	ptz.tilt = t;
	ptz.zoom = z;
	ptzpub.publish(ptz);
	Capteurs.Actualiser(PSx,DxD,DxG,CPx,CPIx);
	cap.read(frame);
	if(PSx[22])
	{
	  driver->point(2,&valp,&valt,&valz);cout<<"Pointing to 2..."<<endl;
	  driver->moveTo(valp,valt,valz);
	  center=0;p2=1;
	}
	else{
	  if(PSx[2]){
	    driver->point(1,&valp,&valt,&valz);cout<<"Pointing to 1..."<<endl;
	    driver->moveTo(valp,valt,valz);
	    center=0;p1=1;
	  }
	  else{
	    if(PSx[9]){
	      driver->point(4,&valp,&valt,&valz);cout<<"Pointing to 4..."<<endl;
	      driver->moveTo(valp,valt,valz);
	      center=0;p4=1;
	    }
	    else{
	      if(PSx[15]){
		driver->point(5,&valp,&valt,&valz);cout<<"Pointing to 5..."<<endl;
		driver->moveTo(valp,valt,valz);
		center=0;p5=1;
	      }
	      else
	      {
		if(center==0 &&( (PSx[24]&&p2)|| (PSx[5]&&p1)|| (PSx[12]&&p4)|| (PSx[17]&&p5))){
		  driver->moveTo(16.6,-27.49,60.29);//centering
		  center=1;
		}
	      }
	    }
	  }
	}
	if(!frame.empty()){
	  //cout<<"envoi"<<endl;
	    msg = cv_bridge::CvImage(std_msgs::Header(),"bgr8",frame).toImageMsg();
	    image_pub.publish(msg);
	}
	ros::spinOnce();
	loop_rate.sleep();
	
    }

    canon.disconnect();
    driver = NULL;

    
    printf("\n");
    return 0;
}
