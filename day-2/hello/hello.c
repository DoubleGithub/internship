#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
static dev_t beep_no;
static int beep_major;
static struct cdev beep_cdev;
static struct file_operations beep_ops;
MODULE_LICENSE("Dual BSD/GPL");
static int __init hello_init(void)
{
	//alloc device number 动态分配 次设备号从0开始，共一个设备，名字为beep device
	alloc_chrdev_region(&beep_no,0,1,"beep device");
	//get major number from device number
	beep_major=MAJOR(beep_no);
	//
	beep_cdev.owner=THIS_MODULE;
	beep_cdev.ops=&beep_ops;
	beep_cdev.dev=beep_no;
	beep_cdev.count=1;
	//
	cdev_init(&beep_cdev,&beep_ops);
	//
	cdev_add(&beep_cdev,beep_no,1);
	
    printk("%s\n",__FUNCTION__);
    return 0;
}
static void __exit hello_exit(void)
{
    printk("%s\n",__FUNCTION__);
	//release device number
	unregister_chrdev_region(beep_no,1);
	//
	cdev_del(&beep_cdev);
}
module_init(hello_init);
module_exit(hello_exit);
