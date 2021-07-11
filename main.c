/************************************************************************
Lab 9 Nios Software

Dong Kai Wang, Fall 2017
Christine Chen, Fall 2013

For use with ECE 385 Experiment 9
University of Illinois ECE Department
************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "aes.h"
#include <math.h>

// Pointer to base address of AES module, make sure it matches Qsys
volatile unsigned int * AES_PTR = (unsigned int *) 0x00000040;

// Execution mode: 0 for testing, 1 for benchmarking
int run_mode = 0;

/** charToHex
 *  Convert a single character to the 4-bit value it represents.
 *
 *  Input: a character c (e.g. 'A')
 *  Output: converted 4-bit value (e.g. 0xA)
 */
char charToHex(char c)
{
	char hex = c;

	if (hex >= '0' && hex <= '9')
		hex -= '0';
	else if (hex >= 'A' && hex <= 'F')
	{
		hex -= 'A';
		hex += 10;
	}
	else if (hex >= 'a' && hex <= 'f')
	{
		hex -= 'a';
		hex += 10;
	}
	return hex;
}

/** charsToHex
 *  Convert two characters to byte value it represents.
 *  Inputs must be 0-9, A-F, or a-f.
 *
 *  Input: two characters c1 and c2 (e.g. 'A' and '7')
 *  Output: converted byte value (e.g. 0xA7)
 */
char charsToHex(char c1, char c2)
{
	char hex1 = charToHex(c1);
	char hex2 = charToHex(c2);
	return (hex1 << 4) + hex2;
}

void RotWord(unsigned char * key, int i) { //this i refers to column BEFORE the column we want to fill
	unsigned char temp[4] = {0};
	key[i+4+0] = key[i+1];
	key[i+4+1] = key[i+2];
	key[i+4+2] = key[i+3];
	key[i+4+3] = key[i+0];
}

void SubWord(unsigned char * key, int i) { //this i refers to the column that we DO want to fill.
	unsigned char temp = 0;
	int j,k;
	for(k = i; k < i+4; k++){
		for(j = 0; j < 4; j++){
			if(key[k]%2 == 0){
				temp += 0;
			} else {
				temp += pow(2,j);
			}
			key[k] = key[k] >> 1;
		}
		key[k] = aes_sbox[key[k]*16+temp];
		temp = 0;
	}
}

void KeyExpansion(unsigned char* key)
{
	int i;
	for(i = 3; i < 43; i++){ //looking at the column index i to fill in columnindex i+1
		if((i+1)%4 == 0){
			RotWord(key, 4*i);
			SubWord(key, 4*(i+1));
			key[4*(i+1)+0] = key[4*(i+1)+0] ^ key[4*(i-3)+0] ^ Rcon[(i+1)/4]; //FYI I deleted the end zeroes on the Rcon in the .h doc.
			key[4*(i+1)+1] = key[4*(i+1)+1] ^ key[4*(i-3)+1];
			key[4*(i+1)+2] = key[4*(i+1)+2] ^ key[4*(i-3)+2];
			key[4*(i+1)+3] = key[4*(i+1)+3] ^ key[4*(i-3)+3];
		} else {
			key[4*(i+1)+0] = key[4*i+0] ^ key[4*(i-3)+0];
			key[4*(i+1)+1] = key[4*i+1] ^ key[4*(i-3)+1];
			key[4*(i+1)+2] = key[4*i+2] ^ key[4*(i-3)+2];
			key[4*(i+1)+3] = key[4*i+3] ^ key[4*(i-3)+3];

		}
	}
}

void AddRoundKey(unsigned char* state, unsigned char* key, unsigned int p) {
	int i;
	int j = 0;
	for(i = p; i < p + 16; i++){
		state[j] = state[j] ^ key[i];
		j++;
	}
}

