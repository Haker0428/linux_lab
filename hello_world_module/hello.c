/*
 * Copyright (c) SkyRain. 2022-2022. All rights reserved.
 * Description:
 * Author: sky rain
 * Create: 2022/12/26
 */

#include <linux/init.h>
#include <linux/module.h>

static int debug = 1;
module_param(debug, int, 0644);
MODULE_PARM_DESC(debug, "enable debugging information");

#define dprintk(args...) \
	if (debug) { \
		printk(KERN_DEBUG args); \
	}

static int test_param = 100;
module_param(test_param, int, 0644);
MODULE_PARM_DESC(debug, "test for module parameters");


int hello_export_func(void)
{
	dprintk("hello export func");
	return 0;	
}
EXPORT_SYMBOL(hello_export_func);

static int __init hello_init(void)
{
	dprintk("my first kernel module init\n");
	dprintk("module parameter=%d\n", test_param);
	return 0;
}

static void __exit hello_exit(void)
{
	dprintk("goodbye\n");
}

module_init(hello_init);
module_exit(hello_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("skyrain");
MODULE_DESCRIPTION("my test kernel module");
MODULE_ALIAS("mytest");
