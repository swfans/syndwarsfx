/******************************************************************************/
// Syndicate Wars Fan Expansion, source port of the classic game from Bullfrog.
/******************************************************************************/
/** @file network.c
 *     Network support.
 * @par Purpose:
 *     Implement functions for handling multiplayer exchange.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     19 Apr 2022 - 27 May 2022
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "network.h"

#include <assert.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "bfdos.h"
#include "bfmemory.h"
#include "bfmemut.h"
#include "bftime.h"

#include "display.h"
#include "dos.h"
#include "keyboard.h"
#include "swlog.h"
/******************************************************************************/
#pragma pack(1)

struct IPXSession { // sizeof=45
    NSESS_HANDLE Id; // offset=0
    ushort GameId; // offset=2
    char Name[8]; // offset=4
    ubyte Reserved[31]; // offset=12
    ubyte HostPlayerNumber; // offset=43
    ubyte field_44; // offset=44
};

struct IPXPlayer { // sizeof=28
    ulong Res0; // offset=0
    ulong Res4; // offset=4
    ushort Res8; // offset=8
    char Name[16]; // offset=10
    ushort Used; // offset=26
};

struct IPXSessionList { // sizeof=271
    struct IPXSession Session; // offset=0
    struct IPXPlayer Player[8]; // offset=45
    ubyte NumberOfPlayers; // offset=269
    ubyte field_270; // offset=270
};

struct ModemResponse { // sizeof=0x2C
    char msg[40];
    ulong code;
};

#pragma pack()
/******************************************************************************/

ubyte lbICommSessionActive;
extern struct TbIPXHandler *IPXHandler;
extern struct TbIPXPlayer IPXPlayer;

char ModemResponseString[80];
char ModemRequestString[80];
extern struct TbUnknCommSt netunkst_1E81E0;

extern struct TbSerialDev *dword_1E85E3;

extern ulong ipx_send_packet_count[8][8];
extern ulong ipx_got_player_send_packet_count[8];
extern struct TbIPXPlayerHeader loggon_header;
extern ubyte player_loggon;

ubyte net_players_num = 1;

extern struct ComHandlerInfo com_dev[4];
extern struct IPXDatagramBackup datagram_backup[8];

struct TbNetworkService NetworkServicePtr;

const struct ModemResponse modem_response[] = {
    {"OK", 1},
    {"CONNECT", 2},
    {"RING", 3},
    {"BUSY", 4},
    {"ERROR", 5},
    {"NO CARRIER", 6},
    {"NO DIALTONE", 7},
    {"", 0},
};

struct ModemCommand modem_cmds[] = {
    {"ATZ"},
    {"ATD"},
    {"ATH"},
    {"ATS0=1"},
};

struct NetworkServiceInfo Network_Service_List[] = {
    {0, 0, NetSvc_IPX, 0x02, 0},
    {0, 0, NetSvc_COM1, 0x01, 0},
    {0, 0, NetSvc_COM2, 0x01, 0},
    {0, 0, NetSvc_COM3, 0x01, 0},
    {0, 0, NetSvc_COM4, 0x01, 0},
    {0, 0, NetSvc_RADICA, 0x02, 0},
    {0, 0, 0, 0, 0},
};

/******************************************************************************/
TbResult LbNetworkSetSessionCreateFunction(void *func)
{
    NetworkServicePtr.F.SessionCreate = func;
    return Lb_SUCCESS;
}

TbResult LbNetworkSetSessionJoinFunction(void *func)
{
    NetworkServicePtr.F.SessionJoin = func;
    return Lb_SUCCESS;
}

TbResult LbNetworkSetSessionExchangeFunction(void *func)
{
    NetworkServicePtr.F.SessionExchange = func;
    return Lb_SUCCESS;
}

TbResult LbNetworkSetSessionUnk2CFunction(void *func)
{
    NetworkServicePtr.F.netsvcfunc_unkn20 = func;
    return Lb_SUCCESS;
}

TbResult LbNetworkSetSessionInitFunction(void *func)
{
    NetworkServicePtr.F.SessionInit = func;
    return Lb_SUCCESS;
}

TbResult LbNetworkSetSessionDialFunction(void *func)
{
    NetworkServicePtr.F.SessionDial = func;
    return Lb_SUCCESS;
}

TbResult LbNetworkSetSessionAnswerFunction(void *func)
{
    NetworkServicePtr.F.SessionAnswer = func;
    return Lb_SUCCESS;
}

TbResult LbNetworkSetSessionHangUpFunction(void *func)
{
    NetworkServicePtr.F.SessionHangUp = func;
    return Lb_SUCCESS;
}

TbResult LbNetworkSetTimeoutSec(ulong tmsec)
{
    NetTimeoutTicks = 100 * tmsec;
    return Lb_SUCCESS;
}

TbBool ipx_is_initialized(void)
{
    return (IPXHandler != NULL);
}

#if defined(DOS)||defined(GO32)

int CallIPX(ubyte a1)
{
    struct DPMI_REGS dpmi_regs;
    union REGS regs;
    struct SREGS sregs;

    IPXHandler->field_2 = a1;
    memset(&dpmi_regs, 0, sizeof(struct DPMI_REGS));
    memset(&regs, 0, sizeof(union REGS));
    /* Use DMPI call 300h to issue the DOS interrupt */
    regs.x.eax = 0x300;
    regs.x.ebx = IPXHandler->InterruptNo;
    segread(&sregs);
    regs.x.edi = (unsigned int)&dpmi_regs;
    return int386x(0x31, &regs, &regs, &sregs);
}

static ubyte CallRealModeInterrupt(ubyte intno, struct DPMI_REGS *dpmi_regs)
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

#endif

void setup_bullfrog_header(struct TbIPXPlayerHeader *ipxhead, int a2)
{
    struct TbIPXHandler *ipxhndl;

    ipxhndl = IPXHandler;
    ipxhead->Magic[0] = 'B';
    ipxhead->Magic[1] = 'U';
    ipxhead->field_2A = a2;
    memcpy(ipxhead->field_1C, ipxhndl->field_2E, sizeof(ipxhead->field_1C));
    memcpy(&ipxhead->field_20, &ipxhndl->field_32, sizeof(ipxhead->field_20));
    memcpy(ipxhead->field_22, &ipxhndl->field_2A, sizeof(ipxhead->field_22));
}

void ipx_add_new_player(struct TbIPXPlayerHeader *p_ipxhead)
{
    asm volatile ("call ASM_ipx_add_new_player\n"
        : : "a" (p_ipxhead) );
}

void ipx_log_on_new_players(void)
{
    short i;
#if defined(DOS)||defined(GO32)
    CallIPX(1);
#endif
    for (i = 0; i < 30; i++)
    {
        struct TbIPXPlayer *p_ipxplyr;

        if (IPXHandler->field_46[i] == 0)
            continue;

        IPXHandler->field_46[i] = 0;
        p_ipxplyr = (struct TbIPXPlayer *)&IPXHandler->PlayerData[i];

        if (strncasecmp(p_ipxplyr->Header.Magic, "BU", 2) != 0)
            continue;
        if (p_ipxplyr->Header.field_2A == 2)
            continue;

        if (IPXPlayer.Data.num_players < 8) {
            ipx_add_new_player(&p_ipxplyr->Header);
        }
    }
}

void ipx_update(void)
{
    static TbClockMSec start_time = 0;
    TbClockMSec curr_time;

    if (IPXHandler->SessionActive != 1) {
        return;
    }
    if (IPXHandler->field_C != IPXHandler->field_D) {
        return;
    }
    if (start_time == 0)
        start_time = LbTimerClock();
    curr_time = LbTimerClock();
    if (curr_time - start_time > 100)
    {
        start_time = curr_time;
        IPXPlayer.Header.field_2A = 1;
        memcpy(&IPXHandler->PlayerData[0], &IPXPlayer, sizeof(struct TbIPXPlayer));
        IPXHandler->PlayerDataSize = sizeof(struct TbIPXPlayer);
#if defined(DOS)||defined(GO32)
        CallIPX(3);
#endif
    }

    if (IPXPlayer.Data.num_players == 1)
    {
        ipx_log_on_new_players();
    }
    else if (IPXPlayer.Data.num_players > 1)
    {
        if (player_loggon) {
            player_loggon = 0;
            ipx_add_new_player(&loggon_header);
        }
    }
}

void ipx_service_init(ushort a1)
{
    struct TbIPXHandler *ipxhndl;

    LOGDBG("Starting");
    ipxhndl = IPXHandler;
    ipxhndl->field_8 = a1;
#if defined(DOS)||defined(GO32)
    CallIPX(8);
#endif
}


TbResult ipx_create_session(char *a1, const char *a2)
{
    struct TbIPXHandler *ipxhndl;
    struct TbIPXPlayer *p_plyrdt;
    ulong tm_start, tm_curr;
    TbResult ret;
    int i;

    LOGDBG("Starting");
    if (IPXHandler->SessionActive != 0) {
        LOGERR("Already have IPX session");
        return Lb_FAIL;
    }

    tm_start = clock();
    while (1)
    {
        tm_curr = clock();
#if defined(DOS)||defined(GO32)
        CallIPX(1);
#endif
        for (i = 1; i < 30; i++)
        {
            ipxhndl = IPXHandler;
            if (!ipxhndl->field_46[i])
                continue;
            ipxhndl->field_46[i] = 0;

            p_plyrdt = (struct TbIPXPlayer *)&ipxhndl->PlayerData[i];
            if (IPXPlayer.Header.field_2 == p_plyrdt->Header.field_2)
            {
                if (strcasecmp(p_plyrdt->Header.field_4, a1) == 0) {
                    LOGERR("String same as remote");
                    return Lb_FAIL;
                }
            }
        }

        ret = 0;
        if (NetworkServicePtr.F.SessionCreate != NULL)
            ret = NetworkServicePtr.F.SessionCreate();
        if (ret == -7) {
          LOGERR("Service callback error %d", (int)ret);
          return ret;
        }

        if (tm_curr - tm_start >= 300)
            break;
    }

    for (i = 0; i < 16; i++)
    {
        ubyte val;

        val = a2[i];
        if (val == 0)
        {
              IPXHandler->field_34[i] = 0;
              break;
        }
        IPXHandler->field_34[i] = val;
    }

    ipxhndl = IPXHandler;
    IPXHandler->field_34[15] = 0;
    setup_bullfrog_header(&IPXPlayer.Header, 1);
    IPXHandler->field_44 = 1;

    if (strlen(a1) > 7)
            a1[7] = 0;
    strcpy(IPXPlayer.Header.field_4, a1);

    memcpy(&IPXPlayer.Data, &IPXHandler->field_2A, 0x1Cu);
    strcpy(IPXPlayer.Header.field_C, IPXHandler->field_34);

    memset(ipx_send_packet_count, 0, 0x100u);
    memset(ipx_got_player_send_packet_count, 0, 0x20u);
    memset(datagram_backup, 0, sizeof(struct IPXDatagramBackup) * 8);

    ipxhndl = IPXHandler;
    ipxhndl->SessionActive = 1;
    ipxhndl->field_C = 0;
    ipxhndl->field_D = 0;
    IPXPlayer.Data.num_players = 1;
    IPXPlayer.Header.field_26 = 0;

    return Lb_SUCCESS;
}

