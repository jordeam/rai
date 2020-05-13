#ifndef _RB_TREE_INTERNAL_H_
#define _RB_TREE_INTERNAL_H_

#include "rb_tree.h"
#define Node rb_node_t

#define BLACK (uint8_t)(0)
#define RED (uint8_t)(1)

#define LESSER (uint8_t)(0)
#define EQUAL (uint8_t)(1)
#define GREATER (uint8_t)(2)

#ifndef NULL
#define NULL 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

struct Node* GetParent(struct Node* n);
struct Node* GetGrandParent(struct Node* n);
struct Node* GetUncle(struct Node* n);

void RotateLeft(struct Node* n);
void RotateRight(struct Node* n);

struct Node* Insert(struct rb_tree_t *tree, struct Node* n);
void InsertRecurse(struct Node* root, struct Node* n, uint8_t (*comparator)(void *, void *));
void InsertRepairTree(struct Node* n);
void InsertCase1(struct Node* n);
void InsertCase2(struct Node* n);
void InsertCase3(struct Node* n);
void InsertCase4(struct Node* n);
void InsertCase4Step2(struct Node* n);

void ReplaceNode(struct Node* n, struct Node* child);
void DeleteOneChild(struct Node* n);
void DeleteCase1(struct Node* n);
void DeleteCase2(struct Node* n);
void DeleteCase3(struct Node* n);
void DeleteCase4(struct Node* n);
void DeleteCase5(struct Node* n);
void DeleteCase6(struct Node* n);

#endif
