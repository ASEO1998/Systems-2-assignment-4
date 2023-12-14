/*-------------------------------------------------------------------------*
 *------*
 *---columnServer.c---*
 *------*
 *---    This file defines a C program that gets file-sys commands---*
 *---from client via a socket, executes those commands in their own---*
 *---threads, and returns the corresponding output back to the---*
 *---client.---*
 *------*
 *--------------------------------------*
 *------*
 *---Version 1aJoseph Phillips---*
 *------*
 *-------------------------------------------------------------------------*/

//Compile with:
//$ gcc columnServer.c -o columnServer -lpthread

//---Header file inclusion---//

#include "clientServer.h"
#include <pthread.h>// For pthread_create()
//const char* THIS_PROGRAM_NAME = "Error for columnServer.c";


//---Definition of constants:---//

#define STD_ERROR_MSG "Error doing operation"

const int STD_ERROR_MSG_LEN = sizeof(STD_ERROR_MSG) - 1;

#define STD_BYE_MSG "Good bye!"

const int STD_BYE_MSG_LEN = sizeof(STD_BYE_MSG) - 1;

const int ERROR_FD = -1;

#define THIS_PROGRAM_NAME "columnServer"
//---Definition of global vars:---//

//  PURPOSE:  To be non-zero for as long as this program should run, or '0'
//otherwise.


//---Definition of functions:---//

void wholeFile(int fd){
  char buffer[BUFFER_LEN];
  int file = open(FILENAME,O_RDONLY);

  if (file <= -1){
    
    write(fd,STD_ERROR_MSG,STD_ERROR_MSG_LEN );
    close (file);
    exit(EXIT_FAILURE);
    //close(fd);
    
  }
  else{

  int numRead = read(file,buffer,BUFFER_LEN-1);
  buffer[numRead] = '\0';
  write(fd,buffer,numRead);
  //printf("%s\n",buffer);
  close(file);
  //close(fd);
  //return (NULL);
  }
}

void columns(int fd, int columnIndex){
  int pid = fork();
  if (pid < 0){
    write(fd,STD_ERROR_MSG,STD_ERROR_MSG_LEN );
    fprintf(stderr, "Too many processes ace!\n");
    exit(EXIT_FAILURE);
  }

  if (pid == 0){
    int file = open(FILENAME,O_RDONLY);
    if (file <= -1){

      write(fd,STD_ERROR_MSG,STD_ERROR_MSG_LEN );
      close (file);
      exit(EXIT_FAILURE);

    }
    dup2(file,STDIN_FILENO);
    dup2(fd,STDOUT_FILENO);
    char awkCmd[BUFFER_LEN];
    snprintf(awkCmd,BUFFER_LEN,"{print $%d}",columnIndex);
    execl("/usr/bin/awk","awk",awkCmd,NULL);

  }

  if( pid > 0){
    wait(NULL);

  }
  
}

//  PURPOSE:  To cast 'vPtr' to the pointer type coming from 'doServer()'
//that points to two integers.  Then, to use those two integers,
//one as a file descriptor, the other as a thread number, to fulfill
//requests coming from the client over the file descriptor socket.
//Returns 'NULL'.
void* handleClient(void* vPtr
		  )
{
  //  I.  Application validity check:

  //  II.  Handle client:
  //  YOUR CODE HERE
  //struct InfoForClient {
  //int fileDecript;
  //int threadID;
  //} ;
  
  int* intPtr = (int*)vPtr;
  
  int    fd = intPtr[0];// <-- CHANGE THAT 0!
  
  int   threadNum = intPtr[1];// <-- CHANGE THAT 0!
  free(vPtr);
  //  YOUR CODE HERE
  char buffer[BUFFER_LEN];
  char command;
  int shouldContinue = 1;

  while (shouldContinue){
    read(fd,buffer,BUFFER_LEN);
    printf("Thread %d received: %s\n",threadNum,buffer);
    command = buffer[0];

    if (command == WHOLE_FILE_CMD_CHAR){
      
      wholeFile(fd);
      
    }

    else if (command == '1' || command == '2' || command =='3'|| command == '4'){
      int status;
      int columnIndex = command - '0';
      columns(fd, columnIndex);
    }

    else if(command == QUIT_CMD_CHAR ){
      write(fd,STD_BYE_MSG,STD_BYE_MSG_LEN);
      shouldContinue = 0;
      close(fd);
    }
  }
  //  III.  Finished:
  printf("Thread %d quitting.\n",threadNum);
  //close(fd);
  return(NULL);
}


