#ifndef _CANON_CMD_H_
#define _CANON_CMD_H_

class canon_cmd{

  static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
  {
    ((string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
  }
public:
  canon_cmd();
  int cmd_pan(float pan);
  int cmd_tilt(float tilt);
  int cmd(float pan,float tilt);
  float get_pan();
  float get_tilt();
  ~canon_cmd();
private:
  std::string id;
};
#endif