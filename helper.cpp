//
// Created by Francisco Ferreira on 12/12/2017.
//

#include "helper.h"

int read_n_bits(FILE* gzFILE, int needBits, unsigned int* rb, char* availBits){
    int aux;
    unsigned char byte;

    while(*availBits < needBits){
        fread(&byte, 1, 1, gzFILE);
        *rb = (byte << *availBits) | *rb;
        *availBits += 8;
    }
    aux = *rb & ((1 << needBits) - 1);
    *rb >>= needBits;
    *availBits -= needBits;
    return aux;
}

void sort_arrays(int* seq, int* size, int len) {
    int aux;
    for (int i = 0; i < len; ++i) {
        for (int j = 0; j < len-1; ++j){
            if (size[j] > size[j+1])
            {
                aux = size[j+1];
                size[j+1] = size[j];
                size[j] = aux;
                aux = seq[j+1];
                seq[j+1] = seq[j];
                seq[j] = aux;
            }
            else if ((size[j] == size[j+1]) && (seq[j] > seq[j+1])){
                aux = seq[j+1];
                seq[j+1] = seq[j];
                seq[j] = aux;
            }

        }
    }
} //DA SORT AO ARRAY DE HCLEN E A SEQUENCIA PARA QUE FIQUEM EM ORDEM

//CODIGO DADO PELO PROFESSOR NO ENUNCIADO/DOCUMENTAÇAO
void get_huff_codes(int *sizes, char **strings, int size) {
    int max = 0;
    int codes[size];
    //calcular o comp tamanho maximo
    for(int i=0; i<size; i++) {
        if(sizes[i]>max)
            max=sizes[i];
    }
    //calcular o numero de codigos em cada comprimento
    int frequencies[max+1];
    for (int i = 0; i < max+1; ++i) {
        frequencies[i] = 0;
    }
    for(int j=0; j<size; j++) {
        frequencies[sizes[j]]++;
    }

    //calcula o codigo mais pequeno para cada tamanho de codigo
    int code=0;
    int next_code[max+1];
    for (int i = 0; i < max+1; ++i) {
        next_code[i] = 0;
    }
    for(int bits=1; bits<=max; bits++){
        code = (code+frequencies[bits-1]) << 1;
        next_code[bits]= code;
    }
    //atribuir codigos a todos os tamanhos
    for(int n=0; n<size; n++) {
        int len = sizes[n];
        if(len != 0)
        {
            *(codes+n) = next_code[len];
            next_code[len]++;
        }
        *(strings+n) = get_bin_code(codes[n],sizes[n]);
    }
}

int* get_size_codes(HuffmanTree* tree, int nOfCodes, FILE* gzFile, char* availBits, unsigned int* rb){
    int* codes = new int[nOfCodes];
    int i = 0, repetitions, pos = 0;

    for(int k = 0; k<nOfCodes;k++){
        codes[k] = 0;
    }
    while(i < nOfCodes){
        while((pos = nextNode(tree,(read_n_bits(gzFile, 1, rb,availBits) + 48))) == -2){
            //NAO FAZ NADA INTENCIONALMENTE
            //APENAS SERVE PARA PESQUISAR NA ARVORE;
        };
        resetCurNode(tree);
        if(pos == 16) {
            repetitions = read_n_bits(gzFile, 2, rb, availBits) + 3;
            for(int k = 0; k < repetitions; k++) {
                codes[i+k] = codes[i - 1];
            }
            i += repetitions;
        }
        else if(pos == 17) {
            repetitions = read_n_bits(gzFile, 3, rb, availBits) + 3;
            for(int k = 0; k < repetitions; k++) {
                codes[i+k] = 0;
            }
            i += repetitions;
        }
        else if (pos == 18) {
            repetitions = read_n_bits(gzFile, 7, rb, availBits) + 11;
            for(int k = 0; k < repetitions; k++) {
                codes[i+k] = 0;
            }
            i += repetitions;
        }
        else if(pos >= 0 && pos <= 15) {
             codes[i] = pos;
            i++;
        }
        else{
            codes[i] = 0;
            i++;
        }
    }
    return codes;
}

void add_hclen_codes_to_tree(HuffmanTree* tree,int n_codes,char** codes,int* sizes,int* seq){
    for(int i=0; i < n_codes; i++) {
        if(sizes[i] != 0) {
            addNode(tree, codes[i], seq[i], 0);
        }
    }
}

void add_other_codes_to_tree(HuffmanTree* tree,int n_codes,char** codes,int* sizes){
    for(int i=0; i< n_codes; i++) {
        if (sizes[i] != 0) {
            addNode(tree, codes[i], i, 0);
        }
    }
}

char* get_bin_code(int n, int size){
    char* string = new char[size+1];
    for(int i = 0;i<size;i++){
        string[size-i-1] = (n%2)+48;
        n = n>>1;
    }
    string[size]='\0';
    return string;
}



/*
 * IMPLEMENTS LZ77 TO DECODE
 * */
void decode(FILE* gzFile, unsigned int* rb, char* availBits, HuffmanTree* hlit_tree, HuffmanTree* hdist_tree, unsigned char* buf, long* posBuf) {

    int array_lengths[29] = {3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 15, 17, 19, 23, 27, 31, 35, 43, 51, 59, 67, 83, 99, 115, 131, 163, 195, 227, 258};

    int array_dist[30] = {1, 2, 3, 4, 5, 7, 9, 13, 17, 25, 33, 49, 65, 97, 129, 193, 257, 385, 513, 769, 1025, 1537, 2049, 3073, 4097, 6145, 8193, 12289, 16385, 24577};

    int literal, len;
    unsigned char* temp;
    buf += (*posBuf);

    while(true){
        while((literal = nextNode(hlit_tree,(read_n_bits(gzFile,1,rb,availBits)+48))) == -2);
        resetCurNode(hlit_tree);

        if (literal == 256) {
            printf("%s\n", buf);
            break;
        }
        else if (literal <= 255) {
            *(buf++) = literal;
        }
        else {
            int bits, dist;
            if (literal < 265) {
                len = literal - 254;
            }
            else if (literal == 285) {
                len = 258;
            }
            else {
                bits = ((literal - 257) / 4) - 1;
                len = array_lengths[literal-257] + read_n_bits(gzFile, bits, rb, availBits);
            }
            int distance_index;
            while((distance_index=nextNode(hdist_tree,(read_n_bits(gzFile,1,rb,availBits)+48)))==-2);
            resetCurNode(hdist_tree);

            if (distance_index < 4) {
                dist = array_dist[distance_index];
            }
            else {
                bits = (distance_index  / 2) - 1;
                dist = array_dist[distance_index] + read_n_bits(gzFile, bits, rb, availBits);
            }
            temp = buf - dist;
            for (int i = 0; i < len; i++) {
                *(buf++) = *(temp++);
            }
        }
    }
}
