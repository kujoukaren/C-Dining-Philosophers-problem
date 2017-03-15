// Mengqi Li 92059150
// Lab 1 Part A

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>

/*Global variables */
int num_threads; 
pthread_mutex_t *mutexes;
pthread_mutex_t *share_p;

/* For representing the status of each philosopher */
typedef enum{
	none,   // No forks
	one,    // One fork
	two     // Both forks to consume
} utensil;

/* Representation of a philosopher */
typedef struct phil_data{
	int phil_num;
	int course;
	utensil forks; 
}phil_data;

phil_data *philosophers;
/* ****************Change function below ***************** */

void shared_print(int id){
	pthread_mutex_lock(&share_p[id]);
	printf("No.%d Philosopher ate dinner\n", id);
	pthread_mutex_unlock(&share_p[id]);
}

void *eat_meal(void *phil_ptr){
/* 3 course meal: Each need to acquire both forks 3 times.
 *  First try for fork in front.
 * Then for the one on the right, if not fetched, put the first one back.
 * If both acquired, eat one course.
 */
	phil_data *phils = phil_ptr;
	int phil_ID = phils->phil_num;
	
	/* for three courses: */
	while(1){
		//printf("Philosopher %d trying pick up forks\n",phil_ID);
		if(philosophers[phil_ID].course != 3){
			if(phil_ID == 0){ // if first phil detected.
				if(philosophers[num_threads-1].forks != two){ //check last phil hold right fork or not
					pthread_mutex_lock(&mutexes[phil_ID]); // lock front fork.
					philosophers[phil_ID].forks = one; // set phil has one fork.
				}
			} 
			else{ // start from second phil
				if(philosophers[phil_ID-1].forks != two){ // check front fork avaliable.
					pthread_mutex_lock(&mutexes[phil_ID]); // lock front fork.
					philosophers[phil_ID].forks = one; // set phil has one fork.
					//printf("%d pick left", phil_ID);
				}
			}
			
			if (philosophers[(phil_ID+1)%num_threads].forks == none) {
				pthread_mutex_lock(&mutexes[(phil_ID+1)%num_threads]);
				philosophers[phil_ID].forks = two;
			}
			else{
				pthread_mutex_unlock(&mutexes[phil_ID]);
				philosophers[phil_ID].forks = none;
			}

			if (philosophers[phil_ID].forks == two){
				sleep(1);
				shared_print(phil_ID);
				philosophers[phil_ID].course += 1;
				pthread_mutex_unlock(&mutexes[phil_ID]);
				pthread_mutex_unlock(&mutexes[(phil_ID+1)%num_threads]);
				philosophers[phil_ID].forks = none;
			}
			
			if (philosophers[phil_ID].course == 3){
				break;
			}
		}
	}
	return 0;
}

/* ****************Add the support for pthreads in function below ***************** */
int main( int argc, char **argv ){
	int num_philosophers, error;

	if (argc < 2) {
          fprintf(stderr, "Format: %s <Number of philosophers>\n", argv[0]);
          return 0;
     }
    
    num_philosophers = num_threads = atoi(argv[1]);
	pthread_t threads[num_threads];
	philosophers = malloc(sizeof(phil_data)*num_philosophers);//Struct for each philosopher
	mutexes = malloc(sizeof(pthread_mutex_t)*num_philosophers); //Each mutex element represent a fork
	share_p = malloc(sizeof(pthread_mutex_t)*num_philosophers);

	/* Initialize structs */
	for( int i = 0; i < num_philosophers; i++ ){
		philosophers[i].phil_num = i;
		philosophers[i].course   = 0;
		philosophers[i].forks    = none;
	}
/* Each thread will represent a philosopher */

/* Initialize Mutex, Create threads, Join threads and Destroy mutex */

	error = 0;
	int mutexes_init = 0;
	int share_p_init = 0;
	
    /* Initialize Mutex */
	for(int i = 1; i < num_threads; i++){
		error =  pthread_mutex_init(&mutexes[i], NULL);
		if(error == -1){
			printf("\n Mutex initialization failed \n");
			exit(1);
		}
		mutexes_init = 1;
	}

	
	for(int i = 1; i < num_threads; i++){
		error =  pthread_mutex_init(&share_p[i], NULL);
		if(error == -1){
			printf("\n Mutex2 initialization failed \n");
			exit(1);
		}
		share_p_init = 1;
	}
	
	/* Create threads */
	for (int i = 0; i < num_threads; i++) {
		error = pthread_create(&threads[i], NULL, (void *)eat_meal, (void *)(&philosophers[i]));
		if(error != 0){
			printf("\n Thread creation failed \n");
			exit(1);
		}
	}

    /* Join threads */
	for (int i = 0; i < num_threads; i++) {
		error = pthread_join(threads[i], NULL);
		if(error!=0){
			printf("\n Threads didnt join \n");
			exit(1);
		}
	}
	
	/* Destroy mutex */
	if (mutexes_init == 1) {
		error = pthread_mutex_destroy(mutexes);
	}
	if (share_p_init == 1) {
		error = pthread_mutex_destroy(share_p);
	}
	
	/* Free Malloc from heap */
	free(philosophers);
	free(mutexes);
	free(share_p);

	
	return 0;
}