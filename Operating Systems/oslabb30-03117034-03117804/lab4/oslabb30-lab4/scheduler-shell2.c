#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <assert.h>

#include <sys/wait.h>
#include <sys/types.h>

#include "proc-common.h"
#include "request.h"
#include "list3.h"
/* Compile-time parameters. */
#define SCHED_TQ_SEC 2                /* time quantum */
#define TASK_NAME_SZ 60               /* maximum size for a task's name */
#define SHELL_EXECUTABLE_NAME "shell" /* executable for shell */

struct list *list, *list2;
int current_id;
char flag; // if 0 then remove head

static void sched_print_tasks(void)
{
		struct process* temp;
		temp = list2->root;
		printf("\nCurrently running->");
		while((temp != NULL)){
				//fprintf(stderr,"now printing list2 %d\n",temp->id);
				printf("[%s](HIGH):with pid %ld and serial_no %d\n",temp->name,(long)temp->pid,temp->id);
				temp = temp->next;
		}
		temp = list->root;
		if(temp == NULL){
				return;
		}
		do{
				printf("[%s](LOW):with pid %ld and serial_no %d\n",temp->name,(long)temp->pid,temp->id);
		}while((temp = temp->next) != NULL);

}
static int sched_kill_task_by_id(int id)
{
		if (list->root != NULL  && list->root->id == id) {
				pid_t pid = Destroy_process(list,id);
				kill(pid,SIGKILL);
				list->counter = list->counter - 1;
				flag=1;
				return 0;

		}
		else if (list2->root !=NULL && list2->root->id == id) {
				kill(list2->root->pid,SIGKILL);
				return 0;
				}
else	{
				pid_t pid = Destroy_process(list,id);
				list->counter = list->counter - 1;
				if(pid == -1){
						list->counter++;
						pid = Destroy_process(list2,id);
						list2->counter = list2->counter - 1;
						if(pid == -1){
								fprintf(stderr,"kill:Requested id doesn't exist\n");
								list2->counter++;
								return 0;
						}
				}
				flag = 1;
				kill(pid,SIGKILL);
				return 0;
		}
}

static void sched_create_task(char *executable)
{
		pid_t pid;
		pid = fork();
		if(pid < 0){
				fprintf(stderr,"FORK");
				exit(1);
		}
		else if(pid == 0){
				char *newargv[] = {executable, NULL, NULL, NULL };
				char *newenviron[] = { NULL };
				raise(SIGSTOP);
				execve(executable,newargv,newenviron);
				exit(1);
		}
		else{
				struct process* temp = Create_process(pid,current_id++);
				temp->name = strdup(executable);
				Add_process(list,temp);
		}
}

static int sched_high_task_by_id (int id) {
		char ACTIVE = 0;
		if(list->root->id == id){
				ACTIVE = 1;
		}
		struct process* removed = Remove_process_by_id(list,id);
		list->counter = list->counter - 1;
		if(removed == NULL){
				list->counter++;
				fprintf(stderr,"set_list2:Requested id doesn't exist in low\n");
				return 0;
		}
		else {
				if (list2->root == NULL){
						Add_process(list2,removed);
						if(!ACTIVE){
								kill(list->root->pid,SIGSTOP);
						}

						return 0;
				}
				else {
						Add_process(list2,removed);
						return 0;
				}
		}
}
static int sched_low_task_by_id(int id){
		char ACTIVE = 0;
		if(list2->root->id == id) {
		ACTIVE = 1;
		}
		struct process* removed = Remove_process_by_id(list2,id);
		list2->counter = list2->counter - 1;
		if(removed == NULL){
				fprintf(stderr,"set_low:Requested id doesn't exist in list2\n");
				return 0;
		}
		else{
				Add_process(list,removed);
				if(ACTIVE) kill(removed->pid,SIGSTOP);
				return 0;
		}
}

/* Process requests by the shell.  */
static int process_request(struct request_struct *rq)
{
		switch (rq->request_no) {
				case REQ_PRINT_TASKS:
						sched_print_tasks();
						return 0;

				case REQ_KILL_TASK:
						return sched_kill_task_by_id(rq->task_arg);

				case REQ_EXEC_TASK:
						sched_create_task(rq->exec_task_arg);
						return 0;

				case REQ_HIGH_TASK:
						return sched_high_task_by_id(rq->task_arg);

				case REQ_LOW_TASK:
						return  sched_low_task_by_id(rq->task_arg);

				default:
						return -ENOSYS;
		}
}

/* 
 * SIGALRM handler
 */
