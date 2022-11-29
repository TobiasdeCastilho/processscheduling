#ifdef __header_control

int ListAllProcess(Control *c){
   ProcessChain *pc = (c->processlist)->first;
   printf("PID  |NAME     |CPU     |I\\O     |PRIORITY");
   while(pc != NULL){
      printf("\r\n%5d|%9s|%8d|%8d|%8d",(pc->process)->pid,(pc->process)->name,(pc->process)->runs,(pc->process)->blocks,(pc->process)->priority);
      pc = pc->next;
   }   
   return 0;
}  

int ListAllFinishedProcess(Control *c){
   ProcessFinished *pf = c->ended;
   printf("PID  |NAME     |CPU - I |CPU - I |I\\O - I |I\\O - F |PRIORITY - I|PRIORITY - F|INSERTED    |ENDED       ");
   while(pf != NULL){
      printf("\r\n%5d|%9s|%8d|%8d|%8d|%8d|%12d|%12d|%.12ld|%.12ld",(pf->process)->pid,(pf->process)->name,(pf->process)->initialruns,(pf->process)->runs,(pf->process)->initialblocks,(pf->process)->blocks,(pf->process)->initialpriority,(pf->process)->priority,(pf->process)->inserted,(pf->process)->ended);
      pf = pf->next;
   }   
   return 0;
}  

int NewControl(Control **c, int priorities, int prioritiesjump){
   if(priorities <= 0)
      return 2;
   *c = (Control *) malloc(sizeof(Control));
   (*c)->id = 100;
   (*c)->blocked = NULL;
   (*c)->running = NULL;
   (*c)->ended = NULL;
   (*c)->lastended = NULL;
   (*c)->querys = NULL;
   (*c)->lastquery = NULL;
   (*c)->priorities = 0;
   (*c)->prioritiesjump = prioritiesjump;
   (*c)->run = 1;
   (*c)->logger = 1;
   (*c)->limit = 10;
   (*c)->numberofprocess = 0;
   for(int i = 0;i < priorities;i++)
      NewQuery(*c);
   (*c)->processlist = (ProcessList *) malloc(sizeof(ProcessQuery));
   ((*c)->processlist)->current = NULL;
   ((*c)->processlist)->first = NULL;
   ((*c)->processlist)->last = NULL;
   (*c)->processstoped = (ProcessList *) malloc(sizeof(ProcessQuery));
   ((*c)->processstoped)->current = NULL;
   ((*c)->processstoped)->first = NULL;
   ((*c)->processstoped)->last = NULL;
   return 0;
}

int NewProcess(Control *c, int runs, int blocks, int priority, char name[10]){
   if(c->numberofprocess == c->limit)
      return 100; //Process limit reached
   if(c->priorities-1 < priority)
      return 101; //Priority does not exist
   if(runs <= 0 && blocks <= 0)
      return 102; //Process without runs or blocks
   if(priority < 0)
      priority = 0;
   ProcessDefs *p = (ProcessDefs *) malloc(sizeof(ProcessDefs));
   p->runs = runs;
   p->initialruns = runs;
   p->blocks = blocks;
   p->initialblocks = blocks;
   p->priority = priority;
   p->initialpriority = priority;
   p->pid = -1;
   p->stop = 0;
   p->kill = 0;
   p->state = pCreating;
   for(int i = 0;i < 10;i++)
      p->name[i] = name[i];    
   int err = __Controller_AddProcess(c,p);
   if(!err){
      ProcessChain *pc = (ProcessChain *) malloc(sizeof(ProcessChain));
      pc->process = p;
      pc->next = NULL;
      if((c->processlist)->first == NULL){
         (c->processlist)->current = pc;
         (c->processlist)->first = pc;
      }else
         ((c->processlist)->last)->next = pc;   
      (c->processlist)->last = pc;
      c->numberofprocess++;
   }else
      return err;
   return 0;
}

int NewQuery(Control *c){
   ProcessQuery *pq = (ProcessQuery *) malloc(sizeof(ProcessQuery));
   pq->nextquery = NULL;
   pq->first = NULL;
   pq->last = NULL;
   pq->priority = c->priorities;
   c->priorities++;
   if(c->querys == NULL)
      c->querys = pq;
   else
      c->lastquery->nextquery = pq;
   c->lastquery = pq;
   return 0;
}

