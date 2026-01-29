#include <SDL.h>
#include <getopt.h>
#include <stdlib.h>

#include "bfmemory.h"
#include "bffile.h"
#include "bfini.h"
#include "bfscreen.h"
#include "bflog.h"
#include "swlog.h"
#include "bfjoyst.h"
#include "display.h"
#include "guitext.h"
#include "game.h"
#include "game_data.h"
#include "game_options.h"
#include "game_save.h"
#include "lvfiles.h"
#include "lvobjctv.h"
#include "network.h"
#include "packet.h"
#include "thing.h"
#include "util.h"

#if defined WIN32 && defined main
// Anti SDL
# undef main
#endif

enum ConfigCmd {
    ConfCmd_CD = 1,
    ConfCmd_InstallDrive,
    ConfCmd_Language,
    ConfCmd_Data,
    ConfCmd_Intro,
    ConfCmd_Anims,
    ConfCmd_Maps,
    ConfCmd_Levels,
    ConfCmd_Sound,
    ConfCmd_DOS,
    ConfCmd_ResGameHi,
    ConfCmd_ResGameLo,
    ConfCmd_ResMenu,
    ConfCmd_ResFMVVidHi,
    ConfCmd_ResFMVidLo,
};

const struct TbNamedEnum conf_file_cmnds[] = {
  {"CD",		ConfCmd_CD},
  {"InstallDrive", ConfCmd_InstallDrive},
  {"Language",	ConfCmd_Language},
  {"Data",		ConfCmd_Data},
  {"Intro",		ConfCmd_Intro},
  {"Anims",		ConfCmd_Anims},
  {"Maps",		ConfCmd_Maps},
  {"Levels",	ConfCmd_Levels},
  {"Sound",		ConfCmd_Sound},
  {"DOS",		ConfCmd_DOS},
  {"ResGameHi",	ConfCmd_ResGameHi},
  {"ResGameLo",	ConfCmd_ResGameLo},
  {"ResMenu",	ConfCmd_ResMenu},
  {"ResFMVVidHi",ConfCmd_ResFMVVidHi},
  {"ResFMVidLo",ConfCmd_ResFMVidLo},
  {NULL,		0},
};

const struct TbNamedEnum conf_file_disk_inst_lev[] = {
  {"Min", 1},
  {"Max", 2},
  {NULL,  0},
  };

TbBool cmdln_fullscreen = true;
TbBool cmdln_lores_stretch = true;

static void
print_help (const char *argv0)
{
  printf (
"Usage: %s [OPTIONS]\n\n"
"Available options:\n"
"                -A        Enter game mode 1; not sure what this mode should be\n"
"                -B        Test scenario 99?\n"
"                -C        Test scenario 100?\n"
"                -D        Direct keyboard mode; queries kb rather than use\n"
"                          events/interrupts\n"
"                -d <str>  Activate debug functions; t - things debug HUD,\n"
"                          o - objectives debug HUD, c - collision debug HUD\n"
"                -E <num>  Joystick config\n"
"                -F        Re-compute and re-save `tables.dat` colour tables\n"
"                          file, using `fade.dat` as input\n"
"                -g        Enter normal gameplay mode; to be used when playing\n"
"                          the normal campaigns\n"
"                -H        Initially enter high resolution mode\n"
"  --help        -h        Display the help message\n"
"                -I <num>  Multiplayer connect through IPX using given IPX\n"
"                          network address\n"
"                -l <str>  Activate additional logging; s - thing states and\n"
"                          commands\n"
"                -m <n>,<n> Load campaign with given index, from which load\n"
"                          mission with given index in single map mode\n"
"                -N        Sets a flag which is never used. Debug feature?\n"
"                -p <num>  Play replay packets from file of given index;\n"
"                          use '-m' to specify mission on which to play\n"
"                -q        Skip intro movie\n"
"                -r        Record replay packets to file in savegame dir;\n"
"                          next unused filename for selected map will be used\n"
"  --level-deep-fix -L     Perform deeper fixes to the loaded levels; this may\n"
"                          fix some glitches, but such autamatic rework may\n"
"                          also break stuff; non-damaging fixes are applied\n"
"                          even without this option\n"
"  --no-stretch  -S        Do not stretch 320x200 graphics to high res display\n"
"                -s <str>  Set multiplayer session name string\n"
"                -T        Re-compute and re-save `tables.dat` colour tables\n"
"                          file, using palette file as input\n"
"  --self-tests  -t        Execute build self tests\n"
"                -u <str>  Set user name (login / network name) string\n"
"  --windowed    -W        Run in windowed mode\n"
"                -w        Lower memory use; decreases size of static arrays\n",
  argv0);
}

