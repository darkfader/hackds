/*
	blockram.cpp
	blockRAM interface program
*/

/*
 * Includes
 */

//#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>	// for libc5
#include <sys/io.h>	// for glibc

#include "mytypes.h"

#define SDK_ARM7
#include <nitro.h>

//#define SDK_ASM
//#include "mmap_global.h"
//#include "ioreg.h"


/****************************************************************************\
\****************************************************************************/


#define _outp(p,d)		outb_p(d,p)
#define _inp(p)			inb_p(p)

/*
 * Defines
 */

#define PP_PORT						0x378		// I/O port
#define ECP_ECR_OFFSET				0x402

// parallel port mode
#define ECR_MODE_STANDARD			0x00
#define ECR_MODE_BYTE				0x20
#define ECR_MODE_PP_FIFO			0x40
#define ECR_MODE_ECP_FIFO			0x60
#define ECR_MODE_EPP				0x80
#define ECR_MODE_FIFO_TEST			0xC0
#define ECR_DISABLE_nERRORINT		0x10
#define ECR_DISABLE_DMA				0x00
#define ECR_DISABLE_SVCINT			0x04

// PP_PORT+1
#define PP_BUSYn					0x80
#define PP_ACK						0x40
#define PP_PAPER_OUT				0x20
#define PP_SELECT_IN				0x10
#define PP_ERROR					0x08

// PP_PORT+2
#define PP_ENABLE_BIDIRECTIONAL		0x20
#define PP_ENABLE_ACK_IRQ			0x10
#define PP_SELECT_PRINTERn			0x08
#define PP_INIT_PRINTER				0x04
#define PP_AUTO_LINEFEEDn			0x02
#define PP_STROBEn					0x01

/*
 * 
 */
unsigned int control = PP_INIT_PRINTER;

/*
 * InitPort
 */
int InitPort()
{
    /*OSVERSIONINFO osvi;
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    GetVersionEx(&osvi);

    if (osvi.dwPlatformId == VER_PLATFORM_WIN32_NT)
    {
		HANDLE h = CreateFile("\\\\.\\giveio", GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		CloseHandle(h);
		if (h == INVALID_HANDLE_VALUE) { fprintf(stderr, "Couldn't open GiveIo!\n"); return -1; }
	}
*/
	iopl(3);

//printf("ioperm...\n");
	//ioperm(PP_PORT, 3, 1);
	//ioperm(PP_PORT+ECP_ECR_OFFSET, 1, 1);

//printf("_outp...\n");
	_outp(PP_PORT+ECP_ECR_OFFSET, ECR_MODE_STANDARD | ECR_DISABLE_nERRORINT | ECR_DISABLE_DMA | ECR_DISABLE_SVCINT);	// compatibility mode
	
	return 0;
}

/****************************************************************************\
\****************************************************************************/


inline unsigned char PP_GetNibble()
{
	return (_inp(PP_PORT+1) ^ 0x80) >> 4;
}

inline void PP_Update()
{
	_outp(PP_PORT+2, control);
}

inline void ClockLow()
{
	control |= PP_STROBEn;
}

inline void ClockHigh()
{
	control &=~ PP_STROBEn;
}

inline void LowNibbleOrByte()
{
	control |= PP_AUTO_LINEFEEDn;
}

inline void HighNibbleOrByte()
{
	control &=~ PP_AUTO_LINEFEEDn;
}

inline void WriteEnable()
{
	control &=~ PP_SELECT_PRINTERn;
}

inline void WriteDisable()
{
	control |= PP_SELECT_PRINTERn;
}

inline void SetAddressEnable()
{
	control |= PP_INIT_PRINTER;
}

inline void SetAddressDisable()
{
	control &=~ PP_INIT_PRINTER;
}


/****************************************************************************\
\****************************************************************************/



