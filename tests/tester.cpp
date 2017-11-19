/*
 struct HTTPRequest {
        string RequestType;
        string URI;
        string Version;
        string KeyValues;
        errorCode eCode;
        string CorrectFilePath;
};

struct HTTPResponse{
        string Server;
        string LastModified;
        string Version;
        string ContentType;
        string ContentLength;
        errorCode eCode;
        string CorrectFilePath;
        bool RequestProblem;
};
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 * 	CURRENTLY TESTS THE FOLLOWING
 *
 * bool CheckRequestType(HTTPRequest* inPtr){
 *
 *
 * */

#include "httpd.h"
#include <iostream>
#include <string>
#include <vector>

// prototypes
void TESTCheckoutRequestType(const std::vector<std::string> &testSet);
void TESTGenerateHTTPRequest();
void TESTCheckFilePathBounds(string doc_root, string fixedFilePath);
void TESTCheckIfProperFormat(std::string inPut);
void TestCorrectnessOfRequest();

int main() {

  std::cout << "HTTPD TESTER NOW RUNNING" << std::endl;
  std::cout << endl;
  std::cout << endl;
  std::vector<std::string> requestTestSet = {"GET", "PUT", "   ", "GeT", "100"};

  TESTCheckoutRequestType(requestTestSet);
  TESTGenerateHTTPRequest();
  TESTCheckFilePathBounds("/test/directory/program/data",
                          "/test/directory/program/data/file.txt");
  TESTCheckFilePathBounds("/test/directory/program/data",
                          "/test/directory/topsecretdata/data/file.txt");
  TESTCheckIfProperFormat("GET /test.html HTTP/1.1\r\n\r\n");
  TESTCheckIfProperFormat("GET/test.html HTTP/1.1\r\n\r\n");
  TESTCheckIfProperFormat("GET /test.html HTTP/1.1");
  TESTCheckIfProperFormat("GET  /test.html HTTP/1.1\r\r\n");
  TESTCheckIfProperFormat("GET/test.htmlHTTP/1.1\r\n\r\n");

  return 0;
}

void TESTCheckoutRequestType(const std::vector<std::string> &testSet) {
  std::cout << "***************************************************************"
            << std::endl;
  std::cout << std::endl;
  std::cout << "TESTING: bool CheckRequestType(HTTPRequest* inPtr)"
            << std::endl;

  HTTPRequest *tempPtr = new HTTPRequest;

  for (int i = 0; i < testSet.size(); i++) {
    // std::cout << testSet[i] << std::endl;
    tempPtr->RequestType = testSet[i];
    if (CheckRequestType(tempPtr)) {
      std::cout << "Input: " << testSet[i] << " -- FAIL" << std::endl;
    } else {
      std::cout << "Input: " << testSet[i] << " -- PASS" << std::endl;
    }
  }

  delete tempPtr;
  std::cout << endl;
  std::cout << endl;
}

void TESTGenerateHTTPRequest() {
  std::cout << std::endl;
  std::cout << "***************************************************************"
            << std::endl;

  std::cout << std::endl;
  std::cout << "TESTING: HTTPRequest* GenerateHTTPRequest(char* inBuffer)"
            << std::endl;

  char test1[] = "GET /test.html HTTP/1.1\r\n\r\n";
  std::cout << "First Test: "
            << "GET /test.html HTTP/1.1<CR><LF><CR><LF>" << std::endl;
  HTTPRequest *first = GenerateHTTPRequest(test1);
  std::cout << "Request Type: " << first->RequestType << std::endl;
  std::cout << "URI: " << first->URI << std::endl;
  std::cout << "Version: " << first->Version << std::endl;
  std::cout << "KeyValue: " << first->KeyValues << std::endl;
  delete first;

  std::cout << std::endl;

  char test2[] = "GET /Directory/stuff/cse124/test.html HTTP/1.1\r\nHost: "
                 "localhost:4000\r\nConnection: close\r\n\r\n";
  std::cout << "Second Test: "
            << "GET /Directory/stuff/cse124/test.html HTTP/1.1<CR><LF>Host: "
               "localhost:4000<CR><LF>Connection: close<CR><LF><CR><LF>"
            << std::endl;
  first = GenerateHTTPRequest(test2);
  std::cout << "Request Type: " << first->RequestType << std::endl;
  std::cout << "URI: " << first->URI << std::endl;
  std::cout << "Version: " << first->Version << std::endl;
  std::cout << "KeyValue: " << first->KeyValues << std::endl;
  delete first;
  std::cout << endl;
  std::cout << endl;
}

void TESTCheckFilePathBounds(string doc_root, string fixedFilePath) {

  std::cout << std::endl;
  std::cout << "***************************************************************"
            << std::endl;

  std::cout << std::endl;
  std::cout
      << "TESTING: CheckFilePathBounds(string doc_root, string fixedFilePath)"
      << std::endl;
  std::cout << "With Input: " << doc_root << std::endl;
  std::cout << "With Input: " << fixedFilePath << std::endl;

  if (CheckFilePathBounds(doc_root, fixedFilePath)) {
    std::cout << "PATH DOES ESCAPE DIRECTORY" << std::endl;
  } else {
    std::cout << "PATH DOES NOT ESCAPE DIRECTORY" << std::endl;
  }
  std::cout << endl;
  std::cout << endl;
}

void TESTCheckIfProperFormat(std::string inPut) {

  std::cout << std::endl;
  std::cout << "***************************************************************"
            << std::endl;
  std::cout << std::endl;
  std::cout << "TESTING: bool CheckIfProperFormat(inPut)" << std::endl;
  std::cout << "With Input: " << inPut << std::endl;
  std::cout << std::endl;

  if (CheckIfProperFormat(inPut)) {
    std::cout << "Request Good To Parse" << std::endl;
  } else {
    std::cout << "Request Not Good To Parse" << std::endl;
  }
  std::cout << endl;
  std::cout << endl;
}
