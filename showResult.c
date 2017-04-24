#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <math.h>
#include "restart.h"
#include <signal.h>
#define FIFOARG 1
#define FIFO_PERMS (S_IRWXU | S_IWGRP| S_IWOTH)


void handler1 (int sig);
FILE *out;
int main (int argc, char *argv[]) {

int resultfd,pidfd;
char result[50];
char pid[8];
signal(SIGINT, handler1);
	if ((resultfd = open("resultfifo", O_RDONLY)) == -1) {
		perror("Result failed to open log fifo for reading");
		return 1;
	}
	
	out=fopen("resultlog","a");
	
	while(1){
	if(r_read(resultfd,result,sizeof(result))>0);
	r_write(STDOUT_FILENO,result,sizeof(result));
	fprintf(out,"m1 %s",result);
	printf("\n");
	}
}
void handler1 (int sig)
{


fprintf(out,"ctrl-c signal\n");

exit(1);
}
