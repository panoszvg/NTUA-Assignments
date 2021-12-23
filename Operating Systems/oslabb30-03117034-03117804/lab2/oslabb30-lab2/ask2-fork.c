#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "proc-common.h"

#define SLEEP_PROC_SEC  10
#define SLEEP_TREE_SEC  3

/*
 *  * Create this process tree:
 *   * A-+-B---D
 *    *   `-C
 *     */
void fork_procs(void)
{
		/*
		 * 	 * initial process is A.
		 * 	 	 */

		pid_t b,c,d;   //THA XREIASTEI NA KATASKEUASOUME TA B,C,D
		int status;

		change_pname("A");
		printf("A: Created!\n");
		printf("A: Creating child B...\n");
		b = fork(); // KATASKEUAZETE TO B
		if (b < 0) {
				perror("B: fork"); //ERROR
				exit(1); 
		}
		if (b == 0) { // AN TO B EINAI 0 TOTE EIMASTE STO PROCCESS TOU CHILD
				/* Child B */
				change_pname("B");
				printf("B: Created!\n");
				printf("B: Creating child D...\n");
				d = fork(); // TO B EXEI PAIDI TO D OPOTE KATASKEUAZOUME TO D
				if (d < 0) {
						perror("D: fork"); // ERROR
						exit(1);
				}
				if (d == 0) { // AN TO D EINAI 0 TOTE EIMASTE STO PROCCES TOU CHILD 
						/* Child D */
						change_pname("D");
						printf("D: Created!\n");
						printf("D: Sleeping...\n");
						sleep(SLEEP_PROC_SEC); // KOMVOI XWRIS PEDIA THELOUME NA EKTELOUN TIN SLEEP
						printf("D: Exiting...\n");
						exit(13);
				}
				printf("B: Waiting...\n");
				d = wait(&status); // PRWTOU KLEISOUME TO B PREPEI NA E3ASFALISOUME OTI TO PAIDI TOU EKLEISE SWSTA
				explain_wait_status(d,status);  // EPE3ERGASIA TOU TI EPESTREPSE H KLHSH TOU WAIT
				printf("B: Exiting...\n");
				exit(19);

		}
		// EXONTAS KLEISEI TO D KAI META TO B, GURNAME STO A OPOU TWRA KATASKEUAZOUME TO C
		printf("A: Creating child C...\n");
		c = fork();
		if (c < 0) {
				perror("C: fork");
				exit(1);
		}
		if (c == 0) { //AN TO C EINAI 0 TOTE EIMASTE STO PROCCESS TOU CHILD
				/* Child C */
				change_pname("C");
				printf("C: Created!\n");
				printf("C: Sleeping...\n");
				sleep(SLEEP_PROC_SEC); // EINAI FULLO ARA PREPEI NA EKTELESEI SLEEP
				printf("C: Exiting...\n");
				exit(17);
		}

		/* Parent  A */
		printf("A: Waiting...\n");
		b = wait(&status);
		explain_wait_status(b,status); // ELEGXOUME AN TA PAIDIA TOU A (B,C) EKLEISAN SWSTA
		c = wait(&status);
		explain_wait_status(c,status);
		printf("A: Exiting...\n");
		exit(16);
}

int main () {
		pid_t pid;
		int status;
		pid = fork();
		if (pid < 0) {
			perror("main: fork");
     		exit(1);
		}
		if (pid == 0) {
			fork_procs();
			exit(1);
		}
	 	sleep(SLEEP_TREE_SEC);
		sleep(SLEEP_TREE_SEC);
		/* Print the process tree root at pid */
		show_pstree(pid);
		/* Wait for the root of the process tree to terminate */
		pid = wait(&status);
		explain_wait_status(pid, status);

		return 0;
}
