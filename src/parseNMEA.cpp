#include "earth.h"
#include "parseNMEA.h"

#include <regex>

namespace NMEA
{

  bool isWellFormedSentence(std::string sentence)
  {
      std::regex sentence_pattern("^\\$GP([A-Z]{3})((?:,[^,\\*$]*)*)\\*([0-9a-fA-F]{2})");
      return std::regex_match(sentence, sentence_pattern);
  }

  bool hasValidChecksum(std::string)
  {
      // Stub definition, needs implementing
      return false;
  }

  SentenceData extractSentenceData(std::string)
  {
      // Stub definition, needs implementing
      return {"",{}};
  }

  GPS::Position positionFromSentenceData(SentenceData)
  {
      // Stub definition, needs implementing
      return GPS::Earth::NorthPole;
  }

  Route routeFromLog(std::istream &)
  {
      // Stub definition, needs implementing
      return {};
  }

}
