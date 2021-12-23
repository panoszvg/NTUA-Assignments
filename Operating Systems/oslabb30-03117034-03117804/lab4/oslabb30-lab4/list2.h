#ifndef PROC_LIST_H_
#define PROC_LIST_H_
#include <string.h>
#include <unistd.h>
#include <sys/types.h>


struct process {
	
	pid_t pid;
	struct process *next;
	int id;
	char* name;
};



struct list {

	struct process *root;
	int counter;

};

struct process* Create_process(pid_t pid,int id);
struct list* Create_list();
void Add_process(struct list *List, struct process *proc);
struct process* Remove_process (struct list *List);
pid_t Destroy_process (struct list *List,int id);

#endif
