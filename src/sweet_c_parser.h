#pragma once

#ifdef __cplusplus
extern "C" {
#endif

///#include "sweet_data_struct.h"

#ifndef _TEMP
#define MAX_INTERN_BUFFER           1024 * 1000
#endif


#define KEYWORD_DEFINE              "#define "

// switch
#define COUNT_0_LINE                1       // count line from 1

#define LINE_BUFFER_SIZE            1024    // the buffer to use during parse a single line (should > the max len of one line)
#define PARSE_BUFFER_INCREMENT      1024    // the amount to allocate when buffer gets full
 
#define MAX_FUNCTION_COUNT          256     // how many functions could a source file contain that i can parse
#define MAX_FUNCTION_NAME           512     // e.g. length of "int main()"
#define MAX_MACRO_COUNT             256     // how many macros caches
#define MAX_MACRO_NAME              256     // max length of "#define SOMETHING"

#define MACRO_KINDS                 6

// parsing lookout
#define MACRO_GENERAL_OFFSET        6       // length of "#ifdef 
#define ROUND_BRACKET_OFFSET        4       // ) {

//#define CHAR_ESCAPE                 0x1b    // ESC (027, 0x1b)
#define CHAR_ESCAPE                 0x20    // ESC (027, 0x1b)

enum _SWEET_MACRO_TYPE
{
    SWEET_MACRO_IFDEF,
    SWEET_MACRO_IFNDEF,
    SWEET_MACRO_UNDEF,
    SWEET_MACRO_ELSE,
    SWEET_MACRO_ELIF,
    SWEET_MACRO_ENDIF,
};

enum _PARSE_FSM
{
    FSM_PLAIN,                              // : ''
    FSM_MET_FORWARD_SLASH,                  // : '\'
    FSM_MET_SINGLE_BACKSLASH,               // : '/'
    FSM_MET_DOUBLE_BACKSLASH,               // : '//'
    FSM_MET_BACKSLASH_STAR,                 // : '/*'
    FSM_MET_BACKSLASH_STAR_STAR,            // : '/**'
    FSM_MET_SINGLE_QUOTE,                   // : '''
    FSM_MET_SINGLE_QUOTE_FORWARD_SLASH,     // : ''\'
    FSM_MET_DOUBLE_QUOTE,                   // : '"'
    FSM_MET_DOUBLE_QUOTE_FORWARD_SLASH,     // : '"\'
};

typedef struct _MACRO_NAME_TABLE
{
    char MacroType[MAX_MACRO_NAME];
    unsigned int MacroTypeIndex;
} MACRO_NAME_TABLE;

typedef struct _SWEET_C_FILE_INFO   // folder also included
{
    int FileType;

    TCHAR FilePath[MAX_PATH];
    TCHAR FileName[MAX_PATH];

    FILETIME FileTime;

    unsigned int FileSize;

    unsigned int VisibleCharCount;

    unsigned int LineCount;
    unsigned int SpaceCount;
    unsigned int TabCount;
    unsigned int CRCount;
    unsigned int LFCount;
    unsigned int CRLFCount;

    unsigned int CommentLineCount;
    unsigned int CodeLineCount;
    unsigned int EmptyLineCount;

    unsigned int Parse2ActiveCharCount;     // use these 2 vars to calculate the
    unsigned int Parse2InactiveCharCount;   // percentage of active/inactive blocks

    unsigned int FunctionCount;
    unsigned int MacroCount;

    unsigned int MAXCompliexity;

    char *FunctionList[MAX_FUNCTION_COUNT];

    // tree-view attrib:
    unsigned int Depth;

    HTREEITEM HTI;

} SWEET_C_FILE_INFO;

extern int sweet_c_parse(char *, SWEET_C_FILE_INFO *pCFInfo);

#ifdef __cplusplus
}
#endif