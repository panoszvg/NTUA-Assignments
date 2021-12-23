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
#include "list.h"
/* Compile-time parameters. */
#define SCHED_TQ_SEC 2                /* time quantum */
#define TASK_NAME_SZ 60               /* maximum size for a task's name */


/*
 * SIGALRM handler
 */
struct list* list;  // global list pointer

static void sigalrm_handler(int signum)
{
		if (signum != SIGALRM) {
				fprintf(stderr, "Internal error: Called for signum %d, not SIGALRM\n",
								signum);
				exit(1);
		}

		fprintf(stderr,"alarm: about to sigstop: %ld\n", (long)list->root->pid);

		kill((list->root)->pid, SIGSTOP); //SIG-ALARM HANDLER ONLY SIGSTOPS BECAUSE SIGSTOPPING SENDS A SIGCHILD SIGNAL ANYWAY
}

/* 
 * SIGCHLD handler
 */
static void sigchld_handler(int signum)
{
		pid_t p;
		int status;

		if (signum != SIGCHLD) {
				fprintf(stderr, "Internal error: Called for signum %d, not SIGCHLD\n",
								signum);
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
						perror("waitpid");
						exit(1);
				}
				if (p == 0)
						break;

				explain_wait_status(p, status);

				if (WIFEXITED(status) || WIFSIGNALED(status)) { // IF CHILD DIED NORMALLY OR SIGKILLED

						if (WIFSIGNALED(status)) {
								Destroy_process (list, p); //SIGKILLED
						}
						else { //DIED NORMALLY
								fprintf(stderr,"%ld child has died\n",(long)list->root->pid);
								fprintf(stderr,"Parent: Received SIGCHLD, child is dead. Exiting.\n");
								struct process* temp;
								temp = Cycle_process(list);
								free(temp);
								if(!(list->counter)){
										exit(3);
								}
								if (alarm(SCHED_TQ_SEC) < 0) {
										perror("ALARM\n");
										exit(1);
								}
								kill((list->root)->pid, SIGCONT);
						}
				}
				else if (WIFSTOPPED(status)) { //SIGSTOPPED
						fprintf(stderr,"%ld child has stopped\n",(long)list->root->pid);
						struct process* temp = Cycle_process(list);
						Add_process(list,temp);
						if (alarm(SCHED_TQ_SEC) < 0) {
								perror("alarm");
								exit(1);
						}
						fprintf(stderr,"stopped:about to sigcont pid: %ld\n",(long)list->root->pid);
						kill((list->root)->pid, SIGCONT);
						printf("Parent: Child has been stopped. Moving right along...\n");
				}

		}
}


/* Install two signal handlers.
 * One for SIGCHLD, one for SIGALRM.
 * Make sure both signals are masked when one of them is running.
 */
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
				perror("Sigchld");
				exit(1);
		}

		sa.sa_handler = sigalrm_handler;
		if (sigaction(SIGALRM, &sa, NULL) < 0) {
				perror("Sigalrm");
				exit(1);
		}
		/*
		 * Ignore SIGPIPE, so that write()s to pipes
		 * with no reader do not result in us being killed,
		 * and write() returns EPIPE instead.
		 */
		if (signal(SIGPIPE, SIG_IGN) < 0) {
				perror("Sigpipe");
				exit(1);
		}
}

int main(int argc, char *argv[])
{
		int nproc;
		pid_t pid;
		struct process* proc;
		list = Create_list();
		/*
		 * For each of argv[1] to argv[argc - 1],
		 * create a new child process, add it to the process list.
		 */
		nproc = argc-1; /* number of proccesses goes here */
		if (nproc == 0) {
				fprintf(stderr, "Scheduler: No tasks. Exiting...\n");
				exit(1);
		}
		int i;
		for (i = 0; i < nproc; i++){ //CREATE ALL THE PROCESSES
				pid = fork();
				if(pid > 0){
						fprintf(stderr,"about to create_process with pid %ld\n",(long)pid);	
						proc = Create_process(pid);
						fprintf(stderr,"proc created with pid %ld \n",(long)proc->pid);
						Add_process(list,proc);
				}
				else if (pid == 0) {
						raise(SIGSTOP); //PAUSE THEM 
						char *newargv[] = {argv[i+1], NULL, NULL, NULL };
						char *newenviron[] = { NULL };
						execve(argv[i+1],newargv,newenviron); //EXECUTE WHAT THEY MUST
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
		/* loop forever  until we exit from inside a signal handler. */
		while (pause());
		/* Unreachable */
		fprintf(stderr, "Internal error: Reached unreachable point\n");
		return 1;
}
