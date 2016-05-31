#ifndef _CANON_CMD_H_
#define _CANON_CMD_H_

/** canon_cmd
 Classe permettant d'envoyer des commande a une camera PTZ Canon.
 */
using namespace std;
class canon_cmd{

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
public:
  //! Constructeur
  /*!
   Demande l'acces a la camera dont l'adresse est precisee
  */
  canon_cmd(const char* hostname);
  
  //! Fonction permettant de commander le pan de la camera.
  /*!
    \param pan valeur du pan (entre -170 et 170 deg).
    \return 0 si commande envoyée
    \sa cmd_tilt(), cmd_zoom() et cmd().
  */  
  int cmd_pan(float pan);
  
  //! Fonction permettant de commander le tilt de la camera.
  /*!
    \param tilt valeur du tilt (si image non inversée, [-90;10] deg, si inversée [90;-10] deg).
    \return 0 si commande envoyée
    \sa cmd_pan(), cmd_zoom() et cmd().
  */  
  int cmd_tilt(float tilt);
  
  //! Fonction permettant de commander le zoom de la camera.
  /*!
    \param zoom valeur du zoom (entre 3.2 et 60.4).
    \return 0 si commande envoyée
    \sa cmd_pan(), cmd_tilt() et cmd().
  */  
  int cmd_zoom(float zoom);
  
  //! Fonction permettant de commander le pan et le tilt de la camera.
  /*!
    \param pan valeur du pan (entre -170 et 170 deg).
    \param tilt valeur du tilt (si image non inversée, [-90;10], si inversée [90;-10]).
    \return 0 si commande envoyée
    \sa cmd_pan(), cmd_tilt() et cmd_zoom().
  */
  int cmd(float pan,float tilt);
  
  //! Fonction permettant de commander le pan, le tilt et le zoom de la camera.
  /*!
    \param pan valeur du pan (entre -170 et 170 deg).
    \param tilt valeur du tilt (si image non inversée, [-90;10], si inversée [90;-10]).
    \param zoom valeur du zoom (entre 3.2 et 60.4).
    \return 0 si commande envoyée
    \sa cmd_pan(), cmd_tilt() et cmd_zoom().
  */
  int cmd(float pan,float tilt,float zoom);
  
  //! Fonction permettant de récupérer le pan de la camera.
  /*!
    \return la valeur du pan
    \sa get_tilt() et get_zoom().
  */
  float get_pan();
  
  //! Fonction permettant de récupérer le tilt de la camera.
  /*!
    \return la valeur du tilt
    \sa get_pan() et get_zoom().
  */
  float get_tilt();
  
  //! Fonction permettant de récupérer le zoom de la camera.
  /*!
    \return la valeur du zoom
    \sa get_tilt() et get_pan().
  */
  float get_zoom();
  
  //int cmd_pan_speed(float pan,float speed);
  //int cmd_tilt_speed(float tilt,float speed);
  
  //! Destructeur
  /*!
   Ferme proprement la connexion avec la camera
  */
  ~canon_cmd();
  
private:
  std::string id,//!< identifiant de session de commande 
  cam_addr; //!< adresse IP de la camera
  
};
#endif