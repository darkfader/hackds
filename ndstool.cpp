/*
	Nintendo DS rom tool
	by Rafael Vuijk (aka DarkFader)

	g++ -o ndstool ndstool.cpp && ndstool -i Metroid.Prime.Hunters.First.Hunt-DF.nds
	
	v1.00 - shows info, fixes CRC, extracts files
	v1.01 - extracts ARM7/ARM9 code, more header info
	v1.02 - added maker codes, logo CRC, cartridge code
*/

/*
 * Includes
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

/*
 * Defines
 */

#define VER			"1.02"
#define MAX_PATH	2048

/*
 * data
 */
const unsigned short crc16tab[] =
{
	0x0000, 0xC0C1, 0xC181, 0x0140, 0xC301, 0x03C0, 0x0280, 0xC241,
	0xC601, 0x06C0, 0x0780, 0xC741, 0x0500, 0xC5C1, 0xC481, 0x0440,
	0xCC01, 0x0CC0, 0x0D80, 0xCD41, 0x0F00, 0xCFC1, 0xCE81, 0x0E40,
	0x0A00, 0xCAC1, 0xCB81, 0x0B40, 0xC901, 0x09C0, 0x0880, 0xC841,
	0xD801, 0x18C0, 0x1980, 0xD941, 0x1B00, 0xDBC1, 0xDA81, 0x1A40,
	0x1E00, 0xDEC1, 0xDF81, 0x1F40, 0xDD01, 0x1DC0, 0x1C80, 0xDC41,
	0x1400, 0xD4C1, 0xD581, 0x1540, 0xD701, 0x17C0, 0x1680, 0xD641,
	0xD201, 0x12C0, 0x1380, 0xD341, 0x1100, 0xD1C1, 0xD081, 0x1040,
	0xF001, 0x30C0, 0x3180, 0xF141, 0x3300, 0xF3C1, 0xF281, 0x3240,
	0x3600, 0xF6C1, 0xF781, 0x3740, 0xF501, 0x35C0, 0x3480, 0xF441,
	0x3C00, 0xFCC1, 0xFD81, 0x3D40, 0xFF01, 0x3FC0, 0x3E80, 0xFE41,
	0xFA01, 0x3AC0, 0x3B80, 0xFB41, 0x3900, 0xF9C1, 0xF881, 0x3840,
	0x2800, 0xE8C1, 0xE981, 0x2940, 0xEB01, 0x2BC0, 0x2A80, 0xEA41,
	0xEE01, 0x2EC0, 0x2F80, 0xEF41, 0x2D00, 0xEDC1, 0xEC81, 0x2C40,
	0xE401, 0x24C0, 0x2580, 0xE541, 0x2700, 0xE7C1, 0xE681, 0x2640,
	0x2200, 0xE2C1, 0xE381, 0x2340, 0xE101, 0x21C0, 0x2080, 0xE041,
	0xA001, 0x60C0, 0x6180, 0xA141, 0x6300, 0xA3C1, 0xA281, 0x6240,
	0x6600, 0xA6C1, 0xA781, 0x6740, 0xA501, 0x65C0, 0x6480, 0xA441,
	0x6C00, 0xACC1, 0xAD81, 0x6D40, 0xAF01, 0x6FC0, 0x6E80, 0xAE41,
	0xAA01, 0x6AC0, 0x6B80, 0xAB41, 0x6900, 0xA9C1, 0xA881, 0x6840,
	0x7800, 0xB8C1, 0xB981, 0x7940, 0xBB01, 0x7BC0, 0x7A80, 0xBA41,
	0xBE01, 0x7EC0, 0x7F80, 0xBF41, 0x7D00, 0xBDC1, 0xBC81, 0x7C40,
	0xB401, 0x74C0, 0x7580, 0xB541, 0x7700, 0xB7C1, 0xB681, 0x7640,
	0x7200, 0xB2C1, 0xB381, 0x7340, 0xB101, 0x71C0, 0x7080, 0xB041,
	0x5000, 0x90C1, 0x9181, 0x5140, 0x9301, 0x53C0, 0x5280, 0x9241,
	0x9601, 0x56C0, 0x5780, 0x9741, 0x5500, 0x95C1, 0x9481, 0x5440,
	0x9C01, 0x5CC0, 0x5D80, 0x9D41, 0x5F00, 0x9FC1, 0x9E81, 0x5E40,
	0x5A00, 0x9AC1, 0x9B81, 0x5B40, 0x9901, 0x59C0, 0x5880, 0x9841,
	0x8801, 0x48C0, 0x4980, 0x8941, 0x4B00, 0x8BC1, 0x8A81, 0x4A40,
	0x4E00, 0x8EC1, 0x8F81, 0x4F40, 0x8D01, 0x4DC0, 0x4C80, 0x8C41,
	0x4400, 0x84C1, 0x8581, 0x4540, 0x8701, 0x47C0, 0x4680, 0x8641,
	0x8201, 0x42C0, 0x4380, 0x8341, 0x4100, 0x81C1, 0x8081, 0x4040
};

