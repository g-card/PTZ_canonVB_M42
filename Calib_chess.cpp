#include <stdio.h>
#include <opencv2/opencv.hpp>

using namespace cv;


int main(int argc, char** argv )
{
    cv::VideoCapture cap;
    cv::Mat image;
    int wid,hei;
    RNG rng(0xFFFFFFFF);

    const std::string videoStreamAddress = "http://192.168.0.100/-wvhttp-01-/video.cgi?.mjpg";

    //open the video stream and make sure it's opened
    if(!cap.open(videoStreamAddress)) {
        std::cout << "Error opening video stream or file" << std::endl;
        return -1;
    }
    Mat acqImageGray;
    int successes = 0;

    // image point vectors
    std::vector<Point2f> pointBuf;
    std::vector<std::vector<Point2f> > imagePoints;

    // object point vectors
    std::vector<Point3f> objectBuf;
    std::vector<std::vector<Point3f> > objectPoints;

    // 3D object point into world's coordinate
    for( int i=0; i<6; i++ ){
        for( int j=0; j<9; j++ ){
            Point3f pt(i, j, 0);
            objectBuf.push_back(pt);
        }
    }
  printf("reconnaisance de mire\n");
    while(1){
  printf("attente de mire\n");
        Mat frame;
        cap >> frame; // get a new frame from camera
        wid = frame.size().width;
	hei = frame.size().height;
        cvtColor(frame, acqImageGray, CV_BGR2GRAY); // image convertion: BGR -> Gray
	//circle(acqImageGray,Point(1016,106),10,2); utilise pour verif param extrinseques
	imshow("imGray", acqImageGray);
	

        if(waitKey(10) != -1){

            pointBuf.clear();
            int patternfound = 0;

            // finding chessboard corners
		patternfound=findChessboardCorners(acqImageGray,cvSize(9,6),pointBuf,CALIB_CB_ADAPTIVE_THRESH+CALIB_CB_NORMALIZE_IMAGE);
		
            if(patternfound){
                printf("Pattern found: %i\n", successes);
                // sub-pixel corner detection
		cornerSubPix(acqImageGray,pointBuf, Size(5,5),Size(-1,-1),TermCriteria(CV_TERMCRIT_EPS + CV_TERMCRIT_ITER, 30, 0.1));
		
                // drawing chessboard into frame
                drawChessboardCorners(frame, Size(9,6), Mat(pointBuf), patternfound);
                imshow("imCorners", frame);
                cvWaitKey(10);

                // Storing points for calibration step
                imagePoints.push_back(pointBuf);
                objectPoints.push_back(objectBuf);

                successes++;
            }
        }
        if( successes == 20) break;
    }

    printf("calibrating...\n");

    Mat intrinsic = Mat(3, 3, CV_32FC1);
    Mat distCoeffs;
    std::vector<Mat> rvecs;
    std::vector<Mat> tvecs;
    
    double RMS = 0;

    // Calibrate the camera
    //Calcul param intriseques
    RMS= calibrateCamera(objectPoints, imagePoints, Size(wid,hei),  intrinsic, distCoeffs,  rvecs,  tvecs,CV_CALIB_ZERO_TANGENT_DIST );

    Mat rvecs2,tvecs2,srvecs,stvecs,R,T;
    bool fl;

    std::cout<< "RMS: "<<RMS<<std::endl;
    std::cout<< "Intr: "<<intrinsic<<std::endl;
    std::cout<< "Coef disto: "<<distCoeffs<<std::endl;

    //Calcul des param. extrinseques (rvecs et tvecs)
    fl=solvePnP(objectPoints[0], imagePoints[0],intrinsic, distCoeffs,rvecs2,tvecs2);
    srvecs=rvecs2;
    stvecs=tvecs2;
    //Moyenne des param sur les n echantillons
    for(int i=0;i<objectPoints.size();i++)
    {
	fl=solvePnP(objectPoints[i], imagePoints[i],intrinsic, distCoeffs,rvecs2,tvecs2);
	srvecs+=rvecs2;
	stvecs+=tvecs2;
    }

    //Construction de la matrice de rot
    Rodrigues(srvecs/objectPoints.size(),R);
    T=stvecs/objectPoints.size();

    std::cout<< "Extr: "<<" "<<R<<T<<std::endl;

    return 0;
}

