#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <stdlib.h>
#include <math.h>

#define TXTSIZE 80 //megisto plhthos bytes apo ta aopoia tha apoteleitai kathe mhnyma pou diakineitai
#define MESNUM 2 //plhthos synolikwn dosolhpsiwn (LEEI STHN EKFWNHSH NA DINETAI WS PARAMETROS.. NA MHN KSEXASW NA TO FTIAKSW)

typedef struct sharedmem_in {
	int filedesc;
	int cur_pid;
} Shm_in;

typedef struct sharedmem_out {
	char* buf[80];
} Shm_out;

union semun {
    int val;
    struct semid_ds *buf;
    ushort *array;
};


int main(void) {
	
	int semin, semread, semout,semlog;
	union semun arg;
	struct sembuf semopr;
	int shmid_in;/*id of in-share-memory*/
	int shmid_out;/*id of out-share-memory*/
	Shm_in* shm1;/*pointer to in-share-memory*/
	Shm_out* shm2;/*pointer to out-share-memory*/
	pid_t pid;/*to pid tou process*/


	/*******************************************************************/
	/*create the 2 shared-memory segments and attach the 2 segments*/
	if ( ((shmid_in = shmget(1234, sizeof(Shm_in), 0666 | IPC_CREAT)) < 0 ) ||
		((shmid_out = shmget(5678, TXTSIZE+1, 0666 | IPC_CREAT)) < 0) ) {
		perror("shmget");
		exit(1);
	}
	/**/

	if ( ((shm1 = shmat(shmid_in,0,0)) == (char*) -1) ||
		((shm2 = shmat(shmid_out,0,0)) == (char*) -1) ) {
		perror("shmat");
		exit(1);
	}
	/*******************************************************************/

	/* Create semaphores*/
	semin = semget((key_t) 111, 1, IPC_CREAT | 0660);
	semread = semget((key_t) 222, 1, IPC_CREAT | 0660);
	semout = semget((key_t) 333, 1, IPC_CREAT | 0660);
	semlog = semget((key_t) 444, 1, IPC_CREAT | 0660);
	if ( (semin == -1) || (semread == -1) || (semout == -1) || (semlog == -1) )
		exit(-1);
	
	/*initialize semaphores*/
	arg.val=1;
	semctl(semin, 0, SETVAL, arg);//semin se up
	semctl(semout, 0, SETVAL, arg);//semout se up
	
	arg.val=0;
	semctl(semread, 0, SETVAL, arg);//semread se down
	semctl(semlog, 0, SETVAL, arg);//semctl se down
	
	/*fork and check with switch if it is the parent (pid != 0, the program C) or the child (pid=0, the program S)*/
	pid=fork();
	/*if error..*/
	if(pid == -1 ) {
		perror("fork");exit(1);
	}
	/**/
	
	/*...else if, it is the child process-S program*/
	else if (pid == 0) {
		printf("I'm process S - the child\n");
		int counter=0;
		while(counter < MESNUM) {//oso exw akoma aithseis gia na staloun
			/*down(semread)*/
			semopr.sem_num = 0;
			semopr.sem_op = -1;
			semopr.sem_flg = 0;
			semop(semread, &semopr, 1);
			
			//EDW DIAVAZW TA PERIEXOMENA TOU IN//
			
			
			/*up(semin)*/
			semopr.sem_num = 0;
			semopr.sem_op = 1;
			semopr.sem_flg = 0;
			semop(semin, &semopr, 1);
			
			if(fork() != 0 ) { //it is S again..
				continue;
			}
			else {//it is S'...
			
				//PAIRNW TO PID KAI TO ZHTOUMENO KEIMENO - DHLADH TA PERIEXOMENA TOU OUT, KAI TA EPEKSERGAZOMAI//
				
				/*apoktaw prosvash ston semtemp tou antistoixou C' tou opoiou exw to pid*/
				int semtempc = semget((key_t) pid /*TO PID POU MOLIS ELAVA*/, 1, IPC_CREAT | 0660);
				
				/*down(out)*/
				semopr.sem_num = 0;
				semopr.sem_op = -1;
				semopr.sem_flg = 0;
				semop(semout, &semopr, 1);
				
				//EDW GRAFW STO OUT TO PERIEXOMENO TOU TXT//
				
				/*up(semptempc)*/
				semopr.sem_num = 0;
				semopr.sem_op = 1;
				semopr.sem_flg = 0;
				semop(semtempc, &semopr, 1);
				
				//kai twra kleinw...//
				exit(0);
				
			}
		}
	}
	/**/
	
	/*...else, if it is the parent process-C program*/
	else{
		int i;
		/*for(i=0;i<MESNUM;i++) {
			//if(temp == EKTHETIKH...)*/
			if (fork() != 0 ) { //it's the C again ..
				//continue;
				printf("ITS THE C  A G A I N ! ! ! \n");
			}
			
			else { //it is C'..
				/* Create a semaphore semtemp for current process C'*/
    				int semtemp = semget((key_t) getpid(), 1, IPC_CREAT | 0660);
    				if (semtemp == -1) exit(-1);
				/*Initialize it to down*/
				arg.val=0;
				semctl(semtemp, 0, SETVAL, arg);
			
				/*down(semin)*/
				semopr.sem_num = 0;
				semopr.sem_op = -1;
				semopr.sem_flg = 0;
				semop(semin, &semopr, 1);
		
				//GRAFW EDW TO STOIXEIO STHN IN MEMORY//
		
				/*up(semread)*/
				semopr.sem_num = 0;
				semopr.sem_op = 1;
				semopr.sem_flg = 0;
				semop(semread, &semopr, 1);
		
				/*down(semtemp)*/
				semopr.sem_num = 0;
				semopr.sem_op = -1;
				semopr.sem_flg = 0;
				semop(semtemp, &semopr, 1);
			
				//EDW DIAVAZEI TA PERIEXOMENA TOU OUT//
			
				/*up(semout)*/
				semopr.sem_num = 0;
				semopr.sem_op = 1;
				semopr.sem_flg = 0;
				semop(semout, &semopr, 1);
			
				/*down(semlog)*/
				semopr.sem_num = 0;
				semopr.sem_op = -1;
				semopr.sem_flg = 0;
				semop(semtemp, &semopr, 1);
			
				//EDW GRAFEI STO LOG.TXT
			
				/*up(semlog)*/
				semopr.sem_num = 0;
				semopr.sem_op = 1;
				semopr.sem_flg = 0;
				semop(semlog, &semopr, 1);
			}
		/*}*/

	}
	/**/


	
	/*delete semaphores*/
	semctl(semin, 0, IPC_RMID, 0);
	semctl(semout, 0, IPC_RMID, 0);
	semctl(semread, 0, IPC_RMID, 0);
	semctl(semlog, 0, IPC_RMID, 0);

	
	
	/*deattach memory segments*/
	if( (shmdt(shm1) == -1 ) || (shmdt(shm2) == -1) ) {
		perror("shmdt");
		exit(1);
	} 
	/**/
	
	/*destroy memory segments*/
	if ( (shmctl(shmid_in,IPC_RMID,0) == -1) || (shmctl(shmid_out,IPC_RMID,0) == -1) ) {
		perror("shmctl");
		exit(1);
	}
	/**/
	
	return 0;
}





