#include <iostream>
#include <curl/curl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include "canon_vbm42/libCanon/CanonDriver.h"
using namespace std;
using namespace boost;

CanonDriver::CanonDriver(const char * hostname)
{
	host = strdup(hostname);
	connected = false;
	verbose = 0;
}

CanonDriver::~CanonDriver() 
{
	disconnect();
	//stopVideoReception();
	//free(host); host = NULL;
}



bool CanonDriver::connect()
{
	//cm.setVerboseLevel(verbose);
	CURL *curl;
	CURLcode res;
	std::string readBuffer,url_claim,url_open;
	vector <string> champs;
	cam_addr="http://root:camera@"+lexical_cast <string>(host)+"/-wvhttp-01-/";

	curl = curl_easy_init();
	//priority=6 pour avoir un acces illimite a la commande
	url_open=cam_addr+"open.cgi?s.priority=6";
	if(curl) {
	  
	  //ouverture du l url donnant l id de la camera
	  cout<<"Ouverture de la session"<<endl;
	  curl_easy_setopt(curl, CURLOPT_URL, url_open.c_str());
	  //ecriture du retour dans readBuffer
	  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
	  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
	  //acces url
	  res = curl_easy_perform(curl);
	  //cout<<readBuffer<<endl;	//Debug informations
	  
	  
	  //on ne garde que l id de la session
	  split(champs,readBuffer,is_any_of("\n"));
	  split(champs,champs[0],is_any_of("="));
	  id=champs[1];
	  
	  curl_easy_cleanup(curl);
	  if (readBuffer.empty()) {
		printf("%s: Can't open Camera '%s:%d'\n",__FUNCTION__,
				host,CONTROL_PORT);
		return false;
	  }
	}
	connected = true;
	//return getCurrentPos();
return true;	
}

bool CanonDriver::disconnect()
{
  CURL *curl;
  CURLcode res;
  std::string readBuffer,url_yield,url_close;
  
  curl = curl_easy_init();
  if(curl) {
    url_yield=cam_addr+"yield.cgi?s="+id;
    curl_easy_setopt(curl, CURLOPT_URL, url_yield.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteNone);
    res = curl_easy_perform(curl);
    //rendre controle camera (obligatoire pour enchainer des commandes)
    cout<<endl<<"Fermeture de la session"<<endl;
    url_close=cam_addr+"close.cgi?s="+id;
    curl_easy_setopt(curl, CURLOPT_URL, url_close.c_str());
    res = curl_easy_perform(curl);
  }
  return true;
}


bool CanonDriver::moveTo(float pan, float tilt, float zoom)
{
  CURL *curl;
  CURLcode res;
  std::string readBuffer,url_claim,url_yield;
  
  curl = curl_easy_init();
  if(curl) {
    url_claim=cam_addr+"claim.cgi?s="+id;
    curl_easy_setopt(curl, CURLOPT_URL, url_claim.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteNone);
    res = curl_easy_perform(curl);
    //modif pan/tilt
    curl_easy_cleanup(curl);
  }
  curl = curl_easy_init();
  if(curl) {  
    int c=panTo(pan);
    c=tiltTo(tilt);
    c=zoomTo(zoom);
    usleep(200000);
    url_yield=cam_addr+"yield.cgi?s="+id;
    curl_easy_setopt(curl, CURLOPT_URL, url_yield.c_str());
    res = curl_easy_perform(curl);  
  curl_easy_cleanup(curl);
  }
  return 0;
}


//fonction pour commander le pan (en position)
int CanonDriver::panTo(float pan)
{
  CURL *curl;
  CURLcode out;
  curl = curl_easy_init();
  int p;
  std::string cmd_url,buff;
  
  //bornes du pan (pas obligatoire car bornage déjà fait sur la camera)
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

//fonction pour commander le tilt (en position)
int CanonDriver::tiltTo(float tilt)
{
  CURL *curl;
  CURLcode out;
  curl = curl_easy_init();
  int t;
  std::string cmd_url,buff;
  
  
  //si image non inversée, [-90;10], si inversée [90;-10]
  if(tilt<-90)
    tilt=-90;
  if(tilt>10)
    tilt=10;
  t=int(tilt*100);
  
  cmd_url=cam_addr+"control.cgi?s="+id+"&tilt="+ lexical_cast <string>(t);
  if(curl) {
    curl_easy_setopt(curl, CURLOPT_URL, cmd_url.c_str());
    out = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
  }
  return 0;
}

//fonction pour commander le zoom (en position)
int CanonDriver::zoomTo(float zoom)
{
  CURL *curl;
  CURLcode out;
  curl = curl_easy_init();
  int z;
  std::string cmd_url,buff;
  
  //entre 3.2 et 60.4
  if(zoom<3.2)
    zoom=3.2;
  if(zoom>60.4)
    zoom=60.4;
  z=int(zoom*100);
  
  cmd_url=cam_addr+"control.cgi?s="+id+"&zoom="+ lexical_cast <string>(z);
  if(curl) {
    curl_easy_setopt(curl, CURLOPT_URL, cmd_url.c_str());
    out = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
  }
  return 0;
}


//avoir le pan pour pouvoir realiser un asservissement dessus
float CanonDriver::get_pan()
{
  CURL *curl;
  CURLcode out;
  curl = curl_easy_init();
  std::string get_url,buff;
  vector<string> champ,c;
  
  get_url=cam_addr+"GetCameraInfo";
  if(curl){//ouverture du l url donnant l id de la camera
    curl_easy_setopt(curl, CURLOPT_URL, get_url.c_str());
    //ecriture du retour dans readBuffer
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buff);
    //acces url
    
    out = curl_easy_perform(curl);
    split(champ,buff,is_any_of("\n"));
    split(c,champ[3],is_any_of("="));
    curl_easy_cleanup(curl);
    return(lexical_cast <float>(c[1])/100);
    }
    return(800);//TODO changer le retour d erreur

}

