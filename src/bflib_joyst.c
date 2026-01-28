/******************************************************************************/
// Bullfrog Engine Emulation Library - for use to remake classic games like
// Syndicate Wars, Magic Carpet or Dungeon Keeper.
/******************************************************************************/
/** @file bflib_joyst.c
 *     Joystick support.
 * @par Purpose:
 *     Implement Joystick input support for games.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     11 Feb 2010 - 11 Oct 2020
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "bflib_joyst.h"

#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include "bfmemory.h"

#include "dos.h"
#include "swlog.h"

#if defined(DOS)||defined(GO32)
#include "joy_grip.h"
#include "joy_splib.h"
#endif

/******************************************************************************/
extern struct TbInputHandler *InputHandler;
extern uint8_t joy_grip_initialized;
extern uint8_t joy_spbal_initialized;

extern uint8_t byte_1E2F0C;
extern int32_t dword_1E2F10;
extern int32_t dword_1E2F14;

extern int32_t dword_1E2F24;
extern int32_t dword_1E2F28;
extern int32_t dword_1E2F2C;
extern int32_t dword_1E2F30;
extern uint8_t byte_1E2F34;
extern uint8_t byte_1E2F36;
extern int32_t dword_1E2F38;

extern ushort (*cbptr_1E5EF8)(short a1, intptr_t a2, int a3);

extern int16_t word_1E6078;
extern int16_t word_1E607A;
extern int16_t word_1E607C;

extern struct UnkVFXStruct1 vfxunk1;
/******************************************************************************/
int JoySetInterrupt(short val)
{
    if (!val)
        return -1;
    InputHandler->InterruptNo = val;
    return 1;
}

int vfx1_unkn_func_03(void)
{
    int ret;
    asm volatile (
      "call ASM_vfx1_unkn_func_03\n"
        : "=r" (ret) : );
    return ret;
}

short vfx1_unkn_func_13(short a1)
{
    short ret;
    asm volatile (
      "call ASM_vfx1_unkn_func_13\n"
        : "=r" (ret) : "a" (a1));
    return ret;
}

int vfx1_unkn_func_12(struct UnkVFXStruct1 *p_uvfx1, short a2)
{
#if 0
    int ret;
    asm volatile (
      "call ASM_vfx1_unkn_func_12\n"
        : "=r" (ret) : "a" (p_uvfx1), "d" (a2));
    return ret;
#endif
    short ret, half;

    ret = vfx1_unkn_func_13(a2);
    if (ret == -1)
        return -1;
    half = ret >> 1;
    p_uvfx1->field_4 = half;
    p_uvfx1->field_0 = half >> 10;
    p_uvfx1->field_2 = (half >> 8) & 3;
    return 0;
}

int joy_func_251(int val, int acen, int amin, int amax)
{
#if 0
    int ret;
    asm volatile (
      "call ASM_joy_func_251\n"
        : "=r" (ret) : "a" (val), "d" (acen), "b" (amin), "c" (amax));
    return ret;
#endif
    int dt1, dt2, dt3;
    int result, dist;

    dt1 = val - acen;
    if (dt1 < 0)
    {
        dt2 = amin - acen;
        dt3 = dt1 - (dt2 >> 3);
        dist = dt2 - 2 * (dt2 >> 3);
        if (dt3 > 0) {
            return 0;
        }
        if (dist == 0)
            dist = 1;

        result = -32767 * dt3 / dist;
        if (result < -32767)
            result = -32767;
    }
    else
    {
        dt2 = amax - acen;
        dt3 = dt1 - (dt2 >> 3);
        dist = dt2 - 2 * (dt2 >> 3);
        if (dt3 < 0) {
            return 0;
        }
        if (dist == 0)
            dist = 1;

        result = 32767 * dt3 / dist;
        if (result > 32767)
            result = 32767;
    }
    return result;
}

int joy_get_device_name(char *textbuf)
{
#if 0
    int ret;
    asm volatile ("call ASM_joy_get_device_name\n"
        : "=r" (ret) : "a" (textbuf));
    return ret;
#endif
    ubyte *p_buf;
    char c;

    if (InputHandler == NULL) {
        return Lb_FAIL;
    }
    p_buf = InputHandler->field_4;
    do
    {
        c = *p_buf;
        *textbuf = c;
        p_buf++;
        textbuf++;
    }
    while (c != 0);

    return Lb_SUCCESS;
}

