// sweet_c_parser.c: Analysis single C++ source file
//
// !!!note!!!: this module process everything in multi-byte

/*
    --- the parsing algorithm ---
    
    composed of 3 phases, parse1, 2 and 3

    parse 1 perform the basic analysis of the source file,
    parse 2 expands the macros,
    while parse3 tries to look into lexical
*/

#include "stdafx.h"
#include "sweet_ui.h"

#include "sweet_c_parser.h"
#include "sweet_data_struct.h"

MACRO_NAME_TABLE macroKeyWordTable[] = {
    {"#ifdef"   , SWEET_MACRO_IFDEF       },
    {"#ifndef"  , SWEET_MACRO_IFNDEF      },
    {"#undef"   , SWEET_MACRO_UNDEF       },
    {"#else"    , SWEET_MACRO_ELSE        },
    {"#elif"    , SWEET_MACRO_ELIF        },
    {"#endif"   , SWEET_MACRO_ENDIF       },
};

int sweet_read_file             (char *, unsigned char **, unsigned int *);

int sweet_parse_1               (unsigned char *, unsigned int, SWEET_C_FILE_INFO *);
int sweet_parse_1_line          (unsigned char *, unsigned int, int *, SWEET_C_FILE_INFO *, char * );

int sweet_parse_2               (char *, SWEET_C_FILE_INFO *, char **);
int sweet_parse_2_evaluate_macro(S_STACK *, S_ELEM *, char **, unsigned int , int *);

int sweet_parse_3               (char *, SWEET_C_FILE_INFO *);
int sweet_parse_3_find_function (char *, char *, char **);

int sweet_c_parse(char *path, SWEET_C_FILE_INFO *pCFInfo)
{
    unsigned char *pFileBuffer = NULL;
    unsigned int fileSize = 0;

    char *pParse1Buffer = NULL;
    unsigned int parse1Size = 0;

    char *pParse2Buffer = NULL;

    // read in
    if (sweet_read_file(path, &pFileBuffer, &fileSize) < 0)  // allocate buffer requires to modify the base address of the puffer pointer
        return -1;
    // parse
    sweet_parse_1(pFileBuffer, fileSize, pCFInfo, &pParse1Buffer, &parse1Size);
    free(pFileBuffer);      // release mem

#ifndef _TEMP
    // !!!temp!!!
    // 大于 100 KB 爆
    pParse2Buffer = (char *)malloc(sizeof(char) * MAX_INTERN_BUFFER);
    memset(pParse2Buffer, 0, MAX_INTERN_BUFFER);
#endif // _TEMP

    sweet_parse_2(pParse1Buffer, pCFInfo, &pParse2Buffer);
    free(pParse1Buffer);    // dispose parse 1 buffer

    if (sweet_parse_3(pParse2Buffer, pCFInfo) == -1)
        ;   // do nothing, since nothing we can do to help the embarrass situation.
    free(pParse2Buffer);    // end, release

//     sweet_parse_3(pParse1Buffer, pCFInfo);
//     free(pParse1Buffer);    // end, release

    return 1;
}

int sweet_read_file(char *path, unsigned char **pFileBuffer, unsigned int *pFileSize)
{
    FILE *pFile;

#ifndef _TEMP
    int err;
#endif

    pFile = fopen(path, "rb");

#ifndef _TEMP
//     if (pFile == NULL)
//     {
//         err = GetLastError();
// 
//         pFile = fopen(path, "rb");
// 
//         MessageBox(NULL, _T("跳过一个不认识的文件"), NULL, NULL);
//         //perror( "fopen !");
//         //fclose(pFile);
//         return -1;
//     }
#endif

    // obtain file size:
    fseek (pFile, 0 , SEEK_END);
    *pFileSize = ftell(pFile);

#ifndef _TEMP
    if (*pFileSize > MAX_INTERN_BUFFER / 2)
        MessageBox(NULL, _T("文件太大，可能会爆"), _T("友情提醒"), NULL);
#endif

    // read the entire file into memory:
    rewind (pFile);

    // allocate the read buffer
    *pFileBuffer = (unsigned char *)malloc(sizeof(unsigned char) * (*pFileSize));
    fread (*pFileBuffer, 1, *pFileSize, pFile);

    fclose (pFile);

    return 1;
}

