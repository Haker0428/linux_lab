#include <asm-generic/ioctl.h>
#include <linux/module.h>
#include <linux/version.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/timer.h>
#include <asm/atomic.h>

#define SECOND_MAJOR  255

static int second_major = SECOND_MAJOR;
module_param(second_major, int, S_IRUGO);

struct second_dev {
	struct cdev cdev;
	atomic_t counter;
	struct timer_list s_timer;
};

static struct second_dev *second_devp;

#if LINUX_VERSION_CODE < KERNEL_VERSION(4,14,0)
static void second_timer_handler(unsigned long arg)
{
	mod_timer(&second_devp->s_timer, jiffies + HZ);
	atomic_inc(&second_devp->counter);
}
#else 
static void second_timer_handler(struct timer_list *t)
{
	mod_timer(&second_devp->s_timer, jiffies + HZ);
	atomic_inc(&second_devp->counter);
}
#endif
static int second_open(struct inode* inode, struct file *filp)
{
	second_devp->s_timer.expires = jiffies + HZ;
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 14, 0)
	init_timer(&second_devp->s_timer);
	second_devp->s_timer.function = &second_timer_handler;
#else 
	timer_setup(&second_devp->s_timer, second_timer_handler, 0);
#endif
	add_timer(&second_devp->s_timer);
	printk("%s ok\n", __func__);
	atomic_set(&second_devp->counter, 0);
	return 0;
}

static int second_release(struct inode *inode, struct file *filp)
{
	del_timer(&second_devp->s_timer);
	return 0;
}

static ssize_t
second_read(struct file *filp, char __user *buf, size_t count, loff_t *ppos)
{
	int counter;
	counter = atomic_read(&second_devp->counter);
	if (put_user(counter, (int*)buf)) {
		return -EFAULT;
	} else {
		return sizeof(unsigned int);
	}
}

static const struct file_operations second_fops = {
	.owner = THIS_MODULE,
	.open = second_open,
	.release = second_release,
	.read = second_read,
};

static void second_setup_cdev(struct second_dev *dev, int index)
{
	int err;
	int devno = MKDEV(second_major, index);
	cdev_init(&dev->cdev, &second_fops);
	dev->cdev.owner = THIS_MODULE;
	err = cdev_add(&dev->cdev, devno, 1);
	if (err) {
		printk(KERN_ERR "Failed to add second device\n");
	}
}

static int __init second_init(void)
{
	int ret;
	dev_t devno =MKDEV(second_major, 0);
	if (second_major) {
		ret = register_chrdev_region(devno, 1, "second");
	} else {
		ret = alloc_chrdev_region(&devno, 0, 1, "second");
		second_major = MAJOR(devno);
	}
	if (ret < 0) {
		return ret;
	}

	second_devp = kzalloc(sizeof(struct second_dev), GFP_KERNEL);
	if (!second_devp) {
		ret = -ENOMEM;
		goto fail_malloc;
	}

	second_setup_cdev(second_devp, 0);
	return 0;
fail_malloc:
	unregister_chrdev_region(devno, 1);
	return ret;
}

static void __exit second_exit(void)
{
	cdev_del(&second_devp->cdev);
	kfree(second_devp);
	unregister_chrdev_region(MKDEV(second_major, 0), 1);
}
module_init(second_init);
module_exit(second_exit);


MODULE_AUTHOR("skyrain");
MODULE_LICENSE("GPL v2");



