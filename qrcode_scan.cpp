#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <zbar.h>
#include <zbar/ImageScanner.h>
#include <iostream>

using namespace cv;
using namespace std;
using namespace zbar;

//g++ main.cpp /usr/local/include/ /usr/local/lib/ -lopencv_highgui.2.4.8 -lopencv_core.2.4.8

int main(int argc, char* argv[])
{
    Mat frame,grey;
    VideoCapture cap; // open the video camera no. 0
    const std::string videoStreamAddress = "http://192.168.0.100/-wvhttp-01-/video.cgi?.mjpg";
    
    bool bSuccess;
    if (!cap.open(videoStreamAddress))  // if not success, exit program
    {
        cout << "Cannot open the video cam" << endl;
        return -1;
    }
    
    ImageScanner scanner;  
    scanner.set_config(ZBAR_NONE, ZBAR_CFG_ENABLE, 1);  

    double dWidth = cap.get(CV_CAP_PROP_FRAME_WIDTH); //get the width of frames of the video
    double dHeight = cap.get(CV_CAP_PROP_FRAME_HEIGHT); //get the height of frames of the video

    namedWindow("MyVideo",CV_WINDOW_AUTOSIZE); //create a window called "MyVideo"

    while (1)
    {
        
        cap>>frame; // read a new frame from video

        cvtColor(frame,grey,CV_BGR2GRAY);
        int width = frame.cols;  
        int height = frame.rows;  
        uchar *raw = (uchar *)grey.data;  
        // wrap image data  
        Image img;
        
        img.set_size(width, height);
	img.set_format("Y800");
	img.set_data(raw,sizeof(raw));  
        // scan the image for barcodes  
	int n= scanner.scan(img);
        // extract results  
        for(Image::SymbolIterator symbol = img.symbol_begin();  
        symbol != img.symbol_end();  
        ++symbol) {  
                vector<Point> vp;  
        // do something useful with results  
        cout << "decoded " << symbol->get_type_name()  << " symbol \"" << symbol->get_data() << '"' <<" "<< endl;  
           int n = symbol->get_location_size();  
           for(int i=0;i<n;i++){  
                vp.push_back(Point(symbol->get_location_x(i),symbol->get_location_y(i))); 
           }  
           RotatedRect r = minAreaRect(vp);  
           Point2f pts[4];  
           r.points(pts);  
           for(int i=0;i<4;i++){  
                line(frame,pts[i],pts[(i+1)%4],Scalar(255,0,0),3);  
           }
           //cout<<"Angle: "<<r.angle<<endl;  
        } 

        imshow("MyVideo", frame); //show the frame in "MyVideo" window

       if (waitKey(30) == 'c') //wait for 'esc' key press for 30ms. If 'esc' key is pressed, break loop
       {
            cout << "esc key is pressed by user" << endl;
            break; 
       }
    }
    return 0;

}