void joy_update_inputs_lab93(struct DevInput *dinp, short ipos)
{
    long val, thresh;

    switch (dinp->ConfigType[ipos])
    {
    case 1:
    case 3:
    case 12:
        byte_1E2F34 = ~(byte_1E2F34 >> 4) & 0xF;
        dinp->Buttons[ipos] = (byte_1E2F34 | dinp->Buttons[ipos]) & 3;
        break;
    case 2:
    case 4:
    case 5:
    case 7:
    case 9:
    case 13:
    case 21:
        byte_1E2F34 = ~(byte_1E2F34 >> 4) & 0xF;
        dinp->Buttons[ipos] |= byte_1E2F34;
        if (dinp->ConfigType[ipos] == 9)
        {
            if (dword_1E2F2C < dinp->HatMax[ipos])
                dinp->Buttons[ipos] |= 0x10;
            if (dword_1E2F30 < dinp->HatMax[ipos])
                dinp->Buttons[ipos] |= 0x20;
        }
        break;
    case 6:
    case 8:
    case 10:
    case 11:
    case 14:
    case 15:
    case 16:
    case 17:
    case 18:
    case 19:
        break;
    case 20:
        byte_1E2F34 = ~(byte_1E2F34 >> 4) & 0xF;
        dinp->Buttons[ipos] = 0;
        dinp->HatY[ipos] = 0;
        dinp->HatX[ipos] = dinp->HatY[ipos];
        switch (byte_1E2F34)
        {
        case 15:
            dinp->HatY[ipos] = -1;
            break;
        case 11:
            dinp->HatX[ipos] = 1;
            break;
        case 7:
            dinp->HatY[ipos] = 1;
            break;
        case 3:
            dinp->HatX[ipos] = -1;
            break;
        default:
            dinp->Buttons[ipos] = byte_1E2F34;
            break;
        }
        break;
    default:
        break;
    }

    {
        if (dinp->MinXAxis[ipos] > dword_1E2F24)
            dinp->MinXAxis[ipos] = dword_1E2F24;
        if (dinp->MaxXAxis[ipos] < dword_1E2F24)
            dinp->MaxXAxis[ipos] = dword_1E2F24;
    }
    {
        if (dinp->MinYAxis[ipos] > dword_1E2F28)
            dinp->MinYAxis[ipos] = dword_1E2F28;
        if (dinp->MaxYAxis[ipos] < dword_1E2F28)
            dinp->MaxYAxis[ipos] = dword_1E2F28;
    }
    if ((dinp->DeviceType[ipos] & 0x80) != 0)
    {
        if (dinp->MinZAxis[ipos] > dword_1E2F30)
            dinp->MinZAxis[ipos] = dword_1E2F30;
        if (dinp->MaxZAxis[ipos] < dword_1E2F30)
            dinp->MaxZAxis[ipos] = dword_1E2F30;
    }

    {
        dinp->XCentre[ipos] = dinp->MinXAxis[ipos] + ((dinp->MaxXAxis[ipos] - dinp->MinXAxis[ipos]) >> 1);
    }
    {
        dinp->YCentre[ipos] = dinp->MinYAxis[ipos] + ((dinp->MaxYAxis[ipos] - dinp->MinYAxis[ipos]) >> 1);
    }
    if ((dinp->DeviceType[ipos] & 0x80) != 0) {
        dinp->ZCentre[ipos] = dinp->MinZAxis[ipos] + ((dinp->MaxZAxis[ipos] - dinp->MinZAxis[ipos]) >> 1);
    }

    {
        thresh = dinp->XCentre[ipos] >> 1;
        val = 0;
        if (dinp->MinXAxis[ipos] + thresh > dword_1E2F24)
            val = -1;
        if (dinp->MaxXAxis[ipos] - thresh < dword_1E2F24)
            val = 1;
        dinp->DigitalX[ipos] = val;
    }
    {
        thresh = dinp->YCentre[ipos] >> 1;
        val = 0;
        if (dinp->MinYAxis[ipos] + thresh > dword_1E2F28)
            val = -1;
        if (dinp->MaxYAxis[ipos] - thresh < dword_1E2F28)
            val = 1;
        dinp->DigitalY[ipos] = val;
    }
    if ((dinp->DeviceType[ipos] & 0x80) != 0)
    {
        thresh = dinp->ZCentre[ipos] >> 1;
        val = 0;
        if (dinp->MinZAxis[ipos] + thresh > dword_1E2F30)
            val = -1;
        if (dinp->MaxZAxis[ipos] - thresh < dword_1E2F30)
            val = 1;
        dinp->DigitalZ[ipos] = val;
    }

    {
      val = joy_func_251(dword_1E2F24, dinp->XCentre[ipos], dinp->MinXAxis[ipos], dinp->MaxXAxis[ipos]);
      dinp->AnalogueX[ipos] = val;
    }
    {
      val = joy_func_251(dword_1E2F28, dinp->YCentre[ipos], dinp->MinYAxis[ipos], dinp->MaxYAxis[ipos]);
      dinp->AnalogueY[ipos] = val;
    }
    if ((dinp->DeviceType[ipos] & 0x80) != 0)
    {
        val = joy_func_251(dword_1E2F30, dinp->ZCentre[ipos], dinp->MinZAxis[ipos], dinp->MaxZAxis[ipos]);
        dinp->AnalogueZ[ipos] = val;
    }
}

#if defined(DOS)||defined(GO32)

static int CallRealModeInterrupt(ushort intno, struct DPMI_REGS *dpmi_regs)
{
    union REGS regs;
    struct SREGS sregs;

    memset(&regs, 0, sizeof(union REGS));
    /* Use DMPI call 300h to issue the DOS interrupt */
    regs.x.eax = 0x300;
    regs.x.ebx = intno;
    segread(&sregs);
    regs.x.edi = (unsigned int)dpmi_regs;
    int386x(49, &regs, &regs, &sregs);
    return dpmi_regs->eax;
}

int CallJoy(ushort a1)
{
    struct DPMI_REGS dpmi_regs;

    InputHandler->field_2 = a1;
    memset(&dpmi_regs, 0, sizeof(struct DPMI_REGS));
    return CallRealModeInterrupt(InputHandler->InterruptNo, &dpmi_regs);
}

#endif

int joy_update_inputs_sub6(struct DevInput *dinp, short ipos)
{
    if (InputHandler == NULL) {
        dinp->Type = 17;
        return -1;
    }
#if defined(DOS)||defined(GO32)
    CallJoy(2);
    memcpy(dinp, &InputHandler->Input, sizeof(struct DevInput));
#endif
    dinp->Type = 17;
    return 1;
}

