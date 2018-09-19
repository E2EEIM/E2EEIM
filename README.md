This project created server and client desktop applications to ensure messages
sent by the client are encrypted. This means messages cannot be read with
understanding between the end points. Both client and server applications use
RSA keys to encrypt and decrypt messages with GnuPG. The client and server
applications are implemented in C++ using Qt as cross-platform applications.
Both applications can run on Linux, macOS and Windows operating systems. All
conversations and each user’s data are saved in the client machines. Users can
run their own server to have full control the of server and make conversations
more private.
