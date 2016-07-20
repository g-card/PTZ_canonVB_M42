#Samples using OpenCV

##Overview
###Files useful to test the functionalities of the camera without ROS:

- `Calib_chess.cpp` to calibrate intrinsic and extrinsic parameters (inspired by [this](http://docs.opencv.org/2.4/doc/tutorials/calib3d/camera_calibration/camera_calibration.html));
- `canon_cmd.cpp` and `canon_cmd.h` library of the PTZ camera;
- `asserv.cpp` visual servoing of a moving object;
- `pid.h` and `pid.cpp` PID controller for visual servoing ([source](https://gist.github.com/bradley219/5373998));
- `qr_code_scan.cpp` permitting to scan QR Codes with [ZBar library](https://github.com/ZBar/ZBar)

##Required

You need to have [OpenCV 3](http://docs.opencv.org/3.0-beta/doc/tutorials/introduction/linux_install/linux_install.html) installed on your computer (3.0 and above used for [OCR](http://docs.opencv.org/master/d7/ddc/classcv_1_1text_1_1OCRTesseract.html#gsc.tab=0) and [Background Substraction](http://docs.opencv.org/3.0-beta/doc/tutorials/video/background_subtraction/background_subtraction.html)) and the [`opencv_contrib`](https://github.com/opencv/opencv_contrib) package to install OCR.
