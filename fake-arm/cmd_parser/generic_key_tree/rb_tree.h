#ifndef _RB_TREE_H_
#define _RB_TREE_H_

#include <inttypes.h>

#define RB_KEY_TYPE uint8_t
#define RB_NODE struct rb_node_t
//#define RB_ROOT struct rb_node_t *
#define RB_TREE struct rb_tree_t

// I copied this code from Wikipedia, and made some minor changes.


struct rb_node_t;

struct rb_node_t{
	struct rb_node_t* parent;
	struct rb_node_t* left;
	struct rb_node_t* right;
	uint8_t color;
	void *key;
	void *data;
};

//my code

struct rb_tree_t{
	uint8_t (*comparator)(void *, void *);  //returns true if second argument is greater than first argument
	struct rb_node_t *root;
	uint8_t is_empty;
};

void *rb_get_data(struct rb_tree_t *tree, void *key);
struct rb_node_t *rb_get_node(struct rb_tree_t *tree, void *key); 

void rb_create(struct rb_tree_t *tree);
void rb_insert(struct rb_tree_t *tree, struct rb_node_t *n);
void rb_remove(struct rb_tree_t *tree, struct rb_node_t *n);
void rb_replace(struct rb_node_t *n, struct rb_node_t *child);

uint8_t rb_comparator_uint8(void *lhs, void *rhs);
uint8_t rb_comparator_int(void *lhs, void *rhs);
uint8_t rb_comparator_string(void *lhs, void *rhs);

#endif
