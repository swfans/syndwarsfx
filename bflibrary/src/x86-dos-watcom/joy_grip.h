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



#ifndef GRIP_H
#define GRIP_H



/* Supported Compiler Checks */
#if !defined(__WATCOMC__)
#error The GrIP SDK is currently only supported by Watcom C/C++
#endif



#ifdef __cplusplus
extern "C"
  {
#endif



/* 2. Type Definitions */
typedef unsigned char         GRIP_SLOT;
typedef unsigned char         GRIP_CLASS;
typedef unsigned char         GRIP_INDEX;
typedef unsigned short        GRIP_VALUE;
typedef unsigned long         GRIP_BITFIELD;
typedef unsigned short        GRIP_BOOL;
#if defined(__FLAT__)
typedef char *                GRIP_STRING;
typedef void *                GRIP_BUF;
typedef unsigned char *       GRIP_BUF_C;
typedef unsigned short *      GRIP_BUF_S;
typedef unsigned long *       GRIP_BUF_L;
#else
typedef char far *            GRIP_STRING;
typedef void far *            GRIP_BUF;
typedef unsigned char far *   GRIP_BUF_C;
typedef unsigned short far *  GRIP_BUF_S;
typedef unsigned long far *   GRIP_BUF_L;
#endif



/* Standard Classes */
#define GRIP_CLASS_BUTTON                 1
#define GRIP_CLASS_AXIS                   2
#define GRIP_CLASS_POV_HAT                3
#define GRIP_CLASS_VELOCITY               4



/* Refresh Flags */
#define GRIP_REFRESH_COMPLETE             0           /* Default */
#define GRIP_REFRESH_PARTIAL              2
#define GRIP_REFRESH_TRANSMIT             0           /* Default */
#define GRIP_REFRESH_NOTRANSMIT           4



/* 3.1 System API Calls */
GRIP_BOOL GrInitialize(void);
void GrShutdown(void);
GRIP_BITFIELD GrRefresh(GRIP_BITFIELD flags);

/* 3.2 Configuration API Calls */
GRIP_BITFIELD GrGetSlotMap(void);
GRIP_BITFIELD GrGetClassMap(GRIP_SLOT s);
GRIP_BITFIELD GrGetOEMClassMap(GRIP_SLOT s);
GRIP_INDEX GrGetMaxIndex(GRIP_SLOT s, GRIP_CLASS c);
GRIP_VALUE GrGetMaxValue(GRIP_SLOT s, GRIP_CLASS c);

/* 3.3 Data API Calls */
GRIP_VALUE GrGetValue(GRIP_SLOT s, GRIP_CLASS c, GRIP_INDEX i);
GRIP_BITFIELD GrGetPackedValues(GRIP_SLOT s, GRIP_CLASS c, GRIP_INDEX start, GRIP_INDEX end);
void GrSetValue(GRIP_SLOT s, GRIP_CLASS c, GRIP_INDEX i, GRIP_VALUE v);

/* 3.4 OEM Information API Calls */
void GrGetVendorName(GRIP_SLOT s, GRIP_STRING name);
GRIP_VALUE GrGetProductName(GRIP_SLOT s, GRIP_STRING name);
void GrGetControlName(GRIP_SLOT s, GRIP_CLASS c, GRIP_INDEX i, GRIP_STRING name);
GRIP_BITFIELD GrGetCaps(GRIP_SLOT s, GRIP_CLASS c, GRIP_INDEX i);

/* 3.5 Library Management Calls */
GRIP_BOOL GrLink(GRIP_BUF image, GRIP_VALUE size);
GRIP_BOOL GrUnlink(void);

GRIP_BOOL Gr__Link(void);
void Gr__Unlink(void);

/* Diagnostic Information Calls */
GRIP_VALUE GrGetSWVer(void);
GRIP_VALUE GrGetHWVer(void);
GRIP_VALUE GrGetDiagCnt(void);
unsigned long GrGetDiagReg(GRIP_INDEX reg);



/* Standard String Constants for Loadable Library */
extern char GrLibName[];
extern char GrLibEnv[];
extern char GrLibDir[];



/* API Call Thunk */
typedef unsigned char GRIP_THUNK[14];
extern GRIP_THUNK GRIP_Thunk;



#ifdef __cplusplus
  }
#endif



/* Model Specific Assembly Language */
#if defined(__FLAT__)
#define GRIP__PACK_EDX                      \
  ""
#define GRIP__UNPACK_EAX                    \
  ""
#define GRIP__UNPACK_STRING                 \
  "mov [edi][00h], ebx                    " \
  "mov [edi][04h], ecx                    " \
  "mov [edi][08h], edx                    " \
  "mov [edi][0Ch], esi                    "
#define GRIP__REG_EDX          edx
#define GRIP__REG_EAX          eax
#define GRIP__REG_STRING       edi
#define GRIP__MOD_STRING       ebx ecx edx esi
#define GRIP__CALL                          \
  "call far [GRIP_Thunk+8]                 "
#else
#define GRIP__PACK_EDX                      \
  "shl edx, 16                            " \
  "mov dx, ax                             "
#define GRIP__UNPACK_EAX                    \
  "mov edx, eax                           " \
  "shr edx, 16                            "
