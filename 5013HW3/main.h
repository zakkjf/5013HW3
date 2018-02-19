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
​ ​*​ ​@file​ ​main.h
​ ​*​ ​@brief​ ​pThreads exercise header file
​ ​*
​ ​*​ ​This is the header file for a three-thread PThreads example demonstrating
 * syncronous logging, printing, signal handling, file handling,
 * and CPU utilization stats
​ ​*
​ ​*​ ​@author​ ​Zach Farmer
​ ​*​ ​@date​ ​Feb 18 2018
​ ​*​ ​@version​ ​1.0
​ ​*
​ ​*/

#ifndef MAIN_H_INCLUDED
#define MAIN_H_INCLUDED

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

struct info{
	char * logfile;
	char * infile;
	char * procstat;
};

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
void sync_printf(const char *format, ...);

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
void sig_handler(int sig);

/**
​ ​*​ ​@brief​ ​Synchronous time-tagging function
​ ​*
​ ​*​ ​Synchronously prints a timestamp to the console
​ ​*
​ ​*​ ​@param filename log filename
 * @param​ msg message to accompany timestamp
​ ​*​ ​@return​ void
​ ​*/
int sync_timetag(char * filename, char * msg);

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
int chartrack(char ch, int * arr);

/**
​ ​*​ ​@brief​ ​Synchronous logging call for thread and POSIX Ids
​ ​*
​ ​*​ ​@param​ ​filename filename of log
 * @param thread name of present thread (as char *)
​ *
​ ​*​ ​@return​ void
​ ​*/
int sync_log_id(char* filename, char* thread);

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
int sync_logwrite(char* filename, char* thread, char* log);

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
void *thread1_fnt(struct info *nfo);

/**
​ ​*​ ​@brief​ ​child thread 2
​ ​*
​ ​*​ ​This child thread reports CPU utilization to the console on 100ms intervals
​ ​*
​ ​*​ ​@param​ ​nfo info struct containing filenames for reading usage stats
​ *
​ ​*​ ​@return​ void
​ ​*/
void *thread2_fnt(struct info *nfo);


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
int main();

#endif //MAIN_H
