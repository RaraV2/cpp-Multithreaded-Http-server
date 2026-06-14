#include "response.hpp"

std::error_code Writer::WriteStatusLine(const StatusCode &status) {

  if (WriterState != WriterStatus::RequestLine) {
    throw std::runtime_error(
        "HTTP Serialization Error: Called WriteStatusLine out of order.");
  }

  switch (status) {
  case StatusCode::StatusOK: {
    std::string response = "HTTP/1.1 200 OK\r\n";
    if (send(client_fd, response.data(), response.size(), 0) < 0) {
      return std::error_code(errno, std::generic_category());
    }
    break;
  }
  case StatusCode::StatusBadRequest: {
    std::string response = "HTTP/1.1 200 OK\r\n";
    if (send(client_fd, response.data(), response.size(), 0) < 0) {
      return std::error_code(errno, std::generic_category());
    }
    break;
  }
  case StatusCode::StatusInternalServerError: {
    std::string response = "HTTP/1.1 200 OK\r\n";
    if (send(client_fd, response.data(), response.size(), 0) < 0) {
      return std::error_code(errno, std::generic_category());
    }
    break;
  }
  default: {
    std::string response =
        "HTTP/1.1 " + std::to_string(static_cast<int>(status)) + "\r\n";
    if (send(client_fd, response.data(), response.size(), 0) < 0) {
      return std::error_code(errno, std::generic_category());
    }
    break;
  }
  }
  WriterState = WriterStatus::Headers;
  return std::error_code();
}

Headers Writer::GetDefaultHeaders(int content_length) {
  Headers headers;
  headers.headersMap.emplace(
      "Content-Length",
      std::vector<std::string>{std::to_string(content_length)});
  headers.headersMap.emplace("Connection", std::vector<std::string>{("close")});
  headers.headersMap.emplace("Content-Type",
                             std::vector<std::string>{("text/plain")});
  return headers;
}

std::error_code Writer::WriteHeaders(const Headers &headers) {

  if (WriterState != WriterStatus::Headers) {
    throw std::runtime_error(
        "HTTP Serialization Error: Called WriteHeaders out of order.");
  }
  std::string response{};
  for (const auto &[key, values] : headers.headersMap) {
    for (const auto &value : values) {
      // Put the whole layout inside the loop so every value gets its own line
      response += key + ": " + value + "\r\n";
    }
  }
  response += "\r\n"; // Trailing blank line to signal end of headers

  if (send(client_fd, response.data(), response.size(), 0) < 0) {
    return std::error_code(errno, std::generic_category());
  }

  WriterState = WriterStatus::Body;
  return std::error_code();
}

std::pair<int, std::error_code> Writer::WriteBody(std::vector<uint8_t> &body) {

  if (WriterState != WriterStatus::Body) {
    throw std::runtime_error(
        "HTTP Serialization Error: Called WriteBody out of order.");
  }

  ssize_t bytes_sent = send(client_fd, body.data(), body.size(), 0);
  if (bytes_sent < 0)
    return {0, std::error_code(errno, std::generic_category())};

  return {bytes_sent, std::error_code()};
}

std::error_code Writer::WriteTrailers(Headers &trailers) {

  if (WriterState != WriterStatus::Body) {
    throw std::runtime_error(
        "HTTP Serialization Error: Called WriteTrailers out of order.");
  }

  std::string serialized;

  for (const auto &[key, values] : trailers.headersMap) {
    for (const auto &value : values) {
      serialized += key;
      serialized += ": ";
      serialized += value;
      serialized += "\r\n";
    }
  }

  serialized += "\r\n";

  std::vector<uint8_t> bytes(serialized.begin(), serialized.end());

  auto [written, err] = WriteBody(bytes);

  if (!err) {
    WriterState = WriterStatus::Finish;
  }

  return err;
}