//avoir le tilt pour pouvoir realiser un asservissement dessus
float CanonDriver::get_tilt()
{
  CURL *curl;
  CURLcode out;
  curl = curl_easy_init();
  std::string get_url,buff;
  vector<string> champ,c;
  
  get_url=cam_addr+"GetCameraInfo";
  if(curl){//ouverture du l url donnant l id de la camera
    curl_easy_setopt(curl, CURLOPT_URL, get_url.c_str());
    //ecriture du retour dans readBuffer
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buff);
    //acces url
    out = curl_easy_perform(curl);
    split(champ,buff,is_any_of("\n"));
    split(c,champ[4],is_any_of("="));
    curl_easy_cleanup(curl);
    return(lexical_cast <float>(c[1])/100);
  }
    return(800);//TODO changer le retour d erreur
  
}

//avoir le zoom pour pouvoir realiser un asservissement dessus
float CanonDriver::get_zoom()
{
  CURL *curl;
  CURLcode out;
  curl = curl_easy_init();
  std::string get_url,buff;
  vector<string> champ,z,p,t;
  
  get_url=cam_addr+"GetCameraInfo";
  if(curl){//ouverture du l url donnant l id de la camera
    curl_easy_setopt(curl, CURLOPT_URL, get_url.c_str());
    //ecriture du retour dans readBuffer
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buff);
    //acces url
    
    out = curl_easy_perform(curl);
    split(champ,buff,is_any_of("\n"));
    split(z,champ[5],is_any_of("="));
    curl_easy_cleanup(curl);
    return(lexical_cast <float>(z[1])/100);
    }
    return(800);//TODO changer le retour d erreur

}

//avoir le zoom pour pouvoir realiser un asservissement dessus
void CanonDriver::getCurrentPos(double* pan,double* tilt,double* zoom)
{
  *pan =(double) get_pan();
  *tilt = (double)get_tilt();
  *zoom =(double) get_zoom();
  //printf("At position: %f %f %f\n", cpan,ctilt,czoom);

}

void CanonDriver::point(int choix)
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
  if(fclose(cmd_file) == 0)
    cout << "Param. lu" << endl;

  pan = coord[choix][1];
  tilt = coord[choix][2];
  zoom = coord[choix][3];
  bool tag = CanonDriver::moveTo(pan,tilt,zoom);
}

bool CanonDriver::setPanSpeed(float pan, float speed) 
{
  CURL *curl;
  CURLcode out;
  curl = curl_easy_init();
  int p,s;
  std::string cmd_url,buff;
  
  //bornes du pan (pas obligatoire car bornage déjà fait sur la camera)
  if(pan<-170)
    pan=-170;
  if(pan>170)
    pan=170;
  //formatage de l'info du pan
  p=int(pan*100);
  
  //bornes du speed (pas obligatoire car bornage déjà fait sur la camera)
  if(speed<0.62)
    speed=0.62;
  if(speed>150)
    speed=150;
  //formatage de l'info du speed
  s=int(speed*100);
  
  //url necessaire pour controler le pan
  cmd_url=cam_addr+"control.cgi?s="+id+"&pan="+ lexical_cast <string>(p)+"&pan.speed.pos="+lexical_cast <string>(s);
  if(curl) {
    curl_easy_setopt(curl, CURLOPT_URL, cmd_url.c_str());
    out = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
  }
  return false;
}

