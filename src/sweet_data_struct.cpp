// sweet_tree.cpp
//

#include "stdafx.h"
#include "sweet_data_struct.h"

/* STACK */
// tiny little stack
void stack_init(S_STACK **s_stack)
{
    *s_stack = (S_STACK *)malloc(sizeof(S_STACK));
    (*s_stack)->base = (S_ELEM *)malloc(sizeof(S_ELEM) * MAX_STACK_DEPTH);
	memset((*s_stack)->base, 0, sizeof(S_ELEM) * MAX_STACK_DEPTH);
    (*s_stack)->top = (*s_stack)->base;
}

int stack_length(S_STACK *s_stack)
{
    return s_stack->top - s_stack->base;
}

S_ELEM *stack_gettop(S_STACK *s_stack)  // only retrieve the value, won't be removed
{
    if (s_stack->top == s_stack->base)
    {
        return NULL;
    }
    return s_stack->top - 1;
}

void stack_push(S_STACK *s_stack, S_ELEM *elem)
{
    if ((s_stack->top - s_stack->base) == MAX_STACK_DEPTH)   // boundary check
        return ;//assert;
    //memcpy(&(*s_stack->top), &elem, sizeof(S_ELEM));
    memcpy(s_stack->top, elem, sizeof(S_ELEM));
    //CopyMemory(s_stack->top, elem, sizeof(S_ELEM));
    s_stack->top ++;
}

S_ELEM *stack_pop(S_STACK *s_stack)
{
    if (s_stack->top == s_stack->base)
        return NULL;  // overflow (down)
    s_stack->top --;
    return s_stack->top;
}


/* TREE */
int tree_init(S_TREE_ROOT **sTreeRoot)
{
    *sTreeRoot = (S_TREE_ROOT *)malloc(sizeof(S_TREE_ROOT));
    //memset(*sTreeRoot, 0x0, sizeof(SWEET_TREE_ROOT));
    ZeroMemory(*sTreeRoot, sizeof(S_TREE_ROOT));
    // ISSUE: y memset corrupts?!

    // create the root node:
    (*sTreeRoot)->sTree = (S_TREE_NODE *)malloc(sizeof(S_TREE_NODE));
//    memset(&(*(*sTreeRoot)->sTree), 0x1, sizeof(S_TREE_NODE));    // strange issue
//    memset((*sTreeRoot)->sTree, 0x2, sizeof(S_TREE_NODE));    // strange issue
    ZeroMemory((*sTreeRoot)->sTree, sizeof(S_TREE_NODE));

    return 1;
}

int tree_destroy(S_TREE_ROOT *sTree)
{

    return 1;
}

int tree_create_child(S_TREE_NODE **sNode) // create in memory, but not assigned to parent
{
    *sNode = (S_TREE_NODE *)malloc(sizeof(S_TREE_NODE));
    //memset(*sNode, 0x0, sizeof(SWEET_TREE_NODE));
    ZeroMemory(*sNode, sizeof(S_TREE_NODE));
    return 1;
}

int tree_delete_child(S_TREE_NODE **sNode)
{
    free(sNode);
    return 1;
}

int tree_attach_child(S_TREE_NODE *sParent, int isRight, S_TREE_NODE *sChild)
{
    //sChild->parent = sParent;
    if (isRight)
    {
        sParent->rChild = sChild;
    }
    else
    {
        sParent->lChild = sChild;
    }
    return 1;
}

int tree_traverse_node(S_TREE_NODE *sNode, int (* visit)(S_TREE_NODE_INFO *sNodeInfo))
{
    if (sNode)
    {
        if (visit(&sNode->data))
        {
            if (tree_traverse_node(sNode->lChild, visit))
            {
                if (tree_traverse_node(sNode->rChild, visit))
                {
                    return 1;
                }
            }
        }
        return 0;
    }
    return 1;
}