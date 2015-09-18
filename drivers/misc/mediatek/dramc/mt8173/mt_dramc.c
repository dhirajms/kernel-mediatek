#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/kallsyms.h>
#include <linux/cpu.h>
#include <linux/smp.h>
#include <linux/vmalloc.h>
#include <linux/memblock.h>
#include <linux/sched.h>
#include <linux/delay.h>
#include <asm/cacheflush.h>
#include <linux/bug.h>

/* #include <asm/outercache.h> */
/* #include <asm/system.h> */
/* #include <asm/delay.h> */
#if 0
#include <mach/mt_reg_base.h>
#include <mach/mt_clkmgr.h>
#include "mt_freqhopping.h"
#include <mach/emi_bwl.h> /*  */
#include <mach/mt_typedefs.h>
#include <mach/memory.h>
#include <mach/mt_sleep.h> /*  */
#include <mach/mt_dramc.h>
#include <mach/dma.h>
#include <mach/mt_dcm.h> /*  */
#include <mach/sync_write.h>
#include <mach/md32_ipi.h> /*  */
#include <mach/md32_helper.h>
#endif
#include <linux/of.h>
#include <linux/of_address.h>

#include <mt-plat/mt_io.h>
#include <mt-plat/dma.h>
#include <mt-plat/sync_write.h>
#include "mt_dramc.h"
#include <mach/emi_bwl.h>
#ifdef CONFIG_MTK_CLKMGR
#include <mach/mt_clkmgr.h>
#endif
#ifdef CONFIG_MTK_DCM
#include <mach/mt_dcm.h>
#endif
#ifdef CONFIG_MTK_MD32
#include <mach/md32_ipi.h>
#endif
#if 0 /* #ifdef CONFIG_MTK_SLEEP */
#include <mach/mt_sleep.h>
#endif


void __iomem *CQDMA_BASE_ADDR;
void __iomem *DRAMCAO_BASE_ADDR;
void __iomem *DDRPHY_BASE_ADDR;
void __iomem *DRAMCNAO_BASE_ADDR;

unsigned char *dst_array_v;
unsigned char *src_array_v;
unsigned int dst_array_p;
unsigned int src_array_p;
int init_done = 0;


static DEFINE_MUTEX(dram_dfs_mutex);

#if 0
void enter_pasr_dpd_config(unsigned char segment_rank0, unsigned char segment_rank1)
{
#if 0
	/* all segments of rank1 are not reserved -> rank1 enter DPD */
	if (segment_rank1 == 0xFF)
		slp_dpd_en(1);
#endif

	slp_pasr_en(1, segment_rank0 | (segment_rank1 << 8));
}

void exit_pasr_dpd_config(void)
{
	/* slp_dpd_en(0); */
	slp_pasr_en(0, 0);
}
#endif

