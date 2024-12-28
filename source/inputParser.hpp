#pragma once

// Base controller class
// by eien86

#include <cstdint>
#include <jaffarCommon/exceptions.hpp>
#include <jaffarCommon/json.hpp>
#include <string>
#include <sstream>

namespace jaffar
{

enum GBKey {
	GBA_KEY_A = 1,
	GBA_KEY_B = 2,
	GBA_KEY_SELECT = 4,
	GBA_KEY_START = 8,
	GBA_KEY_RIGHT = 16,
	GBA_KEY_LEFT = 32,
	GBA_KEY_UP = 64,
	GBA_KEY_DOWN = 128
};

typedef uint16_t port_t;

struct input_t
{
  bool power = false;
  port_t port = 0;
};

class InputParser
{
public:

  enum controller_t { none, gb };

  InputParser(const nlohmann::json &config)
  {
    // Parsing controller type
    {
      bool isTypeRecognized = false;

      const auto controllerType = jaffarCommon::json::getString(config, "Controller Type");
      if (controllerType == "None")    { _controllerType = controller_t::none; isTypeRecognized = true; }
      if (controllerType == "Gameboy") { _controllerType = controller_t::gb;  isTypeRecognized = true; }
      
      if (isTypeRecognized == false) JAFFAR_THROW_LOGIC("Controller type not recognized: '%s'\n", controllerType.c_str()); 
   }
  }

  inline input_t parseInputString(const std::string &inputString) const
  {
    // Storage for the input
    input_t input;

    // Currently read character
    char c;

    // Converting input into a stream for parsing
    std::istringstream ss(inputString);

    // Input separator
    c = ss.get();
    if (c != '|') reportBadInputString(inputString, c);
    
    // Parsing controller inputs
    if (_controllerType == controller_t::gb) parseGBAInput(input.port, input.power, ss, inputString);

    // End separator
    c = ss.get();
    if (c != '|') reportBadInputString(inputString, c);

    // If its not the end of the stream, then extra values remain and its invalid
    c = ss.get();
    if (ss.eof() == false) reportBadInputString(inputString, c);
    
    // Returning input
    return input;
  };

  private:

  static void parseGBAInput(uint16_t& code, bool& power, std::istringstream& ss, const std::string& inputString)
  {
    // Currently read character
    char c;

    // Cleaning code
    code = 0;

    // Up
    c = ss.get();
    if (c != '.' && c != 'U') reportBadInputString(inputString, c);
    if (c == 'U') code |= GBA_KEY_UP;

    // Down
    c = ss.get();
    if (c != '.' && c != 'D') reportBadInputString(inputString, c);
    if (c == 'D') code |= GBA_KEY_DOWN;

    // Left
    c = ss.get();
    if (c != '.' && c != 'L') reportBadInputString(inputString, c);
    if (c == 'L') code |= GBA_KEY_LEFT;

    // Right
    c = ss.get();
    if (c != '.' && c != 'R') reportBadInputString(inputString, c);
    if (c == 'R') code |= GBA_KEY_RIGHT;

    // Start
    c = ss.get();
    if (c != '.' && c != 'S') reportBadInputString(inputString, c);
    if (c == 'S') code |= GBA_KEY_START;

    // Select
    c = ss.get();
    if (c != '.' && c != 's') reportBadInputString(inputString, c);
    if (c == 's') code |= GBA_KEY_SELECT;

    // B
    c = ss.get();
    if (c != '.' && c != 'B') reportBadInputString(inputString, c );
    if (c == 'B') code |= GBA_KEY_B;

    // A
    c = ss.get();
    if (c != '.' && c != 'A') reportBadInputString(inputString, c);
    if (c == 'A') code |= GBA_KEY_A;

    // P
    power = false;
    c = ss.get();
    if (c != '.' && c != 'P') reportBadInputString(inputString, c);
    if (c == 'P') power = true;
  }

  static inline void reportBadInputString(const std::string &inputString, const char c)
  {
    JAFFAR_THROW_LOGIC("Could not decode input string: '%s' - Read: '%c'\n", inputString.c_str(), c);
  }

  input_t _input;
  controller_t _controllerType;
}; // class InputParser

} // namespace jaffar