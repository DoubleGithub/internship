#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <asm/uaccess.h>
#include <asm/io.h>
static dev_t beep_no;
static int beep_major;
static struct cdev beep_cdev;
static int *address=NULL;
int str_len(char *str)
{
	int count=0;
	while(*str!='\0')
	{
		count++;
		str++;
	}
	return count;
}
//static struct file_operations beep_ops;
//ssize_t (*read) (struct file *, char __user *, size_t, loff_t *)
ssize_t beep_read (struct file *fp, char __user *buff, size_t num, loff_t *poff)
{
	char str[]="HELLO,EVERYONE\n";
	long retur=0;
	printk("%s\n",__FUNCTION__);
	retur=copy_to_user(buff,str,str_len(str)+1);
	return retur;
}
//ssize_t (*write) (struct file *, const char __user *, size_t, loff_t *);
ssize_t beep_write (struct file *fp, const char __user *buff, size_t num, loff_t *poff)
{
	char load[100];
	int retur=0;
	retur=copy_from_user(load,buff,num);
	printk("kernel revecive :%s\n",load);
	printk("%s\n",__FUNCTION__);
	return retur;
}
void beep_start(void)
{
	unsigned int value=0;
	value=ioread32(address+1);  //address is int type
	value|=0x1;
	iowrite32(value,address+1);
}
void beep_stop(void)
{
	unsigned int value=0;
	value=ioread32(address+1);  //address is int type
	value&=~0x1;
	iowrite32(value,address+1);
}
//long (*unlocked_ioctl) (struct file *, unsigned int, unsigned long);
long beep_unlocked_ioctl (struct file *fp, unsigned int cmd, unsigned long param)
{
	printk("%s\n",__FUNCTION__);
	switch(cmd)
	{
		case 0://beep stop
			beep_stop();
			break;
		case 1://beep start
			beep_start();
			break;
	}
	printk("%d\n",cmd);
	return param;
}
//int (*open) (struct inode *, struct file *);
int beep_open (struct inode *pnode, struct file *fp)
{
	unsigned int value=0;
	printk("%s\n",__FUNCTION__);
	address=ioremap(0x114000A0,8);
	value=ioread32(address);
	value&=~0x1<<3;	//?
	value&=~0x1<<2;	//?
	value&=~0x1<<1;	//?
	value|=0x1;
	iowrite32(value,address);
	return 0;
}
//int (*release) (struct inode *, struct file *);
int beep_release (struct inode *pnode, struct file *fp)
{
	printk("%s\n",__FUNCTION__);
	return 0;
}
static struct file_operations beep_ops=
{
	.owner=THIS_MODULE,
	.open = beep_open,
	.release=beep_release,
	.read=beep_read,
	.write=beep_write,
	.unlocked_ioctl=beep_unlocked_ioctl,
};
MODULE_LICENSE("Dual BSD/GPL");
static int __init hello_init(void)
{
	//alloc device number
	alloc_chrdev_region(&beep_no,0,1,"beep device");
	//get major number from device number
	beep_major=MAJOR(beep_no);
	printk("beep major=%d\n",beep_major);
	//initial file_operations
	beep_cdev.owner=THIS_MODULE;
	beep_cdev.ops=&beep_ops;
	beep_cdev.dev=beep_no;
	beep_cdev.count=1;
	//initial cdev
	cdev_init(&beep_cdev,&beep_ops);
	//add a cdev
	cdev_add(&beep_cdev,beep_no,1);
	//file_operations

    printk("%s\n",__FUNCTION__);
    return 0;
}
static void __exit hello_exit(void)
{
    printk("%s\n",__FUNCTION__);
	//release device number
	unregister_chrdev_region(beep_no,1);
	//delete a cdev
	cdev_del(&beep_cdev);
}
module_init(hello_init);
module_exit(hello_exit);
