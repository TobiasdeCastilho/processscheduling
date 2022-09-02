#include<stdio.h>
#include<stdlib.h>

typedef struct __ProcessFinished{
   int pid;
   char *name;
   struct __ProcessFinished *next;
}ProcessFinished;

typedef struct __ProcessDefs{
   int pid, priority, blocks, runs;
   char *name;
}ProcessDefs;

typedef struct __ProcessChain{
   struct __ProcessDefs *process;
   struct __ProcessChain *next;
} ProcessChain;

typedef struct __ProcessQuery{
   int priority;
   ProcessChain *first, *last;
   struct __ProcessQuery *nextquery;
} ProcessQuery;

typedef struct __Control{
   int priorities, prioritiescoef, id;
   ProcessDefs *running, *blocked;
   ProcessFinished *ended, *lastended; 
   ProcessQuery *lastquery, *querys;
} Control;

int NewProcess(Control *c, int runs, int blocks, int priority, char *n);
int NewControl(Control **c, int priorities, int prioritiescoef);
static int AddProcess(Control *c, ProcessDefs *p);
static int Block(Control *c, ProcessDefs *p);
static int End(Control *c, ProcessDefs *p);
static int Run(Control *c, ProcessDefs *p);
int ControlRun(Control *c, int rundelay);

int NewControl(Control **c, int priorities, int prioritiescoef){
   if(priorities <= 0)
      exit(2);
   *c = (Control *) malloc(sizeof(Control));
   (*c)->id = 0;
   (*c)->blocked = NULL;
   (*c)->running = NULL;
   (*c)->ended = NULL;
   (*c)->lastended = NULL;
   (*c)->querys = NULL;
   (*c)->lastquery = NULL;
   (*c)->priorities = priorities;
   (*c)->prioritiescoef = prioritiescoef;
   for(int i = 0;i < priorities;i++){
      ProcessQuery *pq = (ProcessQuery *) malloc(sizeof(ProcessQuery));
      pq->nextquery = NULL;
      pq->first = NULL;
      pq->last = NULL;
      pq->priority = i;
      if((*c)->querys == NULL)
         (*c)->querys = pq;
      else
         ((*c)->lastquery)->nextquery = pq;
      (*c)->lastquery = pq;
   }
   return 0;
}

int NewProcess(Control *c, int runs, int blocks, int priority, char *n){
   if(runs <= 0 && blocks <= 0)
      return 1;
   if(priority < 0)
      priority = 0;
   ProcessDefs *p = (ProcessDefs *) malloc(sizeof(ProcessDefs));
   p->runs = runs;
   p->blocks = blocks;
   p->priority = priority;
   p->pid = -1;
   p->name = n;
   return AddProcess(c,p);
}

static int AddProcess(Control *c, ProcessDefs *p){
   if(p->priority >= c->priorities)
      p->priority = c->priorities - 1;
   else if(p->priority < 0)
      p->priority = 0;
   ProcessChain *pc = (ProcessChain *) malloc(sizeof(ProcessChain));
   if(p->pid == -1){
      p->pid = c->id;
      (c->id)++;
   }
   pc->next = NULL;
   pc->process = p;
   ProcessQuery *pq = c->querys;
   while(pq != NULL && pq->priority != p->priority)
      pq = pq->nextquery;
   if(pq->first == NULL)
      pq->first = pc;
   else
      (pq->last)->next =  pc;
   pq->last = pc;
   return 0;
}

static int Block(Control *c, ProcessDefs *p){
   if(c->blocked != NULL){
      ((c->blocked)->priority) -= (c->prioritiescoef);
      AddProcess(c, c->blocked);
   }
   p->blocks--;
   c->blocked = p;
   if(c->running == p)
      c->running = NULL;
   return 0;
}

static int End(Control *c, ProcessDefs *p){
   ProcessFinished *pf = (ProcessFinished *) malloc(sizeof(ProcessFinished));
   pf->pid = p->pid;
   pf->name = p->name;
   pf->next = NULL;
   if(c->ended == NULL)
      c->ended = pf;
   else 
      (c->lastended)->next = pf;
   c->lastended = pf;
   if(c->running == p)
      c->running = NULL;
   else if(c->blocked == p)
      c->blocked = NULL;
   free(p);
   return 0;
}

static int Run(Control *c, ProcessDefs *p){
   if(c->running != NULL)
      if((c->running)->blocks > 0)
         Block(c, c->running);
      else if((c->running->runs) > 0){
         ((c->running)->priority) += (c->prioritiescoef);
         AddProcess(c, c->running);
      }else
         End(c, c->running);
   if(p->runs > 0)
      p->runs--; 
   c->running = p;
   return 0;
}

int ControlRun(Control *c, int rundelay){
   while(1){
      ProcessQuery *pq = c->querys;
      while(pq != NULL)
         if(pq->first != NULL)
            break;
         else
            pq = pq->nextquery;
      if(pq == NULL){
         if(c->running != NULL)
            if((c->running)->blocks)
               Block(c, c->running);
            else if((c->running)->runs){
               AddProcess(c, c->running);
               c->running = NULL;
            }else
               End(c, c->running);
         else if(c->blocked != NULL){
            AddProcess(c, c->blocked);
            c->blocked = NULL;
         }
         else
            break;
         continue;
      }
      if(((pq->first)->process)->blocks)
         Block(c, (pq->first)->process);
      else
         Run(c, (pq->first)->process);
      pq->first = (pq->first)->next;
      if((pq->first) == NULL)
         pq->last = NULL;
   }
   return 0;
}
