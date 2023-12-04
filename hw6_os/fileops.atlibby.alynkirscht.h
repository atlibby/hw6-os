//
// Created by Andrew Libby on 12/4/23.
//

#ifndef HW6_OS_FILEOPS_ATLIBBY_ALYNKIRSCHT_H
#define HW6_OS_FILEOPS_ATLIBBY_ALYNKIRSCHT_H

#define NUMLETTERS 26

#define MAXWORDLEN 31

typedef struct {
    long counts[NUMLETTERS];
    long startPositions[NUMLETTERS];
} FileHeader;

typedef struct {
    char word[1+MAXWORDLEN];
    long nextpos;
} WordRecord;

// return 1 if any chararacters are non-alphabetic;
// otherwise return 0
//int checkWord(char *word);

// convert ach character to its lower-case equivalent;
// leaves characters that are already lower case unchanged;
// returns zero always;
// assumes that convertedWord has sufficient space to hold
// the result
//int convertToLower(char *word, char *convertedWord);

int test_tolower(char *word);

int testUtils();

// Function will insert a word into the file and
// update the data structures in the file, return 0 if no
// error; otherwise return -1
int insertWord(FILE *fp, char *word);

// Function counts how many words are in the file that start with the specified letter. The letter parameter can be
//upper case or lower case (all of the words in the file will have been converted to lower case). Check to see
//though that letter is actually an alphabetic character—if it isn’t, then return 1. If any errors occur, return
//nonzero. Otherwise, return zero and set the count parameter to the number of words in the file that start
//with letter.
int countWords(FILE *fp, char letter, int *count);

// Function gets  a word from the file starting with the specified letter, as specified by the index parameter. The index
//parameter must be a value between 0 and numwords-1, inclusive, where numwords is the number of words
//in the file that start with the specified letter.
//The letter parameter can be upper case or lower case (all of the words in the file will have been converted
//to lower case). Check to see though that letter is actually an alphabetic character—if it isn’t, then return
//NULL.
//If index >= numwords (the number of words in the file that start with the specified letter), then return
//NULL.
char *getWord(FILE *fp, char letter, int index);

#endif //HW6_OS_FILEOPS_ATLIBBY_ALYNKIRSCHT_H
