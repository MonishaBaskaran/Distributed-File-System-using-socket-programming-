# Distributed-File-System-using-socket-programming-

# Overview
This project implements a distributed file system using socket programming. The system consists of three servers:

Smain (main server)
Spdf (for handling PDF files)
Stext (for handling text files)
Clients interact exclusively with Smain, and the system handles file transfers to the appropriate servers in the background. The system supports multiple client connections and various file operations such as upload, download, delete, and more.
