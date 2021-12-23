#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "tree.h"
#include "proc-common.h"

void calculate (struct tree_node *root,int* pipefd) {
		change_pname(root->name);
		int status;
		int new_pipe[2];
		pid_t *p;
		int result;
		int number1,number2;
		unsigned i;
		// Case 1
		if (root->nr_children==0) { //termatikos komvos ara arithmos
				result=atoi(root->name); //metatropi tou string arithmou se int
				sleep(2);
				if (write(pipefd[1],&result,sizeof(result))!=sizeof(result)){
						perror("apotuxia eggrafis\n");
						exit(1);
				}
				exit(0);
		}
		// Case 2
		else { // mi termatikos komvos ara telestis
				if (pipe(new_pipe)<0) { // kataskeui pipe
						perror("error creating pipe");
						exit(1);
				}
				pid_t next;
				p=calloc(root->nr_children,sizeof(pid_t));
				for (i=0;i<root->nr_children;i++) {
						next=fork();
						if (next<0) {
								perror("provlima");
								exit(1);
						}	
						if (next==0) {
								close(new_pipe[0]); // to paidi borei mono na grafei
								calculate(root->children+i,new_pipe);
								exit(0);
						}
						else {
						p[i]=next;
						}
				}
				close(new_pipe[1]); // o parent mono diabazei

				for(i=0;i<root->nr_children;i++){ //perimenoume na teliwsoume ta paidia
						wait(&status);
						explain_wait_status(p[i],status);
				}
				if (read(new_pipe[0],&number1,sizeof(number1))!=sizeof(number1)) {
						perror("apotuxia diavasmatos");
						exit(1);
				}

				if (read(new_pipe[0],&number2,sizeof(number2))!=sizeof(number2)) {
						perror("apotuxia diavasmatos");
						exit(1);
				}
				if (strcmp(root->name,"*")==0){
						result=number1*number2;
				}
				else if (strcmp(root->name,"+")==0){
						result=number1+number2;
						printf("%s %d\n","to apotelesma einai",number2);
				}
				if (write(pipefd[1],&result,sizeof(result))!=sizeof(result)) {
						perror("failed to write to parent"); //apotelesma pra3is gia na to lavei o parent
						exit(1);
				}
				exit(0);
		}
}


		int main (int argc, char* argv[]) {
				int result;
				int fds[2];	
				pid_t pid;
				struct tree_node *root;
				int status;
				if (argc!=2) {
						fprintf(stderr, "Usage: %s <input_tree_file>\n\n", argv[0]);
						exit(1);
				}
				root=get_tree_from_file(argv[1]);
				if (pipe(fds)<0) { // an to pipe epistrepsei -1 exoume error kanonika prepei na epistrefei 0
						perror("pipe in main");
						exit(1);
				}
				pid=fork();
				if (pid<0) {
						perror("Provlima stin main");
						exit(1);
				}
				if (pid==0) {
						close(fds[0]); // kathe child einai write-only
						calculate(root,fds);
						exit(0);
				}
				close(fds[1]); //kathe goneas einai read-only
				sleep(1);
				show_pstree(pid);
				wait(&status);
				explain_wait_status(pid, status);
				if (read(fds[0],&result,sizeof(result)) != sizeof(result)){ //read from the pipe the final result
						perror("read from pipe final \n");
						exit(1);
				}
				printf("Teliko apotelesma dentrou %d\n",result);
				return 0;
		}




















