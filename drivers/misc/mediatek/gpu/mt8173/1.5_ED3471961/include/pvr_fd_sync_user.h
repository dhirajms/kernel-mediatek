/*************************************************************************/ /*!
@File           pvr_fd_sync_user.h
@Title          Userspace definitions to use the kernel sync driver
@Copyright      Copyright (c) Imagination Technologies Ltd. All Rights Reserved
@License        Dual MIT/GPLv2

The contents of this file are subject to the MIT license as set out below.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

Alternatively, the contents of this file may be used under the terms of
the GNU General Public License Version 2 ("GPL") in which case the provisions
of GPL are applicable instead of those above.

If you wish to allow use of your version of this file only under the terms of
GPL, and not to allow others to use your version of this file under the terms
of the MIT license, indicate your decision by deleting the provisions above
and replace them with the notice and other provisions required by GPL as set
out in the file called "GPL-COPYING" included in this distribution. If you do
not delete the provisions above, a recipient may use your version of this file
under the terms of either the MIT license or GPL.

This License is also included in this distribution in the file called
"MIT-COPYING".

EXCEPT AS OTHERWISE STATED IN A NEGOTIATED AGREEMENT: (A) THE SOFTWARE IS
PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
PURPOSE AND NONINFRINGEMENT; AND (B) IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/ /**************************************************************************/
/* vi: set ts=8: */

#ifndef _PVR_FD_SYNC_USER_H_
#define _PVR_FD_SYNC_USER_H_

#include <linux/types.h>
#include <linux/ioctl.h>

#include "pvrsrv_error.h"

#define PVR_SYNC_MAX_QUERY_FENCE_POINTS 14

#define PVR_SYNC_IOC_MAGIC 'W'

#define PVR_SYNC_IOC_CREATE_FENCE \
 _IOWR(PVR_SYNC_IOC_MAGIC, 0, struct pvr_sync_create_fence_ioctl_data)

#define PVR_SYNC_IOC_ENABLE_FENCING \
 _IOW(PVR_SYNC_IOC_MAGIC,  1, struct pvr_sync_enable_fencing_ioctl_data)

#define PVR_SYNC_IOC_ALLOC_FENCE \
 _IOWR(PVR_SYNC_IOC_MAGIC, 3, struct pvr_sync_alloc_fence_ioctl_data)

#define PVR_SYNC_IOC_RENAME \
 _IOWR(PVR_SYNC_IOC_MAGIC, 4, struct pvr_sync_rename_ioctl_data)

#define PVR_SYNC_IOC_FORCE_SW_ONLY \
 _IO(PVR_SYNC_IOC_MAGIC,   5)

#define PVRSYNC_MODNAME "pvr_sync"

struct pvr_sync_alloc_fence_ioctl_data
{
	/* Output */
	int				iFenceFd;
	int				bTimelineIdle;
}
__attribute__((packed, aligned(8)));

struct pvr_sync_create_fence_ioctl_data
{
	/* Input */
	int				iAllocFenceFd;
	char				szName[32];

	/* Output */
	int				iFenceFd;
}
__attribute__((packed, aligned(8)));

struct pvr_sync_enable_fencing_ioctl_data
{
	/* Input */
	int				bFencingEnabled;
}
__attribute__((packed, aligned(8)));

struct pvr_sync_pt_info {
	/* Output */
	__u32 id;
	__u32 ui32FWAddr;
	__u32 ui32CurrOp;
	__u32 ui32NextOp;
	__u32 ui32TlTaken;
} __attribute__((packed, aligned(8)));

struct pvr_sync_rename_ioctl_data
{
	/* Input */
	char szName[32];
} __attribute__((packed, aligned(8)));

#if !defined(__KERNEL__)

#include <sync/sync.h>
#include <stdbool.h>

/* operates on timelines (only) */

PVRSRV_ERROR PVRFDSyncOpen(int *piSyncFd, bool bSoftware);
PVRSRV_ERROR PVRFDSyncClose(int iSyncFd);

/* operates on fences */

PVRSRV_ERROR PVRFDSyncWaitFenceI(int iFenceFd, const char *szFunc);

