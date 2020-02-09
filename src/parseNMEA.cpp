#include "earth.h"
#include "parseNMEA.h"

#include <regex>
#include <cassert>
#include <numeric>
#include <functional>
#include <stdexcept>
#include <unordered_map>

namespace NMEA
{
  const std::regex wellFormedSentence("^\\$GP[A-Z]{3}(?:,[^,\\*$]*)*\\*[0-9a-fA-F]{2}");
  const std::regex checksumGroups("\\$([^\\*]*)\\*(.*)");
  const std::regex dataGroups("GP(.{3})(.*)\\*");

  /* Indices in the data list, which correspond to the position data. */
  struct PositionFormat {
      size_t latitude;
      size_t northing;
      size_t longtitude;
      size_t easting;
      bool containsElevation;
      size_t elevation;
  };

  const std::unordered_map<std::string, PositionFormat> SupportedFormats {
      {"GLL", PositionFormat{0, 1, 2, 3, false, 0}},
      {"GGA", PositionFormat{1, 2, 3, 4, true, 8}},
      {"RMC", PositionFormat{2, 3, 4, 5, false, 0}}
  };

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

  GPS::Position parsePosition(SentenceData data, PositionFormat format)
  {
      try {
          std::string latitude = data.second.at(format.latitude);
          char northing = data.second.at(format.northing).at(0);
          std::string longtitude = data.second.at(format.longtitude);
          char easting = data.second.at(format.easting).at(0);
          std::string elevation = "0";

          if (format.containsElevation) elevation = data.second.at(format.elevation);

          return GPS::Position(latitude, northing, longtitude, easting, elevation);
      }
      catch (std::out_of_range) {
          throw std::invalid_argument("Necessary data fields missing or containing invalid data,");
      }
  }

  bool isSupportedFormat(std::string format)
  {
      return (SupportedFormats.find(format) != SupportedFormats.end());
  }

  GPS::Position positionFromSentenceData(SentenceData data)
  {
      std::string formatCode = data.first;
      if (isSupportedFormat(formatCode)) {
          return parsePosition(data, SupportedFormats.at(formatCode));
      }
      else throw std::invalid_argument(formatCode + " is an unsupported NMEA sentence data format.");
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