struct Country
{
	char countrycode;
	char *name;
};

Country countries[] =
{
	'J', "JPN",
	'E', "USA",
	'P', "EUR",
	'D', "NOE",
	'F', "NOE",
	'I', "ITA",
	'S', "SPA",
	'H', "HOL",
	'K', "KOR",
	'X', "EUU",
};

struct Maker
{
	char *makercode;
	char *name;
};

Maker makers[] =
{
	"01", "Nintendo",
	"02", "Rocket Games, Ajinomoto",
	"03", "Imagineer-Zoom",
	"04", "Gray Matter?",
	"05", "Zamuse",
	"06", "Falcom",
	"07", "Enix?",
	"08", "Capcom",
	"09", "Hot B Co.",
	"0A", "Jaleco",
	"0B", "Coconuts Japan",
	"0C", "Coconuts Japan/G.X.Media",
	"0D", "Micronet?",
	"0E", "Technos",
	"0F", "Mebio Software",
	"0G", "Shouei System",
	"0H", "Starfish",
	"0J", "Mitsui Fudosan/Dentsu",
	"0L", "Warashi Inc.",
	"0N", "Nowpro",
	"0P", "Game Village",
	"10", "?????????????",
	"12", "Infocom",
	"13", "Electronic Arts Japan",
	"15", "Cobra Team",
	"16", "Human/Field",
	"17", "KOEI",
	"18", "Hudson Soft",
	"19", "S.C.P.",
	"1A", "Yanoman",
	"1C", "Tecmo Products",
	"1D", "Japan Glary Business",
	"1E", "Forum/OpenSystem",
	"1F", "Virgin Games",
	"1G", "SMDE",
	"1J", "Daikokudenki",
	"1P", "Creatures Inc.",
	"1Q", "TDK Deep Impresion",
	"20", "Destination Software, KSS",
	"21", "Sunsoft/Tokai Engineering??",
	"22", "POW, VR 1 Japan??",
	"23", "Micro World",
	"25", "San-X",
	"26", "Enix",
	"27", "Loriciel/Electro Brain",
	"28", "Kemco Japan",
	"29", "Seta",
	"2A", "Culture Brain",
	"2C", "Palsoft",
	"2D", "Visit Co.,Ltd.",
	"2E", "Intec",
	"2F", "System Sacom",
	"2G", "Poppo",
	"2H", "Ubisoft Japan",
	"2J", "Media Works",
	"2K", "NEC InterChannel",
	"2L", "Tam",
	"2M", "Jordan",
	"2N", "Smilesoft ???, Rocket ???",
	"2Q", "Mediakite",
	"30", "Viacom",
	"31", "Carrozzeria",
	"32", "Dynamic",
	//"33", "NOT A COMPANY!",
	"34", "Magifact",
	"35", "Hect",
	"36", "Codemasters",
	"37", "Taito/GAGA Communications",
	"38", "Laguna",
	"39", "Telstar Fun & Games, Event/Taito",
	"3B", "Arcade Zone Ltd",
	"3C", "Entertainment International/Empire Software?",
	"3D", "Loriciel",
	"3E", "Gremlin Graphics",
	"3F", "K.Amusement Leasing Co.",
	"40", "Seika Corp.",
	"41", "Ubi Soft Entertainment",
	"42", "Sunsoft US?",
	"44", "Life Fitness",
	"46", "System 3",
	"47", "Spectrum Holobyte",
	"49", "IREM",
	"4B", "Raya Systems",
	"4C", "Renovation Products",
	"4D", "Malibu Games",
	"4F", "Eidos (was U.S. Gold <=1995)",
	"4G", "Playmates Interactive?",
	"4J", "Fox Interactive",
	"4K", "Time Warner Interactive",
	"4Q", "Disney Interactive",
	"4S", "Black Pearl",
	"4U", "Advanced Productions",
	"4X", "GT Interactive",
	"4Y", "RARE?",
	"4Z", "Crave Entertainment",
	"50", "Absolute Entertainment",
	"51", "Acclaim",
	"52", "Activision",
	"53", "American Sammy",
	"54", "Take 2 Interactive (before it was GameTek)",
	"55", "Hi Tech",
	"56", "LJN LTD.",
	"58", "Mattel",
	"5A", "Mindscape, Red Orb Entertainment?",
	"5B", "Romstar",
	"5C", "Taxan",
	"5D", "Midway (before it was Tradewest)",
	"5F", "American Softworks",
	"5G", "Majesco Sales Inc",
	"5H", "3DO",
	"5K", "Hasbro",
	"5L", "NewKidCo",
	"5M", "Telegames",
	"5N", "Metro3D",
	"5P", "Vatical Entertainment",
	"5Q", "LEGO Media",
	"5S", "Xicat Interactive",
	"5T", "Cryo Interactive",
	"5W", "Red Storm Entertainment",
	"5X", "Microids",
	"5Z", "Conspiracy/Swing",
	"60", "Titus",
	"61", "Virgin Interactive",
	"62", "Maxis",
	"64", "LucasArts Entertainment",
	"67", "Ocean",
	"69", "Electronic Arts",
	"6B", "Laser Beam",
	"6E", "Elite Systems",
	"6F", "Electro Brain",
	"6G", "The Learning Company",
	"6H", "BBC",
	"6J", "Software 2000",
	"6L", "BAM! Entertainment",
	"6M", "Studio 3",
	"6Q", "Classified Games",
	"6S", "TDK Mediactive",
	"6U", "DreamCatcher",
	"6V", "JoWood Produtions",
	"6W", "SEGA",
	"6X", "Wannado Edition",
	"6Y", "LSP",
	"6Z", "ITE Media",
	"70", "Infogrames",
	"71", "Interplay",
	"72", "JVC",
	"73", "Parker Brothers",
	"75", "Sales Curve",
	"78", "THQ",
	"79", "Accolade",
	"7A", "Triffix Entertainment",
	"7C", "Microprose Software",
	"7D", "Universal Interactive, Sierra, Simon & Schuster?",
	"7F", "Kemco",
	"7G", "Rage Software",
	"7H", "Encore",
	"7J", "Zoo",
	"7K", "BVM",
	"7L", "Simon & Schuster Interactive",
	"7M", "Asmik Ace Entertainment Inc./AIA",
	"7N", "Empire Interactive?",
	"7Q", "Jester Interactive",
	"7T", "Scholastic",
	"7U", "Ignition Entertainment",
	"7W", "Stadlbauer",
	"80", "Misawa",
	"81", "Teichiku",
	"82", "Namco Ltd.",
	"83", "LOZC",
	"84", "KOEI",
	"86", "Tokuma Shoten Intermedia",
	"87", "Tsukuda Original",
	"88", "DATAM-Polystar",
	"8B", "Bulletproof Software",
	"8C", "Vic Tokai Inc.",
	"8E", "Character Soft",
	"8F", "I'Max",
	"8G", "Saurus",
	"8J", "General Entertainment",
	"8N", "Success",
	"8P", "SEGA Japan",
	"90", "Takara Amusement",
	"91", "Chun Soft",
	"92", "Video System, McO'River???",
	"93", "BEC",
	"95", "Varie",
	"96", "Yonezawa/S'pal",
	"97", "Kaneko",
	"99", "Victor Interactive Software, Pack in Video",
	"9A", "Nichibutsu/Nihon Bussan",
	"9B", "Tecmo",
	"9C", "Imagineer",
	"9F", "Nova",
	"9G", "Den'Z",
	"9H", "Bottom Up",
	"9J", "TGL",
	"9L", "Hasbro Japan?",
	"9N", "Marvelous Entertainment",
	"9P", "Keynet Inc.",
	"9Q", "Hands-On Entertainment",
	"A0", "Telenet",
	"A1", "Hori",
	"A4", "Konami",
	"A5", "K.Amusement Leasing Co.",
	"A6", "Kawada",
	"A7", "Takara",
	"A9", "Technos Japan Corp.",
	"AA", "JVC, Victor Musical Indutries",     
	"AC", "Toei Animation",
	"AD", "Toho",
	"AF", "Namco",
	"AG", "Media Rings Corporation",
	"AH", "J-Wing",
	"AJ", "Pioneer LDC",
	"AK", "KID",
	"AL", "Mediafactory",
	"AP", "Infogrames Hudson",
	"AQ", "Kiratto. Ludic Inc",
	"B0", "Acclaim Japan",
	"B1", "ASCII (was Nexoft?)",
	"B2", "Bandai",
	"B4", "Enix",
	"B6", "HAL Laboratory",
	"B7", "SNK",
	"B9", "Pony Canyon",
	"BA", "Culture Brain",
	"BB", "Sunsoft",
	"BC", "Toshiba EMI",
	"BD", "Sony Imagesoft",
	"BF", "Sammy",
	"BG", "Magical",
	"BH", "Visco",
	"BJ", "Compile",
	"BL", "MTO Inc.",
	"BN", "Sunrise Interactive",
	"BP", "Global A Entertainment",
	"BQ", "Fuuki",
	"C0", "Taito",
	"C2", "Kemco",
	"C3", "Square",
	"C4", "Tokuma Shoten",
	"C5", "Data East",
	"C6", "Tonkin House	(was Tokyo Shoseki)",
	"C8", "Koei",
	"CA", "Konami/Ultra/Palcom",
	"CB", "NTVIC/VAP",
	"CC", "Use Co.,Ltd.",
	"CD", "Meldac",
	"CE", "Pony Canyon",
	"CF", "Angel, Sotsu Agency/Sunrise",
	"CJ", "Boss",
	"CG", "Yumedia/Aroma Co., Ltd",
	"CK", "Axela/Crea-Tech?",
	"CL", "Sekaibunka-Sha, Sumire kobo?, Marigul Management Inc.?",
	"CM", "Konami Computer Entertainment Osaka",
	"CP", "Enterbrain",
	"D0", "Taito/Disco",
	"D1", "Sofel",
	"D2", "Quest, Bothtec",
	"D3", "Sigma, ?????",
	"D4", "Ask Kodansha",
	"D6", "Naxat",
	"D7", "Copya System",
	"D8", "Capcom Co., Ltd.",
	"D9", "Banpresto",
	"DA", "TOMY",
	"DB", "LJN Japan",
	"DD", "NCS",
	"DE", "Human Entertainment",
	"DF", "Altron",
	"DG", "Jaleco???",
	"DH", "Gaps Inc.",
	"DL", "????",
	"DN", "Elf",
	"E0", "Jaleco",
	"E1", "????",
	"E2", "Yutaka",
	"E3", "Varie",
	"E4", "T&ESoft",
	"E5", "Epoch",
	"E7", "Athena",
	"E8", "Asmik",
	"E9", "Natsume",
	"EA", "King Records",
	"EB", "Atlus",
	"EC", "Epic/Sony Records",
	"EE", "IGS",
	"EG", "Chatnoir",
	"EH", "Right Stuff",
	"EJ", "????",
	"EL", "Spike",
	"EM", "Konami Computer Entertainment Tokyo",
	"EN", "Alphadream Corporation",
	"F0", "A Wave",
	"F1", "Motown Software",
	"F2", "Left Field Entertainment",
	"F3", "Extreme Ent. Grp.",
	"F4", "TecMagik",
	"F9", "Cybersoft",
	"FB", "Psygnosis",
	"FE", "Davidson/Western Tech.",
	"G1", "PCCW Japan,"
	"G4", "KiKi Co Ltd",
	"G5", "Open Sesame Inc???",
	"G6", "Sims",
	"G7", "Broccoli",
	"G8", "Avex",
	"G9", "D3 Publisher",
	"GB", "Konami Computer Entertainment Japan",
	"GD", "Square-Enix",
	"IH", "Yojigen",
};

