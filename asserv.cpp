#include <opencv2/core/utility.hpp>
#include "opencv2/video/tracking.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/videoio.hpp"
#include "opencv2/highgui.hpp"
#include <opencv2/video.hpp>
#include <string>
#include <stdlib.h>
#include <curl/curl.h>
#include <vector>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include "pid.cpp"

#include <iostream>
#include <ctype.h>

using namespace cv;
using namespace std;

// Global variables

Mat image;

bool backprojMode = false;
bool selectObject = false;
int trackObject = 0;
bool showHist = true;
Point origin;
Rect selection;
int vmin = 10, vmax = 256, smin = 30;

using namespace boost;
std::string cam_addr="http://192.168.0.100/-wvhttp-01-/";

static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    ((string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

//Envoi de la commande, TODO retourner d'autres valeurs (erreurs,depassements...)

int cmd_pan(std::string id,float pan)
{
  CURL *curl;
  CURLcode out;
  curl = curl_easy_init();
  int p;
  std::string cmd_url,buff;
  
  //bornes du pan (pas obligatoire car bornage déjà fait sur la camera
  if(pan<-170)
    pan=-170;
  if(pan>170)
    pan=170;
  //formatage de l'info du pan
  p=int(pan*100);
  //url necessaire pour controler le pan
  cmd_url=cam_addr+"control.cgi?s="+id+"&pan="+ lexical_cast <string>(p);
  if(curl) {
    curl_easy_setopt(curl, CURLOPT_URL, cmd_url.c_str());
    out = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
  }
  return 0;
}


//Pareil qu'avec le pan
int cmd_tilt(std::string id,float tilt)
{
  CURL *curl;
  CURLcode out;
  curl = curl_easy_init();
  int t;
  std::string cmd_url,buff;
  
  
  //si non inversée, [-90;10], si inversée [90;-10]
  if(tilt<-10)
    tilt=-10;
  if(tilt>90)
    tilt=90;
  t=int(tilt*100);
  
  cmd_url=cam_addr+"control.cgi?s="+id+"&tilt="+ lexical_cast <string>(t);
  if(curl) {
    curl_easy_setopt(curl, CURLOPT_URL, cmd_url.c_str());
    out = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
  }
  return 0;
}
float get_pan()
{
  CURL *curl;
  CURLcode out;
  curl = curl_easy_init();
  std::string get_url,buff;
  vector<string> champ;
  
  get_url=cam_addr+"GetCameraInfo";
  if(curl){//ouverture du l url donnant l id de la camera
    curl_easy_setopt(curl, CURLOPT_URL, get_url.c_str());
    //ecriture du retour dans readBuffer
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buff);
    //acces url
    
    out = curl_easy_perform(curl);
    split(champ,buff,is_any_of("\n"));
    split(champ,champ[3],is_any_of("="));
    return(lexical_cast <float>(champ[1])/100);
  }
    return(800);
  
}

float get_tilt()
{
  CURL *curl;
  CURLcode out;
  curl = curl_easy_init();
  std::string get_url,buff;
  vector<string> champ;
  
  get_url=cam_addr+"GetCameraInfo";
  if(curl){//ouverture du l url donnant l id de la camera
    curl_easy_setopt(curl, CURLOPT_URL, get_url.c_str());
    //ecriture du retour dans readBuffer
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buff);
    //acces url
    out = curl_easy_perform(curl);
    split(champ,buff,is_any_of("\n"));
    split(champ,champ[4],is_any_of("="));
    return(lexical_cast <float>(champ[1])/100);
  }
    return(800);
  
}
int cmd(float pan,float tilt,std::string id)
{
  CURL *curl;
  CURLcode res;
  std::string readBuffer,url_claim,url_yield;
  
  curl = curl_easy_init();
  if(curl) {
    url_claim=cam_addr+"claim.cgi?s="+id;
    curl_easy_setopt(curl, CURLOPT_URL, url_claim.c_str());
    res = curl_easy_perform(curl);
    //modif pan/tilt

    int c=cmd_pan(id,pan);
    c=cmd_tilt(id,tilt);
    
    url_yield=cam_addr+"yield.cgi?s="+id;
    curl_easy_setopt(curl, CURLOPT_URL, url_yield.c_str());
    res = curl_easy_perform(curl);
        
  curl_easy_cleanup(curl);
  }
  return 0;
}
int close_camera(std::string id)
{
  CURL *curl;
  CURLcode res;
  std::string readBuffer,url_yield,url_close;
  
  curl = curl_easy_init();
  if(curl) {
    //rendre controle camera (obligatoire pour enchainer des commandes)
    
    url_close=cam_addr+"close.cgi?s="+id;
    curl_easy_setopt(curl, CURLOPT_URL, url_close.c_str());
    res = curl_easy_perform(curl);
  }
  return 0;
}

