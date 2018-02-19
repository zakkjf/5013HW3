#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdarg.h>     /* va_list, va_start, va_arg, va_end */
#include <sys/syscall.h>
#include "dll.h"

#define FILEPATH "log.txt"
#define FILEPATH2 "random.txt"
struct info{
	char * logfile;
	char * infile;
};

static FILE *fp;

static pthread_t thread1;
static pthread_t thread2;

static pthread_mutex_t printf_mutex;
static pthread_mutex_t log_mutex;

int sync_printf(const char *format, ...)
{
    va_list args;
    va_start(args, format);

    pthread_mutex_lock(&printf_mutex);
    vprintf(format, args);
    pthread_mutex_unlock(&printf_mutex);

    va_end(args);
}

void sig_handler(int sig)
{
  if (sig == SIGINT)
    printf("SIGINT\n");


  if (sig == SIGUSR1 || SIGUSR2)
	//exit threads. Cannot distinguish between threads when force-sending signals
	sync_logwrite(FILEPATH,"Received SIGUSR1");
	pthread_cancel(thread1);
	pthread_cancel(thread2);

}


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
/* this function is run by the second thread */
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


/* this function is run by the second thread */
void *thread2_fnt(struct info *nfo)
{

//sync_printf("In  Thread 2\n");
sync_logwrite(nfo->logfile,"Thread 2","Thread 2");
sync_log_id(nfo->logfile,"Thread 2");

long double a[4], b[4], loadavg;
FILE *fp2;
char dump[50];

struct timespec tim, tim2;
tim.tv_sec = 0;
tim.tv_nsec = 100000000l;

while(1)
{
    //using difference method
    fp2 = fopen("/proc/stat","r");
    fscanf(fp2,"%*s %Lf %Lf %Lf %Lf",&a[0],&a[1],&a[2],&a[3]);
    fclose(fp2);
    nanosleep(&tim , &tim2);

    fp2 = fopen("/proc/stat","r");
    fscanf(fp2,"%*s %Lf %Lf %Lf %Lf",&b[0],&b[1],&b[2],&b[3]);
    fclose(fp2);

    loadavg = 100*((b[0]+b[1]+b[2]) - (a[0]+a[1]+a[2])) / ((b[0]+b[1]+b[2]+b[3]) - (a[0]+a[1]+a[2]+a[3]));
    sync_printf("CPU: %Lf\%\n",loadavg);
}

//return(0);
return NULL;

}

int main()
{
    if (signal(SIGUSR1, sig_handler) == SIG_ERR)
    printf("\ncan't catch SIGUSR1\n");

    if (signal(SIGUSR2, sig_handler) == SIG_ERR)
    printf("\ncan't catch SIGUSR2\n");

    pthread_mutex_init(&printf_mutex, NULL);
    pthread_mutex_init(&log_mutex, NULL);

	struct info *nfo = (struct info*) malloc(sizeof(struct info));


	nfo->infile = FILEPATH2;
	nfo->logfile = FILEPATH;

    sync_logwrite(nfo->logfile,"Thread Main","Thread Main");
    sync_log_id(nfo->logfile,"Thread Main");

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
        sync_logwrite(nfo->logfile,"Thread 1","Successfully joined");
        sync_timetag("Thread 1 End");
    }

    if(pthread_join(thread2, NULL)) {

        sync_logwrite(nfo->logfile,"Thread 2","Joining error");
        return 2;

    }
    else
    {
        sync_logwrite(nfo->logfile,"Thread 2","Successfully joined");
        sync_timetag("Thread 2 End");
    }

    /* show the results - x is now 100 thanks to the second thread */
    //printf("x: %d, y: %d\n", x, y);

    free(nfo);
    return 0;

}
