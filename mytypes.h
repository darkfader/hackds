#define NOREGTYPES

#include <nitro/types.h>

u8 Read8(unsigned int address);
u16 Read16(unsigned int address);
u32 Read32(unsigned int address);
u64 Read64(unsigned int address);
void Write8(unsigned int address, u8 data);
void Write16(unsigned int address, u16 data);
void Write32(unsigned int address, u32 data);
void Write64(unsigned int address, u64 data);

struct REGType8v
{
	operator u32 () const { return Read8((unsigned int)this); }
	u8 operator = (u8 x) { Write8((unsigned int)this, x); return x; }
	u8 operator |= (u8 x) { *this = *this | x; return *this; }
	u8 operator &= (u8 x) { *this = *this & x; return *this; }
};

struct REGType16v
{
	operator u32 () const { return Read16((unsigned int)this); }
	u16 operator = (u16 x) { Write16((unsigned int)this, x); return x; }
	u16 operator |= (u16 x) { *this = *this | x; return *this; }
	u16 operator &= (u16 x) { *this = *this & x; return *this; }
};

struct REGType32v
{
	operator u32 () const { return Read32((unsigned int)this); }
	u32 operator = (u32 x) { Write32((unsigned int)this, x); return x; }
	u32 operator |= (u32 x) { *this = *this | x; return *this; }
	u32 operator &= (u32 x) { *this = *this & x; return *this; }
};

struct REGType64v
{
	operator u64 () const { return Read64((unsigned int)this); }
	u64 operator = (u64 x) { Write64((unsigned int)this, x); return x; }
	u64 operator |= (u64 x) { *this = *this | x; return *this; }
	u64 operator &= (u64 x) { *this = *this & x; return *this; }
};

typedef REGType8v		REGType8;
typedef REGType16v		REGType16;
typedef REGType32v		REGType32;
typedef REGType64v		REGType64;
