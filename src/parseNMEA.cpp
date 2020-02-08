#include "earth.h"
#include "parseNMEA.h"

#include <regex>
#include <cassert>
#include <numeric>
#include <functional>

namespace NMEA
{

  bool isWellFormedSentence(std::string sentence)
  {
      std::regex well_formed_pattern("^\\$GP[A-Z]{3}(?:,[^,\\*$]*)*\\*[0-9a-fA-F]{2}");
      return std::regex_match(sentence, well_formed_pattern);
  }

  bool hasValidChecksum(std::string sentence)
  {
      assert(isWellFormedSentence(sentence));

      std::regex checksum_pattern("^\\$(GP[A-Z]{3}(?:,[^,\\*$]*)*)\\*([0-9a-fA-F]{2})");
      std::smatch matches;
      std::regex_match (sentence, matches, checksum_pattern);
      std::string chars = matches[1];
      int checksum = stoi(matches[2], nullptr, 16);
      int parity = std::accumulate(chars.begin(), chars.end(), 0, std::bit_xor<int>());
      return parity == checksum;
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
