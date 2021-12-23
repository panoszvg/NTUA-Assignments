#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "proc-common.h"

#define SLEEP_PROC_SEC  10
#define SLEEP_TREE_SEC  3

#include "tree.h"

void create (struct tree_node *root) {
		change_pname(root->name); // allazoume to onoma tis proccess
		int status;
		
		printf("Proccess %s created! \n" , root->name);
		if (root->nr_children == 0){
				printf("%s: Sleeping\n",root->name);
				sleep(SLEEP_PROC_SEC); // an einai fullo theloume na koimatai
				printf("%s: Exiting\n",root->name);
				exit(1);
		}
		
		else {
				unsigned i;
				pid_t next;
				for (i=0;i<root->nr_children;i++){
						next=fork();
						if (next<0){
								perror("PROVLIMA\n");
								exit(1);
						}
						if (next==0){
								create(root->children+i);
								exit(1);
						}
				}
				while (i--) {
						printf("%s: Waiting\n",root->name);
						next=wait(&status);
						explain_wait_status(next,status);
				}
		}
		printf("Exiting: %s\n",root->name);
}
int main(int argc, char *argv[])
{
		struct tree_node *root;

		if (argc != 2) {
				fprintf(stderr, "Usage: %s <input_tree_file>\n\n", argv[0]);
				exit(1);
		}
		root = get_tree_from_file(argv[1]);
		print_tree(root);
		int status;
		
		if (root==NULL) {
				printf("%s\n","Tree is empty");
		}

		else {

				pid_t start = fork();
				if (start==0) { // an eimai sto root
						create(root);
						exit(1);
				}
				// AN eimai ektos root
				sleep(SLEEP_TREE_SEC);
				show_pstree(start);
				start = wait(&status);
				explain_wait_status(start, status);

		}

		return 0;

}           
