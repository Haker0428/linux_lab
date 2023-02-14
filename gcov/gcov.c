/*
 * Copyright (c) SkyRain. 2022-2022. All rights reserved.
 * Description:
 * Author: sky rain
 * Create: 2022/12/26
 */

#include <linux/init.h>
#include <linux/module.h>


int hello_export_func(void)
{
	printk("hello export func");
	return 0;	
}
EXPORT_SYMBOL(hello_export_func);

static int __init hello_init(void)
{	
	int a = 0;
	printk("my first kernel module init\n");

	if (a) {
		printk("Hello\n");
	}
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
