#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "fileops.atlibby.alynkirscht.h"

int checkWord(char *word) {
    int i, len;

    len = strlen(word);
    for (i=0; i<len; ++i) {
        if ( ! isalpha(word[i]) )
            return 1;
    }

    return 0;
}

int convertToLower(char *word, char *convertedWord) {
    int i, len;

    strcpy(convertedWord, word);

    len = strlen(word);
    for (i=0; i<len; ++i)
        convertedWord[i] = tolower(word[i]);

    return 0;
}

int test_tolower(char *word) {
    int len, i, val;
    char newword[MAXWORDLEN];

    len = strlen(word);
    strcpy(newword, word);
    for (i=0; i<len; ++i) {
        newword[i] = tolower(word[i]);
    }
    printf("newword is |%s|\n", newword);
    return 0;
}

int testUtils() {
    char buffer[MAXWORDLEN];
    char convertedBuffer[MAXWORDLEN];
    int rc;

    test_tolower("Nargles");

    strcpy(buffer, "John's");
    rc = checkWord(buffer);
    printf("%s: %d\n", buffer, rc);

    strcpy(buffer, "maryRose");
    rc = checkWord(buffer);
    printf("%s: %d\n", buffer, rc);
    if (rc) {
        convertToLower(buffer, convertedBuffer);
        printf("converted: |%s|\n", convertedBuffer);
    }

    strcpy(buffer, "mary Rose");
    rc = checkWord(buffer);
    printf("%s: %d\n", buffer, rc);

    return 0;
}

char *getWord(FILE *fp, char letter, int index) {
    FileHeader fileHeader;
    WordRecord wordRecord;
    int rc;

    // Check for bad letter
    if (!isalpha(letter)) {
        printf("getWord: bad letter '%c'\n", letter);
        return NULL;
    }

    // Read the header
    fseek(fp, 0, SEEK_SET);
    rc = fread(&fileHeader, sizeof(FileHeader), 1, fp);
    if (rc != 1) {
        printf("getWord: fread failed\n");
        return NULL;
    }

    // Check for bad index
    if (index < 0 || index >= fileHeader.counts[letter - 'a']) {
        printf("getWord: bad index %d\n", index);
        return NULL;
    }

    // Check if there are no words starting with the specified letter
    if (fileHeader.counts[letter - 'a'] == 0) {
        printf("getWord: no words starting with '%c'\n", letter);
        return NULL;
    }

    // Check if index is out of bounds
    if (index >= fileHeader.counts[letter - 'a']) {
        printf("getWord: index out of bounds\n");
        return NULL;
    }

    // If index is 0, use startPositions directly
    if (index == 0) {
        fseek(fp, fileHeader.startPositions[letter - 'a'], SEEK_SET);
    } else {
        // Read the last index to get the position
        fseek(fp, fileHeader.startPositions[letter - 'a'] + (index - 1) * sizeof(WordRecord), SEEK_SET);
        rc = fread(&wordRecord, sizeof(WordRecord), 1, fp);
        if (rc != 1) {
            printf("getWord: fread failed\n");
            return NULL;
        }

        // fseek to the correct position for the current index
        fseek(fp, wordRecord.nextpos, SEEK_SET);
    }

    // Read the current index
    rc = fread(&wordRecord, sizeof(WordRecord), 1, fp);
    if (rc != 1) {
        printf("getWord: fread failed\n");
        return NULL;
    }

    // Allocate memory for the word and copy it
    char *wordCopy = strdup(wordRecord.word);
    if (wordCopy == NULL) {
        // Handle memory allocation failure
        printf("getWord: strdup failed\n");
        return NULL;
    }

    return wordCopy;
}


