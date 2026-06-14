#include "server_services.hpp"

void HTTP_Server::Serve(uint16_t PORT,
                        std::optional<HandlerError> (*handler_fxn)(
                            Writer &, const HTTP_request *)) {
  // binding temporary handler function addr(user_handler) to permanent function
  // addr(handler_cb) inside object's private member
  this->handler = handler_fxn;
  sockaddr_in server_listenDetails{}; // tells from which ip and port
                                      // should server bind and listen to

  server_listen_fd =
      socket(AF_INET, SOCK_STREAM,
             0); // creates a ipv4 socket type, tcp protocol, default protool 0

  if (server_listen_fd < 0) {
    throw std::runtime_error("Socket formation failed");
  }

  // tells our application1's address from what family (ipv4/6), which port it
  // wants to use, actual ip address used
  server_listenDetails.sin_family = AF_INET;
  server_listenDetails.sin_port = htons(PORT);
  server_listenDetails.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

  // now bind server socket to requedted socket requirements we made wanted (i.e
  // port 42069)

  if (bind(server_listen_fd, (sockaddr *)&server_listenDetails,
           sizeof(server_listenDetails)) < 0) {
    throw std::runtime_error("Socket binding failed");
  }

  if (listen(server_listen_fd, 10)) {
    throw std::runtime_error("Socket binding failed");
  }
}

void HTTP_Server::Listen() {
  std::thread background_listener_thread([this]() {
    while (running) {
      int client_fd = accept(server_listen_fd, nullptr, nullptr);

      if (client_fd < 0) {
        if (!running)
          break;

        perror("accept failed");
        continue;
      }
      {
        std::lock_guard<std::mutex> lock(client_mtx);
        active_client_connections.push_back(client_fd);
      }
      std::thread client_thread([this, client_fd]() {
        Handle(client_fd);
        {
          std::lock_guard<std::mutex> lock(client_mtx);
          active_client_connections.erase(
              std::remove(active_client_connections.begin(),
                          active_client_connections.end(), client_fd),
              active_client_connections.end());
        }
      });
      client_thread.detach();
    }
  });

  background_listener_thread.detach();
  return;
}

void HTTP_Server::Handle(int client_connection) {
  try {

    HTTP_request request = requestFromReader(client_connection);
    Writer response(client_connection);

    // Call the handler function pointer passing our stream and request
    if (handler != nullptr) {
      handler(response, &request);
    }
    close(client_connection);
    return;
  }

  catch (const std::exception &e) {
    std::cerr << "Encountered error in worker thread: " << e.what() << '\n';
    close(client_connection);
    return;
  }
}

std::error_code HTTP_Server::Close() {
  running = false;

  if (server_listen_fd >= 0) {
    int result = close(server_listen_fd);
    server_listen_fd = -1;

    {
      std::lock_guard<std::mutex> lock(client_mtx);
      for (int x : active_client_connections) {
        // Forcefully wake up any thread stuck waiting for data on recv()
        shutdown(x, SHUT_RDWR);
        close(x);
      }
      active_client_connections.clear();
    }

    // 2. POSIX close() returns -1 on failure
    if (result > 0) {
      // Returns the exact operating system error code (like EBADF)
      return std::error_code(errno, std::generic_category());
    }
  }

  std::cout << "\nServer shhutdowns gracefully... \n";
  return std::error_code();
}

