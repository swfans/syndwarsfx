/************************************************************************\
**                                                                      **
**                GrIP Prototype API Interface Library                  **
**                                for                                   **
**                            Watcom C/C++                              **
**                                                                      **
**                           Revision 1.00                              **
**                                                                      **
**  COPYRIGHT:                                                          **
**                                                                      **
**    (C) Copyright Advanced Gravis Computer Technology Ltd 1995.       **
**        All Rights Reserved.                                          **
**                                                                      **
**  DISCLAIMER OF WARRANTIES:                                           **
**                                                                      **
**    The following [enclosed] code is provided to you "AS IS",         **
**    without warranty of any kind.  You have a royalty-free right to   **
**    use, modify, reproduce and distribute the following code (and/or  **
**    any modified version) provided that you agree that Advanced       **
**    Gravis has no warranty obligations and shall not be liable for    **
**    any damages arising out of your use of this code, even if they    **
**    have been advised of the possibility of such damages.  This       **
**    Copyright statement and Disclaimer of Warranties may not be       **
**    removed.                                                          **
**                                                                      **
**  HISTORY:                                                            **
**                                                                      **
**    0.102   Jul 12 95   David Bollo     Initial public release on     **
**                                          GrIP hardware               **
**    0.200   Aug 10 95   David Bollo     Added Gravis Loadable Library **
**                                          support                     **
**    0.201   Aug 11 95   David Bollo     Removed Borland C++ support   **
**                                          for maintenance reasons     **
**    1.00    Nov  1 95   David Bollo     First official release as     **
**                                          part of GrIP SDK            **
**                                                                      **
\************************************************************************/



#include <stdlib.h>
#include "grip.h"



/* Standard String Constants for Loadable Library */
char GrLibName[] = "GRIP.GLL";
char GrLibEnv[] = "GRIP";
char GrLibDir[] = "C:\\GRIP\\";



/* Global Thunking Data */
GRIP_THUNK GRIP_Thunk;
static int GRIP_Thunked = 0;
static unsigned short GRIP_CS;
static unsigned short GRIP_DS;
#if defined(GRIP_DEBUG)
static unsigned short GRIP_ES;
#endif
#if !defined(__FLAT__)
static void* GRIP_Buf;
#endif



