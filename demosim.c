#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <assert.h>
#include <sys/wait.h>
#include <pthread.h>
#include <semaphore.h>

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
sem_t join_leave; // sem to regulate room access
sem_t demo_a; // sem for demo assistant
sem_t demo_s; // sem for demo student
sem_t ready_a; // sem for demo assistant
sem_t end_s; // sem for demo end students
sem_t leaving_a; // sem for leaving assistannts

int student_count = 0; // number of students currently in the room
int assistant_count = 0; // number of assistants currently in the room
int waiting_students = 0; // students waiting for others for a demo
int ready_students = 0; // students ready to join demo


void* assistant_func() {
	// enter 
	pthread_mutex_lock(&lock);
	printf("Thread ID: %ld, Role:Assistant, I entered the classroom.\n", pthread_self());
	assistant_count++;
	if(student_count <= ((assistant_count*3)-1)) {
		sem_post(&join_leave);
	}
	pthread_mutex_unlock(&lock);

	sem_wait(&demo_a); // assistant waits for 2 students before ending the democd 
	
	//demo
	pthread_mutex_lock(&lock);
	printf("Thread ID: %ld, Role:Assistant, I am now participating.\n", pthread_self());
	pthread_mutex_unlock(&lock);

	// inform two students
	pthread_mutex_lock(&lock);
	sem_post(&demo_s);
	sem_post(&demo_s);
	pthread_mutex_unlock(&lock);

	// students inform assistant
	sem_wait(&ready_a);

	// students can now leave
	pthread_mutex_lock(&lock);
	sem_post(&end_s);
	sem_post(&end_s);
	pthread_mutex_unlock(&lock);

	//end
	pthread_mutex_lock(&lock);
	printf("Thread ID: %ld, Role:Assistant, demo is over.\n", pthread_self());
	pthread_mutex_unlock(&lock);

	//leave
	sem_wait(&leaving_a);
	pthread_mutex_lock(&lock);
	printf("Thread ID: %ld, Role:Assistant, I left the classroom.\n", pthread_self());
	pthread_mutex_unlock(&lock);

	return NULL;
}

void* student_func() {
	// arrive
	pthread_mutex_lock(&lock);
	printf("Thread ID: %ld, Role:Student, I want to enter the classroom.\n", pthread_self());
	pthread_mutex_unlock(&lock);

	// enter
	sem_wait(&join_leave); // can enter only if there are enough assistants
	pthread_mutex_lock(&lock);
	student_count++;
	// if #students <= #assistants*3-1, than another student can join
	if(student_count <= ((assistant_count*3)-1)) {
		sem_post(&join_leave);
	}
	printf("Thread ID: %ld, Role:Student, I entered the classroom.\n", pthread_self());	
	pthread_mutex_unlock(&lock);

	//demo
	// once joined the room, it can join a demo and wait for it to finish
	pthread_mutex_lock(&lock);
	waiting_students++;
	if (waiting_students%2 == 0) {
        sem_post(&demo_a); // inform assistant
    }
    pthread_mutex_unlock(&lock);

	sem_wait(&demo_s); // wait for assistant
	pthread_mutex_lock(&lock);
	printf("Thread ID: %ld, Role:Student, I am now participating.\n", pthread_self());
	ready_students++;
	if (ready_students%2 == 0) {
        sem_post(&ready_a);
    }
	pthread_mutex_unlock(&lock);

	sem_wait(&end_s); // wait for demo to finish

	//leave
	pthread_mutex_lock(&lock);
	student_count--;
	if(student_count <= ((3*assistant_count)-3)) {
		assistant_count--;
        sem_post(&leaving_a);
    }
	printf("Thread ID: %ld, Role:Student, I left the classroom.\n", pthread_self());
	pthread_mutex_unlock(&lock);
	
	return NULL;
}

int main(int argc, char *argv[]) {
	int num_a = atoi(argv[1]); // atoi() converts char into integer
	int num_s = atoi(argv[2]);
	
	if(num_a > 0 && (num_s == (2*num_a))) {
		printf("My program compiles with all the conditions, it has extra restrictions.\n");
		
		// threads
		pthread_t assistants[num_a];
		pthread_t students[num_s];

		sem_init(&join_leave, 0, -1);
		sem_init(&demo_s, 0, 0);
		sem_init(&demo_a, 0, 0);
		sem_init(&leaving_a, 0, 0);

		// create students
		for(int i=0; i<num_s; i++) {
			pthread_create(&students[i], NULL, student_func, NULL);
		}
		// create assistants
		for(int i=0; i<num_a; i++) {
			pthread_create(&assistants[i], NULL, assistant_func, NULL);
		}

		// wait for assistants and students to terminate their execution in order
		// to visualize it on the console
		for(int i=0; i<num_a; i++) {
			pthread_join(assistants[i], NULL);
		}
		for(int i=0; i<num_s; i++) {
			pthread_join(students[i], NULL);
		}

	} else {
		printf("Invalid arguments!\n");
	}

	printf("The main terminates.\n");
	return 0;
}
