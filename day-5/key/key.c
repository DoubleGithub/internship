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
static struct __wait_queue_head key_wait_queue;	//定义一个等待队列
struct key_info
{
	char *name;
	int pin;
	int irq;
};
/*
   描述按键信息的结构体数组
 */
static struct key_info key_array[]=
{
	{"key0",0,IRQ_EINT(26)},
	{"key1",1,IRQ_EINT(27)},
	{"key2",2,IRQ_EINT(28)},
	{"key3",3,IRQ_EINT(29)},
};
/*
   按键对应的外部中断寄存器配置
 */
void key_gpx_conf_init(void)
{
	int i =0;
	unsigned int value=0;
	printk("%s\n",__FUNCTION__);
	address=ioremap(0x11000C60,8);
	value=ioread32(address);
	printk("before modify value=0x%x\n",value);
	for(i=0;i<16;i++)
	{
		value|=0x100<<i;//set bit8-bit23 to 1
	}
	iowrite32(value,address);
	printk("after modify value=0x%x\n",value);
}
/*
   用户程序调用read函数是对应于驱动模块执行的操作
   包括判断是否以非阻塞方式打开设备驱动、设置进程休眠的等待事件
 */
static ssize_t key_read (struct file *fp, char __user *buff, size_t num, loff_t *poff)
{
	int retur=0;
	printk("%s\n",__FUNCTION__);
	if(!key_flag)	//按键未按下时
	{
		if(fp->f_flags&O_NONBLOCK)	//判断是否是以非阻塞方式打开
			return -EAGAIN;
		else
		{
			printk("kernel will be sleep\n");	
			wait_event_interruptible(key_wait_queue,key_flag!=0);	//设置等待事件，进程休眠
			printk("kernel will be wakeup\n");
		}
	}
	else	//按键已经按下
	{
		key_flag=0;
		retur=copy_to_user(buff,key_value,sizeof(key_value));	//将键值传递到用户空间
		memset(key_value,0,sizeof(key_value));	//重新将键值置0
	}
	return num-retur;
}
/*
   用户程序调用open函数是对应于驱动模块执行的操作
   包括外设寄存器初始化和初始化等待队列
 */
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
/*
   轮循函数，返回一个用来描述操作是否可以立即无阻塞执行的位掩码
 */
static unsigned int key_poll (struct file *fp, struct poll_table_struct *pt)
{
	int mask=0;
	printk("%s\n",__FUNCTION__);
	/*
	   poll_wait，如果当前没有文件描述符可用来执行I/O，则内核将使
	   进程在传递到该系统调用的所有文件描述符对应的等待队列上等待
	 */
	poll_wait(fp,&key_wait_queue,pt);
	printk("%s\n","hahaha");
	if(key_flag)
	{
		mask |= POLLIN|POLLRDNORM;
		/*
		   POLLIN	0x0001
		   POLLRDNORM	0x0040
		 */
	}
	return mask;
}
/*初始化file_operations结构体*/
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
	alloc_chrdev_region(&key_no,0,1,"key device");	//设置内核自动分配设备号
	key_major=MAJOR(key_no);	//提取主设备号
	printk("key_major=%d\n",key_major);
	//初始化cdev结构体成员
	key_cdev.owner=THIS_MODULE;
	key_cdev.dev=key_no;
	key_cdev.count=1;
	key_cdev.ops=&key_ops;
	//在内核中添加和注册cdev模块
	cdev_init(&key_cdev,&key_ops);
	cdev_add(&key_cdev,key_no,1);
}
irqreturn_t key_interrupt_handle(int irq,void *dev_id )		//中断处理函数
{
	int state = 0;
	struct key_info *pload=(struct key_info *)dev_id;	//将dev_id中断描述强制转换为key_info型结构体
	printk("%s,pin=%d,state=%d\n",__FUNCTION__,pload->pin,state);
	key_value[pload->pin]=1;	//设置键值为1,表示按键按下
	key_flag=1;	//设置按键标志为1，表示按键已经按下
	//唤醒等待队列中的进程
	wake_up_interruptible((void *)&key_wait_queue);
	return IRQ_RETVAL(IRQ_HANDLED);	//返回中断处理结束标志
}
void register_interrupt_fun(void)	//注册中断
{
	int i=0;
	int retur=0;
	for(i=0;i<sizeof(key_array)/sizeof(struct key_info);i++)
	{
		retur=request_irq(key_array[i].irq,
						  (irq_handler_t)key_interrupt_handle,
						  IRQF_TRIGGER_FALLING,
						  key_array[i].name,
						  (void *)&key_array[i]);
		/*
		   注册中断服务
		   第一个参数	硬件中断号
		   第二个参数	中断处理函数
		   第三个参数	中断触发方式
		   第四个参数	中断名
		   第五个参数一般设置为本设备的设备结构提或者NULL
		*/
		printk("key%d interrupt register retur = %d\n",i,retur);
	}
}
void release_interrupt_fun(void)	//释放中断
{
	int i=0;
	for(i=0;i<sizeof(key_array)/sizeof(struct key_info);i++)
	{
		free_irq(key_array[i].irq,&key_array[i]);	//释放中断号
	}
}
static int __init my_key_init(void)
{
	printk("%s\n",__FUNCTION__);
	cdev_register();	//初始化字符设备结构体
	register_interrupt_fun();	//注册中端
	return 0;
}
static void __exit my_key_exit(void)
{
	printk("%s\n",__FUNCTION__);
	unregister_chrdev_region(key_no,1);	//释放设备号
	cdev_del(&key_cdev);	//删除字符设备
	release_interrupt_fun();	//释放中断
}
module_init(my_key_init);
module_exit(my_key_exit);
MODULE_LICENSE("Dual BSD/GPL");	//许可证声明
