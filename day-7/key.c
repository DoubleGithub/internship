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
#include <linux/timer.h>
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
static struct timer_list key_timer;
void key_gpx_conf_init(void)
{
	int i =0;
	unsigned int value=0;
	address=ioremap(0x11000C60,8);
	value=ioread32(address);
	printk("before modify value=0x%x\n",value);
	for(i=0;i<16;i++)
	{
		value&=~0x100<<i;//set bit8-bit23 to 0
	}
	iowrite32(value,address);
	printk("after modify value=0x%x\n",value);
}
static void timer_handle(unsigned long value_param)
{
	static int i=0;//???
	static int state[4]={0,0,0,0};
	unsigned int value=0;
	int down =0; 
	value = ioread32(address+1);
	for(i=0;i<4;i++)
	{
		down=value&0x4<<i;
		if(down == 0)
		{
			state[i]++;
		}
		if(state[i]==2)
		{
			key_value[i]=1;
			key_flag	=1;
			state[i]	=0;
			wake_up_interruptible((void *)&key_wait_queue);
			while(down == 0)
			{
				value = ioread32(address+1);
				down=value&0x4<<i;
			}
		}
		if(down&&state[i])
		{
			state[i]=0;
		}
	}
	//printk("timer produce i=%d\n",i++);
	key_timer.expires=jiffies+HZ/50;
	add_timer(&key_timer);
}
static void initial_key_timer_list(void)
{
	init_timer(&key_timer);
	key_timer.expires=jiffies+HZ/50;	
	key_timer.function=&timer_handle;
	add_timer(&key_timer);
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
	initial_key_timer_list();
	return 0;
}
int key_release (struct inode *pnode, struct file *fp)
{
	printk("%s\n",__FUNCTION__);
	return 0;
}
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
static int __init my_key_init(void)
{
	printk("%s\n",__FUNCTION__);
	cdev_register();	
	return 0;
}
static void __exit my_key_exit(void)
{
	printk("%s\n",__FUNCTION__);
	unregister_chrdev_region(key_no,1);
	cdev_del(&key_cdev);
	del_timer(&key_timer);
}
module_init(my_key_init);
module_exit(my_key_exit);
//MODULE_LICENSE("Dual BSD/GPL");