#define MEM_TEST_SIZE 0x2000
#define PATTERN1 0x5A5A5A5A
#define PATTERN2 0xA5A5A5A5
int Binning_DRAM_complex_mem_test(void)
{
	unsigned char *MEM8_BASE;
	unsigned short *MEM16_BASE;
	unsigned int *MEM32_BASE;
	unsigned int *MEM_BASE;
	unsigned long mem_ptr;
	unsigned char pattern8;
	unsigned short pattern16;
	unsigned int i, j, size, pattern32;
	unsigned int value;
	unsigned int len = MEM_TEST_SIZE;
	void *ptr;

	ptr = vmalloc(PAGE_SIZE*2);
	MEM8_BASE = (unsigned char *)ptr;
	MEM16_BASE = (unsigned short *)ptr;
	MEM32_BASE = (unsigned int *)ptr;
	MEM_BASE = (unsigned int *)ptr;
	pr_info("Test DRAM start address 0x%lx\n", (unsigned long)ptr);
	pr_info("Test DRAM SIZE 0x%x\n", MEM_TEST_SIZE);
	size = len >> 2;

	/* === Verify the tied bits (tied high) === */
	for (i = 0; i < size; i++)
		MEM32_BASE[i] = 0;

	for (i = 0; i < size; i++) {
		if (MEM32_BASE[i] != 0) {
			vfree(ptr);
			return -1;
		}
		MEM32_BASE[i] = 0xffffffff;
	}

	/* === Verify the tied bits (tied low) === */
	for (i = 0; i < size; i++) {
		if (MEM32_BASE[i] != 0xffffffff) {
			vfree(ptr);
			return -2;
		}
		MEM32_BASE[i] = 0x00;
	}

	/* === Verify pattern 1 (0x00~0xff) === */
	pattern8 = 0x00;
	for (i = 0; i < len; i++)
		MEM8_BASE[i] = pattern8++;
	pattern8 = 0x00;
	for (i = 0; i < len; i++) {
		if (MEM8_BASE[i] != pattern8++) {
			vfree(ptr);
			return -3;
		}
	}

	/* === Verify pattern 2 (0x00~0xff) === */
	pattern8 = 0x00;
	for (i = j = 0; i < len; i += 2, j++) {
		if (MEM8_BASE[i] == pattern8)
			MEM16_BASE[j] = pattern8;
		if (MEM16_BASE[j] != pattern8) {
			vfree(ptr);
			return -4;
		}
		pattern8 += 2;
	}

	/* === Verify pattern 3 (0x00~0xffff) === */
	pattern16 = 0x00;
	for (i = 0; i < (len >> 1); i++)
		MEM16_BASE[i] = pattern16++;
	pattern16 = 0x00;
	for (i = 0; i < (len >> 1); i++) {
		if (MEM16_BASE[i] != pattern16++) {
			vfree(ptr);
			return -5;
		}
	}

	/* === Verify pattern 4 (0x00~0xffffffff) === */
	pattern32 = 0x00;
	for (i = 0; i < (len >> 2); i++)
		MEM32_BASE[i] = pattern32++;
	pattern32 = 0x00;
	for (i = 0; i < (len >> 2); i++) {
		if (MEM32_BASE[i] != pattern32++) {
			vfree(ptr);
			return -6;
		}
	}

	/* === Pattern 5: Filling memory range with 0x44332211 === */
	for (i = 0; i < size; i++)
		MEM32_BASE[i] = 0x44332211;

	/* === Read Check then Fill Memory with a5a5a5a5 Pattern === */
	for (i = 0; i < size; i++) {
		if (MEM32_BASE[i] != 0x44332211) {
			vfree(ptr);
			return -7;
		}
		MEM32_BASE[i] = 0xa5a5a5a5;
	}

	/* === Read Check then Fill Memory with 00 Byte Pattern at offset 0h === */
	for (i = 0; i < size; i++) {
		if (MEM32_BASE[i] != 0xa5a5a5a5) {
			vfree(ptr);
			return -8;
		}
		MEM8_BASE[i * 4] = 0x00;
	}

	/* === Read Check then Fill Memory with 00 Byte Pattern at offset 2h === */
	for (i = 0; i < size; i++) {
		if (MEM32_BASE[i] != 0xa5a5a500) {
			vfree(ptr);
			return -9;
		}
		MEM8_BASE[i * 4 + 2] = 0x00;
	}

	/* === Read Check then Fill Memory with 00 Byte Pattern at offset 1h === */
	for (i = 0; i < size; i++) {
		if (MEM32_BASE[i] != 0xa500a500) {
			vfree(ptr);
			return -10;
		}
		MEM8_BASE[i * 4 + 1] = 0x00;
	}

	/* === Read Check then Fill Memory with 00 Byte Pattern at offset 3h === */
	for (i = 0; i < size; i++) {
		if (MEM32_BASE[i] != 0xa5000000) {
			vfree(ptr);
			return -11;
		}
		MEM8_BASE[i * 4 + 3] = 0x00;
	}

	/* === Read Check then Fill Memory with ffff Word Pattern at offset 1h == */
	for (i = 0; i < size; i++) {
		if (MEM32_BASE[i] != 0x00000000) {
			vfree(ptr);
			return -12;
		}
		MEM16_BASE[i * 2 + 1] = 0xffff;
	}

	/* === Read Check then Fill Memory with ffff Word Pattern at offset 0h == */
	for (i = 0; i < size; i++) {
		if (MEM32_BASE[i] != 0xffff0000) {
			vfree(ptr);
			return -13;
		}
		MEM16_BASE[i * 2] = 0xffff;
	}

	/*===  Read Check === */
	for (i = 0; i < size; i++) {
		if (MEM32_BASE[i] != 0xffffffff) {
			vfree(ptr);
			return -14;
		}
	}

	/************************************************
	* Additional verification
	************************************************/
	/* === stage 1 => write 0 === */

	for (i = 0; i < size; i++)
		MEM_BASE[i] = PATTERN1;

	/* === stage 2 => read 0, write 0xF === */
	for (i = 0; i < size; i++) {
		value = MEM_BASE[i];

		if (value != PATTERN1) {
			vfree(ptr);
			return -15;
		}
		MEM_BASE[i] = PATTERN2;
	}

	/* === stage 3 => read 0xF, write 0 === */
	for (i = 0; i < size; i++) {
		value = MEM_BASE[i];
		if (value != PATTERN2) {
			vfree(ptr);
			return -16;
		}
		MEM_BASE[i] = PATTERN1;
	}

	/* === stage 4 => read 0, write 0xF === */
	for (i = 0; i < size; i++) {
		value = MEM_BASE[i];
		if (value != PATTERN1) {
			vfree(ptr);
			return -17;
		}
		MEM_BASE[i] = PATTERN2;
	}

	/* === stage 5 => read 0xF, write 0 === */
	for (i = 0; i < size; i++) {
		value = MEM_BASE[i];
		if (value != PATTERN2) {
			vfree(ptr);
			return -18;
		}
		MEM_BASE[i] = PATTERN1;
	}

	/* === stage 6 => read 0 === */
	for (i = 0; i < size; i++) {
		value = MEM_BASE[i];
		if (value != PATTERN1) {
			vfree(ptr);
			return -19;
		}
	}

	/* === 1/2/4-byte combination test === */
	mem_ptr = (unsigned long)MEM_BASE;
	while (mem_ptr < ((unsigned long)MEM_BASE + (size << 2))) {
		*((unsigned char *) mem_ptr) = 0x78;
		mem_ptr += 1;
		*((unsigned char *) mem_ptr) = 0x56;
		mem_ptr += 1;
		*((unsigned short *) mem_ptr) = 0x1234;
		mem_ptr += 2;
		*((unsigned int *) mem_ptr) = 0x12345678;
		mem_ptr += 4;
		*((unsigned short *) mem_ptr) = 0x5678;
		mem_ptr += 2;
		*((unsigned char *) mem_ptr) = 0x34;
		mem_ptr += 1;
		*((unsigned char *) mem_ptr) = 0x12;
		mem_ptr += 1;
		*((unsigned int *) mem_ptr) = 0x12345678;
		mem_ptr += 4;
		*((unsigned char *) mem_ptr) = 0x78;
		mem_ptr += 1;
		*((unsigned char *) mem_ptr) = 0x56;
		mem_ptr += 1;
		*((unsigned short *) mem_ptr) = 0x1234;
		mem_ptr += 2;
		*((unsigned int *) mem_ptr) = 0x12345678;
		mem_ptr += 4;
		*((unsigned short *) mem_ptr) = 0x5678;
		mem_ptr += 2;
		*((unsigned char *) mem_ptr) = 0x34;
		mem_ptr += 1;
		*((unsigned char *) mem_ptr) = 0x12;
		mem_ptr += 1;
		*((unsigned int *) mem_ptr) = 0x12345678;
		mem_ptr += 4;
	}

	for (i = 0; i < size; i++) {
		value = MEM_BASE[i];
		if (value != 0x12345678) {
			vfree(ptr);
			return -20;
		}
	}


	/* === Verify pattern 1 (0x00~0xff) === */
	pattern8 = 0x00;
	MEM8_BASE[0] = pattern8;
	for (i = 0; i < size * 4; i++) {
		unsigned char waddr8, raddr8;

		waddr8 = i + 1;
		raddr8 = i;
		if (i < size * 4 - 1)
			MEM8_BASE[waddr8] = pattern8 + 1;
		if (MEM8_BASE[raddr8] != pattern8) {
			vfree(ptr);
			return -21;
		}
		pattern8++;
	}


	/* === Verify pattern 2 (0x00~0xffff) === */
	pattern16 = 0x00;
	MEM16_BASE[0] = pattern16;
	for (i = 0; i < size * 2; i++) {
		if (i < size * 2 - 1)
			MEM16_BASE[i + 1] = pattern16 + 1;
		if (MEM16_BASE[i] != pattern16) {
			vfree(ptr);
			return -22;
		}
		pattern16++;
	}
	/* === Verify pattern 3 (0x00~0xffffffff) === */
	pattern32 = 0x00;
	MEM32_BASE[0] = pattern32;
	for (i = 0; i < size; i++) {
		if (i < size - 1)
			MEM32_BASE[i + 1] = pattern32 + 1;
		if (MEM32_BASE[i] != pattern32) {
			vfree(ptr);
			return -23;
		}
		pattern32++;
	}
	pr_info("complex R/W mem test pass\n");
	vfree(ptr);
	return 1;
}

