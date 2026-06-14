#include "headers.hpp"

std::tuple<HTTP_Header, size_t, bool, bool>
HTTPHeader_Parser(const std::string &rawBytes) {
  HTTP_Header currHeader;
  size_t pos;
  std::vector<std::string> temp;
  pos = rawBytes.find("\r\n");
  if (pos == std::string::npos)
    return {currHeader, 0, false, false};
  else if (pos == 0)
    return {currHeader, pos + 2, true, false};
  std::string temp_header;
  temp_header = rawBytes.substr(0, pos);

  size_t colonPos;
  colonPos = temp_header.find(':');

  if (colonPos == std::string::npos)
    return {currHeader, 0, false, true};

  else if (colonPos == 0) // if (: localhost)
    return {currHeader, 0, false, true};

  else if (std::isspace(temp_header.at(
               colonPos - 1))) // check if there is a whitespace before :
    return {currHeader, 0, false, true};

  std::string key = temp_header.substr(0, colonPos);
  std::string value = trim(temp_header.substr(colonPos + 1));

  if (!fieldNameValidator(key)) {
    return {currHeader, 0, false, true};
  }

  std::transform(key.begin(), key.end(), key.begin(),
                 [](unsigned char c) { return std::tolower(c); });
  // to make field-line lowercase for (standardization of key)

  currHeader.key = key;
  currHeader.value = value;
  return {currHeader, pos + 2, false, false};
}

bool fieldNameValidator(const std::string &field_name) {
  std::string customAllowed = "!#$%&'*+-.^_`|~";

  for (char c : field_name) {
    unsigned char uc = static_cast<unsigned char>(c);

    bool isAlpha = std::isalpha(uc);
    bool isDigit = std::isdigit(uc);
    bool isSymbol = (customAllowed.find(c) != std::string::npos);

    // If it matches none of the RFC specifications, it is invalid
    if (!isAlpha && !isDigit && !isSymbol) {
      return false;
    }
  }
  return true;
}

std::string trim(const std::string &line_value) {
  const char *WhiteSpace = " \t\v\r\n";
  std::size_t start = line_value.find_first_not_of(WhiteSpace);

  if (start == std::string::npos)
    return "";

  std::size_t end = line_value.find_last_not_of(WhiteSpace);
  return line_value.substr(start, end - start + 1);
}
