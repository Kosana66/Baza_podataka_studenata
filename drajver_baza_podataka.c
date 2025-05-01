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
#define BUFF_SIZE 70
MODULE_LICENSE("Dual BSD/GPL");

dev_t my_dev_id;
static struct class *my_class;
static struct device *my_device;
static struct cdev *my_cdev;

DECLARE_WAIT_QUEUE_HEAD(brisanjeQ);
DECLARE_WAIT_QUEUE_HEAD(upisQ);
struct semaphore sem;

char ime[BAZA_SIZE][100];
char prezime[BAZA_SIZE][100];
char brIndexa[BAZA_SIZE][100];
int brBodova[BAZA_SIZE];

int counter=0;
int pos = 0;
int endRead = 0;

int baza_open(struct inode *pinode, struct file *pfile);
int baza_close(struct inode *pinode, struct file *pfile);
ssize_t baza_read(struct file *pfile, char __user *buffer, size_t length, loff_t *offset);
ssize_t baza_write(struct file *pfile, const char __user *buffer, size_t length, loff_t *offset);

struct file_operations my_fops = {
	.owner = THIS_MODULE,
	.open = baza_open,
	.read = baza_read,
	.write = baza_write,
	.release = baza_close,
};


int baza_open(struct inode *pinode, struct file *pfile) {
	printk(KERN_INFO "Succesfully opened baza\n");
	return 0;
}

int baza_close(struct inode *pinode, struct file *pfile) {
	printk(KERN_INFO "Succesfully closed baza\n");
	return 0;
}

ssize_t baza_read(struct file *pfile, char __user *buffer, size_t length, loff_t *offset) {
	int ret,i,j;
	char buff[BUFF_SIZE];
	long int len = 0;
	
	char tmp_ime[100];
	char tmp_prezime[100];
	char tmp_brIndexa[100];
	int tmp_brBodova;

	if (endRead) {
		endRead = 0;
		counter=0;
		return 0;
	}

	if(pos > 0) {
		//zastita deljenih resursa
		if(down_interruptible(&sem))
			return -ERESTARTSYS;

		//sortitranje
		for(i=0; i<pos-1; i++) {
			for(j=i+1; j<pos; j++) {
				if(brBodova[j]<brBodova[i]) {							
					strcpy(tmp_ime,ime[i]);
					strcpy(tmp_prezime,prezime[i]);		
					strcpy(tmp_brIndexa,brIndexa[i]);
					tmp_brBodova=brBodova[i];
									
					strcpy(ime[i],ime[j]);
					strcpy(prezime[i],prezime[j]);		
					strcpy(brIndexa[i],brIndexa[j]);
					brBodova[i]=brBodova[j];
					
					strcpy(ime[j],tmp_ime);
					strcpy(prezime[j],tmp_prezime);		
					strcpy(brIndexa[j],tmp_brIndexa);
					brBodova[j]=tmp_brBodova;
				}
			}
		}
		
		//drugi deo zastite resursa
		up(&sem);

		//citanje baze
		if(counter<pos) {	
			len = scnprintf(buff, BUFF_SIZE, "%s %s %s - %d\n", brIndexa[counter], ime[counter], prezime[counter], brBodova[counter]);
			ret = copy_to_user(buffer, buff, len);
			if(ret)
				return -EFAULT;
			counter++;
		}
		if(counter==pos) {
			printk(KERN_INFO "Uspesno procitana baza \n");
			endRead = 1;
		}
	}
	else
		printk(KERN_WARNING "Baza je prazna\n"); 

	return len;
}