int joy_update_inputs_sub8(struct DevInput *dinp, short ipos)
{
    long val, thresh;

#if defined(DOS)||defined(GO32)
    {
        struct SpwRawData spla;

        SpwSimpleGet(0, &spla);
        dinp->Init[ipos] = 1;

        dinp->AnalogueX[ipos] = spla.anx + 512;
        dinp->AnalogueY[ipos] = spla.any + 512;
        dinp->AnalogueZ[ipos] = spla.anz + 512;
        dinp->AnalogueR[ipos] = spla.anr + 512;
        dinp->AnalogueU[ipos] = spla.anu + 512;
        dinp->AnalogueV[ipos] = spla.anv + 512;
        dinp->Buttons[ipos] = spla.butn;
    }
#endif

    thresh = dinp->XCentre[ipos] >> 1;
    val = 0;
    if (dinp->AnalogueX[ipos] < dinp->MinXAxis[ipos] + thresh)
        val = -1;
    if (dinp->AnalogueX[ipos] > dinp->MaxXAxis[ipos] - thresh)
        val = 1;
    dinp->DigitalX[ipos] = val;

    thresh = dinp->YCentre[ipos] >> 1;
    val = 0;
    if (dinp->AnalogueY[ipos] < dinp->MinYAxis[ipos] + thresh)
        val = -1;
    if (dinp->AnalogueY[ipos] > dinp->MaxYAxis[ipos] - thresh)
        val = 1;
    dinp->DigitalY[ipos] = val;

    thresh = dinp->ZCentre[ipos] >> 1;
    val = 0;
    if (dinp->AnalogueZ[ipos] < dinp->MinZAxis[ipos] + thresh)
        val = -1;
    if (dinp->AnalogueZ[ipos] > dinp->MaxZAxis[ipos] - thresh)
        val = 1;
    dinp->DigitalZ[ipos] = val;

    thresh = dinp->RCentre[ipos] >> 1;
    val = 0;
    if (dinp->AnalogueR[ipos] < dinp->MinRAxis[ipos] + thresh)
        val = -1;
    if (dinp->AnalogueR[ipos] > dinp->MaxRAxis[ipos] - thresh)
        val = 1;
    dinp->DigitalR[ipos] = val;

    thresh = dinp->UCentre[ipos] >> 1;
    val = 0;
    if (dinp->AnalogueU[ipos] < dinp->MinUAxis[ipos] + thresh)
        val = -1;
    if (dinp->AnalogueU[ipos] > dinp->MaxUAxis[ipos] - thresh)
        val = 1;
    dinp->DigitalU[ipos] = val;

    thresh = dinp->VCentre[ipos] >> 1;
    val = 0;
    if (dinp->AnalogueV[ipos] < dinp->MinVAxis[ipos] + thresh)
        val = -1;
    if (dinp->AnalogueV[ipos] > dinp->MaxVAxis[ipos] - thresh)
        val = 1;
    dinp->DigitalV[ipos] = val;

    val = joy_func_251(dinp->AnalogueX[ipos], dinp->XCentre[ipos], dinp->MinXAxis[ipos], dinp->MaxXAxis[ipos]);
    dinp->AnalogueX[ipos] = val;
    val = joy_func_251(dinp->AnalogueY[ipos], dinp->YCentre[ipos], dinp->MinYAxis[ipos], dinp->MaxYAxis[ipos]);
    dinp->AnalogueY[ipos] = val;
    val = joy_func_251(dinp->AnalogueZ[ipos], dinp->ZCentre[ipos], dinp->MinZAxis[ipos], dinp->MaxZAxis[ipos]);
    dinp->AnalogueZ[ipos] = val;
    val = joy_func_251(dinp->AnalogueR[ipos], dinp->RCentre[ipos], dinp->MinRAxis[ipos], dinp->MaxRAxis[ipos]);
    dinp->AnalogueR[ipos] = val;
    val = joy_func_251(dinp->AnalogueU[ipos], dinp->UCentre[ipos], dinp->MinUAxis[ipos], dinp->MaxUAxis[ipos]);
    dinp->AnalogueU[ipos] = val;
    val = joy_func_251(dinp->AnalogueV[ipos], dinp->VCentre[ipos], dinp->MinVAxis[ipos], dinp->MaxVAxis[ipos]);
    dinp->AnalogueV[ipos] = val;
    return 0;
}

int joy_update_inputs_sub7(struct DevInput *dinp, short ipos)
{
#if defined(DOS)||defined(GO32)
    short gridx, i;
    ushort classmap;
    short val;
    long lval;

    GrRefresh(0);
    for (gridx = 1, i = 0; gridx < 5; gridx++, i++)
    {
        classmap = GrGetClassMap(gridx);
        if ((classmap & 0x02) == 0) {
            dinp->DeviceType[i] = 0;
            continue;
        }
        dinp->Init[i] = 1;

        val = GrGetValue(gridx, 2, 0) - 1;
        dinp->DigitalX[i] = val;
        val = 1 - GrGetValue(gridx, 2, 1);
        dinp->DigitalY[i] = val;

        val = GrGetPackedValues(gridx, 1, 0, 9);
        lval = val >> 2;
        if (val & 0x01)
            lval |= 0x80;
        if (val & 0x02)
            lval |= 0x40;
        dinp->Buttons[i] = lval;

        val = dinp->DigitalX[i];
        if (val == -1)
            dinp->AnalogueX[i] = -0x7FFF;
        else if (val == 1)
            dinp->AnalogueX[i] = 0x7FFF;

        val = dinp->DigitalY[i];
        if (val == -1)
            dinp->AnalogueY[i] = -0x7FFF;
        else if (val == 1)
            dinp->AnalogueY[i] = 0x7FFF;

        dinp->DeviceType[i] = 112;
    }
#endif
    if (dinp->NumberOfDevices == 0)
        return -1;
    return 1;
}

#if defined(DOS)||defined(GO32)

ubyte joy_port_unkn01_s(ubyte oval)
{
    int i;
    ubyte val;

    cli();
    outp(0x201, oval);
    for (i = 10000; i > 0; i--)
    {
        val = inp(0x201);
        if ((val & 0xF) == 0)
          break;
        if ((val & 0x01) != 0)
          ++dword_1E2F24;
        if ((val & 0x02) != 0)
          ++dword_1E2F28;
        if ((val & 0x04) != 0)
          ++dword_1E2F2C;
        if ((val & 0x08) != 0)
          ++dword_1E2F30;
    }
    sti();
    byte_1E2F34 = val;
    return (i > 0);
}

ubyte joy_port_unkn02_s(ubyte oval)
{
    int i;
    ubyte val;

    cli();
    outp(0x201, oval);
    for (i = 10000; i > 0; i--)
    {
        val = inp(0x201);
        if ((val & 0xB) == 0)
          break;
        if ((val & 0x01) != 0)
          ++dword_1E2F24;
        if ((val & 0x02) != 0)
          ++dword_1E2F28;
        if ((val & 0x08) != 0)
          ++dword_1E2F30;
    }
    sti();
    byte_1E2F34 = val;
    return (i > 0);
}