static void sigalrm_handler(int signum)
{
		if (signum != SIGALRM){
				fprintf(stderr,"Sigalrm handler called for a different signal");
				exit(1);
		}
		if(list2->root != NULL){
				kill((list2->root)->pid,SIGSTOP);
		}
		else{
				kill((list->root)->pid,SIGSTOP);
		}
}

/* 
 * SIGCHLD handler
 */
char first_time = 1;
static void sigchld_handler(int signum)
{
		pid_t p;
		int status;

		if (signum != SIGCHLD) {
				fprintf(stderr, "Internal error: Called for signum %d, not SIGCHLD\n",signum);
				exit(1);
		}
		/*
		 * Something has happened to one of the children.
		 * We use waitpid() with the WUNTRACED flag, instead of wait(), because
		 * SIGCHLD may have been received for a stopped, not dead child.
		 *
		 * A single SIGCHLD may be received if many processes die at the same time.
		 * We use waitpid() with the WNOHANG flag in a loop, to make sure all
		 * children are taken care of before leaving the handler.
		 */

		for (;;) {
				p = waitpid(-1, &status, WUNTRACED | WNOHANG);
				if (p < 0) {
						perror("WAIT_PID");
						exit(1);
				}
				if (p == 0)
						break;

				explain_wait_status(p, status);

				if (WIFEXITED(status) || WIFSIGNALED(status)) { //AN KATI PETHANE FUSIOLOGIKA H APO SIGKILL
						if(list2->root == NULL) { //AN EIMASTE STIN LOW LISTA
								if(!flag){ //SIGKILLED
										struct process* temp;
										temp = Remove_process(list);
										if(temp->name != NULL)free(temp->name);
										free(temp);
										if(!(list->counter)){
												exit(3);
										}
										if (alarm(SCHED_TQ_SEC) < 0) {
												perror("ALARM");
												exit(1);
										}
										kill((list->root)->pid, SIGCONT);
								}
								else {
								flag = 0;
								}
						}
						else { //AN EIMASTE STIN HIGH LISTA
								if (!flag) {//SIGKILLED
										struct process* temp;
										temp = Remove_process(list2);
										if(temp->name != NULL)
										free(temp->name);
										free(temp);
										if(!(list2->counter)){
												if(!(list->counter)){
														exit(3); //END OF PROGRAM
												}
												else {
														if (alarm(SCHED_TQ_SEC) < 0) {
																perror("ALARM");
																exit(1);
														}
														kill((list->root)->pid, SIGCONT);
														return;
												}
										}
										if (alarm(SCHED_TQ_SEC) < 0) {
												perror("ALARM");
												exit(1);
										}
										kill((list2->root)->pid, SIGCONT);
								}
								else {
								flag = 0;
								}
						}
				}
				if (WIFSTOPPED(status)) { //SIGSTOPPED
						/* A child has stopped due to SIGSTOP/SIGTSTP, etc... */		
						if (list2->root == NULL){
								//LOW PRIORITY LIST
								if (first_time == 1) {
								first_time = 0;
								return;
								}
								Add_process(list,Remove_process(list));
								/* Setup the alarm again */
								if (alarm(SCHED_TQ_SEC) < 0) {
										perror("alarm");
										exit(1);
								}
								kill((list->root)->pid, SIGCONT);				
						}
						else { //HIGH PRIORITY LIST
								Add_process(list2,Remove_process(list2));
								if (alarm(SCHED_TQ_SEC) < 0) {
										perror("alarm");
										exit(1);
								}
								kill((list2->root)->pid, SIGCONT);	
						}
				}
		}
}

/* Disable delivery of SIGALRM and SIGCHLD. */
static void signals_disable(void)
{
		sigset_t sigset;

		sigemptyset(&sigset);
		sigaddset(&sigset, SIGALRM);
		sigaddset(&sigset, SIGCHLD);
		if (sigprocmask(SIG_BLOCK, &sigset, NULL) < 0) {
				perror("signals_disable: sigprocmask");
				exit(1);
		}
}

/* Enable delivery of SIGALRM and SIGCHLD.  */
static void signals_enable(void)
{
		sigset_t sigset;

		sigemptyset(&sigset);
		sigaddset(&sigset, SIGALRM);
		sigaddset(&sigset, SIGCHLD);
		if (sigprocmask(SIG_UNBLOCK, &sigset, NULL) < 0) {
				perror("signals_enable: sigprocmask");
				exit(1);
		}
}
static void install_signal_handlers(void)
{
		sigset_t sigset;
		struct sigaction sa;

		sa.sa_handler = sigchld_handler;
		sa.sa_flags = SA_RESTART;
		sigemptyset(&sigset);
		sigaddset(&sigset, SIGCHLD);
		sigaddset(&sigset, SIGALRM);
		sa.sa_mask = sigset;
		if (sigaction(SIGCHLD, &sa, NULL) < 0) {
				perror("sigaction: sigchld");
				exit(1);
		}

		sa.sa_handler = sigalrm_handler;
		if (sigaction(SIGALRM, &sa, NULL) < 0) {
				perror("sigaction: sigalrm");
				exit(1);
		}
		if (signal(SIGPIPE, SIG_IGN) < 0) {
				perror("signal: sigpipe");
				exit(1);
		}
}

