#! /bin/bash

printf -- 'This is for reading, not executing !!\n' ;
exit -- 1 ;



# This tutorial guides you through creating a portable build.

# Acquisition
cd -- 'Your download directory' ;
git 'clone' --filter='blob:none' --branch 'master' --single-branch --verbose 'https://github.com/swfans/syndwarsfx' ;


# Ressources
'./util/install' -d '/dev/sr0' -f '/media/USERNAME/SYN WARS QA 9.0/' -t '/Your portable game directory/' -l 'eng' ;


# Configuration
autoreconf -fiv ;

declare -x -- PKG_CONFIG_PATH='/usr/lib/i386-linux-gnu/pkgconfig' \
              CFLAGS='-m32 -fno-PIC -fno-PIE -mno-direct-extern-access -std=gnu99 -fpermissive' \
              CXXFLAGS='-m32 -fno-PIC -fno-PIE -mno-direct-extern-access -fpermissive' \
              LDFLAGS='-m32 -fno-PIC -fno-PIE -no-pie -Wl,-z,notext -L/usr/lib/i386-linux-gnu' ;

mkdir -- './release' && cd -- './release' ;
'../configure' --disable-lb-wscreen-control --host='i686-linux-gnu' --with-data-path='.'

# Compilation
make 'clean' && make --jobs="$( nproc )" V='1'
make install prefix='/PWD of Your portable game directory/'

# Play
cd -- '/Your portable game directory/' ;
'./syndwarsfx' -g -H -q --windowed ;

# With full HD resolution set this settings within 'config.ini'
# Windows + Tab will releasse the mouse.
CD="D:\GAME\"
InstallDrive="C:\"
Language="English"
Intro="Max"
Anims="Max"
Sound="Max"
Levels="Max"
Data="Max"
Maps="Max"
DOS="Max"
MouseCapture="Always"
ResMenu="1600x900x8"
ResGameLo="640x480x8"
ResGameHi="1600x900x32"

