#include <Arduino.h>
#ifdef USE_ARDUINO_JSON
#include <ArduinoJson.h>
#endif

#ifndef _ABOUT_STRING_HELPER_H
#define _ABOUT_STRING_HELPER_H

class AboutHelper {
public:
  static String generateAboutString(bool abbreviated = false);
  //static void generateAboutObject(JsonDocument& obj, bool abbreviated = false);
};

#endif