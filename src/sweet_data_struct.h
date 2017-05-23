#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "sweet_c_parser.h"

#define MAX_STACK_DEPTH             16      // how much operand could be stacked

typedef struct _MACRO_NFO
{
    unsigned int MacroType;
    char *MacroName;
    char *Position;
	int activeBlock;
} MACRO_NFO;

typedef MACRO_NFO S_ELEM;

typedef struct _S_STACK
{
    S_ELEM *base;
    S_ELEM *top;
} S_STACK;

typedef SWEET_C_FILE_INFO S_TREE_NODE_INFO;
typedef unsigned int S_TREE_TAG;

typedef struct _S_C_PROJECT_INFO
{
    unsigned int FileCount;
} S_C_PROJECT_INFO;

typedef struct _S_TREE_NODE
{
    S_TREE_NODE_INFO    data;
    //struct _S_TREE_NODE *parent;
    struct _S_TREE_NODE *lChild, *rChild;
    S_TREE_TAG          lTag, rTag;         // tag = 0: child; tag = 1: thread
} S_TREE_NODE;

typedef struct _S_TREE_ROOT         // stores all the information during parsing
{
    S_TREE_NODE *sTree;             // the actual root node
    S_C_PROJECT_INFO sProjInfo;     // overall project info
} S_TREE_ROOT;

/* little stack */
extern void     stack_init      (S_STACK **         );
extern int      stack_length    (S_STACK *          );
extern S_ELEM*  stack_gettop    (S_STACK *          );  // only retrieve the value, won't be ejected
extern void     stack_push      (S_STACK *, S_ELEM *);
extern S_ELEM*  stack_pop       (S_STACK *          );

/* little tree */
extern int tree_init      (S_TREE_ROOT **   );
extern int tree_destroy   (S_TREE_ROOT *    );

extern int tree_create_child   (S_TREE_NODE **);
extern int tree_delete_child   (S_TREE_NODE **);
extern int tree_attach_child   (S_TREE_NODE *, int , S_TREE_NODE *);

extern int tree_traverse_node  (S_TREE_NODE *, int (* )(S_TREE_NODE_INFO *));

#ifdef __cplusplus
}
#endif