std::string open_camera()
{
  CURL *curl;
  CURLcode res;
  std::string readBuffer,url_claim,id,url_open;
  vector <string> champs;
  

  curl = curl_easy_init();
  url_open=cam_addr+"open.cgi";
  if(curl) {
    
    //ouverture du l url donnant l id de la camera
    curl_easy_setopt(curl, CURLOPT_URL, url_open.c_str());
    //ecriture du retour dans readBuffer
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
    //acces url
    res = curl_easy_perform(curl);
    cout<<readBuffer<<endl;
    
    //on ne garde que l id
    split(champs,readBuffer,is_any_of("\n"));
    split(champs,champs[0],is_any_of("="));
    id=champs[1];  
    
    
    curl_easy_cleanup(curl);
  }
  return id;
}

static void onMouse( int event, int x, int y, int, void* )
{
    if( selectObject )
    {
        selection.x = MIN(x, origin.x);
        selection.y = MIN(y, origin.y);
        selection.width = std::abs(x - origin.x);
        selection.height = std::abs(y - origin.y);

        selection &= Rect(0, 0, image.cols, image.rows);
    }

    switch( event )
    {
    case EVENT_LBUTTONDOWN:
        origin = Point(x,y);
        selection = Rect(x,y,0,0);
        selectObject = true;
        break;
    case EVENT_LBUTTONUP:
        selectObject = false;
        if( selection.width > 0 && selection.height > 0 )
            trackObject = -1;
        break;
    }
}

string hot_keys =
    "\n\nHot keys: \n"
    "\tESC - quit the program\n"
    "\tc - stop the tracking\n"
    "\tb - switch to/from backprojection view\n"
    "\th - show/hide object histogram\n"
    "\tp - pause video\n"
    "To initialize tracking, select the object with mouse\n";

static void help()
{
    cout << "\nThis is a demo that shows mean-shift based tracking\n"
            "You select a color objects such as your face and it tracks it.\n"
            "This reads from video camera (0 by default, or the camera number the user enters\n"
            "Usage: \n"
            "   ./camshiftdemo [camera number]\n";
    cout << hot_keys;
}

const char* keys =
{
    "{help h | | show help message}{@camera_number| 0 | camera number}"
};

