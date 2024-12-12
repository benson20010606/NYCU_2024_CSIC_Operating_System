# OS Assignment II
 
312512032 陳昱憲 
 
Title description : https://hackmd.io/@fLANt9b6TbWx5I3lYKkBow/rJKfzGY1Jx#Main-Thread

## I. Describe how you implemented the program in detail 
1. 
```c
#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <sched.h>
#include <string.h>  
typedef struct thread_info
{
    pthread_t thread_id;
    int thread_number;
    int sched_policy;
    int sched_priority;
    double time_wait;
} thread_info;
```
2. 
```c
    while ((opt = getopt(argc,argv,"n:t:s:p:"))!=-1){
    	switch (opt){
        	case 'n':
            	num_threads = atoi(optarg);
				//printf("%d\n",num_threads);
        	break;
        	case 't':
				time_wait = atof(optarg);
				//printf("%f\n",time_wait);
        	break;
        	case 's':
        		policy_temp = malloc(sizeof(char)*(strlen(optarg)+1));
				strcpy(policy_temp,optarg);
				//printf("copy policy!!\n");
        	break;
        	case 'p':
        		priority_temp = malloc(sizeof(char)*(strlen(optarg)+1));
				strcpy(priority_temp,optarg);
				//printf("copy priority!!\n");
        	break;
        	default:
        	break;
        }
    }
```

3. 
```c
    /* 2. Create <num_threads> worker threads */
    thread = malloc(sizeof(thread_info)*num_threads);
	int i=0;
	char_temp=strtok(policy_temp,",");
	while(char_temp != NULL){	
		thread[i].thread_number=i;
		thread[i].time_wait=time_wait;
    	if(strcmp(char_temp,"NORMAL") == 0){
    		thread[i].sched_policy=SCHED_OTHER;
			//printf("NORMAL\n");
    	}else if ( strcmp(char_temp,"FIFO") == 0){
    		thread[i].sched_policy = SCHED_FIFO;
		    //printf("FIFO\n");
		}
		char_temp=strtok(NULL,",");
		i++;
	}
	free(policy_temp);
	i=0;
	char_temp=strtok(priority_temp,",");
	while(char_temp != NULL){
		thread[i].sched_priority = atoi(char_temp);
		//printf("%d\n",atoi(char_temp));   
		char_temp = strtok(NULL,",");
		i++;
	}	
    free(priority_temp);
```


4. 
```c
    /* 3. Set CPU affinity */
	CPU_ZERO(&set);
	CPU_SET(0,&set);
	sched_setaffinity(getpid(),sizeof(set),&set);
   
   
   
	pthread_barrier_init(&barrier,NULL,num_threads+1);
```



5. 
```c
	for (int i = 0; i < num_threads; i++) { 

    	/*4. Set the attributes to each thread */
    	
    	pthread_attr_init(&attr);
    	pthread_attr_setinheritsched(&attr,PTHREAD_EXPLICIT_SCHED);
    	
    	if (thread[i].sched_policy == SCHED_FIFO){
    		
    		
    		pthread_attr_setschedpolicy(&attr,SCHED_FIFO);
    		
    		param.sched_priority=thread[i].sched_priority;
    		pthread_attr_setschedparam(&attr,&param);
    		
    		pthread_create(&thread[i].thread_id,&attr,thread_func,(void*)(thread+i));
    	}    	
    	else if (thread[i].sched_policy == SCHED_OTHER){
    	
    		pthread_attr_setschedpolicy(&attr,SCHED_OTHER);
    		pthread_create(&thread[i].thread_id,&attr,thread_func,(void*)(thread+i));
    	}
    	pthread_attr_destroy(&attr);
	}
        /* 5. Start all threads at once */
	pthread_barrier_wait(&barrier);
    /* 6. Wait for all threads to finish  */ 
    for (int i = 0; i < num_threads; i++) { 
    	pthread_join(thread[i].thread_id,NULL);
    }
    pthread_barrier_destroy(&barrier);
    free(thread);
    return 0;
```

6. 
```c
void* thread_func(void* arg){
	pthread_barrier_wait(&barrier);
	thread_info * thread = (thread_info *)arg;
	//printf("which cpu= %d \n",sched_getcpu());
	for (int i =0 ; i<3 ; i++){
		struct timespec start,end ;
		clock_gettime(CLOCK_THREAD_CPUTIME_ID,&start);
		clock_gettime(CLOCK_THREAD_CPUTIME_ID,&end);
		printf("Thread %d is starting\n", thread->thread_number);
		
		while ((end.tv_sec- start.tv_sec) + (end.tv_nsec-start.tv_nsec)*1e-9 <= thread->time_wait)
		{	 		//busy-wait
			clock_gettime(CLOCK_THREAD_CPUTIME_ID,&end);
		 			
		}
	}
	pthread_exit(NULL);
}

```
## II. Describe the results of sudo ./sched_demo -n 3 -t 1.0 -s


## III. Describe the results of sudo ./sched_demo -n 4 -t 0.5 -s NORMAL,FIFO,NORMAL,FIFO -p -1,10,-1,30, and what causes that.

## IV.Describe how did you implement n-second-busy-waiting?

## V.What does the kernel.sched_rt_runtime_us effect? If this setting is changed, what will happen?