#include <xtos.h>

#define NR_PIX_X 1280
#define NR_PIX_Y 800
#define CHAR_HEIGHT 16
#define CHAR_WIDTH 8
#define NR_CHAR_X (NR_PIX_X / CHAR_WIDTH)
#define CSR_CRMD 0x0
#define CSR_ECFG 0x4
#define CSR_ESTAT 0x5
#define CSR_EENTRY 0xc
#define CSR_TCFG 0x41
#define CSR_TICLR 0x44
#define CSR_CRMD_IE (1UL << 2)
#define CSR_TCFG_EN (1UL << 0)
// 用于控制时钟计数器
#define CSR_TCFG_PER (1UL << 1)
#define CSR_TICLR_CLR (1UL << 0)
#define CSR_ECFG_LIE_TI (1UL << 11)
#define CSR_ESTAT_IS_TI (1UL << 11)
#define CC_FREQ 4
// 飞机由9个字符组成
#define PLANE_NR 9
// 每隔0.01秒发生一次中断
unsigned long val1 = 0xF4240;

struct plane_part {
    int dx;
    int dy;
};

// 飞机中心的位置
int plane_x = 5;
int plane_y = 5;
// 飞机的形状
struct plane_part plane_shape[] = {
    {0, -1 },   // 上
    {-2, 0 },   // 左翼
    {-1, 0 },
    {0, 0 },   // 中心
    {2, 0 },   // 右翼
    {1, 0 },
		{0, 1},  // 腰
    {-1, 2},   // 左下
    {1, 2},   // 右下
};

// 清除飞机
void erase_plane()
{
	int i;
	for (i = 0; i < PLANE_NR; i++)
	{
			erase_char(plane_x + plane_shape[i].dx, plane_y + plane_shape[i].dy);
	}
}

// 绘制飞机
void draw_plane()
{
	int i;
	for (i = 0; i < PLANE_NR; i++)
	{
		write_char('*', plane_x + plane_shape[i].dx, plane_y + plane_shape[i].dy);
	}
}

void clock_interrupt()
{
		// 清除旧飞机
		erase_plane();
		// 飞机坐标加一
		plane_x++;
		
		// 避免越界
		if (plane_x >= NR_CHAR_X - 3)
				plane_x = 3;

		// 重新绘制
		draw_plane();
}

void timer_interrupt()
{
	// 关闭循环模式
	write_csr_64(1, CSR_TCFG_PER);
	// 每次处理完时钟中断后，val1自增0x10000
	val1 += 0x10000;
	// 将val1写入寄存器
	
	write_csr_64(val1, CSR_TCFG);

	printk("hello, world.\n");
}
void do_exception()
{
	unsigned int estat;

	estat = read_csr_32(CSR_ESTAT);
	if (estat & CSR_ESTAT_IS_TI)
	{
		write_csr_32(CSR_TICLR_CLR, CSR_TICLR);
		timer_interrupt();
	}
}
void int_on()
{
	unsigned int crmd;

	crmd = read_csr_32(CSR_CRMD);
	write_csr_32(crmd | CSR_CRMD_IE, CSR_CRMD);
}
void excp_init()
{
	unsigned int val;

	val = read_cpucfg(CC_FREQ);
	write_csr_64((unsigned long)val | CSR_TCFG_EN | CSR_TCFG_PER, CSR_TCFG);
	write_csr_64((unsigned long)exception_handler, CSR_EENTRY);
	write_csr_32(CSR_ECFG_LIE_TI, CSR_ECFG);
}