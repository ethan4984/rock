#ifndef FAYT_RB_TREE_H_
#define FAYT_RB_TREE_H_

#include <fayt/bst.h>

#define BLACK 0
#define RED 1

#define RIGHT 0
#define LEFT 1

#define RB_META(TYPE) \
	TYPE *left; \
	TYPE *right; \
	TYPE *parent; \
	int colour;

#define RB_PARENT_NODE_PTR(ROOT, NODE) ({ \
	__label__ out_pnp; \
	typeof(NODE) *ret; \
	if((NODE)->parent == NULL) { \
		ret = &ROOT; \
		goto out_pnp; \
	} \
	if((NODE)->parent->left == (NODE)) ret = &(NODE)->parent->left; \
	else if((NODE)->parent->right == (NODE)) ret = &(NODE)->parent->right; \
	else ret = NULL; \
out_pnp: \
	ret; \
})

#define RB_GRANDPARENT_NODE_PTR(ROOT, NODE) ({ \
	typeof(NODE) *ret; \
	__label__ out_gnp; \
	typeof(NODE) parent = RB_PARENT_NODE_PTR(ROOT, NODE); \
	if(parent == NULL) goto out_gnp; \
	ret = &parent->parent; \
out_gnp: \
	ret; \
})

#define RB_UNCLE_NODE_PTR(ROOT, NODE) ({ \
	__label__ out_unp; \
	typeof(NODE) *ret = NULL; \
	typeof(NODE) *grandparent = RB_GRANDPARENT_NODE_PTR(ROOT, NODE); \
	if(grandparent == NULL) goto out_unp; \
	if((*grandparent)->left == (NODE)) ret = &(*grandparent)->right; \
	else if((*grandparent)->right == (NODE)) ret = &(*grandparent)->left; \
	else ret = NULL; \
out_unp: \
	ret; \
})

//	RIGHT ROTATION ON NODE:
//		SET THE LEFT CHILD OF NODE NODE TO BE THE ROOT OF THE SUB-TREE
//		MAKE THE RIGHT CHILD OF THE ROOT OF THE SUB-TREE EQUAL TO THE NODE
//		MAKE THE LEFT CHILD OF NODE EQUAL TO THE PREVIOUS RIGHT CHILD OF THE SUB-TREE

#define RB_ROTATE_RIGHT(ROOT, NODE) ({ \
	__label__ out_rr; \
	int ret = -1; \
	ret = 0; \
	typeof(NODE) *root_parent = RB_PARENT_NODE_PTR(ROOT, NODE); \
	if(root_parent == NULL) goto out_rr; \
	if((NODE)->left == NULL) goto out_rr; \
	*root_parent = (NODE)->left; \
	typeof(NODE) tmp = (*root_parent)->right; \
	(*root_parent)->parent = (NODE)->parent; \
	(NODE)->parent = (*root_parent); \
	if(tmp) tmp->parent = (NODE); \
	(*root_parent)->right = (NODE); \
	(NODE)->left = tmp; \
out_rr: \
	ret; \
})

//	LEFT ROTATION ON NODE:
//		SET THE RIGHT CHILD OF NODE TO BE THE ROOT OF THE SUB-TREE
//		SET THE LEFT CHILD OF THE ROOT OF THE SUB-TREE EQUAL TO NODE
//		SET THE RIGHT CHILD OF NODE EQUAL TO THE PREVIOUS LEFT CHILD OF THE ROOT OF THE SUB-TREE

#define RB_ROTATE_LEFT(ROOT, NODE) ({ \
	__label__ out_rl; \
	int ret = -1; \
	ret = 0; \
	typeof(NODE) *root_parent = RB_PARENT_NODE_PTR(ROOT, NODE); \
	if(root_parent == NULL) goto out_rl; \
	if((NODE)->right == NULL) goto out_rl; \
	*root_parent = (NODE)->right; \
	typeof(NODE) tmp = (*root_parent)->left; \
	(*root_parent)->parent = (NODE)->parent; \
	(NODE)->parent = (*root_parent); \
	if(tmp) tmp->parent = (NODE); \
	(*root_parent)->left = (NODE); \
	(NODE)->right = tmp; \
