/*
 * Copyright (c) SkyRain. 2022-2022. All rights reserved.
 * Description:
 * Author: sky rain
 * Create: 2022/12/26
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kallsyms.h>
#include <linux/syscalls.h>
#include <linux/preempt.h>

#define section_size  (end_rodata - start_rodata)
void	**sys_call_table_ptr;

void (*update_mapping_prot)(phys_addr_t phys, unsigned long virt, phys_addr_t size, pgprot_t prot);
unsigned long start_rodata, end_rodata;

typedef asmlinkage long (*func_execve)(const char __user *,
                                   const char __user * const __user *,
                                   const char __user *const  __user *);
int test_func(int dev)
{
	if (dev < 0) {
		printk("SB 1");
	} else {
		printk("SB 2");
	}
	return 0;
}
EXPORT_SYMBOL(test_func);

int hello_export_func(void)
{
	int a = 0;
	printk("my first kernel module init\n");

	if (a) {
		printk("Hello\n");
	}
	
	a = 1;
	if (a) {
		printk("Hello 2\n");
	}
	return 0;
}


static void disable_wirte_protection(void)
{
    update_mapping_prot(__pa_symbol(start_rodata), (unsigned long)start_rodata, section_size, PAGE_KERNEL);
    return ;
}
 
static void enable_wirte_protection(void)
{
    update_mapping_prot(__pa_symbol(start_rodata), (unsigned long)start_rodata, section_size, PAGE_KERNEL_RO);
    return ;
}

asmlinkage long (*old_syscall1)(const struct pt_regs *pt_regs);
asmlinkage long (*old_syscall2)(const struct pt_regs *pt_regs);
asmlinkage long new_syscall1(const struct pt_regs *pt_regs)
{
	printk("HPHP new_syscall\n");
        return 0;
}

asmlinkage long new_syscall2(const struct pt_regs *pt_regs)
{
	printk("HPHP new_syscall\n");
        return old_syscall2(pt_regs);
}

static int __init hello_init(void)
{	

	update_mapping_prot = (void *)kallsyms_lookup_name("update_mapping_prot");
	sys_call_table_ptr = (void**)kallsyms_lookup_name("sys_call_table");
	start_rodata = (unsigned long)kallsyms_lookup_name("__start_rodata");
	end_rodata= (unsigned long)kallsyms_lookup_name("__inittext_begin");
	printk("sys_call_table %p, update_mapping_prot:%lx, start_rodata:%lx, end_rodata:%lx.\n", sys_call_table_ptr, update_mapping_prot, start_rodata, end_rodata);
	int a = 1;
	printk("my first kernel module init %d %d\n", __NR_init_module, __NR_finit_module);
	preempt_disable();
	disable_wirte_protection();
	old_syscall1 = sys_call_table_ptr[__NR_init_module];
	sys_call_table_ptr[__NR_init_module] = new_syscall1;
	old_syscall2 = sys_call_table_ptr[__NR_finit_module];
	sys_call_table_ptr[__NR_finit_module] = new_syscall2;
	printk("old sys_call %p\n", old_syscall1); 
	enable_wirte_protection();
	
	preempt_enable();
	
	if (a) {
		printk("Hello\n");
	}
	
	hello_export_func();
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
