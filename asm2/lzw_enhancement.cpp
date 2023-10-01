/*
* CSCI3280 Introduction to Multimedia Systems *
* --- Declaration --- *
* I declare that the assignment here submitted is original except for source
* material explicitly acknowledged. I also acknowledge that I am aware of
* University policy and regulations on honesty in academic work, and of the
* disciplinary guidelines and procedures applicable to breaches of such policy
* and regulations, as contained in the website
* http://www.cuhk.edu.hk/policy/academichonesty/ *
* Assignment 2
* Name :Ng Ka Wah
* Student ID :1155162668
* Email Addr :1155162668@link.cuhk.edu.hk
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>
#include <vector> // remove this if you are writing a C program.
#include <map>
#include <iostream>
#define CODE_SIZE  12 // you can use CODE_SIZE=16 if you want your compressed files to be 
// human-readable during debugging (it is required to be 12 for submission)
#define TRUE 1
#define FALSE 0


/* function prototypes */
unsigned int read_code(FILE*, unsigned int);
void write_code(FILE*, unsigned int, unsigned int);
void writefileheader(FILE*, char**, int);
void readfileheader(FILE*, char**, int*);
void compress(FILE*, FILE*);
void decompress(FILE*, FILE*);
//added
void insert(struct entry*, unsigned char, int);
void dict(int);
void dedict(struct entry*, unsigned int, int);

struct entry {

    struct entry* temp[256];
    unsigned char* ins;
    int num, loc, len;
    // unsigned char cha;
};

struct entry ro;
struct entry dictionary[4095];
unsigned int dictIndex;
unsigned char cha;

void dict(int cho) {
    if (cho == 1) {
        dictIndex = 0;
        ro.ins = 0;
        for (int i = 0; i < 256; i++) {
            insert(&ro, (unsigned char)i, TRUE);
        }
    }
    else {
        for (int i = 0; i < dictIndex; i++) {
            memset(dictionary[i].temp, 0, sizeof(dictionary[i].temp));
        }
        dictIndex = 256;
    }

}



void insert(struct entry* parent, unsigned char symbol, int shouldAllocate) {
    struct entry* newpo = dictionary + dictIndex;
    memset(newpo->temp, 0, sizeof(unsigned char) * 256);

    if (shouldAllocate) {
        newpo->len = parent->len + 1;
        newpo->ins = (unsigned char*)malloc(newpo->len * sizeof(unsigned char));
        memcpy(newpo->ins, parent->ins, parent->len);
        newpo->ins[parent->len] = symbol;
        memset(newpo->ins + parent->len + 1, 0, 1);
    }

    newpo->loc = dictIndex++;
    parent->temp[(int)symbol] = newpo;
}

void dedict(struct entry* n, unsigned int c, int co) {
    char ch;
    struct entry* ne;
    ne = (dictionary + c);
    if (co == TRUE)
        ch = n->ins[0];
    else
        ch = ne->ins[0];
    insert(ne, ch, TRUE);
}






int main(int argc, char** argv)
{
    int printusage = 0;
    int	no_of_file;
    char** input_file_names;
    char* output_file_names;
    FILE* lzw_file;

    dict(TRUE);
    if (argc >= 3)
    {
        if (strcmp(argv[1], "-c") == 0)
        {
            /* compression */
            lzw_file = fopen(argv[2], "wb");

            /* write the file header */
            input_file_names = argv + 3;
            no_of_file = argc - 3;
            writefileheader(lzw_file, input_file_names, no_of_file);

            /* ADD CODES HERE */
            for (int i = 0; i < no_of_file; i++) {//loop all file
                FILE* fp;
                fp = fopen(input_file_names[i], "rb");
                if (!fp) {
                    printf("ERROR: File:%s not exist!\n", input_file_names[i]);
                    return 0;
                }
                compress(fp, lzw_file);
                fclose(fp);
            }

            write_code(lzw_file, 0, 8);


            fclose(lzw_file);
        }
        else
            if (strcmp(argv[1], "-d") == 0)
            {
                /* decompress */
                lzw_file = fopen(argv[2], "rb");

                /* read the file header */
                no_of_file = 0;
                readfileheader(lzw_file, &output_file_names, &no_of_file);

                /* ADD CODES HERE */
                char* token;//char *strtok(char *str, const char *delim)
                char d[] = "\n";
                token = strtok(output_file_names, d);
                for (int i = 0; i < no_of_file; i++) {
                    FILE* fp;
                    fp = fopen(token, "wb");
                    decompress(lzw_file, fp);

                    token = strtok(NULL, d);
                    fclose(fp);
                }

                fclose(lzw_file);
                free(output_file_names);
            }
            else
                printusage = 1;
    }
    else
        printusage = 1;

    if (printusage)
        printf("Usage: %s -<c/d> <lzw filename> <list of files>\n", argv[0]);

    return 0;
}

