# PTZ_canonVB_M42

####Module Ros pour interfacer la caméra.
####Ainsi que differentes fonctions permettant:
- d'acquérir le flux vidéo de la caméra Canon VB M42;
- d'étalonner la caméra (param intrinsèques/extrinsèques);
- de la commander (via le serveur http webview);
- de pouvoir ensuite traiter ce flux pour pouvoir faire un suivi d'objet grâce à OpenCV (asservissement/soustraction de fond...).

####ROS interface for the camera.
####And few programs useful for:
- reading video from PTZ camera (Canon VB M42);
- finding parameters of the camera using a chessboard (with OpenCV);
- sending commands (with http server);
- doing some video analysis (with OpenCV).