int ipx_session_list(struct IPXSessionList *sesslist, int listlen)
{
    int ret;
    int i, k;
    ushort n;

    if (IPXHandler->SessionActive != 0) {
        LOGERR("Already have IPX session");
        return Lb_FAIL;
    }

#if defined(DOS)||defined(GO32)
    CallIPX(1);
#endif

    ret = -1;
    n = 0;
    for (i = 1; i < 30; i++)
    {
        struct IPXSessionList *p_ipxsess;
        struct TbIPXPlayer *p_plyrdt;

        if (!IPXHandler->field_46[i])
            continue;
        IPXHandler->field_46[i] = 0;

        p_plyrdt = (struct TbIPXPlayer *)&IPXHandler->PlayerData[i];

        if (strncasecmp(p_plyrdt->Header.Magic, "BU", 2) != 0)
            continue;
        if (p_plyrdt->Header.field_2 != IPXPlayer.Header.field_2)
            continue;
        if (p_plyrdt->Header.field_2A != 1)
            continue;

        for (k = 0; k < n; k++)
        {
            p_ipxsess = &sesslist[k];
            if (memcmp(p_plyrdt->Header.field_1C, &p_ipxsess->Session.Reserved[16], 6) == 0)
            {
                k = -1;
                memcpy(p_ipxsess, p_plyrdt, sizeof(struct IPXSessionList));
                break;
            }
        }
        if (k != -1)
        {
            p_ipxsess = &sesslist[n];
            memcpy(p_ipxsess, p_plyrdt, sizeof(struct IPXSessionList));
            ret = ++n;
            if (n >= listlen)
                break;
        }
    }

    return ret;
}

int ipx_join_session(struct IPXSessionList *p_ipxsess, char *a2)
{
    struct TbIPXPlayerHeader ipxhead;
    ulong tm_start, tm_curr;
    TbResult ret;
    ushort my_plyr;
    short i, k;

    LOGDBG("Starting");
    if (IPXHandler->SessionActive != 0) {
        LOGERR("Already have IPX session");
        return Lb_FAIL;
    }

    ret = Lb_FAIL;
    memset(&ipxhead, 0, 45u);
    ipxhead.Magic[0] = 'B';
    ipxhead.Magic[1] = 'U';
    ipxhead.field_2A = 2;
    memcpy(ipxhead.field_1C, IPXHandler->field_2E, sizeof(ipxhead.field_1C));
    memcpy(&ipxhead.field_20, &IPXHandler->field_32, sizeof(ipxhead.field_20));
    memcpy(ipxhead.field_22, &IPXHandler->field_2A, sizeof(ipxhead.field_22));
    strcpy(ipxhead.field_C, a2);
    strcpy(ipxhead.field_4, p_ipxsess->Session.Name);
    ipxhead.field_2 = IPXPlayer.Header.field_2;
    my_plyr = 0;

    tm_start = clock();
    while ( 1 )
    {
        memcpy(IPXHandler->PlayerData, &ipxhead, sizeof(struct TbIPXPlayerHeader));
        IPXHandler->field_B = 45;
        memcpy(IPXHandler->field_12, &p_ipxsess->Session.Reserved[16], 6u);
        memcpy(IPXHandler->field_E, &p_ipxsess->Session.Reserved[22], sizeof(IPXHandler->field_E));
#if defined(DOS)||defined(GO32)
        CallIPX(2);
        CallIPX(1);
#endif

        for (i = 1; i < 30; i++)
        {
            struct TbIPXPlayer *p_plyrdt;

            if (!IPXHandler->field_46[i])
                continue;
            IPXHandler->field_46[i] = 0;

            p_plyrdt = (struct TbIPXPlayer *)&IPXHandler->PlayerData[i];

            if (strncasecmp(p_plyrdt->Header.Magic, "BU", 2) != 0)
                continue;
            if (p_plyrdt->Header.field_2 != IPXPlayer.Header.field_2)
                continue;
            if (p_plyrdt->Header.field_2A != 1)
                continue;
            if (strcasecmp(p_plyrdt->Header.field_4, p_ipxsess->Session.Name) != 0)
                continue;

            for (k = 0; k < NET_PLAYERS_COUNT; k++)
            {
                struct TbIPXOnePlayer *p_nplyr;
                p_nplyr = &p_plyrdt->Data.Data3.player[k];
                if (memcmp(p_nplyr->field_4, ipxhead.field_1C, 6) == 0)
                {
                    tm_start = 0;
                    my_plyr = k;
                    ret = 1;
                    i = 31;
                    break;
                }
            }
        }

        if (ret != 1)
        {
            int ret2;
            ret2 = 0;
            if (NetworkServicePtr.F.SessionJoin)
                ret2 = NetworkServicePtr.F.SessionJoin();
            if (ret2 == -7)
                return ret2;
        }
        tm_curr = clock();
        if (tm_curr - tm_start > 500)
        {
            break;
        }
    }
    if (ret != -1)
    {
        struct TbIPXPlayer *p_plyrdt;

        p_plyrdt = (struct TbIPXPlayer *)&IPXHandler->PlayerData[i];

        IPXPlayer.Header.Magic[0] = 'B';
        IPXPlayer.Header.Magic[1] = 'U';
        IPXPlayer.Header.field_2A = 0;
        memcpy(IPXPlayer.Header.field_1C, IPXHandler->field_2E, sizeof(IPXPlayer.Header.field_1C));
        memcpy(&IPXPlayer.Header.field_20, IPXHandler->field_2E + 4, sizeof(IPXPlayer.Header.field_20));
        memcpy(IPXPlayer.Header.field_22, &IPXHandler->field_2A, sizeof(IPXPlayer.Header.field_22));
        IPXHandler->field_C = my_plyr;
        IPXHandler->SessionActive = 1;
        IPXHandler->field_D = p_plyrdt->Header.field_2B;
        IPXPlayer.Data.num_players = p_plyrdt->Data.num_players;
        IPXPlayer.Data.field_10E = p_plyrdt->Data.field_10E;
        IPXPlayer.Header.field_26 = p_plyrdt->Header.field_26;
        IPXPlayer.Header.field_2B = my_plyr;
        strcpy(IPXPlayer.Header.field_C, a2);
        strcpy(IPXPlayer.Header.field_4, p_plyrdt->Header.field_4);
        memcpy(&IPXPlayer.Data, &p_plyrdt->Data, 0xE0u);
        memset(ipx_send_packet_count, 0, 0x100u);
        memset(ipx_got_player_send_packet_count, 0, 0x20u);
        memset(datagram_backup, 0, 0x1130u);
    }
    return ret;
}

int ipx_get_host_player_number(void)
{
    if (!ipx_is_initialized()) {
        LOGERR("Called before IPX initialization");
        return -1;
    }
    if (IPXHandler->SessionActive == 0) {
        LOGERR("Called without active session");
        return -1;
    }
    return IPXHandler->field_D;
}

int ipx_get_player_number(void)
{
    if (!ipx_is_initialized()) {
        LOGERR("Called before IPX initialization");
        return -1;
    }
    if (IPXHandler->SessionActive == 0) {
        LOGERR("Called without active session");
        return -1;
    }
    return IPXPlayer.Header.field_2B;
}

int ipx_exchange_packets(void *a1, int a2)
{
    int ret;
    asm volatile ("call ASM_ipx_exchange_packets\n"
        : "=r" (ret) : "a" (a1), "d" (a2) );
    return ret;
}

void ipx_shutdown(ushort a1)
{
    struct TbIPXHandler *p_ipxhndl;
    int i;

    LOGDBG("Starting");
    p_ipxhndl = IPXHandler;

    p_ipxhndl->field_8 = a1;
#if defined(DOS)||defined(GO32)
    CallIPX(9);

    for (i = 0; i < 40; i++)
    {
        CallIPX(1);
    }
#else
    (void)i; // unused
#endif
}

void ipx_shutdown_listeners(void)
{
    LOGDBG("Starting");
    if (!ipx_is_initialized()) {
        LOGERR("Called before IPX initialization");
        return;
    }
    if (IPXHandler->SessionActive == 0) {
        LOGERR("Called without active session");
        return;
    }
    IPXHandler->SessionActive = 2;
}

void ipx_openup_listeners(void)
{
    LOGDBG("Starting");
    if (!ipx_is_initialized()) {
        LOGERR("Called before IPX initialization");
        return;
    }
    if (IPXHandler->SessionActive == 0) {
        LOGERR("Called without active session");
        return;
    }
    IPXHandler->SessionActive = 1;
}

int ipx_stop_network(void)
{
    int ret;
    LOGDBG("Starting");
    asm volatile ("call ASM_ipx_stop_network\n"
        : "=r" (ret) : );
    return ret;
}

TbResult ipx_get_player_name(char *name, int plyr)
{
#if 0
    int ret;
    asm volatile ("call ASM_ipx_get_player_name\n"
        : "=r" (ret) : "a" (name), "d" (plyr) );
    return ret;
#endif
    if (plyr >= 8)
        return Lb_FAIL;
    if (IPXHandler == NULL)
        return Lb_FAIL;
    if (!IPXHandler->SessionActive)
        return Lb_FAIL;
    if (!IPXPlayer.Data.Data3.player[plyr].field_1A)
        return Lb_FAIL;

    strcpy(name, IPXPlayer.Data.Data3.player[plyr].name);
    return Lb_SUCCESS;
}

TbResult ipx_network_send(int plyr, ubyte *data, int dtlen)
{
    assert(!"Not implemented");
    return Lb_FAIL;
}

TbResult ipx_network_receive(int plyr, ubyte *data, int dtlen)
{
    assert(!"Not implemented");
    return Lb_FAIL;
}

