/* Copyright (c) 2011-2013, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/module.h>
#include "mt2701-afe.h"
#include "mt2701-afe-reg.h"
#include <linux/proc_fs.h>
#include <linux/fs.h>
static ssize_t audio_register_write(struct file *file, const char *buffer, unsigned long len, void *data)
{
	unsigned int switch_cmd = 0;
	unsigned long reg_addr = 0;
	unsigned long reg_val = 0;
	int i = 0;
	int ret;

	ret = sscanf(buffer, " %x %x %x\n", &reg_addr, &reg_val, &switch_cmd);
	if (ret < 0)
		return -EINVAL;
	if (switch_cmd == 0) {
		afe_write(reg_addr, reg_val);
		pr_debug("write reg %s() addr = 0x%08x, val = 0x%08x\n", __func__, reg_addr, reg_val);
	} else {
		for (i = 0; i < switch_cmd; i++)
			pr_debug("read reg  %s() addr = 0x%08x, val = 0x%08x\n", __func__,
			reg_addr + (i * 4), afe_read(reg_addr + (i * 4)));
	}
	return len;
}
static ssize_t audio_register_read(struct file *file, char *page, size_t count, loff_t *data)
{
	char *p = page;

	pr_debug("%s()\n", __func__);
	return 0;
}
static const struct file_operations reg_status_fops = {
	.owner = THIS_MODULE,
	.read = audio_register_read,
	.write = audio_register_write,
};
static int register_debug_mod_init(void)
{
	struct proc_dir_entry *entry;
	struct proc_dir_entry *mt_audio_dir;

	mt_audio_dir = proc_mkdir("audio", NULL);
	if (!mt_audio_dir) {
		pr_err("%s() fail to mkdir audio\n", __func__);
		return -1;
	}
	entry = proc_create("reg", 00640, mt_audio_dir, &reg_status_fops);
	if (!entry) {
		pr_err("%s() fail to create entry\n", __func__);
		return -1;
	}
	return 0;
}
static void register_debug_mod_exit(void)
{
}
module_init(register_debug_mod_init);
module_exit(register_debug_mod_exit);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("register debug driver");
