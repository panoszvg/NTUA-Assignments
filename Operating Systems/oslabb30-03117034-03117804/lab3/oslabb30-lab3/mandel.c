/*
 *  * mandel.c
 *   *
 *    * A program to draw the Mandelbrot Set on a 256-color xterm.
 *     *
 *      */

#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <semaphore.h>
#include <signal.h>
#include "mandel-lib.h"

#define MANDEL_MAX_ITERATION 100000

/***************************
 *  * Compile-time parameters *
 *   ***************************/


/*
 *  * Output at the terminal is is x_chars wide by y_chars long
 *  */
int y_chars = 50;
int x_chars = 90;

/*
 *  * The part of the complex plane to be drawn:
 *   * upper left corner is (xmin, ymax), lower right corner is (xmax, ymin)
 *   */
double xmin = -1.8, xmax = 1.0;
double ymin = -1.0, ymax = 1.0;

/*
 *  * Every character in the final output is
 *   * xstep x ystep units wide on the complex plane.
 *    */
double xstep;
double ystep;

int N;

struct thread_info_struct {
		pthread_t tid; //Posix thread id
		int *lines;	   // Pointer to an array of integers who hold which LINES will be printed by this struct	
		int number;	   // Struct id in int coding: 0,1,2, etc
		sem_t my_sem; // each struct has its own semaphore, total N will be created

};

struct thread_info_struct *thr;

void *safe_malloc(size_t size)  // In case our attemtp to create the thread array fails to find the needed space in memory
{
		void *p;

		if ((p = malloc(size)) == NULL) {
				fprintf(stderr, "Out of memory, failed to allocate %zd bytes\n",size);
				exit(1);
		}

		return p;
}


/*
 *  * This function computes a line of output
 *   * as an array of x_char color values.
 *    */
void compute_mandel_line(int line, int color_val[])
{
		/*
		 * 	 * x and y traverse the complex plane.
		 * 	 	 */
		double x, y;
		int n;
		int val;

		/* Find out the y value corresponding to this line */
		y = ymax - ystep * line;
		/* and iterate for all points on this line */
		for (x = xmin, n = 0; n < x_chars; x+= xstep, n++) {

				/* Compute the point's color value */
				val = mandel_iterations_at_point(x, y, MANDEL_MAX_ITERATION);
				if (val > 255)
						val = 255;

				/* And store it in the color_val[] array */
				//		printf("Color value about to be inserted about is: %d\n", val);
				val = xterm_color(val);
				color_val[n] = val;
		}
}
/*
 * This function outputs an array of x_char color values
 * to a 256-color xterm.
 */
void output_mandel_line(int fd, int color_val[])
{
		int i;

		char point ='@';
		char newline='\n';

		for (i = 0; i < x_chars; i++) {
				/* Set the current color, then output the point */
				set_xterm_color(fd, color_val[i]);
				if (write(fd, &point, 1) != 1) {
						perror("compute_and_output_mandel_line: write point");
						exit(1);
				}
				reset_xterm_color(1);
		}

		/* Now that the line is done, output a newline character */
		if (write(fd, &newline, 1) != 1) {
				perror("compute_and_output_mandel_line: write newline");
				exit(1);
		}
}

int next(int x){ return ((x+1) % N); }

void compute_and_output_mandel_line(int fd, int line)
{
		/*
		 * A temporary array, used to hold color values for the line being drawn
		 */
		int color_val[x_chars];
		int cur_thread = line % N; // analoga me to poia grammi tupwnetai 3eroume poio thread prepei na tin analavei

		compute_mandel_line(line, color_val);

		sem_wait(&(thr[cur_thread].my_sem));	// edw stop kanoume ta threads na ipologizoun parallila to ti prepei na tupwsoun alla iparxei lock sto tipwma gia na vgei swsto
		
		output_mandel_line(fd, color_val);   // kleidwame to semaphore tou current threa kai kanoume unclock to semaphore tou epomenou thread

		sem_post(&(thr[next(cur_thread)].my_sem));
}


void *all_thread_start(void *arg)
{	
		int i;
		struct thread_info_struct *cur = arg;

		for (i = 0; i < cur->number; i++){ // pame gia kathe thread na etoimasoume to ti prepei na tupwsei
				if(cur->lines[i] == -1) 
						continue;  
				compute_and_output_mandel_line(1, cur->lines[i]);
		}
		return NULL;
}



void find_lines_for_every_thread(int N, int i, int lines[], int number){
		int counter = 0;
		int j;
		for (j = 0; j < y_chars; j++){
				if ( j % N == i){            // bazoume ston pinaka pou tha exei kathe thread mesa tou poia lines ofeilei na ftia3ei kai na tupwsei
						lines[counter] = j;
						counter++;
				}
		}
		for (j = counter; j < number; j++){
				lines[j] = -1;
		}
}

void help(int sign) {
signal(sign,SIG_IGN);
reset_xterm_color(1);
printf("Control C detected\n");
exit(1);
}

int main(int argc, char **argv)
{	
		
		if (argc != 2) {
				fprintf(stderr, "Incorrect number of arguments. Please specify exactly one argument, the number of threads to compute with\n");
				return 0;
		}
		N = atoi(argv[1]);
		if (N <= 0) {
				fprintf(stderr, "Invalid argument. Please specify an integer greater than 0\n");
				return 0;
		}
		
		int nr_of_lines = (y_chars / N) + 1;
		int ret;
		signal(SIGINT,help);
		thr = safe_malloc(N * sizeof(*thr));

		int i;
		xstep = (xmax - xmin) / x_chars;
		ystep = (ymax - ymin) / y_chars;
		for (i = 0; i < N; i++) {
				int *temp;
				temp = safe_malloc(nr_of_lines * sizeof(*temp));
				find_lines_for_every_thread(N,i,temp,nr_of_lines);
				thr[i].lines = temp;
				thr[i].number = nr_of_lines;
				if (i == 0) sem_init(&thr[i].my_sem, 0, 1);
				else sem_init(&thr[i].my_sem, 0, 0); // oloi oi shmaforoi ektos tou 1ou (midenikou) exoun timh 0, mono autos exei timh 1
				ret = pthread_create(&thr[i].tid, NULL,all_thread_start , &thr[i]);
				if (ret) {
						perror("pthread_create");
						exit(1);
				}
		}

		for (i = 0; i < N; i++){
				ret = pthread_join(thr[i].tid, NULL);
				if(ret){
						perror("pthread_join");
						exit(1);
				}
		}

		/*
		 * draw the Mandelbrot Set, one line at a time.
		 * Output is sent to file descriptor '1', i.e., standard output.
		 */
		int k;
		reset_xterm_color(1);
		for (k=0; k<N;k++) sem_destroy(&thr[k].my_sem);
		return 0;
}

