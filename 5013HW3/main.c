/*****************************************************************************
​ ​*​ ​Copyright​ ​(C)​ ​2018 ​by​ Zach Farmer
​ ​*
​ ​*​ ​Redistribution,​ ​modification​ ​or​ ​use​ ​of​ ​this​ ​software​ ​in​ ​source​ ​or​ ​binary
​ ​*​ ​forms​ ​is​ ​permitted​ ​as​ ​long​ ​as​ ​the​ ​files​ ​maintain​ ​this​ ​copyright.​ ​Users​ ​are
​ ​*​ ​permitted​ ​to​ ​modify​ ​this​ ​and​ ​use​ ​it​ ​to​ ​learn​ ​about​ ​the​ ​field​ ​of​ ​embedded
​ ​*​ ​software.​ Zach Farmer, ​Alex​ ​Fosdick​, and​ ​the​ ​University​ ​of​ ​Colorado​ ​are​ ​not​
 * ​liable​ ​for ​any​ ​misuse​ ​of​ ​this​ ​material.
​ ​*
*****************************************************************************/
/**
​ ​*​ ​@file​ ​main.c
​ ​*​ ​@brief​ ​pThreads example source
​ ​*
​ ​*​ ​This is the source file for a three-thread PThreads example demonstrating
 * syncronous logging, printing, signal handling, file handling,
 * and CPU utilization stats
​ ​*
​ ​*​ ​@author​ ​Zach Farmer
​ ​*​ ​@date​ ​Feb 18 2018
​ ​*​ ​@version​ ​1.0
​ ​*
​ ​*/
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdarg.h>
#include <sys/syscall.h>
#include "dll.h"
#include "main.h"

#define FILEPATH "log.txt"
#define FILEPATH2 "random.txt"
#define FILEPATH3 "/proc/stat"

static FILE *fp;

static pthread_t thread1;
static pthread_t thread2;

static pthread_mutex_t printf_mutex;
static pthread_mutex_t log_mutex;

/**
​ ​*​ ​@brief​ ​Synchronous encapsulator for printf
​ ​*
​ ​*​ ​Mutexes printf for asynchronous call protection
 * among multiple threads
​ ​*
​ ​*​ ​@param​ ​format print formatting
 * @param ... variadic arguments for print (char *, char, etc)
​ *
​ ​*​ ​@return​ void
​ ​*/
void sync_printf(const char *format, ...)
{
    va_list args;
    va_start(args, format);

    pthread_mutex_lock(&printf_mutex);
    vprintf(format, args);
    pthread_mutex_unlock(&printf_mutex);

    va_end(args);
}

/**
​ ​*​ ​@brief​ ​Signal handler for this program
​ ​*
​ ​*​ ​handles SIGUSR1 and SIGUSR2 signals,
 * which exit all child threads
​ ​*
​ ​*​ ​@param​ ​sig received signal
​ *
​ ​*​ ​@return​ void
​ ​*/
void sig_handler(int sig)
{

    if (sig == SIGUSR1)
    {
        //exit threads. Cannot distinguish between threads when force-sending signals
        sync_logwrite(FILEPATH,"All Threads", "Received SIGUSR1");
        //thread-specific handling in thread_join component of main
        pthread_cancel(thread1);
        pthread_cancel(thread2);
    }

    if (sig == SIGUSR2)
    {
        //exit threads. Cannot distinguish between threads when force-sending signals
        sync_logwrite(FILEPATH,"All Threads", "Received SIGUSR2");
        //thread-specific handling in thread_join component of main
        pthread_cancel(thread1);
        pthread_cancel(thread2);
    }


}

/**
​ ​*​ ​@brief​ ​Synchronous time-tagging function
​ ​*
​ ​*​ ​Synchronously prints a timestamp to the console
​ ​*
​ ​*​ ​@param​ msg message to accompany timestamp
​ ​*​ ​@return​ void
​ ​*/
int sync_timetag(char * msg)
{
    time_t timer;
    char buffer[26];
    struct tm* time_inf;

    time(&timer);

    time_inf = localtime(&timer);
    strftime(buffer, 26, "%m/%d/%Y %H:%M:%S\n", time_inf);
    sync_printf(msg);
    sync_printf(": ");
    sync_printf(buffer);
    return 0;
}

