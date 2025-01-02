// driver
#include <linux/init.h>
#include <linux/module.h>
#include <linux/printk.h> 
#include <linux/uaccess.h>
#include <linux/cdev.h>
#include <linux/fs.h>

#include "kfetch.h" 

#include <linux/utsname.h>
#include <linux/string.h>
#include <linux/cpu.h>

#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/sysinfo.h>
#include <linux/mm.h>
#include <linux/ktime.h>
#include <linux/time.h>

#include <linux/string.h>

#include <linux/mutex.h>



static void __exit kfetch_driver_exit(void);
static int __init kfetch_driver_init(void);

/***************Driver functions****************/
static int kfetch_open(struct inode *inode , struct file *file);
static int kfetch_release(struct inode *inode , struct file *file);
static ssize_t kfetch_read(struct file *filp,
                           char __user *buffer,
                           size_t length,
                           loff_t *offset);
static ssize_t kfetch_write(struct file *filp,
                            const char __user *buffer,
                            size_t length,
                            loff_t *offset);                            
/**********************************************/

static char *Tux0 = "                   ";
static char *Tux1 = "        .-.        ";
static char *Tux2 = "       (.. |       ";
static char *Tux3 = "       <>  |       ";
static char *Tux4 = "      / --- \\      ";  
static char *Tux5 = "     ( |   | )     ";
static char *Tux6 = "   |\\\\_)__(_//|    ";
static char *Tux7 = "  <__)------(__>   ";


/*
static char *Tux0 = "                   ";
static char *Tux1 = "        .-.        ";
static char *Tux2 = "       (.. |       ";
static char *Tux3 = "       \033[33m<>\033[0m  |       ";
static char *Tux4 = "      / --- \\      ";  
static char *Tux5 = "     ( |   | )     ";
static char *Tux6 = "   \033[33m|\\\033[0m\\_)__(_/\033[33m/|\033[0m    ";
static char *Tux7 = "  \033[33m<__)\033[0m------\033[33m(__>\033[0m   ";

*/





static char kfetch_buf[KFETCH_BUF_SIZE]="\0";
int mask;


dev_t dev=0;
static struct class *dev_class;
static struct cdev kfetch_cdev;

DEFINE_MUTEX(mutex_kfetch);

const static struct file_operations kfetch_ops = {
    .owner   = THIS_MODULE,
    .read    = kfetch_read,
    .write   = kfetch_write,
    .open    = kfetch_open,
    .release = kfetch_release,
};


static const char * get_hostname(void)
{	
	//pr_info("hostname:%s\n",init_uts_ns.name.nodename);
	return init_uts_ns.name.nodename ;
}


static const char * get_dash(uint8_t len)
{	
	static char dash[100]="\0";
	memset(dash,0,100);
	for(uint8_t i=0;i<len;i++){
		strlcat(dash,"-",sizeof(dash));
	}
	
	//pr_info("how long :%d\n" ,len);
	
	
	return dash ;
}


static const char * get_kernel(void){
	static char kernel[100]="\0";
	memset(kernel,0,100);
	
	strlcat(kernel,"kernel:   ",sizeof(kernel));
	strlcat(kernel,init_uts_ns.name.release,sizeof(kernel));
	
	//pr_info("%s\n",kernel);
	return kernel;
}


static const char * get_cupname(void){
	static char cpu[100]="\0";
	memset(cpu,0,100);

	struct cpuinfo_x86 *c =&cpu_data(0);
	strlcat(cpu,"CPU:      ",sizeof(cpu));
	strlcat(cpu,c->x86_model_id,sizeof(cpu));
	
	//pr_info("%s\n",cpu);
	return cpu;

}

static const char * get_cups(void){
	static char cpus[100]="\0";
	
	uint8_t online_cpus=num_online_cpus();
	uint8_t total_cpus=num_possible_cpus();
	snprintf(cpus,sizeof(cpus),"CPUs:     %d / %d",online_cpus,total_cpus);
	//pr_info("%s",cpus);
	return cpus;

}