void BramAddress(unsigned int address, bool write)
{
	ClockHigh();
	if (write) WriteEnable(); else WriteDisable();

	address -= 1;

	_outp(PP_PORT, address & 0xFF);
	LowNibbleOrByte(); PP_Update();
	SetAddressEnable(); PP_Update();
	SetAddressDisable(); PP_Update();

	_outp(PP_PORT, address >> 8);
	HighNibbleOrByte(); PP_Update();
	SetAddressEnable(); PP_Update();
	SetAddressDisable(); PP_Update();
}


void Clock()
{
	ClockLow(); PP_Update();
	ClockHigh(); PP_Update();
}


void BramWrite(unsigned char *data, unsigned int address, unsigned int length)
{
	BramAddress(address, true);

	for (unsigned int i=0; i<length; i++)
	{
		_outp(PP_PORT, data[i]);
		Clock();
	}
}

void BramRead(unsigned char *data, unsigned int address, unsigned int length)
{
	BramAddress(address, false);

	for (unsigned int i=0; i<length; i++)
	{
		LowNibbleOrByte();
		Clock();
		unsigned char low = PP_GetNibble();	
		HighNibbleOrByte(); PP_Update();
		unsigned char high = PP_GetNibble();
		data[i] = high<<4 | low;
	}
}


/****************************************************************************\
\****************************************************************************/


void AddressCommand(unsigned int address, unsigned char command)
{
	BramWrite((unsigned char *)&address, 9*0x200 + 0x1F8, sizeof(address));
	BramWrite((unsigned char *)&command, 9*0x200 + 0x1F0, sizeof(command));
}



u8 Read8(unsigned int address)
{
	AddressCommand(address, 1);
	u8 data;
	BramRead((u8 *)&data, 9*0x200 + 0x1FC, sizeof(data));
	return data;
}

u16 Read16(unsigned int address)
{
	AddressCommand(address, 2);
	u16 data;
	BramRead((u8 *)&data, 9*0x200 + 0x1FC, sizeof(data));
	return data;
}

u32 Read32(unsigned int address)
{
	AddressCommand(address, 3);
	unsigned int data;
	BramRead((u8 *)&data, 9*0x200 + 0x1FC, sizeof(data));
	return data;
}

u64 Read64(unsigned int address)
{
	AddressCommand(address, 3);
	unsigned int data0;
	BramRead((u8 *)&data0, 9*0x200 + 0x1FC, sizeof(data0));

	AddressCommand(address + 4, 3);
	unsigned int data1;
	BramRead((u8 *)&data1, 9*0x200 + 0x1FC, sizeof(data1));

	return (u64)data1<<32 | data0;
}

void Write8(unsigned int address, u8 data)
{
	BramWrite((u8 *)&data, 9*0x200 + 0x1FC, sizeof(data));
	AddressCommand(address, 4);
}

void Write16(unsigned int address, u16 data)
{
	BramWrite((u8 *)&data, 9*0x200 + 0x1FC, sizeof(data));
	AddressCommand(address, 5);
}

void Write32(unsigned int address, u32 data)
{
	BramWrite((u8 *)&data, 9*0x200 + 0x1FC, sizeof(data));
	AddressCommand(address, 6);
}

void Write64(unsigned int address, u64 data)
{
	Write32(address, data & 0xFFFFFFFF);
	Write32(address + 4, data >> 32);
}



void Write(unsigned char *data, unsigned int address, unsigned int length)
{
	for (unsigned int i=0; i<length; i+=4)
	{
		Write32(address + i, *(u32 *)(data + i));
	}
}

void Read(unsigned char *data, unsigned int address, unsigned int length)
{
	for (unsigned int i=0; i<length; i+=4)
	{
		*(u32 *)(data + i) = Read32(address + i);
	}
}


/****************************************************************************\
\****************************************************************************/




bool IsResponsive()
{
	return (Read16(0x08000000) != Read16(0x08000002));
}

void WaitResponsive()
{
	printf("Waiting for connection...\n");
	while (!IsResponsive()) usleep(10000);
	printf("Connected.\n");
}

void PrintResponsive()
{
	if (IsResponsive())
		printf("responsive.\n");
	else
		printf("NOT RESPONDING!\n");
}




