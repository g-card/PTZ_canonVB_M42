#ifndef CANON_DRIVER_H
#define CANON_DRIVER_H

#include <vector>
#include <curl/curl.h>


using namespace std;

class CanonDriver
{
    public :
	const static unsigned int CONTROL_PORT = 80;
	const static unsigned int VIDEO_PORT = 80;
    protected :
      
	bool connected;
	unsigned int verbose;
//	CommManager cm;
//	VideoManager vm;
	char * host;
/*
	bool sendRequestAndWait(unsigned char id, unsigned short status, 
		Datagram & dgm,unsigned char * data=NULL, 
		unsigned int datasize=0);
	bool sendAndWait(unsigned char id, unsigned short status, 
		unsigned char * data=NULL, unsigned int datasize=0);
	bool sendAndWait33(unsigned char id, unsigned short status, 
		unsigned char * data=NULL, unsigned int datasize=0);
	bool interpreteDatagram33(const Datagram & dgm);
	double cpan,ctilt,czoom;
*/
    public :
	CanonDriver(const char * hostname);
	~CanonDriver();

	bool connect();
	bool disconnect();
	bool requestCurrentPos();
	void getCurrentPos(double * pan, double * tilt, double * zoom);
	float get_pan();
	float get_tilt();
	float get_zoom();
	bool moveTo(float pan, float tilt, float zoom);
	int panTo(float pan);
	int tiltTo(float to);
	int zoomTo(float zoom);
	void point(int choix);

	//bool center();
	typedef enum {
	    None=0, Right=1, Left=2, Up=4, UpRight=5, 
	    UpLeft=6, Down=8, DownRight=9, DownLeft=10
	} Direction;
	//bool startMoving(Direction dir);
	//bool stop();

	//bool startDeZooming();
	//bool startZooming();
	//bool stopZooming();

	
	bool setPanSpeed(float pan, float speed) ;
	bool setTiltSpeed(float tilt, float speed) ;
	bool setZoomSpeed(float zoom, float speed);
	
	//bool getSpeeds(unsigned short * pan, unsigned short * tilt,
	//	unsigned short * zoom);

	//bool setMaxSpeed();
	//bool setMinSpeed();

	bool setFocusMode(char* fm, float val_fm);
	bool getFocusMode(char * fm, float * val_fm);

	bool getAutoExposure(char * aem);
	bool setAutoExposure(char* aem);

	bool setExposureParameters(unsigned int aperture, 
		unsigned int inv_shutter, unsigned int gain);
	bool getExposureParameters(unsigned int * aperture, 
		unsigned int * inv_shutter, unsigned int * gain);

	bool setDigitalZoom(unsigned char zoom);
	unsigned char getDigitalZoom();

	bool setNightMode(bool activated=true);
	bool getNightMode();

	bool setImageSize(unsigned int width, unsigned int height);
	bool getImageSize(unsigned int  *width, unsigned int *height);
/*
	bool startVideoReception(unsigned char * dst, 
		VideoManager::SignalF f, void * farg=NULL);
	bool startVideoReception(VideoManager::SignalF f,
		void * farg=NULL);
	void setVideoOutputColorSpace(JpegReader::ColorSpace cspace);
	bool pauseVideoReception();
	bool resumeVideoReception();
	bool setVideoReceptionPauseStatus(bool pause);
	bool getVideoReceptionPauseStatus();
	bool getVideoRecordingStatus() const;
	void setVideoRecordingMode(bool mode);
	bool stopVideoReception();		
	void setRecordingDestination(const char * dirname);
	void startVideoRecording();		
	void stopVideoRecording();		
	void setMaxFrameRate();
	void setFrameRate(double fps);
	double getRequiredFrameRate();
	double getObservedFrameRate();
*/
	void setVerboseLevel(unsigned int v) {
	    verbose = v;
	    printf("Verbose level set to %d\n",verbose);
	}

	bool sendDatagram(const std::vector<unsigned char> & dgm);
	private:
	  std::string id,//!< identifiant de session de commande 
	  cam_addr; //!< adresse IP de la camera
	  //!<useful to get info from url
	  static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
	  {
	    ((string*)userp)->append((char*)contents, size * nmemb);
	    return size * nmemb;
	  }
	  static size_t WriteNone(void *contents, size_t size, size_t nmemb, void *userp)
	  {
	    return size * nmemb;
  }

};

#endif // CANON_DRIVER_H