unsigned int ucDram_Register_Read(unsigned long u4reg_addr)
{
	unsigned int pu4reg_value;

#if 0
	pu4reg_value = (*(unsigned int *)((uintptr_t)CHA_DRAMCAO_BASE + (u4reg_addr))) |
		(*(unsigned int *)((uintptr_t)CHA_DDRPHY_BASE + (u4reg_addr))) |
		(*(unsigned int *)((uintptr_t)CHA_DRAMCNAO_BASE + (u4reg_addr)));
#else
	pu4reg_value = (readl(IOMEM(DRAMCAO_BASE_ADDR + u4reg_addr)) |
		readl(IOMEM(DDRPHY_BASE_ADDR + u4reg_addr)) |
		readl(IOMEM(DRAMCNAO_BASE_ADDR + u4reg_addr)));
#endif

	return pu4reg_value;
}

void ucDram_Register_Write(unsigned long u4reg_addr, unsigned int u4reg_value)
{
#if 0
	(*(unsigned int *)((uintptr_t)CHA_DRAMCAO_BASE + (u4reg_addr))) = u4reg_value;
	(*(unsigned int *)((uintptr_t)CHA_DDRPHY_BASE + (u4reg_addr))) = u4reg_value;
	(*(unsigned int *)((uintptr_t)CHA_DRAMCNAO_BASE + (u4reg_addr))) = u4reg_value;
#else
	writel(u4reg_value, IOMEM(DRAMCAO_BASE_ADDR + u4reg_addr));
	writel(u4reg_value, IOMEM(DDRPHY_BASE_ADDR + u4reg_addr));
	writel(u4reg_value, IOMEM(DRAMCNAO_BASE_ADDR + u4reg_addr));
#endif
	/* dsb(); */
	mb();
}

bool pasr_is_valid(void)
{
	unsigned int ddr_type = 0;

	ddr_type = get_ddr_type();
	/* Following DDR types can support PASR */
	if ((ddr_type == TYPE_LPDDR3) || (ddr_type == TYPE_LPDDR2))
		return true;
	return false;
}

/* ------------------------------------------------------------------------- */
/** Round_Operation
 *  Round operation of A/B
 *  @param  A
 *  @param  B
 *  @retval round(A/B)
 */
/* ------------------------------------------------------------------------- */
unsigned int Round_Operation(unsigned int A, unsigned int B)
{
	unsigned int temp;

	if (B == 0)
		return 0xffffffff;

	temp = A/B;

	if ((A-temp*B) >= ((temp+1)*B-A))
		return (temp+1);
	else
		return temp;
}

unsigned int get_dram_data_rate(void)
{
	unsigned int u4value1, u4value2, MPLL_POSDIV, MPLL_PCW, MPLL_FOUT;
	unsigned int MEMPLL_FBKDIV, MEMPLL_FOUT;

	u4value1 = (*(unsigned int *)(0xF0209280));
	u4value2 = (u4value1 & 0x00000070) >> 4;
	if (u4value2 == 0)
		MPLL_POSDIV = 1;
	else if (u4value2 == 1)
		MPLL_POSDIV = 2;
	else if (u4value2 == 2)
		MPLL_POSDIV = 4;
	else if (u4value2 == 3)
		MPLL_POSDIV = 8;
	else
		MPLL_POSDIV = 16;

	u4value1 = *(unsigned int *)(0xF0209284);
	MPLL_PCW = (u4value1 & 0x001fffff);

	MPLL_FOUT = 26/1*MPLL_PCW;
	MPLL_FOUT = Round_Operation(MPLL_FOUT, MPLL_POSDIV*28); /* freq*16384 */

	u4value1 = *(unsigned int *)(0xF000F614);
	MEMPLL_FBKDIV = (u4value1 & 0x007f0000) >> 16;

	MEMPLL_FOUT = MPLL_FOUT*1*4*(MEMPLL_FBKDIV+1);
	MEMPLL_FOUT = Round_Operation(MEMPLL_FOUT, 16384);

	/* pr_info("MPLL_POSDIV=%d, MPLL_PCW=0x%x, MPLL_FOUT=%d, MEMPLL_FBKDIV=%d, MEMPLL_FOUT=%d\n",
	MPLL_POSDIV, MPLL_PCW, MPLL_FOUT, MEMPLL_FBKDIV, MEMPLL_FOUT); */

	return MEMPLL_FOUT;
}

unsigned int DRAM_MRR(int MRR_num)
{
	unsigned int MRR_value = 0x0;
	unsigned int u4value;

	/* set DQ bit 0, 1, 2, 3, 4, 5, 6, 7 pinmux for LPDDR3 */
	ucDram_Register_Write(DRAMC_REG_RRRATE_CTL, 0x13121110);
	ucDram_Register_Write(DRAMC_REG_MRR_CTL, 0x17161514);

	ucDram_Register_Write(DRAMC_REG_MRS, MRR_num);
	ucDram_Register_Write(DRAMC_REG_SPCMD, 0x00000002);
	udelay(1);
	ucDram_Register_Write(DRAMC_REG_SPCMD, 0x00000000);
	udelay(1);

	u4value = ucDram_Register_Read(DRAMC_REG_SPCMDRESP);
	MRR_value = (u4value >> 20) & 0xFF;

	return MRR_value;
}


unsigned int read_dram_temperature(void)
{
	unsigned int value;

	value = DRAM_MRR(4) & 0x7;
	return value;
}

int get_ddr_type(void)
{
	unsigned int value;

	value =  ucDram_Register_Read(DRAMC_LPDDR2);
	/* check LPDDR2_EN */
	if ((value>>28) & 0x1)
		return TYPE_LPDDR2;

	value = ucDram_Register_Read(DRAMC_PADCTL4);
	/* check DDR3_EN */
	if ((value>>7) & 0x1)
		return TYPE_PCDDR3;

	value = ucDram_Register_Read(DRAMC_ACTIM1);
	/* check LPDDR3_EN */
	if ((value>>28) & 0x1)
		return TYPE_LPDDR3;

	return TYPE_DDR1;
}

#if 0
unsigned int shuffer_done;

void dram_dfs_ipi_handler(int id, void *data, unsigned int len)
{
	shuffer_done = 1;
}
#endif

#if 0
void Reg_Sync_Writel(addr, val)
{
	(*(unsigned int *)(uintptr_t)(addr)) = val;
	dsb();
}

