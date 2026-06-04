#ifndef DETECTOR_H
#define DETECTOR_H

#define MAX_TEXT      10000
#define MAX_WORDS     2000
#define MAX_WORD_LEN  50
#define MAX_SENTENCES 500
#define MAX_FILLERS   20

typedef struct {
    char text[MAX_TEXT];
    char words[MAX_WORDS][MAX_WORD_LEN];
    int  word_count;
    int  sentence_count;
    int  sentence_lengths[MAX_SENTENCES];
    float ai_probability;
} TextData;

#endif
