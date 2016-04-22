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
std::string cam_addr="http://192.168.0.100/-wvhttp-01-/";

canon_cmd::canon_cmd()
{
  CURL *curl;
  CURLcode res;
  std::string readBuffer,url_claim,url_open;
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
}
  
int canon_cmd::cmd_pan(float pan)
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

int canon_cmd::cmd_tilt(float tilt)
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

float canon_cmd::get_pan()
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
float canon_cmd::get_tilt()
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
int canon_cmd::cmd(float pan,float tilt)
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

    int c=cmd_pan(pan);
    c=cmd_tilt(tilt);
    
    url_yield=cam_addr+"yield.cgi?s="+id;
    curl_easy_setopt(curl, CURLOPT_URL, url_yield.c_str());
    res = curl_easy_perform(curl);
        
  curl_easy_cleanup(curl);
  }
  return 0;
}
canon_cmd::~canon_cmd()
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
}
