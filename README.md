# Distributed File System using Socket Programming

## Overview
This project implements a distributed file system using socket programming. The system consists of three servers: 
- **Smain** (main server)
- **Spdf** (for handling PDF files)
- **Stext** (for handling text files)

Clients interact exclusively with **Smain**, and the system handles file transfers to the appropriate servers in the background. The system supports multiple client connections and various file operations such as upload, download, delete, and more.

## Features
- **File Upload**: Clients can upload `.c`, `.pdf`, and `.txt` files. While `.c` files are stored locally in **Smain**, `.pdf` files are sent to **Spdf**, and `.txt` files are sent to **Stext**.
- **File Download**: Clients can download files from **Smain**, including files stored on **Spdf** and **Stext**, which are retrieved via **Smain**.
- **File Deletion**: Clients can request to delete files from the distributed system.
- **Directory Listing**: Clients can list files in directories, with **Smain** collecting files from **Spdf** and **Stext** to present a unified view.
- **Tarball Creation**: Clients can request a tarball of all files of a specific type (`.c`, `.pdf`, or `.txt`), aggregated from **Smain**, **Spdf**, and **Stext** as needed.

## Components
The system consists of the following components:
1. **Smain.c**: The main server that handles all client communication, file storage for `.c` files, and proxies requests for `.pdf` and `.txt` files to **Spdf** and **Stext** respectively.
2. **Spdf.c**: The server responsible for storing and handling `.pdf` files.
3. **Stext.c**: The server responsible for storing and handling `.txt` files.
4. **client24s.c**: The client application that users interact with to upload, download, and manage files.

## Client Commands
The client supports the following commands:

1. **ufile `<filename>` `<destination_path>`**
   - Uploads the specified file to the destination path.
   - `.c` files are stored on **Smain**, `.pdf` files are sent to **Spdf**, and `.txt` files are sent to **Stext**.
   - Example:
     ```bash
     client24s$ ufile sample.c ~smain/folder1/folder2
     client24s$ ufile sample.txt ~smain/folder1/folder2
     ```

2. **dfile `<filename>`**
   - Downloads the specified file from the server to the client’s working directory.
   - Example:
     ```bash
     client24s$ dfile ~smain/folder1/folder2/sample.c
     ```

3. **rmfile `<filename>`**
   - Deletes the specified file from the server.
   - Example:
     ```bash
     client24s$ rmfile ~smain/folder1/folder2/sample.pdf
     ```

4. **dtar `<filetype>`**
   - Downloads a tarball of all files of the specified type from the server.
   - Example:
     ```bash
     client24s$ dtar .txt
     ```

5. **display `<pathname>`**
   - Lists all `.c`, `.pdf`, and `.txt` files in the specified directory path.
   - Example:
     ```bash
     client24s$ display ~smain/folder1/folder2
     ```

## System Architecture
The distributed file system consists of the following elements:
- **Smain**: Acts as the central server for clients. It stores `.c` files locally and forwards `.pdf` and `.txt` files to **Spdf** and **Stext**, respectively. Clients are unaware of the background processes and assume all files are stored in **Smain**.
- **Spdf**: A secondary server responsible for handling all `.pdf` files.
- **Stext**: A secondary server responsible for handling all `.txt` files.

### Communication
All communication between the servers and clients is conducted using socket programming. **Smain** listens for client requests and forks a new process for each connection, allowing for multiple simultaneous client interactions.

## Prerequisites
- **Operating System**: Linux or any UNIX-based system (for handling file paths and directories).
- **Socket Programming Knowledge**: Familiarity with sockets, for inter-process communication.
- **C Compiler**: Ensure you have a C compiler (e.g., `gcc`) installed to compile the source files.

## Installation and Setup
1. Clone the repository:
   ```bash
   git clone https://github.com/yourusername/distributed-file-system.git
   cd distributed-file-system
   ```
2. Compile the source files:
   ```bash
   gcc -o smain Smain.c
   gcc -o spdf Spdf.c
   gcc -o stext Stext.c
   gcc -o client24s client24s.c
   ```
3. Start each server on a different terminal:
   ```bash
   ./smain
   ./spdf
   ./stext
   ```
4. Start the client:
   ```bash
   ./client24s
   ```
## Usage
- Use the client commands listed above to interact with the system.
- All servers must be running simultaneously for the system to function properly.