//  PURPOSE:  To run the server by 'accept()'-ing client requests from
//'listenFd' and doing them.
void doServer(int listenFd
	     )
{
  //  I.  Application validity check:

  //  II.  Server clients:
  pthread_t threadId;
  pthread_attr_t threadAttr;
  int threadCount = 0;

  pthread_attr_init(&threadAttr);
  pthread_attr_setdetachstate(&threadAttr,PTHREAD_CREATE_DETACHED);
  while  (1)
    {
      //  YOUR CODE HERE
      //printf("hello/n");
      int* fd = (int*)calloc(2, sizeof(int));
      //int * fd = (int*)calloc(2,sizeof(int));
      //int * tC = (int*)malloc(sizeof(int));
      //*fd = accept(listenFd,NULL,NULL);
      fd[0] = accept(listenFd,NULL,NULL);
      //fd++;
      //*fd = threadCount;
      fd[1] = threadCount;
      
      //printf("fd is %d and threadcount is %d\n",fd[0],fd[1]);
      
      threadCount++;

      //pthread_attr_init(&threadAttr);
      // pthread_attr_setdetachstate(&threadAttr,PTHREAD_CREATE_DETACHED);
      pthread_create(&threadId,&threadAttr,handleClient,(void*)fd);
      //pthread_attr_destroy(&threadAttr);      
    }

  //  YOUR CODE HERE
  pthread_attr_destroy(&threadAttr);
  //doServer(listenFd);
  //printf("hello");
  //  III.  Finished:
}


//  PURPOSE:  To decide a port number, either from the command line arguments
//'argc' and 'argv[]', or by asking the user.  Returns port number.
int getPortNum (int argc,
	      char* argv[]
	      )
{
  //  I.  Application validity check:

  //  II.  Get listening socket:
  int portNum;

  if  (argc >= 2)
    portNum= strtol(argv[1],NULL,0);
  else
    {
      char buffer[BUFFER_LEN];

      printf("Port number to monopolize? ");
      fgets(buffer,BUFFER_LEN,stdin);
      portNum = strtol(buffer,NULL,0);
    }

  //  III.  Finished:
  return(portNum);
}


//  PURPOSE:  To attempt to create and return a file-descriptor for listening
//to the OS telling this server when a client process has connect()-ed
//to 'port'.  Returns that file-descriptor, or 'ERROR_FD' on failure.
int getServerFileDescriptor
(int port
 )
{
  //  I.  Application validity check:

  //  II.  Attempt to get socket file descriptor and bind it to 'port':
  //  II.A.  Create a socket
  int socketDescriptor = socket(AF_INET, // AF_INET domain
				SOCK_STREAM, // Reliable TCP
				0);

  if  (socketDescriptor < 0)
    {
      perror("socket()");
      return(ERROR_FD);
    }

  //  II.B.  Attempt to bind 'socketDescriptor' to 'port':
  //  II.B.1.  We'll fill in this datastruct
  struct sockaddr_in socketInfo;

  //  II.B.2.  Fill socketInfo with 0's
  memset(&socketInfo,'\0',sizeof(socketInfo));

  //  II.B.3.  Use TCP/IP:
  socketInfo.sin_family = AF_INET;

  //  II.B.4.  Tell port in network endian with htons()
  socketInfo.sin_port = htons(port);

  //  II.B.5.  Allow machine to connect to this service
  socketInfo.sin_addr.s_addr = INADDR_ANY;

  //  II.B.6.  Try to bind socket with port and other specifications
  int status = bind(socketDescriptor, // from socket()
		    (struct sockaddr*)&socketInfo,
		    sizeof(socketInfo)
		    );

  if  (status < 0)
    {
      perror(THIS_PROGRAM_NAME);
      return(ERROR_FD);
    }

  //  II.B.6.  Set OS queue length:
  listen(socketDescriptor,5);

  //  III.  Finished:
  return(socketDescriptor);
}


int main(int argc,
	char* argv[]
	)
{
  //  I.  Application validity check:

  //  II.  Do server:
  int       port = getPortNum(argc,argv);
  int      listenFd = getServerFileDescriptor(port);
  int      status = EXIT_FAILURE;

  if  (listenFd >= 0)
    {
      doServer(listenFd);
      close(listenFd);
      status = EXIT_SUCCESS;
    }

  //  III.  Finished:
  return(status);
}
