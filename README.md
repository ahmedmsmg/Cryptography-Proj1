# Cryptography-Proj1

– Toy Stream Cipher Implementation This exercise will put basic cryptographic tools and toy stream cipher that were discussed during the class into action. Specifically, consider the toy stream cipher built from a Pseudo Random Number Generator (PRNG) as described in “Basics.ppt”, Slide 11. In this exercise, we will utilize the deterministic PRNG function ChaCha20 from Libtomcrypt. Moreover, as alluded to in Slide 12, to avoid repeating keys and information leakage, a PRNG must have a “seed”. 
Alice: 1. Alice reads the message from the “Message.txt” file. The message size must be equal or greater than 32 bytes. (Read the message as an unsigned char so that you could use the functions provided in in-class exercises.) 
2. Alice reads the shared seed from the “SharedSeed.txt” file. The seed is 32 Bytes (Read the message as unsigned char so that you could use the functions provided in in-class exercises.) 
3. Alice generates the secret key from the shared seed based on utilizing the PRNG function from LibTomCrypt. The key size must match the message length. 
4. Alice writes the Hex format of the key in a file named “Key.txt”. 
5. Alice XORs the message with the secret key to obtain the ciphertext: (Ciphertext = MessageLKey). 
6. Alice writes the Hex format of the ciphertext in a file named “Ciphertext.txt”. 
7. Finally, Alice sends the ciphertext to Bob via zeroMQ. (The ciphertext format is unsigned char. Do Not send Hex strings!) 
8. Alice is anticipating an ”acknowledgment” from Bob using ZeroMQ. This ”acknowledgment” refers to the hash value of the original text. Alice has the ability to match the hash she receives with the hash of the original message. (Use SHA256 as Hash function) 
9. If the comparison is successful, Alice can be confident that Bob has received the accurate message. She then writes ”Acknowledgment Successful” in a file called ”Acknowledgment.txt.” Conversely, if the comparison fails, she records ”Acknowledgment Failed.”
Bob: 
1. Bob receives the ciphertext from Alice via ZeroMQ. 
2. Bob reads the shared seed from the “SharedSeed.txt” file. The seed is 32 Bytes (Read the message as unsigned char so that you could use the functions provided in in-class exercises.) 
3. Bob generates the secret key from the shared seed based on utilizing the PRNG function from LibTomCrypt. The key size must match the message length. 4. Bob XORs the received ciphertext with the secret key to obtain the plaintext: (plaintext = ciphertextLkey). 
5. Bob writes the decrypted plaintext in a file named “Plaintext.txt”. 
6. Bob hashes the plaintext via SHA256 and writes the Hex format of the hash in a file named ”Hash.txt”. 
7. Finally, Bob sends the hash over ZeroMQ to the Alice as an Acknowledgment. (Do Not send Hex format! Use unsigned char to send the data.)
2 NOTES:  There are two terminals that we will refer to as “Alice” and “Bob” as the sender and receiver, respectively. These two terminals will communicate via ZeroMQ as discussed in the demo video and in-class exercises. 
 In grading at our side, the seed, message, and lengths will be chosen at random and everything must work.
 The code that throws errors or does not successfully compile/run on our side, receives zero credit (It doesn’t matter if it works on your computer!). 
 You require two files: “Message.txt” and “SharedSeed.txt”. 
 For the correctness of reading from a file and XORing, please use unsigned char. 
 We have provided a script and test files that you could use to test the correctness of your final codes (alice.c and bob.c). Please, only submit your code if it is functional. 
 You can check your answer with each provided test file manually or you can use the script to verify your solution for the whole HW1 programming question. 
 In order to use the script, just put “alice.c”, “bob.c”, the rest of the provide files along with the “VerifyingYourSolution1.sh” in one folder and run the following command in the terminal: bash VerifyingYourSolution1.sh 
 Compile your codes with the following commands: gcc alice.c -ltomcrypt -lzmq -o alice gcc bob.c -ltomcrypt -lzmq -o bob
  Run your codes for the first test files with the following commands: ./alice Message1.txt SharedSeed1.txt ./bob SharedSeed1.txt 