void Call(unsigned int address)
{
	unsigned int registers[16];
	registers[0] = 0x00000000;
	registers[1] = 0x00000000;
	registers[2] = 0x00000000;
	registers[3] = 0x00000000;
	registers[4] = 0x00000000;
	registers[5] = 0x00000000;
	registers[6] = 0x00000000;
	registers[7] = 0x00000000;
	registers[8] = 0x00000000;
	registers[9] = 0x00000000;
	registers[10] = 0x00000000;
	registers[11] = 0x00000000;

	registers[12] = 0x00000000;
	registers[13] = 0x00000000;
	registers[14] = 0x00000000;
	registers[15] = 0x00000000;
	for (unsigned int i=0; i<16; i++) Write32(0x02001000 + i*4, registers[i]);

	AddressCommand(address, 7);
	//Sleep(10);
	usleep(10000);

	for (unsigned int i=0; i<16; i++)
	{
		printf("r%2d = 0x%08X\n", i, Read32(0x02001000 + i*4));
	}
}



//void SVC_CpuSet(const void *srcp, void *destp, u32 dmaCntData);
//void SVC_CpuSetFast(const void *srcp, void *destp, u32 dmaCntData);
//	Write32(0, 0xAAAAAAAA);
//	for (unsigned int i=0x00000000; i<0x00000010; i+=0x4)
//	{
//		printf("%08X %08X\n", i, Read32(i));
//	}


//	for (int i=0; i<0x4000; i+=4)
//	{
//		// the lower bits are inaccurate, so just get it four times :)
//		u32 a = MidiKey2Freq((WaveData *)(i-4), 180-12, 0) * 2;
//		u32 b = MidiKey2Freq((WaveData *)(i-3), 180-12, 0) * 2;
//		u32 c = MidiKey2Freq((WaveData *)(i-2), 180-12, 0) * 2;
//		u32 d = MidiKey2Freq((WaveData *)(i-1), 180-12, 0) * 2;
//		printf("0x%02X%02X%02X%02X,\n", a>>24, d>>24, c>>24, b>>24);
//	}


//             void *srcp     Source address
//             void *destp    Destination address
//  MI_UnpackBitsParam *paramp   MI_UnpackBitsParam struct data address

void SWI(unsigned int number)
{
	unsigned int registers[16];
	registers[0] = 0x02100000;
	registers[1] = 0x02100000;
	registers[2] = 0x02100000;
	registers[3] = 0x02100000;
	registers[4] = 0x00000000;
	registers[5] = 0x00000000;
	registers[6] = 0x00000000;
	registers[7] = 0x00000000;
	registers[8] = 0x00000000;
	registers[9] = 0x00000000;
	registers[10] = 0x00000000;
	registers[11] = 0x00000000;

	registers[12] = 0x00000000;
	registers[13] = 0x00000000;
	registers[14] = 0x00000000;
	registers[15] = 0x00000000;
	for (unsigned int i=0; i<16; i++) Write32(0x02001000 + i*4, registers[i]);

	AddressCommand(number, 8);
	while (!IsResponsive());
	//Sleep(10);

	for (unsigned int i=0; i<16; i++)
	{
		printf("r%2d = 0x%08X\n", i, Read32(0x02001000 + i*4));
	}
}



unsigned int Read32_2(unsigned int address)		// through SWI 0xE
{
	unsigned int registers[16];
	registers[0] = 0x00000000;
	registers[1] = address;
	registers[2] = 0x00000002;
	for (unsigned int i=0; i<3; i++) Write32(0x02001000 + i*4, registers[i]);
	AddressCommand(0x0E, 8);
	while (!IsResponsive());
	unsigned short data0 = Read16(0x02001000 + 3*4);

	registers[1] = address + 2;
	registers[2] = 0x00000002;
	for (unsigned int i=0; i<3; i++) Write32(0x02001000 + i*4, registers[i]);
	AddressCommand(0x0E, 8);
	while (!IsResponsive());
	unsigned short data1 = Read16(0x02001000 + 3*4);

	return data1<<16 | data0;
}



#define u64_split(reg)		(unsigned int)(reg>>32), (unsigned int)(reg&0xFFFFFFFF)



