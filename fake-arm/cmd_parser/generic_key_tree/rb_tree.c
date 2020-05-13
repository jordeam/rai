#include <inttypes.h>

#include "rb_tree.h"
#include "rb_tree_internal.h"

// I copied this code from Wikipedia
// Helper functions:

struct Node* GetParent(struct Node* n) {
	// Note that parent is set to null for the root node.
	return n == NULL ? NULL : n->parent;
}

struct Node* GetGrandParent(struct Node* n) {
	// Note that it will return NULL if this is root or child of root
	return GetParent(GetParent(n));
}

struct Node* GetSibling(struct Node* n) {
	struct Node* p = GetParent(n);

	// No parent means no sibling.
	if (p == NULL) {
		return NULL;
	}

	if (n == p->left) {
		return p->right;
		} else {
		return p->left;
	}
}

struct Node* GetUncle(struct Node* n) {
	struct Node* p = GetParent(n);
	//struct Node* g = GetGrandParent(n);

	// No grandparent means no uncle
	return GetSibling(p);
}

void RotateLeft(struct Node* n) {
	struct Node* nnew = n->right;
	struct Node* p = GetParent(n);
	//assert(nnew != NULL);  // Since the leaves of a red-black tree are empty,
	// they cannot become internal nodes.
	n->right = nnew->left;
	nnew->left = n;
	n->parent = nnew;
	// Handle other child/parent pointers.
	if (n->right != NULL) {
		n->right->parent = n;
	}

	// Initially n could be the root.
	if (p != NULL) {
		if (n == p->left) {
			p->left = nnew;
			} else if (n == p->right) {
			p->right = nnew;
		}
	}
	nnew->parent = p;
}

void RotateRight(struct Node* n) {
	struct Node* nnew = n->left;
	struct Node* p = GetParent(n);
	//assert(nnew != NULL);  // Since the leaves of a red-black tree are empty,
	// they cannot become internal nodes.

	n->left = nnew->right;
	nnew->right = n;
	n->parent = nnew;

	// Handle other child/parent pointers.
	if (n->left != NULL) {
		n->left->parent = n;
	}

	// Initially n could be the root.
	if (p != NULL) {
		if (n == p->left) {
			p->left = nnew;
			} else if (n == p->right) {
			p->right = nnew;
		}
	}
	nnew->parent = p;
}

struct Node* Insert(struct rb_tree_t *tree, struct Node* n) {
	struct Node *root = tree->root;
	
	// Insert new struct Node into the current tree.
	InsertRecurse(root, n, tree->comparator);

	// Repair the tree in case any of the red-black properties have been violated.
	InsertRepairTree(n);

	// Find the new root to return.
	root = n;
	while (GetParent(root) != NULL) {
		root = GetParent(root);
	}
	return root;
}

void InsertRecurse(struct Node* root, struct Node* n, uint8_t (*comparator)(void *, void *)) {
	// Recursively descend the tree until a leaf is found.
	if (root != NULL) {
		if (comparator(n->key, root->key) == LESSER) {
			if (root->left != NULL) {
				InsertRecurse(root->left, n, comparator);
				return;
			}
			else {
				root->left = n;
			}
		}
		else {
			if (root->right != NULL) {
				InsertRecurse(root->right, n, comparator);
				return;
			}
			else {
				root->right = n;
			}
		}
	}

	// Insert new Node n.
	n->parent = root;
	n->left = NULL;
	n->right = NULL;
	n->color = RED;
}

void InsertRepairTree(struct Node* n) {
	if (GetParent(n) == NULL) {
		InsertCase1(n);
		} else if (GetParent(n)->color == BLACK) {
		InsertCase2(n);
		} else if (GetUncle(n) != NULL && GetUncle(n)->color == RED) {
		InsertCase3(n);
		} else {
		InsertCase4(n);
	}
}

void InsertCase1(struct Node* n) {
	if (GetParent(n) == NULL) {
		n->color = BLACK;
	}
}

void InsertCase2(struct Node* n) {
	// Do nothing since tree is still valid.
	return;
}

void InsertCase3(struct Node* n) {
	GetParent(n)->color = BLACK;
	GetUncle(n)->color = BLACK;
	GetGrandParent(n)->color = RED;
	InsertRepairTree(GetGrandParent(n));
}

void InsertCase4(struct Node* n) {
	struct Node* p = GetParent(n);
	struct Node* g = GetGrandParent(n);

	if (n == p->right && p == g->left) {
		RotateLeft(p);
		n = n->left;
		} else if (n == p->left && p == g->right) {
		RotateRight(p);
		n = n->right;
	}

	InsertCase4Step2(n);
}

void InsertCase4Step2(struct Node* n) {
	struct Node* p = GetParent(n);
	struct Node* g = GetGrandParent(n);

	if (n == p->left) {
		RotateRight(g);
		} else {
		RotateLeft(g);
	}
	p->color = BLACK;
	g->color = RED;
}

void ReplaceNode(struct Node* n, struct Node* child) {
	child->parent = n->parent;
	if (n == n->parent->left) {
		n->parent->left = child;
		} else {
		n->parent->right = child;
	}
}

void DeleteOneChild(struct Node* n) {
	// Precondition: n has at most one non-leaf child.
	struct Node* child = (n->right == NULL) ? n->left : n->right;
	//assert(child);

	ReplaceNode(n, child);
	if (n->color == BLACK) {
		if (child->color == RED) {
			child->color = BLACK;
			} else {
			DeleteCase1(child);
		}
	}
	//free(n);
}

void DeleteCase1(struct Node* n) {
	if (n->parent != NULL) {
		DeleteCase2(n);
	}
}

