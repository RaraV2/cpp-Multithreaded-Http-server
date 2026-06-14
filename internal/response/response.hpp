#pragma once
#include <netinet/in.h>
#include <sys/socket.h>
#include <system_error>
#include <unordered_map>
#include <vector>
#include <stdexcept>
#include <string>
#include <utility>

enum class StatusCode {
  StatusOK = 200,
  StatusBadRequest = 400,
  StatusInternalServerError = 500
};

enum class WriterStatus { RequestLine, Headers, Body, Finish };

struct Headers {
  std::unordered_map<std::string, std::vector<std::string>> headersMap;

  void ReplaceHeader(const std::string &key, const std::string &value) {
    headersMap[key] = {value};
  }
  void RemoveHeader(const std::string &key) { headersMap.erase(key); }

  void SetHeader(const std::string &key, const std::string &value) {
    headersMap[key].push_back(value);
  }
};

class Writer {

private:
  int client_fd;
  WriterStatus WriterState{};

public:
  explicit Writer(int fd)
      : client_fd(fd), WriterState(WriterStatus::RequestLine) {} // constructor

  std::error_code WriteStatusLine(const StatusCode &);
  Headers GetDefaultHeaders(int);
  std::error_code WriteHeaders(const Headers &);
  std::pair<int, std::error_code> WriteBody(std::vector<uint8_t> &);
  std::error_code WriteTrailers(Headers &);
};
