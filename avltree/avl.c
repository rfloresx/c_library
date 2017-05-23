/* Copyright (c) 2017 rfloresx
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "avl.h"
#include <stdlib.h>

struct avlnode_s {
     void               *key;     /* User-defined key */
     void               *value;   /* User-defined payload */
     struct avlnode_s   *left;    /* */
     struct avlnode_s   *right;   /* */
     int                height;   /* Node height */
};

struct avltree_s {
    struct avlnode_s *root; /* Top of the tree */
    cmp_key_cb        cmp;  /* Compare two items */
    dup_key_cb        dup;  /* Duplicate item */
    rel_key_cb        rel;  /* Release item */
    size_t            size; /* Number of items */
};


/* *******************************************
 * Private Methods:
 * ********************************************/
//avl util
static avlnode_t* avlnode_new(void);
static void avlnode_delete(avlnode_t *node, rel_key_cb rel);
static int  avlnode_height(avlnode_t *node);
static void avlnode_update_height(avlnode_t *node);

static avlnode_t* avlnode_remove_min(avlnode_t **ptr);

//avl rotations
static void avlnode_lrotation(avlnode_t **ptr);
static void avlnode_rrotation(avlnode_t **ptr);
static void avlnode_rotate(avlnode_t **ptr);

//avl internal api
static avlnode_t* _avltree_find(avltree_t *tree, avlnode_t **ptr, void *key);
static int _avltree_insert(avltree_t *tree, avlnode_t **ptr, void *key, void *value, void **old_out, int replace);
static int _avltree_remove(avltree_t *tree, avlnode_t **ptr, void *key, void **old_out);

static int   def_cmp(const void*p1, const void*p2) {
    return (int)((uint64_t)p1 - (uint64_t)p2);
} /* Default Compare  */
static void *def_dup(void*p) {return p;}                    /* Default Copy */
static void  def_rel(void*p) {}                             /* Default Release */

/* ***********
 * Public Methods:
 * ***********/
avltree_t  *avltree_new(cmp_key_cb cmp, dup_key_cb dup, rel_key_cb rel) {
    avltree_t *tree = (avltree_t *)malloc(sizeof(avltree_t));
    if (tree == NULL) {
        return NULL;
    }
    tree->root = NULL;
    tree->cmp = cmp ? cmp : def_cmp;
    tree->dup = dup ? dup : def_dup;
    tree->rel = rel ? rel : def_rel;
    tree->size = 0;
    return tree;
}

void avltree_delete(avltree_t *tree) {
    if (tree) {
        avlnode_delete(tree->root, tree->rel);
        free(tree);
    }
}

void *avltree_find(avltree_t *tree, void*key) {
    avlnode_t *node = _avltree_find(tree, &tree->root, key);
    if (node == NULL)
    {
        return NULL;
    }
    return node->value;
}

void *avltree_findPtr(avltree_t *tree, void*key) {
    avlnode_t *node = _avltree_find(tree, &tree->root, key);
    if (node == NULL)
    {
        return NULL;
    }
    return &node->value;
}

int avltree_haskey(avltree_t *tree, void*key) {
    avlnode_t *node = _avltree_find(tree, &tree->root, key);
    return node != NULL;
}

int avltree_insert(avltree_t *tree, void*key, void*data, void**old_out, int overwrite) {
    void *out;
    int result =  _avltree_insert(tree, &tree->root, key, data, &out, overwrite);
    if (old_out) {
        *old_out = out;
    }
    return result;
}

int avltree_erase(avltree_t *tree, void*key, void**old_out) {
    void *out = NULL;
    int result =  _avltree_remove(tree, &tree->root, key, &out);
    if (old_out) {
        *old_out = out;
    }
    return result;
}

size_t avltree_size(avltree_t *tree) {
    return tree->size;
}


/* ***********
 * Private methods
 * ***********/

static avlnode_t* avlnode_new() {
    avlnode_t *node = (avlnode_t *)malloc(sizeof(avlnode_t));
    if (node == NULL) {
        return NULL;
    }
    node->key = NULL;
    node->value = NULL;
    node->left = NULL;
    node->right = NULL;
    node->height = 0;
    return node;
}

static void avlnode_delete(avlnode_t *node, rel_key_cb rel){
    if (node != NULL) {
        avlnode_delete(node->left, rel);
        avlnode_delete(node->right, rel);
        rel(node->key);
        free(node);
    }
}

static int avlnode_height(avlnode_t *node) {
    if (node == NULL) {
        return -1;
    }
    return node->height;
}

static void avlnode_update_height(avlnode_t *node) {
    int hleft = avlnode_height(node->left);
    int hright = avlnode_height(node->right);
    if (hleft > hright) {
        node->height = hleft + 1;
    } else {
        node->height = hright + 1;
    }
}

