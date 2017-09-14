#ifndef HTTPD_H
#define HTTPD_H

#include <string>

using namespace std; // get rid of this

/*

*/
enum errorCode { EC200, EC400, EC403, EC404 };

struct HTTPRequest {
  string RequestType;
  string URI;
  string Version;
  string KeyValues;
  errorCode eCode;
  string CorrectFilePath;
  bool ConnectionClose;
  bool isCorrectFormat;
};

struct HTTPResponse {
  string Server;
  string LastModified;
  string Version;
  string ContentType;
  string ContentLength;
  errorCode eCode;
  string CorrectFilePath;
  bool ConnectionClose;
  bool RequestProblem;
};

void start_httpd(unsigned short port, string doc_root);
void HandleTCPClient(int clntSocket);
void HandleHTTPRequest(int clntSocket, string doc_root);
void readHTTPRequest(int clntSocket);
int PopulateBuffer(char *inBuffer, int bufferSize, int clntSocket);
HTTPRequest *GenerateHTTPRequest(char *inBuffer);
void GenerateHTTPRequestHelper(HTTPRequest *inPtr, string inData);

void CheckHTTPRequestFormat(HTTPRequest *inPtr, string doc_root);

HTTPResponse *GenerateHTTPResponse(HTTPRequest *inPtr);

void GetConnectionClose(HTTPRequest *inPtr);

string GetContentLength(string fPath);

string GetLastModified(string fPath);

void sendHTTPResponse(int clntSocket, HTTPResponse *inPtr);

// pointer passed
void sendHTTPResponseHelper(int clntSocket, HTTPResponse *inPtr,
                            string responseMessage);

void SendFileContents(int clntSocket, string fPath);

bool CheckRequestType(HTTPRequest *inPtr);

// Passed by ref
bool CheckURI(string fullFilePath, string &newfullFilePath);

bool CheckFilePathBounds(string doc_root, string fixedFilePath);

bool CheckFileExistence(string newfullFilePath);

bool CheckKeyValues(HTTPRequest *inPtr);

bool CheckFilePermission(string newFullFilePath);

bool CheckIfMoreThanTwoSpace(string httpRequest);

bool CheckIfHostInHeader(string kValue);

bool CheckIfProperFormat(string inBuffer);

bool CheckURIForSlash(string URI);

string GetFileExtension(string newfullFilePath);

#endif // HTTPD_H
