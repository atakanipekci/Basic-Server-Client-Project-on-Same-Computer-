#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <math.h>
#include "restart.h"
#include  <signal.h>
#define FIFOARG 3
#define FIFO_PERMS (S_IRWXU | S_IWGRP| S_IWOTH)

void handler1(int);
int determinant(int **a,int n);
FILE *out;
int clients[100];
int count=0;


int main (int argc, char *argv[]) {
struct timeval t;
double currenttime;
int requestfd;
int sidefifo;
int i, o,det=0;
char buffer[500];
char pid[8];
char sidefifoname[9];
pid_t id;
int **matrix;

if (argc != 4) { /* name of server fifo is passed on the command line */
fprintf(stderr, "Usage: %s fifoname > logfile\n", argv[0]);
return 1;
}
signal(SIGINT, handler1);

/* create a named pipe to handle incoming requests */
if ((mkfifo(argv[FIFOARG], FIFO_PERMS) == -1) && (errno != EEXIST)) {
perror("Server failed to create a FIFO");
return 1;
}
/* open a read/write communication endpoint to the pipe */
if ((requestfd = open(argv[FIFOARG], O_RDWR)) == -1) {
perror("Server failed to open its FIFO");
return 1;
}
else{

	out=fopen("serverlog.txt","a");
	
	matrix = (int **)malloc(atoi(argv[2])*2* sizeof(int *));
    for (i=0; i<atoi(argv[2])*2; i++)
         matrix[i] = (int *)malloc(atoi(argv[2])*2* sizeof(int));

	while(1){
		r_read(requestfd,pid,sizeof(int));
		clients[count]=atoi(pid);
		//printf(":::%d\n",clients[count]);
		count++;
		
		
		if ((id = fork()) < 0)
			printf("fork error\n");

		if(id==0){
		//r_write(STDOUT_FILENO,pid,sizeof(int));

		strcpy(sidefifoname,"");
		sprintf(sidefifoname,"%d",atoi(pid));
		//printf("*-*-*%s**\n",sidefifoname);
		if ((mkfifo(sidefifoname, FIFO_PERMS) == -1) && (errno != EEXIST)) {
		perror("Server failed to create a FIFO");
		return 1;
		}
		if((sidefifo=open(sidefifoname, O_RDWR)) == -1){
		perror("Server failed to open its FIFO");
		return 1;
		}
		
		srand(time(NULL));
		while(det==0){
		for(o = 0; o<atoi(argv[2])*2; o++)
		    for(i = 0; i<atoi(argv[2])*2; i++)
		        matrix[o][i] = rand()%10+1;

			det=determinant(matrix,atoi(argv[2])*2);
		}
		gettimeofday(&t,NULL);
		currenttime=(t.tv_sec)*1000;
		fprintf(out,"pid:%d det:%d time:%.2f\n",atoi(pid),det,currenttime);
		sprintf(buffer,"%d ",atoi(argv[2])*2*atoi(argv[2])*2);
		for(o = 0; o<atoi(argv[2])*2; o++)
		    for(i = 0; i<atoi(argv[2])*2; i++)
		        sprintf(buffer+strlen(buffer),"%d ",matrix[o][i]);

			
		/*for(o = 0; o<atoi(argv[1])*2; o++){
			printf("\n");
		    for(i = 0; i<atoi(argv[1])*2; i++)
		        printf("**--%d ",matrix[o][i]);
		}
				printf("\n");*/
		
		r_write(sidefifo,buffer,sizeof(int)*atoi(argv[2])*2*atoi(argv[2])*2);
		//printf("det:%d\n",determinant(matrix,4));

		strcpy(buffer, "");
		
		exit(1);
		}
		r_close(sidefifo);
		
		sleep(atoi(argv[1])/1000);
	}		
}
	
return 1;
}

int determinant(int **a,int n)
{
    int i,j,j1,j2 ;                   
    int det = 0 ;                   
   	int **m = NULL ;               
                                      

    if (n < 1)    {   }                

    else if (n == 1) {                
        det = a[0][0] ;
        }

    else if (n == 2)  {                
                                       
        det = a[0][0] * a[1][1] - a[1][0] * a[0][1] ;
        }
                                      
    else {                                                                  
        det = 0 ;                                                              
        for (j1 = 0 ; j1 < n ; j1++) {                                       
            m = (int **) malloc((n-1)* sizeof(int *)) ;
            for (i = 0 ; i < n-1 ; i++)
                m[i] = (int *) malloc((n-1)* sizeof(int)) ;                      
            for (i = 1 ; i < n ; i++) {
                j2 = 0 ;                                                   
                for (j = 0 ; j < n ; j++) {
                    if (j == j1) continue ; 

                    m[i-1][j2] = a[i][j] ;                                                                                       
                    j2++ ;                 
                    }
                }
            det += pow(-1.0,1.0 + j1 + 1.0) * a[0][j1] * determinant(m,n-1) ;                                           
            for (i = 0 ; i < n-1 ; i++) free(m[i]) ;                                           
            free(m) ;                       
                                           
        }
    }
    return(det) ;
}

void handler1 (int sig)
{

int i;
fprintf(out,"ctrl-c signal\n");
for(i=0;i<count;i++){

kill(clients[i],SIGKILL);

}
exit(1);
}

