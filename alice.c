#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libtomcrypt/tomcrypt.h>
#include <zmq.h>

// Function prototypes
unsigned char* Read_File(char fileName[], int* fileLen);
unsigned char* Hash_SHA256(unsigned char input[], unsigned long inputlen);
unsigned char* PRNG(unsigned char* seed, unsigned long seedlen, unsigned long prnlen);
void Show_in_Hex(char name[], unsigned char hex[], int hexlen);
void Send_via_ZMQ(unsigned char send[], int sendlen);
unsigned char* Receive_via_ZMQ(int limit, int *received_length);

void check_file(FILE *fp, const char *filename) {
    if (fp == NULL) {
        printf("Failed to open file: %s\n", filename);
        exit(1);
    }
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        printf("Usage: %s <MessageFile> <SeedFile>\n", argv[0]);
        return 1;
    }

    int message_length = 0;
    unsigned char* message = Read_File(argv[1], &message_length);
    printf("Message length: %d\n", message_length);

    int seed_length = 0;
    unsigned char* seed = Read_File(argv[2], &seed_length);
    printf("Seed length: %d\n", seed_length);

    unsigned char* key = PRNG(seed, seed_length, message_length);
    printf("Generated key.\n");

    FILE* keyFile = fopen("Key.txt", "w");
    check_file(keyFile, "Key.txt");
    for (int i = 0; i < message_length; i++) {
        fprintf(keyFile, "%02x", key[i]);
    }
    fclose(keyFile);
    printf("Saved key to Key.txt\n");

    unsigned char* ciphertext = (unsigned char*) malloc(message_length);
    for (int i = 0; i < message_length; i++) {
        ciphertext[i] = message[i] ^ key[i];
    }

    FILE* cipherFile = fopen("Ciphertext.txt", "w");
    check_file(cipherFile, "Ciphertext.txt");
    for (int i = 0; i < message_length; i++) {
        fprintf(cipherFile, "%02x", ciphertext[i]);
    }
    fclose(cipherFile);
    printf("Saved ciphertext to Ciphertext.txt\n");

    printf("Sending ciphertext via ZeroMQ...\n");
    Send_via_ZMQ(ciphertext, message_length);

    int received_length = 0;
    printf("About to receive acknowledgment via ZeroMQ...\n");
unsigned char* received_hash = Receive_via_ZMQ(32, &received_length);
printf("Received acknowledgment via ZeroMQ.\n");

    if (received_hash == NULL || received_length != 32) {
        printf("Failed to receive acknowledgment hash or incorrect length received.\n");
        return 1;
    }
    printf("Received acknowledgment hash.\n");

    unsigned char* original_hash = Hash_SHA256(message, message_length);
    printf("Calculated original hash.\n");

    FILE* ackFile = fopen("Acknowledgment.txt", "w");
    check_file(ackFile, "Acknowledgment.txt");

    if (memcmp(original_hash, received_hash, 32) == 0) {
        fprintf(ackFile, "Acknowledgment Successful");
        printf("Acknowledgment Successful\n");
    } else {
        fprintf(ackFile, "Acknowledgment Failed");
        printf("Acknowledgment Failed\n");
    }
    fclose(ackFile);
    printf("Saved result to Acknowledgment.txt\n");

    return 0;
}


// Include your original functions (Read_File, Hash_SHA256, PRNG, Show_in_Hex, Send_via_ZMQ, Receive_via_ZMQ) here
unsigned char* Read_File (char fileName[], int *fileLen)
{
FILE *pFile;
pFile = fopen(fileName, "r");
if (pFile == NULL)
{
printf("Error opening file.\n");
exit(0);
}
fseek(pFile, 0L, SEEK_END);
int temp_size = ftell(pFile)+1;
fseek(pFile, 0L, SEEK_SET);
unsigned char *output = (unsigned char*) malloc(temp_size);
fgets(output, temp_size, pFile);
fclose(pFile);
*fileLen = temp_size-1;
return output;
}
unsigned char* Hash_SHA256(unsigned char* input, unsigned long inputlen)
{
    unsigned char *hash_result = (unsigned char*) malloc(32); // Always 32 bytes for SHA-256
    hash_state md;
    sha256_init(&md);
    sha256_process(&md, (const unsigned char*)input, inputlen);
    sha256_done(&md, hash_result);
    return hash_result;
}


unsigned char* PRNG(unsigned char *seed, unsigned long seedlen, unsigned long
prnlen)
{
int err;
unsigned char *pseudoRandomNumber = (unsigned char*) malloc(prnlen);
prng_state prng;
//LibTomCrypt structure for PRNG
if ((err = chacha20_prng_start(&prng)) != CRYPT_OK){
//Sets up the PRNG state without a seed
printf("Start error: %s\n", error_to_string(err));
}
if ((err = chacha20_prng_add_entropy(seed, seedlen, &prng)) != CRYPT_OK) {
//Uses a seed to add entropy to the PRNG
printf("Add_entropy error: %s\n", error_to_string(err));
}
if ((err = chacha20_prng_ready(&prng)) != CRYPT_OK) {
//Puts the entropy into action
printf("Ready error: %s\n", error_to_string(err));
}
chacha20_prng_read(pseudoRandomNumber, prnlen, &prng);
//Writes the result into pseudoRandomNumber[]
if ((err = chacha20_prng_done(&prng)) != CRYPT_OK) {
//Finishes the PRNG state
printf("Done error: %s\n", error_to_string(err));
}
return (unsigned char*)pseudoRandomNumber;
}

void Show_in_Hex (char name[], unsigned char hex[], int hexlen)
{
printf("%s: ", name);
for (int i = 0 ; i < hexlen ; i++)
printf("%02x", hex[i]);
printf("\n");
}

void Send_via_ZMQ(unsigned char send[], int sendlen) {
    void *context = zmq_ctx_new();
    void *requester = zmq_socket(context, ZMQ_REQ);
    printf("Connecting to Bob and sending the message...\n");
    zmq_connect(requester, "tcp://localhost:5557");  // Connects to Bob on port 5556
    zmq_send(requester, send, sendlen, 0);
    zmq_close(requester);
    zmq_ctx_destroy(context);
}

unsigned char* Receive_via_ZMQ(int limit, int *received_length) {
    void *context = zmq_ctx_new();
    void *responder = zmq_socket(context, ZMQ_REP);
    int rc = zmq_bind(responder, "tcp://*:5555");

    unsigned char *receive = (unsigned char*) malloc(limit);
    *received_length = zmq_recv(responder, receive, limit, 0);

    unsigned char *temp = (unsigned char*) malloc(*received_length);
    memcpy(temp, receive, *received_length);

    zmq_close(responder);
    zmq_ctx_destroy(context);

    free(receive);
    return temp;
}