#pragma pack(1)

struct Header
{
	char title[0xC];
	char gamecode[0x4];
	unsigned char makercode[2];
	unsigned char unitcode;
	unsigned char devicetype;		// type of device in the game card
	unsigned char devicecap;		// capacity
	unsigned char reserved1[0x9];
	unsigned char romversion;
	unsigned char reserved2;
	unsigned int arm9_rom_offset;
	unsigned int arm9_entry_address;
	unsigned int arm9_ram_address;
	unsigned int arm9_size;
	unsigned int arm7_rom_offset;
	unsigned int arm7_entry_address;
	unsigned int arm7_ram_address;
	unsigned int arm7_size;
	unsigned int fnt_offset;
	unsigned int fnt_size;
	unsigned int fat_offset;
	unsigned int fat_size;
	unsigned int arm9_overlay_offset;
	unsigned int arm9_overlay_size;
	unsigned int arm7_overlay_offset;
	unsigned int arm7_overlay_size;
	unsigned char rom_control_info1[8];
	unsigned int icon_title_offset;
	unsigned short secure_area_crc;
	unsigned short rom_control_info2;
	unsigned int offset_0x70;
	unsigned int offset_0x74;
	unsigned int offset_0x78;
	unsigned int offset_0x7C;
	unsigned int application_end_offset;			// rom size
	unsigned int rom_header_size;
	unsigned int offset_0x88;
	unsigned int offset_0x8C;
	unsigned int offset_0x90;
	unsigned int offset_0x94;
	unsigned int offset_0x98;
	unsigned int offset_0x9C;
	unsigned int offset_0xA0;
	unsigned int offset_0xA4;
	unsigned int offset_0xA8;
	unsigned int offset_0xAC;
	unsigned int offset_0xB0;
	unsigned int offset_0xB4;
	unsigned int offset_0xB8;
	unsigned int offset_0xBC;
	unsigned char logo[156];
	unsigned short logo_crc;
	unsigned short header_crc;
	unsigned char zero[160];
};

