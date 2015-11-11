#include <linux/module.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/device.h>
#ifdef CONFIG_OF
#include <linux/of_fdt.h>
#endif
#include "devinfo.h"

u32 g_devinfo_data[DEVINFO_MAX_SIZE];

/**************************************************************************
*EXTERN FUNCTION
**************************************************************************/
u32 devinfo_get_size(void)
{
	u32 data_size = 0;

	data_size = ARRAY_SIZE(g_devinfo_data);
	return data_size;
}

u32 get_devinfo_with_index(u32 index)
{
	int size = devinfo_get_size();
	u32 ret = 0;

	if ((index >= 0) && (index < size))
		ret = g_devinfo_data[index];
	else {
		pr_warn("devinfo data index out of range:%d\n", index);
		pr_warn("devinfo data size:%d\n", size);
		ret = 0xFFFFFFFF;
	}

	return ret;
}

#ifdef CONFIG_OF
static int __init devinfo_parse_dt(unsigned long node, const char *uname, int depth, void *data)
{
	struct devinfo_tag *tags;
	int i;
	u32 size = 0;

	if (depth != 1 || (strcmp(uname, "chosen") != 0 && strcmp(uname, "chosen@0") != 0))
		return 0;

	tags = (struct devinfo_tag *) of_get_flat_dt_prop(node, "atag,devinfo", NULL);
	if (tags) {
		size = tags->data_size;
		for (i = 0; i < size; i++)
			g_devinfo_data[i] = tags->data[i];
		/* print chip id for debugging purpose */
		pr_debug("tag_devinfo_data size:%d\n", size);
	}

	return 1;
}

static int __init devinfo_of_init(void)
{
	of_scan_flat_dt(devinfo_parse_dt, NULL);
	return 0;
}
early_initcall(devinfo_of_init);
#endif
MODULE_LICENSE("GPL");