std::optional<HandlerError> Handler(Writer &w, const HTTP_request *req) {

  if (!req) {
    std::string_view responseBody =
        "<html><head><title>400 Bad Request</title></head><body><h1>Bad "
        "Request</h1><p>Your request honestly kinda sucked.</p></body></html>";
    auto err = w.WriteStatusLine(StatusCode::StatusBadRequest);

    if (!err) {
      auto headers = w.GetDefaultHeaders(responseBody.size());
      headers.ReplaceHeader("Content-Type", "text/html");
      err = w.WriteHeaders(headers);
    }

    if (err) {
      std::cerr << "Formatting failed: " << err.message() << '\n';
      return std::nullopt;
    }
    std::vector<uint8_t> body(responseBody.begin(), responseBody.end());
    w.WriteBody(body);
    return std::nullopt;
  }

  // 1. If req is present extract path cleanly using your HTTP_request
  std::string_view path = req->requestLine.targetRequest;

  if (path == "/yourproblem") {
    std::string_view responseBody =
        "<html><head><title>400 Bad Request</title></head><body><h1>Bad "
        "Request</h1><p>Your request honestly kinda sucked.</p></body></html>";
    auto err = w.WriteStatusLine(StatusCode::StatusBadRequest);

    if (!err) {
      auto headers = w.GetDefaultHeaders(responseBody.size());
      headers.ReplaceHeader("Content-Type", "text/html");
      err = w.WriteHeaders(headers);
    }

    if (err) {
      std::cerr << "Formatting failed: " << err.message() << '\n';
      return std::nullopt;
    }
    std::vector<uint8_t> body(responseBody.begin(), responseBody.end());
    w.WriteBody(body);
    return std::nullopt;
  }

  // If target path is /myproblem, return a 500
  else if (path == "/myproblem") {
    std::string_view responseBody =
        "<html><head><title>500 Internal Server "
        "Error</title></head><body><h1>Internal Server Error</h1><p>Okay, "
        "you " // compile time string concatenation
        "know what? This one is on me.</p></body></html>";
    auto err = w.WriteStatusLine(StatusCode::StatusInternalServerError);

    if (!err) {
      auto headers = w.GetDefaultHeaders(responseBody.size());
      headers.ReplaceHeader("Content-Type", "text/html");
      err = w.WriteHeaders(headers);
    }

    if (err) {
      std::cerr << "Formatting failed: " << err.message() << '\n';
      return std::nullopt;
    }
    std::vector<uint8_t> body(responseBody.begin(), responseBody.end());
    w.WriteBody(body);
    return std::nullopt;
  }

  else if (path.starts_with("/httpbin/stream/")) {
    constexpr std::string_view prefix = "/httpbin/stream/";
    std::string id = std::string(path.substr(prefix.size()));

    auto err = w.WriteStatusLine(StatusCode::StatusOK);

    if (!err) {
      auto headers = w.GetDefaultHeaders(0);
      headers.RemoveHeader("Content-Length");
      headers.ReplaceHeader("Content-Type", "text/plain");
      headers.SetHeader("Transfer-Encoding", "chunked");
      err = w.WriteHeaders(headers);
    }

    if (err) {
      std::cerr << "Formatting failed: " << err.message() << '\n';
      return std::nullopt;
    }

    httplib::Client cli("httpbin.org");
    std::string stream_path = "/stream/" + id;

    auto now = []() {
      return std::chrono::duration_cast<std::chrono::milliseconds>(
                 std::chrono::steady_clock::now().time_since_epoch())
          .count();
    };

    std::cout << now() << " Before Get()\n";

    cli.Get(stream_path.c_str(), [&](const char *data, size_t data_length) {
      std::cout << now() << " Callback: received " << data_length << " bytes\n";

      size_t offset = 0;

      while (offset < data_length) {
        size_t n = std::min(data_length - offset, static_cast<size_t>(32));

        std::stringstream hex_stream;
        hex_stream << std::hex << n << "\r\n";

        std::string chunk_size_hex = hex_stream.str();

        std::vector<uint8_t> hex_bytes(chunk_size_hex.begin(),
                                       chunk_size_hex.end());

        w.WriteBody(hex_bytes);

        std::vector<uint8_t> data_chunk(data + offset, data + offset + n);

        w.WriteBody(data_chunk);

        std::vector<uint8_t> crlf = {'\r', '\n'};
        w.WriteBody(crlf);

        offset += n;
      }

      return true;
    });

    std::cout << now() << " After Get()\n";

    std::vector<uint8_t> final_chunk = {'0', '\r', '\n', '\r', '\n'};

    w.WriteBody(final_chunk);
    return std::nullopt;
  }

  else if (path == "/httpbin/html") {
    auto err = w.WriteStatusLine(StatusCode::StatusOK);

    if (!err) {
      auto headers = w.GetDefaultHeaders(0);
      headers.RemoveHeader("Content-Length");
      headers.ReplaceHeader("Content-Type", "text/html");
      headers.ReplaceHeader("Transfer-Encoding", "chunked");

      headers.SetHeader("Trailer", "X-Content-SHA256");
      headers.SetHeader("Trailer", "X-Content-Length");

      err = w.WriteHeaders(headers);
    }

    if (err) {
      std::cerr << "Formatting failed: " << err.message() << '\n';
      return std::nullopt;
    }

    httplib::Client cli("httpbin.org");
    std::vector<uint8_t> full_body;
    cli.Get("/html", [&](const char *data, size_t data_length) {
      full_body.insert(full_body.end(), data, data + data_length);

      std::stringstream ss;
      ss << std::hex << data_length << "\r\n";

      std::string chunk_header = ss.str();

      std::vector<uint8_t> chunk_header_bytes(chunk_header.begin(),
                                              chunk_header.end());
      w.WriteBody(chunk_header_bytes);

      std::vector<uint8_t> body_bytes(data, data + data_length);
      w.WriteBody(body_bytes);

      std::vector<uint8_t> crlf = {'\r', '\n'};
      w.WriteBody(crlf);

      return true;
    });

    // end of chunks
    std::vector<uint8_t> endOfChunked({'0', '\r', '\n'});
    w.WriteBody(endOfChunked);
    // compute hash
    std::string full_body_str(full_body.begin(), full_body.end());
    std::string hash_hex = sha256(full_body_str);

    Headers trailers;
    trailers.SetHeader("X-Content-Length",
                       std::to_string(full_body_str.size()));
    trailers.SetHeader("X-Content-SHA256", hash_hex);
    w.Writer::WriteTrailers(trailers);
    return std::nullopt;
  }

  else if (path == "/video") {

    static int req_id = 0;
    int my_id = ++req_id;

    std::cout << "VIDEO REQUEST #" << my_id << '\n';

    std::ifstream video("assets/vim.mp4",std::ios::binary);
    video.seekg(0, std::ios::end);
    size_t video_size = video.tellg();
    video.seekg(0, std::ios::beg);

    auto err = w.WriteStatusLine(StatusCode::StatusOK);

    auto headers = w.GetDefaultHeaders(video_size);
    headers.ReplaceHeader("Content-Type", "video/mp4");

    err = w.WriteHeaders(headers);

    constexpr size_t BUFFER_SIZE = 8192;

    std::vector<uint8_t> buffer(BUFFER_SIZE);

    while (video) {
      video.read(reinterpret_cast<char *>(buffer.data()), buffer.size());

      std::streamsize bytes_read = video.gcount();

      if (bytes_read <= 0) {
        break;
      }

      std::vector<uint8_t> video_chunk(buffer.begin(),
                                       buffer.begin() + bytes_read);

      auto [written, err] = w.WriteBody(video_chunk);

      if (err) {
        if (err.value() == ECONNRESET) {
          std::cout << "VIDEO REQUEST #" << my_id << " disconnected\n";
        }
        break;
      }
    }
    return std::nullopt;
  }

  else {
    std::string_view responseBody =
        "<html><head><title>200 "
        "OK</title></head><body><h1>Success!</h1><p>Your request was an "
        "absolute banger.</p></body></html>";
    auto err = w.WriteStatusLine(StatusCode::StatusOK);

    if (!err) {
      auto headers = w.GetDefaultHeaders(responseBody.size());
      headers.ReplaceHeader("Content-Type", "text/html");
      err = w.WriteHeaders(headers);
    }

    if (err) {
      std::cerr << "Formatting failed: " << err.message() << '\n';
      return std::nullopt;
    }
    std::vector<uint8_t> body(responseBody.begin(), responseBody.end());
    w.WriteBody(body);
    return std::nullopt;
  }
}

std::string sha256(const std::string &str) {
  unsigned char hash[SHA256_DIGEST_LENGTH];
  SHA256_CTX sha256;
  SHA256_Init(&sha256);
  SHA256_Update(&sha256, str.c_str(), str.size());
  SHA256_Final(hash, &sha256);
  std::stringstream ss;
  for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
    ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
  }
  return ss.str();
}
