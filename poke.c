#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/slab.h>

#define MAX_POKE 9216


static char address[8];
static char all[9];
static char d;

char PokeOverflow[MAX_POKE];
char PokeBuffer[9];
char *pPokeBuffer = &PokeBuffer[0];

int PokeCounter = 0;

static struct device* device_data_poke;
static struct class* class_stuff_poke;

static ssize_t filewrite(struct file *file, const char *data, size_t length, loff_t *offset_in_file)
{
	
	int id = copy_from_user(PokeOverflow, data, length);

	if(id != 0)
	{
		printk(KERN_INFO "something went wrong with copy from user poke_write, id is: %d", id);
	}

	//create pointers pointing to the beginning of the buffers
	char *pPokeOverflow = &PokeOverflow[0];

	//variables for casting and writing to memory
	long MemoryLocation;
	char *pMemoryLocation;


	//read each byte one at a time 
	int i = 0;
	for (i = 0; i < length; i++)
	{
		memcpy((void*)pPokeBuffer, (void*)pPokeOverflow, 1);
		pPokeOverflow++;
		pPokeBuffer++;
		PokeCounter++;

		//when 9 bytes are read, extract the data byte, and write it to that address
		if (PokeCounter == 9)
		{
			memcpy(address, PokeBuffer, 8); //extract address

			d = PokeBuffer[8]; //extract data byte
			PokeCounter = 0; //reset counter

			//cast address
			MemoryLocation = *((long*)address);
			pMemoryLocation = MemoryLocation;

			//write to the address
			*pMemoryLocation = d;

			//print data and memory location
			printk(KERN_INFO "writing %c to location: %lx", *pMemoryLocation, MemoryLocation);

			pPokeBuffer = &PokeBuffer[0]; //get buffer ready for next write
		}
	}


	return length;
}


static int open_file(struct inode* inode_pointer, struct file* file_pointer) 
{
	printk(KERN_INFO "Opened poke!");
	return 0;
}

static struct file_operations file_ops =
{
	.open = open_file, //use this to open the file
	.write = filewrite
};


static int __init hi(void) 
{
   	int major_poke = register_chrdev(0, "poke", &file_ops);   
   	class_stuff_poke = class_create(THIS_MODULE, "poke_class");
   	device_data_poke = device_create(class_stuff_poke, NULL, MKDEV(major_poke, 0), NULL, "poke");
  
  printk(KERN_INFO "HI POKE!\n");
  return 0;
}

static void __exit bye(void) 
{
  printk(KERN_INFO "BYE POKE!\n");
}

module_init(hi);
module_exit(bye);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("FT and AF");
MODULE_DESCRIPTION("The poke driver");