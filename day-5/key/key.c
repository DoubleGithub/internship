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
unsigned int *address = NULL;
static struct cdev key_cdev;
static dev_t key_no;
static int key_major;
static int key_value[]={0,0,0,0};
static int key_flag=0;
static struct __wait_queue_head key_wait_queue;
struct key_info
{
	char *name;
	int pin;
	int irq;
};
static struct key_info key_array[]=
{
	{"key0",0,IRQ_EINT(26)},
	{"key1",1,IRQ_EINT(27)},
	{"key2",2,IRQ_EINT(28)},
	{"key3",3,IRQ_EINT(29)},
};

void key_gpx_conf_init(void)
{
	int i =0;
	unsigned int value=0;
	address=ioremap(0x11000C60,8);
	value=ioread32(address);
	for(i=0;i<16;i++)
	{
		value|=0x100<<i;//set bit8-bit23 to 1
	}
	iowrite32(value,address);
}
static ssize_t key_read (struct file *fp, char __user *buff, size_t num, loff_t *poff)
{
	int retur=0;
	printk("%s\n",__FUNCTION__);
	if(!key_flag)
	{
		if(fp->f_flags&O_NONBLOCK)
			return -EAGAIN;
		else
		{
			printk("kernel will be sleep\n");
			wait_event_interruptible(key_wait_queue,key_flag!=0);
			printk("kernel will be wakeup\n");
		}
	}
	else 
	{
		key_flag=0;
		retur=copy_to_user(buff,key_value,sizeof(key_value));
		memset(key_value,0,sizeof(key_value));
	}
	return num-retur;
}
int key_open (struct inode *pnode, struct file *fp)
{
	printk("%s\n",__FUNCTION__);
	//等待队列初始化
	init_waitqueue_head(&key_wait_queue);
	key_gpx_conf_init();
	return 0;
}
int key_release (struct inode *pnode, struct file *fp)
{
	printk("%s\n",__FUNCTION__);
	return 0;
}
//static unsigned int key_poll(struct file)
static unsigned int key_poll (struct file *fp, struct poll_table_struct *pt)
{
	int mask=0;
	poll_wait(fp,&key_wait_queue,pt);
	if(key_flag)
	{
		mask = POLLIN|POLLRDNORM;
	}
	return mask;
}
static struct file_operations key_ops =
{
	.owner=THIS_MODULE,
	.read=key_read,
	.open=key_open,
	.release=key_release,
	.poll=key_poll,
};
void cdev_register(void)
{
	alloc_chrdev_region(&key_no,0,1,"key device");
	key_major=MAJOR(key_no);
	printk("key_major=%d\n",key_major);

	key_cdev.owner=THIS_MODULE;
	key_cdev.dev=key_no;
	key_cdev.count=1;
	key_cdev.ops=&key_ops;

	cdev_init(&key_cdev,&key_ops);
	cdev_add(&key_cdev,key_no,1);
}
void key_interrupt_handle(int irq,void *dev_id,struct pt_regs *pt )
{
	struct key_info *pload=(struct key_info *)dev_id;
	printk("%s,pin=%d\n",__FUNCTION__,key_value[pload->pin]);
	key_value[pload->pin]=1;
	key_flag=1;
	//!!!!!!!!!!!!!!
	wake_up_interruptible((void *)&key_wait_queue);

}
void register_interrupt_fun(void)
{
	int i=0;
	int retur=0;
	for(i=0;i<sizeof(key_array)/sizeof(struct key_info);i++)
	{
		retur=request_irq(key_array[i].irq,
						  (irq_handler_t)key_interrupt_handle,
						  IRQF_TRIGGER_FALLING,
						  key_array[i].name,
						  &key_array[i]);
		printk("key%d interrupt register retur = %d\n",i,retur);
	}
}
void release_interrupt_fun(void)
{
	int i=0;
	for(i=0;i<sizeof(key_array)/sizeof(struct key_info);i++)
	{
		free_irq(key_array[i].irq,&key_array[i]);
	}
}
static int __init my_key_init(void)
{
	printk("%s\n",__FUNCTION__);
	cdev_register();	
	register_interrupt_fun();
	return 0;
}
static void __exit my_key_exit(void)
{
	printk("%s\n",__FUNCTION__);
	unregister_chrdev_region(key_no,1);
	cdev_del(&key_cdev);
	release_interrupt_fun();
}
module_init(my_key_init);
module_exit(my_key_exit);
MODULE_LICENSE("Dual BSD/GPL");
