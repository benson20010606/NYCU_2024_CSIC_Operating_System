# OS Assignment II
 
312512032 陳昱憲 
 
Title description : https://hackmd.io/@fLANt9b6TbWx5I3lYKkBow/rJKfzGY1Jx#Main-Thread

## I. Describe how you implemented the program in detail 
1. Define a struct named `thread_info` to store information for each thread  
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
2. Using `getopt()` to read command-line arguments  
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

3. Create worker threads and pass corresponding arguments (`wait time`, `scheduling policy`, and `priority`) 
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


4.  Bind the process to the CPU with CPU ID 0 to force all threads to compete for the same CPU resource. Then, initialize the number of threads plus one barrier, because in addition to blocking the number of threads specified by `num_thread`, a signal is needed to synchronize their execution, so one more is added. 
```c
    /* 3. Set CPU affinity */
	CPU_ZERO(&set);
	CPU_SET(0,&set);
	sched_setaffinity(getpid(),sizeof(set),&set);
   
   
   
	pthread_barrier_init(&barrier,NULL,num_threads+1);
```



5. Set each thread's attributes based on the conditions. If `SCHED_FIFO` is used, set its priority and policy; if `SCHED_OTHER`, only set its policy:

- `pthread_attr_init`: Initialize thread attributes.
- `pthread_attr_setinheritsched`: Specify whether the thread inherits the main process's attributes.
- `pthread_attr_setschedpolicy`: Set the scheduling policy.
- `param.sched_priority`: If `SCHED_FIFO`, set the priority.
- `pthread_attr_setschedparam`: Set the thread's attributes (priority and policy).
- `pthread_create`: Create the thread using the configured attributes.

    After setting up all the threads, use `pthread_barrier_wait()` to make the barrier reach the specified count, allowing all threads to start simultaneously. They will compete for CPU resources based on their scheduling policies and priorities. Finally, release resources when all threads have completed their tasks.
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

6. Each thread is initially blocked by the barrier until all threads are ready. Afterward, each thread displays its identifier based on the provided information and performs busy-waiting for a specified number of seconds, repeating this process three times.
The busy-waiting implementation records the thread's CPU start time and continuously checks whether the current CPU time occupied by the thread has reached the specified duration.
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
### **Case 1 (when ``sched_rt_runtime_us`` == ``sched_rt_period_us``):**  
For **`SCHED_OTHER` (`SCHED_NORMAL`)**, its default priority is 0, so **Thread 0** can be treated as having a priority of 0. For *`SCHED_FIFO`*, the execution order is determined based on the priority (with static priorities higher than 0 having higher precedence). Therefore, **Thread 2** (with priority 30) will execute first, followed by **Thread 1** (with priority 10), and finally **Thread 0**(with priority 0).
Since **`sched_rt_runtime_us`** is equal to **`sched_rt_period_us`** , **`SCHED_OTHER`** cannot take control of the CPU before **`SCHED_FIFO`** finishes its execution.

![alt text](image\image.png)

### **Case 2 (when `sched_rt_runtime_us < sched_rt_period_us`):**

In this case, `sched_rt_runtime_us = 950000` (0.95 seconds) and `sched_rt_period_us = 1000000` (1 second). This means that within one period (1 second), 0.95 seconds are allocated to real-time tasks (`SCHED_FIFO`), and the remaining 0.05 seconds are allocated to normal tasks (`SCHED_OTHER`). Therefore, unlike **Case 1**, the CPU will not be completely occupied by the real-time tasks.
First, **Thread 2** (with priority 30) will acquire the CPU and run for 0.95 seconds. After that, the remaining 0.05 seconds are allocated by the CFS (Completely Fair Scheduler) to the `SCHED_OTHER` task (there is only one such task, so it gets the full 0.05 seconds). Then, the next 0.95 seconds are given to real-time tasks again, followed by 0.05 seconds for the `SCHED_OTHER` task. This cycle repeats until all tasks are complete.
![alt text](image\image-1.png)


## III. Describe the results of sudo ./sched_demo -n 4 -t 0.5 -s NORMAL,FIFO,NORMAL,FIFO -p -1,10,-1,30, and what causes that.


### **Case 1 (when `sched_rt_runtime_us == sched_rt_period_us`):**

Since `sched_rt_runtime_us` is equal to `sched_rt_period_us`, real-time tasks (`SCHED_FIFO`) will completely occupy the CPU until they are finished. In this scenario, `SCHED_OTHER`  tasks cannot access the CPU before all real-time tasks are completed.

