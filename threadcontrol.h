#ifndef __header_threadcontrol

#include<pthread.h>
#include<unistd.h>

static void *listenner(void *);
static void *logger(void *);
int StartThreadControl(void *);

#define __header_threadcontrol

#include"control.h"
#include"./code/threadcontrol.c"

#endif