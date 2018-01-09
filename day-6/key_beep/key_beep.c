#include <linux/init.h>
#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/gpio.h>
#include <linux/fs.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/uaccess.h>
#include <linux/sched.h>
#include <asm/io.h>
#include <asm/types.h>
#include <linux/poll.h>
static dev_t key_beep_number;
static int key_beep_major;
static struct cdev key_beep_cdev;
static int *key_address	=NULL;
static int *beep_address	=NULL;
static int key_flag=0;
static int key_value[]={0,0,0,0};
static struct __wait_queue_head key_beep_wait_queue;
struct key_info
{
	char name[20];
	int pin;
	int irq;
};
static struct key_info key_array[]	=
{
	{"key0",0,IRQ_EINT(26)},	
	{"key1",1,IRQ_EINT(27)},	
	{"key2",2,IRQ_EINT(28)},	
	{"key3",3,IRQ_EINT(29)},	
};
static ssize_t key_beep_read (struct file *fp, char __user *buff, size_t num, loff_t *poff)
{
	int retur = 0;
	printk("%s\n",__FUNCTION__);
	if(!key_flag)
	{
		if(fp->f_flags&O_NONBLOCK)
			return -EAGAIN;	//try again
		else
		{
			wait_event_interruptible(key_beep_wait_queue,key_flag!=0);
		}
	}
	else
	{
		key_flag=0;
		retur=copy_to_user(buff,key_value,sizeof(key_value));
		memset(key_value,0,sizeof(key_value));
	}
	return retur;
}
static ssize_t key_beep_write (struct file *fp, const char __user *buff, size_t num, loff_t *poff)
{
	printk("%s\n",__FUNCTION__);
	return num;
}
static unsigned int key_beep_poll (struct file *fp, struct poll_table_struct *pt)
{
	int mask=0;
	printk("%s\n",__FUNCTION__);
	poll_wait(fp,&key_beep_wait_queue,pt);
	if(key_flag)
	{
		mask = POLLIN|POLLRDNORM;
	}
	return mask;
}
void beep_start(void)
{
	unsigned int value=0;
	printk("beep start\n");
	value	=ioread32(beep_address+1);
	value	|=0x1;
	iowrite32(value,beep_address+1);
}
void beep_stop(void)
{
	unsigned int value=0;
	printk("beep stop\n");
	value	=ioread32(beep_address+1);
	value	&=~0x1;
	iowrite32(value,beep_address+1);
}
static long key_beep_unlocked_ioctl (struct file *fp, unsigned int cmd, unsigned long param)
{
	printk("%s\n",__FUNCTION__);
	switch(cmd)
	{
		case 1:
			beep_start();
			break;
		default:
			beep_stop();
			break;
	}
	return cmd;
}
static void key_beep_cfg_init(void)
{
	unsigned int key_value=0;
	unsigned int beep_value=0;
	printk("%s\n",__FUNCTION__);
	key_address	=ioremap(0x11000C60,8);
	beep_address=ioremap(0x114000A0,8);
	key_value	=ioread32(key_address);
	beep_value	=ioread32(beep_address);
	printk("before modify key_value=0x%x,beep_value=0x%x\n",key_value,beep_value);
	//modify beep as output
	beep_value	&=0xFFFFFFF0;
	beep_value	|=0x00000001;
	//modify key as input
	key_value	&=0xFF0000FF;
	key_value	|=0x00FFFF00;
	iowrite32(key_value,key_address);
	iowrite32(beep_value,beep_address);
	printk("after modify key_value=0x%x,beep_value=0x%x\n",key_value,beep_value);
}
static int key_beep_open (struct inode *pnode, struct file *fp)
{
	printk("%s\n",__FUNCTION__);
	init_waitqueue_head(&key_beep_wait_queue);
	key_beep_cfg_init();
	return 0;
}
static int key_beep_release (struct inode *pnode, struct file *fp)
{
	printk("%s\n",__FUNCTION__);
	return 0;
}
static struct file_operations key_beep_ops =
{
	.owner	=THIS_MODULE,
	.read	=key_beep_read,
	.write	=key_beep_write,
	.poll	=key_beep_poll,
	.unlocked_ioctl=key_beep_unlocked_ioctl,
	.open	=key_beep_open,
	.release=key_beep_release,
};
static void key_beep_cdev_init(void)
{
	printk("%s\n",__FUNCTION__);
	alloc_chrdev_region(&key_beep_number,0,1,"key_beep device");
	key_beep_major = MAJOR(key_beep_number);
	printk("key_beep_major = %d\n",key_beep_major);
	key_beep_cdev.owner	= THIS_MODULE;
	key_beep_cdev.ops	= &key_beep_ops;
	key_beep_cdev.dev	= key_beep_number;
	key_beep_cdev.count	= 1;
	cdev_init(&key_beep_cdev,&key_beep_ops);
	cdev_add(&key_beep_cdev,key_beep_number,1);
}
static irqreturn_t key_beep_interrupt_handle(int irq,void *dev_id)
{
	struct key_info *pload=(struct key_info *)dev_id;
	printk("%s,pin=%d\n",__FUNCTION__,key_value[pload->pin]);
	key_value[pload->pin]=1;
	key_flag=1;
	wake_up_interruptible((void *)&key_beep_wait_queue);
	return IRQ_RETVAL(IRQ_HANDLED);
}
static void key_beep_register_interrupt(void)
{
	unsigned int i=0,retur=0;
	printk("%s\n",__FUNCTION__);
	for(i=0;i<sizeof(key_array)/sizeof(struct key_info);i++)
	{
		retur=request_irq(key_array[i].irq,
						  (irq_handler_t)key_beep_interrupt_handle,
						  IRQF_TRIGGER_FALLING,
						  key_array[i].name,
						  &key_array[i]);	//the last param?
		printk("key%d interrupt register retur = %d\n",i,retur);
	}
}
static void key_beep_release_interrupt(void)
{
	unsigned int i=0;
	printk("%s\n",__FUNCTION__);
	for(i=0;i<sizeof(key_array)/sizeof(struct key_info);i++)
	{
		free_irq(key_array[i].irq,&key_array[i]);	//the second param?
	}
}
static int __init key_beep_init(void)
{
	printk("%s\n",__FUNCTION__);
	key_beep_cdev_init();
	key_beep_register_interrupt();
	return 0;
}
static void __exit key_beep_exit(void)
{
	printk("%s\n",__FUNCTION__);
	unregister_chrdev_region(key_beep_number,1);
	cdev_del(&key_beep_cdev);
	key_beep_release_interrupt();
}
module_init(key_beep_init);
module_exit(key_beep_exit);
MODULE_LICENSE("Dual BSD/GPL");
