/*
escseq.h - ANSI Escape Sequences
Modified 2022-08-04
*/

/* https://en.wikipedia.org/wiki/ANSI_escape_code */
/* https://docs.microsoft.com/en-us/windows/console/console-virtual-terminal-sequences */

#ifndef ESCSEQ_H
#define ESCSEQ_H

/* Options. */
#define ESCSEQ_ENABLE

#ifdef ESCSEQ_WIDE
#define ESCSEQ_WIDTH__ L""
#else
#define ESCSEQ_WIDTH__ ""
#endif

#ifdef ESCSEQ_ENABLE
#define ESCSEQ__(sequence) ESCSEQ_WIDTH__ sequence
#else
#define ESCSEQ__(sequence) ESCSEQ_WIDTH__
#endif

/*
Colors
*/
/* Dim Foreground */
#define COLOR_FG_BLACK 30
#define COLOR_FG_RED 31
#define COLOR_FG_GREEN 32
#define COLOR_FG_YELLOW 33
#define COLOR_FG_BLUE 34
#define COLOR_FG_MAGENTA 35
#define COLOR_FG_CYAN 36
#define COLOR_FG_WHITE 37
/* Dim Background */
#define COLOR_BG_BLACK 40
#define COLOR_BG_RED 41
#define COLOR_BG_GREEN 42
#define COLOR_BG_YELLOW 43
#define COLOR_BG_BLUE 44
#define COLOR_BG_MAGENTA 45
#define COLOR_BG_CYAN 46
#define COLOR_BG_WHITE 47
/* Bright Foreground */
#define COLOR_FG_BRIGHT_BLACK 90
#define COLOR_FG_BRIGHT_RED 91
#define COLOR_FG_BRIGHT_GREEN 92
#define COLOR_FG_BRIGHT_YELLOW 93
#define COLOR_FG_BRIGHT_BLUE 94
#define COLOR_FG_BRIGHT_MAGENTA 95
#define COLOR_FG_BRIGHT_CYAN 96
#define COLOR_FG_BRIGHT_WHITE 97
/* Bright Background */
#define COLOR_BG_BRIGHT_BLACK 100
#define COLOR_BG_BRIGHT_RED 101
#define COLOR_BG_BRIGHT_GREEN 102
#define COLOR_BG_BRIGHT_YELLOW 103
#define COLOR_BG_BRIGHT_BLUE 104
#define COLOR_BG_BRIGHT_MAGENTA 105
#define COLOR_BG_BRIGHT_CYAN 106
#define COLOR_BG_BRIGHT_WHITE 107

/*
Introducers
*/
#define ESC(sequence) ESCSEQ__("\33" sequence) /* Escape Code */
#define CSI(sequence) ESC("[" sequence) /* Control Sequence Introducer */
#define SGR__(n) CSI(#n "m") /* SGR Base */
#define SGR(n) SGR__(n) /* Select Graphic Rendition */
#define OSC__(n, sequence) ESC("]" #n ";" sequence "\a") /* OSC Base */
#define OSC(n, sequence) OSC__(n, sequence) /* Operating System Command */
#define COLOR_STR(color) ESCSEQ__(#color) /* Stringize Color */

/*
Simple Cursor Positioning
*/
#define CUU1 ESC("A") /* Cursor Up by 1 */
#define CUD1 ESC("B") /* Cursor Down by 1 */
#define CUF1 ESC("C") /* Cursor Forward by 1 */
#define CUB1 ESC("D") /* Cursor Backward by 1 */
#define RI ESC("M") /* Reverse Index */
#define SCP ESC("7") /* Save Current Cursor Position */
#define RCP ESC("8") /* Restore Saved Cursor Position */

/*
Cursor Positioning
*/
#define CUU(n) CSI(#n "A") /* Cursor Up */
#define CUD(n) CSI(#n "B") /* Cursor Down */
#define CUF(n) CSI(#n "C") /* Cursor Forward */
#define CUB(n) CSI(#n "D") /* Cursor Back */
#define CNL(n) CSI(#n "E") /* Cursor Next Line */
#define CPL(n) CSI(#n "F") /* Cursor Previous Line */
#define CHA(n) CSI(#n "G") /* Cursor Horizontal Absolute */
#define VPA(n) CSI(#n, "d") /* Cursor Vertical Absolute */
#define CUP(x, y) CSI(#y ";" #x "H") /* Cursor Position */
#define HVP(x, y) CSI(#y ";" #x "f") /* Horizontal Vertical Position */
#define ANSISYS_SCP CSI("s") /* Save Current Cursor Position */
#define ANSISYS_RCP CSI("u") /* Restore Saved Cursor Position */

/*
Cursor Visibility
*/
#define CURBLINK CSI("?12h") /* Enable Cursor Blinking */
#define CURNOBLINK CSI("?12l") /* Disable Cursor Blinking */
#define CURSHOW CSI("?25h") /* Show Cursor */
#define CURHIDE CSI("?25l") /* Hide Cursor */

/*
Viewport Positioning
*/
#define SU(n) CSI(#n "S") /* Scroll Up */
#define SD(n) CSI(#n "T") /* Scroll Down */

