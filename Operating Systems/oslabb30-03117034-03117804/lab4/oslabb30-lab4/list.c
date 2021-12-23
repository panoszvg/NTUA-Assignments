#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>

#include "list.h"

//creates a new process
struct process* Create_process (pid_t pid) {

		struct process *temp;
		temp = malloc(sizeof(*temp));
		if (temp == NULL) {
				perror("Error creating proccess");
				exit(1);
		}

		temp->pid = pid;
		temp->next = NULL;

		return temp;

} 

struct list* Create_list () {

		struct list* temp;
		temp = malloc(sizeof(*temp));
		if (temp == NULL) {
				perror("Error creating list");
				exit(1);
		}

		temp->root = NULL;
		temp->counter = 0;

		return temp;

}

//add a process at the end of the list
void Add_process (struct list *List,struct process *proc) {

		struct process *temp = List->root;
		struct process *temp2 = temp;
		if(temp == NULL){
				List->root = proc;
				List->counter = List->counter + 1;
		}
		else{
				while (temp != NULL) {
						//printf("trww kark atermono\n");
						temp2 = temp; //temp2 will find the last process
						temp = temp->next; //temp will always turn out NULL
				}

				proc->next = NULL;
				temp2->next = proc;
				List->counter = List->counter + 1;
		}
}

//remove a process from the beginning of the list
struct process* Cycle_process (struct list *List) {

		struct process *temp = List->root;
		if (temp == NULL) return NULL;
		if (List->counter == 1){
				List->root = NULL;
				List->counter--;
				return temp;
		}
		struct process *temp2 = temp->next;	
		List->root = temp2;
		List->counter = List->counter - 1;
		return temp;
}

void Destroy_process (struct list *List,pid_t pid) {

		struct process *temp = List->root;
		if (temp == NULL) return;

		struct process *temp2 = temp;
		while (temp != NULL && temp->pid != pid) {
				temp2 = temp; 
				temp = temp->next; 
		}
		if (temp != NULL) {

		temp2->next=temp->next;
		free(temp);
		}

}

