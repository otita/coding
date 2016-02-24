//
//  HuffmanCoder.cpp
//
//  Created by otita on 2016/02/22.
//
/*
The MIT License (MIT)

Copyright (c) 2016 otita.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#include <queue>

#include "HuffmanCoder.h"

using namespace std;

namespace otita {

namespace coding {

class HuffmanTreeNode;
class CompareNodePtr;

class HuffmanTree {
public:
  HuffmanTree(uint64_t symbols, uint64_t *frequency);
  virtual ~HuffmanTree();
private:
  uint64_t _symbols;
  HuffmanTreeNode *_root;
  HuffmanTreeNode **_leaves;
  friend class HuffmanCoder;
};

class HuffmanTreeNode {
public:
  HuffmanTreeNode(uint64_t symbol, uint64_t frequency);
  HuffmanTreeNode(HuffmanTreeNode *a, HuffmanTreeNode *b);
  HuffmanTreeNode(const HuffmanTreeNode &other);
  HuffmanTreeNode(HuffmanTreeNode &&ohter) noexcept;
  HuffmanTreeNode &operator =(const HuffmanTreeNode &other);
  HuffmanTreeNode &operator =(HuffmanTreeNode &&other);
  virtual ~HuffmanTreeNode();
private:
  uint64_t _symbol;
  uint64_t _frequency;
  HuffmanTreeNode *_parent;
  HuffmanTreeNode *_left;
  HuffmanTreeNode *_right;

  friend class CompareNodePtr;
  friend class HuffmanCoder;
};


// implementation of HuffmanCoder
HuffmanCoder::HuffmanCoder(uint64_t symbols, uint64_t *frequency) {
  _symbols = symbols;
  _tree = new HuffmanTree(symbols, frequency);
}

void HuffmanCoder::encode(uint64_t symbol, uint64_t **code_ptr, uint64_t *len_ptr) {
  if (!len_ptr || symbol>=_symbols) {
    return;
  }
  *len_ptr = 0;
  HuffmanTreeNode *leaf = _tree->_leaves[symbol];
  HuffmanTreeNode *node = leaf;
  while (node->_parent) {
    (*len_ptr)++;
    node = node->_parent;
  }

  if (!code_ptr) {
    return;
  }

  size_t size = (*len_ptr + 64 - 1) / 64;
  *code_ptr = new uint64_t[size];
  for (size_t i = 0; i < size; i++) {
    (*code_ptr)[i] = 0;
  }

  node = leaf;
  uint64_t pos = *len_ptr - 1;
  while (node->_parent) {
    HuffmanTreeNode *child = node;
    node = node->_parent;
    uint64_t idx = pos / 64;
    uint64_t shift = pos % 64;
    if (child == node->_right) {
      (*code_ptr)[idx] |= uint64_t(1) << shift;
    }
    pos--;
  }
}

void HuffmanCoder::encode(uint64_t *symbols, uint64_t len, uint64_t **codes_ptr, uint64_t *len_ptr) {
  if (!len_ptr) {
    return;
  }
  *len_ptr = 0;
  for (uint64_t i = 0; i < len; i++) {
    uint64_t code_len;
    encode(symbols[i], nullptr, &code_len);
    *len_ptr += code_len;
  }

  if (!codes_ptr) {
    return;
  }

  size_t size = (*len_ptr + 64 - 1) / 64;
  *codes_ptr = new uint64_t[size];
  for (size_t i = 0; i < size; i++) {
    (*codes_ptr)[i] = 0;
  }

  uint64_t pos = 0;
  for (uint64_t i = 0; i < len; i++) {
    uint64_t *code_ptr;
    uint64_t code_len;
    encode(symbols[i], &code_ptr, &code_len);

    size_t code_size = (code_len + 64 - 1) / 64;
    uint64_t idx = pos / 64;
    uint64_t shift = pos % 64;
    for (size_t i = 0; i < code_size; i++) {
      (*codes_ptr)[idx] |= code_ptr[i] << shift;
      if (shift) {
        (*codes_ptr)[idx + 1] = code_ptr[i] >> (64 - shift);
      }
      idx++;
    }

    pos += code_len;
    delete [] code_ptr;
  }
}

void HuffmanCoder::decode(uint64_t *code, uint64_t len, uint64_t *symbol_ptr) {
  uint64_t pos = 0;
  _decode(code, &pos, symbol_ptr);
}

void HuffmanCoder::decode(uint64_t *codes, uint64_t len, uint64_t **symbols_ptr, uint64_t *len_ptr) {
  if (!len_ptr) {
    return;
  }
  *len_ptr = 0;
  uint64_t pos = 0;
  while (pos < len) {
    _decode(codes, &pos, nullptr);
    (*len_ptr)++;
  }
  if (!symbols_ptr) {
    return;
  }

  *symbols_ptr = new uint64_t[*len_ptr];
  pos = 0;
  uint64_t idx = 0;
  while (pos < len) {
    _decode(codes, &pos, (*symbols_ptr + idx++));
  }
}

HuffmanCoder::~HuffmanCoder() {
  delete _tree;
}

void HuffmanCoder::_decode(uint64_t *codes, uint64_t *pos_ptr, uint64_t *symbol_ptr) {
  uint64_t &pos = *pos_ptr;
  HuffmanTreeNode *node = _tree->_root;
  while (node->_left && node->_right) {
    uint64_t idx = pos / 64;
    uint64_t shift = pos % 64;
    bool bit = codes[idx] & (uint64_t(1) << shift);
    if (bit) {
      node = node->_right;
    }
    else {
      node = node->_left;
    }
    pos++;
  }
  if (symbol_ptr) {
     *symbol_ptr = node->_symbol;
  }
}


// implementation of HuffmanTree
class CompareNodePtr {
public:
  bool operator() (const HuffmanTreeNode *a, const HuffmanTreeNode *b) {
    if (a->_frequency == b->_frequency) {
      return a->_symbol > b->_symbol;
    }
    return a->_frequency > b->_frequency;
  }
};

HuffmanTree::HuffmanTree(uint64_t symbols, uint64_t *frequency) {
  using symbol_priority_queue = priority_queue<HuffmanTreeNode *, vector<HuffmanTreeNode *>, CompareNodePtr>;

  _leaves = new HuffmanTreeNode *[symbols];
  symbol_priority_queue p_queue;
  for (uint64_t c = 0; c < symbols; c++) {
    HuffmanTreeNode *node = nullptr;
    if (frequency[c]) {
      node = new HuffmanTreeNode(c, frequency[c]);
    }
    _leaves[c] = node;
    if (node) {
      p_queue.push(node); 
    }
  }

  while (p_queue.size()>1) {
    HuffmanTreeNode *a = p_queue.top();
    p_queue.pop();
    HuffmanTreeNode *b = p_queue.top();
    p_queue.pop();
    p_queue.push(new HuffmanTreeNode(a, b));
  }
  _root = p_queue.top();
}

HuffmanTree::~HuffmanTree() {
  delete _root;
  delete [] _leaves;
}

HuffmanTreeNode::HuffmanTreeNode(uint64_t symbol, uint64_t frequency) {
  _symbol = symbol;
  _frequency = frequency;
  _parent = nullptr;
  _left = nullptr;
  _right = nullptr;
}

HuffmanTreeNode::HuffmanTreeNode(HuffmanTreeNode *a, HuffmanTreeNode *b) {
  HuffmanTreeNode *left, *right;
  if (CompareNodePtr()(a, b)) {
    left = a;
    right = b;
  }
  else {
    left = b;
    right = a;
  }
  _symbol = min(a->_symbol, b->_symbol);
  _frequency = a->_frequency + b->_frequency;
  _parent = nullptr;
  _left = left;
  _right = right;
  
  left->_parent = this;
  right->_parent = this;
}

HuffmanTreeNode::HuffmanTreeNode(const HuffmanTreeNode &other) {
  _symbol = other._symbol;
  _frequency = other._frequency;

  if (other._left) {
    _left = new HuffmanTreeNode(*other._left);
  }
  else {
    _left = nullptr;
  }
  if (other._right) {
    _right = new HuffmanTreeNode(*other._right);
  }
  else {
    _right = nullptr;
  }
  if (other._parent) {
    _parent = new HuffmanTreeNode(*other._parent);
  }
  else {
    _parent = nullptr;
  }
}

HuffmanTreeNode::HuffmanTreeNode(HuffmanTreeNode &&other) noexcept {
  _symbol = other._symbol;
  _frequency = other._frequency;
  _left = other._left;
  _right = other._right;
  _parent = other._parent;
  other._left = nullptr;
  other._right = nullptr;
  other._parent = nullptr;
}

HuffmanTreeNode &HuffmanTreeNode::operator =(const HuffmanTreeNode &other) {
  _symbol = other._symbol;
  _frequency = other._frequency;
  
  if (_left) {
    delete _left;
  }
  if (_right) {
    delete _right;
  }
  if (_parent) {
    delete _parent;
  }
  
  if (other._left) {
    _left = new HuffmanTreeNode(*other._left);
  }
  else {
    _left = nullptr;
  }
  if (other._right) {
    _right = new HuffmanTreeNode(*other._right);
  }
  else {
    _right = nullptr;
  }
  if (other._parent) {
    _parent = new HuffmanTreeNode(*other._parent);
  }
  else {
    _parent = nullptr;
  }
  
  return *this;
}

HuffmanTreeNode &HuffmanTreeNode::operator =(HuffmanTreeNode &&other) {
  _symbol = other._symbol;
  _frequency = other._frequency;
  
  if (_left) {
    delete _left;
  }
  if (_right) {
    delete _right;
  }
  if (other._left) {
    _left = other._left;
    other._left = nullptr;
  }
  else {
    _left = nullptr;
  }
  if (other._right) {
    _right = other._right;
    other._right = nullptr;
  }
  else {
    _right = nullptr;
  }
  if (other._parent) {
    _parent = other._parent;
    other._parent = nullptr;
  }
  else {
    _parent = nullptr;
  }
  
  return *this;
}

HuffmanTreeNode::~HuffmanTreeNode() {
  if (_left) {
    delete _left;
  }
  if (_right) {
    delete _right;
  }
}

} // coding

} // otita