ubyte joy_port_unkn03_s(ubyte oval)
{
    int i;
    ubyte val;

    cli();
    outp(0x201, oval);
    for (i = 10000; i > 0; i--)
    {
        val = inp(0x201);
        if ((val & 0x3) == 0)
          break;
        if ((val & 0x01) != 0)
          ++dword_1E2F24;
        if ((val & 0x02) != 0)
          ++dword_1E2F28;
    }
    sti();
    byte_1E2F34 = val;
    return (i > 0);
}
#endif

int joy_update_inputs_sub9(struct DevInput *dinp, short ipos)
{
#if defined(DOS)||defined(GO32)
    if (!joy_port_unkn02_s(dinp->ConfigType[ipos] - 1))
        return 0;
#endif
    joy_update_inputs_lab93(dinp, ipos);
    return 0;
}

int joy_update_inputs_sub4(struct DevInput *dinp, short ipos)
{
#if defined(DOS)||defined(GO32)
    if (!joy_port_unkn01_s(dinp->ConfigType[ipos] - 1))
        return -1;
#endif
    dinp->HatMax[ipos] = (dword_1E2F30 + dword_1E2F2C) >> 2;
    joy_update_inputs_lab93(dinp, ipos);
    return 0;
}

int joy_update_inputs_sub1(struct DevInput *dinp, short ipos)
{
#if defined(DOS)||defined(GO32)
    if (!joy_port_unkn03_s(dinp->ConfigType[ipos] - 1))
        return -1;
#endif
    joy_update_inputs_lab93(dinp, ipos);
    return 0;
}

int joy_update_inputs_sub2(struct DevInput *dinp, short ipos)
{
    int i;
#if defined(DOS)||defined(GO32)
    if (!joy_port_unkn02_s(dinp->ConfigType[ipos] - 1))
        return -1;
#endif
    if (dword_1E2F30 > dinp->HatMax[ipos])
        dinp->HatMax[ipos] = dword_1E2F30;
    if (!dinp->HatMax[0])
        dinp->HatMax[0] = 1;
    i = 8 * dword_1E2F30 / dinp->HatMax[0];
    switch (i)
    {
    case 0:
        dinp->HatX[0] = 0;
        dinp->HatY[0] = -1;
        break;
    case 1:
    case 2:
        dinp->HatX[0] = 1;
        dinp->HatY[0] = 0;
        break;
    case 3:
    case 4:
        dinp->HatX[0] = 0;
        dinp->HatY[0] = 1;
        break;
    case 5:
    case 6:
        dinp->HatX[0] = -1;
        dinp->HatY[0] = 0;
        break;
    default:
        dinp->HatX[0] = 0;
        dinp->HatY[0] = 0;
        break;
    }
    joy_update_inputs_lab93(dinp, ipos);
    return 0;
}

void joy_update_inputs_sub3(struct DevInput *dinp, short ipos, ubyte v115)
{
    int ret;

    if (!dinp->Init[ipos])
        return;
    ret = vfx1_unkn_func_12(&vfxunk1, v115 + 1);
    if (ret == -1)
        return;

    if (vfxunk1.field_0 & 0x08) {
        dinp->DigitalY[ipos] = -1;
        dinp->AnalogueY[ipos] = -0x7FFF;
    } else if (vfxunk1.field_0 & 0x04) {
        dinp->DigitalY[ipos] = 1;
        dinp->AnalogueY[ipos] = 0x7FFF;
    } else {
        dinp->DigitalY[ipos] = 0;
        dinp->AnalogueY[ipos] = 0;
    }

    if (vfxunk1.field_0 & 0x02) {
        dinp->DigitalX[ipos] = -1;
        dinp->AnalogueX[ipos] = -0x7FFF;
    } else if (vfxunk1.field_0 & 0x01) {
        dinp->DigitalX[ipos] = 1;
        dinp->AnalogueX[ipos] = 0x7FFF;
    } else {
        dinp->DigitalX[ipos] = 0;
        dinp->AnalogueX[ipos] = 0;
    }

    dinp->Buttons[ipos] = vfxunk1.field_4;
    if (vfxunk1.field_2 & 0x01) {
        dinp->MenuButtons[ipos] |= 0x04;
        dinp->MenuButtons[ipos] |= 0x0200;
    }
    if (vfxunk1.field_2 & 0x02) {
        dinp->MenuButtons[ipos] |= 0x01;
        dinp->Buttons[ipos] |= 0x0100;
    }
}

int joy_update_inputs_sub5(struct DevInput *dinp, short ipos)
{
    vfx1_unkn_func_03();
    dword_1E2F24 = word_1E6078;
    dinp->AnalogueX[ipos] = dword_1E2F24;
    dword_1E2F28 = word_1E607A;
     dinp->AnalogueY[ipos] = dword_1E2F28;
    dword_1E2F30 = word_1E607C;
    dinp->AnalogueZ[ipos] = dword_1E2F30;
    joy_update_inputs_lab93(dinp, ipos);
    return 0;
}