/*
    === parse 1 ===

    Parse 1 count the chars, lines, comment lines and preprocess the input lines for parse 2.

    Preprocess:
    1. eliminate the single quote, double quote which would cause confusion in analysis

        The following statement could severely damage the parsing:
            printf("funny thing // function name(){");

    2. eliminate the comments
    3. eliminate the line breaks
*/
int sweet_parse_1(unsigned char *pFileBuffer, unsigned int fileSize, SWEET_C_FILE_INFO *pCFInfo, char **pParse1Buffer, unsigned int *parse1Size)
{
    int parse1_fsm;     // finite state machine (parse 1)

    unsigned int    lineSize            = 0;            // length of current line
    unsigned int    remainSize          = fileSize;     // length of data remains to be proceed

    unsigned char   *pChA = NULL;                   // (\r) pointer from memchr
    unsigned char   *pChB = NULL;                   // (\n) pointer from memchr

    unsigned char   *lineHead = pFileBuffer;        // head of file

    char preprocessBuffer[LINE_BUFFER_SIZE];
    unsigned int preBufferLen           = 0;

    unsigned int parse1BufferAllocated  = 0;

// init some global vars:
//    s_init(&opStack);

    parse1_fsm = FSM_PLAIN;

    *pParse1Buffer = (char *)malloc(sizeof(char) * PARSE_BUFFER_INCREMENT);
    memset(*pParse1Buffer, 0, PARSE_BUFFER_INCREMENT);
    parse1BufferAllocated = PARSE_BUFFER_INCREMENT;

    while (remainSize)
    {
        pChA = (unsigned char *)memchr(lineHead, '\r', remainSize); // find line marker
        pChB = (unsigned char *)memchr(lineHead, '\n', remainSize);
        
        if (pChA || pChB)
        {   // end of file?
            if (!pChA && pChB)      // only has \n now:
                pChA = pChB;
            else if (pChA && !pChB) // only has \r now:
                pChB = pChA;

            // calculate the line size
            if (pChB - pChA == 1)   // \r\n
            {
                lineSize = pChB - lineHead + 1;
                pCFInfo->CRLFCount ++;  // special
            }
            else
            {
                (pChB > pChA) ? (lineSize = pChA - lineHead + 1) : (lineSize = pChB - lineHead + 1);
            }
        }
        else
        {   // no more \r OR \n:
            lineSize = remainSize;
        }

        memset(preprocessBuffer, 0, LINE_BUFFER_SIZE);      // clear prep buffer
        sweet_parse_1_line(lineHead, lineSize, &parse1_fsm, pCFInfo,  preprocessBuffer);

        // attach preprocess buffer:
        preBufferLen = strlen(preprocessBuffer);

        if (preBufferLen)   // attach line breaker
            preprocessBuffer[preBufferLen ++] = CHAR_ESCAPE;

        if (((*parse1Size) + preBufferLen) >= parse1BufferAllocated) // buffer full, enlarge
        {
            *pParse1Buffer = (char *)realloc(*pParse1Buffer, sizeof(char) * (parse1BufferAllocated + PARSE_BUFFER_INCREMENT));
            memset((*pParse1Buffer) + parse1BufferAllocated, 0, PARSE_BUFFER_INCREMENT);
            parse1BufferAllocated += PARSE_BUFFER_INCREMENT;
        }
        
        strcat(*pParse1Buffer, preprocessBuffer);
        *parse1Size += preBufferLen;

        lineHead += lineSize;   // move the pointer
        remainSize -= lineSize;
    }

    if (COUNT_0_LINE)
    {
        pCFInfo->LineCount ++;      // add line offset, since we human count from 1
    }

    return 1;
}