/****************************************************************************\
\****************************************************************************/



char *pxi_tagnames[] =
{
	"EX",			// Extension format
	"USER_0",		// for application programmer, use it free
	"USER_1",		// for application programmer, use it free
	"SYSTEM",		// SDK inner usage
	"NVRAM",		// NVRAM
	"RTC",			// RTC
	"TOUCHPANEL",	// Touch Panel
	"SOUND",		// Sound
	"PM",			// Power Management
	"MIC",			// Microphone
	"WM",			// Wireless Manager
};


void TestPXI()
{
	//Write32(REG_MAINPINTF_ADDR, REG_PXI_MAINPINTF_A7STATUS_MASK);
	//printf("%08X\n", Read8(REG_MAINPINTF_ADDR));

//#define REG_PXI_MAINPINTF_A7STATUS_SHIFT                   8
//#define REG_PXI_MAINPINTF_A7STATUS_SIZE                    4
//#define REG_PXI_MAINPINTF_A7STATUS_MASK                    0x0f00
//
//#define REG_PXI_MAINPINTF_A9STATUS_SHIFT                   0
//#define REG_PXI_MAINPINTF_A9STATUS_SIZE                    4
//#define REG_PXI_MAINPINTF_A9STATUS_MASK                    0x000f

//.text:0000000C                 LDR     R12, =PXI_InitFifo
//.text:00000010                 BX      R12

	
	//reg_PXI_MAINPINTF = REG_PXI_MAINPINTF_IREQ_MASK;
	//printf("reg_PXI_MAINPINTF = %04X\n", (int)reg_PXI_MAINPINTF);		// 




	reg_PXI_MAINP_FIFO_CNT = REG_PXI_MAINP_FIFO_CNT_SEND_CL_MASK;		// clear
	reg_PXI_MAINP_FIFO_CNT = REG_PXI_MAINP_FIFO_CNT_E_MASK;				// enable
	printf("reg_PXI_MAINP_FIFO_CNT = %04X\n", (int)reg_PXI_MAINP_FIFO_CNT);		// 
	
//	if (reg_PXI_MAINP_FIFO_CNT & REG_PXI_MAINP_FIFO_CNT_ERR_MASK)
//	{
//		reg_PXI_MAINP_FIFO_CNT = REG_PXI_MAINP_FIFO_CNT_E_MASK | REG_PXI_MAINP_FIFO_CNT_ERR_MASK | REG_PXI_MAINP_FIFO_CNT_RECV_RI_MASK | REG_PXI_MAINP_FIFO_CNT_SEND_CL_MASK | REG_PXI_MAINP_FIFO_CNT_SEND_TI_MASK;	//0xC40C;
//	}


	for (int i=0; i<1000; i++)
	{
	//reg_OS_IE = 0;
	//reg_OS_IME = 0;
		reg_PXI_MAINPINTF = rand()<<16 ^ rand();

		PXIFifoMessage msg;
		//msg.e.tag = PXI_FIFO_TAG_SOUND;
		//msg.e.tag = PXI_FIFO_TAG_NVRAM;
		//msg.e.tag = PXI_FIFO_TAG_MIC;
		//msg.e.tag = PXI_FIFO_TAG_RTC;
		//msg.e.tag = PXI_FIFO_TAG_PM;
		msg.e.tag = rand();
		msg.e.err = rand();	// 1 bit
		msg.e.data = rand() ^ rand()<<16;	// 26 bits


		if (!(reg_PXI_MAINP_FIFO_CNT & REG_PXI_MAINP_FIFO_CNT_SEND_FULL_MASK))
		{
			reg_PXI_SEND_FIFO = (rand()<<16) ^ rand();	//msg.raw;
			if (reg_PXI_MAINP_FIFO_CNT & REG_PXI_MAINP_FIFO_CNT_SEND_EMP_MASK)
				printf("send error\n");
			else
				printf(".");
		}
		else
		{
			printf("cannot send\n");
			//reg_PXI_MAINP_FIFO_CNT |= REG_PXI_MAINP_FIFO_CNT_SEND_CL_MASK;		// clear
			reg_PXI_MAINP_FIFO_CNT = reg_PXI_MAINP_FIFO_CNT ^ 0x8;
		}

//	printf("%04X\n", Read16(0x08000400));


//		for (int i=0; i<3; i++)
//		{
//			printf("%04X\n", (int)reg_PXI_MAINP_FIFO_CNT);		// 
//			usleep(100000);
//		}

		usleep(1000);


		if ((reg_PXI_MAINP_FIFO_CNT & REG_PXI_MAINP_FIFO_CNT_SEND_TI_MASK))
		{
			printf("TI\n");
		}

		if (!(reg_PXI_MAINP_FIFO_CNT & REG_PXI_MAINP_FIFO_CNT_RECV_EMP_MASK))
		//if ((reg_PXI_MAINP_FIFO_CNT & REG_PXI_MAINP_FIFO_CNT_RECV_RI_MASK))
		{
			msg.raw = reg_PXI_RECV_FIFO;
	
			printf("tag %s\n", pxi_tagnames[msg.e.tag]);
			printf("err %d\n", msg.e.err);
			printf("data %04X\n", msg.e.data);
		}
	}

}