#pragma pack()

/*
 * Variables
 */
Header header;
char *extractdir;

/*
 * DoFile
 */
void DoFile(FILE *f, char *filename, unsigned int file_id)
{
	unsigned int save_filepos = ftell(f);
	
	fseek(f, header.fat_offset + 8*file_id, SEEK_SET);

	unsigned int top;
	fread(&top, 1, sizeof(top), f);
	unsigned int bottom;
	fread(&bottom, 1, sizeof(bottom), f);

	fseek(f, top, SEEK_SET);

	FILE *fo = fopen(filename, "wb");
	if (!fo) { fprintf(stderr, "Cannot create file '%s'.\n", filename); exit(1); }
	unsigned char copybuf[1024];
	unsigned int todo = bottom - top;
	while (todo > 0)
	{
		unsigned int todo2 = (todo >= sizeof(copybuf)) ? sizeof(copybuf) : todo;
		fread(copybuf, 1, todo2, f);
		fwrite(copybuf, 1, todo2, fo);
		todo -= todo2;
	}
	fclose(fo);
	
	fseek(f, save_filepos, SEEK_SET);	
}

/*
 * DoDir
 */
void DoDir(FILE *f, char *prefix, unsigned int dir_id)
{
	unsigned int save_filepos = ftell(f);

	fseek(f, header.fnt_offset + 8*(dir_id & 0xFFF), SEEK_SET);

	unsigned int	entry_start;	// Reference location of entry name
	fread(&entry_start, 1, sizeof(entry_start), f);
	unsigned short	entry_file_id;	// File ID of top entry 
	fread(&entry_file_id, 1, sizeof(entry_file_id), f);
	unsigned short	parent_id;	// ID of parent directory
	fread(&parent_id, 1, sizeof(parent_id), f);

	fseek(f, header.fnt_offset + entry_start, SEEK_SET);

	printf("%s\n", prefix);

	for (unsigned int file_id=entry_file_id; ; file_id++)
	{
		unsigned char entry_type_name_length;
		fread(&entry_type_name_length, 1, sizeof(entry_type_name_length), f);
		unsigned int name_length = entry_type_name_length & 127;
		bool entry_type_directory = (entry_type_name_length & 128) ? true : false;
		if (name_length == 0) break;
	
		char entry_name[128];
		memset(entry_name, 0, 128);
		fread(entry_name, 1, entry_type_name_length & 127, f);
		if (entry_type_directory)
		{
			unsigned short dir_id;
			fread(&dir_id, 1, sizeof(dir_id), f);

			char strbuf[MAX_PATH];

			strcpy(strbuf, extractdir);
			strcat(strbuf, prefix);
			strcat(strbuf, entry_name);
			if (mkdir(strbuf, S_IRWXU)) { fprintf(stderr, "Cannot create directory '%s'.\n", strbuf); exit(1); }

			strcpy(strbuf, prefix);
			strcat(strbuf, entry_name);
			strcat(strbuf, "/");
			DoDir(f, strbuf, dir_id);
		}
		else
		{
			printf("%s%s\n", prefix, entry_name);

			char strbuf[MAX_PATH];
			strcpy(strbuf, extractdir);
			strcat(strbuf, prefix);
			strcat(strbuf, entry_name);
			DoFile(f, strbuf, file_id);
		}
	}

	fseek(f, save_filepos, SEEK_SET);
}

