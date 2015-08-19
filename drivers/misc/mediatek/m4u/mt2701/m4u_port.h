#ifndef __M4U_PORT_H__
#define __M4U_PORT_H__

/* ====================================
 * about portid
 * ====================================
 */
#define M4U_LARB0_PORTn(n)      ((n)+0)
#define M4U_LARB1_PORTn(n)      ((n)+11)
#define M4U_LARB2_PORTn(n)      ((n)+21)
#define M4U_LARB3_PORTn(n)      ((n)+44)

#define M4U_LARB5_PORTn(n)      ((n)+44+2) /*BDP larb*/
#define M4U_LARB6_PORTn(n)      ((n)+44+2+4) /*BDP larb*/

#define M4U_LARB7_PORTn(n)      ((n)+44+2+4+3)


enum {
	DISP_OVL_0               =  M4U_LARB0_PORTn(0),
	DISP_RDMA1               =  M4U_LARB0_PORTn(1)    ,
	DISP_RDMA                =  M4U_LARB0_PORTn(2)    ,
	DISP_WDMA                =  M4U_LARB0_PORTn(3)    ,
	MM_CMDQ                  =  M4U_LARB0_PORTn(4)    ,
	MDP_RDMA                 =  M4U_LARB0_PORTn(5)    ,
	MDP_WDMA                 =  M4U_LARB0_PORTn(6)    ,
	MDP_ROTO                 =  M4U_LARB0_PORTn(7)    ,
	MDP_ROTCO                =  M4U_LARB0_PORTn(8)    ,
	MDP_ROTVO                =  M4U_LARB0_PORTn(9)    ,
	MDP_RDMA1                =  M4U_LARB0_PORTn(10)   ,

	VDEC_MC_EXT              =  M4U_LARB1_PORTn(0)    ,
	VDEC_PP_EXT              =  M4U_LARB1_PORTn(1)    ,
	VDEC_PPWRAP_EXT          =  M4U_LARB1_PORTn(2)    ,
	VDEC_AVC_MV_EXT          =  M4U_LARB1_PORTn(3)    ,
	VDEC_PRED_RD_EXT         =  M4U_LARB1_PORTn(4)    ,
	VDEC_PRED_WR_EXT         =  M4U_LARB1_PORTn(5)    ,
	VDEC_VLD_EXT             =  M4U_LARB1_PORTn(6)    ,
	VDEC_VLD2_EXT            =  M4U_LARB1_PORTn(7)    ,
	VDEC_TILE_EXT            =  M4U_LARB1_PORTn(8)    ,
	VDEC_IMG_RESZ_EXT        =  M4U_LARB1_PORTn(9)    ,

	VENC_RCPU	=  M4U_LARB2_PORTn(0)    ,
	VENC_REC_FRM	=	M4U_LARB2_PORTn(1)    ,
	VENC_BSDMA	=	M4U_LARB2_PORTn(2)    ,
	JPGENC_RDMA	=	M4U_LARB2_PORTn(3)    ,
	VENC_LT_RCPU	=	M4U_LARB2_PORTn(4)    ,
	VENC_LT_REC_FRM	=	M4U_LARB2_PORTn(5)    ,
	VENC_LT_BSDMA	=	M4U_LARB2_PORTn(6)    ,
	JPGDEC_BSDMA	=  M4U_LARB2_PORTn(7)     ,
	VENC_SV_COMV	=	M4U_LARB2_PORTn(8)    ,
	VENC_RD_COMV	=	M4U_LARB2_PORTn(9)    ,
	JPGENC_BSDMA	=	M4U_LARB2_PORTn(10),
	VENC_CUR_LUMA	=	M4U_LARB2_PORTn(11),
	VENC_CUR_CHROMA	=	M4U_LARB2_PORTn(12),
	VENC_REF_LUMA	=	M4U_LARB2_PORTn(13),
	VENC_REF_CHROMA	=	M4U_LARB2_PORTn(14),
	IMG_RESZ	=	M4U_LARB2_PORTn(15),
	VENC_LT_SV_COMV	=  M4U_LARB2_PORTn(16),
	VENC_LT_RD_COMV	=  M4U_LARB2_PORTn(17),
	VENC_LT_CUR_LUMA	=  M4U_LARB2_PORTn(18),
	VENC_LT_CUR_CHROMA	=  M4U_LARB2_PORTn(19),
	VENC_LT_REF_LUMA	=  M4U_LARB2_PORTn(20),
	VENC_LT_REF_CHROMA	=  M4U_LARB2_PORTn(21),
	JPGDEC_WDMA	=  M4U_LARB2_PORTn(22),


	BDP_VDO		=	M4U_LARB5_PORTn(0),
	BDP_WR_CHANEL_DI	=	M4U_LARB5_PORTn(1),
	BDP_NR		=	M4U_LARB5_PORTn(2),
	BDP_REL_DECODE	=	M4U_LARB5_PORTn(3),

	BDP_WR_CHANNEL_VDI	=	M4U_LARB6_PORTn(0),
	BDP_OSD		=	M4U_LARB6_PORTn(1),
	BDP_TVE		=	M4U_LARB6_PORTn(2),

	M4U_CLNTMOD_LCDC_UI      = M4U_LARB7_PORTn(1),

	M4U_PORT_UNKNOWN,
};

#define M4U_CLNTMOD_MAX M4U_PORT_UNKNOWN

/*for build*/
enum {
	CAM_IMGO = M4U_PORT_UNKNOWN + 1,
	CAM_IMG2O,
	CAM_LSCI,
	CAM_IMGI,
	CAM_ESFKO,
	CAM_AAO,
};

#endif

