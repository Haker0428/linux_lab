#include <linux/build-salt.h>
#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

BUILD_SALT;

MODULE_INFO(vermagic, VERMAGIC_STRING);
MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

#ifdef CONFIG_RETPOLINE
MODULE_INFO(retpoline, "Y");
#endif

static const struct modversion_info ____versions[]
__used
__attribute__((section("__versions"))) = {
	{ 0xc8a61a2c, "module_layout" },
	{ 0xc5850110, "printk" },
	{ 0xe4bbc1dd, "kimage_voffset" },
	{ 0xe007de41, "kallsyms_lookup_name" },
	{ 0xdecd0b29, "__stack_chk_fail" },
	{ 0x7fd0b7cf, "cpu_hwcaps" },
	{ 0x2fee02ba, "cpu_hwcap_keys" },
	{ 0x4829a47e, "memcpy" },
	{ 0xb2ead97c, "kimage_vaddr" },
	{ 0x52da2b65, "arm64_const_caps_ready" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";

