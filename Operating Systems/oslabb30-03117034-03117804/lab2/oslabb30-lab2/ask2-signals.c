#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "tree.h"
#include "proc-common.h"

void fork_procs(struct tree_node *root)
{
		int status;
		unsigned i;
		pid_t *p;
		printf("PID = %ld, name %s, starting...\n",	(long)getpid(), root->name);
		change_pname(root->name);
		int number=root->nr_children;
		if (number==0) {
				printf("%s: %s\n",root->name,"is a leaf");
				raise(SIGSTOP);
				printf("PID = %ld, name = %s is awake\n",(long)getpid(), root->name);
				exit(0);
		}
		else {
				p= calloc(number,sizeof(pid_t));
				// kanoume pinaka pou tha apothikeusoume ta PID twn children
				for (i=0;i<root->nr_children;i++) {
						pid_t next;
						next=fork();
						if (next<0) perror("provlima");
						if (next==0) {
								fork_procs(root->children+i);
								exit(0);
						}
						else { //edw theloume na sunexisei mono o goneas gia na kanei save ta PID twn paidiwn
								p[i]=next;
						}
						wait_for_ready_children(1);
				} 
				raise(SIGSTOP);

				printf("PID = %ld, name = %s is awake\n",(long)getpid(), root->name);
				for (i=0;i<number;i++) {
						kill(p[i],SIGCONT);
						wait(&status);
						explain_wait_status(p[i],status);
				}
				wait(&status);
				explain_wait_status(getpid(),status);
				exit(0);
		}
}

/*
 * The initial process forks the root of the process tree,
 * waits for the process tree to be completely created,
 * then takes a photo of it using show_pstree().
 *
 * How to wait for the process tree to be ready?
 * In ask2-{fork, tree}:
 *      wait for a few seconds, hope for the best.
 * In ask2-signals:
 *      use wait_for_ready_children() to wait until
 *      the first process raises SIGSTOP.
 */

int main(int argc, char *argv[])
{
		pid_t pid;
		int status;
		struct tree_node *root;

		if (argc < 2){
				fprintf(stderr, "Usage: %s <tree_file>\n", argv[0]);
				exit(1);
		}

		/* Read tree into memory */
		root = get_tree_from_file(argv[1]);

		/* Fork root of process tree */
		pid = fork();
		if (pid < 0) {
				perror("main: fork");
				exit(1);
		}
		if (pid == 0) {
				/* Child */
				fork_procs(root);
				exit(0);
		}

		wait_for_ready_children(1);
		show_pstree(pid);
		/* for ask2-signals */
		kill(pid, SIGCONT);

		/* Wait for the root of the process tree to terminate */
		wait(&status);
		explain_wait_status(pid, status);


		return 0;
}