TbResult ipx_send_packet_to_player_nowait(int plyr, ubyte *data, int dtlen)
{
    struct TbIPXHandler *p_ipxhndl;
    struct TbIPXPlayer *p_plyrdt;

    LOGDBG("Starting");
    if (plyr >= 8) {
        LOGERR("Target player invalid");
        return Lb_FAIL;
    }
    if (IPXPlayer.Data.Data1.Sub1[plyr].field_47) {
        LOGERR("Cond 1 triggered");
        return Lb_FAIL;
    }

    p_ipxhndl = IPXHandler;
    p_plyrdt = (struct TbIPXPlayer *)&IPXHandler->PlayerData[0];

    IPXPlayer.Header.field_2A = 4;
    memcpy(&p_plyrdt->Header, &IPXPlayer.Header, sizeof(struct TbIPXPlayerHeader));
    memcpy(&p_plyrdt->Data, data, dtlen);
    p_ipxhndl->PlayerDataSize = sizeof(struct TbIPXPlayerHeader) + dtlen;

    memcpy(&p_ipxhndl->field_12[0], &IPXPlayer.Data.Data2.Sub1[plyr].field_2D[4], 6);
    memcpy(p_ipxhndl->field_E, &IPXPlayer.Data.Data2.Sub1[plyr].field_2D[0], sizeof(p_ipxhndl->field_E));
#if defined(DOS)||defined(GO32)
    CallIPX(2);
#endif
    return Lb_SUCCESS;
}

TbResult ipx_receive_packet_from_player_nowait(int plyr, ubyte *data, int dtlen)
{
    TbResult ret;
    int i;
  
    ret = Lb_OK;
    for (i = 1; i < 30; i++)
    {
        struct TbIPXPlayer *p_plyrdt;

        if (!IPXHandler->field_46[i])
            continue;
        IPXHandler->field_46[i] = 0;

        p_plyrdt = (struct TbIPXPlayer *)&IPXHandler->PlayerData[i];

        if (strncasecmp(p_plyrdt->Header.Magic, "BU", 2) != 0)
            continue;
        if (strcasecmp(p_plyrdt->Header.field_4, IPXPlayer.Header.field_4) != 0)
            continue;
        if (p_plyrdt->Header.field_2 != IPXPlayer.Header.field_2)
            continue;
        if ((p_plyrdt->Header.field_2A != 4) || (p_plyrdt->Header.field_2B != plyr))
            continue;

        memcpy(data, &p_plyrdt->Data, dtlen);
        ret = Lb_SUCCESS;
        break;
    }
    return ret;
}

TbResult ipx_send_packet_to_player_wait(int plyr, ubyte *data, int dtlen)
{
    ubyte *dt;
    int i;
    ushort pkt_count;

    LOGDBG("Starting");
    if (!IPXPlayer.Header.field_2C) {
        LOGERR("Cond 1 not met");
        return Lb_OK;
    }
    if ((plyr >= NET_PLAYERS_COUNT && plyr != 0xFFFF)
      || (plyr == IPXPlayer.Header.field_2B)
      || (plyr != 0xFFFF && !IPXPlayer.Data.Data1.Sub1[plyr].field_47) )
    {
        LOGERR("Target player invalid");
        return Lb_OK;
    }

    dt = data;
    pkt_count = dtlen / 400;
    for (i = 0; i < pkt_count; i++)
    {
        ipx_network_send(plyr, dt, 400);
        dt += 400;
    }
    if (dtlen % 400)
    {
        ipx_network_send(plyr, dt, dtlen % 400);
    }
    return Lb_SUCCESS;
}

TbResult ipx_receive_packet_from_player_wait(int plyr, ubyte *data, int dtlen)
{
    ubyte *dt;
    int i;
    ushort pkt_count;

    LOGDBG("Starting");
    if (!IPXPlayer.Header.field_2C) {
        LOGERR("Cond 1 not met");
        return Lb_OK;
    }
    if ((plyr >= NET_PLAYERS_COUNT)
      || (plyr == IPXPlayer.Header.field_2B)
      || (!IPXPlayer.Data.Data1.Sub1[plyr].field_47) )
    {
        LOGERR("Target player invalid");
        return Lb_OK;
    }

    dt = data;
    pkt_count = dtlen / 400;
    for (i = 0; i < pkt_count; i++)
    {
        ipx_network_receive(plyr, dt, 400);
        dt += 400;
    }
    if (dtlen % 400)
    {
        ipx_network_receive(plyr, dt, dtlen % 400);
    }
    return Lb_SUCCESS;
}

int ipx_session_list_conv(struct TbNetworkSessionList *p_nslist, int listlen)
{
    struct TbNetworkSessionList *p_nslent;
    struct IPXSessionList ipxsess;
    int n_ent;
    int i, k;

    memset(&ipxsess, 0, sizeof(struct IPXSessionList));

    n_ent = 0;
    p_nslent = p_nslist;
    for (i = 0; i < listlen; i++)
    {
        TbResult ret;

        ret = ipx_session_list(&ipxsess, 1);
        memset(p_nslent, 0, sizeof(struct TbNetworkSessionList));
        if (ret != Lb_SUCCESS)
            continue;
        p_nslent->NumberOfPlayers = ipxsess.NumberOfPlayers;
        p_nslent->Session.GameId = ipxsess.Session.GameId;
        p_nslent->Session.HostPlayerNumber = ipxsess.Session.HostPlayerNumber;
        strcpy(p_nslent->Session.Name, ipxsess.Session.Name);

        for (k = 0; k < NET_PLAYERS_COUNT; k++)
        {
            if (ipxsess.Player[k].Used)
            {
                p_nslent->Player[k].Id = 1;
                p_nslent->Player[k].PlayerNumber = k;
                strcpy(p_nslent->Player[k].Name, ipxsess.Player[k].Name);

                if (k == ipxsess.Session.HostPlayerNumber) {
                    memcpy(&p_nslent->Session.Reserved[0], &ipxsess.Player[k].Res0, 4);
                    memcpy(&p_nslent->Session.Reserved[4], &ipxsess.Player[k].Res4, 4);
                    memcpy(&p_nslent->Session.Reserved[8], &ipxsess.Player[k].Res8, 2);
                }
            }
        }
        n_ent++;
        p_nslent++;
    }
    return n_ent;
}

int net_unkn_func_352(void)
{
    int ret;
    LOGDBG("Starting");
    asm volatile ("call ASM_net_unkn_func_352\n"
        : "=r" (ret) :  );
    return ret;
}

int net_unkn_func_338(struct TbUnknCommSt *p_a1)
{
    int ret;
    LOGDBG("Starting");
    asm volatile ("call ASM_net_unkn_func_338\n"
        : "=r" (ret) : "a" (p_a1) );
    return ret;
}

int radica_create_session(struct TbNetworkSession *session, const char *a2)
{
    int ret;
    LOGDBG("Starting");
    asm volatile ("call ASM_radica_create_session\n"
        : "=r" (ret) : "a" (session), "d" (a2) );
    return ret;
}

int radica_join_session(struct TbNetworkSession *session, char *a2)
{
    int ret;
    LOGDBG("Starting");
    asm volatile ("call ASM_radica_join_session\n"
        : "=r" (ret) : "a" (session), "d" (a2) );
    return ret;
}

int radica_update(void)
{
    assert(!"Not implemented");
    return Lb_SUCCESS;
}

TbResult radica_service_init(struct NetworkServiceInfo *p_nsvc)
{
    TbResult ret;
    LOGDBG("Starting");
    asm volatile ("call ASM_radica_service_init\n"
        : "=r" (ret) : "a" (p_nsvc) );
    return ret;
}

int radica_session_list(struct TbNetworkSessionList *p_nslist, int listlen)
{
    int ret;
    LOGDBG("Starting");
    asm volatile ("call ASM_radica_session_list\n"
        : "=r" (ret) : "a" (p_nslist), "d" (listlen) );
    return ret;
}

int radica_exchange_packets(void *a1, int a2)
{
    int ret;
    asm volatile ("call ASM_radica_exchange_packets\n"
        : "=r" (ret) : "a" (a1), "d" (a2) );
    return ret;
}

int radica_shutdown(void)
{
    LOGSYNC("Quitting RADICA");
    return net_unkn_func_352();
}

int SetBps(struct TbSerialDev *p_serdev, int rate)
{
    if (rate < 300)
        rate = 300;
    else if (rate > 115200)
        rate = 115200;

#if defined(DOS)||defined(GO32)
    cli();
    outp(p_serdev->field_1096 + 3, 131);
    outp(p_serdev->field_1096 + 1, 115200 / rate >> 8);
    outp(p_serdev->field_1096 + 0, (115200 / rate));
    outp(p_serdev->field_1096 + 3, 3);
    sti();
#endif
    return 115200 / (115200 / rate);
}

#if defined(DOS)||defined(GO32)

void write_char_no_buff(struct TbSerialDev *p_serdev, ubyte c)
{
    while ( !(inp(p_serdev->field_1096 + 5) & 0x20) )
        ;
    cli();
    outp(p_serdev->field_1096, c);
    sti();
}

void write_char(struct TbSerialDev *p_serdev, ubyte c)
{
    write_char_no_buff(p_serdev, c);
}

#endif

void write_string(struct TbSerialDev *p_serdev, const char *str)
{
#if defined(DOS)||defined(GO32)
    uint i;
    ubyte c;

    for (i = 0; i < strlen(str); i++)
    {
        c = str[i];
        write_char(p_serdev, c);
    }
#else
    // On Windows, WriteFile() should be used
    // On Linux, write the device file with standard file ops
    assert(!"not implemented");
#endif
}

void write_buffer(struct TbSerialDev *p_serdev, const ubyte *buf, uint buflen)
{
#if defined(DOS)||defined(GO32)
    uint i;
    ubyte c;

    for (i = 0; i < buflen; i++)
    {
        c = buf[i];
        write_char(p_serdev, c);
    }
#elif defined(WIN32)
    // On Windows, WriteFile() should be used
    //uint written;
    //TbBool ret;
    //ret = WriteFile(p_serdev->Handle, buf, buflen, &written, NULL);
    assert(!"not implemented");
#else
    // On Linux, write the device file with standard file ops
    assert(!"not implemented");
#endif
}

uint read_buffer(struct TbSerialDev *p_serdev, const ubyte *buf, uint buflen, uint a3)
{
#if defined(DOS)||defined(GO32)
    uint i;

    for (i = 0; i < buflen; i++)
    {
        if (!net_unkn_func_02(&buf[i], a3))
            break;
    }
    return i;
#elif defined(WIN32)
    // On Windows, ReadFile() should be used
    //uint received;
    //TbBool ret;
    //ret = ReadFile(p_serdev->Handle, buf, buflen, &received, NULL);
    assert(!"not implemented");
#else
    // On Linux, write the device file with standard file ops
    assert(!"not implemented");
#endif
}

