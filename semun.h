#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/signal.h>
#include <sys/times.h>
#include <sys/types.h>
#include <math.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>
#include <time.h>

#define PERMS 0666
#define SHMKEY (key_t)1234
#define SEMKEY (key_t)5678

union semun
{
	int val;
	struct semid_ds *buff;
	unsigned short *array;
};

struct shared_use{
	int v;
	long t;
}shared_use, *ptr_shared_use;

int SEMID=0;
int SHMID=0;
