#ifndef __DDP_PATH_H__
#define __DDP_PATH_H__

#include "ddp_ovl.h"
#include "ddp_rdma.h"
#include "ddp_wdma.h"
#include "ddp_bls.h"
#include "ddp_drv.h"

#define DDP_OVL_LAYER_MUN 4
#define DDP_BACKUP_REG_NUM 0x1000
#define DDP_UNBACKED_REG_MEM 0xdeadbeef

extern unsigned int fb_width;
extern unsigned int fb_height;

int disp_check_engine_status(int mutexID);
int disp_path_release_mutex_(int mutexID);
void disp_path_stop_access_memory(void);

#endif