/**
​ ​*​ ​@brief​ Tracks alphanumeric characters into counting bins
​ ​*
​ ​*​ ​Mutexes printf for asynchronous call protection
 * among multiple threads
​ ​*
​ ​*​ ​@param​ ch char to track
 * @param arr storage structure for counted bins
​ *
​ ​*​ ​@return​ void
​ ​*/
int chartrack(char ch, int * arr)
{
    if(ch>96 && ch<123)
    {
        ch=ch-32;
    }

    if(ch>64 && ch<91)
    {
        //incrememnt the correct bin
        (*(arr+(ch-65)))++;
        ch=ch-65;
        //sync_printf("%d\n", ch);
        return 0;
    }
    else
    {
        return 1;
    }
}

/**
​ ​*​ ​@brief​ ​Synchronous logging call for thread and POSIX Ids
​ ​*
​ ​*​ ​@param​ ​filename filename of log
 * @param thread name of present thread (as char *)
​ *
​ ​*​ ​@return​ void
​ ​*/
int sync_log_id(char* filename, char* thread)
{
    pthread_mutex_lock(&log_mutex);
    int tid, pid;
    tid = syscall(SYS_gettid);
    pid = getpid();

	fp = fopen (filename, "a");

	fprintf(fp, thread);
	fprintf(fp, " : ");
	fprintf(fp, "Linux Thread ID: %d POSIX Thread ID: %d",tid,pid);
	fprintf(fp, "\n");
	//Flush file output
	fflush(fp);

	//Close the file
	fclose(fp);
    pthread_mutex_unlock(&log_mutex);

    return 0;
}

/**
​ ​*​ ​@brief​ ​Synchronous logging call
​ ​*
​ ​*​ ​logs synchonously to specified file
​ ​*
​ ​*​ ​@param​ ​filename of log
 * @param name of thread thread currently logging
 * @param log text to log
​ *
​ ​*​ ​@return​ void
​ ​*/
int sync_logwrite(char* filename, char* thread, char* log)
{
    pthread_mutex_lock(&log_mutex);

	fp = fopen (filename, "a");

	fprintf(fp, thread);
	fprintf(fp, " : ");
	fprintf(fp, log);
	fprintf(fp, "\n");
	//Flush file output
	fflush(fp);

	//Close the file
	fclose(fp);
    pthread_mutex_unlock(&log_mutex);

    return 0;
}
/**
​ ​*​ ​@brief​ ​child thread 1
​ ​*
​ ​*​ ​This child thread sorts an input random text file
 * from a doubly linked list and prints any characters
 * occurring more than 3 times to the console
​ ​*
​ ​*​ ​@param​ ​nfo info struct containing filenames for reading and logging
​ *
​ ​*​ ​@return​ void
​ ​*/
void *thread1_fnt(struct info *nfo)
{
    sync_logwrite(nfo->logfile,"Thread 1","Thread 1");
    sync_log_id(nfo->logfile,"Thread 1");
    FILE * fp;
    fp = fopen(nfo->infile, "r");

    if(fp==NULL)
    {
        sync_printf("File handle doesn't seem to exist");
        return -1;
    }
    char ch;
    int i;
    Node * alphanode = create(0);

    int alphalog[26];

    for(i=0;i<26;i++)
    {
        alphalog[i]=0;
    }

    //loop through whole file grabbing characters
    while (ch != EOF)
    {
        ch = getc(fp);
        alphanode= insert_at_end(alphanode,ch);
        /* display contents of file on screen */
    }

            //printAll(alphanode);
    while (alphanode !=NULL)
    {
        ch=peek_value(alphanode,1);
        alphanode = delete_from_beginning(alphanode);
        chartrack(ch,alphalog);
    }
    for(i=0;i<26;i++)
    {
        if(alphalog[i]==3)
        {
            char ch = i + 65;
            sync_printf(&ch);
            sync_printf(" %d",alphalog[i]);
            sync_printf("\n");
        }
    }


    if (feof(fp))
        printf("\n End of file reached.");
    else
        printf("\n Something went wrong.");

    fclose(fp);

    return NULL;
}

