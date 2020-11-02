#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/uaccess.h>

#define MAX_SIZE 1024

static char buf_read[MAX_SIZE]; //allocate 1024 bytes 
static struct device* device_data;
static struct class* class_stuff;


/* 
	Gets the address of preallocated memory buf_read and sends it to user
*/
static ssize_t fileread(struct file *file, char *data, size_t length, loff_t *offset_in_file)
{
	if(length != 8)
	{
		return -1;
	}

	long p = buf_read; 

	int t = copy_to_user(data, &p, 8); //copies the allocated address to data
	printk(KERN_INFO "buffread address: %lx", p);


	return length; //return number of bytes copied
}


static int open_file(struct inode* inode_pointer, struct file* file_pointer) 
{
	printk(KERN_INFO "Opened!");
	return 0;
}

static struct file_operations file_ops =
{
	.open = open_file, //use this to open the file
	.read = fileread, //this reads the file
};


static int __init hi(void) 
{
   	int major = register_chrdev(0, "getPointer", &file_ops);   
   	class_stuff = class_create(THIS_MODULE, "getPointer_class");
   	device_data = device_create(class_stuff, NULL, MKDEV(major, 0), NULL,
			       "getptr");
  
  printk(KERN_INFO "HI!\n");
  return 0;
}

static void __exit bye(void) 
{
  printk(KERN_INFO "BYE!\n");
}

module_init(hi);
module_exit(bye);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("RL");
MODULE_DESCRIPTION("A sample driver that just says hello!");