void SubBytes(unsigned char* state){
	unsigned char temp = 0; //technically, you don't need it as an array lol.
	int i,j;
	for(i = 0; i < 16; i++){
		for(j = 0; j < 4; j++){
			if(state[i]%2 == 0){
				temp += 0;
			} else {
				temp += pow(2,j);
			}
			state[i] = state[i] >> 1;
		}
		state[i] = aes_sbox[state[i]*16+temp];
		temp = 0;
	}
}

void ShiftRows(unsigned char* state){

	//2nd row left shift 1
	unsigned char temp = state[1];
	state[1] = state[5];
	state[5] = state[9];
	state[9] = state[13];
	state[13] = temp;

	//3rd row left shift 2
	unsigned char temp1 = state[2];
	unsigned char temp2 = state[6];
	state[2] = state[10];
	state[6] = state[14];
	state[10] = temp1;
	state[14] = temp2;

	//4th row left shift 3 = right shift 1
	unsigned temp3 = state[15];
	state[15] = state[11];
	state[11] = state[7];
	state[7] = state[3];
	state[3] = temp3;
}


unsigned char xtime(unsigned char x) {
	return ((x << 1) ^ ((x & 0x80) ? 0x1b : 0x00));
}

void MixColumns(unsigned char* a) {
	unsigned char b[16];
	int j;
	for(j = 0; j < 4; j++) {
		b[4*j+0] = xtime(a[4*j+0]) ^ (xtime(a[4*j+1]) ^ a[4*j+1]) ^ a[4*j+2] ^ a[4*j+3];
		b[4*j+1] = a[4*j+0] ^ xtime(a[4*j+1]) ^ (xtime(a[4*j+2]) ^ a[4*j+2]) ^ a[4*j+3];
		b[4*j+2] = a[4*j+0] ^ a[4*j+1] ^ xtime(a[4*j+2]) ^ (xtime(a[4*j+3]) ^ a[4*j+3]);
		b[4*j+3] = (xtime(a[4*j+0]) ^ a[4*j+0]) ^ a[4*j+1] ^ a[4*j+2] ^ xtime(a[4*j+3]);
	}
	for (j = 0; j < 16; j++){
        a[j] = b[j];
	}
}



/** encrypt
 *  Perform AES encryption in software.
 *
 *  Input: msg_ascii - Pointer to 32x 8-bit char array that contains the input message in ASCII format
 *         key_ascii - Pointer to 32x 8-bit char array that contains the input key in ASCII format
 *  Output:  msg_enc - Pointer to 4x 32-bit int array that contains the encrypted message
 *               key - Pointer to 4x 32-bit int array that contains the input key
 */
void encrypt(unsigned char * msg_ascii, unsigned char * key_ascii, unsigned int * msg_enc, unsigned int * key)
{
	// Implement this function
	unsigned char k_temp[176] = {0};
	unsigned char * k = k_temp;
	unsigned char state_temp[16] = {0};
	unsigned char * state = state_temp;
	unsigned char fkey = 0; //final key
	unsigned char fmsg_enc = 0; //final msg_enc
	int i;
	int a;

	for (i = 0; i < 16; i++){
		k[i] = charsToHex(key_ascii[2*i], key_ascii[2*i+1]);
		state[i] = charsToHex(msg_ascii[2*i], msg_ascii[2*i+1]);
	}

	KeyExpansion(k);
	AddRoundKey(state, k, 0);

	for (i = 0; i < 9; i++){
		SubBytes(state);
		ShiftRows(state);
		MixColumns(state);
		AddRoundKey(state, k, 16*(i+1));
	}

	SubBytes(state);
	ShiftRows(state);
	AddRoundKey(state, k, 16*10);
	for (int i = 0; i < 4; i++) {
		msg_enc[i] = (state[4*i] << 24) + (state[4*i + 1] << 16) + (state[4*i + 2] << 8) + state[4*i+3];
		key[i] = (k[i*4] << 24) + (k[i*4+1] << 16)+ (k[i*4+2] << 8) + k[i*4+3];
	}

}