int sweet_parse_1_line(unsigned char *pLineHead, unsigned int lineSize, int *parse1_fsm, SWEET_C_FILE_INFO *pCFInfo, char *preprocessBuffer)
{
    char *bufferHead = preprocessBuffer;

    unsigned int lineHasContent = 0;
    // parse every char

    do
    {
        if (*pLineHead >= 32 && *pLineHead < 127)   // visible char
        {
            pCFInfo->VisibleCharCount ++;
        }
        switch (*pLineHead)
        {
        case ' ':
            pCFInfo->SpaceCount ++;
            break;
        case '\t':
            pCFInfo->TabCount ++;
            break;
        case '\r':
            pCFInfo->CRCount ++;
            break;
        case '\n':
            pCFInfo->LFCount ++;
            break;
        case '\\':  // forward slash (note! they are always in quote)
//             if (parse1_fsm == FSM_PLAIN)
//                 parse1_fsm = FSM_MET_FORWARD_SLASH;
            if (*parse1_fsm == FSM_MET_SINGLE_QUOTE)
                *parse1_fsm = FSM_MET_SINGLE_QUOTE_FORWARD_SLASH;
            else if (*parse1_fsm == FSM_MET_DOUBLE_QUOTE)
                *parse1_fsm = FSM_MET_DOUBLE_QUOTE_FORWARD_SLASH;
            else if (*parse1_fsm == FSM_MET_SINGLE_QUOTE_FORWARD_SLASH)
                *parse1_fsm = FSM_MET_SINGLE_QUOTE;
            else if (*parse1_fsm == FSM_MET_DOUBLE_QUOTE_FORWARD_SLASH)
                *parse1_fsm = FSM_MET_DOUBLE_QUOTE;
            lineHasContent ++;
            break;
        case '/':   // backslash
            if (*parse1_fsm == FSM_PLAIN)
                *parse1_fsm = FSM_MET_SINGLE_BACKSLASH;
            else if (*parse1_fsm == FSM_MET_SINGLE_BACKSLASH)
                *parse1_fsm = FSM_MET_DOUBLE_BACKSLASH;
            else if (*parse1_fsm == FSM_MET_BACKSLASH_STAR_STAR)
            {
                pCFInfo->CommentLineCount ++;
                *parse1_fsm = FSM_PLAIN; // End of comment
            }
            lineHasContent ++;
            break;
        case '*':   // star
            if (*parse1_fsm == FSM_MET_SINGLE_BACKSLASH)
                *parse1_fsm = FSM_MET_BACKSLASH_STAR;
            else if (*parse1_fsm == FSM_MET_BACKSLASH_STAR)
                *parse1_fsm = FSM_MET_BACKSLASH_STAR_STAR;
            else if (*parse1_fsm == FSM_PLAIN)
                *(preprocessBuffer ++) = *pLineHead;
            lineHasContent ++;
            break;
        case '\'':  // single quote
            if (*parse1_fsm == FSM_PLAIN)
            {
                *(preprocessBuffer ++) = *pLineHead;
                *parse1_fsm = FSM_MET_SINGLE_QUOTE;
            }
            else if (*parse1_fsm == FSM_MET_SINGLE_QUOTE)
            {
                *(preprocessBuffer ++) = *pLineHead;
                *parse1_fsm = FSM_PLAIN;
            }
            else if (*parse1_fsm == FSM_MET_SINGLE_QUOTE_FORWARD_SLASH)
            {
                *parse1_fsm = FSM_MET_SINGLE_QUOTE;
            }
            lineHasContent ++;
            break;
        case '"':   // double quote
            if (*parse1_fsm == FSM_PLAIN)
            {
                *(preprocessBuffer ++) = *pLineHead;
                *parse1_fsm = FSM_MET_DOUBLE_QUOTE;
            }
            else if (*parse1_fsm == FSM_MET_DOUBLE_QUOTE)
            {
                *(preprocessBuffer ++) = *pLineHead;
                *parse1_fsm = FSM_PLAIN;
            }
            else if (*parse1_fsm == FSM_MET_DOUBLE_QUOTE_FORWARD_SLASH)
            {
                *parse1_fsm = FSM_MET_DOUBLE_QUOTE;
            }
            lineHasContent ++;
            break;
        default:
// never met:
//             if (parse1_fsm == FSM_MET_FORWARD_SLASH)
//                 parse1_fsm = FSM_PLAIN;
            if (*parse1_fsm == FSM_PLAIN)
                *(preprocessBuffer ++) = *pLineHead;
            else if (*parse1_fsm == FSM_MET_SINGLE_BACKSLASH)
                *parse1_fsm = FSM_PLAIN;     // restore FSM for divide operator
            else if (*parse1_fsm == FSM_MET_SINGLE_QUOTE_FORWARD_SLASH)
                *parse1_fsm = FSM_MET_SINGLE_QUOTE;
            else if (*parse1_fsm == FSM_MET_DOUBLE_QUOTE_FORWARD_SLASH)
                *parse1_fsm = FSM_MET_DOUBLE_QUOTE;
            else if (*parse1_fsm == FSM_MET_BACKSLASH_STAR_STAR)
                *parse1_fsm = FSM_MET_BACKSLASH_STAR;
            lineHasContent ++;
            break;
        }
        pLineHead ++;
    } while (-- lineSize );

    switch (*parse1_fsm)
    {
    case FSM_MET_DOUBLE_BACKSLASH:
        pCFInfo->CommentLineCount ++;
        *parse1_fsm = FSM_PLAIN;
        break;
    case FSM_MET_BACKSLASH_STAR:
    case FSM_MET_BACKSLASH_STAR_STAR:
        pCFInfo->CommentLineCount ++;
        break;
    }

    if (lineHasContent)
    {
        if (preprocessBuffer - bufferHead)
            pCFInfo->CodeLineCount ++;
    }
    else
    { // empty line
        pCFInfo->EmptyLineCount ++;
    }

    pCFInfo->LineCount ++;  // sum up lines
    return 1;
}

