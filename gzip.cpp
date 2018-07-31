/*Author: Rui Pedro Paiva
Teoria da Informa��o, LEI, 2007/2008*/

#include <cstdlib>
#include <cstring>
#include <string.h>
#include <math.h>
#include "gzip.h"
#include "helper.h"


int seq[] = {16,17,18,0,8,7,9,6,10,5,11,4,12,3,13,2,14,1,15};

//fun��o principal, a qual gere todo o processo de descompacta��o
int main(int argc, char** argv)
{
//--- Gzip file management variables
    FILE *gzFile;  //ponteiro para o ficheiro a abrir
    long fileSize;
    long origFileSize;
    int numBlocks = 0;
    gzipHeader gzh;
    unsigned char byte;  //vari�vel tempor�ria para armazenar um byte lido directamente do ficheiro
    unsigned int rb = 0;  //�ltimo byte lido (poder� ter mais que 8 bits, se tiverem sobrado alguns de leituras anteriores)
    char needBits = 0, availBits = 0;
    unsigned char HLIT, HDIST, HCLEN;



    //--- obter ficheiro a descompactar
    //char fileName[] = "FAQ.txt.gz";
    /*
    if (argc != 2)
    {
        printf("Linha de comando invalida!!!");
        return -1;
    }*/
    //char* fileName = "FAQ.txt.gz";

    //--- processar ficheiro
    gzFile = fopen("FAQ.txt.gz", "r");
    if(gzFile==NULL){
        perror("");
    }
    fseek(gzFile, 0L, SEEK_END);
    fileSize = ftell(gzFile);
    fseek(gzFile, 0L, SEEK_SET);

    //ler tamanho do ficheiro original (acrescentar: e definir Vector com s�mbolos
    origFileSize = getOrigFileSize(gzFile);


    //--- ler cabe�alho
    int erro = getHeader(gzFile, &gzh);
    if (erro != 0)
    {
        printf ("Formato invalido!!!");
        return -1;
    }

    unsigned char* outBuf;
    outBuf = new unsigned char[origFileSize + 1];
    long posBuf = 0;
    unsigned char* origBuf = outBuf;
    //--- Para todos os blocos encontrados
    char BFINAL;

    do
    {
        //--- ler o block header: primeiro byte depois do cabe�alho do ficheiro
        needBits = 3;
        if (availBits < needBits)
        {
            fread(&byte, 1, 1, gzFile);
            rb = (byte << availBits) | rb;
            availBits += 8;
        }

        //obter BFINAL
        //ver se � o �ltimo bloco
        BFINAL = rb & 0x01; //primeiro bit � o menos significativo
        printf("BFINAL = %d\n", BFINAL);
        rb = rb >> 1; //descartar o bit correspondente ao BFINAL
        availBits -=1;

        //analisar block header e ver se � huffman din�mico
        if (!isDynamicHuffman(rb))  //ignorar bloco se n�o for Huffman din�mico
            continue;
        rb = rb >> 2; //descartar os 2 bits correspondentes ao BTYPE
        availBits -= 2;

        //--- Se chegou aqui --> compactado com Huffman din�mico --> descompactar
        //**************************************************
        //****** ADICIONAR PROGRAMA... *********************
        //**************************************************
//1
        HLIT = read_n_bits(gzFile, 5, &rb, &availBits);//read Bits for HLT, HDIST,HCLEN
        HDIST= read_n_bits(gzFile, 5, &rb, &availBits);
        HCLEN = read_n_bits(gzFile, 4, &rb, &availBits);

        printf("HLIT: %d\nHDIST: %d\nHCLEN: %d\n", HLIT, HDIST, HCLEN);


        /*
         * HCLEN PORTION OF THE BLOCK
         * READS THE LENGTH OF ALL THE CODES AND CREATES THE CORRESPONDING HUFFMAN TREE
         * */
 //2
        int hclen_lens[19];

        for (int i = 0; i < HCLEN+4; ++i){
            hclen_lens[i] = read_n_bits(gzFile, 3, &rb, &availBits);
        }
        sort_arrays(seq, hclen_lens, HCLEN+4); //DA SORT AO ARRAY DE HCLEN E A SEQUENCIA PARA QUE FIQUEM EM ORDEM
//3
        char* hclen_codes[HCLEN + 4];

        for (int i = 0; i < HCLEN + 4; ++i) {
            hclen_codes[i] = new char[hclen_lens[i]];
        }

        get_huff_codes(hclen_lens, hclen_codes, HCLEN + 4);

        printf("\n\nHCLEN Codes:\n\n");
        for (int i = 0; i < HCLEN + 4; ++i) {
            printf("%d with length %d: %s\n", seq[i], hclen_lens[i], hclen_codes[i]);
        }

        HuffmanTree* hclen_tree;
        hclen_tree = createHFTree();

        add_hclen_codes_to_tree(hclen_tree,HCLEN+4,hclen_codes,hclen_lens, seq);
        /*
         * HLIT PORTION OF THE BLOCK
         * READS THE LENGTH OF ALL THE CODES AND CREATES THE CORRESPONDING HUFFMAN TREE
         * */
        int* lit_lens = get_size_codes(hclen_tree, HLIT + 257, gzFile, &availBits, &rb);

        char* lit_codes[HLIT+257];

        for (int i = 0; i < HLIT + 257; ++i) {
            lit_codes[i] = new char[20];
        }

        get_huff_codes(lit_lens, lit_codes, HLIT + 257);

        printf("\n\nHLIT Codes:\n\n");
        for (int i = 0; i < HLIT + 257; ++i) {
            printf("%d with length %d: %s\n", i, lit_lens[i], lit_codes[i]);
        }

        HuffmanTree* hlit_tree;
        hlit_tree = createHFTree();
        add_other_codes_to_tree(hlit_tree,HLIT+257+1,lit_codes,lit_lens);// +1 porque senao seria <=


        /*
         * HDIST PORTION OF THE BLOCK
         * READS THE LENGTH OF ALL THE CODES AND CREATES THE CORRESPONDING HUFFMAN TREE
         * */
        int* dist_lens = get_size_codes(hclen_tree, HDIST + 1, gzFile, &availBits, &rb);
        char* hdist_codes[HDIST+1];

        for (int i = 0; i < HDIST + 1; ++i) {
            hdist_codes[i] = new char[dist_lens[i]];
        }

        get_huff_codes(dist_lens, hdist_codes, HDIST + 1);

        printf("\n\nHDIST Codes: \n\n");
        for (int i = 0; i < HDIST + 1; ++i) {
            printf("%d with length %d: %s\n", i, dist_lens[i], hdist_codes[i]);
        }


        HuffmanTree* hdist_tree;
        hdist_tree = createHFTree();
        add_other_codes_to_tree(hdist_tree,HDIST+1,hdist_codes,dist_lens);


        /*
         * from DATA BYTES USE THE HUFFMAN TREES TO DECOMPRESS WITH LZ77 AND WRITE TO OUTBUF
         *
         * */
        memset(outBuf, 0, origFileSize);
        decode(gzFile, &rb, &availBits, hlit_tree, hdist_tree, outBuf, &posBuf);

        //--------------------------------------------------------------------------------------------
        outBuf = origBuf;
        free(hclen_tree);
        free(hlit_tree);
        free(hdist_tree);


        //actualizar n�mero de blocos analisados
        numBlocks++;
    }while(BFINAL == 0);


    //termina��es
    FILE* output = fopen(gzh.fName,"wb");
    fprintf(output,"%s",outBuf);
    fclose(output);
    free(outBuf);
    fclose(gzFile);
    printf("End: %d bloco(s) analisado(s).\n", numBlocks);


    //teste da fun��o bits2String: RETIRAR antes de criar o execut�vel final
    //char str[9];
    //bits2String(str, 0x03);
    //printf("%s\n", str);


    //RETIRAR antes de criar o execut�vel final
    //system("PAUSE");
    //return EXIT_SUCCESS;
    return 0;
}


