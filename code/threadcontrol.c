#ifdef __header_threadcontrol
#ifdef __header_control

static void *runner(void *data){
  Control *c=(Control *)data;
  ControlRun((Control *)c);
}

static void *logger(void *data){
  time_t secs=time(NULL);
  char s[100];
  Control *c=(Control *)data;
  sprintf(s,"/tmp/processcontrol/logger%d.txt",c->id);
  FILE *f=fopen(s,"w");
  fprintf(f,"PID  |NAME    |CPU     |I\\O     |PRIORITY|INSERTED    |ENDED       ");
  fclose(f);
  int lastended=-1;
  while(c->logger){
    sleep(100);
    if(c->lastended!=NULL){
      struct __ProcessDefs *l=(c->lastended)->process;
      if(l->pid!=lastended){
        f=fopen(s,"a");
        fprintf(f,"\r\n%.5d|%8s|%.8d|%.8d|%.8d|%.12ld|%.12ld",l->pid,l->name,l->initialruns,l->initialblocks,l->initialpriority,l->inserted,l->ended);
        lastended=l->pid;
        fclose(f);
      }
    }
  }
}

int StartThreadControl(void *c){
	// if(access("/tmp/processcontrol/",F_OK)==-1)
	// 	system("mkdir /tmp/processcontrol");

  Control *controller=(Control *)c;
  //controller->loggerhandle = (pthread_t *) malloc(sizeof(pthread_t));
  controller->runnerhandle = (pthread_t *) malloc(sizeof(pthread_t));
  //pthread_create(controller->loggerhandle,NULL,&logger,c);
  pthread_create(controller->runnerhandle,NULL,&runner,c);
  return 0;
}

#endif
#endif