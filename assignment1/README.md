# OS Assignment I 
 
312512032 陳昱憲 
 
Title description : https://hackmd.io/@-ZZtFnnqSZ2F1A-Uy-GMlw/HJJj4BcKC#Appendix  
## I.  Compiling the Linux Kernel &&  Change kernel local version

1. Install the necessary packages before building the kerne
```
sudo apt-get install git build-essential ncurses-dev libssl-dev bc flex libelf-dev bison

```

2. Modify the `.config` file, then set the kernel local version suffix as required
```
make menuconfig
vim .config
```
![image](picture/suffix.jpg)

3. Build the kernel and install the modul

```
make
sudo make modules_install 
```

4. Verify the kernel version to be installed

```
make kernelrelease
```

5. Install the kernel.

```
sudo make install
```
6. Update the GRUB bootloader then reboot
```
sudo update-grub
sudo reboot
```
7. Verify the current kernel version.

```
uname -a
cat /etc/os-release
```


### reference:  
<https://phoenixnap.com/kb/build-linux-kernel>

## II. Implementing a new System Calls

1. Create a `revstr` directory in Linux and write the system call content in `revstr.c`  
This function has two parameters, `msg` and `len`, so `SYSCALL_DEFINE2` is used.Since data cannot be directly shared between user space and kernel space, `copy_from_user()` and `copy_to_user()` are used for data transfer, and dynamic memory allocation is used to store the data.
```c
#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
// syscall_num =451
SYSCALL_DEFINE2(revstr,char __user * ,msg,unsigned int ,len){
    //printk(KERN_INFO"%d",len);
    char* kernel_msg =  kmalloc((len+1)*sizeof(char),GFP_KERNEL);
    if(! kernel_msg){
    	printk(KERN_ERR "memory allocation failed \n");
	return -ENOMEM ;
    }
    if (copy_from_user(kernel_msg,msg,len)){
    	printk(KERN_ERR "can not copy form user ,please check your address!\n");
    	kfree(kernel_msg);
	return -EFAULT;
    }
/**********revstr function**********/
    kernel_msg[len]='\0';
    printk(KERN_INFO "The origin string: %s\n",kernel_msg);
    for(unsigned int i=0,j=len-1 ;i<j;i++,j--){
    	char temp = kernel_msg[i];
	kernel_msg[i]=kernel_msg[j];
	kernel_msg[j]=temp;
    }
    kernel_msg[len]='\0';
    printk(KERN_INFO "The reversed string: %s\n",kernel_msg);
/**********************************/
    if ( copy_to_user(msg,kernel_msg,len)){
   	printk(KERN_ERR "can not copy to user!!\n");
	return -EFAULT;
    }
    kfree(kernel_msg);
    return 0;
}
```
2. Write a Makefile in `~/linux/revstr` to compile `revstr.c` and link it into the kernel image.

```
obj-y :=revstr.o
```
3. Add the written system call to `~/linux/arch/x86/entry/syscalls/syscall_64.tbl`, placing it at the end of the 64-bit section. The system call number is` 451`, the name is` revstr`, and the entry point is `sys_revstr`.

4. Declare the prototype of `sys_revst`r in` ~/linux/include/linux/syscalls.h` so that the Linux Kernel compiler can recognize this new system call and invoke it where needed.


5. Add the relative path of the` ~/linux/revstr `directory to` core-y` in `~/linux/Makefile` so that it is included during the compilation process

6. Reflash and reboot
```
make -j8 && sudo make modules_install && sudo make install && sudo reboot
```
7. Write a test file;` __NR_revstr` is the system call number, which allows the `syscall()` function to execute the system call corresponding to that number in the system call table.
```c
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

#define __NR_revstr 451

int main(int argc ,char* argv[]){

    char str1[20] ="hello";
    printf("Ori: %s\n",str1);
    int ret1 = syscall(__NR_revstr,str1,strlen(str1));
    assert(ret1 == 0);
    printf("Rev: %s\n",str1);

    char str2[20] = "Operating System";
    printf("Ori: %s\n",str2);
    int ret2=syscall(__NR_revstr,str2,strlen(str2));
    assert(ret2 == 0);
    printf("Rev: %s\n",str2);

    return 0;
}

```


8. Execute the test file and check the results in user space  and the kernel space results using `dmesg`
```
./test_revstr

sudo dmesg
```