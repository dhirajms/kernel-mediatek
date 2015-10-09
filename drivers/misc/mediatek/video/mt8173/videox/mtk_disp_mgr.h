#ifndef __H_MTK_DISP_MGR__
#define __H_MTK_DISP_MGR__

#include "disp_session.h"

long mtk_disp_mgr_ioctl(struct file *file, unsigned int cmd, unsigned long arg);
int disp_destroy_session(disp_session_config *config);
char *disp_session_mode_spy(unsigned int session_id);
const char *_disp_format_spy(DISP_FORMAT format);

#endif
