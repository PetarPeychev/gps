#include "earth.h"
#include "parseNMEA.h"

#include <regex>
#include <cassert>
#include <numeric>
#include <functional>
#include <stdexcept>

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
      std::regex checksum_pattern("\\$([^\\*]*)\\*(.*)");
      std::smatch matches;
      std::regex_match(sentence, matches, checksum_pattern);
      std::string chars = matches[1];
      int checksum = stoi(matches[2], nullptr, 16);
      int parity = std::accumulate(chars.begin(), chars.end(), 0, std::bit_xor<int>());
      return parity == checksum;
  }

  SentenceData extractSentenceData(std::string sentence)
  {
      assert(isWellFormedSentence(sentence));
      std::regex sentence_data_pattern("GP(.{3})(.*)\\*");
      std::smatch matches;
      std::regex_search(sentence, matches, sentence_data_pattern);
      std::string format = matches[1];
      std::string data = matches[2];
      std::vector<std::string> fields = {};
      std::istringstream stringstream(data);
      std::string token;

      while(std::getline(stringstream, token, ','))
          fields.push_back(token);

      if (data.back() == ',')
          fields.push_back("");

      if (fields.size() > 0)
          fields.erase(fields.begin());

      return {format, fields};
  }

  GPS::Position positionFromSentenceData(SentenceData data)
  {
      if (data.first == "GLL") {
          if (data.second.size() == 5) {
              return GPS::Position(data.second[0],
                                   data.second[1][0],
                                   data.second[2],
                                   data.second[3][0],
                                   "0");
          }
          else throw std::invalid_argument("NMEA Sentence contains " + std::to_string(data.second.size()) + " data fields, expected 5 for GLL format.");
      }
      else if (data.first == "GGA") {
          if (data.second.size() == 14) {
              return GPS::Position(data.second[1],
                                   data.second[2][0],
                                   data.second[3],
                                   data.second[4][0],
                                   data.second[8]);
          }
          else throw std::invalid_argument("NMEA Sentence contains " + std::to_string(data.second.size()) + " data fields, expected 14 for GGA format.");
      }
      else if (data.first == "RMC") {
          if (data.second.size() == 11) {
              return GPS::Position(data.second[2],
                                   data.second[3][0],
                                   data.second[4],
                                   data.second[5][0],
                                   "0");
          }
          else throw std::invalid_argument("NMEA Sentence contains " + std::to_string(data.second.size()) + " data fields, expected 11 for RMC format.");
      }
      else throw std::invalid_argument(data.first + "is an unsupported NMEA sentence data format.");
  }

  Route routeFromLog(std::istream &)
  {
      // Stub definition, needs implementing
      return {};
  }

}