out_rl: \
	ret; \
})

#define RB_GENERIC_INSERT(ROOT, BASE, NODE) ({ \
	__label__ out_rbi; \
	int ret = BST_GENERIC_INSERT(ROOT, BASE, NODE); \
	if(ret == -1) goto out_rbi; \
	if((ROOT) == (NODE)) (ROOT)->colour = BLACK; \
	else (NODE)->colour = RED; \
	for(typeof(NODE) node = NODE; node && node->parent && node->parent->colour != BLACK;) { \
		int direction = (node->parent->right == node) ? RIGHT : LEFT; \
		typeof(NODE) grandparent = node->parent->parent; \
		if(grandparent == NULL) { break; } \
		typeof(NODE) uncle = (node->parent == grandparent->left) ? grandparent->right : grandparent->left; \
		if(uncle == NULL || uncle->colour == BLACK) { \
			if(direction == LEFT) { RB_ROTATE_LEFT(ROOT, node->parent); } \
			else if(direction == RIGHT) { RB_ROTATE_RIGHT(ROOT, grandparent); } \
			else { ret = -1; goto out_rbi; } \
			node->parent->colour = BLACK; \
			grandparent->colour = BLACK; \
		} else if(uncle && uncle->colour == RED) { \
			node->parent->colour = BLACK; \
			grandparent->colour = RED; \
			uncle->colour = BLACK; \
			continue; \
		} else { ret = -1; goto out_rbi; } \
	} \
out_rbi: \
	ret; \
})

#define RB_GENERIC_DELETE(TABLE_ROOT, BASE, NODE) ({ \
	__label__ out_rbd; \
	typeof(NODE) _sibling = (NODE)->parent ? (((NODE)->parent->left == (NODE)) ? \
		(NODE)->parent->right : (NODE)->parent->left) : NULL; \
	int ret = BST_GENERIC_DELETE(TABLE_ROOT, BASE, NODE); \
	if(ret == -1) goto out_rbd; \
	if(_sibling == NULL) goto out_rbd; \
	for(; _sibling;) { \
		if(_sibling->colour == RED) { \
			_sibling->colour = BLACK; \
			(NODE)->parent->colour = RED; \
			if((NODE)->parent->left == (NODE)) RB_ROTATE_LEFT(TABLE_ROOT, (NODE)->parent); \
			else RB_ROTATE_RIGHT(TABLE_ROOT, (NODE)->parent); \
			_sibling = (NODE)->parent->left == (NODE) ? (NODE)->parent->right : (NODE)->parent->left; \
		} else { \
			if((_sibling->left && _sibling->left->colour == RED) || \
			   (_sibling->right && _sibling->right->colour == RED)) { \
				if(_sibling->left && _sibling->left->colour == RED) { \
					if(_sibling->parent->left == _sibling) { \
						_sibling->left->colour = BLACK; \
						_sibling->colour = RED; \
						RB_ROTATE_RIGHT(TABLE_ROOT, _sibling); \
					} \
				} \
				_sibling->colour = _sibling->parent->colour; \
				_sibling->parent->colour = BLACK; \
				if(_sibling->right) _sibling->right->colour = BLACK; \
				if((NODE)->parent->left == (NODE)) RB_ROTATE_LEFT(TABLE_ROOT, (NODE)->parent); \
				else RB_ROTATE_RIGHT(TABLE_ROOT, (NODE)->parent); \
				break; \
			} else { \
				if(_sibling->parent->colour == BLACK) { \
					_sibling->colour = RED; \
					typeof(NODE) _grandparent = (NODE)->parent ? (NODE)->parent->parent : NULL; \
					_sibling = _grandparent ? ((_grandparent->left == (NODE)) ? \
						_grandparent->right : _grandparent->left) : NULL; \
				} else { \
					_sibling->colour = RED; \
					_sibling->parent->colour = BLACK; \
					break; \
				} \
			} \
		} \
	} \
out_rbd: \
	ret; \
})

#endif