bool CanonDriver::setTiltSpeed(float tilt, float speed) 
{
  CURL *curl;
  CURLcode out;
  curl = curl_easy_init();
  int t,s;
  std::string cmd_url,buff;
  
  //bornes du tilt (pas obligatoire car bornage déjà fait sur la camera)
  if(tilt<-170)
    tilt=-170;
  if(tilt>170)
    tilt=170;
  //formatage de l'info du tilt
  t=int(tilt*100);
  
  //bornes du speed (pas obligatoire car bornage déjà fait sur la camera)
  if(speed<0.62)
    speed=0.62;
  if(speed>150)
    speed=150;
  //formatage de l'info du speed
  s=int(speed*100);
  
  //url necessaire pour controler le tilt
  cmd_url=cam_addr+"control.cgi?s="+id+"&tilt="+ lexical_cast <string>(t)+"&tilt.speed.pos="+lexical_cast <string>(s);
  if(curl) {
    curl_easy_setopt(curl, CURLOPT_URL, cmd_url.c_str());
    out = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
  }
  return 0;
}

bool CanonDriver::setZoomSpeed(float zoom, float speed) 
{
  CURL *curl;
  CURLcode out;
  curl = curl_easy_init();
  int z,s;
  std::string cmd_url,buff;
  
  //bornes du zoom (pas obligatoire car bornage déjà fait sur la camera)
  if(zoom<-170)
    zoom=-170;
  if(zoom>170)
    zoom=170;
  //formatage de l'info du zoom
  z=int(zoom*100);
  
  //bornes du speed (pas obligatoire car bornage déjà fait sur la camera)
  if(speed<0.62)
    speed=0.62;
  if(speed>150)
    speed=150;
  //formatage de l'info du speed
  s=int(speed*100);
  
  //url necessaire pour controler le zoom
  cmd_url=cam_addr+"control.cgi?s="+id+"&zoom="+ lexical_cast <string>(z)+"&zoom.speed.pos="+lexical_cast <string>(s);
  if(curl) {
    curl_easy_setopt(curl, CURLOPT_URL, cmd_url.c_str());
    out = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
  }
  return 0;
}



/*bool CanonDriver::getSpeeds(unsigned short * pan, 
		unsigned short * tilt, unsigned short * zoom)
{

	return true;
}*/

bool CanonDriver::setFocusMode(char* fm, float val_fm)
{
  CURL *curl;
  CURLcode out;
  curl = curl_easy_init();
  int z,s;
  std::string cmd_url,buff;
  
  if(fm=="manual"){
    //url necessaire pour controler le focus
    cmd_url=cam_addr+"control.cgi?s="+id+"&zoom="+ lexical_cast <string>(fm)+"&focus.value="+lexical_cast <string>(val_fm);
    if(curl) {
      curl_easy_setopt(curl, CURLOPT_URL, cmd_url.c_str());
      out = curl_easy_perform(curl);
      curl_easy_cleanup(curl);
    }
    return 0;
  }
  else{
    //url necessaire pour controler le focus
    cmd_url=cam_addr+"control.cgi?s="+id+"&focus="+ lexical_cast <string>(fm);
    if(curl) {
      curl_easy_setopt(curl, CURLOPT_URL, cmd_url.c_str());
      out = curl_easy_perform(curl);
      curl_easy_cleanup(curl);
    }
    return 0;
  }
  return 1;

}

bool CanonDriver::getFocusMode(char * fm, float * val_fm)
{
  CURL *curl;
  CURLcode out;
  curl = curl_easy_init();
  std::string get_url,buff;
  vector<string> champ,focus,val;

  get_url=cam_addr+"info.cgi";
  if(curl){
    //ouverture du l url donnant l id de la camera
    curl_easy_setopt(curl, CURLOPT_URL, get_url.c_str());
    //ecriture du retour dans readBuffer
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buff);
    //acces url
    out = curl_easy_perform(curl);
    split(champ,buff,is_any_of("focus:="));
    split(focus,champ[1],is_any_of("\n"));
    curl_easy_cleanup(curl);
    focus[0].copy(fm,focus[0].length()+1);      
    }
    
    if(fm=="manual"){
      curl = curl_easy_init();
      get_url=cam_addr+"info.cgi";
      if(curl){
	//ouverture du l url donnant l id de la camera
	curl_easy_setopt(curl, CURLOPT_URL, get_url.c_str());
	//ecriture du retour dans readBuffer
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buff);
	//acces url
	out = curl_easy_perform(curl);
	split(champ,buff,is_any_of("focus.value:="));
	split(val,champ[1],is_any_of("\n"));
	curl_easy_cleanup(curl);
	*val_fm = atof(val[0].c_str());      
	}
    }
  return false;
}

bool CanonDriver::getAutoExposure(char * aem)
{
  CURL *curl;
  CURLcode out;
  curl = curl_easy_init();
  std::string get_url,buff;
  vector<string> champ,exposure;

  get_url=cam_addr+"info.cgi";
  if(curl){
    //ouverture du l url donnant l id de la camera
    curl_easy_setopt(curl, CURLOPT_URL, get_url.c_str());
    //ecriture du retour dans readBuffer
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buff);
    //acces url
    out = curl_easy_perform(curl);
    split(champ,buff,is_any_of("exp:="));
    split(exposure,champ[1],is_any_of("\n"));
    curl_easy_cleanup(curl);
    exposure[0].copy(aem,exposure[0].length()+1);       
    }
  return false;
}


