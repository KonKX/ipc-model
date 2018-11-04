#include "semun.h"

void down(int semid)
{
    struct sembuf sem_b = {.sem_num = 0, .sem_op = -1, .sem_flg = 0};

    if((semop(semid, &sem_b, 1)) < 0)
    {
        printf("ERROR in semop(), on P() signal\n\aSYSTEM: %s\n\n", strerror(errno));
        exit(getpid());
    }
}

void up(int semid)
{
    struct sembuf sem_b = {.sem_num = 0, .sem_op = 1, .sem_flg = 0};

    if((semop(semid, &sem_b, 1)) < 0)
    {
        printf("ERROR in semop(), on V() signal\n\aSYSTEM: %s\n\n", strerror(errno));
        exit(getpid());
    }
}

int request_shared_mem(int size)
{
    int shmid;

    if((shmid=shmget(SHMKEY, size, PERMS | IPC_CREAT)) < 0)
    {
        printf("ERROR in shmget()\n\aSYSTEM: %s\n\n", strerror(errno));
        exit(1);
    }

    return shmid;
}

char *attach(int shmid)
{
    char *shmem=NULL;

    if((shmem = shmat(shmid, (char *)0, 0))==(char *)-1)
    {
        printf("ERROR in shmat()\n\aSYSTEM: %s\n\n", strerror(errno));
        exit(1);
    }

    return shmem;
}

void create_shared_mem()
{
    SHMID = request_shared_mem(sizeof(struct shared_use));
    ptr_shared_use = (struct shared_use *)((void *)attach(SHMID));
}

void detach_shared_mem(char *addr)
{
    if(shmdt(addr) < 0)
        printf("ERROR in shmdt()\n\aSYSTEM: %s\n\n", strerror(errno));

}

void remove_shared_mem(int shmid)
{
    struct shmid_ds dummy;

    if((shmctl(shmid, IPC_RMID, &dummy)) < 0)
        printf("ERROR in shmctl()\n\aSYSTEM: %s\n\n", strerror(errno));
}

void sem_init(int semid, int val)
{

    union semun arg;

    arg.val = val;

    if (semctl(semid, 0, SETVAL, arg) < 0)
    {
        printf("ERROR in sem_init()\n\aSYSTEM: %s\n\n", strerror(errno));
        exit(1);
    }
}

void sem_remove(int semid)
{
    if((semctl(semid, 0, IPC_RMID, 0)) < 0)
        printf("ERROR in semctl()\n\aSYSTEM: %s\n\n", strerror(errno));
}

long get_current_time_with_ms (void)
{
    long   ms;
    time_t s;
    struct timespec spec;

    clock_gettime(CLOCK_REALTIME, &spec);

    s  = spec.tv_sec;
    ms = round(spec.tv_nsec / 1.0e6); // Convert nanoseconds to milliseconds

    return ms;
}

int main(int argc, const char *argv[])
{
    if(argc < 3)
    {
        printf("give <values> <consumers>\n");
        exit(1);
    }
    if(atoi(argv[1]) < 3000)
    {
        printf("number of values must be > 3000\n");
        exit(1);
    }

    int i, j, m, status;
    double avg_time = 0;

    pid_t pid;

    srand(time(NULL));

    int num_of_values = atoi(argv[1]); //M
    int num_of_consumers = atoi(argv[2]); //n

    create_shared_mem(); //create shared memory and attach it to a global variable

    int *values = calloc(num_of_values, sizeof(int)); //table of values

    int *sems = calloc(num_of_consumers + 3, sizeof(int)); //table of semaphores

    sems[0] = semget(SEMKEY, 1, IPC_CREAT | PERMS);  //semaphore used for counting
    if (sems[0] < 0)
    {
        printf("ERROR in semget()\n\aSYSTEM: %s\n\n", strerror(errno));
        exit(1);
    }

    sems[1] = semget(SEMKEY + 1, 1, IPC_CREAT | PERMS);  //empty
    if (sems[1] < 0)
    {
        printf("ERROR in semget()\n\aSYSTEM: %s\n\n", strerror(errno));
        exit(1);
    }

    sems[2] = semget(SEMKEY + 2, 1, IPC_CREAT | PERMS);  //full
    if (sems[2] < 0)
    {
        printf("ERROR in semget()\n\aSYSTEM: %s\n\n", strerror(errno));
        exit(1);
    }

    for(i = 3; i < num_of_consumers + 3; i++)
    {
        sems[i] = semget(SEMKEY + i, 1, IPC_CREAT | PERMS);
        if (sems[i] < 0)
        {
            printf("ERROR in semget()\n\aSYSTEM: %s\n\n", strerror(errno));
            exit(1);
        }
    }

    sem_init(sems[0], num_of_consumers); //start with n
    sem_init(sems[1], 1);
    sem_init(sems[2], 0);

    for(i = 3; i < num_of_consumers + 3; i++) //consumers sleeping
        sem_init(sems[i], 0);

    for(i = 0; i < num_of_consumers; i++)
    {
        printf("fork...\n");
        if((pid = fork())< 0)
        {
            printf("ERROR in fork()\n\aSYSTEM: %s\n\n", strerror(errno));
            exit(1);
        }
        else if(!pid) break;

    }
    if(!pid) //I'm the child
    {
        int semValue;
        FILE* fp = fopen("output", "w");

        int *values_read= calloc(num_of_values, sizeof(int));

        printf("eimai to paidi %d sem_pos = %d\n", getpid(), i);

        for(j = 0; j < num_of_values; j++)
        {
            down(sems[i+3]);

            down(sems[2]); //other consumers need to wait

            values_read[j] = ptr_shared_use -> v; //consumer keeps the value

            avg_time += get_current_time_with_ms() - ptr_shared_use -> t;
            avg_time /= 2;

            down(sems[0]); //semaphore counter -1


            union semun arg;
            semValue = semctl(sems[0], 0, GETVAL, arg);

            if(!semValue)  //if all consumers have done their work
            {
                sem_init(sems[0], num_of_consumers); //reset counter
                up(sems[1]); //unblock feeder
            }
            else up(sems[2]); //unblock next consumer

        }

        if(!semValue) //final int has been written
        {
            printf("Average delay time:\n");
            printf("%.15lf ms\n", avg_time);

            fprintf(fp, "Consumer with PID %d has finished running\n", getpid());
            fprintf(fp, "Output:\n");
            for(m = 0; m < num_of_values; m++)
                fprintf(fp, "%d\n", values_read[m]);
            fprintf(fp,"Average delay time:\n");
            fprintf(fp,"%.15lf ms\n", avg_time);

        }

        fclose(fp);
        free(values_read);

        exit(0);
    }
    else //I'm the parent
    {
        int k;
        printf("eimai o feeder!\n");
        for(k = 0; k < num_of_values; k++)
        {
            down(sems[1]); //feeder will start writing

            values[k] = rand() % 100; //fill the feeder's array with values < 100

            ptr_shared_use -> v = values[k];
            ptr_shared_use -> t = get_current_time_with_ms();  //get current time in ms

            for(j = 3; j < num_of_consumers + 3; j++) up(sems[j]);//wake up all consumers

            up(sems[2]); //memory is full
        }
    }

    while((wait(&status)) > 0);

    free(values);
    for(i = 0; i < num_of_consumers + 3; i++)
        sem_remove(sems[i]);
    detach_shared_mem((char *)ptr_shared_use);
    remove_shared_mem(SHMID);

    return 0;
}