unsigned int Reg_Readl(addr)
{
	return (*(unsigned int *)(uintptr_t)(addr));
}
#else
#define Reg_Sync_Writel(addr, val)   writel(val, IOMEM(addr))
#define Reg_Readl(addr) readl(IOMEM(addr))
#endif

#if 0
__attribute__ ((__section__(".sram.func"))) void uart_print(unsigned char ch)
{
	int i;

	for (i = 0; i < 5; i++)
		(*(unsigned int *)(0xF1003000)) = ch;
}
#endif

void do_DRAM_DFS(int high_freq)
{
	/*U32 u4HWTrackR0, u4HWTrackR1, u4HWGatingEnable;*/
	int ddr_type;

	  mutex_lock(&dram_dfs_mutex);

	ddr_type = get_ddr_type();
	switch (ddr_type) {
	case TYPE_DDR1:
		pr_info("not support DDR1\n");
		BUG();
	case TYPE_LPDDR2:
		pr_info("[DRAM DFS] LPDDR2\n");
		break;
	case TYPE_LPDDR3:
		pr_info("[DRAM DFS] LPDDR3\n");
		break;
	case TYPE_PCDDR3:
		pr_info("not support PCDDR3\n");
		BUG();
	default:
		BUG();
	}

	if (high_freq == 1)
		pr_info("[DRAM DFS] Switch to high frequency\n");
	else
		pr_info("[DRAM DFS] switch to low frequency\n");

	/* DramcEnterSelfRefresh(p, 1); // enter self refresh */
	/* mcDELAY_US(1); */
	/* Read back HW tracking first. After shuffle finish, need to copy this value into SW fine tune. */

#if 0
	if (Reg_Readl((DRAMCAO_BASE_ADDR + 0x1c0)) & 0x80000000)
		u4HWGatingEnable = 1;
	else
		u4HWGatingEnable = 0;

	if (u4HWGatingEnable) {
		/* cha DLLFRZ=0 */
		Reg_Sync_Writel((DRAMCAO_BASE_ADDR + 0x028), Reg_Readl((DRAMCAO_BASE_ADDR + 0x028)) & (~(0x01<<30)));
		u4HWTrackR0 = Reg_Readl((DRAMCNAO_BASE_ADDR + 0x374));	/* cha r0 */
		u4HWTrackR1 = Reg_Readl((DRAMCNAO_BASE_ADDR + 0x378));	/* cha r1 */
		Reg_Sync_Writel((DRAMCAO_BASE_ADDR + 0x028), Reg_Readl((DRAMCAO_BASE_ADDR + 0x028)) | (0x01<<30));
	}
#endif
	/* shuffer_done = 0; */

	if (high_freq == 1) {
		/* Shuffle to high */
#ifndef SHUFFER_BY_MD32
		unsigned int read_data;
		unsigned int bak_data1;
		unsigned int bak_data2;
		unsigned int bak_data3;
		unsigned int bak_data4;
#else
		unsigned int shuffer_to_high = 1;
#endif

#if 0
		if (u4HWGatingEnable) {
			/* Current is low frequency.
			Save to low frequency fine tune here because shuffle enable will cause HW GW reload. */
			Reg_Sync_Writel((DRAMCAO_BASE_ADDR + 0x840), u4HWTrackR0);
			Reg_Sync_Writel((DRAMCAO_BASE_ADDR + 0x844), u4HWTrackR1);
		}
#endif

		/* MR2 RL/WL set */
		#ifdef DUAL_FREQ_DIFF_RLWL
		unsigned int value;

		value =  ucDram_Register_Read(0x041C);
		/* 1866 AC timing */
		if (value == 0x44444444)
			ucDram_Register_Write(0x088, (ddr_type == TYPE_LPDDR3) ? 0x001C0002 : LPDDR2_MODE_REG_2);
		else
			ucDram_Register_Write(0x088, (ddr_type == TYPE_LPDDR3) ? LPDDR3_MODE_REG_2 : LPDDR2_MODE_REG_2);
		#else
			ucDram_Register_Write(0x088, (ddr_type == TYPE_LPDDR3) ? LPDDR3_MODE_REG_2 : LPDDR2_MODE_REG_2);
		#endif

#ifndef SHUFFER_BY_MD32
		bak_data1 = Reg_Readl((CHA_DRAMCAO_BASE + (0x77 << 2)));
		Reg_Sync_Writel((CHA_DRAMCAO_BASE + (0x77 << 2)), bak_data1 & ~(0xc0000000));

		Reg_Sync_Writel((CLK_CFG_0_CLR), 0x300);
		Reg_Sync_Writel((CLK_CFG_0_SET), 0x100);

		bak_data2 = Reg_Readl((CHA_DDRPHY_BASE + (0x190 << 2)));
		bak_data4 = Reg_Readl((PCM_INI_PWRON0_REG));

		/* Shuffle to high start. */
		/* Reg.28h[17]=1 R_DMSHU_DRAMC */
		/* Reg.28h[16]=0 R_DMSHU_LOW */
		bak_data3 = Reg_Readl((CHA_DRAMCAO_BASE + (0x00a << 2)));

		/* Block EMI start. */
		Reg_Sync_Writel((CHA_DRAMCAO_BASE + (0x00a << 2)), (bak_data3 & (~0x00030000)) | 0x20000);

		/* Wait shuffle_end Reg.16ch[0] == 1 */
		read_data = Reg_Readl((CHA_DRAMCAO_BASE + (0x05b << 2)));
		while ((read_data & 0x01)  != 0x01)
			read_data = Reg_Readl((CHA_DRAMCAO_BASE + (0x05b << 2)));

		/* [3:0]=1 : VCO/4 . [4]=0 : RG_MEMPLL_ONEPLLSEL. [12:5] RG_MEMPLL_RSV. RG_MEMPLL_RSV[1]=1 */
		Reg_Sync_Writel((CHA_DDRPHY_BASE + (0x1a6 << 2)), 0x00001e41);
		/* K2?? *(unsigned int*)(0x10006000) = 0x0b160001?? */
		Reg_Sync_Writel((PCM_INI_PWRON0_REG), bak_data4 & (~0x8000000));

		mcDELAY_US(10);		/* Wait 1us. */

		Reg_Sync_Writel((CLK_CFG_UPDATE), 0x02);

		Reg_Sync_Writel((CHA_DDRPHY_BASE + (0x190 << 2)), bak_data2 & (~0x01));	/* sync = 1 */
		/* sync back to original. Should be 0.
		But in case of SPM control, need to make sure SPM is not toggling. */
		Reg_Sync_Writel((CHA_DDRPHY_BASE + (0x190 << 2)), bak_data2 | 0x01);

		/* block EMI end. */
		Reg_Sync_Writel((CHA_DRAMCAO_BASE + (0x00a << 2)), bak_data3 & (~0x30000));

		Reg_Sync_Writel((CHA_DRAMCAO_BASE + (0x77 << 2)), bak_data1);
#else
		while (md32_ipi_send(IPI_DFS, (void *)&shuffer_to_high, sizeof(U32), 1) != DONE)
			;

		/* while(shuffer_done == 0); */
		pr_info("[DRAM DFS] Shuffer to high by MD32\n");
#endif

#if 0
		Reg_Sync_Writel((MEM_DCM_CTRL),
		(0x1f << 21) | (0x0 << 16) | (0x01 << 9) | (0x01 << 8) | (0x01 << 7) | (0x0 << 6) | (0x1f<<1));
		Reg_Sync_Writel((MEM_DCM_CTRL), Reg_Readl(MEM_DCM_CTRL) | 0x01);
		Reg_Sync_Writel((MEM_DCM_CTRL), Reg_Readl(MEM_DCM_CTRL) & (~0x01));

		Reg_Sync_Writel((DFS_MEM_DCM_CTRL),
			(0x01 << 28) | (0x1f << 21) | (0x0 << 16) | (0x01 << 9) |
			(0x01 << 8) | (0x0 << 7) | (0x0 << 6) | (0x1f << 1));
		Reg_Sync_Writel((DFS_MEM_DCM_CTRL), Reg_Readl(DFS_MEM_DCM_CTRL) | 0x01);
		Reg_Sync_Writel((DFS_MEM_DCM_CTRL), Reg_Readl(DFS_MEM_DCM_CTRL) & (~0x01));
		Reg_Sync_Writel((DFS_MEM_DCM_CTRL), Reg_Readl(DFS_MEM_DCM_CTRL) | (0x01 << 31));
#else
#ifdef CONFIG_MTK_DCM
		mt_dcm_emi_3pll_mode();
#endif
#endif

		/* [3:0]=1 : VCO/4 . [4]=0 : RG_MEMPLL_ONEPLLSEL. [12:5] RG_MEMPLL_RSV. RG_MEMPLL_RSV[1]=0 ==>
		disable output. */
		Reg_Sync_Writel((DDRPHY_BASE_ADDR + (0x1a6 << 2)), 0x00001e01);
	} else {
		/* Shuffle to low */
#ifndef SHUFFER_BY_MD32
	unsigned int read_data;
	unsigned int bak_data1;
	unsigned int bak_data2;
	unsigned int bak_data3;
#else
	unsigned int shuffer_to_high = 0;
#endif

#if 0
		if (u4HWGatingEnable) {
			/* Current is low frequency.
			Save to high frequency fine tune here because shuffle enable will cause HW GW reload. */
			Reg_Sync_Writel((DRAMCAO_BASE_ADDR + 0x94), u4HWTrackR0);
			Reg_Sync_Writel((DRAMCAO_BASE_ADDR + 0x98), u4HWTrackR1);
		}
#endif

		/* MR2 RL/WL set */
		#ifdef DUAL_FREQ_DIFF_RLWL
		ucDram_Register_Write(0x088, (ddr_type == TYPE_LPDDR3) ? LPDDR3_MODE_REG_2_LOW : LPDDR2_MODE_REG_2_LOW);
		#else
		ucDram_Register_Write(0x088, (ddr_type == TYPE_LPDDR3) ? LPDDR3_MODE_REG_2 : LPDDR2_MODE_REG_2);
		#endif

		/* [3:0]=1 : VCO/4 . [4]=0 : RG_MEMPLL_ONEPLLSEL. [12:5] RG_MEMPLL_RSV. RG_MEMPLL_RSV[1]=1 */
		Reg_Sync_Writel((DDRPHY_BASE_ADDR + (0x1a6 << 2)), 0x00001e41);

		Reg_Sync_Writel((DDRPHY_BASE_ADDR + (0x186 << 2)),
			Reg_Readl(DDRPHY_BASE_ADDR + (0x186 << 2)) | 0x10000);	/* Switch MEMPLL2 reset mode select */
		Reg_Sync_Writel((DDRPHY_BASE_ADDR + (0x189 << 2)),
			Reg_Readl(DDRPHY_BASE_ADDR + (0x189 << 2)) | 0x10000);	/* Switch MEMPLL3 reset mode select */

#if 0
		Reg_Sync_Writel((MEM_DCM_CTRL),
		(0x1f << 21) | (0x0 << 16) | (0x01 << 9) | (0x01 << 8) | (0x0 << 7) | (0x0 << 6) | (0x1f << 1));
		Reg_Sync_Writel((MEM_DCM_CTRL), Reg_Readl(MEM_DCM_CTRL) | 0x01);
		Reg_Sync_Writel((MEM_DCM_CTRL), Reg_Readl(MEM_DCM_CTRL) & (~0x01));

		Reg_Sync_Writel((DFS_MEM_DCM_CTRL), (0x01 << 31));
		Reg_Sync_Writel((DFS_MEM_DCM_CTRL),
			Reg_Readl(DFS_MEM_DCM_CTRL) | ((0x01 << 28) | (0x1f << 21) |
			(0x0 << 16) | (0x01 << 9) | (0x01 << 8) | (0x01 << 7) | (0x0 << 6) | (0x1f << 1)));
		Reg_Sync_Writel((DFS_MEM_DCM_CTRL), Reg_Readl(DFS_MEM_DCM_CTRL) &  (~(0x01 << 31)));

		Reg_Sync_Writel((DFS_MEM_DCM_CTRL), Reg_Readl(DFS_MEM_DCM_CTRL) | 0x01);
		Reg_Sync_Writel((DFS_MEM_DCM_CTRL), Reg_Readl(DFS_MEM_DCM_CTRL) & (~0x01));
#else
#ifdef CONFIG_MTK_DCM
	mt_dcm_emi_1pll_mode();
#endif
#endif

#ifndef SHUFFER_BY_MD32
		bak_data1 = Reg_Readl((CHA_DRAMCAO_BASE + (0x20a << 2)));
		Reg_Sync_Writel((CHA_DRAMCAO_BASE + (0x20a << 2)), bak_data1 & (~0xc0000000));

		Reg_Sync_Writel((CLK_CFG_0_CLR), 0x300);
		Reg_Sync_Writel((CLK_CFG_0_SET), 0x200);

		bak_data2 = Reg_Readl((CHA_DDRPHY_BASE + (0x190 << 2)));

		/* Shuffle to low. */
		/* Reg.28h[17]=1 R_DMSHU_DRAMC */
		/* Reg.28h[16]=1 R_DMSHU_LOW */
		bak_data3 = Reg_Readl((CHA_DRAMCAO_BASE + (0x00a << 2)));

		/* Block EMI start. */
		Reg_Sync_Writel((CHA_DRAMCAO_BASE + (0x00a << 2)), bak_data3 | 0x30000);

		/* Wait shuffle_end Reg.16ch[0] == 1 */
		read_data = Reg_Readl((CHA_DRAMCAO_BASE + (0x05b << 2)));
		while ((read_data & 0x01)  != 0x01)
			read_data = Reg_Readl((CHA_DRAMCAO_BASE + (0x05b << 2)));

		/* [3:0]=1 : VCO/4 . [4]=1 : RG_MEMPLL_ONEPLLSEL. [12:5] RG_MEMPLL_RSV. RG_MEMPLL_RSV[1]=1. [15]=1?? */
		Reg_Sync_Writel((CHA_DDRPHY_BASE + (0x1a6 << 2)), 0x00009e51);

		Reg_Sync_Writel((CLK_CFG_UPDATE), 0x02);

		Reg_Sync_Writel((CHA_DDRPHY_BASE + (0x190 << 2)), bak_data2 & (~0x01));
		Reg_Sync_Writel((CHA_DDRPHY_BASE + (0x190 << 2)), bak_data2 | 0x01);

		/* Block EMI end. */
		Reg_Sync_Writel((CHA_DRAMCAO_BASE + (0x00a << 2)), bak_data3 & (~0x30000));
		Reg_Sync_Writel((CHA_DRAMCAO_BASE + (0x20a << 2)), bak_data1);

		Reg_Sync_Writel((PCM_INI_PWRON0_REG), Reg_Readl(PCM_INI_PWRON0_REG) | 0x8000000);
#else
	while (md32_ipi_send(IPI_DFS, (void *)&shuffer_to_high, sizeof(U32), 1) != DONE)
		;

	/* while(shuffer_done == 0); */
	pr_info("[DRAM DFS] Shuffer to low by MD32\n");
#endif
	}

	mutex_unlock(&dram_dfs_mutex);

	/* DramcEnterSelfRefresh(p, 0); // exit self refresh */
}