/*
 * Extract
 */
void Extract(char *ndsfilename, char *_extractdir)
{
	FILE *f = fopen(ndsfilename, "rb");
	if (!f) { fprintf(stderr, "Cannot open file '%s'.\n", ndsfilename); exit(1); }
	fread(&header, 512, 1, f);

	extractdir = _extractdir;

	if (mkdir(extractdir, S_IRWXU)) { fprintf(stderr, "Cannot create directory '%s'.\n", extractdir); exit(1); }

	DoDir(f, "/", 0xF000);

	fclose(f);
}

/*
 * ExtractIndirect
 */
void ExtractIndirect(char *ndsfilename, char *outfilename, unsigned int offset_hdrofs, unsigned size_hdrofs)
{
	FILE *f = fopen(ndsfilename, "rb");
	if (!f) { fprintf(stderr, "Cannot open file '%s'.\n", ndsfilename); exit(1); }
	fread(&header, 512, 1, f);

	fseek(f, *((unsigned int *)&header + offset_hdrofs/4), SEEK_SET);
	unsigned int todo = *((unsigned int *)&header + size_hdrofs/4);

	FILE *fo = fopen(outfilename, "wb");
	if (!fo) { fprintf(stderr, "Cannot create file '%s'.\n", outfilename); exit(1); }
	
	unsigned char copybuf[1024];
	while (todo > 0)
	{
		unsigned int todo2 = (todo >= sizeof(copybuf)) ? sizeof(copybuf) : todo;
		fread(copybuf, 1, todo2, f);
		fwrite(copybuf, 1, todo2, fo);
		todo -= todo2;
	}

	fclose(fo);
	fclose(f);
}

/*
 * CalcHeaderCRC
 */
unsigned short CalcHeaderCRC()
{
	unsigned short crc = 0xFFFF;
	for (int i=0; i<0x15E; i++)
	{
		unsigned char data = *((unsigned char *)&header + i);
		crc = (crc >> 8) ^ crc16tab[(crc ^ data) & 0xFF];
	}
	return crc;
}

/*
 * CalcLogoCRC
 */
unsigned short CalcLogoCRC()
{
	unsigned short crc = 0xFFFF;
	for (int i=0xC0; i<0xC0+156; i++)
	{
		unsigned char data = *((unsigned char *)&header + i);
		crc = (crc >> 8) ^ crc16tab[(crc ^ data) & 0xFF];
	}
	return crc;
}

#if 1

/*
 * CalcSecureAreaCRC
 */
