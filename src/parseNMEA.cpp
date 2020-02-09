#include "earth.h"
#include "parseNMEA.h"

#include <regex>
#include <cassert>
#include <numeric>
#include <functional>
#include <stdexcept>

namespace NMEA
{
  const std::regex wellFormedSentence("^\\$GP[A-Z]{3}(?:,[^,\\*$]*)*\\*[0-9a-fA-F]{2}");
  const std::regex checksumGroups("\\$([^\\*]*)\\*(.*)");
  const std::regex dataGroups("GP(.{3})(.*)\\*");

  bool isWellFormedSentence(std::string sentence)
  {
      return std::regex_match(sentence, wellFormedSentence);
  }

  bool hasValidChecksum(std::string sentence)
  {
      assert(isWellFormedSentence(sentence));

      std::smatch matches;
      std::regex_match(sentence, matches, checksumGroups);
      std::string chars = matches[1];
      int checksum = std::stoi(matches[2], nullptr, 16);
      int parity = std::accumulate(chars.begin(), chars.end(), 0, std::bit_xor<int>());

      return parity == checksum;
  }

  SentenceData extractSentenceData(std::string sentence)
  {
      assert(isWellFormedSentence(sentence));

      std::smatch matches;
      std::regex_search(sentence, matches, dataGroups);
      std::string format = matches[1];
      std::string data = matches[2];
      std::istringstream stringStream(data);
      std::vector<std::string> fields = {};
      std::string token;

      while(std::getline(stringStream, token, ','))
          fields.push_back(token);

      /* std::getline skips the last element if empty,
       * so it must be added manually. */
      if (data.back() == ',') fields.push_back("");

      /* Comma-separated list begins with a leading comma,
       * which shouldn't be parsed as an empty data field. */
      if (fields.size() > 0) fields.erase(fields.begin());

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

  Route routeFromLog(std::istream &istream)
  {
      Route route;
      std::string sentence;
      while(std::getline(istream, sentence)) {
          if (isWellFormedSentence(sentence) && hasValidChecksum(sentence)) {
              try {
                  SentenceData data = extractSentenceData(sentence);
                  GPS::Position position = positionFromSentenceData(data);
                  route.push_back(position);
              }
              catch(std::invalid_argument) {
                /* If the sentence format is not supported or
                 * does not contain valid data, ignore it. */
              }
          }
      }
      return route;
  }

}