ssize_t baza_write(struct file *pfile, const char __user *buffer, size_t length, loff_t *offset) {
	char buff[BUFF_SIZE];
	char rec[20];
	int br;
	int i=0;
	int j=0;
	int ret;

	char tmp_ime[100];
	char tmp_prezime[100];
	char tmp_brIndexa[100];
	int tmp_brBodova;

	ret = copy_from_user(buff, buffer, length);
	if(ret)
		return -EFAULT;
	buff[length-1] = '\0';

	if (buff[0]=='i' && buff[1]=='z' && buff[2]=='b' && buff[3]=='r' && buff[4]=='i' && buff[5]=='s' && buff[6]=='i') {
		//zastita deljenih resursa i blokiranje
		if(down_interruptible(&sem))
			return -ERESTARTSYS;
		while(pos == 0) {
			up(&sem);
			if(wait_event_interruptible(brisanjeQ,(pos>0)))
				return -ERESTARTSYS;
			if(down_interruptible(&sem))
				return -ERESTARTSYS;
		}

		//brisanje studenta
		if(pos>0) {
			ret = sscanf(buff,"%20[^=]=%100[^,],%100[^,],%s", rec, tmp_ime, tmp_prezime, tmp_brIndexa);
			if(ret==4) {
				br=0;	
				for (i=0; i<pos; i++) {
					if(strcmp(ime[i],tmp_ime)==0 &&  strcmp(prezime[i],tmp_prezime)==0 && strcmp(brIndexa[i],tmp_brIndexa)==0 ) {
						br++; // takav student je pronadjen
						for(j=i; j<pos-1; j++) {	// brisanje studenta i pomeranje cele baze za mesto udesno
							strcpy(ime[j],ime[j+1]);
							strcpy(prezime[j],prezime[j+1]);
							strcpy(brIndexa[j],brIndexa[j+1]);
							brBodova[j]=brBodova[j+1];
						}
						ime[pos-1][0]='\0';
						prezime[pos-1][0]='\0';
						brIndexa[pos-1][0] = '\0';
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
				printk(KERN_WARNING "Komanda mora biti izbrisi=ime,prezime,brIndexa\n");
		}
		else
			printk(KERN_WARNING "Baza je prazna, ne mozes izbrisati podatak\n");

		//drugi deo zastite resursa i blokiranja
		up(&sem);
		wake_up_interruptible(&upisQ);
	}
	else {
		//zastita deljenih resursa i blokiranje
		if(down_interruptible(&sem))
			return -ERESTARTSYS;
		while(pos == BAZA_SIZE) {
			up(&sem);
			if(wait_event_interruptible(upisQ,(pos<BAZA_SIZE)))
				return -ERESTARTSYS;
			if(down_interruptible(&sem))
				return -ERESTARTSYS;
		}

		// upis tj. izmena br. bodova
		if(pos<BAZA_SIZE) {
			ret = sscanf(buff,"%100[^,],%100[^,],%100[^=]=%d", tmp_ime, tmp_prezime, tmp_brIndexa, &tmp_brBodova);
			if(ret==4) {	
				br=0;
				for (i=0; i<pos; i++) {
					if(strcmp(ime[i],tmp_ime)==0 &&  strcmp(prezime[i],tmp_prezime)==0 && strcmp(brIndexa[i],tmp_brIndexa)==0 ) {
						br++; //takav student je pronadjen
						if(brBodova[i]==tmp_brBodova) 
				    			printk(KERN_INFO "Takav student vec postoji u bazi\n");
						else {
							brBodova[i]=tmp_brBodova; //izmena broja bodova studentu					
				    			printk(KERN_INFO "Izmena broja bodova studentu\n");
						}
						break;
					}
				}
				if(br==0) {	//upis studenta
				       	strcpy(ime[pos],tmp_ime);
				       	strcpy(prezime[pos],tmp_prezime);
				       	strcpy(brIndexa[pos],tmp_brIndexa);
				       	brBodova[pos]=tmp_brBodova;
					printk(KERN_INFO "Uspesno upisan student na poziciji %d ", pos); 
					pos=pos+1;
				}
			}
			else
				printk(KERN_WARNING "Komanda mora biti ime_prezime_brIndexa=brBodova\n");
		}
		else
			printk(KERN_WARNING "Baza je puna, podatak se ne moze upisati\n");
	
		//drugi deo zastite deljenih resursa i blokiranja
		up(&sem);
		wake_up_interruptible(&brisanjeQ);
	}
	return length;
}

static int __init baza_init(void) {
	int ret = 0;
	int i=0;
	
	sema_init(&sem,1);

	//Inicijalizacija niza
	for (i=0; i<BAZA_SIZE; i++) {
		ime[i][0] = '\0';
		prezime[i][0] = '\0';
		brIndexa[i][0] = '\0';
		brBodova[i] = 0;
	}
  	ret = alloc_chrdev_region(&my_dev_id, 0, 1, "baza");
	if (ret) {
      		printk(KERN_ERR "failed to register char device\n");
      		return ret;
   	}
   	printk(KERN_INFO "char device region allocated\n");

  	my_class = class_create(THIS_MODULE, "baza_class");
   	if (my_class == NULL) {
      		printk(KERN_ERR "failed to create class\n");
      		goto fail_0;
   	}
   	printk(KERN_INFO "class created\n");
   
   	my_device = device_create(my_class, NULL, my_dev_id, NULL, "baza");
   	if (my_device == NULL) {
		printk(KERN_ERR "failed to create device\n");
      		goto fail_1;
   	}
   	printk(KERN_INFO "device created\n");

	my_cdev = cdev_alloc();	
	my_cdev->ops = &my_fops;
	my_cdev->owner = THIS_MODULE;
	ret = cdev_add(my_cdev, my_dev_id, 1);
	if (ret) {
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

static void __exit baza_exit(void) {
	cdev_del(my_cdev);
   	device_destroy(my_class, my_dev_id);
   	class_destroy(my_class);
   	unregister_chrdev_region(my_dev_id,1);
   	printk(KERN_INFO "Goodbye, cruel world\n");
}

module_init(baza_init);
module_exit(baza_exit);
