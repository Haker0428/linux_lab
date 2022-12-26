#include <linux/init.h>
#include <linux/module.h>

static int __init hello_init(void)
{
	printk("my first kernel module init\n");
	return 0;
}

static void __exit hello_exit(void)
{
	printk("goodbye\n");
}

module_init(hello_init);
module_exit(hello_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("skyrain");
MODULE_DESCRIPTION("my test kernel module");
MODULE_ALIAS("mytest");
