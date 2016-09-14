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
#include <zbar.h>
#include <zbar/ImageScanner.h>

using namespace cv;
using namespace xfeatures2d;
using namespace std;
using namespace zbar;

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
	    scene_corners.push_back(Point(468,304-110));//top left corner
	    scene_corners.push_back(Point(490,489-110));//bottom left corner
	    scene_corners.push_back(Point(873,482-110));//bottom right corner
	    scene_corners.push_back(Point(836,296-110));//top right corner
	    break;
    case 2: obj_corners.push_back(Point(503,126));//top left corner
	    obj_corners.push_back(Point(503,550));//bottom left corner
	    obj_corners.push_back(Point(783,552));//bottom right corner
	    obj_corners.push_back(Point(783,122));//top right corner
	    scene_corners.push_back(Point(360,383-110));//top left corner
	    scene_corners.push_back(Point(617,603-110));//bottom left corner
	    scene_corners.push_back(Point(1070,548-110));//bottom right corner
	    scene_corners.push_back(Point(802,337-110));//top right corner
	    break;
    case 4: /*scene_corners.push_back(Point(467,111-45));
	    scene_corners.push_back(Point(539,470-45));
	    scene_corners.push_back(Point(836,423-45));
	    scene_corners.push_back(Point(744,71-45));*/
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
  //waitKey(0);
  return 0;
}


int homog_qr(int num_poste, Mat img_object,Mat img, Mat* img_out)
{
  if( !img_object.data || !img.data )
  { std::cout<< " --(!) Error reading images " << std::endl; return -1; }
  vector<Point2f> obj_corners;
  vector<Point2f> scene_corners;
  Mat img_object1;
  resize(img_object,img_object1,Size(1280,720));

  switch(num_poste)
  {
    case 1: obj_corners.push_back(Point(323,330));
	    obj_corners.push_back(Point(353,401));
	    obj_corners.push_back(Point(840,400));
	    obj_corners.push_back(Point(872,329));
	    scene_corners.push_back(Point(460,543));//Up left corner
	    scene_corners.push_back(Point(482,593));//Down left corner
	    scene_corners.push_back(Point(841,585));//Down right corner
	    scene_corners.push_back(Point(867,533));//Up right corner
	    break;
    case 2: obj_corners.push_back(Point(397,255));//Up left corner
	    obj_corners.push_back(Point(397,395));//Down left corner
	    obj_corners.push_back(Point(984,389));//Down right corner
	    obj_corners.push_back(Point(993,250));//Up right corner
	    scene_corners.push_back(Point(618,581));//Up left corner
	    scene_corners.push_back(Point(619,685));//Down left corner
	    scene_corners.push_back(Point(1056,634));//Down right corner
	    scene_corners.push_back(Point(1066,530));//Up right corner
	    break;
    case 4: obj_corners.push_back(Point(400,428));
	    obj_corners.push_back(Point(394,691));
	    obj_corners.push_back(Point(1078,697));
	    obj_corners.push_back(Point(1041,439));
	    scene_corners.push_back(Point(544,618));
	    scene_corners.push_back(Point(539,682));
	    scene_corners.push_back(Point(838,633));
	    scene_corners.push_back(Point(822,577));
	    break;
    case 5: obj_corners.push_back(Point(405,255));
	    obj_corners.push_back(Point(398,391));
	    obj_corners.push_back(Point(986,387));
	    obj_corners.push_back(Point(991,251));
	    scene_corners.push_back(Point(838,592));
	    scene_corners.push_back(Point(839,655));
	    scene_corners.push_back(Point(1027,478));
	    scene_corners.push_back(Point(1034,414));
	    break;
    default: cout<<"Bad argument"<<endl;
	      return -1;
  }
  //cout<<"Debug:"<<obj_corners<<scene_corners<<endl;
  Mat H = findHomography( obj_corners, scene_corners, RANSAC );

  warpPerspective(img,*img_out,H.inv()/H.at<double>(2,2),Size(img_object1.cols,img_object1.rows));
  //imshow( "Homog poste ", *img_out );
  //waitKey(0);
  return 0;
}

int detect_piece(Scalar low_hsv, Scalar up_hsv,vector<Point2f> *center,Mat src)
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
  
  imshow("thresh",threshold_output); //debug
  imwrite("filter.jpg",threshold_output);
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
	//cout<<boundRect.back().size.area()<<endl<<boundRect.back().size<<((double)boundRect.back().size.height)/(boundRect.back().size.width)<<endl;
	if(((boundRect.back().size.area())<240)||
	  ((boundRect.back().size.area())>1100)||
	    (((double)boundRect.back().size.height/boundRect.back().size.width)>1.5)||
	  (((double)boundRect.back().size.height/boundRect.back().size.width)<0.6))
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
	center->push_back(boundRect[i].center);
	//cout<<"ici"<<endl;
      }
      //imshow("contours",drawing); //debug
    return boundRect.size();
  }
  return 0;

}  

