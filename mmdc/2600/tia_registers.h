
Overview

0000 - 002C  TIA Write
0000 - 000D  TIA Read(sometimes mirrored at 0030 - 003D)
0080 - 00FF  PIA RAM(128 bytes)
0280 - 0297  PIA Ports and Timer
F000 - FFFF  Cartridge Memory(4 Kbytes area)


Below lists address, name, used bits(ordered 76543210, "1" = used), and function of all I / O ports and memory.Ports marked as <strobe> are executing the the specified function when wrinting any data to it(the written data itself is ignored).

TIA - WRITE ADDRESS SUMMARY(Write only)

00      VSYNC   ......1.  vertical sync set - clear
01      VBLANK  11....1.  vertical blank set - clear
02      WSYNC   <strobe>  wait for leading edge of horizontal blank
03      RSYNC   <strobe>  reset horizontal sync counter
04      NUSIZ0  ..111111  number - size player - missile 0
05      NUSIZ1  ..111111  number - size player - missile 1
06      COLUP0  1111111.  color - lum player 0 and missile 0
07      COLUP1  1111111.  color - lum player 1 and missile 1
08      COLUPF  1111111.  color - lum playfield and ball
09      COLUBK  1111111.  color - lum background
0A      CTRLPF  ..11.111  control playfield ball size & collisions
0B      REFP0   ....1...  reflect player 0
0C      REFP1   ....1...  reflect player 1
0D      PF0     1111....  playfield register byte 0
0E      PF1     11111111  playfield register byte 1
0F      PF2     11111111  playfield register byte 2
10      RESP0   <strobe>  reset player 0
11      RESP1   <strobe>  reset player 1
12      RESM0   <strobe>  reset missile 0
13      RESM1   <strobe>  reset missile 1
14      RESBL   <strobe>  reset ball
15      AUDC0   ....1111  audio control 0
16      AUDC1   ....1111  audio control 1
17      AUDF0   ...11111  audio frequency 0
18      AUDF1   ...11111  audio frequency 1
19      AUDV0   ....1111  audio volume 0
1A      AUDV1   ....1111  audio volume 1
1B      GRP0    11111111  graphics player 0
1C      GRP1    11111111  graphics player 1
1D      ENAM0   ......1.  graphics(enable) missile 0
1E      ENAM1   ......1.  graphics(enable) missile 1
1F      ENABL   ......1.  graphics(enable) ball
20      HMP0    1111....  horizontal motion player 0
21      HMP1    1111....  horizontal motion player 1
22      HMM0    1111....  horizontal motion missile 0
23      HMM1    1111....  horizontal motion missile 1
24      HMBL    1111....  horizontal motion ball
25      VDELP0  .......1  vertical delay player 0
26      VDELP1  .......1  vertical delay player 1
27      VDELBL  .......1  vertical delay ball
28      RESMP0  ......1.  reset missile 0 to player 0
29      RESMP1  ......1.  reset missile 1 to player 1
2A      HMOVE   <strobe>  apply horizontal motion
2B      HMCLR   <strobe>  clear horizontal motion registers
2C      CXCLR   <strobe>  clear collision latches

TIA - READ ADDRESS SUMMARY(Read only)

30      CXM0P   11......read collision M0 - P1, M0 - P0(Bit 7, 6)
31      CXM1P   11......read collision M1 - P0, M1 - P1
32      CXP0FB  11......read collision P0 - PF, P0 - BL
33      CXP1FB  11......read collision P1 - PF, P1 - BL
34      CXM0FB  11......read collision M0 - PF, M0 - BL
35      CXM1FB  11......read collision M1 - PF, M1 - BL
36      CXBLPF  1.......  read collision BL - PF, unused
37      CXPPMM  11......read collision P0 - P1, M0 - M1
38      INPT0   1.......  read pot port
39      INPT1   1.......  read pot port
3A      INPT2   1.......  read pot port
3B      INPT3   1.......  read pot port
3C      INPT4   1.......  read input
3D      INPT5   1.......  read input


PIA 6532 - RAM, Switches, and Timer(Read / Write)

80..FF  RAM     11111111  128 bytes RAM(in PIA chip) for variables and stack
0280    SWCHA   11111111  Port A; input or output(read or write)
0281    SWACNT  11111111  Port A DDR, 0 = input, 1 = output
0282    SWCHB   11111111  Port B; console switches(read only)
0283    SWBCNT  11111111  Port B DDR(hardwired as input)
0284    INTIM   11111111  Timer output(read only)
0285    INSTAT  11......Timer Status(read only, undocumented)
0294    TIM1T   11111111  set 1 clock interval(838 nsec / interval)
0295    TIM8T   11111111  set 8 clock interval(6.7 usec / interval)
0296    TIM64T  11111111  set 64 clock interval(53.6 usec / interval)
0297    T1024T  11111111  set 1024 clock interval(858.2 usec / interval)

Cartridge Memory(4 Kbytes area)

F000 - FFFF ROM   11111111  Cartridge ROM(4 Kbytes max)
F000 - F07F RAMW  11111111  Cartridge RAM Write(optional 128 bytes)
F000 - F0FF RAMW  11111111  Cartridge RAM Write(optional 256 bytes)
F080 - F0FF RAMR  11111111  Cartridge RAM Read(optional 128 bytes)
F100 - F1FF RAMR  11111111  Cartridge RAM Read(optional 256 bytes)
003F      BANK  ......11  Cart Bank Switching(for some 8K ROMs, 4x2K)
FFF4 - FFFB BANK  <strobe>  Cart Bank Switching(for ROMs greater 4K)
FFFC - FFFD ENTRY 11111111  Cart Entrypoint(16bit pointer)
FFFE - FFFF BREAK 11111111  Cart Breakpoint(16bit pointer)



Atari 2600 Memory Map :
----------------------
$0000 - 002F TIA Primary Image
$0030 - 005F[shadow] TIA
$0060 - 007F[shadow - partial] TIA
$0080 - 00FF 128 bytes of RAM Primary Image(zero page image)
$0100 - 002F[shadow] TIA
$0130 - 005F[shadow] TIA
$0160 - 017F[shadow - partial] TIA
$0180 - 01FF[shadow] 128 bytes of RAM(CPU stack image)
$0200 - 022F[shadow] TIA
$0230 - 025F[shadow] TIA
$0260 - 027F[shadow - partial] TIA
$0280 - 029F 6532 - PIA I / O ports and timer Primary image
$02A0 - 02BF[shadow] 6532 - PIA
$02C0 - 02DF[shadow] 6532 - PIA
$02D0 - 02FF[shadow] 6532 - PIA
$0300 - 032F[shadow] TIA
$0330 - 035F[shadow] TIA
$0360 - 037F[shadow - partial] TIA
$0380 - 039F[shadow] 6532 - PIA
$03A0 - 03BF[shadow] 6532 - PIA
$03C0 - 03DF[shadow] 6532 - PIA
$03E0 - 03FF[shadow] 6532 - PIA
$0400 - 07FF[shadow] Repeat the pattern from $0000 - 03FF
$0800 - 0BFF[shadow] Repeat the pattern from $0000 - 03FF
$0C00 - 0FFF[shadow] Repeat the pattern from $0000 - 03FF

$1000 - 17FF Lower 2K Cartridge ROM(4K carts start here)
$1800 - 1FFF Upper 2K Cartridge ROM(2K carts go here)