void read_write_clear_flag(struct TbSerialDev *p_serdev, ushort port, ubyte c)
{
#if defined(DOS)||defined(GO32)
    ubyte val;
    val = inp(port) & ~c;
    outp(port, val);
#else
    assert(!"not implemented");
#endif
}

void read_write_set_flag(struct TbSerialDev *p_serdev, ushort port, ubyte c)
{
#if defined(DOS)||defined(GO32)
    ubyte val;
    val = inp(port) | c;
    outp(port, val);
#else
    assert(!"not implemented");
#endif
}

void wait(ulong msec)
{
#if defined(DOS)||defined(GO32)
    delay(msec);
#else
    usleep(msec * 1000);
#endif
}

void send_string(struct TbSerialDev *p_serdev, const char *str)
{
    char locstr[80];

    strcpy(locstr, str);
    strcat(locstr, "\r");
    write_string(p_serdev, locstr);
    strcpy(ModemRequestString, locstr);
}

uint net_unkn_callback1(ubyte *data, uint datalen)
{
    write_buffer(dword_1E85E3, data, datalen);
    return datalen;
}

uint net_unkn_callback2(ubyte *data, uint datalen, uint a3)
{
    uint len;
    len = read_buffer(dword_1E85E3, data, datalen, a3);
    return len;
}

void inbuf_pos_inc(struct TbSerialDev *p_serdev)
{
    int pos;
    pos = ++p_serdev->inbuf_pos;
    if (pos > 2047)
        p_serdev->inbuf_pos = 0;
}

int read_char(struct TbSerialDev *p_serdev)
{
    ubyte chr;
    if (p_serdev->inbuf_pos == p_serdev->field_109E)
        return -1;
    chr = p_serdev->inbuf[p_serdev->inbuf_pos];
    inbuf_pos_inc(p_serdev);
    return chr;
}

int get_modem_response(struct TbSerialDev *p_serdev)
{
    const struct ModemResponse *resp;
    char locstr[80];
    TbClockMSec start_time;
    int ret;
    int chr;
    ushort lspos, mrpos;
    TbBool done;

    ret = -1;
    done = false;
    mrpos = 0;
    start_time = LbTimerClock();
    memset(locstr, 0, sizeof(locstr));
    lspos = 0;
    while (!done)
    {
        if (LbTimerClock() - start_time > 4000)
            return -1;
        chr = read_char(p_serdev);
        if (mrpos >= strlen(ModemRequestString)) {
            mrpos = 0;
        } else if (chr == ModemRequestString[mrpos]) {
            chr = -1;
            mrpos++;
        }
        if (chr == -1)
        {
            if (NetworkServicePtr.F.UsedSessionInit) {
                // TODO not sure if UsedSessionInit gets the chr as parameter
                if (NetworkServicePtr.F.UsedSessionInit() == -7)
                    return -7;
            }
            continue;
        }
        if ((chr == 10 && lspos == 0) || (lspos >= sizeof(locstr)))
        {
            for (resp = modem_response; resp->code != 0; resp++)
            {
                if (strncasecmp(locstr, resp->msg, strlen(resp->msg)) == 0)
                {
                    ret = resp->code;
                    strcpy(ModemResponseString, locstr);
                    done = true;
                    break;
                }
            }
            if (done)
                break; // break the main loop
            lspos = 0;
            memset(locstr, 0, sizeof(locstr));
        }

        if (chr >= 32)
        {
            locstr[lspos] = chr;
            lspos++;
        }
        // Periodically trigger user callback, if set
        if (NetworkServicePtr.F.UsedSessionInit != NULL)
        {
            if (NetworkServicePtr.F.UsedSessionInit() == -7)
                return -7;
        }
    }
    return ret;
}

#if defined(DOS)||defined(GO32)

void backup_serial_int_vectors(struct TbSerialDev *p_serdev)
{
    union REGS regs;

    memset(&regs, 0, sizeof(regs));
    regs.x.eax = 0x200; // Get Real Mode Interrupt Vector
    regs.h.cl = p_serdev->field_1098;
    int386(49, &regs, &regs);
    // CX:DX = Selector:Offset of exception handler
    com_dev[p_serdev->comdev_id].field_18 = regs.x.cx;
    com_dev[p_serdev->comdev_id].field_1A = regs.x.dx;

    memset(&regs, 0, sizeof(regs));
    regs.x.eax = 0x204; // Get Protected Mode Interrupt Vector
    regs.h.cl = p_serdev->field_1098;
    int386(49, &regs, &regs);
    // CX:(E)DX = Selector:Offset of exception handler
    com_dev[p_serdev->comdev_id].field_16 = regs.x.cx;
    com_dev[p_serdev->comdev_id].field_12 = regs.x.edx;
}

void setup_serial_int_vectors(struct TbSerialDev *p_serdev)
{
    union REGS regs;
    void (*handler_real)();
    void (*handler_prot)();
    ushort handler_real_seg;
    int lock_len;
    ubyte comdev_id;

    comdev_id = p_serdev->comdev_id;
    switch (comdev_id)
    {
    case 0:
        handler_real_seg = __CS__;
        handler_real = com_handler0;
        handler_prot = com_handler0;
        break;
    case 1:
        handler_real_seg = __CS__;
        handler_real = com_handler1;
        handler_prot = com_handler1;
        break;
    case 2:
        handler_real_seg = __CS__;
        handler_real = com_handler2;
        handler_prot = com_handler2;
        break;
    case 3:
        handler_real_seg = __CS__;
        handler_real = com_handler3;
        handler_prot = com_handler3;
        break;
    }

    memset(&regs, 0, sizeof(regs));
    regs.x.eax = 0x205;
    regs.x.ebx = p_serdev->field_1098;
    regs.x.ecx = handler_real_seg;
    regs.x.edx = handler_real;
    int386(49, &regs, &regs);

    memset(&regs, 0, sizeof(regs));
    regs.x.eax = 0x201;
    regs.x.ebx = p_serdev->field_1098;
    regs.x.ecx = (((uintptr_t)p_serdev) >> 4);
    regs.x.edx = (((uintptr_t)p_serdev) - regs.x.ecx);
    int386(49, &regs, &regs);

    lock_len = ((uintptr_t)lock_code_end - (uintptr_t)lock_code_start);
    memset(&regs, 0, sizeof(regs));
    regs.x.eax = 0x600;
    regs.x.ebx = lock_code_start >> 16;
    regs.x.ecx = lock_code_start & 0xFFFF;
    regs.x.esi = 0;
    regs.x.edi = lock_len;
    int386(49, &regs, &regs);
}

void init_com_port(struct TbSerialDev *p_serdev)
{
  ushort btmask;
  ushort ival;

  btmask = 1 << p_serdev->field_109A;
  ival = inp(0x21u);
  outp(33, ival | btmask);

  outp(p_serdev->field_1096 + 4, 0);
  outp(p_serdev->field_1096 + 1, 0);
  outp(p_serdev->field_1096 + 2, 0);
  outp(p_serdev->field_1096 + 1, 1);
  outp(p_serdev->field_1096 + 4, 11);

  btmask = ~(1 << p_serdev->field_109A);
  ival = inp(0x21u);
  outp(33, ival & btmask);

  inp(p_serdev->field_1096 + 2);
  inp(p_serdev->field_1096 + 0);
  inp(p_serdev->field_1096 + 5);

  ival = inp(p_serdev->field_1096 + 6);
  sti(ival);
  SetRts(p_serdev, 1);
  SetDtr(p_serdev, 1);
  SetBps(p_serdev, 9600);
}

#else

void init_com_port(struct TbSerialDev *p_serdev)
{
    //TODO implement port init for non-DOS platforms
}

#endif

void net_unkn_post_init_1(struct TbUnknCommSt *p_a1)
{
    int i;

    p_a1->field_0 = 0;
    p_a1->field_1 = 0;
    p_a1->WriteCb = NULL;
    p_a1->ReadCb = NULL;
    p_a1->ExchangeCb = NULL;
    p_a1->field_2 = 0;
    p_a1->field_6 = 0;
    p_a1->field_110 = 0;
    p_a1->field_197 = 0;
    p_a1->field_89 = 0;
    p_a1->field_1A7 = 0;
    p_a1->field_1AB = 0;
    p_a1->field_2B7 = 0;
    p_a1->field_2AF = 0;
    p_a1->field_2B3 = 0;

    for (i = 0; i < 256; i++)
    {
        ubyte bt[4];

        bt[0] = 0;
        bt[1] = 0;
        bt[2] = 0;
        bt[3] = 0;
        if ((i & 1) != 0) {
            bt[3]++;
        }
        if ((i & 2) != 0) {
            bt[2]++;
            bt[3]++;
        }
        if ((i & 4) != 0) {
            bt[1]++;
            bt[3]++;
        }
        if ((i & 8) != 0) {
            bt[1]++;
            bt[2]++;
            bt[3]++;
        }
        if ((i & 0x10) != 0) {
            bt[0]++;
            bt[3]++;
        }
        if ((i & 0x20) != 0) {
            bt[0]++;
            bt[2]++;
            bt[3]++;
        }
        if ((i & 0x40) != 0) {
            bt[0]++;
            bt[1]++;
            bt[3]++;
        }
        if ((i & 0x80) != 0) {
            bt[0]++;
            bt[1]++;
            bt[2]++;
            bt[3]++;
        }
        p_a1->field_2BB[i] = (1 * (bt[0] & 1)) | (2 * (bt[1] & 1)) | (4 * (bt[2] & 1)) | (8 * (bt[3] & 1));
    }
    p_a1->field_3BB = 0;
    p_a1->field_3BC = 17;
    p_a1->field_3BD = 25;
    p_a1->field_3BE = 8;
    p_a1->field_3BF = 21;
    p_a1->field_3C0 = 4;
    p_a1->field_3C1 = 12;
    p_a1->field_3C2 = 29;

    for (i = 0; i < 32; i++)
    {
        p_a1->field_3C3[i] = (8 * (i & 1)) | (4 * (i & 4)) | (4 * (i & 8)) | (4 * (i & 0x10)) | ((i & 2) << 6);
    }

    for (i = 0; i < 32; i++)
    {
        p_a1->field_3E3[i] = (2 * (i & 2)) | (2 * (i & 4)) | (2 * (i & 8)) | ((i & 0x10) >> 3) | (i & 1);
    }
}

void net_unkn_post_init_2(struct TbUnknCommSt *p_a1,
        uint (*write_cb)(ubyte *, uint))
{
    p_a1->WriteCb = write_cb;
}

