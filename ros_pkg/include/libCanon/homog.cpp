#include <stdio.h>
#include <iostream>
#include "opencv2/core.hpp"
#include "opencv2/features2d.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/calib3d.hpp"
#include "opencv2/xfeatures2d.hpp"
#include "opencv2/imgproc.hpp"
#include <boost/lexical_cast.hpp>
#include <vector>

using namespace cv;
using namespace xfeatures2d;
using namespace std;

RNG rng(12345);

int homog_up(int num_poste, Mat img_object,Mat img, Mat* img_out,  Mat *homog)
{
  if( !img_object.data || !img.data )
  { std::cout<< " --(!) Error reading images " << std::endl; return -1; }
  vector<Point2f> obj_corners;
  vector<Point2f> scene_corners;
  Mat img_object1;
  resize(img_object,img_object1,Size(1280,720));

  switch(num_poste)
  {
    case 1: obj_corners.push_back(Point(458,109));
	    obj_corners.push_back(Point(455,623));
	    obj_corners.push_back(Point(797,619));
	    obj_corners.push_back(Point(795,116));
	    scene_corners.push_back(Point(468,304-110));//Up left corner
	    scene_corners.push_back(Point(490,489-110));//Down left corner
	    scene_corners.push_back(Point(873,482-110));//Down right corner
	    scene_corners.push_back(Point(836,296-110));//Up right corner
	    break;
    case 2: obj_corners.push_back(Point(503,126));//Up left corner
	    obj_corners.push_back(Point(503,550));//Down left corner
	    obj_corners.push_back(Point(783,552));//Down right corner
	    obj_corners.push_back(Point(783,122));//Up right corner
	    scene_corners.push_back(Point(360,383-110));//Up left corner
	    scene_corners.push_back(Point(617,603-110));//Down left corner
	    scene_corners.push_back(Point(1070,548-110));//Down right corner
	    scene_corners.push_back(Point(802,337-110));//Up right corner
	    break;
    case 4: /*obj_corners.push_back(Point(x,y));
	    obj_corners.push_back(Point(x,y));
	    obj_corners.push_back(Point(x,y));
	    obj_corners.push_back(Point(x,y));*/
	    break;
    case 5: /*obj_corners.push_back(Point(x,y));
	    obj_corners.push_back(Point(x,y));
	    obj_corners.push_back(Point(x,y));
	    obj_corners.push_back(Point(x,y));*/
	    break;
    default: cout<<"Bad argument"<<endl;
	      return -1;
  }
  //cout<<"Debug:"<<obj_corners<<scene_corners<<endl;
  Mat H = findHomography( obj_corners, scene_corners, RANSAC );

  warpPerspective(img,*img_out,H.inv()/H.at<double>(2,2),Size(img_object1.cols,img_object1.rows));
  *homog = H.inv();
  imshow( "Homog poste ", *img_out );
  waitKey(0);
  return 0;
  }


int detect_piece(Scalar low_hsv, Scalar up_hsv,/*vector<Point2f> *center,*/Mat src)
{
  Mat threshold_output,src_hsv;
  vector<vector<Point> > contours;
  vector<Vec4i> hierarchy;
  
  /// Detect edges using Threshold
  cvtColor(src, src_hsv, COLOR_BGR2HSV);
  inRange(src_hsv, low_hsv, up_hsv, threshold_output);
  
  //morphological opening (remove small objects from the foreground)
  erode(threshold_output, threshold_output, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) );
  dilate( threshold_output, threshold_output, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) );
  
  //morphological closing (fill small holes in the foreground)
  dilate( threshold_output, threshold_output, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) ); 
  erode(threshold_output, threshold_output, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) );
  
  //imshow("thresh",threshold_output); //debug
  waitKey(0);
  /// Find contours
  findContours( threshold_output, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE);
  if(contours.size()>0){
    /// Approximate contours to polygons + get bounding rects and circles
    vector<vector<Point> > contours_poly( contours.size() );
    vector<RotatedRect> boundRect;
    vector<vector<Point> > contours_poly2;
    Moments m;
    for( int i = 0; i < contours.size(); i++ )
      { approxPolyDP( Mat(contours[i]), contours_poly[i], 3, true );
	contours_poly2.push_back(contours_poly[i]);
	boundRect.push_back(minAreaRect( Mat(contours_poly2.back()) ));
	if(((boundRect.back().size.area())<200)||
	  ((boundRect.back().size.area())>800)||
	    ((boundRect.back().size.height/boundRect.back().size.width)>1.6)||
	  ((boundRect.back().size.height/boundRect.back().size.width)<0.6))
	  {
	  boundRect.pop_back();
	  contours_poly2.pop_back();
	  }
      }
    /// Draw polygonal contour + bonding rects + circles
    Mat drawing = Mat::zeros( threshold_output.size(), CV_8UC3 );
    for( int i = 0; i< boundRect.size(); i++ )
      {
	Scalar color = Scalar( rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255) );
	drawContours( drawing, contours_poly2, i, color, 1, 8, vector<Vec4i>(), 0, Point() );
	rectangle( drawing, boundRect[i].boundingRect(), color, 2, 8, 0 );
	//cout<<"avant"<<boundRect[i].center<<endl;
	//(*center).push_back(boundRect[i].center);
	//cout<<"ici"<<endl;
      }
      //imshow("contours",drawing); //debug
    return boundRect.size();
  }
  return 0;

}  

int pieces_sur_nav(/*vector<Point2f> *center_jaune,vector<Point2f> *center_orange,*/Mat src/*, int &nb_or, int &nb_jau*/)
{
  Scalar jau_low = Scalar(20, 100, 150);
  Scalar jau_high = Scalar(100, 180, 255);
  Scalar or_low = Scalar(0, 120, 200);
  Scalar or_high = Scalar(30, 255, 255);
  
  cout<<detect_piece(Scalar(0, 120, 200),Scalar(20, 255, 255),/*center_orange,*/src)<<" piece(s) orange"<<endl; //orange
  cout<<detect_piece(Scalar(20, 70, 180),Scalar(120, 150, 255),/*center_jaune,*/src)<<" piece(s) jaune"<<endl; //jaune
  return(0);
  
}
  