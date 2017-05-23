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
 
#ifndef __RFX_AVL_H__
#define __RFX_AVL_H__
#include <ctype.h>
#include <inttypes.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* *******************************************
* Public avltree API:
* ********************************************/
typedef struct avltree_s avltree_t;
typedef struct avlnode_s avlnode_t;
typedef struct avltrav_s avltrav_t;

typedef int   (*cmp_key_cb) (const void *p1, const void *p2);
typedef void *(*dup_key_cb) (void*p);
typedef void  (*rel_key_cb) (void*p);

avltree_t  *avltree_new(cmp_key_cb cmp, dup_key_cb dup, rel_key_cb rel);
void        avltree_delete(avltree_t *tree);
void       *avltree_find(avltree_t *tree, void*key);
void       *avltree_findPtr(avltree_t *tree, void*key);
int         avltree_haskey(avltree_t *tree, void*key);
int         avltree_insert(avltree_t *tree, void*key, void*data, void**old_out, int overwrite);
int         avltree_erase(avltree_t *tree, void*key, void**out);
size_t      avltree_size(avltree_t *tree);

#ifdef __cplusplus
}
#endif

#endif