/**
​ ​*​ ​@brief​ ​child thread 2
​ ​*
​ ​*​ ​This child thread reports CPU utilization to the console on 100ms intervals
​ ​*
​ ​*​ ​@param​ ​nfo info struct containing filenames for reading usage stats
​ *
​ ​*​ ​@return​ void
​ ​*/
void *thread2_fnt(struct info *nfo)
{

sync_logwrite(nfo->logfile,"Thread 2","Thread 2");
sync_log_id(nfo->logfile,"Thread 2");

FILE *fp2;
long double first[4], second[4], loadavg;

struct timespec tim, tim2;
tim.tv_sec = 0;
tim.tv_nsec = 100000000l; //100ms

while(1)
{
    //using difference method for reading CPU usage from proc/stat
    fp2 = fopen(nfo->procstat,"r");
    fscanf(fp2,"%*s %Lf %Lf %Lf %Lf",&first[0],&first[1],&first[2],&first[3]);
    fclose(fp2);
    nanosleep(&tim , &tim2);

    fp2 = fopen(nfo->procstat,"r");
    fscanf(fp2,"%*s %Lf %Lf %Lf %Lf",&second[0],&second[1],&second[2],&second[3]);
    fclose(fp2);

    loadavg = 100*((second[0]+second[1]+second[2]) - (first[0]+first[1]+first[2])) / ((second[0]+second[1]+second[2]+second[3]) - (first[0]+first[1]+first[2]+first[3]));
    sync_printf("CPU: %Lf\%\n",loadavg);
}

return NULL;

}

/**
​ ​*​ ​@brief​ main
​ ​*
​ ​*​ Begins logging, calls child threads, synchonously
 * waits until child thread completion to continue,
 * initializes signal handlers, mutexes, and info object
​ ​*
​ ​*​ ​
​ ​*​ ​@return​ void
​ ​*/
int main()
{
    //attach signal handlers
    if (signal(SIGUSR1, sig_handler) == SIG_ERR)
    printf("Error: Can't catch SIGUSR1\n");

    if (signal(SIGUSR2, sig_handler) == SIG_ERR)
    printf("Error: Can't catch SIGUSR2\n");

    //initialize mutexes for logging and printing
    pthread_mutex_init(&printf_mutex, NULL);
    pthread_mutex_init(&log_mutex, NULL);

    //create and assign info struct to pass into threads
	struct info *nfo = (struct info*) malloc(sizeof(struct info));

	nfo->infile = FILEPATH2;
	nfo->logfile = FILEPATH;
	nfo->procstat= FILEPATH3;

    sync_logwrite(nfo->logfile,"Thread Main","Thread Main");
    sync_log_id(nfo->logfile,"Thread Main");
    sync_timetag("Thread Main Start");


    /* create a first thread which executes thread1_fnt(&x) */
    sync_timetag("Thread 1 Start");
    if(pthread_create(&thread1, NULL, thread1_fnt, nfo)) {

        fprintf(stderr, "Error creating Thread 1\n");
        return 1;

    }
        /* create a second thread which executes thread2_fnt(&x) */
    sync_timetag("Thread 2 Start");
    if(pthread_create(&thread2, NULL, thread2_fnt, nfo)) {

        fprintf(stderr, "Error creating Thread 2\n");
        return 1;

    }


    /* wait for the second thread to finish */
    if(pthread_join(thread1, NULL)) {

        sync_logwrite(nfo->logfile,"Thread 1","Joining error");
        return 2;

    }
    else
    {
        sync_logwrite(nfo->logfile,"Thread 1","Exiting");
        sync_timetag("Thread 1 End");
    }

    if(pthread_join(thread2, NULL)) {

        sync_logwrite(nfo->logfile,"Thread 2","Joining error");
        return 2;

    }
    else
    {
        sync_logwrite(nfo->logfile,"Thread 2","Exiting");
        sync_timetag("Thread 2 End");
    }

    sync_logwrite(nfo->logfile,"Thread Main","Exiting");
    sync_timetag("Thread Main End");
    free(nfo); //free nfo struct memory
    return 0;
}