/*
    === parse 2 ===

    Parse 2 expands the macros
*/
int sweet_parse_2(char *pParse1Buffer, SWEET_C_FILE_INFO *pCFInfo, char **pParse2Buffer)
{
    // for read in define.h:
    FILE *pFile;
    char lineBuffer[MAX_MACRO_NAME] = { 0 };
    char *pDefine;


    unsigned int remainSize = strlen(pParse1Buffer);
    char *pBufferTail = pParse1Buffer + remainSize;

    char *macroBuffer[MAX_MACRO_COUNT];
    unsigned int macroBufferCount = 0;


    char *pProud, *pLastProud = pParse1Buffer;
    int iMacroType;

    S_STACK *macroStack;                // stores currently macroed macro
    S_ELEM  macroNfo = { 0 };           //

    int activeBlock;                    // whether current block is active

    // init the stack
    stack_init(&macroStack);

    // read in the macro define file
    pFile = fopen("define.h", "r");     // temp solution

    // tmp:
    if (pFile == NULL)
    {
        MessageBox(NULL, _T("预定义文件 define.h，可能会爆！！"), NULL, NULL);
        return ;
    }

    while (!feof(pFile))
    {
        // TO DO: different point level
        macroBuffer[macroBufferCount] = (char *)malloc(sizeof(char) * MAX_MACRO_NAME);
        fgets(lineBuffer, LINE_BUFFER_SIZE, pFile);
        // dispose #define:
        pDefine = strstr(lineBuffer, KEYWORD_DEFINE);
        strcpy(macroBuffer[macroBufferCount], pDefine + strlen(KEYWORD_DEFINE));
        macroBufferCount ++;
    }
    
    fclose(pFile);

    // to ensure #ifdef 1:
    macroBuffer[macroBufferCount] = (char *)malloc(sizeof(char) * MAX_MACRO_NAME);
    strcpy(macroBuffer[macroBufferCount], "1\n");
    macroBufferCount ++;

    // find each macro now:
    pProud = (char *)memchr(pParse1Buffer, '#', remainSize);
    while (pProud)  // we should use remainsize instead? NO
    {
		activeBlock = 1;
        // find the pound sign '#'
        // once find, check which keyword
        if (pProud)
        {
			// are we correct???
			remainSize = pBufferTail - pProud;

            for (iMacroType = 0 ; iMacroType < MACRO_KINDS; iMacroType ++)
            { // search the macro table:
                if (strncmp(pProud, macroKeyWordTable[iMacroType].MacroType, strlen(macroKeyWordTable[iMacroType].MacroType)) == 0)
                { // match, then evaluate:
                    macroNfo.MacroType		= iMacroType;
                    macroNfo.Position		= pProud;
					macroNfo.activeBlock	= 1;
                    sweet_parse_2_evaluate_macro(macroStack, &macroNfo, macroBuffer, macroBufferCount, &activeBlock);
                }
            }

			if (activeBlock)
			{
				// if block is active, then attach it:
				strncat(*pParse2Buffer, pLastProud, pProud - pLastProud);
				pCFInfo->Parse2ActiveCharCount += (pProud - pLastProud);
			}
			else
			{
				pCFInfo->Parse2InactiveCharCount += (pProud - pLastProud);
			}

			pLastProud = pProud;
            pProud = (char *)memchr(pProud + 1, '#', remainSize);
        }
    }

    // copy the remaining content:
    strncat(*pParse2Buffer, pLastProud, remainSize);
	pCFInfo->Parse2ActiveCharCount += remainSize;

    // TO DO: calculate percentage of inactive blocks

    return 1;
}

