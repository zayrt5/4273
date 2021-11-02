How to run:

"make" will compile the server

"make run" will run the server on port 8888

"make test" will run a few curl commands as fast as possible

"make clean" will remove the excutable server




How it runs:

I just took the sample tcp echo code and expanded it to accept GET requests.
The client connection fd is connected to a file stream, and the stream is put into buf.

the buffer(request) is scanned for the method, uri, and version, and loaded into respective variables.
The headers of the request are printed parsed between CRLFs.
The file name string has www added and then the uri from the request.
If the file does not exist, 404 error is sent and connection is closed.
The filename is scanned to determine its filetype.
The above info is loaded into respective response headers and loaded into the client connection stream.

The requested file is opened, mmapped to a pointer, and that pointer data is written to the connection stream.
The memory is freed and the connection is closed.
