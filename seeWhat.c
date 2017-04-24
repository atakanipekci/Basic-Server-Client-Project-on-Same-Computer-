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
#include  <signal.h>
#define FIFOARG 1
#define MAX_SIZE 20
#define FIFO_PERMS (S_IRWXU | S_IWGRP| S_IWOTH)

void handler1(int);
void handler2 (int sig);
void inversematrix(double **num,int f);
void transpose(double **num,double kofaktor[MAX_SIZE][MAX_SIZE],double **inv,int r);
int determinant(int **a,int n);
double doubledeterminant(double **a,int n);
FILE *out;

int main (int argc, char *argv[]) {
time_t curtime;
struct timeval t1,t2;
int len;
char requestbuf[PIPE_BUF];
char fifoname[4];
char sidefifoname[9];
char buffer[500];
char logname[8];
char results[50];
int requestfd;
int matrixfd,resultfd,pidfd;
const char s[2] = " ";
char *token;
int * mymatrix;
int **my2dmatrix;
double **minor1;
double **minor2;
double **minor3;
double **minor4;
double **inv;
int i=0,j,n,k,p,myid;
double result1,timeelapsed;
clock_t timeafter,timebefore;
pid_t id1,id2;

	if (argc != 2) { /* name of server fifo is passed on the command line */
		fprintf(stderr, "Usage: %s fifoname", argv[0]);
		return 1;
	}


	signal(SIGINT, handler1);
	signal(SIGKILL, handler1);

	if ((mkfifo("resultfifo", FIFO_PERMS) == -1) && (errno != EEXIST)) {
		perror("Client failed to create a FIFO");
		return 1;
		}

	if ((resultfd = open("resultfifo", O_WRONLY)) == -1) {
		perror("Client failed to open log fifo for writing");
		return 1;
	}

	while(1){
	if ((requestfd = open(argv[FIFOARG], O_WRONLY)) == -1) {
		perror("Client failed to open log fifo for writing");
		return 1;
	}
	
	sprintf(requestbuf, "%d", (int)getpid());
	len = strlen(requestbuf);
	if (r_write(requestfd, requestbuf, len) != len) {
		perror("Client failed to write");
		return 1;
	}
	strcpy(requestbuf, "");
	sprintf(sidefifoname,"%d",(int)getpid());
	//printf("******%s***\n",sidefifoname);
	if ((mkfifo(sidefifoname, FIFO_PERMS) == -1) && (errno != EEXIST)) {
		perror("Client failed to create a FIFO");
		return 1;
		}

	if ((matrixfd = open(sidefifoname, O_RDONLY)) == -1) {
		perror("Client failed to open log fifo for reading");
		return 1;
	}
	
	
	r_read(matrixfd,buffer,sizeof(buffer));
		
	token = strtok(buffer, s);
   	mymatrix= (int*)malloc(atoi(token)*sizeof(int));
	n=(int)sqrt(atoi(token));
	/* walk through other tokens */
	i=0;
   while( token != NULL ) 
   {
		if(i>0){
			//printf( " %d\n", atoi(token) );
			mymatrix[i-1]=atoi(token);
			
		}
		token = strtok(NULL, s);
		i++;
   }
   	/*j=i;
	for(i=0; i<j-1; i++)
		printf("::%d\n",mymatrix[i]);*/

	my2dmatrix = (int **)malloc(n*2* sizeof(int *));
    for (i=0; i<n*2; i++)
         my2dmatrix[i] = (int *)malloc(n*2* sizeof(int));

	inv = (double **)malloc(n* sizeof(double *));
    for (i=0; i<n; i++)
         inv[i] = (double *)malloc(n* sizeof(double));

	minor1 = (double **)malloc(n* sizeof(double *));
    for (i=0; i<n; i++)
         minor1[i] = (double *)malloc(n* sizeof(double));

	minor2 = (double **)malloc(n* sizeof(double *));
    for (i=0; i<n; i++)
         minor2[i] = (double *)malloc(n* sizeof(double));

	minor3 = (double **)malloc(n* sizeof(double *));
    for (i=0; i<n; i++)
         minor3[i] = (double *)malloc(n* sizeof(double));

	minor4 = (int **)malloc(n* sizeof(double *));
    for (i=0; i<n; i++)
         minor4[i] = (int *)malloc(n* sizeof(double));


	k=0;
	for(i=0; i<n; i++)
		for(j=0; j<n; j++,k++)
			my2dmatrix[i][j]=mymatrix[k];

	/*for(i=0; i<n; i++){
		for(j=0; j<n; j++,k++)
			//printf("%d ",my2dmatrix[i][j]);

		//printf("\n");
	}*/
	
	for(i=0; i<n/2; i++){
		for(j=0; j<n/2; j++){
			minor1[i][j]=my2dmatrix[i][j];
			//printf("%f ",minor1[i][j]);
		}
		//printf("\n");
	}
	for(i=0,k=0; i<n/2; i++,k++){
		for(j=n/2,p=0; j<n; j++,p++){
			minor2[k][p]=my2dmatrix[i][j];
			//printf("%f ",minor2[k][p]);
		}
		//printf("\n");
		
	}

	for(i=n/2,k=0; i<n; i++,k++){
		for(j=0,p=0; j<n/2; j++,p++){
			minor3[k][p]=my2dmatrix[i][j];
			//printf("%f ",minor3[k][p]);
		}
		printf("\n");
		
	}


	for(i=n/2,k=0; i<n; i++,k++){
		for(j=n/2,p=0; j<n; j++,p++){
			minor4[k][p]=my2dmatrix[i][j];
			//printf("%f ",minor4[k][p]);
		}
		//printf("\n");
		
	}	
	

	//cofactor(my2dmatrix,n,inv);

	myid=(int)getpid();
/********************INVERSE FORK*************************/
	if ((id1 = fork()) < 0)
		printf("fork error\n");
	if(id1==0){
	sprintf(logname,"logs/%d",(int)getpid());
	out=fopen(logname,"a");
	timebefore=clock();
	inversematrix(minor1,n/2);
	inversematrix(minor2,n/2);
	inversematrix(minor3,n/2);
	inversematrix(minor4,n/2);
	printf("inverse\n");

	for(i=0,k=0; i<n/2; i++,k++){
		for(j=n/2,p=0; j<n; j++,p++)
			inv[k][p]=minor1[i][j];
			
		
	}

	for(i=0,k=0; i<n/2; i++,k++){
		for(j=n/2,p=n/2; j<n; j++,p++)
			inv[k][p]=minor2[i][j];
			
		
	}


	for(i=0,k=n/2; i<n/2; i++,k++){
		for(j=n/2,p=0; j<n; j++,p++)
			inv[k][p]=minor3[i][j];

		
	}

	for(i=0,k=n/2; i<n/2; i++,k++){
		for(j=n/2,p=n/2; j<n; j++,p++)
			inv[k][p]=minor4[i][j];

		
	}
	result1=determinant(my2dmatrix,n)-doubledeterminant(inv,n);
	
	timeafter=clock();

	fprintf(out,"original = [");
	for(i=0; i<n; i++){
		for(j=0; j<n; j++)
			if(j<n-1)
			fprintf(out,"%d, ",my2dmatrix[i][j]);
			else if(i<n-1)
			fprintf(out,"%d; ",my2dmatrix[i][j]);
			else
			fprintf(out,"%d",my2dmatrix[i][j]);
		
	}
	fprintf(out,"]");
	fprintf(out,"\n");
	fprintf(out,"inverse = [");
	for(i=0; i<n; i++){
		for(j=0; j<n; j++)
			if(j<n-1)
			fprintf(out,"%f, ",inv[i][j]);
			else if(i<n-1)
			fprintf(out,"%f; ",inv[i][j]);
			else
			fprintf(out,"%f",inv[i][j]);
		
	}
	fprintf(out,"]");
	
	//printf("*-*-*-*%f\n",result1);
	
	timeelapsed = (double)(timeafter - timebefore) * 1000.0 / CLOCKS_PER_SEC;
	sprintf(results,"%d\t%f\t%f",myid,result1,timeelapsed);
	r_write(resultfd,results,sizeof(results));
	printf("%s\n",results);
	
	close(out);
	exit(1);
	}
/**********************END OF FORK**************/
	wait(NULL);
	
	//printf("***************************************\n");
	//r_write(STDOUT_FILENO,buffer,sizeof(int)*(int)(buffer[0]-'0'));
	//free
	for (i = 0 ; i < n ; i++) free(minor1[i]) ;                                           
		free(minor1) ; 
	for (i = 0 ; i < n ; i++) free(minor2[i]) ;                                           
		free(minor2) ; 
	for (i = 0 ; i < n ; i++) free(minor3[i]) ;                                           
		free(minor3) ; 
	for (i = 0 ; i < n ; i++) free(minor4[i]) ;                                           
		free(minor4) ;
	for (i = 0 ; i < n ; i++) free(my2dmatrix[i]) ;                                           
		free(my2dmatrix) ;
	for (i = 0 ; i < n ; i++) free(inv[i]) ;                                           
		free(inv) ;  	
	r_close(requestfd);
	r_close(matrixfd);
	unlink(sidefifoname);
	
	}
	return 0;
}


