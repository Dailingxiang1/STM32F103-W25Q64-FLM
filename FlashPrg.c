#include <stdint.h>
#include "FlashOS.h" // 引用头文件以确保定义一致

/* 寄存器基地址定义 */
#define RCC_APB2ENR      (*((volatile uint32_t *)0x40021018))
#define GPIOA_CRL        (*((volatile uint32_t *)0x40010800))
#define GPIOA_ODR        (*((volatile uint32_t *)0x4001080C))
#define GPIOA_BSRR       (*((volatile uint32_t *)0x40010810))
#define SPI1_CR1         (*((volatile uint32_t *)0x40013000))
#define SPI1_SR          (*((volatile uint32_t *)0x40013008))
#define SPI1_DR          (*((volatile uint32_t *)0x4001300C))

#define RCC_CR       (*((volatile uint32_t *)0x40021000))
#define RCC_CFGR     (*((volatile uint32_t *)0x40021004))
#define FLASH_ACR    (*((volatile uint32_t *)0x40022000))
	
/* Cortex-M 核心寄存器用于开关中断 */
#define __disable_irq()  __asm volatile ("cpsid i" : : : "memory")

/* 你的 Flash 映射的首地址 */
#define FLASH_BASE_ADDR  0x90000000


/* --- 移除所有全局变量，确保位置无关性 --- */

/* --- 底层硬件操作 --- */

static void delay_us(uint32_t us)
{
    volatile uint32_t i;
    for (i = 0; i < us * 8; i++);
}

static uint8_t SPI_ReadWrite(uint8_t data)
{
    uint32_t timeout = 0xFFFF;
    while (!(SPI1_SR & 0x02) && timeout--); // TXE
    *((volatile uint8_t *)&SPI1_DR) = data; // 强制8位写入

    timeout = 0xFFFF;
    while (!(SPI1_SR & 0x01) && timeout--); // RXNE
    return *((volatile uint8_t *)&SPI1_DR); // 强制8位读取
}

static inline void CS_Low(void)
{
    GPIOA_BSRR = (1 << 20);    // PA4 Reset
}
static inline void CS_High(void)
{
    GPIOA_BSRR = (1 << 4);     // PA4 Set
}

/* --- W25Qxx 指令封装 --- */

static void W25Q_WaitBusy(void)
{
    uint8_t status;
    do
    {
        CS_Low();
        SPI_ReadWrite(0x05); // Read Status Register-1
        status = SPI_ReadWrite(0xFF);
        CS_High();
    }
    while (status & 0x01);   // BUSY
}

static void W25Q_WriteEnable(void)
{
    CS_Low();
    SPI_ReadWrite(0x06);
    CS_High();
}

/* --- FLM 标准接口函数 --- */

int Init(unsigned long adr, unsigned long clk, unsigned long fnc)
{
    // 关键：禁用中断，防止 SysTick 或其他中断打断 Flash 操作导致死机

    // 1. 开启时钟 (GPIOA + SPI1 + AFIO)
    RCC_APB2ENR |= (1 << 2) | (1 << 12) | (1 << 0);

    // 2. 配置引脚 (PA4:CS, PA5:SCK, PA6:MISO, PA7:MOSI)
    // 修正：掩码改为 FFFF0000 以包含 PA4
    GPIOA_CRL &= ~0xFFFF0000;
    // PA7(AF_PP)=B, PA6(IN_PU)=8, PA5(AF_PP)=B, PA4(GP_PP)=3
    GPIOA_CRL |=  0xB8B30000;

    GPIOA_ODR |= (1 << 6) | (1 << 4); // PA6 上拉, PA4 初始高

    // 3. 配置 SPI1
    SPI1_CR1 = 0;
    // BR=256分频, MSTR=1, CPOL=1, CPHA=1, SSM=1, SSI=1, SPE=1
    //SPI1_CR1 |= (0x7 << 3) | (1 << 2) | (1 << 1) | (1 << 0) | (1 << 9) | (1 << 8) | (1 << 6);
		//Bits 5:3  用于分频
    SPI1_CR1 |=   (1 << 2) | (1 << 1) | (1 << 0) | (1 << 9) | (1 << 8) | (1 << 6);   //4M SPI
    delay_us(100);
    return 0;
}

int UnInit(unsigned long fnc)
{
    SPI1_CR1 &= ~(1 << 6); // 禁用 SPI
    return 0;
}

int EraseSector(unsigned long adr)
{
    // 使用宏定义计算偏移，替代全局变量
    uint32_t offset = adr - FLASH_BASE_ADDR;

    W25Q_WaitBusy();
    W25Q_WriteEnable();

    CS_Low();
    SPI_ReadWrite(0x20); // Sector Erase (4KB)
    SPI_ReadWrite((offset >> 16) & 0xFF);
    SPI_ReadWrite((offset >> 8) & 0xFF);
    SPI_ReadWrite(offset & 0xFF);
    CS_High();

    W25Q_WaitBusy();
    return 0;
}

int ProgramPage(unsigned long adr, unsigned long sz, unsigned char *buf)
{
    uint32_t offset = adr - FLASH_BASE_ADDR;

    W25Q_WaitBusy();
    W25Q_WriteEnable();

    CS_Low();
    SPI_ReadWrite(0x02); // Page Program
    SPI_ReadWrite((offset >> 16) & 0xFF);
    SPI_ReadWrite((offset >> 8) & 0xFF);
    SPI_ReadWrite(offset & 0xFF);

    while (sz--)
    {
        SPI_ReadWrite(*buf++);
    }
    CS_High();

    W25Q_WaitBusy();
    return 0;
}

unsigned long Verify(unsigned long adr, unsigned long sz, unsigned char *buf)
{
    uint32_t offset = adr - FLASH_BASE_ADDR;
    uint8_t flash_data;
    unsigned long i; // 显式声明变量

    W25Q_WaitBusy();

    CS_Low();
    SPI_ReadWrite(0x03);
    SPI_ReadWrite((offset >> 16) & 0xFF);
    SPI_ReadWrite((offset >> 8) & 0xFF);
    SPI_ReadWrite(offset & 0xFF);

    for (i = 0; i < sz; i++)
    {
        flash_data = SPI_ReadWrite(0xFF);
        if (flash_data != buf[i])
        {
            CS_High();
            return (adr + i);
        }
    }

    CS_High();
    return (adr + sz);
}
