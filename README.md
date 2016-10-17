# ROS package for PTZ camera from Canon


##Overview

####ROS interface for PTZ camera (this may work with others Canon's cameras) and few programs useful for:
- reading video from PTZ camera (Canon VB M42);
- finding parameters of the camera using a chessboard (with OpenCV);
- sending commands (with http server of the PTZ camera);
- doing some video analysis (with OpenCV).


## How to run the package


###Required
  -  [ROS](http://www.ros.org/install/) (Indigo or above);
  -  [OpenCV3](http://wiki.ros.org/opencv3) for ROS (usually included since Jade);
  -  And the [vision_opencv](http://wiki.ros.org/vision_opencv) package to make the link between OpenCV and ROS.


###Download and compile the package


You can clone (`git clone https://github.com/g-card/PTZ_canonVB_M42/`) this directory in your [ROS workspace](http://wiki.ros.org/ROS/Tutorials/InstallingandConfiguringROSEnvironment) (in the `/src` folder).
After this, you can compile the package with the `catkin_make` command and install it by running `catkin_make install` command.


###Run the package

-  Run the command `roscore` to launch the **master** node;
-  In your ~/.bashrc file you can add this line `source /Path_to_your_workspace/install/setup.bash`;
-  Or you can run `source /Path_to_your_workspace/install/setup.bash` in every command shell where you would like to launch the package.

Then you can do a `rosrun` of the nodes:
- `rosrun canon_vbm42 main hostname` (where **hostname** is the address of the camera) to launch the main node publishing video stream and informations about the camera;
- `rosrun canon_vbm42 cmd` to launch the command node;
- `rosrun canon_vbm42 sub` to launch a node showing the video stream.

####Note:
- you'll need [this package](https://github.com/BrunoDatoMeneses/M1_ISTR/tree/master/Ligne_transitique_MONTRAC) to run the command node (or you can edit the node instead);
- this package is inspired by [this one](https://github.com/ethz-asl/ros-drivers).


####Todo:
- Send message when the command node finishes the video analysis;
- Update the configuration of the camera with a service based on dynamic reconfiguration.
