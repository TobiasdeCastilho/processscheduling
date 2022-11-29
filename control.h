#ifndef __header_control

#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<time.h>

typedef int ProcessState;
const ProcessState pCreating  = 0;
const ProcessState pWaiting   = 1;
const ProcessState pRunning   = 2;
const ProcessState pBlocked   = 3;
const ProcessState pFinished  = 4;
const ProcessState pStopping  = 5;
const ProcessState pStopped   = 6;
const ProcessState pKilling   = 7;

typedef struct __ProcessDefs{
   char name[10];
   int pid, priority, blocks, runs, initialblocks, initialruns, initialpriority;
   ProcessState state;
   int stop:2, kill:2;
   time_t inserted, ended;
}ProcessDefs;

typedef struct __ProcessFinished{
   struct __ProcessDefs *process;
   struct __ProcessFinished *next;
}ProcessFinished;

typedef struct __ProcessChain{
   struct __ProcessDefs *process;
   struct __ProcessChain *next;
} ProcessChain;

typedef struct __ProcessQuery{
   int priority;
   ProcessChain *first, *last;
   struct __ProcessQuery *nextquery;
} ProcessQuery;

typedef struct __ProcessList{
   struct __ProcessChain *current, *first, *last;
} ProcessList;

typedef struct __Control{
   int id, priorities, prioritiesjump;
   unsigned int rundelay, limit, numberofprocess;
   ProcessDefs *running, *blocked;
   ProcessFinished *ended, *lastended; 
   ProcessQuery *lastquery, *querys, *proccess;
   ProcessList  *processlist, *processstoped;
   //CharTree *tree;
   #ifdef __header_threadcontrol
   pthread_t *runnerhandle, *loggerhandle;
   int logger:2;
   #endif
   int run:2;
} Control;

#ifdef __header_threadcontrol
typedef struct __ProcessLogger{
   ProcessDefs *process;
   ProcessState state;
   struct  __ProcessLogger *next;
}ProcessLogger;
#endif

int ListAllProcess(Control*);
int NewControl(Control**,int,int);
int NewProcess(Control*,int,int,int,char[10]);
int NewQuery(Control*);
static int __Controller_AddProcess(Control*,ProcessDefs*);
static int __Controller_Block(Control*,ProcessDefs*);
static int __Controller_End(Control*,ProcessDefs*);
static int __Controller_Run(Control*,ProcessDefs*);
static int __Controller_Stop(Control*,ProcessDefs*);
static int __Controller_VerifyProcess(Control*,ProcessDefs*,ProcessState);
int ControlRun(Control*);

#define __header_control

#include"./code/control.c"

#endif