int joy_update_inputs(struct DevInput *dinp)
{
    short i, ipos;
    ubyte v115;
    int ret;

    v115 = 0;
    if (dinp->Type == -1)
        return -1;

    for (i = 0; i < 16; i++)
    {
        dinp->NumberOfButtons[i] = 0;
        dinp->Buttons[i] = 0;
        dinp->HatMax[i] = 0;
        dinp->HatY[i] = 0;
        dinp->HatX[i] = 0;
        dinp->AnalogueZ[i] = 0;
        dinp->AnalogueY[i] = 0;
        dinp->AnalogueY[i] = 0;
        dinp->DigitalZ[i] = 0;
        dinp->DigitalY[i] = 0;
        dinp->DigitalX[i] = 0;
        dinp->AnalogueR[i] = 0;
        dinp->AnalogueV[i] = 0;
        dinp->AnalogueU[i] = 0;
        dinp->DigitalR[i] = 0;
        dinp->DigitalV[i] = 0;
        dinp->DigitalU[i] = 0;
    }
    dword_1E2F28 = 0;
    dword_1E2F24 = 0;
    dword_1E2F2C = 0;
    dword_1E2F30 = 0;

    for (ipos = 0; ipos < 16; ipos++)
    {
        if (dinp->Init[ipos] == 0)
            continue;
        switch (dinp->ConfigType[ipos])
        {
        case 1:
        case 2:
        case 5:
        case 0xC:
        case 0xD:
            ret = joy_update_inputs_sub1(dinp, ipos);
            if (ret != 0)
                return ret;
            break;
        case 3:
        case 4:
        case 7:
            ret = joy_update_inputs_sub2(dinp, ipos);
            if (ret != 0)
                return ret;
            break;
        case 6:
            joy_update_inputs_sub3(dinp, ipos, v115);
            ++v115;
            break;
        case 9:
        case 0xE:
        case 0xF:
            ret = joy_update_inputs_sub4(dinp, ipos);
            if (ret != 0)
                return ret;
            break;
        case 0xB:
        case 0x16:
            ret = joy_update_inputs_sub5(dinp, ipos);
            if (ret != 0)
                return ret;
            break;
        case 0x11:
            joy_update_inputs_sub6(dinp, ipos);
            return ret;
            break;
        case 0x12:
            ret = joy_update_inputs_sub7(dinp, ipos);
            if (ret != 0)
                return ret;
            break;
        case 0x13:
            ret = joy_update_inputs_sub8(dinp, ipos);
            if (ret != 0)
                return ret;
            break;
        case 0x14:
        case 0x15:
            ret = joy_update_inputs_sub9(dinp, ipos);
            if (ret != 0)
                return ret;
            break;
        default:
            joy_update_inputs_lab93(dinp, ipos);
            break;
        }
    }
    return 1;
}

short vfx1_init(void)
{
    int ret;
    asm volatile ("call ASM_vfx1_init\n"
        : "=r" (ret) :  : "eax");
    return ret;
}

int joy_refresh_devices(struct DevInput *dinp)
{
    int ret;
    asm volatile ("call ASM_joy_refresh_devices\n"
        : "=r" (ret) : "a" (dinp));
    return ret;
}

int joy_func_081(struct UnkVFXStruct1 *p_vfxu1, short a2, short a3)
{
#if 0
    int ret;
    asm volatile ("call ASM_joy_func_081\n"
        : "=r" (ret) : "a" (p_vfxu1), "d" (a2), "b" (a3));
    return ret;
#endif
#if defined(DOS)||defined(GO32)
    ubyte bt;
    short i;
    short v6;
    ubyte o, t, repeat;
    char v17;
    short v18;
    ubyte v20;
    char v22;
    ubyte k;
    ubyte k3, k2, k1;
    char v28;
    sbyte v29;

    v29 = -1;
    v28 = 0;
    repeat = 0;
    if (a2 == 1)
    {
        k1 = 0x30;
        k2 = 0x10;
        k3 = 0x20;
    }
    else
    {
        k1 = 0xC0;
        k2 = 0x40;
        k3 = 0x80;
    }

    cli();

    k = 15;
LABEL_5:
    {
        if (k == 1)
            goto DONE;
        --k;
        bt = inp(0x201u);
        o = k1 & bt;
        v6 = 0;
        for (i = 4095; i > 0; i--)
        {
            bt = inp(0x201u);
            t = o ^ k1 & bt;
            if (t != 0)
            {
                o ^= t;
                if ((k2 & t) != 0) {
                    if (++v6 >= 6)
                        break;
                }
                v6 = 0;
            }
        }
        if (i == 0)
            goto DONE;

        for (i = 4095; i > 0; i--)
        {
            bt = inp(0x201u);
            t = o ^ k1 & bt;
            if (t != 0)
            {
                o ^= t;
                if ((k3 & t) != 0) {
                    break;
                }
            }
        }
        if (i == 0)
            goto DONE;

        for (i = 4095; i > 0; i--)
        {
            bt = inp(0x201u);
            t = o ^ k1 & bt;
            if (t != 0)
            {
                o ^= t;
                if ((k2 & t) != 0) {
                    break;
                }
                repeat = 1;
                break;
            }
        }
        if (i == 0)
              goto DONE;
        if (repeat)
            goto LABEL_5;
    }

    v29 = 1;
    v17 = 4;
    for (i = 4095; i > 0; i--)
    {
        bt = inp(0x201u);
        v22 = o ^ k1 & bt;
        if (v22 == 0)
            continue;
        o ^= v22;
        __asm { rcl     bx, 1 }
        if ( --v17 )
            continue;
        if (++v28 == 3)
            break;
        for (v18 = 4095; v18 > 0; v18--)
        {
            bt = inp(0x201u);
            v20 = o ^ k1 & bt;
            if (v20 != 0)
                break;
        }
        if (v20 == 0)
            break;
        o ^= v20;
        if ((k3 & v20) == 0)
        {
            v29 = 9;
            break;
        }
        v17 = 5;
        i = 4095;
    }
DONE:
    sti();
    return v29;
#else
    return 0;
#endif
}

void devinput_clear(struct DevInput *dinp)
{
    memset(dinp, 0, sizeof(struct DevInput));
    dinp->MinXAxis[0] = 0x10000;
    dinp->MaxXAxis[0] = 0;
    dinp->MinYAxis[0] = 0x10000;
    dinp->MaxYAxis[0] = 0;
    dinp->MinZAxis[0] = 0x10000;
    dinp->MaxZAxis[0] = 0;
    dinp->MinRAxis[0] = 0x10000;
    dinp->MaxRAxis[0] = 0;
    dinp->MinUAxis[0] = 0x10000;
    dinp->MaxUAxis[0] = 0;
    dinp->MinVAxis[0] = 0x10000;
    dinp->MaxVAxis[0] = 0;
    dinp->HatMax[0] = 0;
    dinp->NumberOfDevices = 1;
}

ushort cbptr_call(short v26, intptr_t a2, int v27)
{
    ushort cb_ret;
#if defined(DOS)||defined(GO32)
    cb_ret = MK_FP(*((_WORD *)&cbptr_1E5EF8 + 2), cbptr_1E5EF8)(v26, a2, v27);
#else
    cb_ret = cbptr_1E5EF8(v26, a2, v27);
#endif
    return cb_ret;
}

