#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <openssl/hmac.h>
#include <openssl/evp.h>
#include <omp.h>

int base64url_decode(const char *input, unsigned char *output) {
    int len = strlen(input);
    int i, j = 0;
    int v = 0, val = 0;
    for (i = 0; i < len; i++) {
        char c = input[i];
        if (c >= 'A' && c <= 'Z') v = c - 'A';
        else if (c >= 'a' && c <= 'z') v = c - 'a' + 26;
        else if (c >= '0' && c <= '9') v = c - '0' + 52;
        else if (c == '-') v = 62;
        else if (c == '_') v = 63;
        else continue;

        val = (val << 6) | v;
        if (i % 4 == 3) {
            output[j++] = (val >> 16) & 0xFF;
            output[j++] = (val >> 8) & 0xFF;
            output[j++] = val & 0xFF;
            val = 0;
        }
    }
    if (i % 4 == 2) {
        output[j++] = (val >> 4) & 0xFF;
    } else if (i % 4 == 3) {
        output[j++] = (val >> 10) & 0xFF;
        output[j++] = (val >> 2) & 0xFF;
    }
    return j;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <wordlist.file> <JWT.token>\n", argv[0]);
        return 1;
    }

    char *wordlist_file = argv[1];
    char *token = argv[2];

    FILE *file = fopen(wordlist_file, "r");
    if (!file) {
        printf("'%s' file not found...\n", wordlist_file);
        return 1;
    }

    char *dot1 = strchr(token, '.');
    char *dot2 = strrchr(token, '.');
    
    if (!dot1 || !dot2 || dot1 == dot2) {
        printf("Invalid JWT token format!!\n");
        fclose(file);
        return 1;
    }

    int capacity = 1000000;
    char **wordlist = malloc(capacity * sizeof(char*));
    int word_count = 0;
    char line[512];

    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\r\n")] = 0; 
        if (strlen(line) == 0) continue;

        if (word_count >= capacity) {
            capacity *= 2;
            wordlist = realloc(wordlist, capacity * sizeof(char*));
        }
        wordlist[word_count] = strdup(line);
        word_count++;
    }
    fclose(file);
    printf("%d words found!!\n\n", word_count);

    char data[2048] = {0};
    char sig_b64[512] = {0};
    unsigned char target_hash[32];

    strncpy(data, token, dot2 - token);
    strcpy(sig_b64, dot2 + 1);

    base64url_decode(sig_b64, target_hash);

    bool is_found = false;
    char found_key[512] = {0};

    double start_time = omp_get_wtime();

    #pragma omp parallel
    {
        unsigned char hash[EVP_MAX_MD_SIZE];
        unsigned int hlen;

        #pragma omp for schedule(dynamic, 1000)
        for (int i = 0; i < word_count; i++) {
            if (is_found) continue;

            char *guess_key = wordlist[i];
            HMAC(EVP_sha256(), guess_key, strlen(guess_key), (unsigned char*)data, strlen(data), hash, &hlen);

            if (memcmp(hash, target_hash, 32) == 0) {
                #pragma omp critical
                {
                    if (!is_found) {
                        is_found = true;
                        strcpy(found_key, guess_key);
                    }
                }
            }
        }
    }

    double end_time = omp_get_wtime();

    if (is_found) {
        printf("Found Secret Key: %s\n", found_key);
    } else {
        printf("Cannot Found Secret Key... Try to use another wordlist\n");
    }

    printf("Time Consumed: %.4f초\n", end_time - start_time);

    for (int i = 0; i < word_count; i++) {
        free(wordlist[i]);
    }
    free(wordlist);

    return 0;
}