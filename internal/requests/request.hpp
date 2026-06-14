#pragma once
#include "response.hpp"
#include "headers.hpp"
#include "sys/socket.h"
#include "unistd.h"
#include <algorithm>
#include <cctype>
#include <cstddef>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <sys/types.h>
#include <tuple>
#include <unordered_map>
#include <vector>
#include <algorithm>

constexpr std::string_view CRLF = "\r\n";
constexpr int BUFFER_SIZE{32};

enum class ParserState { requestLine, headers, body, done, error };

struct HTTP_requestLine {
  std::string HTTP_method;   // POSt or GET
  std::string targetRequest; // /cats
  std::string HTTP_version;  // HTTP/1.1 etc
};

struct HTTP_request {
  HTTP_requestLine requestLine{}; // http method target and version
  std::unordered_map<std::string, std::vector<std::string>>
      headers{};      // header section   ex) ContentLength = 22
  std::string body{}; // body section
  ParserState state = ParserState::requestLine;
  StatusCode Reponsdstatus = StatusCode::StatusOK; // default

  // struct same as class, defining methods now, you can also just give
  // declaration and define in other file by including .hpp file
  size_t parser(const std::string &);
  std::optional<std::string> Get(const std::string &) const;
};

static const std::unordered_set<std::string> validMethods{
    "GET",
    "POST",
};

HTTP_request requestFromReader(int conn);
std::tuple<HTTP_requestLine, size_t, bool>
HTTPRequestLine_Parser(const std::string &);
std::pair<size_t, bool> GetInt(const HTTP_request &, const std::string &);