int joy_setup_device_sub1(void)
{
    short v26;
    int v27;
    uint v28;

    if (!joy_grip_initialized)
        return -1;

    if (dword_1E2F14 == 0)
        dword_1E2F14 = dos_clock();

    v26 = dos_clock();
    v27 = dword_1E2F14;
    v28 = v26 - dword_1E2F14;
    dword_1E2F10 = v26;
    if (v26 - dword_1E2F14 > 300)
    {
        dword_1E2F14 = v26;
        if (cbptr_call(-31583, v28, v27))
        {
            byte_1E2F36 = 1;
            dword_1E2F38 = 1;
        }
        else
        {
            byte_1E2F36 = 1;
            dword_1E2F38 = -1;
        }
    }
    return dword_1E2F38;
}

int joy_setup_device_sub2(void)
{
    int cbi;

    cbi = 0x58;
    cbi = (1 << 8) | (cbi & 0xFF);
    byte_1E2F0C = 1;
    if ( joy_grip_initialized )
    {
        if (cbptr_call(-31583, 0, cbi))
        {
            byte_1E2F36 = 1;
            dword_1E2F38 = 1;
        }
        else
        {
            byte_1E2F36 = 1;
            dword_1E2F38 = -1;
        }
    }
    else
    {
        dword_1E2F38 = -1;
    }
    return dword_1E2F38;
}

int joy_func_049(struct DevInput *dinp, int a2)
{
    int ret;
    asm volatile ("call ASM_joy_func_049\n"
        : "=r" (ret) : "a" (dinp), "d" (a2));
    return ret;
}

int joy_enumerate_devices(struct DevInput *dinp)
{
    int cbi;
    int cbr;
    int joyno;
    ubyte v14;
    struct DevInput *dinp_inc;

    cbi = 0x58;
    dinp->NumberOfDevices = 0;
    for (joyno = 0; joyno < 4; joyno++)
    {
        cbi = ((joyno + 1) << 8) | (cbi & 0xFF);
        cbr = cbptr_call(-31567, (intptr_t)dinp_inc, cbi);
        if ((cbr & 2) != 0)
        {
            cbi = ((joyno + 1) << 8) | (1 & 0xFF);
            dinp->NumberOfDevices++;
            v14 = cbptr_call(-31565, (intptr_t)dinp_inc, cbi);
            dinp->DeviceType[joyno] = 112;
            dinp->NumberOfButtons[joyno] = v14 + 1;
            dinp->Init[joyno] = 1;
            dinp->ConfigType[joyno] = 18;
            // Version of dinp shifter so that 32-bit fields at index joyno become index [0]
            // Looks ugly, but we need it for the callback parameter; dinp->Init[] is not 32-bit
            dinp_inc = (struct DevInput *)((ubyte *)dinp_inc + 4);
        }
    }
    return dinp->NumberOfDevices;
}