void net_unkn_post_init_3(struct TbUnknCommSt *p_a1,
        uint (*read_cb)(ubyte *, uint, uint))
{
    p_a1->ReadCb = read_cb;
}

void net_unkn_sub_332(struct TbUnknCommSt *p_a1,
        int (*exchange_cb)(void))
{
    p_a1->ExchangeCb = exchange_cb;
}

void net_unkn_change_state(struct TbUnknCommSt *p_a1)
{
    LOGSYNC("State Change: WAITING_FOR_ACK = ", p_a1->field_2 ? "TRUE" : "FALSE");
    LOGSYNC("  WAITING_FOR_DATA = ", p_a1->field_6 ? "TRUE" : "FALSE");
}

void net_unkn_sub_335(struct TbUnknCommSt *p_ucs1, ubyte *p_buf, uint buf_len)
{
    char locstr[512];
    uint i;

    if (p_ucs1->WriteCb == NULL) {
        return;
    }

    if (p_ucs1->field_1A7 || p_ucs1->field_2B7)
    {
        for (i = 0; i < buf_len; i++) {
            sprintf(&locstr[3 * i], "%02X ", (int)p_buf[i]);
        }
    }

    if (p_ucs1->field_1A7) {
        locstr[3 * buf_len] = '\0';
        LOGSYNC("TX DATA: %s", locstr);
    }

    if (p_ucs1->field_2B7)
    {
        locstr[3 * buf_len] = '\n';
        locstr[3 * buf_len + 1] = '\r';
        p_ucs1->WriteCb((ubyte *)locstr, 3 * buf_len + 2);
    }
    else
    {
        p_ucs1->WriteCb(p_buf, buf_len);
    }
}

void net_unkn_sub_323(struct TbUnknCommSt *p_ucs1, ubyte *p_inpbuf, uint inpbuf_len)
{
    ubyte locdata[128];
    ubyte lochead[4];
    uint i, dtlen, pos;
    ubyte cksum;
    TbBool no_data;

    if (p_inpbuf == NULL) {
        return;
    }
    no_data = 1;
    if ((p_ucs1->field_110 != 0) && (inpbuf_len == p_ucs1->field_8D))
    {
        for (i = 0; i < inpbuf_len; i++)
        {
            if (p_inpbuf[i] != p_ucs1->field_91[i])
                no_data = 0;
        }
    }
    else
    {
        no_data = 0;
    }
    if (p_ucs1->field_1AB != 0) {
        no_data = 0;
    }

    if (no_data)
    {
        cksum = 0;
        lochead[0] = 0xAD;
        cksum ^= lochead[0];
        lochead[2] = p_ucs1->field_0;
        cksum ^= lochead[2];
        lochead[1] = cksum & 0x7F;
        net_unkn_sub_335(p_ucs1, lochead, 3);
        for (i = 0; i < 3; i++)
        {
            p_ucs1->field_A[i] = lochead[i];
        }
        p_ucs1->field_89 = 1;
        return;
    }

    p_ucs1->field_110 = 0;
    pos = 0;
    for (i = 0; i < inpbuf_len; i++)
    {
        ubyte c;

        c = p_inpbuf[i] + 0x56;
        switch (c)
        {
        case 0:
            locdata[pos++] = 0xAE;
            locdata[pos++] = 0;
            break;
        case 1:
            locdata[pos++] = 0xAE;
            locdata[pos++] = 1;
            break;
        case 2:
            locdata[pos++] = 0xAE;
            locdata[pos++] = 2;
            break;
        case 3:
            locdata[pos++] = 0xAE;
            locdata[pos++] = 3;
            break;
        case 4:
            locdata[pos++] = 0xAE;
            locdata[pos++] = 4;
            break;
        default:
            locdata[pos++] = p_inpbuf[i];
            break;
        }
    }
    dtlen = pos;

    cksum = 0;
    lochead[0] = 0xAA;
    cksum ^= lochead[0];
    lochead[1] = dtlen;
    cksum ^= lochead[1];
    lochead[3] = p_ucs1->field_0;
    cksum ^= lochead[3];
    for (i = 0; i < dtlen; i++) {
        cksum ^= locdata[i];
    }
    lochead[2] = cksum & 0x7F;

    net_unkn_sub_335(p_ucs1, lochead, 4);
    net_unkn_sub_335(p_ucs1, locdata, dtlen);

    for (i = 0; i < 4; i++)
        p_ucs1->field_A[i] = lochead[i];
    for (i = 0; i < dtlen; i++)
        p_ucs1->field_A[i + 4] = locdata[i];

    p_ucs1->field_89 = 1;
    p_ucs1->field_8D = inpbuf_len;
    for (i = 0; i < inpbuf_len; i++)
        p_ucs1->field_91[i] = p_inpbuf[i];
}

sbyte net_unkn_sub_324(struct TbUnknCommSt *p_a1, void *a2, intptr_t *params, int a4)
{
    assert(!"Not implemented");
}

void net_unkn_sub_329(struct TbUnknCommSt *p_a1)
{
    ubyte c;

    if (p_a1->field_89 == 0) {
        return;
    }

    c = p_a1->field_A[0];

    if (c == 170)
    {
        net_unkn_sub_335(p_a1, p_a1->field_A, p_a1->field_A[1] + 4);
        if (p_a1->field_1A7) {
            LOGSYNC("Resending %d", (int)p_a1->field_A[3]);
        }
    }
    else if (c == 173)
    {
        net_unkn_sub_335(p_a1, p_a1->field_A, 3);
        if (p_a1->field_1A7) {
            LOGSYNC("Resending delta %d", (int)p_a1->field_A[2]);
        }
    }
}

void unkn_exchange_start(struct TbUnknCommSt *p_a1, void *a2, unsigned int a3)
{
    if (p_a1->field_1A7) {
        LOGSYNC("Starting exchange # %d", (int)p_a1->field_0);
    }
    net_unkn_sub_323(p_a1, a2, a3);
    p_a1->field_2 = 1;
}

int unkn_exchange(struct TbUnknCommSt *p_a1, void *a2, intptr_t *params)
{
    sbyte ret;
    TbBool more, done;
    int unkmax;
    uint num;

    more = 1;
    done = 0;
    num = 0;
    p_a1->field_6 = 1;
    if (p_a1->field_1A7) {
        net_unkn_change_state(p_a1);
    }

    while (more)
    {
        if (p_a1->field_2)
            unkmax = 100;
        else
            unkmax = 30000;

        ret = net_unkn_sub_324(p_a1, a2, params, unkmax);
        switch (ret)
        {
        case 0:
        default:
            break;
        case 1:
            if (p_a1->field_2) {
                net_unkn_sub_329(p_a1);
                ++num;
            } else {
                more = 0;
            }
            break;
        case 2:
            p_a1->field_6 = 0;
            if (p_a1->field_1A7) {
                net_unkn_change_state(p_a1);
            }
            break;
        case 3:
            p_a1->field_2 = 0;
            if (p_a1->field_1A7) {
                net_unkn_change_state(p_a1);
            }
            p_a1->field_110 = 1;
            break;
        case 4:
            if (p_a1->field_2) {
                net_unkn_sub_329(p_a1);
                ++num;
            }
            break;
        }

        if (p_a1->ExchangeCb != NULL)
        {
            if (p_a1->ExchangeCb() == -7) {
                more = 0;
            }
        }

        if (num >= 400)
            more = 0;

        if (!p_a1->field_2 && !p_a1->field_6)
        {
            p_a1->field_1++;
            p_a1->field_1 &= ~0x80;
            p_a1->field_0++;
            p_a1->field_0 &= ~0x80;
            more = 0;
            done = 1;
        }
    }

    if (p_a1->field_1A7)
    {
        LOGSYNC("Completed exchange - Return Code = %s", done ? "TRUE" : "FALSE");
    }
    return done;
}

int run_exchange_func()
{
    static uint32_t start_time[4];
    uint32_t end_time;
    int idx = 0;

    end_time = dos_clock();
    if ((end_time - start_time[idx]) < 10) {
        return 0;
    }
    start_time[idx] = end_time;

    if (NetworkServicePtr.F.SessionExchange == NULL) {
        return 0;
    }
    return NetworkServicePtr.F.SessionExchange();
}

struct TbSerialDev *LbCommInit(ushort idx)
{
#if 0
    struct TbSerialDev *ret;
    LOGDBG("Starting");
    asm volatile ("call ASM_LbCommInit\n"
        : "=r" (ret) : "a" (idx) );
    return ret;
#else
    struct TbSerialDev *p_serdev;

    assert(sizeof(struct TbSerialDev) == 4301);

    if (idx > 3)
        return 0;

    if ((com_dev[idx].field_A[0] & 1) != 0)
        p_serdev = com_dev[idx].serdev;
    else
        p_serdev = (struct TbSerialDev *)LbMemoryAllocLow(sizeof(struct TbSerialDev));

    if (p_serdev == NULL)
        return 0;

    //memcpy(p_serdev, ser_start, 150); -- makes little sense
    LbMemorySet(p_serdev, 0, sizeof(struct TbSerialDev));
    p_serdev->field_2 = com_dev[idx].field_4;
    p_serdev->comdev_id = idx;
    p_serdev->field_1096 = com_dev[p_serdev->comdev_id].field_4;
    p_serdev->field_1098 = com_dev[p_serdev->comdev_id].field_6;
    p_serdev->field_109A = com_dev[p_serdev->comdev_id].field_8;
    p_serdev->field_10A0 = 0;
    p_serdev->field_10A2 = 0;
    p_serdev->inbuf_pos = 0;
    p_serdev->field_109E = 0;
    p_serdev->field_10AB = 1;

    com_dev[p_serdev->comdev_id].serdev = p_serdev;
    com_dev[p_serdev->comdev_id].field_A[0] = 1;
#if defined(DOS)||defined(GO32)
    backup_serial_int_vectors(p_serdev);
    setup_serial_int_vectors(p_serdev);
#else
#endif
    init_com_port(p_serdev);

    p_serdev->baudrate = 9600;
    p_serdev->num_players = 2;
    dword_1E85E3 = p_serdev;

    net_unkn_post_init_1(&netunkst_1E81E0);
    net_unkn_post_init_2(&netunkst_1E81E0, net_unkn_callback1);
    net_unkn_post_init_3(&netunkst_1E81E0, net_unkn_callback2);

    return p_serdev;
#endif
}

