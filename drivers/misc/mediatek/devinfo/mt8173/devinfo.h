#ifndef DEVFINO_H
#define DEVFINO_H

 /*****************************************************************************
 * MODULE DEFINITION
 *****************************************************************************/
#define MODULE_NAME	 "[devinfo]"

#ifdef CONFIG_OF
/*device information data*/
#define DEVINFO_MAX_SIZE 48
struct devinfo_tag {
	u32 size;
	u32 tag;
	u32 data[DEVINFO_MAX_SIZE];	/* device information */
	u32 data_size;  /* device information size */
};
#endif

#endif /* end of DEVFINO_H */