static const char * get_meminfo(void){
	static char mem[100]="\0";
	
	struct sysinfo si;
	si_meminfo(&si);
	unsigned long free =si.freeram /(1024*1024/si.mem_unit);
	unsigned long total =si.totalram /(1024*1024/si.mem_unit);

	
	snprintf(mem,sizeof(mem),"Mem:      %lu MB / %lu MB",free,total);
	//pr_info("%s",mem);
	return mem;

}

static const char * get_procsinfo(void){
	int count=0;
	static char procs[100]="\0";
	struct task_struct *task;
	for_each_process(task){
	count++;
	}

	snprintf(procs,sizeof(procs),"Procs:    %d",count);
	//pr_info("%s",procs);
	return procs;
}

static const char * get_uptimeinfo(void){
	static char uptimeinfo[100]="\0";
	struct timespec64 uptime;
	ktime_get_boottime_ts64(&uptime);

	snprintf(uptimeinfo,sizeof(uptimeinfo),"Uptime:   %lu mins",(unsigned long )uptime.tv_sec/60);
	//pr_info("%s",uptimeinfo);
	return uptimeinfo;
	
	/*struct sysinfo si;
	sysinfo(&si);
	snprintf(uptime,sizeof(uptime),"Uptime:\t%lu\n",si.uptime/60);
	pr_info("%s",uptime);
	return uptime;*/
}



static int kfetch_open(struct inode *inode , struct file *file)
{	
	mutex_lock(&mutex_kfetch);
	pr_info("Mutex lock and Device File Opened...!!!\n");
	
	return 0;
}

static int kfetch_release(struct inode *inode , struct file *file)
{	
	mutex_unlock(&mutex_kfetch);
	pr_info("Mutex unlock and Device File Closed...!!!\n");
	return 0;
}

