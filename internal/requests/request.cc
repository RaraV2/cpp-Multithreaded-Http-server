#include "request.hpp"

HTTP_request requestFromReader(int conn) {
  char temp_tcp[BUFFER_SIZE];
  std::string buffer;
  ssize_t numberOfBytesReadFromTCP;

  HTTP_request request;

  while (request.state != ParserState::done &&
         request.state != ParserState::error) {

    ssize_t bytesToBeErased = request.parser(buffer);
    if (request.state == ParserState::done) {
      break;
    }
    // if bytesToBeErased == -1 so it means error exit the loop immmediately
    if (bytesToBeErased < 0) {
      break;
    }

    if (bytesToBeErased > 0) {
      buffer.erase(0, bytesToBeErased);
      continue;
    }

    // if bytes to be erased == 0, means we recive more data
    numberOfBytesReadFromTCP = recv(conn, temp_tcp, BUFFER_SIZE, 0);

    if (numberOfBytesReadFromTCP > 0) {
      buffer.append(temp_tcp, numberOfBytesReadFromTCP);
    }

    else if (numberOfBytesReadFromTCP == 0) {
      throw std::runtime_error("connection closed");
    }

    else {
      throw std::runtime_error("invalid data");
    }
  }

  if (request.state == ParserState::error) {
    throw std::runtime_error("invalid HTTP request");
  }

  return request;
};

size_t HTTP_request::parser(
    const std::string &rawBytes) // we have to return number of bytes processed
{
  switch (state) {
  case ParserState::requestLine: {
    auto [rl, consumed, error] = HTTPRequestLine_Parser(rawBytes);
    if (error == true) {
      state = ParserState::error;
      return -1;
    }
    if (consumed == 0)
      return consumed;
    requestLine = rl;
    state = ParserState::headers;
    return consumed;
  }

  case ParserState::headers: {
    auto [currHeader, consumed, done, error] = HTTPHeader_Parser(rawBytes);
    if (error == true) {
      std::cout << "HEADER ERROR\n";
      state = ParserState::error;
      return -1;
    } else if (done == true) {
      state = ParserState::body;
      return consumed;
    } else if (consumed == 0)
      return consumed;

    headers[currHeader.key].push_back(currHeader.value);
    return consumed;
  }

  case ParserState::body: {
    auto [contentLength, error] = GetInt(*this, "Content-Length");
    // No Content-Length so it has no body for this assignment
    if (error) {
      state = ParserState::done;
      return 0;
    }
    if (contentLength == 0) {
      state = ParserState::done;
      return 0;
    }

    // 1. Calculate how many bytes are left to fulfill Content-Length
    size_t bytesNeeded = contentLength - body.size();

    // 2. Limit our parsing step so we don't bleed into future requests
    size_t bytesToConsume = std::min(bytesNeeded, rawBytes.size());

    // 3. Append only the required slice of the string data window
    body.append(rawBytes, 0, bytesToConsume);

    // 4. Update the state machine transition flags
    if (body.size() == contentLength) {
      state = ParserState::done;
    }

    // 5. Return the true number of bytes consumed from requestFromReader's
    // buffer
    return bytesToConsume;
  }

  case ParserState::done: {
    return 0;
  }

  case ParserState::error: {
    return -1;
  }
  }
  return 0;
}

std::tuple<HTTP_requestLine, size_t, bool>
HTTPRequestLine_Parser(const std::string &rawBytes) {
  HTTP_requestLine requestLine;
  size_t pos;
  std::vector<std::string> temp_requestLine_split;

  pos = rawBytes.find(CRLF);
  if (pos == std::string::npos) {
    return {requestLine, 0, false};
  }
  std::string temp_requestLine;
  temp_requestLine = rawBytes.substr(0, pos);
  std::stringstream ss(temp_requestLine);
  std::string token;

  while (ss >> token) {
    temp_requestLine_split.push_back(token);
  }
  // if size = 4 means it has extra
  if (temp_requestLine_split.size() != 3)
    return {requestLine, 0, true};
  //
  // check if the method, request and version are valid or not and then put the
  // values
  //
  if (validMethods.find(temp_requestLine_split[0]) == validMethods.end()) {
    return {requestLine, 0, true};
  }

  if (temp_requestLine_split[1].empty() ||
      temp_requestLine_split[1][0] != '/') {
    return {requestLine, 0, true};
  }

  if (temp_requestLine_split[2] != "HTTP/1.1") {
    return {requestLine, 0, true};
  }

  requestLine.HTTP_method = temp_requestLine_split[0];
  requestLine.targetRequest = temp_requestLine_split[1];
  requestLine.HTTP_version = temp_requestLine_split[2].substr(5); // after /

  return {requestLine, pos + 2, false};
}

std::optional<std::string>
HTTP_request::Get(const std::string &header_name) const {
  std::string header_name_lower = header_name;
  std::transform(header_name_lower.begin(), header_name_lower.end(),
                 header_name_lower.begin(),
                 [](unsigned char c) { return std::tolower(c); });
  std::string value;

  if (headers.count(header_name_lower)) {
    auto &values = headers.at(header_name_lower);
    bool first = true;
    for (auto const &valuesPart : values) {
      if (!first)
        value.append(",");

      value.append(valuesPart);
      first = false;
    }
    return value;
  }
  return std::nullopt;
}

std::pair<size_t, bool> GetInt(const HTTP_request &request,
                               const std::string &header_name) {
  int value;
  auto valueStr = request.Get(header_name);
  if (!valueStr)
    return {0, true};

  try {
    value = std::stoi(*valueStr);
    if (value < 0)
      return {0, true}; // negative content length
    else
      return {static_cast<size_t>(value), false};
  }

  catch (...) {
    return {0, true};
  }
}