#define GRIP__UNPACK_STRING                 \
  "mov es:[di][00h], ebx                  " \
  "mov es:[di][04h], ecx                  " \
  "mov es:[di][08h], edx                  " \
  "mov es:[di][0Ch], esi                  "
#define GRIP__REG_EDX          dx ax
#define GRIP__REG_EAX          dx ax
#define GRIP__REG_STRING       es di
#define GRIP__MOD_STRING       bx cx dx si
#if defined(__COMPACT__) || defined(__LARGE__) || defined(__HUGE__)
#define GRIP__CALL                          \
  "push ds                                " \
  "push seg GRIP_Thunk                    " \
  "pop ds                                 " \
  "call far [GRIP_Thunk+0]                " \
  "pop ds                                 "
#else
#define GRIP__CALL                          \
  "call far [GRIP_Thunk+0]                 "
#endif
#endif



#pragma aux GrInitialize =                  \
  "mov ax, 84A1h                          " \
  GRIP__CALL                                \
  value [ax];

#pragma aux GrShutdown =                    \
  "mov ax, 84A2h                          " \
  GRIP__CALL                                \
  modify [ax];

#pragma aux GrRefresh =                     \
  GRIP__PACK_EDX                            \
  "mov ax, 84A0h                          " \
  GRIP__CALL                                \
  GRIP__UNPACK_EAX                          \
  modify [ax]                               \
  parm [GRIP__REG_EDX]                      \
  value [GRIP__REG_EAX];

#pragma aux GrGetSlotMap =                  \
  "mov ax, 84B0h                          " \
  GRIP__CALL                                \
  GRIP__UNPACK_EAX                          \
  value [GRIP__REG_EAX];

#pragma aux GrGetClassMap =                 \
  "mov ax, 84B1h                          " \
  GRIP__CALL                                \
  GRIP__UNPACK_EAX                          \
  parm [bh]                                 \
  value [GRIP__REG_EAX];

#pragma aux GrGetOEMClassMap =              \
  "mov ax, 84B2h                          " \
  GRIP__CALL                                \
  GRIP__UNPACK_EAX                          \
  parm [bh]                                 \
  value [GRIP__REG_EAX];

#pragma aux GrGetMaxIndex =                 \
  "mov ax, 84B3h                          " \
  GRIP__CALL                                \
  parm [bh] [bl]                            \
  value [al];

#pragma aux GrGetMaxValue =                 \
  "mov ax, 84B4h                          " \
  GRIP__CALL                                \
  parm [bh] [bl]                            \
  value [ax];

#pragma aux GrGetValue =                    \
  "mov ax, 84C0h                          " \
  GRIP__CALL                                \
  parm [bh] [bl] [ch]                       \
  value [ax];

#pragma aux GrGetPackedValues =             \
  "mov ax, 84C1h                          " \
  GRIP__CALL                                \
  GRIP__UNPACK_EAX                          \
  parm [bh] [bl] [ch] [cl]                  \
  value [GRIP__REG_EAX];

#pragma aux GrSetValue =                    \
  "mov ax, 84C2h                          " \
  GRIP__CALL                                \
  parm [bh] [bl] [ch] [dx];

#pragma aux GrGetVendorName =               \
  "mov ax, 84D0h                          " \
  GRIP__CALL                                \
  GRIP__UNPACK_STRING                       \
  modify [GRIP__MOD_STRING ax]              \
  parm [bh] [GRIP__REG_STRING];

#pragma aux GrGetProductName =              \
  "mov ax, 84D1h                          " \
  GRIP__CALL                                \
  GRIP__UNPACK_STRING                       \
  modify [GRIP__MOD_STRING]                 \
  parm [bh] [GRIP__REG_STRING]              \
  value [ax];

#pragma aux GrGetControlName =              \
  "mov ax, 84D2h                          " \
  GRIP__CALL                                \
  GRIP__UNPACK_STRING                       \
  modify [GRIP__MOD_STRING ax]              \
  parm [bh] [bl] [ch] [GRIP__REG_STRING];

#pragma aux GrGetCaps =                     \
  "xor eax, eax                           " \
  GRIP__UNPACK_EAX                          \
  parm [bh] [bl] [ch]                       \
  value [GRIP__REG_EAX];

#pragma aux Gr__Link =                      \
  "mov ax, 8490h                          " \
  GRIP__CALL                                \
  value [ax];

#pragma aux Gr__Unlink =                    \
  "mov ax, 8491h                          " \
  GRIP__CALL                                \
  modify [ax];

#pragma aux GrGetSWVer =                    \
  "mov ax, 84F0h                          " \
  GRIP__CALL                                \
  value [ax];

#pragma aux GrGetHWVer =                    \
  "mov ax, 84F1h                          " \
  GRIP__CALL                                \
  value [ax];

#pragma aux GrGetDiagCnt =                  \
  "mov ax, 84F2h                          " \
  GRIP__CALL                                \
  value [ax];

#pragma aux GrGetDiagReg =                  \
  "mov ax, 84F3h                          " \
  GRIP__CALL                                \
  GRIP__UNPACK_EAX                          \
  parm [bh]                                 \
  value [GRIP__REG_EAX];



#endif /* GRIP_H */