int main( int argc, const char** argv )
{
    VideoCapture cap;
    //TODO trouver les bons gains (Kp,Kd,Ki)
    
    
    PIDImpl cor_pan(0.4,100,-100,0.0023,0.0001,0.00001);
    PIDImpl cor_tilt(0.3,10,-90,0.0027,0.0001,0.00001);
    Rect trackWindow;
    int hsize = 16;
    float hranges[] = {0,180};
    const float* phranges = hranges;
    std::string id;
    Mat frameM; //current frame
    Mat fgMaskMOG2; //fg mask fg mask generated by MOG2 method
    Ptr<BackgroundSubtractor> pMOG2; //MOG2 Background subtractor
   const std::string videoStreamAddress = "http://192.168.0.100/-wvhttp-01-/video.cgi?.mjpg"; 
   cap.open(videoStreamAddress);
    if(!cap.isOpened()){
        //error in opening the video input
        cerr << "Unable to open video file: " << videoStreamAddress << endl;
        exit(EXIT_FAILURE);
    }

    id=open_camera();
    cout << hot_keys;
    //namedWindow( "Histogram", 0 );
    namedWindow( "CamShift Demo", 0 );
    //namedWindow("FG Mask MOG 2");
    setMouseCallback( "CamShift Demo", onMouse, 0 );
    createTrackbar( "Vmin", "CamShift Demo", &vmin, 256, 0 );
    createTrackbar( "Vmax", "CamShift Demo", &vmax, 256, 0 );
    createTrackbar( "Smin", "CamShift Demo", &smin, 256, 0 );
    pMOG2 = createBackgroundSubtractorMOG2(500,50,false); //MOG2 approach
        //input data coming from a video
    
    
        //update the background model
      
    Mat frame,back, hsv, hue, mask, hist, histimg = Mat::zeros(200, 320, CV_8UC3), backproj;
    bool paused = false;
    
    for(;;)
    {
        if( !paused )
        {
            cap >> frame;
	   // pMOG2->apply(frame, fgMaskMOG2,0.01);
	  
            if( frame.empty() )
                break;
        }
        //pMOG2->getBackgroundImage(back);
	/*frame.copyTo(image);
	printf("%d,%d\t%d,%d\t,%d,%d\n",fgMaskMOG2.rows,fgMaskMOG2.cols,frame.rows,frame.cols,image.rows,image.cols);
	bitwise_and(frame,fgMaskMOG2,image);*/
	//subtract(frame,back,frame);
        frame.copyTo(image);
	
        if( !paused )
        {
            cvtColor(image, hsv, COLOR_BGR2HSV);
	    

            if( trackObject )
            {
                int _vmin = vmin, _vmax = vmax;

                inRange(hsv, Scalar(0, smin, MIN(_vmin,_vmax)),
                        Scalar(180, 256, MAX(_vmin, _vmax)), mask);
                int ch[] = {0, 0};
                hue.create(hsv.size(), hsv.depth());
                mixChannels(&hsv, 1, &hue, 1, ch, 1);

                if( trackObject < 0 )
                {
                    Mat roi(hue, selection), maskroi(mask, selection);
                    calcHist(&roi, 1, 0, maskroi, hist, 1, &hsize, &phranges);
                    normalize(hist, hist, 0, 255, NORM_MINMAX);

                    trackWindow = selection;
                    trackObject = 1;

                    histimg = Scalar::all(0);
                    int binW = histimg.cols / hsize;
                    Mat buf(1, hsize, CV_8UC3);
                    for( int i = 0; i < hsize; i++ )
                        buf.at<Vec3b>(i) = Vec3b(saturate_cast<uchar>(i*180./hsize), 255, 255);
                    cvtColor(buf, buf, COLOR_HSV2BGR);

                    for( int i = 0; i < hsize; i++ )
                    {
                        int val = saturate_cast<int>(hist.at<float>(i)*histimg.rows/255);
                        rectangle( histimg, Point(i*binW,histimg.rows),
                                   Point((i+1)*binW,histimg.rows - val),
                                   Scalar(buf.at<Vec3b>(i)), -1, 8 );
                    }
                }

                calcBackProject(&hue, 1, 0, hist, backproj, &phranges);
                backproj &= mask;
                RotatedRect trackBox = CamShift(backproj, trackWindow,
                                    TermCriteria( TermCriteria::EPS | TermCriteria::COUNT, 10, 1 ));
                if( trackWindow.area() <= 1 )
                {
                    int cols = backproj.cols, rows = backproj.rows, r = (MIN(cols, rows) + 5)/6;
                    trackWindow = Rect(trackWindow.x - r, trackWindow.y - r,
                                       trackWindow.x + r, trackWindow.y + r) &
                                  Rect(0, 0, cols, rows);
                }

                //if( backprojMode )
                    cvtColor( backproj, image, COLOR_GRAY2BGR );
                ellipse( image, trackBox, Scalar(0,0,255), 3, LINE_AA );
            }
        }
        else if( trackObject < 0 )
            paused = false;

        if( selectObject && selection.width > 0 && selection.height > 0 )
        {
            Mat roi(image, selection);
            bitwise_not(roi, roi);
        }

        
	Point center(((trackWindow.br().x-trackWindow.tl().x)/2)+trackWindow.tl().x,((trackWindow.br().y-trackWindow.tl().y)/2)+trackWindow.tl().y);
		
	cout<<"Pos centre: "<<center<<endl;
	
	if(trackObject){
	   cmd(get_pan()-cor_pan.calculate(640,center.x),get_tilt()+cor_tilt.calculate(360,center.y),id);
	   usleep(50000);
	}  

        imshow( "CamShift Demo", image );
        //imshow( "Histogram", histimg );
	//imshow("FG Mask MOG 2", fgMaskMOG2);
	
        char c = (char)waitKey(10);
        if( c == 27 )
            break;
        switch(c)
        {
        case 'b':
            backprojMode = !backprojMode;
            break;
        case 'c':
            trackObject = 0;
            histimg = Scalar::all(0);
            break;
        case 'h':
            showHist = !showHist;
            if( !showHist )
                destroyWindow( "Histogram" );
            else
                namedWindow( "Histogram", 1 );
            break;
        case 'p':
            paused = !paused;
            break;
        default:
            ;
        }
    }
    close_camera(id);
    return 0;
}