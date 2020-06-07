/*
* CSCI3280 Introduction to Multimedia Systems *
* --- Declaration --- *
* I declare that the assignment here submitted is original except for source
* material explicitly acknowledged. I also acknowledge that I am aware of
* University policy and regulations on honesty in academic work, and of the
* disciplinary guidelines and procedures applicable to breaches of such policy
* and regulations, as contained in the website
* http://www.cuhk.edu.hk/policy/academichonesty/ *
* Assignment 3
* Name : Wong Ching Yeung Wallace
* Student ID : 1155093534
* Email Addr : 1155093534@link.cuhk.edu.hk
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>

#define CODE_SIZE  12
#define TRUE 1
#define EMPTY 0x0000
#define FALSE 0

int counting = 0 ;

/* function prototypes */
unsigned int read_code(FILE*, unsigned int);
void write_code(FILE*, unsigned int, unsigned int);
void writefileheader(FILE*, char**, int);
void readfileheader(FILE*, char**, int*);
void compress(FILE*, FILE*);
void decompress(FILE*, FILE*);


//Using 2D array to store the dictionary.
unsigned int dictionary[4096][257];
unsigned int number, switching;
bool pass; 

int main(int argc, char** argv)
{
	int printusage = 0;
	int	no_of_file;
	char** input_file_names;
	char* output_file_names;
	FILE* lzw_file;

	if (argc >= 3)
	{
		if (strcmp(argv[1], "-c") == 0)
		{
			/* compression */
			lzw_file = fopen(argv[2], "wb");

			/* write the file header */
			input_file_names = argv + 3;
			no_of_file = argc - 3;
			for (int i = 3; i < (3+ no_of_file); i++)
				printf("Adding : %s\n",argv[i]);
			writefileheader(lzw_file, input_file_names, no_of_file);

			counting = no_of_file;
			/* ADD CODES HERE */
			int multi = 4095 * 257;
			number = 256;
			memset(dictionary, EMPTY, sizeof(unsigned short) * multi);
			
			int no = 0;
			while (1)
			{
				if (no == counting)
				{
					break;
				}
				FILE* inputFile = fopen(input_file_names[no], "rb");
				compress(inputFile, lzw_file);
				fclose(inputFile);
				no++;
			}

		}
		else
			if (strcmp(argv[1], "-d") == 0)
			{
				/* decompress */
				lzw_file = fopen(argv[2], "rb");

				/* read the file header */
				no_of_file = 0;
				readfileheader(lzw_file, &output_file_names, &no_of_file);
				counting = no_of_file;

				/* ADD CODES HERE */
				int multi = 4095 * 257;
				number = 256;
				memset(dictionary, EMPTY, sizeof(unsigned short) * multi);
				char* outPutSingleFilename = strtok(output_file_names, "\n"); // reading the filename by spiliting "\n"
				int no = 0;
				while (1)
				{
					if (no == counting)
					{
						break;
					}
					FILE * outputFile  = fopen(outPutSingleFilename, "wb");
					printf("Deflating : %s\n", outPutSingleFilename);
					decompress(lzw_file, outputFile);
					//Cutting the file name by NULL;
					outPutSingleFilename = strtok(NULL, "\n");
					no++;
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
		input_bit_buffer |= (unsigned long)getc(input) << (24 - input_bit_count);
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
 *
 ****************************************************************/
void compress(FILE* input, FILE* output)
{
	/* ADD CODES HERE */
	FILE* inputFile = input;
	int compressNumber = 0, counter = 0;
	unsigned char singleCharacter,s,ch;
	unsigned short prefix;

	prefix = fread(&ch, 1, 1, inputFile);
	if (prefix == 0)
	{
		write_code(output, 4095, 256);
		return;
	}
	for (int i = 0; i < counting; i++)
	{
		compressNumber++;
	}

	prefix = ch;
	singleCharacter = ch;

	while (fread(&ch, 1, 1, inputFile) > 0 )
	{
		//printf("%c", ch);
		//printf("%d %c \n", Node, ch);
		if (dictionary[prefix][ch] == NULL)
		{
			//printf("%d", dictionary[prefix][ch]);
			//If not found 
			write_code(output, prefix, CODE_SIZE);
			if (number == 4095)
			{
				number = 256;
				memset(dictionary, EMPTY, sizeof(unsigned short) * (257 * 4095));
			}
			else 
			{
				//If found
				dictionary[prefix][ch] = number;
				number++;
			}
			prefix = ch;
		}
		else if (dictionary[prefix][ch] != NULL) // If found
		{
			prefix = dictionary[prefix][ch];
		}
	}
	write_code(output, prefix, CODE_SIZE);
	write_code(output, 4095, CODE_SIZE);
	fclose(inputFile);
}

unsigned char changing[3839], mapper[3839];
unsigned short setter[3800];
int foo;

unsigned char CWPW(unsigned short cW, int pw,FILE* input)
{
	char C, S, P;
	int length =0, root=0;

	if (cW < 256)
	{
		fwrite(&cW, 1, 1, input);
		return cW;
	}

	while (cW >= 256) {
		changing[length] = mapper[cW - 256];
		cW = dictionary[cW][256];
		length++;
	}
	C = cW;
	changing[length] = C;
	if (pw == 1 )
	{
		for (int i = 0; i < pw; i++)
		{
			pass = false;
		}
	}
	while (length >= 0) {
		fwrite(&(changing[length]), 1, 1, input);
		length--;
	}
	return C;
}

/*****************************************************************
 *
 * decompress() - decompress a compressed file to the orig. file
 *
 ****************************************************************/
void decompress(FILE* input, FILE* output)
{
	FILE* outPutFile = output;
	FILE* lzwFile = input;
	unsigned short cW, S;
	char* filename;
	char C;
	//printf("HI \n");

	cW = read_code(input, CODE_SIZE) & 0x0FFF;

	if (cW == 4095) 
	{
		fclose(outPutFile);
		return;
	}
	CWPW( cW,counting, outPutFile);
	while (cW != 4095) 
	{
		S = cW;
		cW = read_code(lzwFile, CODE_SIZE) & 0x0FFF;
		if (cW == 4095) 
			break;

		if (cW < 256|| dictionary[cW][256] != EMPTY ) 
		{
			C = CWPW(cW, counting, outPutFile);
		}
		else
		{
			C = CWPW(S, counting, outPutFile);
			CWPW( C, counting, outPutFile);
		}
		int multi = 257*4095;
		if (number == 4095)
		{
			number = 256;
			memset(dictionary, EMPTY, sizeof(unsigned short)*multi);
			continue;
		}
		//Let PW = CW.  
		dictionary[S][C] = number;
		dictionary[number][256] = S;
		mapper[number - 256] = C;
		number++;
	}
	fclose(outPutFile);
}