void DeleteCase2(struct Node* n) {
	struct Node* s = GetSibling(n);

	if (s->color == RED) {
		n->parent->color = RED;
		s->color = BLACK;
		if (n == n->parent->left) {
			RotateLeft(n->parent);
			} else {
			RotateRight(n->parent);
		}
	}
	DeleteCase3(n);
}

void DeleteCase3(struct Node* n) {
	struct Node* s = GetSibling(n);

	if ((n->parent->color == BLACK) && (s->color == BLACK) &&
	(s->left->color == BLACK) && (s->right->color == BLACK)) {
		s->color = RED;
		DeleteCase1(n->parent);
		} else {
		DeleteCase4(n);
	}
}

void DeleteCase4(struct Node* n) {
	struct Node* s = GetSibling(n);

	if ((n->parent->color == RED) && (s->color == BLACK) &&
	(s->left->color == BLACK) && (s->right->color == BLACK)) {
		s->color = RED;
		n->parent->color = BLACK;
		} else {
		DeleteCase5(n);
	}
}

void DeleteCase5(struct Node* n) {
	struct Node* s = GetSibling(n);

	// This if statement is trivial, due to case 2 (even though case 2 changed
	// the sibling to a sibling's child, the sibling's child can't be red, since
	// no red parent can have a red child).
	if (s->color == BLACK) {
		// The following statements just force the red to be on the left of the
		// left of the parent, or right of the right, so case six will rotate
		// correctly.
		if ((n == n->parent->left) && (s->right->color == BLACK) &&
		(s->left->color == RED)) {
			// This last test is trivial too due to cases 2-4.
			s->color = RED;
			s->left->color = BLACK;
			RotateRight(s);
		} else if ((n == n->parent->right) && (s->left->color == BLACK) &&
		(s->right->color == RED)) {
			// This last test is trivial too due to cases 2-4.
			s->color = RED;
			s->right->color = BLACK;
			RotateLeft(s);
		}
	}
	DeleteCase6(n);
}

void DeleteCase6(struct Node* n) {
	struct Node* s = GetSibling(n);

	s->color = n->parent->color;
	n->parent->color = BLACK;

	if (n == n->parent->left) {
		s->right->color = BLACK;
		RotateLeft(n->parent);
		} else {
		s->left->color = BLACK;
		RotateRight(n->parent);
	}
}

//this is my code :p

void *rb_get_data(struct rb_tree_t *tree, void *key){
	struct Node *node_curr = tree->root;
	uint8_t comparison_result;
	void *node_data;
	if (node_curr != NULL) {
		comparison_result = tree->comparator(node_curr->key, key);
	}
	while ((node_curr != NULL) && (comparison_result != EQUAL)) {
		if (tree->comparator(key, node_curr->key) == LESSER) {
			node_curr = node_curr->left;
		}
		else {
			node_curr = node_curr->right;
		}
		if (node_curr != NULL) {
			comparison_result = tree->comparator(node_curr->key, key);
		}
	}
	if (node_curr != NULL) {
		node_data = node_curr->data;
	}
	else{
		node_data = NULL;
	}
	return node_data;
}

struct Node *rb_get_node(struct rb_tree_t *tree, void *key){
	struct Node* node_curr = tree->root;
	uint8_t comparison_result;
	if (node_curr != NULL) {
		comparison_result = tree->comparator(node_curr->key, key);
	}
	while ((node_curr != NULL) && (comparison_result != EQUAL)) {
		if (tree->comparator(key, node_curr->key) == LESSER) {
			node_curr = node_curr->left;
		}
		else {
			node_curr = node_curr->right;
		}
		if (node_curr != NULL) {
			comparison_result = tree->comparator(node_curr->key, key);
		}
	}
	return node_curr;
}

void rb_create(struct rb_tree_t *tree){
	tree->is_empty = TRUE;
}

void rb_insert(struct rb_tree_t *tree, struct Node *n){
	if(tree->is_empty){
		n->left = NULL;
		n->right = NULL;
		n->parent = NULL;
		n->color = BLACK;
		
		tree->root = n;
		tree->is_empty = FALSE;
	}
	else{
		tree->root = Insert(tree, n);
	}
}

void rb_remove(struct rb_tree_t *tree, struct Node *n){
	if(n != NULL){
		if((n->parent == NULL) && (n->left == NULL) && (n->right == NULL)){
			tree->is_empty = TRUE;
		}
		DeleteOneChild(n);
	}
}

void rb_replace(struct Node *n, struct Node *child){
	ReplaceNode(n, child);	
}

uint8_t rb_comparator_uint8(void *lhs, void *rhs) {
	uint8_t lhsi = *((uint8_t *)lhs);
	uint8_t rhsi = *((uint8_t *)rhs);
	if (lhsi < rhsi) {
		return LESSER;
	}
	else if (lhsi > rhsi) {
		return GREATER;
	}
	else {
		return EQUAL;
	}
}

uint8_t rb_comparator_int(void *lhs, void *rhs) {
	int lhsi = *((int *)lhs);
	int rhsi = *((int *)rhs);
	if (lhsi < rhsi) {
		return LESSER;
	}
	else if (lhsi > rhsi) {
		return GREATER;
	}
	else {
		return EQUAL;
	}
}

uint8_t rb_comparator_string(void *lhs, void *rhs) {
	uint8_t pos = 0;
	char *str_l = (char *)lhs;
	char *str_r = (char *)rhs;
	while (str_l[pos] && str_r[pos]) {
		if (str_l[pos] < str_r[pos]) {
			return LESSER;
		}
		else if (str_l[pos] > str_r[pos]) {
			return GREATER;
		}
		pos++;
	}
	if (str_l[pos]) {
		return GREATER;
	}
	else if (str_r[pos]) {
		return LESSER;
	}
	else {
		return EQUAL;
	}
}