static TbBool setup_log(void)
{
    const char *rel_kind;

#  ifdef DEBUG
    rel_kind = "debug";
#  else
    rel_kind = "standard";
#  endif

#if !defined(PACKAGE_NAME)
# error PACKAGE_NAME is not defined, config.h needs to be included
#endif

    printf(PACKAGE_NAME" ver "VERSION" (%s release)\n"
        "The original by Bullfrog\n"
        "Ported by Unavowed and Gynvael Coldwind.\n"
        "Expanded by other fans, signed in commits.\n"
        "Web site: https://github.com/swfans/syndwarsfx/\n",
        rel_kind);


    if (LbErrorLogSetup(NULL, NULL, Lb_ERROR_LOG_NEW) != Lb_SUCCESS) {
        printf("Execution log setup failed\n");
        return false;
    }

    LbSyncLog("Application: "PACKAGE" ver "VERSION" (%s release)\n", rel_kind);

    return true;
}

static void reset_log(void)
{
    LbSyncLog("Application: "PACKAGE" ver "VERSION" closing\n");
    LbErrorLogReset();
}

// To be moved to its own file when there are more tests
static void tests_execute(void)
{
    // TODO prpare tests
    //if (!test_gpoly())
    //    exit(51);
    exit(0);
}

static TbBool process_options(int *argc, char ***argv)
{
    int index;
    int val;
    char *argv0;
    int tmpint;

    static struct option options[] =
    {
      {"windowed",    0, NULL, 'W'},
      {"no-stretch",  0, NULL, 'S'},
      {"level-deep-fix", 0, NULL, 'L'},
      {"self-test",   0, NULL, 't'},
      {"help",        0, NULL, 'h'},
      {NULL,          0, NULL,  0 },
    };

    argv0 = (*argv)[0];
    index = 0;

    while ((val = getopt_long (*argc, *argv, "ABCDd:E:FgHhI:Ll:m:Np:qrSs:Ttu:Ww", options, &index)) >= 0)
    {
        LOGDBG("Command line option: '%c'", val);
        switch (val)
        {
        case 'A':
            ingame.GameMode = GamM_Unkn1;
            ingame.cmdln_param_a = 3200;
            break;

        case 'B':
            cmdln_param_bcg = 99;
            break;

        case 'C':
            cmdln_param_bcg = 100;
            break;

        case 'D':
            keyboard_mode_direct = 1;
            break;

        case 'd':
            for (tmpint = 0; optarg[tmpint] != '\0'; tmpint++)
            {
                switch (optarg[tmpint])
                {
                case 't':
                    debug_hud_things = true;
                    break;
                case 'o':
                    byte_1C844F = 1;
                    break;
                case 'c':
                    debug_hud_collision = 1;
                    break;
                default:
                    LOGERR("Invalid value after '-d' parameter. Unexpected char '%c'.", optarg[tmpint]);
                    return false;
                }
            }
            break;

        case 'E':
            tmpint = atoi(optarg);
            if ( JoySetInterrupt(tmpint) != -1 )
              unkn01_maskarr[17] = 17;
            break;

        case 'F':
            cmdln_colour_tables = 2;
            break;

        case 'g':
            cmdln_param_bcg = 1;
            ingame.GameMode = GamM_Unkn3;
            ingame.Flags |= GamF_Unkn0008;
            break;

        case 'H':
            game_high_resolution = true;
            break;

        case 'h':
            print_help (argv0);
            exit (0);
            break;

        case 'I':
            tmpint = atoi(optarg);
            LbNetworkSetupIPXAddress(tmpint);
            break;

        case 'L':
            level_deep_fix = true;
            break;

        case 'l':
            for (tmpint = 0; optarg[tmpint] != '\0'; tmpint++)
            {
                switch (optarg[tmpint])
                {
                case 's':
                    debug_log_things |= 0x01;
                    break;
                default:
                    LOGERR("Invalid value after '-l' parameter. Unexpected char '%c'.", optarg[tmpint]);
                    return false;
                }
            }
            break;

        case 'm':
            is_single_game = 1;
            if (sscanf(optarg, "%hhu,%d", &background_type, &tmpint) != 2)
            {
                LOGERR("Invalid value after '-m' parameter. Required two comma separated ints.");
                return false;
            }
            ingame.GameMode = GamM_Unkn2;
            ingame.Flags |= GamF_Unkn0008;
            ingame.CurrentMission = tmpint;
            ingame.UserFlags |= UsrF_Cheats;
            LOGDBG("Campaign %d mission index %d", (int)background_type, (int)ingame.CurrentMission);
            break;

        case 'N':
            cmdln_param_n = 1;
            break;

        case 'p':
            is_single_game = 1;
            pktrec_mode = PktR_PLAYBACK;
            packet_rec_no = atoi(optarg);
            LOGDBG("packet file play %hu", packet_rec_no);
            break;

        case 'q':
            ingame.Flags |= GamF_SkipIntro;
            break;

        case 'r':
            pktrec_mode = PktR_RECORD;
            LOGDBG("packet file record enabled");
            break;

        case 'S':
            cmdln_lores_stretch = false;
            break;

        case 's':
            sprintf(session_name, "%s", optarg);
            LOGDBG("session name '%s'", session_name);
            break;

        case 'T':
            cmdln_colour_tables = 1;
            break;

        case 't':
            tests_execute();
            break;

        case 'u':
            sprintf(user_name, "%s", optarg);
            strcpy(login_name, user_name);
            LOGDBG("user name '%s'", user_name);
            break;

        case 'W':
            cmdln_fullscreen = false;
            break;

        case 'w':
            ingame.LowerMemoryUse = 1;
            break;

        default:
            LOGERR("Command line parser error");
            return false;
        }
    }
    return true;
}