/*
Text Modification
*/
#define ICH(n) CSI(#n "@") /* Insert Character */
#define DCH(n) CSI(#n "P") /* Delete Character */
#define ECH(n) CSI(#n "X") /* Erase Character */
#define IL(n) CSI(#n "L") /* Insert Line */
#define DL(n) CSI(#n "M") /* Delete Line */
#define ED(n) CSI(#n "J") /* Erase in Display */
#define CLS ED(2) /* Clear Entire Screen */
#define CLSL ED(1) /* Clear Screen Leftwards */
#define CLSR ED() /* Clear Screen Rightwards */
#define EL(n) CSI(#n "K") /* Erase in Line */
#define CLL EL(2) /* Clear Entire Line */
#define CLLL EL(1) /* Clear Line Leftwards */
#define CLLR EL() /* Clear Line Rightwards */

/*
Text Formatting
*/
#define RESET SGR(0)
#define BRIGHT SGR(1)
#define BOLD BRIGHT
#define NOBRIGHT SGR(21) SGR(22) /* Conflicting Information. */
#define NOBOLD NOBRIGHT
#define UNDERLINE SGR(4)
#define NOUNDERLINE SGR(24)
#define INVERT SGR(7)
#define NOINVERT SGR(27)
#define FGRESET SGR(39)
#define BGRESET SGR(49)

/* Dim Foreground */
#define FGBLACK SGR(COLOR_FG_BLACK)
#define FGRED SGR(COLOR_FG_RED)
#define FGGREEN SGR(COLOR_FG_GREEN)
#define FGYELLOW SGR(COLOR_FG_YELLOW)
#define FGBLUE SGR(COLOR_FG_BLUE)
#define FGMAGENTA SGR(COLOR_FG_MAGENTA)
#define FGCYAN SGR(COLOR_FG_CYAN)
#define FGWHITE SGR(COLOR_FG_WHITE)
/* Dim Background */
#define BGBLACK SGR(COLOR_BG_BLACK)
#define BGRED SGR(COLOR_BG_RED)
#define BGGREEN SGR(COLOR_BG_GREEN)
#define BGYELLOW SGR(COLOR_BG_YELLOW)
#define BGBLUE SGR(COLOR_BG_BLUE)
#define BGMAGENTA SGR(COLOR_BG_MAGENTA)
#define BGCYAN SGR(COLOR_BG_CYAN)
#define BGWHITE SGR(COLOR_BG_WHITE)
/* Bright Foreground */
#define FGBBLACK SGR(COLOR_FG_BRIGHT_BLACK)
#define FGBRED SGR(COLOR_FG_BRIGHT_RED)
#define FGBGREEN SGR(COLOR_FG_BRIGHT_GREEN)
#define FGBYELLOW SGR(COLOR_FG_YELLOW)
#define FGBBLUE SGR(COLOR_FG_BRIGHT_BLUE)
#define FGBMAGENTA SGR(COLOR_FG_BRIGHT_MAGENTA)
#define FGBCYAN SGR(COLOR_FG_BRIGHT_CYAN)
#define FGBWHITE SGR(COLOR_FG_BRIGHT_WHITE)
/* Bright Background */
#define BGBBLACK SGR(COLOR_BG_BRIGHT_BLACK)
#define BGBRED SGR(COLOR_BG_BRIGHT_RED)
#define BGBGREEN SGR(COLOR_BG_BRIGHT_GREEN)
#define BGBYELLOW SGR(COLOR_BG_BRIGHT_YELLOW)
#define BGBBLUE SGR(COLOR_BG_BRIGHT_BLUE)
#define BGBMAGENTA SGR(COLOR_BG_BRIGHT_MAGENTA)
#define BGBCYAN SGR(COLOR_BG_BRIGHT_CYAN)
#define BGBWHITE SGR(COLOR_BG_BRIGHT_WHITE)

/* Extended Colors */
#define FG8BIT(n) SGR(38;5;n)
#define BG8BIT(n) SGR(48;5;n)
#define FGRGB(r, g, b) SGR(38;2;n)
#define BGRGB(r, g, b) SGR(48;2;n)

/* Screen Colors */
#define SETCOLOR(i, hexr, hexg, hexb) OSC(4, #i ";rgb:" #hexr "/" #hexg "/" #hexb)

/*
Query State
*/
#define DSR CSI("6n") /* Device Status Report */

/*
Tabs
*/
#define HTS ESC("H") /* Horizontal Tab Set */
#define CHT(n) CSI(#n "I") /* Cursor Forwards Tab */
#define CBT(n) CSI(#n "Z") /* Cursor Backwards Tab */
#define TBCC CSI("0g") /* Tab Clear (Current Column) */
#define TBCA CSI("3g") /* Tab Clear (All Columns) */

/*
Scrolling Margins
*/
#define STBM(topline, bottomline) CSI(#topline ";" #bottomline "r") /* Set Scrolling Region */

/*
Window Title
*/
#define TITLE(title) OSC(0, title)

/*
Alternate Screen Buffer
*/
#define ALTSCR1 CSI("?1049h") /* Enable Alternative Screen Buffer */
#define ALTSCR0 CSI("?1049l") /* Disable Alternative Screen Buffer */

#endif /* !ESCSEQ_H */