/*
 * WriteHeader
 */
void WriteHeader()
{
	FILE *f = fopen("header.bin", "rb");
	if (!f) exit(1);
	unsigned char data[512];
	fread(data, 1, 512, f);
	BramWrite(data, 0, 512);
	fclose(f);
}

/*
 * WriteARM7
 */
void WriteARM7()
{
	//f = fopen("F-ZERO.gba", "rb");
	FILE *f = fopen("arm7.bin", "rb");
	if (!f) exit(1);
	unsigned char data[512];
	fread(data, 1, 512, f);
	BramWrite(data, 9*0x200, 512-0x10);
	fclose(f);
}

/*
 * PrintARM7
 */
void PrintARM7()
{
	unsigned char data[512];
	BramRead(data, 9*0x200, 512);
	for (int i=0; i<512/32; i++)
	{
		for (int j=0; j<32; j++) printf("%02X ", data[i*32 + j]);
		printf("\n");
	}
}


/****************************************************************************\
\****************************************************************************/


/*
 * main
 */
int main(int argc, char *argv[])
{
	printf("Nintendo DS testing tool by Rafael Vuijk (aka DarkFader)\n");
	printf("http://darkfader.net/ds/\n");

	if (InitPort() < 0) return -1;

	BramAddress(0, false);


/*
	{
		unsigned char data[512];
		for (int i=0; i<512; i++) data[i] = i;
		BramWrite(data, 9*0x200, 512);
	}

	{
		unsigned char data = 0xAA;
		//BramWrite(&data, 9*0x200 + 0x1FE, 1);
	}
*/


	

/*
	BramRead(bram_data, 0, 10*0x200);

	for (int i=0; i<10*0x200/32; i++)
	{
		for (int j=0; j<32; j++) printf("%02X ", bram_data[i*32 + j]);
		printf("\n");
	}
*/

	//PrintARM7();
	WriteHeader();
	WriteARM7();


//return 0;

	//printf("%08X\n", Read16(0x08000000));

	WaitResponsive();


	/*{
		FILE *f = fopen("00000000_2.bin", "wb");
		if (!f) exit(1);
		for (unsigned int i=0x00000000; i<0x00010000; i+=0x4)
		{
			//printf("%08X %08X\n", i, Read32(i));
			unsigned int data = Read32(i);
			fwrite(&data, 1, 4, f);
		}
		fclose(f);
	}

	{
		FILE *f = fopen("FFFF0000.bin", "wb");	
		if (!f) exit(1);
		for (unsigned int i=0xFFFF0000; i<0xFFFF8000; i+=0x4)
		{
			//printf("%08X %08X\n", i, Read32(i));
			unsigned int data = Read32(i);
			fwrite(&data, 1, 4, f);
		}
		fclose(f);
	}*/


	//reg_OS_PROTADR
	//reg_MI_EXMEMCNT = reg_MI_EXMEMCNT &~ 0x2000;


//	Write32(0, 0xDEADC0DE);
//	Write32(4, 0xFFFFFFFF);
//	printf("%08X\n", Read32(0));
//	printf("%08X\n", Read32(4));
	

// ARM7:
//04100000 8018C008		PXI fifo
// 
//mirrors:
//02072000 00000001
//02472000 00000001
//02872000 00000001
//02C72000 00000001

//00000000 FFFFFFFF
//00001000 FFFFFFFF
//00002000 FFFFFFFF
//00003000 FFFFFFFF



//0x0001fffc
//	Write32(0x1204, 0xFFFFFFFF);
//// 0x1204
//	reg_OS_PROTADR = 0xFFFFFFFF;
//	printf("%08X\n", (int)reg_OS_PROTADR);

	printf("%08X\n", (int)reg_MI_EXMEMCNT_L);
	reg_MI_EXMEMCNT_L = reg_MI_EXMEMCNT_L | REG_MI_EXMEMCNT_L_CP_MASK;
	
	//Write32(REG_PROTADR_ADDR, 0);

//  |  |  |  | SIO_RECV_ENABLE | /* SIO_FIFO_ENABLE |*/ SIO_IF_ENABLE;


	reg_
	reg_EXI_SIOCNT_UART = REG_EXI_SIOCNT_UART_MD1_MASK | REG_EXI_SIOCNT_UART_MD0_MASK | 3 << REG_EXI_SIOCNT_UART_BAUD_SHIFT | REG_EXI_SIOCNT_UART_CTS_MASK | REG_EXI_SIOCNT_UART_DATALEN_MASK | REG_EXI_SIOCNT_UART_SENDEF_MASK | REG_EXI_SIOCNT_UART_RECVEF_MASK;


	reg_EXI_SIOCNT = 0xFFFF;
	printf("%08X\n", (int)reg_EXI_SIOCNT);
	

	//if (reg_EXI_SIOCNT_UART & REG_EXI_SIOCNT_H_TFEMP_MASK
	{
		rSIODATA8 = 0xAA;
	}

	if (!(rSIOCNT & SIO_RECV_DATA_EMPTY))
	{
		printf("rSIODATA8=%08X\n", (int)rSIODATA8);
	}


//	TestPXI();



//	{
//		FILE *f = fopen("FFFF0000_2.bin", "wb");
//		if (!f) exit(1);
//		for (unsigned int i=0xFFFF0000; i<0xFFFFFFFC; i+=4)
//		{
//			unsigned int data = Read32_2(i);
//			printf("%08X ", data);
//			fwrite(&data, 1, sizeof(data), f);
//		}
//		fclose(f);
//	}

/*
	MI_UnpackBitsParam ubp;
    ubp.srcNum = 2;                 // Source data. Number of bytes.
    ubp.srcBitNum = 1;            // 1 source data. Number of bits.
    ubp.destBitNum = 32;           // 1 destination data. Number of bits.
    ubp.destOffset = 0;	       // Number to add to source data
    ubp.destOffset0_on = 1;       // Flag for whether to add an offset to 0 data.

	Write((unsigned char *)&ubp, 0x02020000, sizeof(ubp));
	//Read((unsigned char *)&ubp, 0x02020000, sizeof(ubp));
	//printf("%d\n", ubp.srcNum);
*/

/*
	Write32(0x02100000, 0xAAAAAAAA);
	Write32(0x02100004, 0xAAAAAAAA);
	Write32(0x02100008, 0xAAAAAAAA);
	Write32(0x0210000C, 0xAAAAAAAA);
//	printf("%08X\n", Read32(0x02100000));
//	printf("%08X\n", Read32(0x02100004));
	SWI(0);
	printf("%08X\n", Read32(0x02100000));
	printf("%08X\n", Read32(0x02100004));
	printf("%08X\n", Read32(0x02100008));
	printf("%08X\n", Read32(0x0210000C));
*/





	PrintResponsive();


	BramAddress(0, false);

	return 0;
}