TbResult LbCommSetBaud(int rate, ushort dev_id)
{
    struct TbSerialDev *serdev;

    LOGDBG("Starting");
    if (dev_id > 3)
        return Lb_FAIL;

    serdev = com_dev[dev_id].serdev;
    if (serdev == NULL)
        return Lb_FAIL;

    serdev->baudrate = SetBps(serdev, rate);
    return Lb_SUCCESS;
}

int LbCommExchange(ushort idx, void *data, int datalen)
{
#if 0
    int ret;
    asm volatile ("call ASM_LbCommExchange\n"
        : "=r" (ret) : "a" (idx), "d" (data), "b" (datalen) );
    return ret;
#else
    struct TbSerialDev *p_serdev;
    intptr_t params[2];

    p_serdev = com_dev[idx].serdev;
    if (p_serdev == NULL) {
        return -1;
    }
    net_unkn_sub_332(&netunkst_1E81E0, NetworkServicePtr.F.SessionExchange);

    params[1] = (intptr_t)data;
    params[0] = datalen;

    if (p_serdev->num_players <= 1) {
        return 1;
    }
    if ( p_serdev->field_10A9 )
    {
        if (run_exchange_func() == -7)
            return -7;
        unkn_exchange_start(&netunkst_1E81E0, data, datalen);
        if (!unkn_exchange(&netunkst_1E81E0, data + datalen, params))
            return -7;
        if (run_exchange_func() == -7)
            return -7;
    }
    else
    {
        if (run_exchange_func() == -7)
            return -7;
        unkn_exchange_start(&netunkst_1E81E0, data + datalen, datalen);
        if (!unkn_exchange(&netunkst_1E81E0, data, params))
            return -7;
        if (run_exchange_func() == -7)
            return -7;
    }
    return 1;
#endif
}

int LbCommStopExchange(ubyte a1)
{
    net_unkn_func_338(&netunkst_1E81E0);
    return Lb_SUCCESS;
}

int LbCommDeInit(struct TbSerialDev *serhead)
{
#if defined(DOS)||defined(GO32)
    struct TbSerialDev *p_serdev;
    struct ComHandlerInfo *cdev;
    union REGS regs;
    ubyte r, n;

    LOGDBG("Starting");
    cdev = &com_dev[serhead->comdev_id];

    outp(serhead->field_1096 + 1, 0);
    outp(serhead->field_1096 + 4, 0);
    n = 1 << serhead1->field_109A;
    r = inp(0x21);
    outp(0x21, r | n);

    memset(&regs, 0, sizeof(union REGS));
    regs.x.eax = 0x201;
    regs.x.ebx = serhead1->field_1098;
    regs.x.ecx = cdev->field_18;
    regs.x.edx = cdev->field_1A;
    int386(0x31, &regs, &regs);

    memset(&regs, 0, sizeof(union REGS));
    regs.x.eax = 0x205;
    regs.x.ebx = serhead->field_1098;
    regs.x.ecx = cdev->field_16;
    regs.x.edx = cdev->field_12;
    int386(0x31, &regs, &regs);

    memset(&regs, 0, sizeof(union REGS));
    regs.x.eax = 0xF3;
    regs.x.edx = serhead->comdev_id;
    int386(0x14, &regs, &regs);

    return Lb_SUCCESS;
#else
    LOGDBG("Starting");
    return Lb_FAIL;
#endif
}

TbResult LbModemInit(ushort dev_id)
{
    struct TbSerialDev *p_serdev;
    TbResult ret;

    LOGDBG("Starting");
    if (dev_id > 3)
        return Lb_FAIL;

    p_serdev = com_dev[dev_id].serdev;

    if (p_serdev == NULL)
        return Lb_FAIL;

    send_string(p_serdev, modem_cmds[0].cmd);
    NetworkServicePtr.F.UsedSessionInit = NetworkServicePtr.F.SessionInit;
    ret = get_modem_response(p_serdev);
    NetworkServicePtr.F.UsedSessionInit = NULL;

    return ret;
}

TbResult LbModemDial(ushort dev_id, const char *distr)
{
    struct TbSerialDev *p_serdev;
    char locstr[80];
    TbResult ret;

    LOGDBG("Starting");
    if (dev_id > 3)
        return Lb_FAIL;

    p_serdev = com_dev[dev_id].serdev;

    if (p_serdev == NULL)
        return Lb_FAIL;

    strcpy(locstr, modem_cmds[1].cmd);
    switch(p_serdev->field_10AB)
    {
    case 1:
        strcat(locstr, "T");
        break;
    case 2:
        strcat(locstr, "P");
        break;
    }
    strcat(locstr, distr);

    send_string(p_serdev, locstr);
    NetworkServicePtr.F.UsedSessionInit = NetworkServicePtr.F.SessionDial;
    ret = get_modem_response(p_serdev);
    NetworkServicePtr.F.UsedSessionInit = NULL;

    return ret;
}

TbResult LbModemAnswer(ushort dev_id)
{
    struct TbSerialDev *p_serdev;
    TbResult ret;

    LOGDBG("Starting");
    if (dev_id > 3)
        return Lb_FAIL;

    p_serdev = com_dev[dev_id].serdev;
    if (p_serdev == NULL)
        return Lb_FAIL;

    NetworkServicePtr.F.UsedSessionInit = NetworkServicePtr.F.SessionAnswer;
    send_string(p_serdev, modem_cmds[3].cmd);
    ret = 3;
    while (ret == 3 || ret == 1)
        ret = get_modem_response(p_serdev);
    NetworkServicePtr.F.UsedSessionInit = NULL;
    return ret;
}

TbResult LbModemHangUp(ushort dev_id)
{
    struct TbSerialDev *p_serdev;
    TbResult ret;

    LOGDBG("Starting");
    if (dev_id > 3)
        return Lb_FAIL;

    p_serdev = com_dev[dev_id].serdev;
    if (p_serdev == NULL)
        return Lb_FAIL;

    read_write_clear_flag(p_serdev, p_serdev->field_1096 + 4, 0x01);
    wait(1250);

    read_write_set_flag(p_serdev, p_serdev->field_1096 + 4, 0x01);
    wait(1300);

    send_string(p_serdev, "+++");
    wait(1300);

    NetworkServicePtr.F.UsedSessionInit = NetworkServicePtr.F.SessionHangUp;
    ret = get_modem_response(p_serdev);
    NetworkServicePtr.F.UsedSessionInit = NULL;
    send_string(p_serdev, modem_cmds[2].cmd);

    return ret;
}

TbResult LbModemRingType(ushort dev_id, ubyte rtyp)
{
    struct TbSerialDev *p_serdev;

    LOGDBG("Starting");
    if (dev_id > 3)
        return Lb_FAIL;

    p_serdev = com_dev[dev_id].serdev;
    if (p_serdev == NULL)
        return Lb_FAIL;

    p_serdev->field_10AB = rtyp;

    return Lb_SUCCESS;
}

TbResult LbNetworkServiceStart(struct NetworkServiceInfo *p_nsvc)
{
    TbResult ret;
    ulong k;

    ret = Lb_FAIL;
    memcpy(&NetworkServicePtr.I, p_nsvc, sizeof(struct NetworkServiceInfo));
    switch (NetworkServicePtr.I.Type)
    {
    case NetSvc_IPX:
        if (IPXHandler == NULL) {
            IPXHandler = LbMemoryAlloc(sizeof(struct TbIPXHandler));
        }
        if (IPXHandler == NULL) {
            LOGERR("Allocating IPX handler failed");
            ret = Lb_FAIL;
            break;
        }
        memset(&IPXPlayer.Header, 0, sizeof(struct TbIPXPlayerHeader));
        memset(&IPXPlayer.Data, 0, sizeof(struct TbIPXPlayerData));
        IPXPlayer.Header.field_2 = p_nsvc->GameId;
        k = p_nsvc->Param + 0x4545;
        NetworkServicePtr.I.Id = &IPXPlayer.Header;
        if (k > 0x4FFF)
            k = 0x4FFF;
        ipx_service_init((k << 8) | (k >> 8));
        ret = Lb_SUCCESS;
        break;
    case NetSvc_COM1:
    case NetSvc_COM2:
    case NetSvc_COM3:
    case NetSvc_COM4:
        NetworkServicePtr.I.Id = LbCommInit(NetworkServicePtr.I.Type - NetSvc_COM1);
        if (NetworkServicePtr.I.Id == NULL) {
            LOGERR("Serial service init failed");
            ret = Lb_FAIL;
            break;
        }
        ret = Lb_SUCCESS;
        break;
    case NetSvc_RADICA:
        ret = radica_service_init(p_nsvc);
        if (ret != Lb_SUCCESS) {
            LOGERR("RADICA service init failed");
            ret = Lb_FAIL;
            NetworkServicePtr.I.Type = NetSvc_NONE; // Why only this one is reverting the type on fail?
        }
        break;
    }
    return ret;
}

TbResult LbNetworkUpdate(void)
{
    TbResult ret;

    ret = Lb_FAIL;
    switch (NetworkServicePtr.I.Type)
    {
    case NetSvc_IPX:
        ipx_update();
        ret = Lb_SUCCESS;
        break;
    case NetSvc_COM1:
    case NetSvc_COM2:
    case NetSvc_COM3:
    case NetSvc_COM4:
        ret = Lb_SUCCESS;
        break;
    case NetSvc_RADICA:
        radica_update();
        ret = Lb_SUCCESS;
        break;
    }
    return ret;
}

int LbNetworkSessionList(struct TbNetworkSessionList *p_nslist, int listlen)
{
    int ret;

    ret = 0;
    switch (NetworkServicePtr.I.Type)
    {
    case NetSvc_IPX:
        ret = ipx_session_list_conv(p_nslist, listlen);
        break;
    case NetSvc_COM1:
    case NetSvc_COM2:
    case NetSvc_COM3:
    case NetSvc_COM4:
        ret = 0;
        break;
    case NetSvc_RADICA:
        ret = radica_session_list(p_nslist, listlen);
        break;
    }
    return ret;
}

/** Prepares Network_Service_List and returns its size.
 */
int LbNetworkServiceList(void)
{
    struct NetworkServiceInfo *nsinfo;
    int i;

    nsinfo = Network_Service_List;
    i = 0;
    while (nsinfo->Type != 0)
    {
        nsinfo++;
        i++;
    }
    return i;
}

int LbNetworkSessionNumberPlayers(void)
{
    struct TbSerialDev *serhead;
    struct TbIPXPlayerData *ipxdata;
    int ret;

    ret = Lb_FAIL;
    switch (NetworkServicePtr.I.Type)
    {
    case NetSvc_IPX:
        ipxdata = &IPXPlayer.Data;
        ret = ipxdata->num_players;
        break;
    case NetSvc_COM1:
    case NetSvc_COM2:
    case NetSvc_COM3:
    case NetSvc_COM4:
        serhead = NetworkServicePtr.I.Id;
        ret = serhead->num_players;
        break;
    case NetSvc_RADICA:
        ret = Lb_FAIL;
        break;
    }
    return ret;
}