/** decrypt
 *  Perform AES decryption in hardware.
 *
 *  Input:  msg_enc - Pointer to 4x 32-bit int array that contains the encrypted message
 *              key - Pointer to 4x 32-bit int array that contains the input key
 *  Output: msg_dec - Pointer to 4x 32-bit int array that contains the decrypted message
 */
void decrypt(unsigned int * msg_enc, unsigned int * msg_dec, unsigned int * key)
{
	// Implement this function
	AES_PTR[0] = key[3];
	AES_PTR[1] = key[2];
	AES_PTR[2] = key[1];
	AES_PTR[3] = key[0];

	AES_PTR[4] = msg_enc[3];
	AES_PTR[5] = msg_enc[2];
	AES_PTR[6] = msg_enc[1];
	AES_PTR[7] = msg_enc[0];

	AES_PTR[15] = 0; //clear it just in case.
	AES_PTR[14] = 1;
	while(AES_PTR[15] != 1){
		//nothing happens
	}
	msg_dec[0] = AES_PTR[11];
	msg_dec[1] = AES_PTR[10];
	msg_dec[2] = AES_PTR[9];
	msg_dec[3] = AES_PTR[8];
	AES_PTR[14] = 0;


}


/** main
 *  Allows the user to enter the message, key, and select execution mode
 *
 */
int main()
{

	/*
	AES_PTR[15] = 0x13111;
	if (AES_PTR[15] != 0x13111){
		printf("There is an error");
	} else {
		printf("There are no errors yaaaaaay!!!");
	}
	*/



	// Input Message and Key as 32x 8-bit ASCII Characters ([33] is for NULL terminator)
	unsigned char msg_ascii[33] = "ece298dcece298dcece298dcece298dc";
	unsigned char key_ascii[33] = "000102030405060708090a0b0c0d0e0f";
	// Key, Encrypted Message, and Decrypted Message in 4x 32-bit Format to facilitate Read/Write to Hardware
	unsigned int key[4];
	unsigned int msg_enc[4];
	unsigned int msg_dec[4];

	printf("Select execution mode: 0 for testing, 1 for benchmarking: ");
	scanf("%d", &run_mode);

	if (run_mode == 0) {
		// Continuously Perform Encryption and Decryption
		while (1) {
			int i = 0;
			printf("\nEnter Message:\n");

			/*
			scanf("%s", msg_ascii);
			printf("\n");
			printf("\nEnter Key:\n");
			scanf("%s", key_ascii);
			printf("\n");
			*/

			encrypt(msg_ascii, key_ascii, msg_enc, key);
			printf("\nEncrypted message is: \n");
			for(i = 0; i < 4; i++){
				printf("%08x", msg_enc[i]);
			}
			printf("\n");


			decrypt(msg_enc, msg_dec, key);
			printf("\nDecrypted message is: \n");
			for(i = 0; i < 4; i++){
				printf("%08x", msg_dec[i]);
			}
			printf("\n");


		}

	}
	else {
		// Run the Benchmark
		int i = 0;
		int size_KB = 2;
		// Choose a random Plaintext and Key
		for (i = 0; i < 32; i++) {
			msg_ascii[i] = 'a';
			key_ascii[i] = 'b';
		}
		// Run Encryption
		clock_t begin = clock();
		for (i = 0; i < size_KB * 64; i++)
			encrypt(msg_ascii, key_ascii, msg_enc, key);
		clock_t end = clock();
		double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
		double speed = size_KB / time_spent;
		printf("Software Encryption Speed: %f KB/s \n", speed);
		// Run Decryption
		begin = clock();
		for (i = 0; i < size_KB * 64; i++)
			decrypt(msg_enc, msg_dec, key);
		end = clock();
		time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
		speed = size_KB / time_spent;
		printf("Hardware Encryption Speed: %f KB/s \n", speed);
	}
	return 0;

}