int joy_setup_device(struct DevInput *dinp, int jtype)
{
#if 0
    int ret;
    asm volatile ("call ASM_joy_setup_device\n"
        : "=r" (ret) : "a" (dinp), "d" (jtype));
    return ret;
#endif
    int loc_jtype;
    int result;
    int jaddr;
    int i;

    loc_jtype = jtype;
    devinput_clear(dinp);
    dword_1E2F2C = 0;
    dword_1E2F28 = 0;
    dword_1E2F24 = 0;

    switch (jtype)
    {
    case 1:
    case 2:
    case 20:
    case 21:
        jaddr = joy_setup_device_sub1();
        if (jaddr == 1)
        {
            result = -1;
            break;
        }
TRY_TYPE_01:
#if defined(DOS)||defined(GO32)
        if (!joy_port_unkn03_s(jaddr))
#else
        if (1)
#endif
        {
            result = -1;
            dinp->Type = -1;
            break;
        }
        dinp->DeviceType[0] = 112;
        if (loc_jtype == 20)
            dinp->DeviceType[0] |= 0x88u;
        else if (loc_jtype == 21)
            dinp->DeviceType[0] |= 0x80u;
        dinp->XCentre[0] = dword_1E2F24;
        dinp->YCentre[0] = dword_1E2F28;
        dinp->MaxXAxis[0] = dword_1E2F24 + dword_1E2F24 / 2;
        dinp->MaxYAxis[0] = dword_1E2F28 + dword_1E2F28 / 2;
        dinp->MinXAxis[0] = dword_1E2F24 / 6;
        dinp->MinYAxis[0] = dword_1E2F28 / 6;
        dinp->NumberOfButtons[0] = 4;
        dinp->Init[0] = 1;
        dinp->ConfigType[0] = loc_jtype;
        dinp->Type = loc_jtype;
        result = 1;
        break;

    case 3:
    case 4:
    case 7:
    case 8:
        jaddr = joy_setup_device_sub1();
        if (jaddr == 1)
        {
            result = -1;
            break;
        }
#if defined(DOS)||defined(GO32)
        if (!joy_port_unkn02_s(jaddr))
#else
        if (1)
#endif
        {
            loc_jtype = 2;
            goto TRY_TYPE_01;
        }
        dinp->XCentre[0] = dword_1E2F24;
        dinp->YCentre[0] = dword_1E2F28;
        dinp->MaxXAxis[0] = dword_1E2F24 + dword_1E2F24 / 2;
        dinp->MaxYAxis[0] = dword_1E2F28 + dword_1E2F28 / 2;
        dinp->MinXAxis[0] = dword_1E2F24 / 6;
        dinp->MinYAxis[0] = dword_1E2F28 / 6;
        dinp->DeviceType[0] = 120;
        dinp->NumberOfButtons[0] = 4;
        dinp->Init[0] = 1;
        dinp->ConfigType[0] = loc_jtype;
        dinp->Type = loc_jtype;
        result = 1;
        break;

    case 5:
    case 12:
    case 13:
        jaddr = joy_setup_device_sub1();
        if (jaddr == 1)
        {
            result = -1;
            break;
        }
TRY_TYPE_05:
#if defined(DOS)||defined(GO32)
        if (!joy_port_unkn03_s(jaddr))
#else
        if (1)
#endif
        {
            dinp->Type = -1;
            result = -1;
            break;
        }
        dinp->XCentre[0] = dword_1E2F24;
        dinp->YCentre[0] = dword_1E2F28;
        dinp->MaxXAxis[0] = dword_1E2F24 + dword_1E2F24 / 2;
        dinp->MaxYAxis[0] = dword_1E2F28 + dword_1E2F28 / 2;
        dinp->MinXAxis[0] = dword_1E2F24 / 6;
        dinp->MinYAxis[0] = dword_1E2F28 / 6;
        dinp->DeviceType[0] = 112;
        dinp->NumberOfButtons[0] = 4;
        dinp->Init[0] = 1;
        dinp->ConfigType[0] = loc_jtype;
        dinp->Type = loc_jtype;
        result = 1;
        break;

    case 6:
        jaddr = joy_setup_device_sub1();
        if (jaddr == 1)
        {
            result = -1;
            break;
        }
        memset(&vfxunk1, 0, sizeof(vfxunk1));
        i = 0;
#if defined(DOS)||defined(GO32)
        if (joy_func_081(&vfxunk1, 1, 6) == 1)
#else
        if (0)
#endif
        {
          dinp->DeviceType[0] = 112;
          dinp->NumberOfButtons[0] = 10;
          dinp->Init[0] = 1;
          dinp->ConfigType[0] = 6;
        }
        else
        {
          i = 1;
        }
#if defined(DOS)||defined(GO32)
        if (joy_func_081(&vfxunk1, 2, 6) == 1)
#else
        if (0)
#endif
        {
          dinp->DeviceType[1] = 112;
          dinp->NumberOfButtons[1] = 10;
          dinp->Init[1] = 1;
          dinp->ConfigType[1] = 6;
          dinp->NumberOfDevices++;
        }
        else
        {
          ++i;
        }
        if (i >= 2)
        {
            loc_jtype = 9;
            goto TRY_TYPE_09;
        }
        dinp->Type = loc_jtype;
        result = 1;
        break;

    case 9:
TRY_TYPE_09:
        jaddr = joy_setup_device_sub1();
        if (jaddr == 1)
        {
            result = -1;
            break;
        }
#if defined(DOS)||defined(GO32)
        if (!joy_port_unkn01_s(jaddr))
#else
        if (1)
#endif
        {
            loc_jtype = 13;
            dword_1E2F2C = 0;
            dword_1E2F28 = 0;
            dword_1E2F24 = 0;
            goto TRY_TYPE_05;
        }
        dinp->XCentre[0] = dword_1E2F24;
        dinp->YCentre[0] = dword_1E2F28;
        dinp->MaxXAxis[0] = dword_1E2F24 + dword_1E2F24 / 2;
        dinp->MaxYAxis[0] = dword_1E2F28 + dword_1E2F28 / 2;
        dinp->MinXAxis[0] = dword_1E2F24 / 6;
        dinp->MinYAxis[0] = dword_1E2F28 / 6;
        dinp->DeviceType[0] = 112;
        dinp->Init[0] = 1;
        dinp->ConfigType[0] = loc_jtype;
        dinp->NumberOfButtons[0] = 6;
        dinp->Type = loc_jtype;
        result = 1;
        break;

    case 11:
    case 22:
        if (!vfx1_init())
        {
            dinp->Type = -1;
            result = -1;
            break;
        }
        dinp->MinXAxis[0] = -32766;
        dinp->MaxXAxis[0] = 32767;
        dinp->MinYAxis[0] = -12743;
        dinp->MaxYAxis[0] = 12743;
        dinp->MinZAxis[0] = -12743;
        dinp->MaxZAxis[0] = 12743;
        dinp->DeviceType[0] = 2288;
        dinp->ConfigType[0] = jtype;
        dinp->Init[0] = 1;
        dinp->Type = loc_jtype;
        result = 1;
        break;

    case 14:
    case 15:
        jaddr = joy_setup_device_sub1();
        if (jaddr == 1)
        {
            result = -1;
            break;
        }
#if defined(DOS)||defined(GO32)
        if (!joy_port_unkn02_s(jaddr))
#else
        if (1)
#endif
        {
            loc_jtype = 13;
            dword_1E2F2C = 0;
            dword_1E2F28 = 0;
            dword_1E2F24 = 0;
            goto TRY_TYPE_05;
        }
        dinp->XCentre[0] = dword_1E2F24;
        dinp->YCentre[0] = dword_1E2F28;
        dinp->MaxXAxis[0] = dword_1E2F24 + dword_1E2F24 / 2;
        dinp->MaxYAxis[0] = dword_1E2F28 + dword_1E2F28 / 2;
        dinp->MinXAxis[0] = dword_1E2F24 / 6;
        dinp->MinYAxis[0] = dword_1E2F28 / 6;
        dinp->DeviceType[0] = 120;
        dinp->NumberOfButtons[0] = 4;
        dinp->Init[0] = 1;
        dinp->ConfigType[0] = loc_jtype;
        dinp->Type = loc_jtype;
        result = 1;
        break;

    case 17:
#if defined(DOS)||defined(GO32)
        CallJoy(1);
#endif
        dinp->NumberOfDevices = 1;
        dinp->ConfigType[0] = 17;
        dinp->Init[0] = 1;
        dinp->Type = jtype;
        result = 1;
        break;

    case 18:
        if (byte_1E2F0C)
          i = joy_setup_device_sub1();
        else
          i = joy_setup_device_sub2();
        if (i != 1)
        {
            dinp->Type = -1;
            result = -1;
            break;
        }
        joy_enumerate_devices(dinp);
        if (dinp->NumberOfDevices < 1)
        {
            dinp->Type = -1;
            result = -1;
            break;
        }
        result = 1;
        dinp->Type = loc_jtype;
        break;

    case 19:
        if (joy_func_049(dinp, 0) != 1)
        {
            dinp->Type = -1;
            result = -1;
            break;
        }
        dinp->ConfigType[0] = jtype;
        dinp->Type = jtype;
        result = 1;
        break;

    case 23:
    case 24:
        jaddr = joy_setup_device_sub1();
        if (jaddr == 1)
        {
            result = -1;
            break;
        }
#if defined(DOS)||defined(GO32)
        if (!joy_port_unkn01_s(jaddr))
#else
        if (1)
#endif
        {
            result = -1;
            break;
        }
        dinp->XCentre[0] = dword_1E2F24;
        dinp->YCentre[0] = dword_1E2F28;
        dinp->MaxXAxis[0] = dword_1E2F24 + dword_1E2F24 / 2;
        dinp->MaxYAxis[0] = dword_1E2F28 + dword_1E2F28 / 2;
        dinp->MinXAxis[0] = dword_1E2F24 / 6;
        dinp->MinYAxis[0] = dword_1E2F28 / 6;
        dinp->DeviceType[0] = 112;
        dinp->NumberOfButtons[0] = 4;
        dinp->Init[0] = 1;
        dinp->ConfigType[0] = loc_jtype;

        dinp->XCentre[1] = dword_1E2F2C;
        dinp->YCentre[1] = dword_1E2F30;
        dinp->MaxXAxis[1] = dword_1E2F2C + dword_1E2F2C / 2;
        dinp->MaxYAxis[1] = dword_1E2F30 + dword_1E2F30 / 2;
        dinp->MinXAxis[1] = dword_1E2F2C / 6;
        dinp->MinYAxis[1] = dword_1E2F30 / 6;
        dinp->DeviceType[1] = 112;
        dinp->NumberOfButtons[1] = 4;
        dinp->Init[1] = 1;
        dinp->ConfigType[1] = loc_jtype;

        dinp->Type = loc_jtype;
        result = 1;
        break;
    default:
        dinp->Type = loc_jtype;
        result = 1;
        break;
    }
    return result;
}