static int __Controller_AddProcess(Control *c, ProcessDefs *p){
   if(p->priority < 0)
      p->priority = 0;
   int error = __Controller_VerifyProcess(c, p, pCreating);
   if(error)
      return error;
   ProcessChain *pc = (ProcessChain *) malloc(sizeof(ProcessChain));
   if(p->pid == -1){
      p->pid = c->id;
      (c->id)++;
   }
   if(p->state==pRunning){
      p->priority+=c->prioritiesjump;
      if(p->priority>c->priorities)
         p->priority=c->priorities-1;
   }else if(p->state==pBlocked){
      p->priority-=c->prioritiesjump;
      if(p->priority<0)
         p->priority=0;
   }
   p->inserted = time(NULL);
   p->state = pWaiting;
   pc->next = NULL;
   pc->process = p;
   ProcessQuery *pq = c->querys;
   while(pq != NULL && p->priority != p->priority)
      pq = pq->nextquery;
   if(pq->first == NULL)
      pq->first = pc;
   else
      (pq->last)->next =  pc;
   pq->last = pc;
   return 0;
}

static int __Controller_Block(Control *c, ProcessDefs *p){
   int error = __Controller_VerifyProcess(c, p, pBlocked);
   if(error)
      return error;
   if(c->blocked != NULL){
      ((c->blocked)->priority) -= (c->prioritiesjump);
      __Controller_AddProcess(c, c->blocked);
   }
   p->blocks--;
   c->blocked = p;
   if(c->running == p)
      c->running = NULL;
   p->state = pBlocked;
   return 0;
}

static int __Controller_End(Control *c, ProcessDefs *p){
   ProcessFinished *pf = (ProcessFinished *) malloc(sizeof(ProcessFinished));
   p->ended = time(NULL);
   pf->process = p;
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
   p->state = pFinished;
   c->numberofprocess--;
   return 0;
}

static int __Controller_Run(Control *c, ProcessDefs *p){
   int error = __Controller_VerifyProcess(c, p, pRunning);
   if(error)
      return error;
   if(c->running != NULL)
      if((c->running)->blocks > 0)
         __Controller_Block(c, c->running);
      else if((c->running->runs) > 0){
         ((c->running)->priority) += (c->prioritiesjump);
         __Controller_AddProcess(c, c->running);
      }else
         __Controller_End(c, c->running);
   if(p->runs > 0)
      p->runs--; 
   p->state = pRunning;
   c->running = p;
   return 0;
}

static int __Controller_Stop(Control *c, ProcessDefs *p){
   //p->state = pStopped;
   return 0;
}

static int __Controller_VerifyProcess(Control *c, ProcessDefs *p,ProcessState state){
   if(p->state==pStopping){
      __Controller_Stop(c,p);
      return 200;//Process has been stoped by user
   }else if(p->state==pKilling){
      __Controller_End(c,p);
      return 200;//Process has been finished by user
   }else{
      if(state==pRunning){
         if(p->state!=pWaiting)
            return 201; //The current process state can't be change into the informed
      }else if(state==pBlocked){
         if(p->state!=pWaiting&&p->state!=pRunning)
            return 201;
      }else if(state==pWaiting){
         if(p->state!=pCreating&&p->state!=pRunning&&p->state!=pBlocked&&p->state!=pStopped)
            return 201;
      }
   }
   
   return 0;
}

int ControlRun(Control *c){
   while(1){
      sleep(c->rundelay);
      if(!c->run)
         continue;
      ProcessQuery *pq = c->querys;
      while(pq != NULL)
         if(pq->first != NULL)
            break;
         else
            pq = pq->nextquery;
      if(pq == NULL){
         if(c->running != NULL)
            if((c->running)->blocks){
               __Controller_Block(c, c->running);
            }else if((c->running)->runs){
               __Controller_AddProcess(c, c->running);
               c->running = NULL;
            }else
               __Controller_End(c, c->running);
         else if(c->blocked != NULL){
            __Controller_AddProcess(c, c->blocked);
            c->blocked = NULL;
         }
         continue;
      }
      if(((pq->first)->process)->blocks)
         __Controller_Block(c, (pq->first)->process);
      else
         __Controller_Run(c, (pq->first)->process);
      pq->first = (pq->first)->next;
      if((pq->first) == NULL)
         pq->last = NULL;
   }
   return 0;
}

#endif