static void do_shell(char *executable, int wfd, int rfd)
{
		char arg1[10], arg2[10];
		char *newargv[] = { executable, NULL, NULL, NULL };
		char *newenviron[] = { NULL };

		sprintf(arg1, "%05d", wfd);
		sprintf(arg2, "%05d", rfd);
		newargv[1] = arg1;
		newargv[2] = arg2;
		raise(SIGSTOP);
		execve(executable, newargv, newenviron);
		/* execve() only returns on error */
		perror("scheduler: child: execve");
		exit(1);
}

/* Create a new shell task.
 *
 * The shell gets special treatment:
 * two pipes are created for communication and passed
 * as command-line arguments to the executable.
 */
static void sched_create_shell(char *executable, int *request_fd, int *return_fd)
{
		pid_t p;
		int pfds_rq[2], pfds_ret[2];

		if (pipe(pfds_rq) < 0 || pipe(pfds_ret) < 0) {
				perror("PIPE");
				exit(1);
		}

		p = fork();
		if (p < 0) {
				perror("FORK");
				exit(1);
		}

		if (p == 0) {
				/* Child */
				close(pfds_rq[0]);
				close(pfds_ret[1]);
				do_shell(executable, pfds_rq[1], pfds_ret[0]);
				assert(0);
		}
		/* Parent */
		close(pfds_rq[1]);
		close(pfds_ret[0]);
		*request_fd = pfds_rq[0];
		*return_fd = pfds_ret[1];
		struct process* temp = Create_process(p,current_id);
		temp->name = strdup("Shell");
		Add_process(list,temp);
		current_id++;
}

static void shell_request_loop(int request_fd, int return_fd)
{
		int ret;
		struct request_struct rq;
		for (;;) {
				if (read(request_fd, &rq, sizeof(rq)) != sizeof(rq)) {
						perror("SHELL ERROR");
						fprintf(stderr, "Scheduler: giving up on shell request processing.\n");
						break;
				}

				signals_disable();
				ret = process_request(&rq);
				signals_enable();

				if (write(return_fd, &ret, sizeof(ret)) != sizeof(ret)) {
						perror("scheduler: write to shell");
						fprintf(stderr, "Scheduler: giving up on shell request processing.\n");
						break;
				}
		}
}

int main(int argc, char *argv[])
{
		int nproc;
		static int request_fd, return_fd;
		current_id = 0;
		pid_t pid;
		struct process* proc;
		list = Create_list(); // LOW PRIORITY
		list2 = Create_list(); //HIGH PRIORITY
		sched_create_shell(SHELL_EXECUTABLE_NAME, &request_fd, &return_fd);
		nproc = argc-1; /* number of proccesses goes here */
		if (nproc == 0) {
				fprintf(stderr, "Scheduler: No tasks. Exiting...\n");
				exit(1);
		}
		int i;
		for(i = 0; i < nproc; i++){
				pid = fork();
				if(pid < 0){
						fprintf(stderr,"kako fork\n");
						exit(1);
				}
				if(pid > 0){
						proc = Create_process(pid,current_id);
						proc->name = strdup(argv[i+1]);
						current_id++;
						Add_process(list,proc);
				}
				else if(pid == 0){
						char *newargv[] = {argv[i+1], NULL, NULL, NULL };
						char *newenviron[] = { NULL };
						raise(SIGSTOP);
						execve(argv[i+1],newargv,newenviron);
						fprintf(stderr,"Xasame\n");
						exit(1);
				}
		}
		/* Wait for all children to raise SIGSTOP before exec()ing. */
		wait_for_ready_children(nproc);

		/* Install SIGALRM and SIGCHLD handlers. */
		install_signal_handlers();

		if (alarm(SCHED_TQ_SEC) < 0) {
				perror("alarm");
				exit(1);
		}
		kill((list->root)->pid, SIGCONT);
		shell_request_loop(request_fd, return_fd);
		while (pause())
				;

		fprintf(stderr, "Internal error: Reached unreachable point\n");
		return 1;
}