//---------------------------------------------------------------
//L� o cabe�alho do ficheiro gzip: devolve erro (-1) se o formato for inv�lidodevolve, ou 0 se ok
int getHeader(FILE *gzFile, gzipHeader *gzh) //obt�m cabe�alho
{
    unsigned char byte;

    //Identica��o 1 e 2: valores fixos
    fread(&byte, 1, 1, gzFile);
    (*gzh).ID1 = byte;
    if ((*gzh).ID1 != 0x1f) return -1; //erro no cabe�alho

    fread(&byte, 1, 1, gzFile);
    (*gzh).ID2 = byte;
    if ((*gzh).ID2 != 0x8b) return -1; //erro no cabe�alho

    //M�todo de compress�o (deve ser 8 para denotar o deflate)
    fread(&byte, 1, 1, gzFile);
    (*gzh).CM = byte;
    if ((*gzh).CM != 0x08) return -1; //erro no cabe�alho

    //Flags
    fread(&byte, 1, 1, gzFile);
    unsigned char FLG = byte;

    //MTIME
    char lenMTIME = 4;
    fread(&byte, 1, 1, gzFile);
    (*gzh).MTIME = byte;
    for (int i = 1; i <= lenMTIME - 1; i++)
    {
        fread(&byte, 1, 1, gzFile);
        (*gzh).MTIME = (byte << 8) + (*gzh).MTIME;
    }

    //XFL (not processed...)
    fread(&byte, 1, 1, gzFile);
    (*gzh).XFL = byte;

    //OS (not processed...)
    fread(&byte, 1, 1, gzFile);
    (*gzh).OS = byte;

    //--- Check Flags
    (*gzh).FLG_FTEXT = (char)(FLG & 0x01);
    (*gzh).FLG_FHCRC = (char)((FLG & 0x02) >> 1);
    (*gzh).FLG_FEXTRA = (char)((FLG & 0x04) >> 2);
    (*gzh).FLG_FNAME = (char)((FLG & 0x08) >> 3);
    (*gzh).FLG_FCOMMENT = (char)((FLG & 0x10) >> 4);

    //FLG_EXTRA
    if ((*gzh).FLG_FEXTRA == 1)
    {
        //ler 2 bytes XLEN + XLEN bytes de extra field
        //1� byte: LSB, 2�: MSB
        char lenXLEN = 2;

        fread(&byte, 1, 1, gzFile);
        (*gzh).xlen = byte;
        fread(&byte, 1, 1, gzFile);
        (*gzh).xlen = (byte << 8) + (*gzh).xlen;

        (*gzh).extraField = new unsigned char[(*gzh).xlen];

        //ler extra field (deixado como est�, i.e., n�o processado...)
        for (int i = 0; i <= (*gzh).xlen - 1; i++)
        {
            fread(&byte, 1, 1, gzFile);
            (*gzh).extraField[i] = byte;
        }
    }
    else
    {
        (*gzh).xlen = 0;
        (*gzh).extraField = 0;
    }

    //FLG_FNAME: ler nome original
    if ((*gzh).FLG_FNAME == 1)
    {
        (*gzh).fName = new char[1024];
        unsigned int i = 0;
        do
        {
            fread(&byte, 1, 1, gzFile);
            if (i <= 1023)  //guarda no m�ximo 1024 caracteres no array
                (*gzh).fName[i] = byte;
            i++;
        }while(byte != 0);
        if (i > 1023)
            (*gzh).fName[1023] = 0;  //apesar de nome incompleto, garantir que o array termina em 0
    }
    else
        (*gzh).fName = 0;

    //FLG_FCOMMENT: ler coment�rio
    if ((*gzh).FLG_FCOMMENT == 1)
    {
        (*gzh).fComment = new char[1024];
        unsigned int i = 0;
        do
        {
            fread(&byte, 1, 1, gzFile);
            if (i <= 1023)  //guarda no m�ximo 1024 caracteres no array
                (*gzh).fComment[i] = byte;
            i++;
        }while(byte != 0);
        if (i > 1023)
            (*gzh).fComment[1023] = 0;  //apesar de coment�rio incompleto, garantir que o array termina em 0
    }
    else
        (*gzh).fComment = 0;


    //FLG_FHCRC (not processed...)
    if ((*gzh).FLG_FHCRC == 1)
    {
        (*gzh).HCRC = new unsigned char[2];
        fread(&byte, 1, 1, gzFile);
        (*gzh).HCRC[0] = byte;
        fread(&byte, 1, 1, gzFile);
        (*gzh).HCRC[1] = byte;
    }
    else
        (*gzh).HCRC = 0;

    return 0;
}