int DFS_APDMA_early_init(void)
{
	phys_addr_t max_dram_size = get_max_DRAM_size();
	phys_addr_t dummy_read_center_address;

	if (init_done == 0) {
		if (max_dram_size == 0x100000000ULL) {
			/* dram size = 4GB */
			dummy_read_center_address = 0x80000000ULL;
		} else if (max_dram_size <= 0xC0000000) {
			/* dram size <= 3GB*/
			dummy_read_center_address = DRAM_BASE+(max_dram_size >> 1);
		} else
			BUG_ON(!(0));

		src_array_p = (unsigned int)(dummy_read_center_address - (BUFF_LEN >> 1));
		dst_array_p = (unsigned int)(dummy_read_center_address + (BUFF_LEN >> 1));

#ifdef APDMAREG_DUMP
		src_array_v = ioremap(rounddown(src_array_p, IOREMAP_ALIGMENT),
		IOREMAP_ALIGMENT << 1)+IOREMAP_ALIGMENT-(BUFF_LEN >> 1);
		dst_array_v = src_array_v+BUFF_LEN;
#endif

		init_done = 1;
	}

	return 1;
}

int DFS_APDMA_Init(void)
{
	writel(((~DMA_GSEC_EN_BIT)&readl(DMA_GSEC_EN)), DMA_GSEC_EN);
	return 1;
}