int pieces_sur_nav(vector<vector<Point2f> >*center,Mat src, vector<int> *nb_pie)
{
  Scalar jau_low = Scalar(35, 60, 110);
  Scalar jau_high = Scalar(60, 255, 255);
  Scalar or_low = Scalar(0, 80, 120);
  Scalar or_high = Scalar(20, 180, 255);
  Scalar ve_low = Scalar(60, 100, 170);
  Scalar ve_high = Scalar(90, 255, 255);
  Scalar bl_low = Scalar(80, 110, 190);
  Scalar bl_high = Scalar(110, 180, 255);
  Scalar ro_low = Scalar(160, 100, 100);
  Scalar ro_high = Scalar(180, 255, 255);
  Scalar vi_low = Scalar(105, 60, 110);
  Scalar vi_high = Scalar(135, 200, 255);
  vector<Point2f> center_j,center_o,center_b,center_v,center_r,center_vi;
  int sum=0;
  nb_pie->clear();
  center->clear();
  
  nb_pie->push_back(detect_piece(or_low,or_high,&center_o,src));
  cout<<nb_pie->back()<<" piece(s) orange"<<endl; //orange
  nb_pie->push_back(detect_piece(jau_low,jau_high,&center_j,src));
  cout<<nb_pie->back()<<" piece(s) jaune"<<endl; //jaune
  nb_pie->push_back(detect_piece(ve_low,ve_high,&center_v,src));
  cout<<nb_pie->back()<<" piece(s) verte"<<endl; //vert
  nb_pie->push_back(detect_piece(bl_low,bl_high,&center_b,src));
  cout<<nb_pie->back()<<" piece(s) bleue"<<endl; //bleu
  nb_pie->push_back(detect_piece(ro_low,ro_high,&center_r,src));
  cout<<nb_pie->back()<<" piece(s) rose"<<endl; //rose
  nb_pie->push_back(detect_piece(vi_low,vi_high,&center_vi,src));
  cout<<nb_pie->back()<<" piece(s) violette"<<endl; //violet
  for(vector<int>::iterator it = nb_pie->begin(); it != nb_pie->end(); ++it)
    sum += *it;
  
  center->push_back(center_o);
  center->push_back(center_j);
  center->push_back(center_v);
  center->push_back(center_b);
  center->push_back(center_r);
  center->push_back(center_vi);
  
  for(vector<vector<Point2f> >::iterator i = center->begin();i != center->end(); i++)
  {
    for(vector<Point2f>::iterator ite = i->begin(); ite != i->end(); ite++)
      circle(src,*ite,20,Scalar(40, 65, 70),2);
  }
  destroyWindow("Homog poste ");
  imshow("contours",src); //debug
  imwrite("contours.jpeg",src);
  waitKey(30);
  
  return(sum);
  
}

int read_QR(Mat src)
{
  Mat threshold_output,src_gray;
  vector<vector<Point> > contours;
  vector<Vec4i> hierarchy;

  cvtColor( src, src_gray, CV_BGR2GRAY );
  blur( src_gray, src_gray, Size(3,3) );
  /// Detect edges using Threshold
  threshold( src_gray, threshold_output, 200, 255, THRESH_BINARY );
  /// Find contours
  findContours( threshold_output, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0) );

  /// Approximate contours to polygons + get bounding rects and circles
  vector<vector<Point> > contours_poly( contours.size() );
  vector<Rect> boundRect( contours.size() );
  vector<Point2f>center( contours.size() );
  vector<float>radius( contours.size() );
  int max_area=0;
  int index_qr=0;
  for( int i = 0; i < contours.size(); i++ )
     { 
       approxPolyDP( Mat(contours[i]), contours_poly[i], 3, true );
       boundRect[i] = boundingRect( Mat(contours_poly[i]) );
       //cout<<boundRect[i].size()<<boundRect[i].area()<<" "<<((double)boundRect[i].size().width/boundRect[i].size().height)<<endl;
       if(boundRect[i].area()>max_area && 
	 ((double)boundRect[i].size().width/boundRect[i].size().height)<1.2 && 
	 ((double)boundRect[i].size().width/boundRect[i].size().height)>0.8)
       {
	 //cout<<boundRect[i].size()<<boundRect[i].area()<<endl;
	 max_area=boundRect[i].area();
	 index_qr=i;
       }
     }


  /// Draw polygonal contour + bonding rects (debug)
  /*Mat drawing = Mat::zeros( threshold_output.size(), CV_8UC3 );
  for( int i = 0; i< contours.size(); i++ )
     {
       Scalar color = Scalar( rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255) );
       drawContours( drawing, contours_poly, i, color, 1, 8, vector<Vec4i>(), 0, Point() );
       rectangle( drawing, boundRect[i].tl(), boundRect[i].br(), color, 2, 8, 0 );
     }*/
    
  Mat cropped = src(boundRect[index_qr]);

  /// Show in a window (debug)
  
  imshow( "Crop", cropped );
  //imwrite("qr_id.jpg",cropped);
  
  waitKey(0);
  
  // create a reader
  ImageScanner scanner;

  // configure the reader
  scanner.set_config(ZBAR_NONE, ZBAR_CFG_ENABLE, 1);
  // obtain image data
  Mat grey;
  int width = cropped.cols;   // extract dimensions
  int height = cropped.rows;
  cvtColor(cropped,grey,CV_BGR2GRAY);
  uchar *raw= grey.data;
  // wrap image data
  Image image(width, height, "Y800", raw, width * height);

  // scan the image for barcodes
  int n = scanner.scan(image);

  // extract results
  for(Image::SymbolIterator symbol = image.symbol_begin();
      symbol != image.symbol_end();
      ++symbol) {
      // do something useful with results
      cout << "QR code: "<<symbol->get_data() << endl;
  }
  return n;
}
  
