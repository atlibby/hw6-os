//
// Created by Andrew Libby on 12/4/23.
//

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
    FileHeader fh;
    int rc, i, len;
    WordRecord wr;

    // check for bad letter
    if ( ! isalpha(letter) ) {
        printf("getWord: bad letter '%c'\n", letter);
        return NULL;
    }

    // read the header
    fseek(fp, 0, SEEK_SET);
    rc = fread(&fh, sizeof(FileHeader), 1, fp);
    if (rc != 1) {
        printf("getWord: fread failed\n");
        return NULL;
    }

    // check for bad index
    if (index < 0 || index >= fh.counts[letter - 'a']) {
        printf("getWord: bad index %d\n", index);
        return NULL;
    }

    // read the word
    fseek(fp, fh.startPositions[letter - 'a'] + index * sizeof(WordRecord), SEEK_SET);
    rc = fread(&wr, sizeof(WordRecord), 1, fp);
    if (rc != 1) {
        printf("getWord: fread failed\n");
        return NULL;
    }

    return wr.word;
}

int insertWord(FILE *fp, char *word) {
    char convertedWord[1+MAXWORDLEN];
    int rc, i, len;
    WordRecord wr;
    FileHeader fh;

    // check for bad word
    rc = checkWord(word);
    if (rc) {
        printf("insertWord: bad word '%s'\n", word);
        return -1;
    }

    // convert to lower case
    rc = convertToLower(word, convertedWord);
    if (rc) {
        printf("insertWord: convertToLower failed\n");
        return -1;
    }

    // check for duplicate
    rc = countWords(fp, convertedWord[0], &len);
    if (rc) {
        printf("insertWord: countWords failed\n");
        return -1;
    }
    for (i=0; i<len; ++i) {
        char *w = getWord(fp, convertedWord[0], i);
        if (w == NULL) {
            printf("insertWord: getWord failed\n");
            return -1;
        }
        if (strcmp(w, convertedWord) == 0) {
            printf("insertWord: duplicate word '%s'\n", convertedWord);
            return -1;
        }
    }

    // write the word
    strcpy(wr.word, convertedWord);
    wr.nextpos = -1;
    fseek(fp, 0, SEEK_END);
    rc = fwrite(&wr, sizeof(WordRecord), 1, fp);
    if (rc != 1) {
        printf("insertWord: fwrite failed\n");
        return -1;
    }

    // update the header
    fseek(fp, 0, SEEK_SET);
    rc = fread(&fh, sizeof(FileHeader), 1, fp);
    if (rc != 1) {
        printf("insertWord: fread failed\n");
        return -1;
    }
    fh.counts[convertedWord[0] - 'a']++;
    fh.startPositions[convertedWord[0] - 'a'] = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    rc = fwrite(&fh, sizeof(FileHeader), 1, fp);
    if (rc != 1) {
        printf("insertWord: fwrite failed\n");
        return -1;
    }

    return 0;
}

int countWords(FILE *fp, char letter, int *count) {
    FileHeader fh;
    int rc;

    // check for bad letter
    if ( ! isalpha(letter) ) {
        printf("countWords: bad letter '%c'\n", letter);
        return 1;
    }

    // read the header
    fseek(fp, 0, SEEK_SET);
    rc = fread(&fh, sizeof(FileHeader), 1, fp);
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
    *count = fh.counts[letter - 'a'];
    return 0;
}