unsigned short CalcSecureAreaCRC(FILE *f)
{
	unsigned char data[0x4000];
	fseek(f, 0x4000, SEEK_SET);
	fread(data, 0x4000, 1, f);
	unsigned short crc;
	//for (int x=0; x<256; x++)
	{
		crc = 0xFFFF;
		for (int i=0x0; i<0x4000; i++)
		{
			//unsigned char y = 0;
			//if (i < 0x800) y = x;
			crc = (crc >> 8) ^ crc16tab[(crc ^ data[i]) & 0xFF];
			
			//crc += data[i];
		}
		//if (crc == 0xC44D) printf("OKOKOKOK !!\n");
	}

//	for (int i=0; i<0x4000; i++)
//	{
//		printf("%02X ", data[i]);
//		if (!(~i&31)) printf("\n");
//	}

// 00 21 00 18 00 21 00 18 00 21 00 18 00 21		14

	
	return crc;
}

/*
 * Patch
 */
void Patch(char *ndsfilename, char *outfilename)
{
	FILE *f = fopen(ndsfilename, "rb");
	if (!f) { fprintf(stderr, "Cannot open file '%s'.\n", ndsfilename); exit(1); }
	fread(&header, 512, 1, f);
	fclose(f);


//3C 60 3E BD C3 F5 23 81

/*
0x00  Game title                 S.MARIO64DS 
0x0C  Game code                  ASME (NTR-ASME-USA)
0x2C  ARM9 code size             0x580AC
0x30  ARM7 ROM offset            0x1B0000
0x3C  ARM7 code size             0x24B64
0x40  File name table offset     0x1D4B64
0x44  File name table size       0xA5E6
0x48  FAT offset                 0x1DF14C
0x4C  FAT size                   0x3C80
0x50  ARM9 overlay offset        0x5C0B8
0x54  ARM9 overlay size          0xCE0
0x68  Icon/title offset          0x1E2E00
0x6C  Secure area CRC            0xCAC3 (INVALID)
0x80  Application end offset     0x00E587AD
0x15E Header CRC                 0xFD28 (OK)

0x00  Game title                 FIRST HUNT  
0x0C  Game code                  AMFE (NTR-AMFE-USA)
0x2C  ARM9 code size             0x81D58
0x30  ARM7 ROM offset            0xB3000
0x3C  ARM7 code size             0x26494
0x40  File name table offset     0xD9600
0x44  File name table size       0x11B6
0x48  FAT offset                 0xDA800
0x4C  FAT size                   0x678
0x50  ARM9 overlay offset        0x85E00
0x54  ARM9 overlay size          0x60
0x68  Icon/title offset          0xDB000
0x6C  Secure area CRC            0xC44D (INVALID)
0x80  Application end offset     0x00EE3E44
0x15E Header CRC                 0x00F8 (OK)
*/


//3C XX XX XX -X XX XX --
//3C 6A AB D8 63 E4 81 AC		@MFE
//3C 60 3E BD C3 F5 23 81 		AMFE
//3C 82 32 A8 08 6A C6 7C 		AMFD
//3C 0C F1 21 4A 3C CE 71 		AMFU
//3C AA F2 27 DC AB 7B 1D 		AMFF
//3C A2 24 FE F1 A6 76 F4 		EFMA


//	header.gamecode[0] = 'A';
//	header.gamecode[1] = 'M';
//	header.gamecode[2] = 'F';
//	header.gamecode[3] = 'E';

//	header.makercode[0] = '0';
//	header.makercode[1] = '1';

//	header.unitcode = 0;
//	header.devicetype = 0;
//	header.devicecap = 7;
	//header.reserved1
//	header.romversion = 0;
//	header.reserved2 = 0;



//	header.arm9_rom_offset = 0x00004000+1;

//	header.arm9_entry_address = 0x0200448E;	//0xFFFF0000;		//0x02004800+1;			// entry can't be pointed to bios

//	header.arm9_ram_address = 0x02004000+4;		// aligned to 4 bytes

//	header.arm9_size = 0x4800;	//0x00081D58;	// size 0x7FFFFFFF still starts metroid
																// first 0x4000 bytes use type-1 requests


//	header.arm7_rom_offset = 0x000B3000+1;
	header.arm7_entry_address = 0x08000000;	//0x02380000;		//
//	header.arm7_ram_address = 0x02380000;
//	header.arm7_size = 0;	//0x00026494;
//	header.fnt_offset = 0x000D9600;
//	header.fnt_size = 0;	//0x000011B6;
//	header.fat_offset = 0x000DA800;
//	header.fat_size = 0;	//0x00000678;
//
//	header.arm9_overlay_offset = 0x00085E00;		// nothing					?
//	header.arm9_overlay_size = 0x00000060;		// nothing					length
//
//	header.arm7_overlay_offset = 0x00000000;
//	header.arm7_overlay_size = 0x00000000;
//

	// 00605800F8081800
//	header.rom_control_info1[0] = 0;
//	header.rom_control_info1[1] = 0;
//	header.rom_control_info1[2] = 0;
//	header.rom_control_info1[3] = 0;
//	header.rom_control_info1[4] = 0;
//	header.rom_control_info1[5] = 0;
//	header.rom_control_info1[6] = 0;
//	header.rom_control_info1[7] = 0;
//
//	header.icon_title_offset = 0x000DB000+1;


//	header.secure_area_crc = 0;
	//header.rom_control_info2

	//header.application_end_offset = 0x00EE3E44;
	//header.rom_header_size = 0x4000;


	header.header_crc = CalcHeaderCRC();
	header.logo_crc = CalcLogoCRC();

	f = fopen(outfilename, "wb");
	if (!f) { fprintf(stderr, "Cannot open file.\n"); exit(1); }
	fwrite(&header, 512, 1, f);
	fclose(f);
}

