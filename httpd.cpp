#include "httpd.h"
#include <arpa/inet.h>
#include <ctype.h>
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <limits.h>
#include <locale>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/sendfile.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <time.h>
#include <unistd.h>

static const int MAXPENDING = 5; // Maximum outstanding connection requests
static const int MAXBUFFERSIZE =
    8192; // As per specification; HTTP Request is assumed to be max at 8KB

void start_httpd(unsigned short port, string doc_root) {

  // This server's Port.
  in_port_t servPort = port;
  int servSock;

  if ((servSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
    std::cout << "ERROR"
              << std::endl; // DieWithSystemMessage("socket() failed");

  // Construct local address structure
  struct sockaddr_in servAddr;                  // Local address
  memset(&servAddr, 0, sizeof(servAddr));       // Zero out structure
  servAddr.sin_family = AF_INET;                // IPv4 address family
  servAddr.sin_addr.s_addr = htonl(INADDR_ANY); // Any incoming interface
  servAddr.sin_port = htons(servPort);          // Local port

  // Bind to the local address
  if (bind(servSock, (struct sockaddr *)&servAddr, sizeof(servAddr)) < 0)
    std::cout << "ERROR" << std::endl; // DieWithSystemMessage("bind() failed");

  // Mark the socket so it will listen for incoming connections
  if (listen(servSock, MAXPENDING) < 0)
    std::cout << "ERROR"
              << std::endl; // DieWithSystemMessage("listen() failed");

  for (;;) { // Run forever

    struct sockaddr_in clntAddr; // Client address
    // Set length of client address structure (in-out parameter)
    socklen_t clntAddrLen = sizeof(clntAddr);

    // Wait for a client to connect
    int clntSock = accept(servSock, (struct sockaddr *)&clntAddr, &clntAddrLen);
    if (clntSock < 0)
      std::cout << "ERROR"
                << std::endl; // DieWithSystemMessage("accept() failed");

    char clntName[INET_ADDRSTRLEN]; // String to contain client address
    if (inet_ntop(AF_INET, &clntAddr.sin_addr.s_addr, clntName,
                  sizeof(clntName)) != NULL)
      printf("Handling client %s/%d\n", clntName, ntohs(clntAddr.sin_port));
    else
      puts("Unable to get client address");

    HandleHTTPRequest(clntSock, doc_root);
  }
}

/*
    //Main driver of a single http response/request
    //Control of program flow
    //Request/Response flow.
*/
void HandleHTTPRequest(int clntSocket, string doc_root) {

  char buffer[MAXBUFFERSIZE]; // HTTP Request data that is read from the socket,
                              // to be stored here.

  std::size_t amountRead = PopulateBuffer(
      buffer, MAXBUFFERSIZE, clntSocket); // also how would you deal with two
                                          // requests, and partial data input

  (void)amountRead;

  HTTPRequest *hRequest = GenerateHTTPRequest(buffer);

  CheckHTTPRequestFormat(hRequest, doc_root);

  HTTPResponse *hResponse = GenerateHTTPResponse(hRequest);

  sendHTTPResponse(clntSocket, hResponse);

  delete hRequest;

  delete hResponse;

  close(clntSocket);
}

/*
        TODO: Still need to edit this so that recv() doesn't work
        under the assumption that all bytes are sent in one call.
*/
int PopulateBuffer(char *inBuffer, int bufferSize, int clntSocket) {

  ssize_t totalBytesRead = 0;

  totalBytesRead = recv(clntSocket, inBuffer, bufferSize, 0);
  inBuffer[totalBytesRead] = '\0';
  return totalBytesRead;
}

bool CheckIfProperFormat(string inBuffer) {
  string tString = inBuffer;

  // Checks if in form request<sp>uri<sp>version<cr><lf>
  std::size_t count = 0;
  for (size_t i = 0; i < tString.size(); i++) {
    if (tString[i] == ' ') {
      count++;
    }
    if (tString[i] == '\r') {
      break;
    }
  }

  // Does it have exactly 2 <SP>
  if (count != 2) {
    return false;
  }
  // Now Checks if number of CR == LF and if both are greater than 2..
  std::size_t numCR = 0;
  std::size_t numLF = 0;
  for (std::size_t i = 0; i < tString.size(); i++) {
    if (tString[i] == '\r') {
      numCR++;
    }
    if (tString[i] == '\n') {
      numLF++;
    }
  }

  if (numCR != numLF) {
    return false;
  }

  if (numCR < 2) {
    return false;
  }

  return true;
}

/*
        TODO: FURTHER TESTING NEEDED
*/
HTTPRequest *GenerateHTTPRequest(char *inBuffer) {

  HTTPRequest *temp = new HTTPRequest;

  string Request = inBuffer; // Convert C string to String for easier
                             // operations.

  bool isGoodToParse = CheckIfProperFormat(Request);

  if (isGoodToParse) {
    string furtherParsing;
    for (size_t iter = 0; iter < Request.length(); iter++) {
      if (Request[iter] == '\r') {
        furtherParsing = Request.substr(0, iter);
        temp->KeyValues = Request.substr(iter, Request.length() - iter);
        break;
      }
    }
    GenerateHTTPRequestHelper(temp, furtherParsing);
    GetConnectionClose(temp);
    temp->isCorrectFormat = true;
    return temp;
  } else {
    temp->isCorrectFormat = false;
    return temp;
  }
  // return temp;
}

/*
 *    Need to test further

 * */
void GetConnectionClose(HTTPRequest *inPtr) {
  std::string toFind = "Connection: close";
  std::size_t found = inPtr->KeyValues.find(toFind);

  if (found != std::string::npos) {
    inPtr->ConnectionClose = true;
  } else {
    inPtr->ConnectionClose = false;
  }
}

/*
        TODO: FURTHER TESTING NEEDED


*/
void GenerateHTTPRequestHelper(HTTPRequest *inPtr, string inData) {

  int startPosition;
  bool flag = true;
  for (size_t iter = 0; iter < inData.length(); iter++) {
    if (inData[iter] == ' ') {
      if (flag) {
        inPtr->RequestType = inData.substr(0, iter);
        startPosition = iter;
        flag = false;
      } else {
        inPtr->URI = inData.substr(startPosition + 1, iter - startPosition - 1);
        inPtr->Version = inData.substr(iter + 1, inData.length() - iter - 1);
        break;
      }
    }
  }
}

/*

*/
void CheckHTTPRequestFormat(HTTPRequest *inPtr, string doc_root) {

  if (inPtr->URI.compare("/") == 0) {
    // Map / -> /index.html
    inPtr->URI = "/index.html";
  }
  string fullFilePath = doc_root + inPtr->URI; // Concatenate the full filePath.
  string newfullFilePath; // Needed for other functions, since realPath has to
                          // be called..

  // First check if request is not understood by program..
  if (!inPtr->isCorrectFormat) {
    inPtr->eCode = EC400;
  } else if (CheckURIForSlash(inPtr->URI)) {
    inPtr->eCode = EC400;
  } else if (CheckRequestType(inPtr)) {
    inPtr->eCode = EC400;
  } else if (CheckIfHostInHeader(inPtr->KeyValues)) {
    inPtr->eCode = EC400;
  } else if (CheckURI(fullFilePath, newfullFilePath)) {
    inPtr->eCode = EC404;
  } else if (CheckFilePathBounds(doc_root, newfullFilePath)) {
    inPtr->eCode = EC404;
  } else if (CheckFilePermission(newfullFilePath)) {
    inPtr->eCode = EC403;
  } else {
    inPtr->CorrectFilePath = newfullFilePath;
    inPtr->eCode = EC200;
  }
}

bool CheckFilePermission(string fPath) {

  struct stat result;
  stat(fPath.c_str(), &result);

  if (result.st_mode && S_IRWXO) {
    return false;
  }

  if (result.st_mode && S_IROTH) {
    return false;
  }

  return true;
}

bool CheckURIForSlash(string URI) {
  if (URI[0] != '/') {
    return true;
  } else {
    return false;
  }
}

/*
        STATUS: GOOD
*/
bool CheckFileExistence(string newfullFilePath) {

  struct stat result;
  if (stat(newfullFilePath.c_str(), &result) == 0) {
    //  cout << "OK" << endl;
    return false;
  }

  // Means file could not be open..so error.
  return true;
}

/*
 *
 * Checks to see if Host: would be in header.
 *
 *
 * *
 * */

bool CheckIfHostInHeader(string kValue) {

  std::string toFind = "Host";
  std::size_t found = kValue.find(toFind);

  // If found == npos -> implies that there does not exist Host
  if (found != std::string::npos) {
    return false;
  } else {
    return true;
  }
}

/*
        Just checks to see if Request type
        is GET.

*/
bool CheckRequestType(HTTPRequest *inPtr) {

  string reqType = "GET";
  if ((reqType.compare(inPtr->RequestType) == 0)) {
    return false;
  } else {
    return true;
  }
}

/*

  uses realpath to test if the doc_root and URI is a legit filepath.

*/
bool CheckURI(string fullFilePath, string &newFullFilePath) {

  char resolvedPath[4096]; // Assumption here is that length of filepath is not
                           // going to be bigger than 4096..
  char *doesExist;
  doesExist = realpath(fullFilePath.c_str(), resolvedPath);

  // Which means there was an error with what the client has sent
  // Since realPath returned NULL
  if (doesExist == NULL) {
    return true;
  }

  // Set newFullFilePath to the unrapped one.
  newFullFilePath = doesExist;

  // cout << "OK" << endl;
  return false;
}

/*

        converts HTTPRequest to HTTPResponse

*/
HTTPResponse *GenerateHTTPResponse(HTTPRequest *inPtr) {

  if (inPtr->eCode == EC200) {
    HTTPResponse *hResponse = new HTTPResponse;

    hResponse->Server = "Server: TritonHTTP";
    hResponse->LastModified =
        "Last-Modified: " + GetLastModified(inPtr->CorrectFilePath);
    hResponse->ContentType =
        "Content-Type: " + GetFileExtension(inPtr->CorrectFilePath);
    hResponse->Version = inPtr->Version;
    hResponse->ContentLength =
        "Content-Length: " + GetContentLength(inPtr->CorrectFilePath);
    hResponse->eCode = inPtr->eCode;
    hResponse->RequestProblem = false;
    hResponse->CorrectFilePath = inPtr->CorrectFilePath;
    hResponse->ConnectionClose = inPtr->ConnectionClose;
    return hResponse;
  } else {
    HTTPResponse *temp = new HTTPResponse;
    temp->eCode = inPtr->eCode;
    temp->RequestProblem = true;
    return temp;
  }
}

/*

        Finds the file extension..

*/
string GetFileExtension(string newFullFilePath) {
  size_t startPosition = newFullFilePath.rfind(".");
  string temp =
      newFullFilePath.substr(startPosition + 1, newFullFilePath.length());
  if (temp.compare("html") == 0) {
    return ("text/" + temp);
  } else {
    if ((temp.compare("jpg") == 0) || (temp.compare("jpe") == 0)) {
      return ("image/jpeg");
    }
    return ("image/" + temp);
  }
}

/*
        STATUS: GOOD
*/
string GetLastModified(string fPath) {

  struct stat result;
  if (stat(fPath.c_str(), &result) == 0) {
    char buffer[1024];
    time_t temp = result.st_mtime;
    struct tm *gmt;
    time(&temp);
    gmt = gmtime(&temp);
    strftime(buffer, 1024, "%a, %d %b %Y %X GMT", gmt);
    string GMTUTC = buffer;
    return GMTUTC;
  }

  return NULL; // Reached here means error.
}

/*
        STATUS: GOOD
*/
string GetContentLength(string fPath) {
  struct stat st;
  if (stat(fPath.c_str(), &st) == 0) {
    string temp = std::to_string(st.st_size);
    return temp;
  }

  // Signify error, might refactor this later
  return "-99999";
}

/*
        Control unit for the sendHTTP response
*/
void sendHTTPResponse(int clntSocket, HTTPResponse *inPtr) {
  switch (inPtr->eCode) {
  case EC200:
    sendHTTPResponseHelper(clntSocket, inPtr, "HTTP/1.1 200 OK");
    SendFileContents(clntSocket, inPtr->CorrectFilePath);
    break;
  case EC400:
    sendHTTPResponseHelper(clntSocket, inPtr, "HTTP/1.1 400 Client Error");
    break;
  case EC403:
    sendHTTPResponseHelper(clntSocket, inPtr, "HTTP/1.1 403 Forbidden");
    break;
  case EC404:
    sendHTTPResponseHelper(clntSocket, inPtr, "HTTP/1.1 404 Not Found");
    break;
  default:
    sendHTTPResponseHelper(clntSocket, inPtr,
                           "HTTP/1.1 500 Server Error"); // Throw 500 error
    break;
  }
}

void SendFileContents(int clntSocket, string fPath) {

  size_t fd;

  struct stat stbuf;

  fd = open(fPath.c_str(), O_RDONLY);

  fstat(fd, &stbuf);

  sendfile(clntSocket, fd, 0, stbuf.st_size);
}

/*

        Tests to see if escape directory

*/
bool CheckFilePathBounds(string doc_root, string fixedFilePath) {
  std::locale loc1;
  std::locale loc2;

  for (size_t i = 0; i < doc_root.length(); i++) {
    if (std::toupper(doc_root[i], loc1) !=
        std::toupper(fixedFilePath[i], loc2)) {
      // So, the 'fixed' file path goes outside the directory given.
      return true;
    }
  }
  // cout << "OK" << endl;
  return false;
}

/*

        prepares the string to be sent.

*/
void sendHTTPResponseHelper(int clntSocket, HTTPResponse *inPtr,
                            string responseMessage) {

  (void)clntSocket;
  string toSend;
  if (!(inPtr->RequestProblem)) {
    if (inPtr->ConnectionClose) {

      toSend = responseMessage + "\r\n" + inPtr->Server + "\r\n" +
               inPtr->LastModified + "\r\n" + inPtr->ContentType + "\r\n" +
               inPtr->ContentLength + "\r\n" + "Connection: close" + "\r\n" +
               "\r\n";
    } else {
      toSend = responseMessage + "\r\n" + inPtr->Server + "\r\n" +
               inPtr->LastModified + "\r\n" + inPtr->ContentType + "\r\n" +
               inPtr->ContentLength + "\r\n" + "\r\n";
    }

    int lengthR = toSend.length();

    // Send header information..
    send(clntSocket, toSend.c_str(), lengthR, 0);

    //  cout << toSend;
  } else {
    string toSend = responseMessage + "\r\n" + "\r\n";

    int lengthR = toSend.length();

    send(clntSocket, toSend.c_str(), lengthR, 0);
  }
}