// filter the inactive block:
/*
    [in]    macroStack  :   store currently macro structure
    [in]    newMacroNfo :   newly found macro description
    [in]    macroBuffer :   extracted from "define.h"
    [out]   activeBlock :   to tell whether upper block should be copied
*/
int sweet_parse_2_evaluate_macro(S_STACK *macroStack, S_ELEM *newMacroNfo, char **macroBuffer, unsigned int macroBufferCount, int *activeBlock)
{
    int matched = 0;
    int iMacroName;

    S_ELEM *lastMacroNfo, *last2MacroNfo;

    // match the macro name (e.g. find whether GSN has defined in define.h)
    if (newMacroNfo->MacroType >= SWEET_MACRO_IFDEF && newMacroNfo->MacroType <= SWEET_MACRO_IFNDEF)
    {
        for (iMacroName = 0 ; iMacroName < macroBufferCount; iMacroName ++)
        {
            if (strncmp(newMacroNfo->Position + strlen(macroKeyWordTable[newMacroNfo->MacroType].MacroType),
                macroBuffer[iMacroName], strlen(macroBuffer[iMacroName]) - 1) == 0)
            { // same, the value in iMacroName is valid
                newMacroNfo->MacroName = macroBuffer[iMacroName];
                break;
            }
        }
    }

    // calculate the active block:
    lastMacroNfo = stack_gettop(macroStack);
	if (lastMacroNfo)
		*activeBlock = lastMacroNfo->activeBlock;
//     if (lastMacroNfo)
//     {
//         switch(lastMacroNfo->MacroType)
//         {
//         case SWEET_MACRO_IFDEF:
//             if ((lastMacroNfo->MacroName) && (strcmp(lastMacroNfo->MacroName, "0") == 0))     // ifdef 0
//                 *activeBlock = 0;
//             break;
//         case SWEET_MACRO_IFNDEF:
// 			if (lastMacroNfo->MacroName)
// 				*activeBlock = 0;
//             break;
//         }
//     }

    // maintain the macro stack:
    switch (newMacroNfo->MacroType)
    {
    case SWEET_MACRO_IFDEF:
		if (lastMacroNfo && lastMacroNfo->activeBlock == 0)
			newMacroNfo->activeBlock = 0;
		else if ((!newMacroNfo->MacroName) || (strcmp(newMacroNfo->MacroName, "0") == 0))     // ifdef 0
			newMacroNfo->activeBlock = 0;
        stack_push(macroStack, newMacroNfo);
		break;
    case SWEET_MACRO_IFNDEF:
//        if (*activeBlock)
		if (lastMacroNfo && lastMacroNfo->activeBlock == 0)
			newMacroNfo->activeBlock = 0;
		else if (newMacroNfo->MacroName)
			newMacroNfo->activeBlock = 0;
        stack_push(macroStack, newMacroNfo);
        break;
    case SWEET_MACRO_ELSE:
        if (lastMacroNfo)
        {
            lastMacroNfo	= stack_pop(macroStack);
			last2MacroNfo	= stack_gettop(macroStack);	// warning!!!
            newMacroNfo->MacroName = lastMacroNfo->MacroName;
            if (lastMacroNfo->MacroType == SWEET_MACRO_IFDEF)
            {
                newMacroNfo->MacroType = SWEET_MACRO_IFNDEF;
// 				if (last2MacroNfo && last2MacroNfo->activeBlock == 0)
// 					newMacroNfo->activeBlock = 0;
				newMacroNfo->activeBlock = 0;
            }
            else if (lastMacroNfo->MacroType == SWEET_MACRO_IFNDEF)
            {
                newMacroNfo->MacroType = SWEET_MACRO_IFDEF;

                // tmp: bye-bye
                return ;

				if (lastMacroNfo && last2MacroNfo->activeBlock == 0)
					newMacroNfo->activeBlock = 0;
				else
					newMacroNfo->activeBlock = 1;
            }
            stack_push(macroStack, newMacroNfo);
        }
        break;
    case SWEET_MACRO_ENDIF:
        // free mem?
        stack_pop(macroStack);
        break;
    // *bye-bye here: they won't be supported
    case SWEET_MACRO_ELIF:
    case SWEET_MACRO_UNDEF:
    default:
        break;
    }

    return 1;
}