static void fixup_options(void)
{
    if (!is_single_game) {
        cmdln_param_bcg = 1;
    }
    // If game mode was not selected, set it to normal gameplay
    if (ingame.GameMode == GamM_None) {
        ingame.GameMode = GamM_Unkn3;
        ingame.Flags |= GamF_Unkn0008;
    }
}

void read_conf_file(void)
{
    TbFileHandle conf_fh;
    int i;
    int cmd_num;
    char locbuf[1024];
    char locstr[40];
    struct TbIniParser parser;
    char *conf_fname = "config.ini";
    int conf_len;

    conf_fh = LbFileOpen(conf_fname, Lb_FILE_MODE_READ_ONLY);
    if (conf_fh != INVALID_FILE) {
        conf_len = LbFileRead(conf_fh, locbuf, sizeof(locbuf));
        LOGSYNC("Processing %s file, %d bytes", conf_fname, conf_len);
        LbFileClose(conf_fh);
    } else {
        LOGERR("Could not open installation config file, going with defaults.");
        conf_len = 0;
    }
    locbuf[conf_len] = '\0';
    LbIniParseStart(&parser, locbuf, conf_len);
    // Parse the loaded file
#define COMMAND_TEXT(cmd_num) LbNamedEnumGetName(conf_file_cmnds,cmd_num)
#define CONFWRNLOG(format,args...) LOGWARN("%s(line %lu): " format, conf_fname, parser.line_num, ## args)
#define CONFDBGLOG(format,args...) LOGDBG("%s(line %lu): " format, conf_fname, parser.line_num, ## args)

    while (parser.pos < parser.buflen)
    {
        // Finding command number in this line
        i = 0;
        cmd_num = LbIniRecognizeKey(&parser, conf_file_cmnds);
        // Now store the config item in correct place
        switch (cmd_num)
        {
        case ConfCmd_CD:
            i = LbIniValueGetStrWhole(&parser, cd_drive, sizeof(cd_drive));
            if (i <= 0) {
                CONFWRNLOG("Couldn't read \"%s\" command parameter.", COMMAND_TEXT(cmd_num));
                break;
            }
            CONFDBGLOG("Dir with CD data '%s'", cd_drive);
            break;
        case ConfCmd_InstallDrive:
            // option ignored
            break;
        case ConfCmd_Language:
            i = LbIniValueGetStrWhole(&parser, language_3str, sizeof(language_3str));
            if (i <= 0) {
                CONFWRNLOG("Couldn't read \"%s\" command parameter.", COMMAND_TEXT(cmd_num));
                break;
            }
            for (i = 0; i < 3; i++) {
                language_3str[i] = tolower(language_3str[i]);
            }
            language_3str[i] = '\0';
            CONFDBGLOG("Language '%s'", language_3str);
            break;
        case ConfCmd_Data:
            i = LbIniValueGetNamedEnum(&parser, conf_file_disk_inst_lev);
            if (i <= 0) {
                CONFWRNLOG("Couldn't recognize \"%s\" command parameter.", COMMAND_TEXT(cmd_num));
                break;
            }
            game_dirs[DirPlace_Data].use_cd = (i != 2);
            break;
        case ConfCmd_Intro:
            i = LbIniValueGetNamedEnum(&parser, conf_file_disk_inst_lev);
            if (i <= 0) {
                CONFWRNLOG("Couldn't recognize \"%s\" command parameter.", COMMAND_TEXT(cmd_num));
                break;
            }
            game_dirs[DirPlace_Intro].use_cd = (i != 2);
            break;
        case ConfCmd_Anims:
            i = LbIniValueGetNamedEnum(&parser, conf_file_disk_inst_lev);
            if (i <= 0) {
                CONFWRNLOG("Couldn't recognize \"%s\" command parameter.", COMMAND_TEXT(cmd_num));
                break;
            }
            game_dirs[DirPlace_Equip].use_cd = (i != 2);
            break;
        case ConfCmd_Maps:
            i = LbIniValueGetNamedEnum(&parser, conf_file_disk_inst_lev);
            if (i <= 0) {
                CONFWRNLOG("Couldn't recognize \"%s\" command parameter.", COMMAND_TEXT(cmd_num));
                break;
            }
            game_dirs[DirPlace_Maps].use_cd = (i != 2);
            break;
        case ConfCmd_Levels:
            i = LbIniValueGetNamedEnum(&parser, conf_file_disk_inst_lev);
            if (i <= 0) {
                CONFWRNLOG("Couldn't recognize \"%s\" command parameter.", COMMAND_TEXT(cmd_num));
                break;
            }
            game_dirs[DirPlace_Levels].use_cd = (i != 2);
            break;
        case ConfCmd_Sound:
            i = LbIniValueGetNamedEnum(&parser, conf_file_disk_inst_lev);
            if (i <= 0) {
                CONFWRNLOG("Couldn't recognize \"%s\" command parameter.", COMMAND_TEXT(cmd_num));
                break;
            }
            game_dirs[DirPlace_Sound].use_cd = (i != 2);
            break;
        case ConfCmd_DOS:
            i = LbIniValueGetNamedEnum(&parser, conf_file_disk_inst_lev);
            if (i <= 0) {
                CONFWRNLOG("Couldn't recognize \"%s\" command parameter.", COMMAND_TEXT(cmd_num));
                break;
            }
            //game_dirs[DirPlace_?].use_cd = (i != 2);// option ignored
            break;
        case ConfCmd_ResGameHi:
        case ConfCmd_ResGameLo:
        case ConfCmd_ResMenu:
        case ConfCmd_ResFMVVidHi:
        case ConfCmd_ResFMVidLo:
            i = LbIniValueGetStrWhole(&parser, locstr, sizeof(locstr));
            if (i <= 0) {
                CONFWRNLOG("Couldn't read \"%s\" command parameter.", COMMAND_TEXT(cmd_num));
                break;
            }
            i = LbRegisterVideoModeString(locstr);
            if (i == Lb_SCREEN_MODE_INVALID) {
                CONFWRNLOG("Couldn't register \"%s\" mode \"%s\".", COMMAND_TEXT(cmd_num), locstr);
                break;
            }
            CONFDBGLOG("Resolution %s set to '%s' mode %d", COMMAND_TEXT(cmd_num), locstr, i);
            switch (cmd_num)
            {
            case ConfCmd_ResGameHi:
                screen_mode_game_hi = i;
                break;
            case ConfCmd_ResGameLo:
                screen_mode_game_lo = i;
                break;
            case ConfCmd_ResMenu:
                screen_mode_menu = i;
                break;
            case ConfCmd_ResFMVVidHi:
                screen_mode_fmvid_hi = i;
                break;
            case ConfCmd_ResFMVidLo:
                screen_mode_fmvid_lo = i;
                break;
            }
            break;
        case 0: // comment
            break;
        case -1: // end of buffer
            break;
        default:
            CONFWRNLOG("Unrecognized command.");
            break;
        }
        LbIniSkipToNextLine(&parser);
    }
#undef CONFDBGLOG
#undef CONFWRNLOG
#undef COMMAND_TEXT
    LbIniParseEnd(&parser);
}

int
main (int argc, char **argv)
{
    // the initial mode will be overwritten in game_setup()
    lbDisplay.ScreenMode = Lb_SCREEN_MODE_320_200_8;
    game_high_resolution = false;
    ingame.GameMode = GamM_None;
    ingame.LowerMemoryUse = 0;
    ingame.Flags = 0;
    setup_log();
    /* Gravis Grip joystick driver initialization */
    joy_driver_init();

    if (!process_options(&argc, &argv))
        return 1;

    fixup_options();
    adjust_memory_use();

    if (!game_initialise())
        return 1;

    read_conf_file();
    setup_language_file_names();

    display_set_full_screen(cmdln_fullscreen);
    display_set_lowres_stretch(cmdln_lores_stretch);

    set_default_user_settings();
    read_strings_file();
    if (!game_setup())
        return 1;

    game_process();

    game_reset();
    if ( in_network_game ) {
        LbNetworkReset();
    }
    joy_driver_shutdown();
    reset_log();
    LbMemoryReset();
    game_quit();
    // the above function never returns
    return 0;
}
