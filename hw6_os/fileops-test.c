// jdh CS3010 Fall 2023
//
// od commands are:
// od -Ad -t u8 -v *.dat
// od -Ad -c -v *.dat

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "fileops.atlibby.alynkirscht.h"

// header will have 26 longs + 26 longs (one for each letter)

int checkWord(char *);
int convertToLower(char *, char *);

//---------------------------------------------------------------
// - write "Nineveh" to the file (as "nineveh")
// - write "kerfuffle" to the file
// - attempt to write "No Can Do" to the file; this should fail
// - write "nincompoop" to the file
// - then check

int writeTestOne(char *filename, char **existingWords[], int existingCounts[], int newfileFlag) {
    char buf[1+MAXWORDLEN];
    int rc, i, j, numWords, fileExists;
    FILE *fp;

    char *empty[] = {""};
    char *ns[] = {"Nineveh", "No Can Do", "nincompoop", ""};
    char *ks[] = {"kerfuffle", ""};
    char **newWords[] = { empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, ks, empty, empty, ns, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty};
    int newCounts[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

    // if newFileFlag is set, the rewrite (open and delete) the file
    // if newFIleFlag is not set, then try to open the file in append mode

    fileExists = 0;

    if ( ! newfileFlag ) {
        fp = (FILE *) fopen(filename, "r+");
        if (fp != NULL)
            fileExists = 1;
    }

    if ( newfileFlag || ! fileExists ) {
        fp = (FILE *) fopen(filename, "w+");
        if (fp == NULL) {
            printf("cannot open file '%s'\n", filename);
            return 8;
        }
    }

    // insert n and k, alternating
    int flag, nidx, kidx;
    flag = 0;
    nidx = 0;
    kidx = 0;
    numWords = sizeof(ns) / sizeof(char *);
    numWords = numWords + sizeof(ks) / sizeof(char *);
    numWords = numWords - 2; // the "" at the end of each list
    for (i=0; i<numWords; ++i) {
        if ( ! flag ) {
            flag = 1;
            if (nidx < (sizeof(ns) / sizeof(char *)) - 1 ) {
                strcpy(buf, ns[nidx]);
                ++nidx;
            } else {
                strcpy(buf, ks[kidx]);
                ++kidx;
            }
        } else {
            flag = 0;
            if (kidx < (sizeof(ks) / sizeof(char *)) - 1 ) {
                strcpy(buf, ks[kidx]);
                ++kidx;
            } else {
                strcpy(buf, ns[nidx]);
                ++nidx;
            }
        }

        printf("insert '%s'\n", buf);
        rc = insertWord(fp, buf);
        printf("rc = %d\n", rc);
    } // for

    // add in the existing
    for (i=0; i<NUMLETTERS; ++i) {
        existingCounts[i] = newCounts[i] + existingCounts[i];
        int numWords_1 = 0;
        while (strlen(newWords[i][numWords_1]) > 0)
            numWords_1 = numWords_1 + 1;
        // printf("*** new %c has %d words\n", (i+'a'), numWords_1);

        int numWords_2 = 0;
        while (strlen(existingWords[i][numWords_2]) > 0)
            numWords_2 = numWords_2 + 1;
        // printf("*** existing %c has %d words\n", (i+'a'), numWords_2);
        int total = numWords_1 + numWords_2;
        char **w = (char **) malloc((total + 1) * sizeof(char *));
        for (j=0; j<total; ++j) {
            w[j] = malloc(1 + MAXWORDLEN);
            if (j < numWords_1)
                strcpy(w[j], newWords[i][j]);
            else
                strcpy(w[j], existingWords[i][j-numWords_1]);
        }
        w[total] = malloc(4); // big enough to hold terminating ""
        strcpy(w[total], "");
        existingWords[i] = w;
    }

    fclose(fp);
    return 0;
} // writeTestOne()

//---------------------------------------------------

int checkFile(char *filename, char **existingWords[], int existingCounts[]) {
    int i, j, rc, numWords;
    char convertedWord[1+MAXWORDLEN];
    FILE *fp;
    char *word;

    // open the file for read
    fp = (FILE *) fopen(filename, "r");
    if (fp == NULL) {
        printf("ERROR in checkFile(): cannot open '%s' for read\n", filename);
        return 8;
    }

    for (char ch='a'; ch<='z'; ++ch) {
        int charIdx = ch - 'a';
        int numGood = 0;
        j = 0;
        while (j < existingCounts[charIdx]) {
            if (checkWord(existingWords[charIdx][j]) == 0)
                ++numGood;
            j = j + 1;
        }
        rc = countWords(fp, ch, &numWords);
        if (rc != 0) {
            printf("ERROR %d from countWords() for '%c' in checkFile()\n", rc, ch);
            fclose(fp);
            return 8;
        }

        if (numWords != numGood) {
            printf("ERROR: expect %d from countWords() for '%c'; got %d in checkFile()\n", numGood, ch, numWords);
            fclose(fp);
            return 8;
        }

        int wordIdx = 0;
        for (j=0; j<existingCounts[charIdx]; ++j) {
            if (checkWord(existingWords[charIdx][j]) == 0) {
                convertToLower(existingWords[charIdx][j], convertedWord);
                word = getWord(fp, ch, wordIdx);
                if (word != NULL) {
                    printf("words[%d] = '%s'\n", wordIdx, word);
                    if ( strcmp(word, convertedWord) ) {
                        printf("ERROR: expect '%s' from getWord(); got '%s' in checkFile()\n", convertedWord, word);
                        fclose(fp);
                        return 8;
                    }
                    free(word);
                } else {
                    printf("ERROR: expect '%s' from getWord(); got NULL in checkFile()\n", convertedWord);
                    fclose(fp);
                    return 8;
                }
                ++wordIdx;
            } else {
                ; // skip this word: checkWord() returned zero
            }
        } // retrieved word is not NULL
    } // for

    fclose(fp);

    printf("PASS checkFile()\n");
    return 0;
} // checkFile()

//---------------------------------------------------------------

int listAll(FILE *fp) {
    char *word;
    char c;
    int i, rc, numWords;

    for (c='a'; c<='z'; ++c) {
        rc = countWords(fp, c, &numWords);
        if (rc != 0) {
            printf("ERROR %d from countWords() for '%c'\n", rc, c);
            return 8;
        }

        if (numWords > 0) {
            printf("words starting with %c (%d words):\n", c, numWords);
            for (i=0; i<numWords; ++i) {
                word = getWord(fp, c, i);
                if (word != NULL) {
                    printf("words[%d] = '%s'\n", i, word);
                    free(word);
                } else {
                    printf("error: unexpected NULL\n");
                }
            }
        }
    }
    return 0;
} // listAll()

//---------------------------------------------------------------

int main() {
    char *filename = "testfile.dat";
    int i, rc, newfileFlag;
    FILE *fp;
    char buf[1 + MAXWORDLEN];

    char *empty[] = {""};
    char **existingWords[] = { empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty};
    int existingCounts[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

    newfileFlag = 1;
    rc = writeTestOne(filename, existingWords, existingCounts, newfileFlag);
    if (rc != 0) {
        printf("writeTestOne() failed\n");
        return 8;
    }

    rc = checkFile(filename, existingWords, existingCounts);
    if (rc != 0) {
        printf("checkFile() failed\n");
        return 0;
    }

    // open the file for append
    fp = (FILE *) fopen(filename, "r+");
    if (fp == NULL) {
        printf("UNEXPECTED ERROR: cannot open '%s' for append\n", filename);
        return 8;
    }

    printf("\n");
    printf("---------------------------------------------------\n");
    printf("here are the words in the file after writeTestOne()\n");
    listAll(fp);
    printf("---------------------------------------------------\n");
    printf("\n");

    char *as[] = {"acacia", "avatar", ""};

    strcpy(buf, as[0]);
    rc = insertWord(fp, buf);
    if (rc != 0) {
        printf("UNEXPECTED ERROR: cannot write '%s' to file\n", buf);
        fclose(fp);
        return 0;
    }

    strcpy(buf, as[1]);
    rc = insertWord(fp, buf);
    if (rc != 0) {
        printf("UNEXPECTED ERROR: cannot write '%s' to file\n", buf);
        fclose(fp);
        return 0;
    }

    printf("\n");
    printf("-----------------------------------------------------------\n");
    printf("here are the words in the file after writing two more words\n");
    listAll(fp);
    printf("-----------------------------------------------------------\n");
    printf("\n");

    fclose(fp);

    existingWords[0] = as;
    existingCounts[0] = 2;

    rc = checkFile(filename, existingWords, existingCounts);
    if (rc != 0) {
        printf("checkFile() failed\n");
        return 0;
    }

    newfileFlag = 0;
    rc = writeTestOne(filename, existingWords, existingCounts, newfileFlag);
    if (rc != 0) {
        printf("writeTestOne() failed\n");
        return 8;
    }

    rc = checkFile(filename, existingWords, existingCounts);
    if (rc != 0) {
        printf("checkFile() failed\n");
        return 0;
    }
    return 0;
} // main()


