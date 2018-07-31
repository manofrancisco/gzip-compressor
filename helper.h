

#ifndef TI_TP2_HELPER_H
#define TI_TP2_HELPER_H

#include "huffman.h"

void decode(FILE* gzFile, unsigned int* rb, char* availBits, HuffmanTree* hlit_tree, HuffmanTree* hdist_tree, unsigned char* buf, long* posBuf);

char* get_bin_code(int n, int tam);

int* get_size_codes(HuffmanTree* tree, int nOfCodes, FILE* gzFile, char* availBits, unsigned int* rb);

void get_huff_codes(int *tamanhos, char **strings, int size);

void sort_arrays(int* seq, int* size, int len);

int read_n_bits(FILE* gzFILE, int needBits, unsigned int* rb, char* availBits);

void add_hclen_codes_to_tree(HuffmanTree* tree,int n_codes,char** codes,int* sizes,int* seq);

void add_other_codes_to_tree(HuffmanTree* tree,int n_codes,char** codes,int* sizes);

#endif //TI_TP2_HELPER_H
