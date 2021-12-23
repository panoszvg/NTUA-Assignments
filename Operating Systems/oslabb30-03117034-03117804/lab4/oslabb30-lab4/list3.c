#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>

#include "list3.h"
//Initialize a process

//creates a new process
struct process* Create_process (pid_t pid, int id) {

	struct process *temp;
	temp = malloc(sizeof(*temp));
	temp->name = NULL;
	
	if (temp == NULL) {
			perror("Error creating proccess");
			exit(1);
	}

	temp->pid = pid;
	temp->next = NULL;
	temp->id = id;

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
		proc->next=NULL;
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
struct process* Remove_process (struct list *List) {

	struct process *temp = List->root;
	if (temp == NULL) return NULL;
	if (List->counter == 1){
		List->root = NULL;
		List->counter = List->counter - 1;
		return temp;
	}
	struct process *temp2 = temp->next;	// A-B-C
	List->root = temp2;
	List->counter = List->counter - 1;
	return temp;
}

// Deletes the process from the list and returns its pid
pid_t Destroy_process (struct list *List,int id) 
{
	struct process *temp = List->root;
	if (temp == NULL) return -1;
	if (temp->id == id){
		List->root = temp->next;
		pid_t temp3 = temp->pid;
		if(temp->name != NULL)free(temp->name);
		free(temp);
		return temp3;

	}
	struct process *temp2 = temp;
	while (temp != NULL && temp->id != id) {
			temp2 = temp; //temp2 will find the last process
			temp = temp->next; //temp will always turn out NULL
	}
	if (temp != NULL){
		temp2->next=temp->next;
		pid_t temp3 = temp->pid;
		if(temp->name != NULL)free(temp->name);
		free(temp);
		return temp3;
	}
	else{ return -1; }
}

struct process* Remove_process_by_id(struct list *List, int id){
	struct process *temp = List->root;
	if (temp == NULL) return NULL;
	if (temp->id == id){
		List->root = temp->next;
		temp->next=NULL;
		return temp;
	}
	struct process *temp2 = temp;
	while (temp != NULL && temp->id != id){
		temp2 = temp;
		temp = temp->next;
	}
	if (temp != NULL){
		temp2->next=temp->next;
		temp->next=NULL;
		return temp;
	}
	else
	{
	return NULL;
	}
}
