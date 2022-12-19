//#include <stdio.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/types.h>
#include <linux/cdev.h>
#include <linux/kdev_t.h>
#include <linux/uaccess.h>
#include <linux/errno.h>
#include <linux/device.h>
#include <linux/wait.h>
#include <linux/semaphore.h>
#define BAZA_SIZE 10
#define BUFF_SIZE 100
MODULE_LICENSE("Dual BSD/GPL");

dev_t my_dev_id;
static struct class *my_class;
static struct device *my_device;
static struct cdev *my_cdev;
/*
DECLARE_WAIT_QUEUE_HEAD(readQ);
DECLARE_WAIT_QUEUE_HEAD(writeQ);
struct semaphore sem;
*/

char *ime[BAZA_SIZE];
char *prezime[BAZA_SIZE];
char *brIndexa[BAZA_SIZE];
int brBodova[BAZA_SIZE];

int pos = 0;
int endRead = 0;

int baza_open(struct inode *pinode, struct file *pfile);
int baza_close(struct inode *pinode, struct file *pfile);
ssize_t baza_read(struct file *pfile, char __user *buffer, size_t length, loff_t *offset);
ssize_t baza_write(struct file *pfile, const char __user *buffer, size_t length, loff_t *offset);

struct file_operations my_fops =
{
	.owner = THIS_MODULE,
	.open = baza_open,
	.read = baza_read,
	.write = baza_write,
	.release = baza_close,
};


int baza_open(struct inode *pinode, struct file *pfile) 
{
		printk(KERN_INFO "Succesfully opened baza\n");
		return 0;
}

int baza_close(struct inode *pinode, struct file *pfile) 
{
		printk(KERN_INFO "Succesfully closed baza\n");
		return 0;
}

ssize_t baza_read(struct file *pfile, char __user *buffer, size_t length, loff_t *offset) 
{
	int ret,i;
	char buff[BUFF_SIZE];
	long int len = 0;
	if (endRead){
		endRead = 0;
		return 0;
	}



	if(pos > 0)
	{
		
		for (i=0; i<pos; i++)
		{		
			len = scnprintf(buff, BUFF_SIZE, "%s %s %s - %d \n", brIndexa[i], ime[i], prezime[i], brBodova[i]);
			ret = copy_to_user(buffer, buff, len);
			if(ret)
				return -EFAULT;
			//puts(buffer);
			printk(KERN_INFO "%s \n", buffer);
		}
		printk(KERN_INFO "Succesfully read\n");
		endRead = 1;
	}
	else
	{
			printk(KERN_WARNING "Baza je prazna\n"); 
	}


	return len;
}

