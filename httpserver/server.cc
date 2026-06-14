#include "server_services.hpp"

constexpr uint16_t PORT{42069};

int main() {

  sigset_t set;
  sigemptyset(&set);
  sigaddset(&set, SIGINT);

  // Block SIGINT delivery so sigwait can catch it instead
  pthread_sigmask(SIG_BLOCK, &set, nullptr);

  HTTP_Server Server;
  Server.Serve(PORT, Handler);
  Server.Listen();

  int sig;
  // This line completely blocks execution until Ctrl+C is pressed
  sigwait(&set, &sig);
  Server.Close();

  return 0;
}
