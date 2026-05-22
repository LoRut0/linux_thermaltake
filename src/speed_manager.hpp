#include "globals.hpp"

class SpeedManager {
  public:
    virtual uint8_t* get_speed();
};

class Locked : SpeedManager {};