static ssize_t kfetch_read(struct file *filp,
                           char __user *buffer,
                           size_t length,
                           loff_t *offset)
{
    /* fetching the information   */
	if(mask==-1){
		mask=63;
	}


	if(mask == 0)
	{
	
	}else {
		memset(kfetch_buf,0,KFETCH_BUF_SIZE);
		strlcat(kfetch_buf,Tux0,KFETCH_BUF_SIZE);
		strlcat(kfetch_buf,get_hostname(),KFETCH_BUF_SIZE);
		strlcat(kfetch_buf,"\n",KFETCH_BUF_SIZE);
		strlcat(kfetch_buf,Tux1,KFETCH_BUF_SIZE);
		strlcat(kfetch_buf,get_dash(strlen(get_hostname())),KFETCH_BUF_SIZE);
		strlcat(kfetch_buf,"\n",KFETCH_BUF_SIZE);
		
		for(uint8_t i=0;i< KFETCH_NUM_INFO ; i++ ){
			switch (i){
				case 0 :
					strlcat(kfetch_buf,Tux2,KFETCH_BUF_SIZE);
					break;
				case 1 :
					strlcat(kfetch_buf,Tux3,KFETCH_BUF_SIZE);
					break;
				case 2 :
					strlcat(kfetch_buf,Tux4,KFETCH_BUF_SIZE);
					break;
				case 3 :
					strlcat(kfetch_buf,Tux5,KFETCH_BUF_SIZE);
					break;
				case 4 :
					strlcat(kfetch_buf,Tux6,KFETCH_BUF_SIZE);
					break;
				case 5 :
					strlcat(kfetch_buf,Tux7,KFETCH_BUF_SIZE);
					break;
			}



			
			if(mask & KFETCH_RELEASE){
				//strlcat(kfetch_buf,"Kernel:\t",KFETCH_BUF_SIZE);
				strlcat(kfetch_buf,get_kernel(),KFETCH_BUF_SIZE);
				strlcat(kfetch_buf,"\n",KFETCH_BUF_SIZE);
				mask &= ~(KFETCH_RELEASE);
				continue;
			}else if(mask & KFETCH_CPU_MODEL){
				//strlcat(kfetch_buf,"CPU:\t",KFETCH_BUF_SIZE);
				strlcat(kfetch_buf,get_cupname(),KFETCH_BUF_SIZE);
				strlcat(kfetch_buf,"\n",KFETCH_BUF_SIZE);
				mask &= ~(KFETCH_CPU_MODEL);
				continue;
			}else if(mask & KFETCH_NUM_CPUS){
				//strlcat(kfetch_buf,"CPUs:\t",KFETCH_BUF_SIZE);
				strlcat(kfetch_buf,get_cups(),KFETCH_BUF_SIZE);
				strlcat(kfetch_buf,"\n",KFETCH_BUF_SIZE);
				mask &= ~(KFETCH_NUM_CPUS);
				continue;
			}else if(mask & KFETCH_MEM ){
				//strlcat(kfetch_buf,"Mem:\t",KFETCH_BUF_SIZE);
				strlcat(kfetch_buf,get_meminfo(),KFETCH_BUF_SIZE);
				strlcat(kfetch_buf,"\n",KFETCH_BUF_SIZE);
				mask &= ~(KFETCH_MEM );
				continue;
			}else if(mask & KFETCH_NUM_PROCS){
				//strlcat(kfetch_buf,"Procs:\t",KFETCH_BUF_SIZE);
				strlcat(kfetch_buf,get_procsinfo(),KFETCH_BUF_SIZE);
				strlcat(kfetch_buf,"\n",KFETCH_BUF_SIZE);
				mask &= ~(KFETCH_NUM_PROCS);
				continue;
			}else if(mask & KFETCH_UPTIME){
				//strlcat(kfetch_buf,"Uptime:\t",KFETCH_BUF_SIZE);
				strlcat(kfetch_buf,get_uptimeinfo(),KFETCH_BUF_SIZE);
				strlcat(kfetch_buf,"\n",KFETCH_BUF_SIZE);
				mask &= ~(KFETCH_UPTIME);
				continue;
			}
			strlcat(kfetch_buf,"\n",KFETCH_BUF_SIZE);
		
		}
		
	}




	pr_info("%s",kfetch_buf);
    if (copy_to_user(buffer, kfetch_buf, strlen(kfetch_buf)) ){
        pr_alert("Failed to copy data to user");
        return 0;
    }
  	mask=0;
     /*cleaning up */
    return strlen(kfetch_buf);
}


static ssize_t kfetch_write(struct file *filp,
                            const char __user *buffer,
                            size_t length,
                            loff_t *offset)
{
   
	int mask_info;
    if (copy_from_user(&mask_info, buffer, length)) {
        pr_alert("Failed to copy data from user");
        return 0;
    }
	mask=mask_info;
    //pr_info("Masked:%d!!!\n",mask);
	//pr_info("%s",)




    /*  setting the information mask */
   return 0;
}





static int __init kfetch_driver_init(void)
{
	if((alloc_chrdev_region(&dev,0,1,KFETCH_DEV_NAME))<0){
	pr_err("Cannot allocate major number\n");
	}
	pr_info("Major = %d Minor = %d \n",MAJOR(dev),MINOR(dev));
	cdev_init(&kfetch_cdev,&kfetch_ops);
	
	
	cdev_add(&kfetch_cdev,dev,1);
	dev_class=class_create(THIS_MODULE,"kfetch_class");
	device_create(dev_class,NULL,dev,NULL,KFETCH_DEV_NAME);
	
	mask=-1;
	pr_info("Device Driver Insert...Done!!!\n");
	
	

	return 0;

}


static void __exit kfetch_driver_exit(void)
{
	device_destroy(dev_class,dev);
	class_destroy(dev_class);
	unregister_chrdev_region(dev,1);
	pr_info("Device Driver Remove...Done!!\n");
	

}

module_init(kfetch_driver_init);
module_exit(kfetch_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("benson.ee12@nucu.edu.tw");
MODULE_DESCRIPTION("OS HW3 ");