int DFS_APDMA_Enable(void)
{
#ifdef APDMAREG_DUMP
	int i;
#endif

	while (readl(DMA_START) & 0x1)
		;
	writel(src_array_p, DMA_SRC);
	writel(dst_array_p, DMA_DST);
	writel(BUFF_LEN , DMA_LEN1);
	writel(DMA_CON_BURST_8BEAT, DMA_CON);

#ifdef APDMAREG_DUMP
	pr_info("src_p=0x%x, dst_p=0x%x, src_v=0x%x, dst_v=0x%x, len=%d\n",
	src_array_p, dst_array_p, (unsigned int)src_array_v, (unsigned int)dst_array_v, BUFF_LEN);
	for (i = 0; i < 0x60; i += 4)
		pr_info("[Before]addr:0x%x, value:%x\n", (unsigned int)(DMA_BASE+i), *((unsigned int *)(DMA_BASE+i)));

#ifdef APDMA_TEST
	for (i = 0; i < BUFF_LEN/sizeof(unsigned int); i++) {
		dst_array_v[i] = 0;
		src_array_v[i] = i;
	}
#endif
#endif

	mt_reg_sync_writel(0x1, DMA_START);

#ifdef APDMAREG_DUMP
	for (i = 0; i < 0x60; i += 4)
		pr_info("[AFTER]addr:0x%x, value:%x\n", (unsigned int)(DMA_BASE+i), *((unsigned int *)(DMA_BASE+i)));

#ifdef APDMA_TEST
	for (i = 0; i < BUFF_LEN/sizeof(unsigned int); i++) {
		if (dst_array_v[i] != src_array_v[i]) {
			pr_info("DMA ERROR at Address %x\n (i=%d, value=0x%x(should be 0x%x))",
				(unsigned int)&dst_array_v[i], i, dst_array_v[i], src_array_v[i]);
			ASSERT(0);
		}
	}
	pr_info("Channe0 DFS DMA TEST PASS\n");
#endif
#endif
	return 1;
}

int DFS_APDMA_END(void)
{
	while (readl(DMA_START))
		;

	return 1;
}

void dma_dummy_read_for_vcorefs(int loops)
{
	int i, count;
	unsigned long long start_time, end_time, duration;

	DFS_APDMA_early_init();
	#ifdef CONFIG_MTK_CLKMGR
	enable_clock(MT_CG_INFRA_GCE, "CQDMA");
	#endif
	for (i = 0; i < loops; i++) {
		count = 0;
		start_time = sched_clock();
		do {
			DFS_APDMA_Enable();
			DFS_APDMA_END();
			end_time = sched_clock();
			duration = end_time - start_time;
			count++;
		} while (duration < 4000L);
		/* pr_info("[DMA_dummy_read[%d], duration=%lld, count = %d\n", duration, count); */
	}
	#ifdef CONFIG_MTK_CLKMGR
	disable_clock(MT_CG_INFRA_GCE, "CQDMA");
	#endif
}

