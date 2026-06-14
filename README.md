# CPP-Multithreaded-HTTP-Server

A multithreaded HTTP/1.1 server written in C++ using POSIX sockets. The implementation follows the HTTP semantics and messaging specifications defined in RFC 9110 and RFC 9112.

The server uses a dedicated listener thread to accept incoming connections and spawns worker threads to handle client requests concurrently.

## Features

### Core HTTP

* HTTP/1.1 request parsing
* Request line parsing
* Header parsing
* Response serialization
* Content-Length handling
* Static file serving

### Streaming

* Binary file streaming
* Video streaming using incremental reads from disk
* Proxy streaming from upstream HTTP servers

### Transfer Encodings

* Chunked Transfer Encoding
* Incremental chunk generation
* End-of-stream chunk handling

### HTTP Trailers

* Trailer header advertisement
* Trailer serialization
* SHA-256 response integrity trailer
* Response length trailer

### Proxying

* Upstream HTTP requests using cpp-httplib
* Streaming responses from httpbin.org to connected clients

### Concurrency

* Dedicated listener thread
* Worker thread per client connection

## Project Structure

```text
.
├── Http-Server/
│   └── server.cc
│
├── internal/
│   ├── Header/
│   ├── Request/
│   ├── Response/
│   └── Server_services/
│
├── 3rd-party-library-used/
│
└── README.md
```

## Build Requirements

* C++20 compiler
* CMake 3.16+
* OpenSSL

### Arch Linux

```bash
sudo pacman -S --needed base-devel cmake openssl
```

### Build

```bash
git clone <repo-url>
cd CPP-Multithreaded-HTTP-Server

cmake -B build
cmake --build build

./build/server
```

## Third-Party Libraries

* cpp-httplib (MIT License) by Yuji Hirose

  * https://github.com/yhirose/cpp-httplib

Used as the HTTP client for upstream proxy requests and streamed responses.

## References

* RFC 9110 - HTTP Semantics
* RFC 9112 - HTTP/1.1
