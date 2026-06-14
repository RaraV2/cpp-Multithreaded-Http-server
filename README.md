# HTTP/1.1 Server in C++

A multithreaded HTTP/1.1 server written in C++ using POSIX sockets. The implementation follows the HTTP semantics and messaging specifications defined in RFC 9110 and RFC 9112.

The server uses a dedicated listener thread to accept incoming connections and spawns worker threads to handle client requests concurrently.

## Features

* HTTP/1.1 request parsing

  * Request line parsing
  * Header parsing
  * Content-Length handling

* HTTP response serialization

  * Status line generation
  * Header generation
  * Response body serialization

* Static file serving

* Binary file and video streaming using incremental reads

* Proxy endpoints

  * Forwarding requests to httpbin.org
  * Streaming upstream responses to clients

* Chunked Transfer Encoding

  * Chunk generation
  * Incremental response streaming
  * End-of-stream chunk handling

* HTTP Trailers

  * Trailer header advertisement
  * Trailer serialization
  * SHA-256 response integrity trailer
  * Response length trailer

* SHA-256 hashing of streamed response bodies

* Concurrent request handling using worker threads

## What I learned

* TCP socket programming
* HTTP/1.1 message formatting and parsing
* Incremental and streaming I/O
* Chunked transfer encoding and trailers
* Binary data handling
* Multithreaded request processing
* Debugging protocol-level interoperability issues with clients and browsers
