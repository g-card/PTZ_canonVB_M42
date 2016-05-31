#include <iostream>
#include <string>
#include <stdlib.h>
#include <curl/curl.h>
#include <vector>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include "canon_cmd.h"

using namespace std;
using namespace boost;


//Constructeur, ouverture session commande
canon_cmd::canon_cmd(const char* hostname)
{
  CURL *curl;
  CURLcode res;
  std::string readBuffer,url_claim,url_open;
  vector <string> champs;
  cam_addr="http://root:camera@"+lexical_cast <string>(hostname)+"/-wvhttp-01-/";

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
    //cout<<readBuffer<<endl;
    
    //on ne garde que l id de la session
    split(champs,readBuffer,is_any_of("\n"));
    split(champs,champs[0],is_any_of("="));
    id=champs[1];
    
    curl_easy_cleanup(curl);
  }
}

//fonction pour commander le pan (en position)
int canon_cmd::cmd_pan(float pan)
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
int canon_cmd::cmd_tilt(float tilt)
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
int canon_cmd::cmd_zoom(float zoom)
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
float canon_cmd::get_pan()
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
float canon_cmd::get_tilt()
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
float canon_cmd::get_zoom()
{
  CURL *curl;
  CURLcode out;
  curl = curl_easy_init();
  std::string get_url,buff;
  vector<string> champ,z;
  
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

//commande du pan et du tilt l un apres l autre (en position)
int canon_cmd::cmd(float pan,float tilt)
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

    int c=cmd_pan(pan);
    c=cmd_tilt(tilt);
    usleep(200000);
    url_yield=cam_addr+"yield.cgi?s="+id;
    curl_easy_setopt(curl, CURLOPT_URL, url_yield.c_str());
    res = curl_easy_perform(curl);
        
  curl_easy_cleanup(curl);
  }
  return 0;
}

//commande du pan du tilt et du zoom l un apres l autre (en position)
int canon_cmd::cmd(float pan,float tilt,float zoom)
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

    int c=cmd_pan(pan);
    c=cmd_tilt(tilt);
    c=cmd_zoom(zoom);
    usleep(200000);
    url_yield=cam_addr+"yield.cgi?s="+id;
    curl_easy_setopt(curl, CURLOPT_URL, url_yield.c_str());
    res = curl_easy_perform(curl);
        
  curl_easy_cleanup(curl);
  }
  return 0;
}

//commande en vitesse du pan
/*
int canon_cmd::cmd_pan_speed(float pan,float speed)
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
  return 0;
}
 
 */

//commande en vitesse du tilt
/*
int canon_cmd::cmd_tilt_speed(float tilt,float speed)
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
 
 */

//destructeur, fermeture de la session de commande
canon_cmd::~canon_cmd()
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
}
