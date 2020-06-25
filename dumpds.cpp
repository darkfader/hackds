/*
	Nintendo DS dumper by Rafael Vuijk (aka DarkFader)
	http://darkfader.net/ds/

	Compile-and-run:
		g++ -O2 -o dumpds dumpds.cpp -lioperm && nice dumpds
*/

/*
 * Includes
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/io.h>
#include <time.h>

/*
 * Defines
 */
 
#define _outp(p,d)		outb_p(d,p)
#define _inp(p)			inb_p(p)

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
	iopl(3);
	_outp(PP_PORT+ECP_ECR_OFFSET, ECR_MODE_STANDARD | ECR_DISABLE_nERRORINT | ECR_DISABLE_DMA | ECR_DISABLE_SVCINT);	// compatibility mode
	return 0;
}

inline unsigned char PP_GetNibble()
{
	return (_inp(PP_PORT+1) ^ 0x80) >> 4;
}

inline void PP_Update()
{
	_outp(PP_PORT+2, control);
}

inline void PP_Input()
{
	control |= PP_INIT_PRINTER;
	control &=~ PP_AUTO_LINEFEEDn;		// prevent reset
}

inline void PP_Output()
{
	control &=~ PP_INIT_PRINTER;
	control &=~ PP_AUTO_LINEFEEDn;		// prevent reset
}

inline void PP_Reset()
{
	control &=~ PP_INIT_PRINTER;
	control |= PP_AUTO_LINEFEEDn;
}

inline void PP_ChipSelect()
{
	control |= PP_SELECT_PRINTERn;
}

inline void PP_ChipDeselect()
{
	control &=~ PP_SELECT_PRINTERn;
}

inline void PP_ClockHigh()
{
	control &=~ PP_STROBEn;
}

inline void PP_ClockLow()
{
	control |= PP_STROBEn;
}

inline void PP_LowNibble()
{
	control |= PP_AUTO_LINEFEEDn;
}

inline void PP_HighNibble()
{
	control &=~ PP_AUTO_LINEFEEDn;
}

/*
 * WriteByte
 */
void WriteByte(unsigned char data)
{
	_outp(PP_PORT, data);
	PP_ClockLow();
	PP_Output();
	PP_Update();
	PP_ClockHigh();
	PP_Input();
	PP_Update();
}

/*
 * ReadByte
 */
unsigned char ReadByte()
{
	PP_Input();
	PP_ClockLow();
	PP_Update();
	PP_ClockHigh();
	PP_LowNibble();
	PP_Update();
	unsigned char low = PP_GetNibble();
	PP_HighNibble();
	PP_Update();
	unsigned char high = PP_GetNibble();
	return high<<4 | low;
}

/*
 * data
 */
unsigned char cartridgeId[] = { 0xC2,0x0F,0x00,0x00 };
unsigned char hdrCommand[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };


struct Command
{
	unsigned char command[8];
	int dataLength;
};

Command commands[] =
{
	#include "sm64_commands.txt"
};

#define ALL_COMMANDS		(sizeof(commands) / sizeof(commands[0]))
#define LEADING_COMMANDS	(12+5)		// includes logo

unsigned char allDataXor[(ALL_COMMANDS - LEADING_COMMANDS) * 512];

/*
 * DoCommand
 */
void DoCommand(unsigned char *command, unsigned char *commandXor, int dataLength, unsigned char *dataXor, unsigned int dataXorAnd, FILE *fDataOut, unsigned char *dataOut)
{
	PP_ChipSelect();
	PP_Update();

	// command
	for (int i=0; i<8; i++)
	{
		unsigned data = command[i];
		if (commandXor) data ^= commandXor[i];
		WriteByte(data);
	}

	// data
	for (int i=0; i<dataLength; i++)
	{
		unsigned char data = ReadByte();
		if (dataXor) data ^= dataXor[i & dataXorAnd];
		if (dataOut) dataOut[i] = data;
		if (fDataOut) fputc(data, fDataOut);
	}

	PP_ChipDeselect();
	PP_Update();
}

/*
 * ResetCartridge
 */
void ResetCartridge()
{
	PP_ClockHigh();
	PP_Reset();
	PP_Update();
}

/*
 * LeadingCommands
 */
void LeadingCommands()
{
	for (int c=0; c<LEADING_COMMANDS; c++)
	{
		DoCommand(commands[c].command, 0, commands[c].dataLength, 0, 0, 0, 0);
	}
}

/*
 * DumpXor
 */
void DumpXor()
{
	ResetCartridge();
	LeadingCommands();

	unsigned char commandXor[8] = { 0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
	unsigned char *dataXor = allDataXor;

	for (int c=LEADING_COMMANDS; c<ALL_COMMANDS; c++)
	{
		DoCommand(commands[c].command, commandXor, commands[c].dataLength, cartridgeId, 4-1, 0, dataXor);
		dataXor += commands[c].dataLength;
	}
}

/*
 * DumpData
 */
void DumpData()
{
	FILE *fdata = fopen("dumped.nds", "wb");
	if (!fdata) exit(1);

	ResetCartridge();

	// read unencrypted header
	unsigned char header[0x1000];
	DoCommand(hdrCommand, 0, 0x1000, 0, 0, fdata, header);

	// fill unknown area
	for (unsigned int i=0; i<0x3000; i++) fputc(0x00, fdata);

	// fill first part of ARM9 code
	for (unsigned int i=0; i<0x4000; i++) fputc(0xAA, fdata);

	// read type-2 encrypted data
	unsigned int address = 0x8000;
	unsigned int addressEnd = (1 << header[0x14]) * (1024 * 1024 / 8);

	while (address < addressEnd)
	{
		unsigned char *dataXor = allDataXor;	
		unsigned int addressCapture = 0x8000;
		ResetCartridge();
		LeadingCommands();

		for (int c=LEADING_COMMANDS; c<ALL_COMMANDS; c++)
		{
			unsigned addressXor = addressCapture ^ address;
			unsigned char commandXor[8] =
			{
				0x00,
				addressXor>>24,
				addressXor>>16,
				addressXor>>8,
				addressXor>>0,
				0x00,
				0x00,
				0x00,
			};

			DoCommand(commands[c].command, commandXor, commands[c].dataLength, dataXor, (unsigned int)-1, fdata, 0);
			address += commands[c].dataLength;
			addressCapture += commands[c].dataLength;
			dataXor += commands[c].dataLength;
			if (address >= addressEnd) break;
		}

		printf("\r%d%%", address * 100 / addressEnd);
		fflush(stdout);
	}
	printf("\n");
	printf("Don't forget to make another few dumps and compare.\n");
	printf("After that, replace 0x4000 to 0x8000 with RAM contents.\n");

	fclose(fdata);
}

/*
 * main
 */
int main(int argc, char *argv[])
{
	printf("Nintendo DS dumper by Rafael Vuijk (aka DarkFader)\n");
	printf("http://darkfader.net/ds/\n");

	if (InitPort() < 0) return -1;

	DumpXor();
	DumpData();

	return 0;
}
