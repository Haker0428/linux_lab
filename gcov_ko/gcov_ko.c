/*
 * Copyright (c) SkyRain. 2022-2022. All rights reserved.
 * Description:
 * Author: sky rain
 * Create: 2022/12/26
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/sched.h>
#include <linux/kallsyms.h>
#include <linux/syscalls.h>
#include <linux/preempt.h>
#include <linux/elf.h>

#define GCOV_COUNTERS			9
#define section_size  (end_rodata - start_rodata)

struct load_info {
	const char *name;
	/* pointer to module in temporary copy, freed at end of load_module() */
	struct module *mod;
	Elf_Ehdr *hdr;
	unsigned long len;
	Elf_Shdr *sechdrs;
	char *secstrings, *strtab;
	unsigned long symoffs, stroffs;
	struct _ddebug *debug;
	unsigned int num_debug;
	bool sig_ok;
#ifdef CONFIG_KALLSYMS
	unsigned long mod_kallsyms_init_off;
#endif
	struct {
		unsigned int sym, str, mod, vers, info, pcpu;
	} index;
};

typedef void (*ctor_fn_t)(void);

typedef long long gcov_type;

struct gcov_ctr_info {
	unsigned int num;
	gcov_type *values;
};

struct gcov_fn_info {
	const struct gcov_info *key;
	unsigned int ident;
	unsigned int lineno_checksum;
	unsigned int cfg_checksum;
	struct gcov_ctr_info ctrs[0];
};

struct gcov_info {
	unsigned int version;
	struct gcov_info *next;
	unsigned int stamp;
	const char *filename;
	void (*merge[GCOV_COUNTERS])(gcov_type *, unsigned int);
	unsigned int n_functions;
	struct gcov_fn_info **functions;
};

void	**sys_call_table_ptr;
void (*update_mapping_prot)(phys_addr_t phys, unsigned long virt, phys_addr_t size, pgprot_t prot);
asmlinkage long (*old_syscall1)(const struct pt_regs *pt_regs);
asmlinkage long (*old_syscall2)(const struct pt_regs *pt_regs);
ctor_fn_t *g_ctor;
unsigned int g_num_ctors;
unsigned long start_rodata, end_rodata;
struct load_info g_load_info;
void *g_hdr;

/* Find a module section: 0 means not found. */
static unsigned int find_sec(struct load_info *info, const char *name)
{
	unsigned int i;

	for (i = 1; i < info->hdr->e_shnum; i++) {
		Elf_Shdr *shdr = &info->sechdrs[i];
		/* Alloc bit cleared means "ignore it." */
		printk("i %d Compare %s\n", i, info->secstrings + shdr->sh_name);
		if ((shdr->sh_flags & SHF_ALLOC)
		    && strcmp(info->secstrings + shdr->sh_name, name) == 0)
			return i;
	}
	return 0;
}

/* Find a module section, or NULL.  Fill in number of "objects" in section. */
static void *section_objs(struct load_info *info,
			  const char *name,
			  size_t object_size,
			  unsigned int *num)
{
	unsigned int sec = find_sec(info, name);

	/* Section 0 has sh_addr 0 and sh_size 0. */
	*num = info->sechdrs[sec].sh_size / object_size;
	printk("output %x\n", info->sechdrs[sec].sh_addr);
	return (void *)info->sechdrs[sec].sh_addr;
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

asmlinkage long new_syscall1(const struct pt_regs *pt_regs)
{
	printk("HPHP new_syscall\n");
        return 0;
}

asmlinkage long new_syscall2(const struct pt_regs *pt_regs)
{
	int err;
	int fd;
	loff_t size;
	
	fd =  (int)pt_regs->regs[0];
	err = kernel_read_file_from_fd(fd, &g_hdr, &size, INT_MAX,
				       READING_MODULE);
	if (err) {
		return err;
	}
	
	g_load_info.hdr = g_hdr;
	g_load_info.len = size; 
	printk("HPHP new_syscall %d \n", g_load_info.hdr->e_shnum);

        return old_syscall2(pt_regs);
}

void ctor_enable_init(void)
{
	update_mapping_prot = (void *)kallsyms_lookup_name("update_mapping_prot");
	sys_call_table_ptr = (void**)kallsyms_lookup_name("sys_call_table");
	start_rodata = (unsigned long)kallsyms_lookup_name("__start_rodata");
	end_rodata= (unsigned long)kallsyms_lookup_name("__inittext_begin");
}

void change_syscall(void)
{
	preempt_disable();
	disable_wirte_protection();
	old_syscall1 = sys_call_table_ptr[__NR_init_module];
	sys_call_table_ptr[__NR_init_module] = new_syscall1;
	old_syscall2 = sys_call_table_ptr[__NR_finit_module];
	sys_call_table_ptr[__NR_finit_module] = new_syscall2;
	enable_wirte_protection();
	preempt_enable();
	printk("change module init ok \n"); 
}

int __gcov_init(struct gcov_info *info)
{
	printk("_gcov_init func %d\n", info->version);
	return 0;	
}
EXPORT_SYMBOL(__gcov_init);

void __gcov_exit(void)
{
	/* Unused. */
	printk("__gcov_exit func");
}
EXPORT_SYMBOL(__gcov_exit);


int __gcov_merge_add(void)
{
	printk("__gcov_merge_add func");
	return 0;	
}
EXPORT_SYMBOL(__gcov_merge_add);

static int gcov_module_notifier(struct notifier_block *nb, unsigned long event,
				void *data)
{
	if (event == MODULE_STATE_GOING) {

		printk("MODULE_STATE_GOING %d\n", g_num_ctors);
	} else if (event == MODULE_STATE_LIVE) {
		printk("MODULE_STATE_LIVE\n");
	} else if (event == MODULE_STATE_COMING) {
		g_load_info.sechdrs = (void *)g_load_info.hdr + g_load_info.hdr->e_shoff;
		g_load_info.secstrings = (void *)g_load_info.hdr
			+ g_load_info.sechdrs[g_load_info.hdr->e_shstrndx].sh_offset;
		g_ctor = section_objs(&g_load_info, ".ctors",
				  sizeof(ctor_fn_t), &g_num_ctors);
		if (!g_ctor) {
			g_ctor = section_objs(&g_load_info, ".init_array",
					sizeof(ctor_fn_t), &g_num_ctors);
		}
		
		if (!g_ctor) {
			printk("Fucking Error");
		}
		printk("MODULE_STATE_COMING\n");
	}
	return 0;
}

static struct notifier_block gcov_nb = {
	.notifier_call	= gcov_module_notifier,
};

static int __init gcov_ko_init(void)
{
	ctor_enable_init();
	change_syscall();
	printk("my first kernel module init\n");
	register_module_notifier(&gcov_nb);
	return 0;
}

static void __exit gcov_ko_exit(void)
{
	printk("goodbye\n");
}

module_init(gcov_ko_init);
module_exit(gcov_ko_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("skyrain");
MODULE_DESCRIPTION("my test kernel module");
MODULE_ALIAS("mytest");
