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

Mat img_out,H,obj,img_qr;
canon_vbm42::PTZ ptz;
Mat frame;
int flag=0;
int homog=0;
vector<int> nb_pieces; //orange,jaune
vector<vector<Point2f> > centre_pieces; //orange,jaune
void imageCallback(const sensor_msgs::ImageConstPtr& msg)
{
  try
  {
    switch(homog)
    {
      case 1:frame = cv_bridge::toCvCopy(msg, "bgr8")->image;
	homog_up(1,imread("/home/etudiant/catkin_ws/src/canon_vbm42/src/Poste_1_up.JPG"), frame, &img_out, &H);
	homog_qr(1,imread("/home/etudiant/catkin_ws/src/canon_vbm42/src/Poste_1.JPG"), frame, &img_qr);
	homog=0;
	flag=true;
	break;
      case 2:frame = cv_bridge::toCvCopy(msg, "bgr8")->image;
	homog_up(2,imread("/home/etudiant/catkin_ws/src/canon_vbm42/src/Poste_2_up.JPG"), frame, &img_out, &H);
	homog_qr(2,imread("/home/etudiant/catkin_ws/src/canon_vbm42/src/Poste_2.JPG"), frame, &img_qr);
	homog=0;
	flag=true;
	break;
      case 4:frame = cv_bridge::toCvCopy(msg, "bgr8")->image;
	homog_qr(2,imread("/home/etudiant/catkin_ws/src/canon_vbm42/src/Poste_4.JPG"), frame, &img_qr);
	homog=0;
	flag=true;
	break;
      case 5:frame = cv_bridge::toCvCopy(msg, "bgr8")->image;
	homog_qr(2,imread("/home/etudiant/catkin_ws/src/canon_vbm42/src/Poste_2.JPG"), frame, &img_qr);
	homog=0;
	flag=true;
	break;
      default:
	break;
    }
  }
  catch (cv_bridge::Exception& e)
  {
    ROS_ERROR("Could not convert from '%s' to 'bgr8'.", msg->encoding.c_str());
  }
}


void point(int choix, float *p, float *t, float *z)
{
  FILE *cmd_file;
  int num_poste = 0;
  float coord[5][3];
  float pan, tilt, zoom;

  cmd_file = fopen("/home/etudiant/catkin_ws/src/canon_vbm42/src/node/pointage.txt","r");
  if(cmd_file == NULL)
  {
    perror("Error");
  }
  for(int i = 0; i < 4; i++)
  {
    fscanf(cmd_file,"%d:\n",&num_poste);
    fscanf(cmd_file,"P:%f\nT:%f\nZ:%f\n",&pan,&tilt,&zoom);
    coord[num_poste][1] = pan;
    coord[num_poste][2] = tilt;
    coord[num_poste][3] = zoom;
  }
  *p = coord[choix][1];
  *t = coord[choix][2];
  *z = coord[choix][3];
  if(fclose(cmd_file) == 0)
    cout << "Param. lu " <<*p<<", "<<*t<<", "<<*z<< endl;
}

 int main(int argc, char* argv[]) 
{
    bool PSx[25],DxD[13],DxG[13],CPx[11],CPIx[9];
    	for(int i=0;i<25;i++) PSx[i]=0;
	for(int i=0;i<13;i++) DxD[i]=0;
	for(int i=0;i<13;i++) DxG[i]=0;
	for(int i=0;i<11;i++) CPx[i]=0;
	for(int i=0;i<9;i++) CPIx[i]=0;
    bool center=0,p2=0,p1=0,p4=0,p5=0; //flag poste ou centre
    ros::init(argc, argv, "cmd_camera");
    ros::NodeHandle nh("~");
    Capteurs Capteurs(nh);
    ros::Publisher ptzpub = nh.advertise<canon_vbm42::PTZ>("cmd_ptz", 1);
    image_transport::ImageTransport it(nh);
    image_transport::Subscriber sub = it.subscribe("/canon_vbm42/image", 1, imageCallback);

    int capt;
    center=1;
    
    ros::Rate loop_rate(10);

    int tot_pieces;
    while (ros::ok()) {
      
      Capteurs.Actualiser(PSx,DxD,DxG,CPx,CPIx);
      if(PSx[21]&&(!p2))
	{
	  cout<<"Pointage 2"<<endl;
	  point(2,&ptz.pan,&ptz.tilt,&ptz.zoom);
	  ptzpub.publish(ptz);sleep(4);
	  homog=2;
	  center=0;
	  p2=1;
	}else{
	  if(PSx[3]&&(!p1)){
	    cout<<"Pointage 1"<<endl;
	    point(1,&ptz.pan,&ptz.tilt,&ptz.zoom);
	    ptzpub.publish(ptz);sleep(4);
	    homog=1;
	    center=0;
	    p1=1;
	  }else{
	    if(PSx[9]&&(!p4)){
	      cout<<"Pointage 4"<<endl;
	      point(4,&ptz.pan,&ptz.tilt,&ptz.zoom);
	      ptzpub.publish(ptz);
	      center=0;
	      p4=1;
	    }
	    else{
	      if(PSx[15]&&(!p5)){
		cout<<"Pointage 5"<<endl;
		point(5,&ptz.pan,&ptz.tilt,&ptz.zoom);
		ptzpub.publish(ptz);
		center=0;
		p5=1;
	      }
	      if(!center&& ((p5&&PSx[17]) || (p4&&PSx[12]) || (p2&&PSx[24]) || (p1&&PSx[5]) ) ){
		cout<<"Pointage centre"<<endl;
		ptz.pan=11;
		ptz.tilt=-20;
		ptz.zoom=41;
		ptzpub.publish(ptz);
		p5=0;p4=0;p2=0;p1=0;
		center=1;
	      }
	    }
	  }
	}
	if(flag)
	{
	  tot_pieces = 0;
	  tot_pieces = pieces_sur_nav(&centre_pieces,img_out,&nb_pieces);
	  cout<<tot_pieces<<" pieces sur la navette"<<endl;
	  int nb_qr = read_QR(img_qr);
	  if(nb_qr==0)
	  {
	    cout<<"No QR code found"<<endl;
	  }
	  flag=0;
	}
	
	  
	ros::spinOnce();
	loop_rate.sleep();
	
    }
    
    printf("\n");
    return 0;
}
