#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "tree.h"
#include "proc-common.h"
void let_it_fork(struct tree_node * root,int []);
int main(int argc, char *argv[])
{
		int calc;
		pid_t pid;
		int status;
		struct tree_node *root;
		int filedes[2]; //variable for pipe
	//	one cell to read one to write
				if (argc < 2){
						fprintf(stderr, "Usage: %s <tree_file>\n", argv[0]);
						exit(1);
				}
		/* Read tree into memory */
		root = get_tree_from_file(argv[1]);
		//create the pipe
		if(pipe(filedes)<0){
				perror("pipe");
				exit(1);
		}
		/* Fork root of process tree */
		pid = fork();
		if (pid < 0) {
				perror("main: fork");
				exit(1);
		}
		if (pid == 0) {
				/* Child */
				/* root of my tree */
				close(filedes[0]); //child is write-only
				let_it_fork(root,filedes);
				exit(0);
		}
		close(filedes[1]); //father is read-only
		/*
		 * Father
		 */
		/* for ask2-signals */
		//////////wait_for_ready_children(1);
		/* for ask2-{fork, tree} */
		/* sleep(SLEEP_TREE_SEC); */
		/* Print the process tree root at pid */
		sleep(3);
		show_pstree(pid);
		/////////kill(pid, SIGCONT);
		/* Wait for the root of the process tree to terminate */
		wait(&status);
		explain_wait_status(pid, status);
		if (read(filedes[0],&calc,sizeof(calc)) != sizeof(calc)){ //read from the pipe the final result
				perror("read from pipe\n");
				exit(1);
		}
		printf("The result of my expression tree is %d\n",calc);
		return 0;
}
void let_it_fork( struct tree_node * root, int* filedes )
{ int filedesnew[2];
		int res;
		int num1,num2; //variables to read numbers
		//definition of the necessary variables
		change_pname(root->name); //gia na allaksoume to id ka8e diadikasias sto epi8ymhto onoma
		int i;
		int status;
		pid_t p[root->nr_children];
		//create an array to hold all my children's PIDs so as to send SIGCONT
		int num;
		if (root->nr_children == 0){
				printf("Process %s started. It's a leaf\n",root->name);
				//// printf("Process %s: Waiting for SIGCONT\n",root->name);
				///////////// raise(SIGSTOP);
				sleep(10);
				printf("PID = %ld, name = %s is awake\n",
								(long)getpid(), root->name);
				//computation with pipes
				//its a leaf so write to the pipe
				num = atoi(root->name);//convert string to its equivalent integer
				if (write(filedes[1],&(num),sizeof(num)) != sizeof(num)) {
						perror("write to pipe\n");
						exit(1);
				}
				//printf("number %d\n",num);
				printf("Process %s: Exiting\n",root->name);
				exit(0);
		}
		else{
				//create the new pipe for parent and his children
				if(pipe(filedesnew)<0){
						perror("pipe");
						exit(1);
				}
				//periptwsh pou exoume paidia, prepei na ta kanoume fork
				pid_t pid;
				printf("Process %s started creating children\n",root->name);
				for (i=0;i<root->nr_children;i++){ //max na treksei 2 fores afou exoume 2 paidia h
						//kanena
						pid=fork();
						if (pid<0){
								perror("problem with fork\n");
								exit(1);
						}
						if (pid == 0){
								close(filedesnew[0]); //child is write-only
								let_it_fork(root->children+i,filedesnew); //child, recursive call gia ka8e
								//pointer se child
								printf("Process %s : Exiting\n",root->name);
								exit(0);
						}
						else{ //so as to enter only for the parent
								p[i]=pid;
						}
						//////////wait_for_ready_children(1); //waits for every child
				}
				close(filedesnew[1]);//father is read-only
				//wait_for_ready_children(root->nr_children);
				////// printf("PID= %ld, name = %s is waiting for SIGCONT\n",(long)getpid(),root->name);
				/////////////raise(SIGSTOP); //waiting for SIGCONT
				printf("PID = %ld, name = %s is awake\n",
								(long)getpid(), root->name);
				//edw prepei na steilei SIGCONT diadoxika sta paidia ths kai na perimenei na teleiwsoun
				//o,ti exoun na kanoun
				for(i=0;i<root->nr_children;i++){
						//////////////kill(p[i],SIGCONT);
						wait(&status);
						explain_wait_status(p[i],status);
				}
				//computations with pipes
				//read numbers pou exoun grapsei sto pipe ta paidia tou
				if (read(filedesnew[0], &num1 , sizeof(num1)) != sizeof(num1)){
						perror("read from pipe\n");
						exit(1);
				}
				if (read(filedesnew[0],&num2,sizeof(num2)) != sizeof(num2)){
						perror("read from pipe\n");
						exit(1);
				}
				//printf("\n\n\n %d,,,,,%d\n\n\n",num1,num2);
				if (strcmp(root->name,"*")==0){
						res=num1*num2;
						//(root->name)=itoa(res);
				}
				else if (strcmp(root->name,"+")==0){
						res=num1+num2;
				}
				printf("\ncurrent result: %d \n",res);
				//write to result tou computation sto pipe pou 8a dei o pateras sou
				if (write(filedes[1],&res,sizeof(res)) != sizeof(res)) {
						perror("write to pipe\n");
						exit(1);
				}
				// pid=wait(&status);
				// explain_wait_status(pid,status);
				exit(0);
		}
}