void inversematrix(double **matrix,int n)
{
double  ratio,a;
int i, j, k;
    for(i = 0; i < n; i++){
        for(j = n; j < 2*n; j++){
            if(i==(j-n))
                matrix[i][j] = 1.0;
            else
                matrix[i][j] = 0.0;
        }
    }
    for(i = 0; i < n; i++){
        for(j = 0; j < n; j++){
            if(i!=j){
                ratio = matrix[j][i]/matrix[i][i];
                for(k = 0; k < 2*n; k++){
                    matrix[j][k] -= ratio * matrix[i][k];
                }
            }
        }
    }
    for(i = 0; i < n; i++){
        a = matrix[i][i];
        for(j = 0; j < 2*n; j++){
            matrix[i][j] /= a;
        }
    }
   /* printf("The inverse matrix is: \n");
    for(i = 0; i < n; i++){
        for(j = n; j < 2*n; j++){
            printf("%.2f", matrix[i][j]);
            printf("\t");
        }
        printf("\n");
    }*/
}

void transpose(double **num,double kofaktor[MAX_SIZE][MAX_SIZE],double **inv,int r)
{
  int i,j;
  double holdminor[MAX_SIZE][MAX_SIZE],d;
  double inverse[MAX_SIZE][MAX_SIZE];
 
  for (i=0;i<r;i++)
    {
     for (j=0;j<r;j++)
       {
         holdminor[i][j]=kofaktor[j][i];
        }
    }
	 
  d=determinant(num,r);
  for (i=0;i<r;i++)
    {
     for (j=0;j<r;j++)
       {
        inverse[i][j]=holdminor[i][j] / d;
        }
    }
   printf("\n\n\nThe inverse of matrix is : \n");
 
   for (i=0;i<r;i++)
    {
     for (j=0;j<r;j++)
       {
         printf("\t%f",holdminor[i][j]);
        }
    printf("\n");
     }

	for (i=0;i<r;i++)
    {
     for (j=0;j<r;j++)
       {
         inv[i][j]=inverse[i][j];
        }
   
     }
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
double doubledeterminant(double **a,int n)
{
    int i,j,j1,j2 ;                   
    double det = 0 ;                   
   	double **m = NULL ;               
                                      

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
            m = (double **) malloc((n-1)* sizeof(double *)) ;
            for (i = 0 ; i < n-1 ; i++)
                m[i] = (double *) malloc((n-1)* sizeof(double)) ;                      
            for (i = 1 ; i < n ; i++) {
                j2 = 0 ;                                                   
                for (j = 0 ; j < n ; j++) {
                    if (j == j1) continue ; 

                    m[i-1][j2] = a[i][j] ;                                                                                       
                    j2++ ;                 
                    }
                }
            det += pow(-1.0,1.0 + j1 + 1.0) * a[0][j1] * doubledeterminant(m,n-1) ;                                           
            for (i = 0 ; i < n-1 ; i++) free(m[i]) ;                                           
            free(m) ;                       
                                           
        }
    }
    return(det) ;
}

void handler1 (int sig)
{


fprintf(out,"ctrl-c signal\n");

exit(1);
}

void handler2 (int sig)
{


fprintf(out,"kill signal\n");

exit(1);
}