#if 0
/*
 * XXX: Reserved memory in low memory must be 1MB aligned.
 *	 This is because the Linux kernel always use 1MB section to map low memory.
 *
 *	We Reserved the memory regien which could cross rank for APDMA to do dummy read.
 *
 */

void DFS_Reserved_Memory(void)
{
	phys_addr_t high_memory_phys;
	phys_addr_t DFS_dummy_read_center_address;
	phys_addr_t max_dram_size = get_max_DRAM_size();

	high_memory_phys = virt_to_phys(high_memory);

	if (max_dram_size == 0x100000000ULL) {
		/* dram size = 4GB */
		DFS_dummy_read_center_address = 0x80000000ULL;
	} else if (max_dram_size <= 0xC0000000) {
		/* dram size <= 3GB */
		DFS_dummy_read_center_address = DRAM_BASE+(max_dram_size >> 1);
	} else {
		ASSERT(0);
	}

	/*For DFS Purpose, we remove this memory block for Dummy read/write by APDMA.*/
	pr_info("[DFS Check]DRAM SIZE:0x%llx\n", (unsigned long long)max_dram_size);
	pr_info("[DFS Check]DRAM Dummy read from:0x%llx to 0x%llx\n",
		(unsigned long long)(DFS_dummy_read_center_address-(BUFF_LEN >> 1)),
		(unsigned long long)(DFS_dummy_read_center_address+(BUFF_LEN >> 1)));
	pr_info("[DFS Check]DRAM Dummy read center address:0x%llx\n",
		(unsigned long long)DFS_dummy_read_center_address);
	pr_info("[DFS Check]High Memory start address 0x%llx\n", (unsigned long long)high_memory_phys);

	if ((DFS_dummy_read_center_address - SZ_4K) >= high_memory_phys) {
		pr_info("[DFS Check]DFS Dummy read reserved 0x%llx to 0x%llx\n",
			(unsigned long long)(DFS_dummy_read_center_address-SZ_4K),
			(unsigned long long)(DFS_dummy_read_center_address+SZ_4K));
		memblock_reserve(DFS_dummy_read_center_address-SZ_4K, (SZ_4K << 1));
		memblock_free(DFS_dummy_read_center_address-SZ_4K, (SZ_4K << 1));
		memblock_remove(DFS_dummy_read_center_address-SZ_4K, (SZ_4K << 1));
	} else {
#ifndef CONFIG_ARM_LPAE
		pr_info("[DFS Check]DFS Dummy read reserved 0x%llx to 0x%llx\n",
		(unsigned long long)(DFS_dummy_read_center_address-SZ_1M),
		(unsigned long long)(DFS_dummy_read_center_address+SZ_1M));
		memblock_reserve(DFS_dummy_read_center_address-SZ_1M, (SZ_1M << 1));
		memblock_free(DFS_dummy_read_center_address-SZ_1M, (SZ_1M << 1));
		memblock_remove(DFS_dummy_read_center_address-SZ_1M, (SZ_1M << 1));
#else
		pr_info("[DFS Check]DFS Dummy read reserved 0x%llx to 0x%llx\n",
		(unsigned long long)(DFS_dummy_read_center_address-SZ_2M),
		(unsigned long long)(DFS_dummy_read_center_address+SZ_2M));
		memblock_reserve(DFS_dummy_read_center_address-SZ_2M, (SZ_2M << 1));
		memblock_free(DFS_dummy_read_center_address-SZ_2M, (SZ_2M << 1));
		memblock_remove(DFS_dummy_read_center_address-SZ_2M, (SZ_2M << 1));
#endif
	}
}

void sync_hw_gating_value(void)
{
	unsigned int reg_val;

	reg_val = (*(unsigned int *)(0xF0004028)) & (~(0x01<<30));		 /* cha DLLFRZ=0 */
	mt_reg_sync_writel(reg_val, 0xF0004028);
	reg_val = (*(unsigned int *)(0xF0011028)) & (~(0x01<<30));		 /* chb DLLFRZ=0 */
	mt_reg_sync_writel(reg_val, 0xF0011028);

	mt_reg_sync_writel((*(unsigned int *)(0xF020e374)), 0xF0004094);   /* cha r0 */
	mt_reg_sync_writel((*(unsigned int *)(0xF020e378)), 0xF0004098);   /* cha r1 */
	mt_reg_sync_writel((*(unsigned int *)(0xF0213374)), 0xF0011094);   /* chb r0 */
	mt_reg_sync_writel((*(unsigned int *)(0xF0213378)), 0xF0011098);   /* chb r1 */

	reg_val = (*(unsigned int *)(0xF0004028)) | (0x01<<30);			/* cha DLLFRZ=1 */
	mt_reg_sync_writel(reg_val, 0xF0004028);
	reg_val = (*(unsigned int *)(0xF0011028)) | (0x01<<30);			/* chb DLLFRZ=0 */
	mt_reg_sync_writel(reg_val, 0xF0011028);
}
#endif

unsigned int is_one_pll_mode(void)
{
	int data;

	data = Reg_Readl(DRAMCAO_BASE_ADDR + (0x00a << 2));
	if (data & 0x10000) {
		/* print("It is 1-PLL mode (value = 0x%x)\n", data); */
		return 1;
	}

	/* print("It is 3-PLL mode (value = 0x%x)\n", data); */
	return 0;
}

static ssize_t complex_mem_test_show(struct device_driver *driver, char *buf)
{
	int ret;

	ret = Binning_DRAM_complex_mem_test();
	if (ret > 0)
		return snprintf(buf, PAGE_SIZE, "MEM Test all pass\n");
	else
		return snprintf(buf, PAGE_SIZE, "MEM TEST failed %d\n", ret);
}

static ssize_t complex_mem_test_store(struct device_driver *driver, const char *buf, size_t count)
{
	return count;
}

#if 0
#ifdef APDMA_TEST
static ssize_t DFS_APDMA_TEST_show(struct device_driver *driver, char *buf)
{
	dma_dummy_read_for_vcorefs(7);
	return snprintf(buf, PAGE_SIZE, "DFS APDMA Dummy Read Address 0x%x\n", (unsigned int)src_array_p);
}
static ssize_t DFS_APDMA_TEST_store(struct device_driver *driver, const char *buf, size_t count)
{
	return count;
}
#endif
#endif

#ifdef READ_DRAM_TEMP_TEST
static ssize_t read_dram_temp_show(struct device_driver *driver, char *buf)
{
	return snprintf(buf, PAGE_SIZE, "DRAM MR4 = 0x%x\n", read_dram_temperature());
}
static ssize_t read_dram_temp_store(struct device_driver *driver, const char *buf, size_t count)
{
	return count;
}
#endif