#endif

/*
 * FixCRCs
 */
void FixCRCs(char *ndsfilename)
{
	FILE *f = fopen(ndsfilename, "r+b");
	if (!f) { fprintf(stderr, "Cannot open file '%s'.\n", ndsfilename); exit(1); }
	fread(&header, 512, 1, f);
	header.header_crc = CalcHeaderCRC();
	header.logo_crc = CalcLogoCRC();
	fseek(f, 0, SEEK_SET);
	fwrite(&header, 512, 1, f);
	fclose(f);
}

/*
 * ShowHeader
 */
void ShowHeader(char *ndsfilename)
{
	FILE *f = fopen(ndsfilename, "rb");
	if (!f) { fprintf(stderr, "Cannot open file '%s'.\n", ndsfilename); exit(1); }
	fread(&header, 512, 1, f);

	printf("0x00  %-25s  ", "Game title"); for (int i=0; i<sizeof(header.title); i++) printf("%c", header.title[i]); printf("\n");

	printf("0x0C  %-25s  ", "Game code"); for (int i=0; i<sizeof(header.gamecode); i++) printf("%c", header.gamecode[i]);
	for (int i=0; i<sizeof(countries) / sizeof(countries[0]); i++)
	{
		if (countries[i].countrycode == header.gamecode[3])
		{
			printf(" (NTR-");
			for (int j=0; j<sizeof(header.gamecode); j++) printf("%c", header.gamecode[j]);
			printf("-%s)", countries[i].name);
			break;
		}
	}
	printf("\n");
	
	printf("0x10  %-25s  ", "Maker code"); for (int i=0; i<sizeof(header.makercode); i++) printf("%c", header.makercode[i]);
	for (int j=0; j<sizeof(makers) / sizeof(makers[0]); j++)
	{
		if ((makers[j].makercode[0] == header.makercode[0]) && (makers[j].makercode[1] == header.makercode[1]))
		{
			printf(" (%s)", makers[j].name);
			break;
		}
	}
	printf("\n");

	printf("0x12  %-25s  0x%02X\n", "Unit code", header.unitcode);
	printf("0x13  %-25s  0x%02X\n", "Devide type", header.devicetype);
	printf("0x14  %-25s  0x%02X (%d Mbit)\n", "Device capacity", header.devicecap, 1<<header.devicecap);
	printf("0x15  %-25s  ", "reserved 1"); for (int i=0; i<sizeof(header.reserved1); i++) printf("%02X", header.reserved1[i]); printf("\n");
	printf("0x1E  %-25s  0x%02X\n", "ROM version", header.romversion);
	printf("0x1F  %-25s  0x%02X\n", "reserved 2", header.reserved2);
	printf("0x20  %-25s  0x%X\n", "ARM9 ROM offset", header.arm9_rom_offset);
	printf("0x24  %-25s  0x%X\n", "ARM9 entry address", header.arm9_entry_address);
	printf("0x28  %-25s  0x%X\n", "ARM9 RAM address", header.arm9_ram_address);
	printf("0x2C  %-25s  0x%X\n", "ARM9 code size", header.arm9_size);
	printf("0x30  %-25s  0x%X\n", "ARM7 ROM offset", header.arm7_rom_offset);
	printf("0x34  %-25s  0x%X\n", "ARM7 entry address", header.arm7_entry_address);
	printf("0x38  %-25s  0x%X\n", "ARM7 RAM address", header.arm7_ram_address);
	printf("0x3C  %-25s  0x%X\n", "ARM7 code size", header.arm7_size);
	printf("0x40  %-25s  0x%X\n", "File name table offset", header.fnt_offset);
	printf("0x44  %-25s  0x%X\n", "File name table size", header.fnt_size);
	printf("0x48  %-25s  0x%X\n", "FAT offset", header.fat_offset);
	printf("0x4C  %-25s  0x%X\n", "FAT size", header.fat_size);
	printf("0x50  %-25s  0x%X\n", "ARM9 overlay offset", header.arm9_overlay_offset);
	printf("0x54  %-25s  0x%X\n", "ARM9 overlay size", header.arm9_overlay_size);
	printf("0x58  %-25s  0x%X\n", "ARM7 overlay offset", header.arm7_overlay_offset);
	printf("0x5C  %-25s  0x%X\n", "ARM7 overlay size", header.arm7_overlay_size);
	printf("0x60  %-25s  ", "ROM control info 1"); for (int i=0; i<sizeof(header.rom_control_info1); i++) printf("%02X", header.rom_control_info1[i]); printf("\n");
	printf("0x68  %-25s  0x%X\n", "Icon/title offset", header.icon_title_offset);
	#if 1
		unsigned short secure_area_crc = CalcSecureAreaCRC(f);
		//printf("%04X\n", secure_area_crc);
		printf("0x6C  %-25s  0x%04X (%s)\n", "Secure area CRC", header.secure_area_crc, secure_area_crc == header.secure_area_crc ? "OK" : "INVALID");
	#else
		printf("0x6C  %-25s  0x%04X (?)\n", "Secure area CRC", header.secure_area_crc);
	#endif
	printf("0x6E  %-25s  0x%04X\n", "ROM control info 2", header.rom_control_info2);
	printf("0x70  %-25s  0x%X\n", "?", header.offset_0x70);
	printf("0x74  %-25s  0x%X\n", "?", header.offset_0x74);
	printf("0x78  %-25s  0x%08X\n", "?", header.offset_0x78);
	printf("0x7C  %-25s  0x%08X\n", "?", header.offset_0x7C);
	printf("0x80  %-25s  0x%08X\n", "Application end offset", header.application_end_offset);
	printf("0x84  %-25s  0x%08X\n", "ROM header size", header.rom_header_size);
	printf("0x88  %-25s  0x%08X\n", "?", header.offset_0x88);
	printf("0x8C  %-25s  0x%08X\n", "?", header.offset_0x8C);
	printf("0x90  %-25s  0x%08X\n", "?", header.offset_0x90);
	printf("0x94  %-25s  0x%08X\n", "?", header.offset_0x94);
	printf("0x98  %-25s  0x%08X\n", "?", header.offset_0x98);
	printf("0x9C  %-25s  0x%08X\n", "?", header.offset_0x9C);
	printf("0xA0  %-25s  0x%08X\n", "?", header.offset_0xA0);
	printf("0xA4  %-25s  0x%08X\n", "?", header.offset_0xA4);
	printf("0xA8  %-25s  0x%08X\n", "?", header.offset_0xA8);
	printf("0xAC  %-25s  0x%08X\n", "?", header.offset_0xAC);
	printf("0xB0  %-25s  0x%08X\n", "?", header.offset_0xB0);
	printf("0xB4  %-25s  0x%08X\n", "?", header.offset_0xB4);
	printf("0xB8  %-25s  0x%08X\n", "?", header.offset_0xB8);
	printf("0xBC  %-25s  0x%08X\n", "?", header.offset_0xBC);
	unsigned short logo_crc = CalcLogoCRC();
	printf("0x15C %-25s  0x%04X (%s)\n", "Logo CRC", header.logo_crc, logo_crc == header.logo_crc ? "OK" : "INVALID");
	unsigned short header_crc = CalcHeaderCRC();
	printf("0x15E %-25s  0x%04X (%s)\n", "Header CRC", header.header_crc, header_crc == header.header_crc ? "OK" : "INVALID");

	fclose(f);
}

