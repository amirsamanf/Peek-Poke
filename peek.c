#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/slab.h>

#define MAX_PEEK 8192

static char memloc[8];
char Overflow[MAX_PEEK];
char Buffer[8];
char *pBuffer = &Buffer[0];
int counter = 0;

static struct device* device_data_peek;
static struct class* class_stuff_peek;


//for the peek write, you need to get 8 bytes from the user and store it in the kernel space (here) somehow
//allocate memory in this kernel space and store the 8 bytes here
//if the given packet is not 8 bytes you will need to wait for the remaining bytes before continuing
//if there are more than 8 bytes, then you read 8 first, do whatever, then read the others later
static ssize_t fwrite(struct file *file, const char *data, size_t length, loff_t *offset_in_file)
{

	//copy everything to overflow
	int id = copy_from_user(Overflow, data, length);
	if(id != 0)
	{
		printk(KERN_INFO "something went wrong with copy from user peek_write, id is: %d", id);
	}

	//point to first elelment of Overflow buffer
	char *pOverflow = &Overflow[0];

	//read each byte one at a time
	//when 8 bytes are read, save them to a location in the kernel called memloc
	//and increment relevant pointers
	int k = 0;
	for (k = 0; k < length; k++)
	{
		memcpy((void*)pBuffer, (void*)pOverflow, 1);
		pOverflow++;
		pBuffer++;
		counter++;

		if (counter == 8)
		{
			memcpy(memloc, Buffer, 8);

			counter = 0; //reset counter

			pBuffer = &Buffer[0]; //get buffer ready for next write
		}
	}

	
	printk(KERN_INFO "%d memory location is: %lx", id, *((unsigned long*)memloc));


	return length;
}

	/*
										**f_read**

		doing *((unsigned long*)memloc) will first cast memloc to a long pointer,
		then dereference that pointer to get the address
		the reason to do this is because memloc is a char ARRAY, when you cast it
		to a long pointer, the pointer will point to the ENTIRE address in the array
		then you dereference that pointer to get the value of the address
	*/
static ssize_t f_read(struct file *file, char *data, size_t length, loff_t *offset_in_file)
{
	int id;

	printk(KERN_INFO "peeking memory location memloc: %lx\n", *((unsigned long*)memloc));
	
	id = copy_to_user(data, *((unsigned long*)memloc), 1); //read one byte

	if(id != 0)
	{
		printk(KERN_INFO "something went wrong with copy to user peek_read, id is: %d", id);
	}

	printk(KERN_INFO "data at location %lx is: %c", *((unsigned long*)memloc), data);


	return length;
}



static int open_file_peek(struct inode* inode_pointer, struct file* file_pointer) 
{
	printk(KERN_INFO "Opened peek!");
	return 0;
}

static struct file_operations file_ops_peek =
{
	.open = open_file_peek, //use this to open the file
	.write = fwrite,
	.read = f_read,
};


static int __init hi_peek(void) 
{
   	int major_peek = register_chrdev(0, "peek", &file_ops_peek);   
   	class_stuff_peek = class_create(THIS_MODULE, "peek_class");
   	device_data_peek = device_create(class_stuff_peek, NULL, MKDEV(major_peek, 0), NULL, "peek");
  
  printk(KERN_INFO "HI peek!\n");
  return 0;
}

static void __exit bye_peek(void) 
{
  printk(KERN_INFO "BYE peek!\n");
}

module_init(hi_peek);
module_exit(bye_peek);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("FT and AF");
MODULE_DESCRIPTION("The peek driver");