//Analisa block header e v� se � huffman din�mico
int isDynamicHuffman(unsigned char rb)
{
    unsigned char BTYPE = rb & 0x03;

    if (BTYPE == 0) //--> sem compress�o
    {
        printf("Ignorando bloco: sem compactacao!!!\n");
        return 0;
    }
    else if (BTYPE == 1)
    {
        printf("Ignorando bloco: compactado com Huffman fixo!!!\n");
        return 0;
    }
    else if (BTYPE == 3)
    {
        printf("Ignorando bloco: BTYPE = reservado!!!\n");
        return 0;
    }
    else
        return 1;
}



//Obt�m tamanho do ficheiro original
long getOrigFileSize(FILE * gzFile)
{
    //salvaguarda posi��o actual do ficheiro
    long fp = ftell(gzFile);

    //�ltimos 4 bytes = ISIZE;
    fseek(gzFile, -4, SEEK_END);

    //determina ISIZE (s� correcto se cabe em 32 bits)
    unsigned long sz = 0;
    unsigned char byte;
    fread(&byte, 1, 1, gzFile);
    sz = byte;
    for (int i = 0; i <= 2; i++)
    {
        fread(&byte, 1, 1, gzFile);
        sz = (byte << 8*(i+1)) + sz;
    }


    //restaura file pointer
    fseek(gzFile, fp, SEEK_SET);

    return sz;
}


void bits2String(char *strBits, unsigned char byte)
{
    char mask = 0x01;  //get LSbit

    strBits[8] = 0;
    for (char bit, i = 7; i >= 0; i--)
    {
        bit = byte & mask;
        strBits[i] = bit +48; //converter valor num�rico para o caracter alfanum�rico correspondente
        byte = byte >> 1;
    }
}