/*
    === parse 3 ===

    Parse 3 count the name of functions
*/
int sweet_parse_3(char *pParse2Buffer, SWEET_C_FILE_INFO *pCFInfo)
{
    //S_STACK *opStack;   // operand stack        (parse 2)
    unsigned int bracketLevel = 0;

    unsigned int remainSize = strlen(pParse2Buffer);

    char *pLBracket, *pRBracket, *pRRoundBacket;
    char *pBufferTail = pParse2Buffer + remainSize;

    char *pFunctionHead = NULL;

    //char functionNameBuffer[MAX_FUNCTION_NAME];

// init the function list
//    pCFInfo->FunctionList = malloc()

    // find the first couple
    pLBracket = (char *)memchr(pParse2Buffer, '{', remainSize);     // find left bracket
    pRBracket = (char *)memchr(pParse2Buffer, '}', remainSize);     // find right bracket

    while (remainSize && pRBracket)
    {
        if (pLBracket && (pLBracket < pRBracket))
        {
            if (bracketLevel == 0)
            { // met function header, find function name and add
                pRRoundBacket = (char *)memchr(pLBracket - ROUND_BRACKET_OFFSET, ')', ROUND_BRACKET_OFFSET);     // find round ')'
                if (pRRoundBacket)  // got '(',  which means this is a function
                {
                    if (!sweet_parse_3_find_function(pParse2Buffer, pLBracket, &pFunctionHead))
                        return 0;   // parse err -> fail

                    if (pFunctionHead > pParse2Buffer)
                    { // is function, record its name
                        pCFInfo->FunctionList[pCFInfo->FunctionCount] = (char *)malloc(sizeof(char) * MAX_FUNCTION_NAME);
                        memset(pCFInfo->FunctionList[pCFInfo->FunctionCount], 0, MAX_FUNCTION_NAME);

#ifndef _TEMP
                        if (pCFInfo->FunctionList[pCFInfo->FunctionCount] == NULL)
                        {
                            MessageBox(NULL, _T("MALLOC 失败！！！内存用完了？"), NULL, 0);
                            return -1;
                        }
#endif

                        // err
                        if (pLBracket - pFunctionHead < MAX_FUNCTION_NAME)
                            strncpy(pCFInfo->FunctionList[pCFInfo->FunctionCount], pFunctionHead, pLBracket - pFunctionHead);
                    //sweet_print(_T("%s"), functionNameBuffer);
                        pCFInfo->FunctionCount ++;
                    }
                }
            }
            bracketLevel ++;
            // left bracket '{' consumed,find next '{' :
            remainSize = pBufferTail - pLBracket;
            pLBracket = (char *)memchr(pLBracket + 1, '{', remainSize);
        }
        else
        {
            bracketLevel --;
            // find next '}' :
            remainSize = pBufferTail - pRBracket;
            pRBracket = (char *)memchr(pRBracket + 1, '}', remainSize);
        }
    }
    return 1;
}

// in in out
int sweet_parse_3_find_function(char *pParse1Buffer, char *pBackTrack, char **pFunctionHead)
{
    *pFunctionHead = pBackTrack;

    // find the left round bracket '(':
    while (((*pFunctionHead) >= pParse1Buffer) && ((**pFunctionHead) != '('))
    {
        (*pFunctionHead) --;
    }

    if ((*pFunctionHead) == pParse1Buffer)
        return 0;       // failed

    // TO DO: judge function wisely
    while (((*pFunctionHead) > pParse1Buffer) && ((**pFunctionHead) != CHAR_ESCAPE))
    {
        (*pFunctionHead) --;
    }

    return 1;
}