/*
 * Help
 */
void Help()
{
	printf("Show header:       -i game.nds\n");
	printf("Fix CRC16:         -f game.nds\n");
	printf("Extract:           -x game.nds directory\n");
	printf("Extract ARM7 code: -7 game.nds arm7.bin\n");
	printf("Extract ARM9 code: -9 game.nds arm9.bin\n");
	exit(0);
}

/*
 * main
 */
int main(int argc, char *argv[])
{
	if (sizeof(Header) != 0x200) { printf("%d != %d\n", sizeof(Header), 0x200); exit(1); }
	
	printf("Nintendo DS rom tool "VER" by Rafael Vuijk (aka DarkFader)\n\n");

	if (argc < 2) Help();

	for (int a=1; a<argc; a++)
	{
		if (argv[a][0] == '-')
		{
			switch (argv[a][1])
			{
				case 'i':
				{
					char *ndsfilename = argv[++a];
					ShowHeader(ndsfilename);
					break;
				}

				case 'f':
				{
					char *ndsfilename = argv[++a];
					FixCRCs(ndsfilename);
					break;
				}

				case 'x':
				{
					char *ndsfilename = argv[++a];
					char *extractdir = argv[++a];
					Extract(ndsfilename, extractdir);
					break;
				}

				case '7':
				{
					char *ndsfilename = argv[++a];
					char *outfilename = argv[++a];
					ExtractIndirect(ndsfilename, outfilename, 0x30, 0x3C);
					break;
				}

				case '9':
				{
					char *ndsfilename = argv[++a];
					char *outfilename = argv[++a];
					ExtractIndirect(ndsfilename, outfilename, 0x20, 0x2C);
					break;
				}

#if 1
				case 'p':
				{
					char *ndsfilename = argv[++a];
					char *outfilename = argv[++a];
					Patch(ndsfilename, outfilename);
					break;
				}
#endif

				default:
				{
					Help();
					break;
				}
			}
		}
		else
		{
			Help();
		}
		
	}

	return 0;
}
