#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

void doWrite (int fd,const char* buff,int len) {
 		ssize_t ret;
		ret=write(fd,buff,len);
		if (ret<0) {
				perror("write");
				exit(1);
		}
		

}

void write_file (int fd, const char *infile) {
		int fd_1=open(infile, O_RDONLY);
		struct stat st;
		stat(infile, &st);
	    long size = st.st_size;
		
		ssize_t ret;
		char *buff;
		buff=malloc(st.st_size); 
		
		ret=read(fd_1,buff,size);
		
		if (ret<0) {
				perror("read");
				exit (1);
		}
		
		doWrite(fd,buff,size);
				if (close(fd_1)<0) {
						perror("close");
						exit(1); 
				} 

}