bool CanonDriver::setAutoExposure(char* aem)
{
	  CURL *curl;
  CURLcode out;
  curl = curl_easy_init();
  int z,s;
  std::string cmd_url;
  
  if(aem=="manual"){
    printf("make sure you set exposure parameters\n");
    //url necessaire pour controler l exposure
    cmd_url=cam_addr+"control.cgi?s="+id+"&exp="+ lexical_cast <string>(aem);
    if(curl) {
      curl_easy_setopt(curl, CURLOPT_URL, cmd_url.c_str());
      out = curl_easy_perform(curl);
      curl_easy_cleanup(curl);
    }
  }
  else{
    //url necessaire pour controler l exposure
    cmd_url=cam_addr+"control.cgi?s="+id+"&exp="+ lexical_cast <string>(aem);
    if(curl) {
      curl_easy_setopt(curl, CURLOPT_URL, cmd_url.c_str());
      out = curl_easy_perform(curl);
      curl_easy_cleanup(curl);
    }
  }
return false;
}


bool CanonDriver::setExposureParameters(unsigned int aperture, 
		unsigned int inv_shutter, unsigned int gain)
{
  CURL *curl;
  CURLcode out;
  curl = curl_easy_init();
  std::string cmd_url;
  
 
    //url necessaire pour controler le focus
    cmd_url=cam_addr+"control.cgi?s="+id+"&me.shutter="+ lexical_cast <string>(inv_shutter)+"&me.iris="+ lexical_cast <string>(aperture)+"&me.gain="+ lexical_cast <string>(gain);
    if(curl) {
      curl_easy_setopt(curl, CURLOPT_URL, cmd_url.c_str());
      out = curl_easy_perform(curl);
      curl_easy_cleanup(curl);
    }
    return 0;
;
}

bool CanonDriver::getExposureParameters(unsigned int * aperture, 
		unsigned int * inv_shutter, unsigned int * gain)
{
  CURL *curl;
  CURLcode out;
  curl = curl_easy_init();
  std::string get_url,buff;
  vector<string> val,champ;

  get_url=cam_addr+"info.cgi";
  if(curl){
    //ouverture du l url donnant l id de la camera
    curl_easy_setopt(curl, CURLOPT_URL, get_url.c_str());
    //ecriture du retour dans readBuffer
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buff);
    //acces url
    out = curl_easy_perform(curl);
    split(champ,buff,is_any_of("me.iris:="));
    split(val,champ[1],is_any_of("\n"));
    curl_easy_cleanup(curl);
    *aperture = atoi(val[0].c_str());      
  }
  curl = curl_easy_init();
  if(curl){
    //ouverture du l url donnant l id de la camera
    curl_easy_setopt(curl, CURLOPT_URL, get_url.c_str());
    //ecriture du retour dans readBuffer
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buff);
    //acces url
    out = curl_easy_perform(curl);
    split(champ,buff,is_any_of("me.shutter:="));
    split(val,champ[1],is_any_of("\n"));
    curl_easy_cleanup(curl);
    *inv_shutter = atoi(val[0].c_str());      
  }
  curl = curl_easy_init();
  if(curl){
    //ouverture du l url donnant l id de la camera
    curl_easy_setopt(curl, CURLOPT_URL, get_url.c_str());
    //ecriture du retour dans readBuffer
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buff);
    //acces url
    out = curl_easy_perform(curl);
    split(champ,buff,is_any_of("me.gain:="));
    split(val,champ[1],is_any_of("\n"));
    curl_easy_cleanup(curl);
    *gain = atoi(val[0].c_str());      
  }
  return false;
    
}

bool CanonDriver::setDigitalZoom(unsigned char zoom)
{
	return true;
}

bool CanonDriver::setNightMode(bool activated)
{
	return true;
}


unsigned char CanonDriver::getDigitalZoom()
{

	return 1;
}

bool CanonDriver::getNightMode()
{

	return true;
}

/*
bool CanonDriver::setImageSize(unsigned int width, unsigned int height)
{
	return true;
}

bool CanonDriver::getImageSize(unsigned int *width, unsigned int *height)
{
	return true;
}

void CanonDriver::setMaxFrameRate()
{
	vm.setMaxFrameRate();
}

void CanonDriver::setFrameRate(double fps)
{
	vm.setFrameRate(fps);
}

double CanonDriver::getRequiredFrameRate()
{
	return vm.getRequiredFrameRate();
}

double CanonDriver::getObservedFrameRate()
{
	return vm.getObservedFrameRate();
}
*/