#ifdef __cplusplus
extern "C"
  {
#endif




#if defined(__FLAT__)


void* DPMI_AllocDOS(unsigned short paragraphs, GRIP_BUF_S ds);
#pragma aux DPMI_AllocDOS =                 \
  "mov eax, 100h                          " \
  "int 31h                                " \
  "mov ebx, 0                             " \
  "jc done                                " \
  "mov ebx, eax                           " \
  "shl ebx, 4                             " \
  "mov word ptr [ecx], dx                 " \
  "done:                                  " \
  modify exact [eax ebx edx]                \
  parm [bx] [ecx]                           \
  value [ebx];


unsigned char DPMI_FreeDOS(unsigned short ds);
#pragma aux DPMI_FreeDOS =                  \
  "mov eax, 101h                          " \
  "int 31h                                " \
  "jc done                                " \
  "xor eax, eax                           " \
  "done:                                  " \
  modify exact [eax]                        \
  parm [dx]                                 \
  value [al];


unsigned short DPMI_AllocSel();
#pragma aux DPMI_AllocSel =                 \
  "mov eax, 0h                            " \
  "mov ecx, 1                             " \
  "int 31h                                " \
  "jnc done                               " \
  "xor eax, eax                           " \
  "done:                                  " \
  modify exact [eax ecx]                    \
  value [ax];


unsigned char DPMI_FreeSel(unsigned short sel);
#pragma aux DPMI_FreeSel =                  \
  "mov eax, 1h                            " \
  "int 31h                                " \
  "setc al                                " \
  modify exact [eax]                        \
  parm [bx]                                 \
  value [al];


unsigned char DPMI_SetBounds(unsigned short sel, void* bs, unsigned short lm);
#pragma aux DPMI_SetBounds =                \
  "mov ecx, edx                           " \
  "shr ecx, 16                            " \
  "mov eax, 7h                            " \
  "int 31h                                " \
  "jc done                                " \
  "mov edx, esi                           " \
  "dec edx                                " \
  "xor ecx, ecx                           " \
  "mov eax, 8h                            " \
  "int 31h                                " \
  "jc done                                " \
  "xor eax, eax                           " \
  "done:                                  " \
  modify exact [eax ecx edx]                \
  parm [bx] [edx] [si]                      \
  value [al];


unsigned char DPMI_SetCodeAR(unsigned short sel);
#pragma aux DPMI_SetCodeAR =                \
  "mov cx, cs                             " \
  "lar ecx, ecx                           " \
  "shr cx, 8                              " \
  "mov eax, 9h                            " \
  "int 31h                                " \
  "setc al                                " \
  modify exact [eax ecx]                    \
  parm [bx]                                 \
  value [al];


unsigned char DPMI_SetDataAR(unsigned short sel);
#pragma aux DPMI_SetDataAR =                \
  "mov cx, ds                             " \
  "lar ecx, ecx                           " \
  "shr cx, 8                              " \
  "mov eax, 9h                            " \
  "int 31h                                " \
  "setc al                                " \
  modify exact [eax ecx]                    \
  parm [bx]                                 \
  value [al];


#endif



GRIP_BOOL GrLink(GRIP_BUF image, GRIP_VALUE size)
  {
  unsigned short img_off;     // Offset of binary image
  unsigned short img_size;    // Size of binary image
  unsigned char img_check;    // Checksum of binary image
  GRIP_BUF_C header;          // Image header
  GRIP_BUF_C core;            // Image core
  GRIP_BUF_L memory;          // Image memory
  int i;

  // Check for duplicate GrLink calls
  if (GRIP_Thunked)
    return 0;

  // Sanity check the inputs
  if (image == 0 || size < 16)
    return 0;

  // Check the image header for the proper signature bytes
  header = (GRIP_BUF_C)image;
  if (header[0] != 'D' || header[1] != 'B')
    return 0;

  // Extract the image offsets from the header
  img_off = ( (header[2] & 0x0F) << 12 ) |
            ( (header[3] & 0x0F) <<  8 ) |
            ( (header[4] & 0x0F) <<  4 ) |
            ( (header[5] & 0x0F) );
  img_size = ( (header[6] & 0x0F) << 12 ) |
             ( (header[7] & 0x0F) <<  8 ) |
             ( (header[8] & 0x0F) <<  4 ) |
             ( (header[9] & 0x0F) );
  img_check = ( (header[10] & 0x0F) <<  4 ) |
              ( (header[11] & 0x0F) );

  // Compute the start of image core
  if ( size < ((img_off + img_size) << 4) )
    return 0;
  core = header + (img_off << 4);

  // Verify the checksum
  for (i = 0; i < (img_size << 4); i++)
    img_check ^= core[i];
  if (img_check != 0)
    return 0;

  // Allocate memory for the core
  #if defined(__FLAT__)
    memory = (GRIP_BUF_L)DPMI_AllocDOS(img_size, &GRIP_DS);
    if (!memory)
      return 0;
  #else
    GRIP_Buf = malloc( (img_size+1) << 4 );
    if (!GRIP_Buf)
      return 0;
    GRIP_DS = (unsigned short)
             ( (((((unsigned long)(GRIP_BUF)GRIP_Buf) >> 12) & 0xFFFF0uL) +
               ((unsigned short)(GRIP_BUF)GRIP_Buf) + 0xF) >> 4 );
    memory = (GRIP_BUF_L)(((unsigned long)GRIP_DS) << 16);
  #endif

  // Copy the image
  for (i = 0; i < (img_size << 2); i++)
    memory[i] = ((GRIP_BUF_L)core)[i];

  // Allocate a code selector for the core
  #if defined(__FLAT__)
    GRIP_CS = DPMI_AllocSel();
    if (GRIP_CS == 0)
      {
      DPMI_FreeDOS(GRIP_DS);
      return 0;
      }
    if (DPMI_SetBounds(GRIP_CS, memory, img_size << 4) != 0)
      {
      DPMI_FreeSel(GRIP_CS);
      DPMI_FreeDOS(GRIP_DS);
      return 0;
      }
    if (DPMI_SetCodeAR(GRIP_CS) != 0)
      {
      DPMI_FreeSel(GRIP_CS);
      DPMI_FreeDOS(GRIP_DS);
      return 0;
      }
  #else
    GRIP_CS = GRIP_DS;
  #endif

  // Prepare the thunking layer
  for (i = 0; i < sizeof(GRIP_THUNK); i++)
    GRIP_Thunk[i] = ((GRIP_BUF_C)core)[i];
  #if defined(__FLAT__)
    *(GRIP_BUF_S)(GRIP_Thunk+0x02) = 0;
    *(GRIP_BUF_S)(GRIP_Thunk+0x06) = GRIP_CS;
    *(GRIP_BUF_S)(GRIP_Thunk+0x0C) = GRIP_CS;
  #else
    *(GRIP_BUF_S)(GRIP_Thunk+0x02) = GRIP_CS;
    *(GRIP_BUF_S)(GRIP_Thunk+0x06) = 0;
    *(GRIP_BUF_S)(GRIP_Thunk+0x0C) = 0;
  #endif

  // Allocate a debugging selector if GRIP_DEBUG is defined
  #if defined(GRIP_DEBUG)
    #if defined(__FLAT__)
      GRIP_ES = DPMI_AllocSel();
      if (GRIP_ES == 0)
        {
        DPMI_FreeSel(GRIP_CS);
        DPMI_FreeDOS(GRIP_DS);
        return 0;
        }
      if (DPMI_SetBounds(GRIP_ES, (void*)0xB0000, 80*25*2) != 0)
        {
        DPMI_FreeSel(GRIP_ES);
        DPMI_FreeSel(GRIP_CS);
        DPMI_FreeDOS(GRIP_DS);
        return 0;
        }
      if (DPMI_SetDataAR(GRIP_ES) != 0)
        {
        DPMI_FreeSel(GRIP_ES);
        DPMI_FreeSel(GRIP_CS);
        DPMI_FreeDOS(GRIP_DS);
        return 0;
        }
      *(GRIP_BUF_S)(((GRIP_BUF_C)memory)+0x12) = 0xB000;
      *(GRIP_BUF_S)(((GRIP_BUF_C)memory)+0x16) = GRIP_ES;
    #else
      GRIP_ES = 0xB000;
      *(GRIP_BUF_S)(((GRIP_BUF_C)memory)+0x12) = 0xB000;
      *(GRIP_BUF_S)(((GRIP_BUF_C)memory)+0x16) = 0;
    #endif
  #else
    *(GRIP_BUF_S)(((GRIP_BUF_C)memory)+0x12) = 0;
    *(GRIP_BUF_S)(((GRIP_BUF_C)memory)+0x16) = 0;
  #endif

  // Save the data selector
  #if defined(__FLAT__)
    *(GRIP_BUF_S)(((GRIP_BUF_C)memory)+0x10) = 0;
    *(GRIP_BUF_S)(((GRIP_BUF_C)memory)+0x14) = GRIP_DS;
  #else
    *(GRIP_BUF_S)(((GRIP_BUF_C)memory)+0x10) = GRIP_DS;
    *(GRIP_BUF_S)(((GRIP_BUF_C)memory)+0x14) = 0;
  #endif

  // Complete the link
  if (Gr__Link() == 0)
    {
    #if defined(__FLAT__)
      #if defined(GRIP_DEBUG)
        DPMI_FreeSel(GRIP_ES);
      #endif
      DPMI_FreeSel(GRIP_CS);
      DPMI_FreeDOS(GRIP_DS);
    #else
      free(GRIP_Buf);
    #endif
    return 0;
    }

  GRIP_Thunked = 1;

  return 1;
  }



GRIP_BOOL GrUnlink(void)
  {
  // Check that the library has been loaded
  if (!GRIP_Thunked)
    return 0;

  // Unlink the library
  Gr__Unlink();

  // Free the library memory
  #if defined(__FLAT__)
    #if defined(GRIP_DEBUG)
      if (DPMI_FreeSel(GRIP_ES) != 0)
        return 0;
    #endif
    if (DPMI_FreeSel(GRIP_CS) != 0)
      return 0;
    if (DPMI_FreeDOS(GRIP_DS) != 0)
      return 0;
  #else
    free(GRIP_Buf);
  #endif

  GRIP_Thunked = 0;

  return 1;
  }



#ifdef __cplusplus
  }
#endif
