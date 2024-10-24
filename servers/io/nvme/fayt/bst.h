#ifndef FAYT_BST_H_
#define FAYT_BST_H_

#define BST_GENERIC_SEARCH(ROOT, BASE, VALUE) ({ \
	typeof(ROOT) *_root = ROOT; \
	while(_root) { \
		if(_root->BASE > (VALUE)) { \
			_root = _root->left; \
		} else { \
			_root = _root->right; \
		} \
	} \
	_root \
})

#define BST_GENERIC_INSERT(TABLE_ROOT, BASE, NODE) ({ \
	__label__ out_bgi; \
	int ret = 0; \
	typeof(TABLE_ROOT) _root = TABLE_ROOT; \
	typeof(TABLE_ROOT) _parent = NULL; \
	if((NODE) == NULL) { \
		ret = -1; \
		goto out_bgi; \
	} \
	while(_root) { \
		_parent = _root; \
		if(_root->BASE > (NODE)->BASE) { \
			_root = _root->left; \
		} else { \
			_root = _root->right; \
		} \
	} \
	(NODE)->parent = _parent; \
	if(_parent == NULL) { \
		TABLE_ROOT = (NODE); \
	} else if(_parent->BASE > (NODE)->BASE) { \
		_parent->left = (NODE); \
	} else { \
		_parent->right = (NODE); \
	} \
out_bgi: \
	ret; \
})

#define BST_GENERIC_DELETE(TABLE_ROOT, BASE, NODE) ({ \
	__label__ out_bgd; \
	int ret = 0; \
	typeof(NODE) _parent = (NODE)->parent; \
	if((NODE) == NULL) { \
		ret = -1; \
		goto out_bgd; \
	} \
	if((NODE)->left == NULL && (NODE)->right == NULL) { \
		if(_parent == NULL) TABLE_ROOT = NULL; \
		else if(_parent->left == (NODE)) _parent->left = NULL; \
		else _parent->right = NULL; \
	} else if((NODE)->left && (NODE)->right == NULL) { \
		if(_parent->left == (NODE)) { \
			_parent->left = (NODE)->left; \
			_parent->left->parent = _parent; \
		} else { \
			_parent->right = (NODE)->left; \
			_parent->right->parent = _parent; \
		} \
	} else if((NODE)->right && (NODE)->left == NULL) { \
		if(_parent->left == (NODE)) { \
			_parent->left = (NODE)->right; \
			_parent->left->parent = _parent; \
		} else { \
			_parent->right = (NODE)->right; \
			_parent->right->parent = _parent; \
		} \
	} else { \
		typeof(NODE) _successor = (NODE)->right; \
		for(;_successor->left != NULL;) _successor = _successor->left; \
		if(_successor->parent->left == _successor) { \
			_successor->parent->left = _successor->right; \
			if(_successor->right) _successor->right->parent = _successor->parent; \
		} else { \
			_successor->parent->right = _successor->right; \
			if(_successor->right) _successor->right->parent = _successor->parent; \
		} \
		if(_parent->left == (NODE)) _parent->left = _successor; \
		else _parent->right = _successor; \
		_successor->left = (NODE)->left; \
		if(_successor->left) _successor->left->parent = _successor; \
		_successor->right = (NODE)->right; \
		if(_successor->right) _successor->right->parent = _successor; \
	} \
out_bgd: \
	ret; \
})

#endif
