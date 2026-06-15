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
├── cmakelists.txt
├── readme.md
├── .gitignore
│
├── httpserver/
│   └── server.cc
│
├── internal/
│   ├── headers/
│   │   ├── headers.hpp
│   │   └── headers.cc
│   │
│   ├── requests/
│   │   ├── request.hpp
│   │   └── request.cc
│   │
│   ├── response/
│   │   ├── response.hpp
│   │   └── response.cc
│   │
│   └── server_services/
│       ├── server_services.hpp
│       └── server_services.cc
│
├── assets/
│   └── vim.mp4
│
└── 3rd-party-library/
    └── httplib.h
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
cd Multithreaded-Http-server-in-Cpp

cmake -B build
cmake --build build

./build/server
```
## Configuration

The listening port is currently configured in `httpserver/server.cc`.

By default the server runs on:

```text
http://localhost:42069
```

To use a different port, modify the port value in `server.cc`, rebuild the project, and run the server again.

Example:

```cpp
constexpr int PORT = 8080;
```

## Available Routes

### GET /myproblem

Returns a custom **500 Internal Server Error** page.

```bash
curl http://localhost:42069/myproblem
```

---

### GET /yourproblem

Returns a custom **400 Bad Request** page.

```bash
curl http://localhost:42069/yourproblem
```

---

### GET /httpbin/stream/{n}

Fetches a streaming response from httpbin.org and forwards it to the client using **HTTP chunked transfer encoding**.

Example:

```bash
curl http://localhost:42069/httpbin/stream/100
```

This endpoint demonstrates:

* Fetching data from another server
* Streaming responses
* Chunked transfer encoding
* Sending data to the client as it arrives

---

### GET /httpbin/html

Fetches the `/html` endpoint from httpbin.org and forwards it using **chunked transfer encoding** with **HTTP trailers**.

Example:

```bash
curl --raw http://localhost:42069/httpbin/html
```

Trailer headers included:

* `X-Content-Length`
* `X-Content-SHA256`

This endpoint demonstrates:

* Chunked transfer encoding
* HTTP trailers
* Streaming HTML content
* SHA-256 integrity verification

---

### GET /video

Streams an MP4 video file to the client.

```bash
curl http://localhost:42069/video --output video.mp4
```

The downloaded file can be played by any compatible media player.

You can also open the endpoint directly in a browser:

```text
http://localhost:42069/video
```

Most modern browsers will automatically play the video or provide built-in playback controls.

---

### Any Other Route

Any route that does not match one of the endpoints above returns a custom **200 OK** HTML page.

Example:

```bash
curl http://localhost:42069/hello
```

Response:

```html
<html>
<head><title>200 OK</title></head>
<body>
<h1>Success!</h1>
<p>Your request was an absolute banger.</p>
</body>
</html>
```


## Third-Party Libraries

* cpp-httplib (MIT License) by Yuji Hirose

  * https://github.com/yhirose/cpp-httplib

Used as the HTTP client for upstream proxy requests and streamed responses.

## References

* RFC 9110 - HTTP Semantics
* RFC 9112 - HTTP/1.1