TbResult LbNetworkSend(int plyr, ubyte *data, int dtlen)
{
    TbResult ret;

    ret = Lb_FAIL;
    switch (NetworkServicePtr.I.Type)
    {
    case NetSvc_IPX:
        ipx_send_packet_to_player_wait(plyr, data, dtlen);
        ret = Lb_SUCCESS;
        break;
    case NetSvc_COM1:
    case NetSvc_COM2:
    case NetSvc_COM3:
    case NetSvc_COM4:
        ret = Lb_FAIL;
        break;
    case NetSvc_RADICA:
        ret = Lb_FAIL;
        break;
    }
    return ret;
}

TbResult LbNetworkReceive(int plyr, ubyte *data, int dtlen)
{
    TbResult ret;

    ret = Lb_FAIL;
    switch (NetworkServicePtr.I.Type)
    {
    case NetSvc_IPX:
        ipx_receive_packet_from_player_wait(plyr, data, dtlen);
        ret = Lb_SUCCESS;
        break;
    case NetSvc_COM1:
    case NetSvc_COM2:
    case NetSvc_COM3:
    case NetSvc_COM4:
        ret = Lb_FAIL;
        break;
    case NetSvc_RADICA:
        ret = Lb_FAIL;
        break;
    }
    return ret;
}

TbResult LbNetworkMessageSend(int plyr, ubyte *data, int dtlen)
{
    TbResult ret;

    ret = Lb_FAIL;
    switch (NetworkServicePtr.I.Type)
    {
    case NetSvc_IPX:
        ipx_send_packet_to_player_wait(plyr, data, dtlen);
        ret = Lb_SUCCESS;
        break;
    case NetSvc_COM1:
    case NetSvc_COM2:
    case NetSvc_COM3:
    case NetSvc_COM4:
        ret = Lb_FAIL;
        break;
    case NetSvc_RADICA:
        ret = Lb_FAIL;
        break;
    }
    return ret;
}

TbResult LbNetworkMessageReceive(int plyr, ubyte *data, int dtlen)
{
    TbResult ret;

    ret = Lb_FAIL;
    switch (NetworkServicePtr.I.Type)
    {
    case NetSvc_IPX:
        ipx_receive_packet_from_player_wait(plyr, data, dtlen);
        ret = Lb_SUCCESS;
        break;
    case NetSvc_COM1:
    case NetSvc_COM2:
    case NetSvc_COM3:
    case NetSvc_COM4:
        ret = Lb_FAIL;
        break;
    case NetSvc_RADICA:
        ret = Lb_FAIL;
        break;
    }
    return ret;
}

TbResult LbNetworkSendNoWait(int plyr, ubyte *data, int dtlen)
{
    TbResult ret;

    ret = Lb_FAIL;
    switch (NetworkServicePtr.I.Type)
    {
    case NetSvc_IPX:
        ipx_send_packet_to_player_nowait(plyr, data, dtlen);
        ret = Lb_SUCCESS;
        break;
    case NetSvc_COM1:
    case NetSvc_COM2:
    case NetSvc_COM3:
    case NetSvc_COM4:
        ret = Lb_FAIL;
        break;
    case NetSvc_RADICA:
        ret = Lb_FAIL;
        break;
    }
    return ret;
}

TbResult LbNetworkReceiveNoWait(int plyr, ubyte *data, int dtlen)
{
    TbResult ret;

    ret = Lb_FAIL;
    switch (NetworkServicePtr.I.Type)
    {
    case NetSvc_IPX:
        ipx_receive_packet_from_player_nowait(plyr, data, dtlen);
        ret = Lb_SUCCESS;
        break;
    case NetSvc_COM1:
    case NetSvc_COM2:
    case NetSvc_COM3:
    case NetSvc_COM4:
        ret = Lb_FAIL;
        break;
    case NetSvc_RADICA:
        ret = Lb_FAIL;
        break;
    }
    return ret;
}

TbResult LbNetworkInit(void)
{
    struct TbSerialDev *serhead;
    TbResult ret;

    ret = Lb_FAIL;
    switch (NetworkServicePtr.I.Type)
    {
    case NetSvc_IPX:
        ret = Lb_FAIL;
        break;
    case NetSvc_COM1:
    case NetSvc_COM2:
    case NetSvc_COM3:
    case NetSvc_COM4:
        serhead = NetworkServicePtr.I.Id;
        ret = LbModemInit(serhead->comdev_id);
        break;
    case NetSvc_RADICA:
        ret = Lb_FAIL;
        break;
    }
    return ret;
}

TbResult LbNetworkDial(const char *distr)
{
    struct TbSerialDev *serhead;
    TbResult ret;

    ret = Lb_FAIL;
    switch (NetworkServicePtr.I.Type)
    {
    case NetSvc_IPX:
        ret = Lb_FAIL;
        break;
    case NetSvc_COM1:
    case NetSvc_COM2:
    case NetSvc_COM3:
    case NetSvc_COM4:
        serhead = NetworkServicePtr.I.Id;
        ret = LbModemDial(serhead->comdev_id, distr);
        break;
    case NetSvc_RADICA:
        ret = Lb_FAIL;
        break;
    }
    return ret;
}

TbResult LbNetworkAnswer(void)
{
    struct TbSerialDev *serhead;
    TbResult ret;

    ret = Lb_FAIL;
    switch (NetworkServicePtr.I.Type)
    {
    case NetSvc_IPX:
        ret = Lb_FAIL;
        break;
    case NetSvc_COM1:
    case NetSvc_COM2:
    case NetSvc_COM3:
    case NetSvc_COM4:
        serhead = NetworkServicePtr.I.Id;
        ret = LbModemAnswer(serhead->comdev_id);
        break;
    case NetSvc_RADICA:
        ret = Lb_FAIL;
        break;
    }
    return ret;
}

TbResult LbNetworkSessionStop(void)
{
    struct TbSerialDev *serhead;
    TbResult ret;

    ret = Lb_FAIL;
    switch (NetworkServicePtr.I.Type)
    {
    case NetSvc_IPX:
        ipx_stop_network();
        ret = Lb_SUCCESS;
        break;
    case NetSvc_COM1:
    case NetSvc_COM2:
    case NetSvc_COM3:
    case NetSvc_COM4:
        serhead = NetworkServicePtr.I.Id;
        LbCommStopExchange(serhead->comdev_id);
        lbICommSessionActive = 0;
        ret = Lb_SUCCESS;
        break;
    case NetSvc_RADICA:
        ret = Lb_FAIL;
        break;
    }
    return ret;
}

TbResult LbNetworkHostPlayerNumber(void)
{
    struct TbSerialDev *serhead;
    TbResult ret;

    ret = Lb_FAIL;
    switch (NetworkServicePtr.I.Type)
    {
    case NetSvc_IPX:
        ret = ipx_get_host_player_number();
        break;
    case NetSvc_COM1:
    case NetSvc_COM2:
    case NetSvc_COM3:
    case NetSvc_COM4:
        serhead = NetworkServicePtr.I.Id;
        if (!serhead->field_10A9)
            ret = 0;
        else
            ret = 1;
        break;
    case NetSvc_RADICA:
        ret = Lb_FAIL;
        break;
    }
    return ret;
}

TbResult LbNetworkSetupIPXAddress(ulong addr)
{
    if (addr == 0)
        return Lb_FAIL;
#if 0
    IPXHandler->unkn_addr_field = addr;
#endif
    NetworkServicePtr.I.Param = addr;
    return Lb_SUCCESS;
}

int LbNetworkPlayerNumber(void)
{
    struct TbSerialDev *serhead;
    int ret;

    ret = Lb_FAIL;
    switch (NetworkServicePtr.I.Type)
    {
    case NetSvc_IPX:
        ret = ipx_get_player_number();
        break;
    case NetSvc_COM1:
    case NetSvc_COM2:
    case NetSvc_COM3:
    case NetSvc_COM4:
        serhead = NetworkServicePtr.I.Id;
        if (!serhead->field_10A9)
            ret = 0;
        else
            ret = 1;
        break;
    case NetSvc_RADICA:
        break;
    }
    return ret;
}

TbResult LbNetworkPlayerName(char *name, int plyr)
{
    struct TbSerialDev *serhead;
    TbResult ret;

    ret = Lb_FAIL;
    switch (NetworkServicePtr.I.Type)
    {
    case NetSvc_IPX:
        ret = ipx_get_player_name(name, plyr);
        break;
    case NetSvc_COM1:
    case NetSvc_COM2:
    case NetSvc_COM3:
    case NetSvc_COM4:
        serhead = NetworkServicePtr.I.Id;
        if (plyr == 0) {
            strcpy(name, &serhead->field_10AD[0]);
            ret = Lb_SUCCESS;
        } else if (plyr == 1) {
            strcpy(name, &serhead->field_10AD[16]);
            ret = Lb_SUCCESS;
        }
        break;
    case NetSvc_RADICA:
        break;
    }
    return ret;
}

TbResult LbNetworkExchange(void *a1, int a2)
{
    struct TbSerialDev *serhead;
    TbResult ret;

    ret = Lb_FAIL;
    switch (NetworkServicePtr.I.Type)
    {
    case NetSvc_IPX:
        ret = ipx_exchange_packets(a1, a2);
        break;
    case NetSvc_COM1:
    case NetSvc_COM2:
    case NetSvc_COM3:
    case NetSvc_COM4:
        serhead = NetworkServicePtr.I.Id;
        LbCommExchange(serhead->comdev_id, a1, a2);
        ret = Lb_SUCCESS;
        break;
    case NetSvc_RADICA:
        radica_exchange_packets(a1, a2);
        ret = Lb_SUCCESS;
        break;
    }
    return ret;
}

TbResult LbNetworkReset(void)
{
    TbResult ret;

    ret = Lb_FAIL;
    switch (NetworkServicePtr.I.Type)
    {
    case NetSvc_IPX:
        if (ipx_is_initialized()) {
            ipx_shutdown(IPXHandler->field_8);
            ret = Lb_SUCCESS;
        }
        break;
    case NetSvc_COM1:
    case NetSvc_COM2:
    case NetSvc_COM3:
    case NetSvc_COM4:
        ret = LbCommDeInit(NetworkServicePtr.I.Id);
        break;
    case NetSvc_RADICA:
        ret = radica_shutdown();
        break;
    }
    NetworkServicePtr.I.Type = NetSvc_NONE;
    return ret;
}