/** Gravis Grip joystick driver initialization.
 *
 * GrIP joystick support library initialization.
 * Real-mode implementation based on "GrIP API Demo Program" from SDK.
 */
int joy_grip_init(void)
{
#if defined(DOS)||defined(GO32)
    // Requires declarations from "joy_grip.h"

    // Full path for GrIP library
    char lib[128];
    // File handle for library
    FILE* flib;
    // Size of library file (bytes)
    long size;
    // Memory image of library file
    void* image;

    // Search for the GrIP library: GRIP.GLL
    //
    // First search in the current directory, then search in any directories
    // specified by the "SET GRIP=" environment variable, then search in any
    // directories in the path, and finally search in "C:\GRIP\".
    //
    // Note that the filename for the library ("GRIP.GLL"), the environment
    // variable name ("GRIP") and the default install directory ("C:\GRIP\")
    // are defined in the module GRIP.C as GrLibName, GrLibEnv, and
    // GrLibDir respectively.
    _searchenv(GrLibName, GrLibEnv, lib);
    if (lib[0] == 0)
        _searchenv(GrLibName, "PATH", lib);
    if (lib[0] == 0) {
        strcpy(lib, GrLibDir);
        strcat(lib, GrLibName);
    }
    // Open the loadable library file and determine its size
    if ((flib = fopen(lib, "rb")) == 0)
    {
        LOGERR("Unable to load library file \"%s\"", lib);
        return -1;
    }
    fseek(flib, 0, SEEK_END);
    size = ftell(flib);

    // Allocate memory for the library and read it all into memory
    //
    // (If the size is greater than 32k, then this is definitely not
    // the correct file, so signal an error.)
    if (size > 32*1024L)
    {
        LOGERR("Invalid library file \"%s\" (File too large)", lib);
        return -1;
    }
    image = LbMemoryAlloc(size);
    if (image == NULL)
    {
        LOGERR("Unable to allocate sufficient memory for library", lib);
        return -1;
    }
    fseek(flib, 0, SEEK_SET);
    fread(image, 1, size, flib);
    fclose(flib);

    // Link to the library (and free up the temporary memory)
    if (!GrLink(image, size))
    {
        LOGERR("Unable to initialize loadable library!");
        return -1;
    }
    LbMemoryFree(image);
    return 1;
#else
    // No joystick support lib required for modern OSes
    return 0;
#endif
}

int joy_grip_shutdown(void)
{
#if defined(DOS)||defined(GO32)
    // Shutdown the GrIP system
    GrShutdown();
    // Disconnect from the GrIP library
    GrUnlink();
    return 1;
#else
    // No joystick support lib required for modern OSes
    return 0;
#endif
}

/** SpaceBall from Spacetec IMC game controller driver initialization.
 */
int joy_spaceball_init(void)
{
#if defined(DOS)||defined(GO32)
    // Requires declarations from "joy_splib.h"
    short ret;
    ret = SpwSimpleOpen(0);
    return (ret != 0);
#else
    // No joystick support lib required for modern OSes
    return 0;
#endif
}

int joy_spaceball_shutdown(void)
{
#if defined(DOS)||defined(GO32)
    short ret;
    ret = SpwSimpleClose(0);
    return (ret != 0);
#else
    // No joystick support lib required for modern OSes
    return 0;
#endif
}

/** Joystick drivers initialization.
 */
int joy_driver_init(void)
{
    int ret;
    if (!joy_grip_initialized)
    {
        ret = joy_grip_init();
        if (ret == 1)
            joy_grip_initialized = 1;
    }
    if (!joy_spbal_initialized)
    {
        ret = joy_spaceball_init();
        if (ret == 1)
            joy_spbal_initialized = 1;
    }
    return 1;
}

/** Joystick drivers shutdown.
 */
int joy_driver_shutdown(void)
{
    if (joy_grip_initialized)
    {
        joy_grip_shutdown();
        joy_grip_initialized = 0;
    }
    if (joy_spbal_initialized)
    {
        joy_spaceball_shutdown();
        joy_spbal_initialized = 0;
    }
    return 1;
}
/******************************************************************************/
