#define _GNU_SOURCE

#include <dlfcn.h>
#include <stdio.h>
 
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdarg.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <string.h>
#include <unistd.h>
#include <err.h>


// The following line declares a function pointer with the same prototype as the open function.  
int (*orig_open)(const char *pathname, int flags, ...);  // mode_t mode is needed when flags includes O_CREAT
int (*orig_close)(int fd);
ssize_t (*orig_read)(int fd, void *buf, size_t count);
ssize_t (*orig_write)(int fd, const void *buf, size_t count);
off_t (*orig_lseek)(int fd, off_t offset, int whence);
int (*orig_stat)(const char *pathname, struct stat *statbuf);
int (*orig_unlink)(const char *pathname);
ssize_t (*orig_getdirentries)(int fd, char *restrict buf, size_t nbytes, off_t *restrict basep);
struct dirtreenode* (*orig_getdirtree)( const char *path );
void (*orig_freedirtree)( struct dirtreenode* dt );



// Sending and receiving message from server
void connect_message(char *buf) {
	char *serverport;
	unsigned short port;
	int sockfd, rv;
	char *serverip;

	// Get environment variable indicating the ip/port of the server
	serverip = getenv("server15440");
	// printf("it is %s\n", serverip);
	serverport = getenv("serverport15440");
	if (serverport) port = (unsigned short)atoi(serverport);
	else port=18440;
	if (serverip == NULL) {
		serverip = "127.0.0.1";
	}

	// Create socket
	sockfd = socket(AF_INET, SOCK_STREAM, 0);	// TCP/IP socket
	if (sockfd<0) err(1, 0);			// in case of error

	struct sockaddr_in srv;
	// setup address structure to indicate server port

	memset(&srv, 0, sizeof(srv));			// clear it first

	srv.sin_family = AF_INET;			// IP family

	srv.sin_addr.s_addr = inet_addr(serverip);	// server IP address

	srv.sin_port = htons(port);			// server port

	rv = connect(sockfd, (struct sockaddr*)&srv, sizeof(struct sockaddr));
	// Check the return value to make sure connection succeeds.
	if (rv < 0) {
		err(1, 0);
	}

	rv = send(sockfd, buf, strlen(buf), 0);
}

// This is our replacement for the open function from libc.
int open(const char *pathname, int flags, ...) {
	mode_t m=0;
	if (flags & O_CREAT) {
		va_list a;
		va_start(a, flags);
		m = va_arg(a, mode_t);
		va_end(a);
	}
	// we just print a message, then call through to the original open function (from libc)
	fprintf(stderr, "mylib: open called for path %s\n", pathname);
    char *buf = "open\n";
    // send message to server, indicating type of operation
	connect_message(buf);
	return orig_open(pathname, flags, m);
}

int close(int fd) {
	char *buf = "close\n";
	connect_message(buf);
	return orig_close(fd);
}

ssize_t read(int fd, void *buf, size_t count) {
	char *message = "read\n";
	connect_message(message);
	return orig_read(fd, buf, count);
}

ssize_t write (int fd, const void *buf, size_t count) {
	char *message = "write\n";
	connect_message(message);
	return orig_write(fd, buf, count);
}

off_t lseek(int fd, off_t offset, int whence) {
	char *message = "lseek\n";
	connect_message(message);
	return orig_lseek(fd, offset, whence);
}

int stat (const char *pathname, struct stat *statbuf) {
	char *message = "stat\n";
	connect_message(message);
	return orig_stat(pathname, statbuf);
}
int unlink (const char *pathname) {
	char *message = "unlink\n";
	connect_message(message);
	return orig_unlink(pathname);
}
ssize_t getdirentries (int fd, char *restrict buf, size_t nbytes, off_t *restrict basep) {
	char *message = "getdirentries\n";
	connect_message(message);
	return orig_getdirentries(fd, buf, nbytes, basep);
}

struct dirtreenode* getdirtree( const char *path ) {
	char *message = "getdirentries\n";
	connect_message(message);
	return orig_getdirtree(path);
}

void freedirtree( struct dirtreenode* dt ) {
	char *message = "getdirentries\n";
	connect_message(message);
	return orig_freedirtree(dt);
}




// This function is automatically called when program is started
void _init(void) {
	// set function pointer orig_open to point to the original open function
	orig_open = dlsym(RTLD_NEXT, "open");
	orig_close = dlsym(RTLD_NEXT, "close");
	orig_read = dlsym(RTLD_NEXT, "read");
	orig_write = dlsym(RTLD_NEXT, "write");
	orig_lseek = dlsym(RTLD_NEXT, "lseek");
	orig_stat = dlsym(RTLD_NEXT, "stat");
	orig_unlink = dlsym(RTLD_NEXT, "unlink");
	orig_getdirentries = dlsym(RTLD_NEXT, "getdirentries");
	orig_getdirtree = dlsym(RTLD_NEXT, "getdirtree");
	orig_freedirtree = dlsym(RTLD_NEXT, "freedirtree");

	fprintf(stderr, "Init mylib\n");
}


