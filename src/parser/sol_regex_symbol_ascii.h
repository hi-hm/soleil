#ifndef _SOL_REGEX_SYMBOL_ASCII_H_
#define _SOL_REGEX_SYMBOL_ASCII_H_ 1

#include "sol_pattern.h"

#define SOL_REGEX_SYMBOL_ASCII_NUL 0 // \0

#define SOL_REGEX_SYMBOL_ASCII_HT 9 // \t
#define SOL_REGEX_SYMBOL_ASCII_EOL 10 // \n
#define SOL_REGEX_SYMBOL_ASCII_VT 11 // \v
#define SOL_REGEX_SYMBOL_ASCII_VT 12 // \f
#define SOL_REGEX_SYMBOL_ASCII_CR 13 // \r

#define SOL_REGEX_SYMBOL_ASCII_LBR 28 // (
#define SOL_REGEX_SYMBOL_ASCII_RBR 29 // )

#define SOL_REGEX_SYMBOL_ASCII_SPACE 32 // ' '
#define SOL_REGEX_SYMBOL_ASCII_SHARP 35 // #

#define SOL_REGEX_SYMBOL_ASCII_STAR 42 // *
#define SOL_REGEX_SYMBOL_ASCII_PLUS 43 // +
#define SOL_REGEX_SYMBOL_ASCII_COMMA 44 // ,
#define SOL_REGEX_SYMBOL_ASCII_MIUS 45 // -
#define SOL_REGEX_SYMBOL_ASCII_DOT 46 // .
#define SOL_REGEX_SYMBOL_ASCII_SLASH 47 // /

#define SOL_REGEX_SYMBOL_ASCII_N_0 48
#define SOL_REGEX_SYMBOL_ASCII_N_9 57

#define SOL_REGEX_SYMBOL_ASCII_COLON 58 // :
#define SOL_REGEX_SYMBOL_ASCII_SEMICOLON 59 // ;

#define SOL_REGEX_SYMBOL_ASCII_QM 63 // ?

#define SOL_REGEX_SYMBOL_ASCII_L_A 65
#define SOL_REGEX_SYMBOL_ASCII_L_Z 90

#define SOL_REGEX_SYMBOL_ASCII_LSB 91 // [
#define SOL_REGEX_SYMBOL_ASCII_BS 92  /* \ */
#define SOL_REGEX_SYMBOL_ASCII_RSB 93 // ]

#define SOL_REGEX_SYMBOL_ASCII_L_a 97
#define SOL_REGEX_SYMBOL_ASCII_L_z 122

SolPattern* solRegexSymbolAscii_pattern();
int solRegexSymbolAscii_match(void*, void*);

#define solRegexSymbolAscii_char(s) (*(char*)s)

#endif