First, the highest priority thread, **Thread 3**, will acquire the CPU and execute. After that, the next highest priority thread, **Thread 1**, will take over the CPU. Once all the real-time tasks (`SCHED_FIFO`) are completed, the CPU resources will be fairly distributed to the normal tasks (`SCHED_OTHER`)

![alt text](image\image-3.png)



### **Case 2 (when `sched_rt_runtime_us < sched_rt_period_us`):**

In this case, `sched_rt_runtime_us = 950000` (0.95 seconds) and `sched_rt_period_us = 1000000` (1 second). This means that within one period (1 second), 0.95 seconds are allocated to real-time tasks (`SCHED_FIFO`), and the remaining 0.05 seconds are allocated to normal tasks (`SCHED_OTHER`). Therefore, the execution will not be completely dominated by the real-time tasks as in **Case 1**.

First, **Thread 3** (with priority 30) will acquire the CPU and run for 0.95 seconds. After that, the remaining 0.05 seconds will be allocated by the CFS (Completely Fair Scheduler) to normal tasks (`SCHED_OTHER`). Since there are two normal tasks (Thread 0 and Thread 2), this 0.05 seconds will be fairly split between them. 

Then, the system will allocate the next 0.95 seconds to the real-time tasks (`SCHED_FIFO`), followed by another 0.05 seconds for the `SCHED_OTHER` tasks. This cycle repeats until all tasks are completed.

![alt text](image\image-4.png)

## IV. Describe how did you implement n-second-busy-waiting?
### Using `clock_gettime()` for Busy-Waiting

We use `clock_gettime()` to record the starting time (`start`), where `CLOCK_CPUTIME_ID` refers to the time the thread spends executing on the CPU. Then, a `while` loop is employed to implement **busy-waiting**. The thread will continuously check the elapsed time (using `clock_gettime()`) until the waiting time reaches the specified length (e.g., `n` seconds).

- **While loop**: The thread keeps reading the current CPU time (`end`) until the difference between `end` and `start` meets the required waiting time. This ensures that the thread actively occupies the CPU during the waiting period .

### Why `sleep()` Doesn’t Work for Busy-Waiting

The reason `sleep()` is not suitable for busy-waiting is that it releases the current thread’s CPU resources temporarily. When `sleep()` is invoked, the CPU time is given up, allowing other higher-priority threads to preempt the current thread. As a result, the thread may not accurately control the waiting time and cannot meet the busy-waiting requirement. Therefore, **busy-waiting** must involve active CPU usage, which is not achievable with `sleep()`.

```c
		struct timespec start,end ;
		clock_gettime(CLOCK_THREAD_CPUTIME_ID,&start);
		clock_gettime(CLOCK_THREAD_CPUTIME_ID,&end);
		printf("Thread %d is starting\n", thread->thread_number);
		
		while ((end.tv_sec- start.tv_sec) + (end.tv_nsec-start.tv_nsec)*1e-9 <= thread->time_wait)
		{	 		//busy-wait
			clock_gettime(CLOCK_THREAD_CPUTIME_ID,&end);
		 			
		}
```
## V. What does the kernel.sched_rt_runtime_us effect? If this setting is changed, what will happen?

### Adjusting `kernel.sched_rt_runtime_us` for CPU Resource Allocation

We can control the ratio of CPU time allocated to Real-time tasks (`SCHED_FIFO`) and Normal tasks (`SCHED_OTHER`) within a cycle (defined by `kernel.sched_rt_period_us`) by adjusting the value of `kernel.sched_rt_runtime_us`. 

- When `kernel.sched_rt_runtime_us` equals `kernel.sched_rt_period_us`, all CPU resources are initially allocated to real-time tasks (`SCHED_FIFO`). In this case, normal tasks (`SCHED_OTHER`) will only receive CPU time after all real-time tasks have completed, according to the CFS (Completely Fair Scheduler) allocation.

- When `kernel.sched_rt_runtime_us` is not equal to `kernel.sched_rt_period_us`, the CPU resources for a period (`kernel.sched_rt_period_us`) will be divided, with `kernel.sched_rt_runtime_us` allocated to real-time tasks (`SCHED_FIFO`). Even if real-time tasks have not completed, after the specified runtime (e.g., 0.95 seconds), the system will allocate the remaining time (e.g., 0.05 seconds) to normal tasks (`SCHED_OTHER`). Then, it will switch back to real-time tasks, repeating this cycle.

This design prevents real-time tasks (`SCHED_FIFO`) from monopolizing the CPU and ensures that normal tasks (`SCHED_OTHER`) have an opportunity to execute. By adjusting `kernel.sched_rt_runtime_us` according to application requirements, the system can balance between real-time and fair scheduling.
