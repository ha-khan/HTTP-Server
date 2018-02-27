# Basic HTTP Server
This project only attempts to mimic a small subset of 
features that a full web server would employ. This web server is able to accept only the HTTP/1.1 GET method and
returns file formats of jpg, png, and html. Main component of program is located in httpd.cpp file; where much 
of the server logic is written.


## Building project
The makefile supplied in the src folder.

```
make clean
make
```
## Tests
The tests folder contains some units tests to make sure that the parser
function is working.