static avlnode_t* avlnode_remove_min(avlnode_t **ptr) {
    avlnode_t *node = *ptr;
    if (node == NULL) {
        return NULL;
    }
    if (node->left == NULL) {
        *ptr = node->right;
        node->right = NULL;
        return node;
    }
    avlnode_t *ret = avlnode_remove_min(&node->left);
    avlnode_update_height(node);
    avlnode_rotate(ptr);
    return ret;
}

//rotations
static void avlnode_lrotation(avlnode_t **ptr) {
    avlnode_t *node = *ptr;
    if (node == NULL || node->right == NULL) {
        return;
    }
    avlnode_t *a = node;
    avlnode_t *b = node->right;
    a->right = b->left;
    b->left = a;
    *ptr = b;

    a->height = (a->height - 2);
    if (a->height < 0) {
        a->height = 0;
    }
}

static void avlnode_rrotation(avlnode_t **ptr) {
    avlnode_t *node = *ptr;
    if (node == NULL || node->left == NULL) {
        return;
    }
    avlnode_t *a = node;
    avlnode_t *b = node->left;
    a->left = b->right;
    b->right = a;
    *ptr = b;
    a->height = (a->height - 2);
    if (a->height < 0) {
        a->height = 0;
    }
}

static void avlnode_rotate(avlnode_t **ptr) {
    if (*ptr == NULL) {
        return;
    }
    avlnode_t *node = *ptr;

    int hleft = avlnode_height(node->left);
    int hright = avlnode_height(node->right);

    if (hleft > (hright + 1)) {
        avlnode_t *left = node->left;
        hleft = avlnode_height(left->left);
        hright = avlnode_height(left->right);
        if (hright > (hleft + 1)) {
            avlnode_lrotation(&node->left);
        }
        avlnode_rrotation(ptr);
    } else if (hright > (hleft + 1)) {
        avlnode_t *right = node->right;
        hleft = avlnode_height(right->left);
        hright = avlnode_height(right->right);
        if (hright > (hleft + 1)) {
            avlnode_rrotation(&node->right);
        }
        avlnode_lrotation(ptr);
    }
}


//internal api
static int _avltree_insert(avltree_t *tree, avlnode_t **ptr, void *key, void *value, void **old_out, int replace) {
    int result = 0;
    avlnode_t *node = *ptr;
    if (node == NULL) {
        *old_out = NULL;
        node = avlnode_new();
        node->key = tree->dup(key);
        node->value = value;
        *ptr = node;
    } else {
        int cmp;
        if ((cmp = tree->cmp(node->key, key))) {
            if (cmp < 0) {
                result = _avltree_insert(tree, &node->left, key, value, old_out, replace);
            } else {
                result = _avltree_insert(tree, &node->right, key, value, old_out, replace);
            }
            if (*old_out == NULL) {
                avlnode_update_height(node);
                avlnode_rotate(ptr);
            }
        } else {
            *old_out = node->value;
            if (replace) {
                node->value = value;
                result = 1;
            }
        }
    }
    return result;
}

static int _avltree_remove(avltree_t *tree, avlnode_t **ptr, void *key, void **old_out) {
    int result = 0;
    avlnode_t *node = *ptr;
    if (node != NULL) {
        int cmp;
        if ((cmp = tree->cmp(node->key, key)))
        {
            if (cmp < 0) {
                result = _avltree_remove(tree, &node->left, key, old_out);
            } else {
                result = _avltree_remove(tree, &node->right, key, old_out);
            }
        } else {
            tree->rel(node->key);
            *old_out = node->value;
            if (node->left == NULL || node->right == NULL) {
                avlnode_t *temp = NULL;
                if (node->left != NULL) {
                    temp = node->left;
                } else {
                    temp = node->right;
                }
                if (temp == NULL) {
                    temp = *ptr;
                    *ptr = NULL;
                } else {
                    **ptr = *temp;
                }
                free(temp);
            } else {
                avlnode_t * temp = avlnode_remove_min(&node->right);
                node->key = temp->key;
                node->value = temp->value;
                free(temp);
            }
            result = 1;
        }

        if (*old_out != NULL) {
            avlnode_update_height(node);
            avlnode_rotate(ptr);
        }
    }
    return result;
}

static avlnode_t* _avltree_find(avltree_t *tree, avlnode_t **ptr, void *key) {

    avlnode_t *node = *ptr;
    if (node == NULL) {
        return NULL;
    }
    int cmp = tree->cmp(node->key, key);
    if (cmp < 0) {
        return _avltree_find(tree, &node->left, key);
    } else if (cmp > 1) {
        return _avltree_find(tree, &node->right, key);
    }
    return node;
}