TbBool LbNetworkSessionActive(void)
{
    TbBool ret;

    ret = false;
    switch (NetworkServicePtr.I.Type)
    {
    case NetSvc_IPX:
        if (ipx_is_initialized())
            ret = IPXHandler->SessionActive;
        break;
    case NetSvc_COM1:
    case NetSvc_COM2:
    case NetSvc_COM3:
    case NetSvc_COM4:
        ret = lbICommSessionActive;
        break;
    case NetSvc_RADICA:
        ret = false;
        break;
    }
    return ret;
}

TbResult LbNetworkHangUp(void)
{
    struct TbSerialDev *serhead;
    TbResult ret;

    ret = Lb_FAIL;
    switch (NetworkServicePtr.I.Type)
    {
    case NetSvc_IPX:
        ret = Lb_FAIL;
        break;
    case NetSvc_COM1:
    case NetSvc_COM2:
    case NetSvc_COM3:
    case NetSvc_COM4:
        serhead = NetworkServicePtr.I.Id;
        ret = LbModemHangUp(serhead->comdev_id);
        break;
    case NetSvc_RADICA:
        ret = Lb_FAIL;
        break;
    }
    return ret;
}

TbResult LbNetworkReadConfig(const char *fname)
{
    return LbModemReadConfig(fname);
}

TbResult LbNetworkSetBaud(int rate)
{
    struct TbSerialDev *serhead;
    TbResult ret;

    ret = Lb_FAIL;
    switch (NetworkServicePtr.I.Type)
    {
    case NetSvc_IPX:
        ret = Lb_FAIL;
        break;
    case NetSvc_COM1:
    case NetSvc_COM2:
    case NetSvc_COM3:
    case NetSvc_COM4:
        serhead = NetworkServicePtr.I.Id;
        LbCommSetBaud(rate, serhead->comdev_id);
        ret = Lb_SUCCESS;
        break;
    case NetSvc_RADICA:
        ret = Lb_FAIL;
        break;
    }
    return ret;
}

TbResult LbNetworkShutDownListeners(void)
{
    TbResult ret;

    ret = Lb_FAIL;
    switch (NetworkServicePtr.I.Type)
    {
    case NetSvc_IPX:
        ipx_shutdown_listeners();
        ret = Lb_SUCCESS;
        break;
    case NetSvc_COM1:
    case NetSvc_COM2:
    case NetSvc_COM3:
    case NetSvc_COM4:
        ret = Lb_SUCCESS;
        break;
    case NetSvc_RADICA:
        ret = Lb_SUCCESS;
        break;
    }
    return ret;
}

TbResult LbNetworkOpenUpListeners(void)
{
    TbResult ret;

    ret = Lb_FAIL;
    switch (NetworkServicePtr.I.Type)
    {
    case NetSvc_IPX:
        ipx_openup_listeners();
        ret = Lb_SUCCESS;
        break;
    case NetSvc_COM1:
    case NetSvc_COM2:
    case NetSvc_COM3:
    case NetSvc_COM4:
        ret = Lb_SUCCESS;
        break;
    case NetSvc_RADICA:
        ret = Lb_SUCCESS;
        break;
    }
    return ret;
}

TbResult LbCommSessionCreate(struct TbSerialDev *serhead, const char *sess_name, const char *a2)
{
    NSVC_SESSIONCB exchange_cb_bkp;
    char locstr[16];
    TbResult ret;

    LOGDBG("Starting");
    serhead->field_10A9 = 1;
    strcpy(serhead->field_10AD, a2);

    exchange_cb_bkp = NetworkServicePtr.F.SessionExchange;
    NetworkServicePtr.F.SessionExchange = NetworkServicePtr.F.SessionCreate;
    ret = LbCommExchange(serhead->comdev_id, serhead->field_10AD, 16);
    if (ret == Lb_SUCCESS)
    {
        strcpy(locstr, sess_name);
        ret = LbCommExchange(serhead->comdev_id, locstr, 8);
        lbICommSessionActive = 1;
    }
    NetworkServicePtr.F.SessionExchange = exchange_cb_bkp;

    return ret;
}

TbResult LbCommSessionJoin(struct TbSerialDev *serhead, char *sess_name, const char *a2)
{
    NSVC_SESSIONCB exchange_cb_bkp;
    char locstr[16];
    TbResult ret;

    LOGDBG("Starting");
    LbMemorySet(locstr, 0, sizeof(locstr));
    serhead->field_10A9 = 0;
    strcpy(&serhead->field_10AD[16], a2);

    exchange_cb_bkp = NetworkServicePtr.F.SessionExchange;
    NetworkServicePtr.F.SessionExchange = NetworkServicePtr.F.SessionJoin;
    ret = LbCommExchange(serhead->comdev_id, serhead->field_10AD, 16);
    if (ret == Lb_SUCCESS)
    {
        ret = LbCommExchange(serhead->comdev_id, locstr, 8);
        if (ret == Lb_SUCCESS) {
            strcpy(sess_name, locstr);
        }
        lbICommSessionActive = 1;
    }
    NetworkServicePtr.F.SessionExchange = exchange_cb_bkp;

    return ret;
}

TbResult LbNetworkSessionCreate(struct TbNetworkSession *session, char *a2)
{
    struct TbSerialDev *serhead;
    TbResult ret;

    ret = Lb_FAIL;
    NetTimeoutTicks = 3000;
    switch (NetworkServicePtr.I.Type)
    {
    case NetSvc_IPX:
        IPXPlayer.Data.field_10E = session->MaxPlayers;
        ret = ipx_create_session(session->Name, a2);
        break;
    case NetSvc_COM1:
    case NetSvc_COM2:
    case NetSvc_COM3:
    case NetSvc_COM4:
        serhead = NetworkServicePtr.I.Id;
        ret = LbCommSessionCreate(serhead, session->Name, a2);
        break;
    case NetSvc_RADICA:
        ret = radica_create_session(session, a2);
        break;
    }
    LOGSYNC("Service %d create result=%d", (int)NetworkServicePtr.I.Type, (int)ret);
    return ret;
}

void read_a_line(FILE *fp, char *buf)
{
    char c;
    char *s;

    for (s = buf; ; s++)
    {
        c = fgetc(fp);
        if ((c == '\r') || (c == '\n'))
            break;
        *s = c;
    }
    *s = '\0';
}

TbResult LbNetworkSessionJoin(struct TbNetworkSession *session, char *a2)
{
    struct TbSerialDev *serhead;
    struct IPXSessionList ipxsess;
    TbResult ret;

    NetTimeoutTicks = 5000;
    ret = Lb_FAIL;
    switch (NetworkServicePtr.I.Type)
    {
    case NetSvc_IPX:
        memset(&ipxsess, 0, sizeof(struct IPXSessionList));
        memcpy(ipxsess.Session.Name, session->Name, sizeof(ipxsess.Session.Name));
        ipxsess.Session.Name[7] = '\0';
        memcpy(&ipxsess.Session.Reserved[22], &session->Reserved[0], 4);
        memcpy(&ipxsess.Session.Reserved[16], &session->Reserved[4], 6);
        ret = ipx_join_session(&ipxsess, a2);
        break;
    case NetSvc_COM1:
    case NetSvc_COM2:
    case NetSvc_COM3:
    case NetSvc_COM4:
        serhead = NetworkServicePtr.I.Id;
        ret = LbCommSessionJoin(serhead, session->Name, a2);
        break;
    case NetSvc_RADICA:
        ret = radica_join_session(session, a2);
        break;
    }
    return ret;
}

TbResult LbModemReadConfig(const char *fname)
{
    char locstr[80];
    FILE *fp;
    int i;

    LOGDBG("Starting");
    if (fname == NULL)
        return Lb_FAIL;
    fp = fopen(fname, "rt");
    if (!fp)
        return Lb_FAIL;
    for (i = 0; i < 4; i++)
    {
        read_a_line(fp, locstr);
        read_a_line(fp, locstr);
        com_dev[i].field_4 = strtol(locstr, NULL, 16);
        read_a_line(fp, locstr);
        com_dev[i].field_8 = atol(locstr);
        com_dev[i].field_6 = com_dev[i].field_8 + 8;
    }
    read_a_line(fp, locstr);
    read_a_line(fp, modem_cmds[0].cmd);
    read_a_line(fp, modem_cmds[1].cmd);
    read_a_line(fp, modem_cmds[2].cmd);
    read_a_line(fp, modem_cmds[3].cmd);
    fclose(fp);
    return Lb_SUCCESS;
}

int my_net_session_callback()
{
    swap_wscreen();
    if (is_key_pressed(KC_ESCAPE, KMod_DONTCARE))
        return -7;
    return 0;
}

void net_system_init0(void)
{
    LbNetworkSetSessionCreateFunction(my_net_session_callback);
    LbNetworkSetSessionJoinFunction(my_net_session_callback);
    LbNetworkSetSessionUnk2CFunction(my_net_session_callback);
    LbNetworkSetSessionInitFunction(my_net_session_callback);
    LbNetworkSetSessionDialFunction(my_net_session_callback);
    LbNetworkSetSessionAnswerFunction(my_net_session_callback);
    LbNetworkSetSessionHangUpFunction(my_net_session_callback);
    if (LbNetworkReadConfig("modem.cfg") != Lb_FAIL)
        data_1c4a70 = 1;
}

void net_system_init2(void)
{
    LbNetworkSetSessionCreateFunction(my_net_session_callback);
    LbNetworkSetSessionJoinFunction(my_net_session_callback);
    LbNetworkSetSessionExchangeFunction(my_net_session_callback);
    LbNetworkSetSessionInitFunction(my_net_session_callback);
    LbNetworkSetSessionDialFunction(my_net_session_callback);
    LbNetworkSetSessionAnswerFunction(my_net_session_callback);
    LbNetworkSetSessionHangUpFunction(my_net_session_callback);
    if (LbNetworkReadConfig("modem.cfg") != Lb_FAIL)
        data_1c4a70 = 1;
}

void net_system_reset(void)
{
    LbNetworkSetSessionCreateFunction(NULL);
    LbNetworkSetSessionJoinFunction(NULL);
    LbNetworkSetSessionExchangeFunction(NULL);
    LbNetworkSetSessionInitFunction(NULL);
    LbNetworkSetSessionDialFunction(NULL);
    LbNetworkSetSessionAnswerFunction(NULL);
    LbNetworkSetSessionHangUpFunction(NULL);
}

/******************************************************************************/
