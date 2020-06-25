/*---------------------------------------------------------------------------*
  Project:  NitroSDK - - types definition
  File:     types.h

  Copyright 2003-2004 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: types.h,v $
  Revision 1.24  2004/02/05 07:09:03  yasu
  changed SDK prefix iris -> nitro

  Revision 1.23  2004/01/29 08:47:04  nishida_kenji
  Changed typedefs for better type check.
  s32: signed int -> signed long
  u32: unsigned int -> unsigned long
  BOOL: s32 -> int

  Revision 1.22  2004/01/05 04:11:16  nishida_kenji
  added f32, vf32

  Revision 1.21  2003/12/02 12:09:22  yasu
  Dealt with problem of preprocessor built into assembler regarding #<SPACE> as comment  

  Revision 1.20  2003/11/27 06:35:17  yada
  Corrected comment

  Revision 1.19  2003/11/11 09:14:41  nishida_kenji
  Made RegTypeNNv type typedef

  Revision 1.18  2003/11/11 06:10:18  nishida_kenji
  Moved fixed point and matrix to gx/gxcommon.h

  Revision 1.17  2003/11/11 05:46:19  yada
  Added BOOL, TRUE, FALSE. Separated INLINE and SDK_ASSERT()

  Revision 1.16  2003/11/11 04:12:52  nishida_kenji
  Added fx16rs

  Revision 1.15  2003/11/05 07:40:40  yasu
  Silenced the VecFx16 padding warning

  Revision 1.14  2003/11/04 12:04:59  yasu
  Added aligned, but only CW

  Revision 1.13  2003/11/04 11:45:20  yasu
  Added NULL, etc.

  Revision 1.12  2003/11/04 09:37:03  yada
  Removed REG_BASE

  Revision 1.11  2003/11/04 04:52:20  Nishida_Kenji
  Quit using bit fields

  $NoKeywords: $
 *---------------------------------------------------------------------------*/

#ifndef NITRO_TYPES_H_
#define NITRO_TYPES_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned char           u8;
typedef unsigned short int      u16;
typedef unsigned long           u32;
typedef unsigned long long int  u64;

typedef signed char             s8;
typedef signed short int        s16;
typedef signed long             s32;
typedef signed long long int    s64;


typedef volatile u8             vu8;
typedef volatile u16            vu16;
typedef volatile u32            vu32;
typedef volatile u64            vu64;

typedef volatile s8             vs8;
typedef volatile s16            vs16;
typedef volatile s32            vs32;
typedef volatile s64            vs64;

typedef float                   f32;
typedef volatile f32            vf32;

/*
    Macro and types used with io_register_list_XX.h
    Need to discuss if should be placed in this file
 */

#ifndef NOREGTYPES

typedef u8                     REGType8;
typedef u16                    REGType16;
typedef u32                    REGType32;
typedef u64                    REGType64;

typedef vu8                     REGType8v;
typedef vu16                    REGType16v;
typedef vu32                    REGType32v;
typedef vu64                    REGType64v;

#endif

#ifndef BOOL
typedef int                 BOOL;
#endif  // BOOL

#ifndef TRUE
// Any non-zero value is considered TRUE
#define TRUE                1
#endif  // TRUE

#ifndef FALSE
#define FALSE               0
#endif  // FALSE


#ifndef NULL
#ifdef  __cplusplus
#define NULL                0
#else   // __cplusplus
#define NULL                ((void *)0)
#endif  // __cplusplus
#endif  // NULL

#ifdef __cplusplus
} /* extern "C" */
#endif

/* NITRO_TYPES_H_ */
#endif