/*****************************************************************
 *
 * writefileheader() -  write the lzw file header to support multiple files
 * arguments:
 * 	lzw_file - the output file during compression
 * 	input_file_names - array of char arrays, containing the input filenames
 *  no_of_files - number of files in the input
 *
 ****************************************************************/
void writefileheader(FILE* lzw_file, char** input_file_names, int no_of_files)
{
    int i;
    /* write the file header */
    for (i = 0; i < no_of_files; i++)
    {
        fprintf(lzw_file, "%s\n", input_file_names[i]);

    }
    fputc('\n', lzw_file);

}

/*****************************************************************
 *
 * readfileheader() - read the fileheader from the lzw file
 * arguments:
 * 	lzw_file - the input file during decompression
 * 	output_file_names - a char array passed by pointer, will be
 * 		filled with chars containing all file names (you need to
 *  	split the filenames manually by "\n" yourself)
 * 	no_of_files - number of files in the file header, passed by
 * 		pointer
 * behaviour: this function leave the file pointer at the first code of
 * 		the code sequence. You can move the file pointer to the start
 * 		of code sequence using this function if you messed up with the
 * 		file pointer location.
 *
 ****************************************************************/
void readfileheader(FILE* lzw_file, char** output_filenames, int* no_of_files)
{
    int noofchar;
    char c, lastc;

    noofchar = 0;
    lastc = 0;
    *no_of_files = 0;
    /* find where is the end of double newline */
    while ((c = fgetc(lzw_file)) != EOF)
    {
        noofchar++;
        if (c == '\n')
        {
            if (lastc == c)
                /* found double newline */
                break;
            (*no_of_files)++;
        }
        lastc = c;
    }

    if (c == EOF)
    {
        /* problem .... file may have corrupted*/
        *no_of_files = 0;
        return;

    }
    /* allocate memeory for the filenames */
    *output_filenames = (char*)malloc(sizeof(char) * noofchar);
    /* roll back to start */
    fseek(lzw_file, 0, SEEK_SET);

    fread((*output_filenames), 1, (size_t)noofchar, lzw_file);

    return;
}

/*****************************************************************
 *
 * read_code() - reads a specific-size code from the code file
 * arguments:
 * 	input: the file pointer of input file. Make sure you file pointer
 * 		is at the start of the code sequence before you call this
 * 		function for the first time.
 * 	code_size: the length of code in bits. It should be 12 for reading
 * 		12-bit code.
 * 	behaviour: reads in one code each time. Will return the maximum
 * 		value if the data left in the file is less than the length
 * 		indicated by code_size.
 *
 ****************************************************************/
unsigned int read_code(FILE* input, unsigned int code_size)
{
    unsigned int return_value;
    static int input_bit_count = 0;
    static unsigned long input_bit_buffer = 0L;

    /* The code file is treated as an input bit-stream. Each     */
    /*   character read is stored in input_bit_buffer, which     */
    /*   is 32-bit wide.                                         */

    /* input_bit_count stores the no. of bits left in the buffer */

    while (input_bit_count <= 24) {
        input_bit_buffer |= (unsigned long)(unsigned char)getc(input) << (24 - input_bit_count);
        input_bit_count += 8;
    }

    return_value = input_bit_buffer >> (32 - code_size);
    input_bit_buffer <<= code_size;
    input_bit_count -= code_size;

    return(return_value);
}


/*****************************************************************
 *
 * write_code() - write a code (of specific length) to the file
 * arguments:
 * 	output: the file pointer of the output file.
 * 	code: an unsigned code value you want to write.
 * 	code_size: the length of code in bits. It should be 12 for reading
 * 		12-bit code.
 * behaviour: only flushes the code to the file when there is more than
 * 		8-bits of data in its buffer. So, to make sure all important data are written
 * 		to the compressed file, write at least 8 extra bits using this function
 * 		after you write the last code. (Writing a <4095> after the last code
 * 		of the last file will do the job)
 *
 ****************************************************************/
