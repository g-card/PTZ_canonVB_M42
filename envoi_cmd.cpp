#include <iostream>
#include <string>
#include <stdlib.h>
#include <curl/curl.h>
#include <vector>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

using namespace std;
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

int main(void)
{
  CURL *curl;
  CURLcode res;
  std::string readBuffer,id,url_open,url_claim,url_yield;
  vector <string> champs;
  float pan,tilt;

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
    
    //on ne garde que l id
    split(champs,readBuffer,is_any_of("\n"));
    split(champs,champs[0],is_any_of("="));
    id=champs[1];
    //cout<<readBuffer<<endl;
    //demander controle camera
    url_claim=cam_addr+"claim.cgi?s="+id;
    curl_easy_setopt(curl, CURLOPT_URL, url_claim.c_str());
    res = curl_easy_perform(curl);
    
    //modif pan/tilt
    cout<<"pan [-170;170]:";
    scanf("%f",&pan);
    cout<<"tilt [-90;10]:";
    scanf("%f",&tilt);
    int c=cmd_pan(id,pan);
    c=cmd_tilt(id,tilt);
    
    //rendre controle camera (obligatoire pour enchainer des commandes)
    url_yield=cam_addr+"yield.cgi?s="+id;
    curl_easy_setopt(curl, CURLOPT_URL, url_yield.c_str());
    res = curl_easy_perform(curl);
    
  curl_easy_cleanup(curl);
  }
  
  
  return 0;
}