PVRSRV_ERROR PVRFDSyncCheckFenceI(int iFenceFd, const char *szFunc);

PVRSRV_ERROR PVRFDSyncMergeFencesI(const char *pcszName,
				   int iFenceFd1, int iFenceFd2,
				   int *piNewFenceFd, const char *szFunc);

PVRSRV_ERROR PVRFDSyncAllocFenceI(int iSyncFd, int *piFenceFd,
				  int *pbTimelineIdle, const char *szFunc);

PVRSRV_ERROR PVRFDSyncCreateFenceI(int iSyncFd, const char *pcszName,
				   int iAllocFenceFd, int *piFenceFd,
				   const char *szFunc);

PVRSRV_ERROR PVRFDSyncEnableFencingI(int iSyncFd, int bFencingEnabled,
				     const char *szFunc);

PVRSRV_ERROR PVRFDSyncQueryFenceI(int iFenceFd,
				  struct sync_fence_info_data *psInfo,
				  uint32_t *pui32NumSyncs,
				  struct sync_pt_info *apsPtInfo,
				  const char *szFunc);

PVRSRV_ERROR PVRFDSyncCloseFenceI(int iFenceFd, const char *szFunc);

int PVRFDSyncDupFenceI(int iFenceFd, const char *szFunc);

PVRSRV_ERROR PVRFDSyncDumpFence(int iFenceFd, const char *pcszModule,
				const char *pcszFmt, ...)
	__attribute__((format(printf,3,4)));

/* sw_sync prototypes; functions defined in libsync */
int sw_sync_timeline_inc(int fd, unsigned count);
int sw_sync_fence_create(int fd, const char *name, unsigned value);

#if defined(PVRSRV_NEED_PVR_DPF)

#define PVRFDSyncWaitFence(a) \
	PVRFDSyncWaitFenceI(a, __func__)
#define PVRFDSyncCheckFence(a) \
	PVRFDSyncCheckFenceI(a, __func__)
#define PVRFDSyncMergeFences(a, b, c, d) \
	PVRFDSyncMergeFencesI(a, b, c, d, __func__)
#define PVRFDSyncAllocFence(a, b, c) \
	PVRFDSyncAllocFenceI(a, b, c, __func__)
#define PVRFDSyncCreateFence(a, b, c, d) \
	PVRFDSyncCreateFenceI(a, b, c, d, __func__)
#define PVRFDSyncEnableFencing(a, b) \
	PVRFDSyncEnableFencingI(a, b, __func__)
#define PVRFDSyncQueryFence(a, b, c, d) \
	PVRFDSyncQueryFenceI(a, b, c, d, __func__)
#define PVRFDSyncCloseFence(a) \
	PVRFDSyncCloseFenceI(a, __func__)
#define PVRFDSyncDupFence(a) \
	PVRFDSyncDupFenceI(a, __func__)

#else /* defined(PVRSRV_NEED_PVR_DPF) */

#define PVRFDSyncWaitFence(a)            PVRFDSyncWaitFenceI(a, "")
#define PVRFDSyncCheckFence(a)           PVRFDSyncCheckFenceI(a, "")
#define PVRFDSyncMergeFences(a, b, c, d) PVRFDSyncMergeFencesI(a, b, c, d, "")
#define PVRFDSyncAllocFence(a, b, c)     PVRFDSyncAllocFenceI(a, b, c, "")
#define PVRFDSyncCreateFence(a, b, c, d) PVRFDSyncCreateFenceI(a, b, c, d, "")
#define PVRFDSyncEnableFencing(a, b)     PVRFDSyncEnableFencingI(a, b, "")
#define PVRFDSyncQueryFence(a, b, c, d)  PVRFDSyncQueryFenceI(a, b, c, d, "")
#define PVRFDSyncCloseFence(a)           PVRFDSyncCloseFenceI(a, "")
#define PVRFDSyncDupFence(a)             PVRFDSyncDupFenceI(a, "")

#endif /* defined(PVRSRV_NEED_PVR_DPF) */

#endif /* defined(__KERNEL__) */

#endif /* _PVR_FD_SYNC_USER_H_ */
