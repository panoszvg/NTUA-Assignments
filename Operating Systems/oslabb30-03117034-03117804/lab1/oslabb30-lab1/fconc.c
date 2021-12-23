#include <stdio.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include "fconc.h"
#include <string.h>

int main (int argc, char** argv) {
		if (argc<3 || argc>4) printf("Usage: ./fconc infile1 infile2 [outfile (default:fconc.out)]\n");
		int fd1,fd2,fd3;
		bool output_file=false;
		if (argc==4) output_file=true;
		if (argc==3 || argc==4) { 
				fd1=open(argv[1],O_RDONLY);
				if (fd1<0) {
						perror(argv[1]);
						exit(1);
				}
				close(fd1);

				fd2=open(argv[2],O_RDONLY);
				if (fd2<0) {
						perror(argv[2]);
						exit(1);
				}
				close(fd2); 
				if (output_file) {
						if (strcmp(argv[2],argv[3]) != 0) {

								fd3=open(argv[3],O_CREAT,S_IRUSR | S_IWUSR);
								close (fd3);
								fd3=open(argv[3],O_WRONLY);
								write_file(fd3,argv[1]);
								write_file(fd3,argv[2]);
								if (close(fd3)<0) {

										perror("close");
										exit(1);
								}
						}
						else	{ 

								fd3=open("fconc.out",O_WRONLY);
								write_file(fd3,argv[1]);
								write_file(fd3,argv[2]);
								fd2=open(argv[3],O_WRONLY);
								if (close(fd3)<0) {
										perror("close");
										exit(1);
								}
								write_file(fd2,"fconc.out");
								close(fd2);


						}
				}
				else {
						fd3=open("fconc.out",O_WRONLY);
						write_file(fd3,argv[1]);
						write_file(fd3,argv[2]);
						if (close(fd3)<0) {
								perror("close");
								exit(1);
						}

				}
		}
}

//end