#if 0
static ssize_t read_dram_data_rate_show(struct device_driver *driver, char *buf)
{
	return snprintf(buf, PAGE_SIZE, "DRAM data rate = %d\n", get_dram_data_rate());
}
static ssize_t read_dram_data_rate_store(struct device_driver *driver, const char *buf, size_t count)
{
	return count;
}
#endif

int dfs_status;
static ssize_t dram_dfs_show(struct device_driver *driver, char *buf)
{
	dfs_status = is_one_pll_mode()?0:1;
	#ifdef CONFIG_MTK_PM
	return snprintf(buf, PAGE_SIZE, "Current DRAM DFS status : %s, emi_freq = %d\n",
	dfs_status?"High frequency":"Low frequency", mt_get_emi_freq());
	#else
	return snprintf(buf, PAGE_SIZE, "Current DRAM DFS status : %s\n", dfs_status?"High frequency":"Low frequency");
	#endif
}
static ssize_t dram_dfs_store(struct device_driver *driver, const char *buf, size_t count)
{
	char *p = (char *)buf;
	unsigned int num;

	num = kstrtoul(p, 10, (unsigned long *)&p);
	dfs_status = is_one_pll_mode()?0:1;
	if (num == dfs_status) {
		if (num == 1)
			pr_info("[DRAM DFS] Current DRAM frequency is already high freqency\n");
		else
			pr_info("[DRAM DFS] Current DRAM frequency is already low freqency\n");

		return count;
	}

	switch (num) {
	case 0:
		do_DRAM_DFS(0);
		break;
	case 1:
		do_DRAM_DFS(1);
		break;
	default:
		break;
	}

	return count;
}

DRIVER_ATTR(emi_clk_mem_test, 0664, complex_mem_test_show, complex_mem_test_store);

#if 0
#ifdef APDMA_TEST
DRIVER_ATTR(dram_dummy_read_test, 0664, DFS_APDMA_TEST_show, DFS_APDMA_TEST_store);
#endif

#ifdef READ_DRAM_TEMP_TEST
DRIVER_ATTR(read_dram_temp_test, 0664, read_dram_temp_show, read_dram_temp_store);
#endif

DRIVER_ATTR(read_dram_data_rate, 0664, read_dram_data_rate_show, read_dram_data_rate_store);
#endif

DRIVER_ATTR(dram_dfs, 0664, dram_dfs_show, dram_dfs_store);

static struct platform_driver dram_test_drv = {
	.driver = {
		.name = "emi_clk_test",
		.bus = &platform_bus_type,
		.owner = THIS_MODULE,
	}
};

/* extern char __ssram_text, _sram_start, __esram_text; */
int __init dram_test_init(void)
{
	int ret;
	struct device_node *node;

	/* unsigned char * dst = &__ssram_text; */
	/* unsigned char * src = &_sram_start; */
	/* pr_err("sram start = 0x%x, sram end = 0x%x, src=0x%x, dst=0x%x\n",&__ssram_text,&__esram_text, src, dst); */

	/* DTS version */

	/*
	node = of_find_compatible_node(NULL, NULL, "mediatek,CQDMA");
	if (node) {
		CQDMA_BASE_ADDR = of_iomap(node, 0);
		pr_warn("[DRAMC]get CQDMA_BASE_ADDR @ %p\n", CQDMA_BASE_ADDR);
	} else {
		pr_warn("[DRAMC]can't find compatible node\n");
		return -1;
	}
	*/
	node = of_find_compatible_node(NULL, NULL, "mediatek,mt8173-dramco");
	if (node) {
		DRAMCAO_BASE_ADDR = of_iomap(node, 0);
		pr_info("get DRAMCAO_BASE_ADDR @ %p\n", DRAMCAO_BASE_ADDR);
	} else {
		pr_warn("can't find DRAMC0 compatible node\n");
		return -1;
	}

	node = of_find_compatible_node(NULL, NULL, "mediatek,mt8173-ddrphy");
	if (node) {
		DDRPHY_BASE_ADDR = of_iomap(node, 0);
		pr_info("get DDRPHY_BASE_ADDR @ %p\n", DDRPHY_BASE_ADDR);
	} else {
		pr_warn("can't find DDRPHY compatible node\n");
		return -1;
	}

	node = of_find_compatible_node(NULL, NULL, "mediatek,mt8173-dramc_nao");
	if (node) {
		DRAMCNAO_BASE_ADDR = of_iomap(node, 0);
		pr_info("get DRAMCNAO_BASE_ADDR @ %p\n", DRAMCNAO_BASE_ADDR);
	} else {
		pr_warn("can't find DRAMCNAO compatible node\n");
		return -1;
	}

	ret = driver_register(&dram_test_drv.driver);
	if (ret) {
		pr_err("fail to create the dram_test driver\n");
		return ret;
	}

	ret = driver_create_file(&dram_test_drv.driver, &driver_attr_emi_clk_mem_test);
	if (ret) {
		pr_err("fail to create the emi_clk_mem_test sysfs files\n");
		return ret;
	}

#if 0
#ifdef APDMA_TEST
	ret = driver_create_file(&dram_test_drv.driver, &driver_attr_dram_dummy_read_test);
	if (ret) {
		pr_err("fail to create the DFS sysfs files\n");
		return ret;
	}
#endif

#ifdef READ_DRAM_TEMP_TEST
	ret = driver_create_file(&dram_test_drv.driver, &driver_attr_read_dram_temp_test);
	if (ret) {
		pr_err("fail to create the read dram temp sysfs files\n");
		return ret;
}
#endif

	ret = driver_create_file(&dram_test_drv.driver, &driver_attr_read_dram_data_rate);
	if (ret) {
		pr_err("fail to create the read dram data rate sysfs files\n");
		return ret;
	}
#endif

	ret = driver_create_file(&dram_test_drv.driver, &driver_attr_dram_dfs);
	if (ret) {
		pr_err("fail to create the dram dfs sysfs files\n");
		return ret;
	}

	/*
	for (dst = &__ssram_text ; dst < (unsigned char *)&__esram_text ; dst++,src++) {
		*dst = *src;
	}

	pr_info(KERN_INFO "[DRAM DFS] Register MD32 DRAM DFS Handler...\n");
	md32_ipi_registration(IPI_DFS, dram_dfs_ipi_handler, "dfs");
	*/

	return 0;
}

arch_initcall(dram_test_init);
