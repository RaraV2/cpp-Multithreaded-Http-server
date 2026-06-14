#include "request.hpp"
#include "response.hpp"
#include "httplib.h"
#include <atomic>
#include <cstdint>
#include <mutex>
#include <netinet/in.h>
#include <openssl/evp.h>
#include <openssl/sha.h>
#include <ostream>
#include <sys/socket.h>
#include <system_error>
#include <unistd.h>
#include <vector>
#include <stdexcept>
#include <string>


struct HandlerError {
  StatusCode status_code;
  std::string message;

  // writes a Handler Error
  void write_to(std::ostream &w) const {
    w << "HTTP/1.1 " << static_cast<int>(status_code) << " Error\r\n"
      << "Content-Length: " << message.length() << "\r\n"
      << "Content-Type: text/plain\r\n\r\n"
      << message;
  }
};

class HTTP_Server {
private:
  int server_listen_fd{-1};
  std::atomic<bool> running{true};
  std::mutex client_mtx;
  std::vector<int> active_client_connections;
  std::optional<HandlerError> (*handler)(Writer &,
                                         const HTTP_request *){nullptr};

public:
  void Serve(uint16_t PORT,
             std::optional<HandlerError> (*handler_fxn)(Writer &,
                                                        const HTTP_request *));
  void Listen();
  void Handle(int);
  std::error_code Close();
};

std::optional<HandlerError> Handler(Writer &, const HTTP_request *);
std::string sha256(const std::string &);