int insertWord(FILE *fp, char *word) {
    char convertedWord[1+MAXWORDLEN];
    int rc;
    WordRecord wordRecord;
    FileHeader fileHeader;

    // check for bad word
    rc = checkWord(word);
    if (rc) {
        printf("insertWord: bad word '%s'\n", word);
        return -1;
    }

    // convert to lower case
    convertToLower(word, convertedWord);

    // Writing a word to file
    // If the file is empty
    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    if (size == 0) {

        // Initialize counts and startPositions to 0
        for (int i = 0; i < NUMLETTERS; i++) {
            fileHeader.counts[i] = 0;
            fileHeader.startPositions[i] = 0;
        }

        // Write a new header with header.counts[10] = 1 and header.startPositions = sizeof(Header)
        fileHeader.counts[convertedWord[0] - 'a']++;
        fileHeader.startPositions[convertedWord[0] - 'a'] = sizeof(FileHeader);
        fseek(fp, 0, SEEK_SET);
        fwrite(&fileHeader, sizeof(FileHeader), 1, fp);

        // fseek() to position sizeof(Header) and write a record with (word, 0)
        fseek(fp, sizeof(FileHeader), SEEK_SET);
        strcpy(wordRecord.word, convertedWord);
        wordRecord.nextpos = 0;
        fwrite(&wordRecord, sizeof(WordRecord), 1, fp);

    // If the file is not empty
    } else {
        //  read the header
        fseek(fp, 0, SEEK_SET);
        fread(&fileHeader, sizeof(FileHeader), 1, fp);

        // increment header.counts[10]
        fileHeader.counts[convertedWord[0] - 'a']++;

        // if header.counts[10] == 1
        if (fileHeader.counts[convertedWord[0] - 'a'] == 1) {

            // set header.startPositions[10] to the current size of the file (must seek to file start and write the updated header)
            fseek(fp, 0,SEEK_END);
            fread(&fileHeader, sizeof(FileHeader), 1, fp);
            fileHeader.startPositions[convertedWord[0] - 'a'] = ftell(fp);
            fseek(fp, 0, SEEK_SET);
            fwrite(&fileHeader, sizeof(FileHeader), 1, fp);

            // fseek() to the end of the file and write a record with (word, 0)
            fseek(fp, 0, SEEK_END);
            strcpy(wordRecord.word, convertedWord);
            wordRecord.nextpos = 0;
            fwrite(&wordRecord, sizeof(WordRecord), 1, fp);

        //  else
        } else {
            fseek(fp, sizeof(FileHeader), SEEK_SET);
            while (fread(&wordRecord, sizeof(WordRecord), 1, fp) == 1) {
                // Check if the word already exists
                if (strcmp(wordRecord.word, convertedWord) == 0) {
                    printf("insertWord: duplicate word '%s'\n", convertedWord);
                    return -1;
                }
            }
            // successively read the records for words starting with 'k'
            fseek(fp, sizeof(FileHeader), SEEK_SET);
            int index = 0;
            while (fread(&wordRecord, sizeof(WordRecord), 1, fp) == 1) { // while there are still records to read

                if (wordRecord.word[0] == convertedWord[0] && wordRecord.nextpos == 0) {
                    // update the nextpos last record for words starting with 'k' to the current file size (must read this record, update it, and write it)

                    // Get the position of the beginning of the word
                    fseek(fp, sizeof(FileHeader), SEEK_SET);
                    long wordPosition = ftell(fp) + (sizeof(WordRecord) * index);

                    fseek(fp, 0, SEEK_END);
                    fread(&wordRecord, sizeof(WordRecord), 1, fp);
                    wordRecord.nextpos = ftell(fp);

                    // fseek to the beginning of the word
                    fseek(fp, wordPosition, SEEK_SET);
                    // Write the updated record back to the file
                    fwrite(&wordRecord, sizeof(WordRecord), 1, fp);

                    // fseek() to the end of the file and write a record with ("kerfuffle", 0)
                    fseek(fp, 0, SEEK_END);
                    strcpy(wordRecord.word, convertedWord);
                    wordRecord.nextpos = 0;
                    fwrite(&wordRecord, sizeof(WordRecord), 1, fp);

                    fseek(fp, ftell(fp) - sizeof(WordRecord), SEEK_SET);

                    // Write the updated record back to the file
                    fwrite(&wordRecord, sizeof(WordRecord), 1, fp);
                }
                index++;
            }
            // Update the header
            fseek(fp, 0, SEEK_SET);
            fwrite(&fileHeader, sizeof(FileHeader), 1, fp);
        }
    }
    return 0;
}

int countWords(FILE *fp, char letter, int *count) {
    FileHeader fileHeader;
    int rc;

    // check for bad letter
    if ( ! isalpha(letter) ) {
        printf("countWords: bad letter '%c'\n", letter);
        return 1;
    }

    // read the header
    fseek(fp, 0, SEEK_SET);
    rc = fread(&fileHeader, sizeof(FileHeader), 1, fp);
    if (rc != 1) {
        printf("countWords: fread failed\n");
        return 1;
    }

    // check for bad letter
    if ( ! isalpha(letter) ) {
        printf("countWords: bad letter '%c'\n", letter);
        return 1;
    }

    // return the count
    *count = fileHeader.counts[letter - 'a'];
    printf("countWords: count for '%c' is %d\n", letter, fileHeader.counts[letter - 'a']);
    return 0;
}
