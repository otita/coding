//
//  HuffmanCoder.h
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

#ifndef _HUFFMAN_CODER_H_
#define _HUFFMAN_CODER_H_

#include <cstdint>

namespace otita {

namespace coding {

class HuffmanTree;

class HuffmanCoder {
public:
  // len(frequency) = symbols
  HuffmanCoder(uint64_t symbols, uint64_t *frequency);
  HuffmanCoder(uint64_t *bp, uint64_t len);
  // encode one symbol.
  void encode(uint64_t symbol, uint64_t **code_ptr, uint64_t *len_ptr);
  // encode symbols.
  void encode(uint64_t *symbols, uint64_t len, uint64_t **codes_ptr, uint64_t *len_ptr);
  // decode one codeword.
  void decode(uint64_t *code, uint64_t len, uint64_t *symbol_ptr);
  // decode codewords.
  void decode(uint64_t *codes, uint64_t len, uint64_t **symbols_ptr, uint64_t *len_ptr);

  // export flags
  void usedSymbols(uint64_t **flags_ptr, uint64_t *len_ptr);
  // export Huffman tree
  void huffmanTree2Bits(uint64_t **bits_ptr, uint64_t *len_ptr);
  void huffmanTree2BP(uint64_t **bp_ptr, uint64_t *len_ptr);
  virtual ~HuffmanCoder();
private:
  uint64_t _symbols;
  HuffmanTree *_tree;
  void _decode(uint64_t *codes, uint64_t *pos_ptr, uint64_t *symbol_ptr);
};

} // coding

} // otita

#endif  // _HUFFMAN_CODER_H_
