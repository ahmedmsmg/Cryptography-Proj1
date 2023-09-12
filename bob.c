#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libtomcrypt/tomcrypt.h>
#include <zmq.h>

// Function prototypes
unsigned char* Read_File(char fileName[], int* fileLen);
unsigned char* Hash_SHA256(unsigned char input[], unsigned long inputlen);
unsigned char* PRNG(unsigned char* seed, unsigned long seedlen, unsigned long prnlen);
void Send_via_ZMQ(unsigned char send[], int sendlen);
unsigned char* Receive_via_ZMQ(int limit, int *received_length);
void Write_File(char fileName[], unsigned char* data, int dataLen);

int main(int argc, char* argv[]) {
    // Read the shared seed from the file specified in argv[1]
    int seed_length = 0;
    unsigned char* seed = Read_File(argv[1], &seed_length);
    
    // Receive the ciphertext from Alice via ZeroMQ
    int received_length = 0;
    unsigned char* ciphertext = Receive_via_ZMQ(1024, &received_length);  // Assuming max length of 1024
    
    // Generate the key using the PRNG function
    unsigned char* key = PRNG(seed, seed_length, received_length);
    
    // Decrypt the ciphertext to get the original message
    unsigned char* decrypted_message = (unsigned char*) malloc(received_length);
    for (int i = 0; i < received_length; i++) {
        decrypted_message[i] = ciphertext[i] ^ key[i];
    }

    Write_File("Plaintext.txt", decrypted_message, received_length);
    // Hash the decrypted message
    unsigned char* hash_message = Hash_SHA256(decrypted_message, received_length);
    
    FILE* hashFile = fopen("Hash.txt", "w");
    for (int i = 0; i < 32; i++) {
        fprintf(hashFile, "%02x", hash_message[i]);
    }
    fclose(hashFile);
    
    Send_via_ZMQ(hash_message, 32);
    return 0;
}

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
    void *requester = zmq_socket(context, ZMQ_REQ); // Creates requester that sends the messages
    printf("Connecting to ALice and Receiving the message...\n");
    printf("Connecting to Alice and sending the acknowledgment...\n");
    zmq_connect(requester, "tcp://localhost:5555"); // Make outgoing connection from socket
    zmq_send(requester, send, sendlen, 0); // Send msg to Bob
    zmq_close(requester); // Closes the requester socket
    zmq_ctx_destroy(context); // Destroys the context & terminates all 0MQ processes
}

unsigned char* Receive_via_ZMQ(int limit, int *received_length) {
    void *context = zmq_ctx_new();
    void *responder = zmq_socket(context, ZMQ_REP);
    int rc = zmq_bind(responder, "tcp://*:5557");

    if (rc != 0) {
        printf("Error binding to port 5557. Exiting.\n");
        exit(1);
    }

    unsigned char *receive = (unsigned char*) malloc(limit);
    *received_length = zmq_recv(responder, receive, limit, 0);  // Blocking call

    unsigned char *temp = (unsigned char*) malloc(*received_length);
    memcpy(temp, receive, *received_length);

    zmq_close(responder);
    zmq_ctx_destroy(context);

    free(receive);
    return temp;
}

void Write_File(char fileName[], unsigned char* data, int dataLen) {
    FILE *file = fopen(fileName, "w");
    if (file == NULL) {
        printf("Error opening file for writing: %s\n", fileName);
        exit(1);
    }
    fwrite(data, sizeof(unsigned char), dataLen, file);
    fclose(file);
}