void write_code(FILE* output, unsigned int code, unsigned int code_size)
{
    static int output_bit_count = 0;
    static unsigned long output_bit_buffer = 0L;

    /* Each output code is first stored in output_bit_buffer,    */
    /*   which is 32-bit wide. Content in output_bit_buffer is   */
    /*   written to the output file in bytes.                    */

    /* output_bit_count stores the no. of bits left              */

    output_bit_buffer |= (unsigned long)code << (32 - code_size - output_bit_count);
    output_bit_count += code_size;
    while (output_bit_count >= 8) {
        putc(output_bit_buffer >> 24, output);
        output_bit_buffer <<= 8;
        output_bit_count -= 8;
    }


    /* only < 8 bits left in the buffer                          */

}

/*****************************************************************
 *
 * compress() - compress the source file and output the coded text
 * hints: write this function yourself according to the LZW algorithm.
 * 		You are free to change the arguments of this function.
 *
 ****************************************************************/
void compress(FILE* input, FILE* output)
{

    /* ADD CODES HERE */
    struct entry* curr = &ro;
    int check = fread(&cha, sizeof(unsigned char), 1, input);
    //printf("%d", byte_read);
    while (check == TRUE) {
        struct entry* next = curr->temp[cha];

        if (next == 0) {//0 is empty
            // If (STRING+CHAR) is not in DICT
            if (dictIndex >= 4095) {//new dict
                write_code(output, curr->loc, CODE_SIZE);
                //printf("%d\n", curr->loc );
                dict(FALSE);
                curr = &ro;
                continue;
            }
            else {
                write_code(output, curr->loc, CODE_SIZE);
                //printf("%d\n", curr->loc );
                insert(curr, cha, FALSE);
                curr = ro.temp[cha];
            }
        }
        else
            // If (STRING+CHAR) is in DICT
            curr = next;


        check = fread(&cha, sizeof(unsigned char), 1, input);
    }
    //printf("%d\n", curr->loc );
    write_code(output, curr->loc, CODE_SIZE);
    write_code(output, 4095, CODE_SIZE);
    //write_code(output, 4095, CODE_SIZE);
}



/*****************************************************************
 *
 * decompress() - decompress a compressed file to the orig. file
 * hints: write this function yourself according to the LZW algorithm.
 * 		You are free to change the arguments of this function.
 *
 ****************************************************************/
void decompress(FILE* input, FILE* output)
{

    /* ADD CODES HERE */
    unsigned int ncode, ocode;
    ncode = read_code(input, CODE_SIZE) & 0x0FFF;
    ocode = ncode;
    int i;
    if (ncode != 4095) {
        struct entry* wdict;
        wdict = (dictionary + ncode);
        for (i = 0; i < wdict->len; i++)
            putc(wdict->ins[i], output);


        while ((ncode = (read_code(input, CODE_SIZE) & 4095)) != 4095) {
            //printf("%d %d\n", ocode, ncode );
            // Check if the dictionary is full
            if (dictIndex >= 4095) {
                dict(FALSE);// If the dictionary is full, reset it
                ocode = ncode;
                wdict = (dictionary + ncode);
                for (i = 0; i < wdict->len; i++)
                    putc(wdict->ins[i], output);
            }
            else {// Get the entry at index ncode in the dictionary
                wdict = (dictionary + ncode);
                if (ncode < dictIndex) {//have

                    for (i = 0; i < wdict->len; i++)
                        putc(wdict->ins[i], output);
                    //fwrite(wdict->ins, 1, wdict->len, output);
                    //printf("%c\n", wdict->ins[i] );
                    dedict(wdict, ocode, TRUE);
                }
                else {//not
                    dedict(wdict, ocode, FALSE);

                    for (i = 0; i < wdict->len; i++)
                        putc(wdict->ins[i], output);
                    //fwrite(wdict->ins, 1, wdict->len, output);
                    //printf("%c\n", wdict->ins[i] );

                }
                ocode = ncode;
                //printf("%d %d\n", ocode, ncode );
            }
        }

    }
}

