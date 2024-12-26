# TorrentSystemTCDS.O

The initial state of this project was achieved with the colaboration of [Filinho](https://github.com/Filinho) and [Pedro Dias](https://github.com/pedrogabdias)

The main goal of this project is to create a file sharing platform between directories, each directory is an abstraction for a user, each user has an number n of files, after the end of the program all users will have the same files. 

## User
  A user is an abstraction for a directory in our system, each user is represented by a thread that work as an server(provide the present file in that user for the other users) and as an client(request files from other users). The Thread User is responsable for create an boolean vector that represent the present files in the user and also invoque Threads for the user as an client and as an server.

## User -> Client
  The client is an Thread responsible for insert a request in the Global Requests List, an request is compose by an index of a file, an buffer pointer, an Thread array and a counter that represent the number of servers that is attending that request.
## User -> Server
  The server is a waiter, this Thread check the Global Requests List and attend the requests.