ssize_t baza_write(struct file *pfile, const char __user *buffer, size_t length, loff_t *offset) 
{
	char buff[BUFF_SIZE];
	char tmp_ime[100];
	char tmp_prezime[100];
	char tmp_brIndexa[100];
	int tmp_brBodova;
	char rec[20];
	int br;
	int i=0;
	int j=0;


	int ret;

	ret = copy_from_user(buff, buffer, length);
	if(ret)
		return -EFAULT;
	buff[length-1] = '\0';



	if (buff[0]=='i' && buff[1]=='z' && buff[2]=='b' && buff[3]=='r' && buff[4]=='i' && buff[5]=='s' && buff[6]=='i')
	{
		if(pos>0)
		{
			ret = sscanf(buff,"%20[^=]=%100[^,],%100[^,],%s", rec, tmp_ime, tmp_prezime, tmp_brIndexa);
			if(ret==4)//four parameters parsed in sscanf
			{
				printk(KERN_INFO "Brisanje: "); 
				printk(KERN_INFO "ime %s ", tmp_ime); 
				printk(KERN_INFO "prezime %s ", tmp_prezime); 
				printk(KERN_INFO "broj indexa %s ", tmp_brIndexa); 
				
				br=0;	
				for (i=0; i<pos; i++)
				{
					if(strcmp(ime[i],tmp_ime)==0 &&  strcmp(prezime[i],tmp_prezime)==0 && strcmp(brIndexa[i],tmp_brIndexa)==0 )
					{
						br++;
						for(j=i; j<pos-1; j++)
						{
							ime[j] = ime[j+1];
							prezime[j] = prezime[j+1];
							brIndexa[j] = brIndexa[j+1];
							brBodova[j] = brBodova[j+1];
						}
						ime[pos-1]='\0';
						prezime[pos-1]='\0';
						brIndexa[pos-1] = '\0';
						brBodova[pos-1] = 0;
						pos=pos-1;
					    	printk(KERN_INFO "Uspesno izbrisan zeljeni student\n");
				
						break; 
					}
				}
				if(br==0)
					printk(KERN_WARNING "Nema takvog studenta u bazi\n");
			}
			else
			{
				printk(KERN_WARNING "Komanda mora biti izbrisi=ime,prezime,brIndexa\n");
			}
		}
		else
		{	
			printk(KERN_WARNING "Baza je prazna, ne mozes izbrisati podatak\n");
		}
	}
	else
	{
		if(pos<BUFF_SIZE)
		{
			ret = sscanf(buff,"%100[^,],%100[^,],%100[^=]=%d", tmp_ime, tmp_prezime, tmp_brIndexa, &tmp_brBodova);
			if(ret==4)//four parameters parsed in sscanf
			{	
				br=0;
				for (i=0; i<pos; i++)
				{
					if(strcmp(ime[i],tmp_ime)==0 &&  strcmp(prezime[i],tmp_prezime)==0 && strcmp(brIndexa[i],tmp_brIndexa)==0 )
					{
						br++;
						if(brBodova[i]==tmp_brBodova)
				    			printk(KERN_INFO "Takav student vec postoji u bazi\n");
						else
						{
							brBodova[i]=tmp_brBodova;					
				    			printk(KERN_INFO "Izmena broja bodova studentu\n");
						}
						break;
					}
				}
				if(br==0)
				{	
					printk(KERN_INFO "UPIS:"); 
					ime[pos] = tmp_ime; 
					prezime[pos] = tmp_prezime; 
					brIndexa[pos] = tmp_brIndexa; 
					brBodova[pos] = tmp_brBodova; 
					printk(KERN_INFO "ime: %s ", ime[pos]); 
					printk(KERN_INFO "prezime: %s ", prezime[pos]); 
					printk(KERN_INFO "broj indexa: %s ", brIndexa[pos]); 
					printk(KERN_INFO "broj bodova: %d ", brBodova[pos]); 
					printk(KERN_INFO "Pozicija na koju se upisuje: %d ", pos); 
					pos=pos+1;
				}
			}
			else
			{
				printk(KERN_WARNING "Komanda mora biti ime_prezime_brIndexa=brBodova\n");
			}
		}
		else
		{	
			printk(KERN_WARNING "Baza je puna, podatak se ne moze upisati\n");
		}
	}

	return length;
}

static int __init baza_init(void)
{
   int ret = 0;
	int i=0;
	
//	sema_init(&sem,1);

	//Initialize array
	for (i=0; i<BAZA_SIZE; i++)
	{
		ime[i] = '\0';
		prezime[i] = '\0';
		brIndexa[i] = '\0';
		brBodova[i] = 0;
	}
   ret = alloc_chrdev_region(&my_dev_id, 0, 1, "baza");
   if (ret){
      printk(KERN_ERR "failed to register char device\n");
      return ret;
   }
   printk(KERN_INFO "char device region allocated\n");

   my_class = class_create(THIS_MODULE, "baza_class");
   if (my_class == NULL){
      printk(KERN_ERR "failed to create class\n");
      goto fail_0;
   }
   printk(KERN_INFO "class created\n");
   
   my_device = device_create(my_class, NULL, my_dev_id, NULL, "baza");
   if (my_device == NULL){
      printk(KERN_ERR "failed to create device\n");
      goto fail_1;
   }
   printk(KERN_INFO "device created\n");

	my_cdev = cdev_alloc();	
	my_cdev->ops = &my_fops;
	my_cdev->owner = THIS_MODULE;
	ret = cdev_add(my_cdev, my_dev_id, 1);
	if (ret)
	{
      printk(KERN_ERR "failed to add cdev\n");
		goto fail_2;
	}
   printk(KERN_INFO "cdev added\n");
   printk(KERN_INFO "Hello world\n");

   return 0;

   fail_2:
      device_destroy(my_class, my_dev_id);
   fail_1:
      class_destroy(my_class);
   fail_0:
      unregister_chrdev_region(my_dev_id, 1);
   return -1;
}

static void __exit baza_exit(void)
{
   cdev_del(my_cdev);
   device_destroy(my_class, my_dev_id);
   class_destroy(my_class);
   unregister_chrdev_region(my_dev_id,1);
   printk(KERN_INFO "Goodbye, cruel world\n");
}


module_init(baza_init);
module_exit(baza_exit);
