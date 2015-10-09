#ifndef _MT_HDMI_DEBUG_H_
#define  _MT_HDMI_DEBUG_H_

#ifdef CONFIG_MTK_INTERNAL_HDMI_SUPPORT

extern unsigned char _bHdcpOff;

extern HDMI_AV_INFO_T _stAvdAVInfo;
extern struct HDMI_SINK_AV_CAP_T _HdmiSinkAvCap;
extern unsigned char cDstStr[50];
extern const unsigned char _cFsStr[][7];

extern void mt_hdmi_show_info(char *pbuf);
extern unsigned char vIsDviMode(void);
extern void hdmi_force_plug_out(void);
extern void hdmi_force_plug_in(void);
extern unsigned char hdmi_plug_test_mode;


#endif
#endif
