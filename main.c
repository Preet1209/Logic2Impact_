#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_TEXT 10000
#define MAX_WORD 100
#define MAX_WORDS_LIST 500

// ═══════════════════════════════════════════
//  FUNCTION 1: Count Sentences
// ═══════════════════════════════════════════
int countSentences(char *text) {
    int count = 0;
    for (int i = 0; text[i]; i++)
        if (text[i]=='.' || text[i]=='!' || text[i]=='?') count++;
    return count == 0 ? 1 : count;
}

// ═══════════════════════════════════════════
//  FUNCTION 2: Count Words
// ═══════════════════════════════════════════
int countWords(char *text) {
    int count = 0, inWord = 0;
    for (int i = 0; text[i]; i++) {
        if (isspace(text[i])) inWord = 0;
        else if (!inWord) { inWord = 1; count++; }
    }
    return count == 0 ? 1 : count;
}

// ═══════════════════════════════════════════
//  FUNCTION 3: Count Unique Words
// ═══════════════════════════════════════════
int countUniqueWords(char *text) {
    char words[MAX_WORDS_LIST][MAX_WORD];
    int total = 0;
    char temp[MAX_TEXT];
    strncpy(temp, text, MAX_TEXT - 1);
    char *token = strtok(temp, " ,.!?;\n\t\"'()[]");
    while (token != NULL) {
        for (int i = 0; token[i]; i++) token[i] = tolower(token[i]);
        if (strlen(token) == 0) { token = strtok(NULL, " ,.!?;\n\t\"'()[]"); continue; }
        int found = 0;
        for (int i = 0; i < total; i++)
            if (strcmp(words[i], token) == 0) { found = 1; break; }
        if (!found && total < MAX_WORDS_LIST) {
            strncpy(words[total], token, MAX_WORD - 1);
            total++;
        }
        token = strtok(NULL, " ,.!?;\n\t\"'()[]");
    }
    return total;
}

// ═══════════════════════════════════════════
//  FUNCTION 4: Count AI Filler Words
// ═══════════════════════════════════════════
int countFillerWords(char *text) {
    char *fillers[] = {
        "furthermore","additionally","notably","therefore",
        "thus","hence","however","moreover","in conclusion",
        "it is important","it is worth","one must",
        "it should be noted","in summary","to summarize",
        "as a result","consequently","in addition",
        "this ensures","plays a crucial","plays a vital",
        "pivotal","delve","utilizes","leverages",
        "facilitate","demonstrate","comprehensively","significantly"
    };
    int fillerCount = sizeof(fillers) / sizeof(fillers[0]);
    int count = 0;
    char temp[MAX_TEXT];
    strncpy(temp, text, MAX_TEXT - 1);
    for (int i = 0; temp[i]; i++) temp[i] = tolower(temp[i]);
    for (int i = 0; i < fillerCount; i++) {
        char *pos = temp;
        while ((pos = strstr(pos, fillers[i])) != NULL) { count++; pos++; }
    }
    return count;
}

// ═══════════════════════════════════════════
//  FUNCTION 5: Score Text & Output JSON
// ═══════════════════════════════════════════
void analyzeAndPrintJSON(char *text) {
    int totalSentences = countSentences(text);
    int totalWords     = countWords(text);
    int uniqueWords    = countUniqueWords(text);
    int fillerWords    = countFillerWords(text);

    float avgSentenceLen = (float)totalWords / totalSentences;
    float vocabRichness  = (float)uniqueWords / totalWords * 100;
    float fillerRatio    = (float)fillerWords / totalWords * 100;

    float score = 0;

    // Signal 1: Sentence length
    float sentScore = 0;
    if (avgSentenceLen >= 15 && avgSentenceLen <= 25) sentScore = 35;
    else if (avgSentenceLen > 25) sentScore = 20;
    else sentScore = 5;
    score += sentScore;

    // Signal 2: Vocabulary richness
    float vocabScore = 0;
    if (vocabRichness < 60)      vocabScore = 30;
    else if (vocabRichness < 70) vocabScore = 15;
    else                         vocabScore = -10;
    score += vocabScore;

    // Signal 3: Filler words
    float fillerScore = 0;
    if      (fillerWords >= 3) fillerScore = 35;
    else if (fillerWords == 2) fillerScore = 20;
    else if (fillerWords == 1) fillerScore = 10;
    else                       fillerScore = -15;
    score += fillerScore;

    if (score < 0)   score = 0;
    if (score > 100) score = 100;

    char verdict[50];
    if      (score >= 60) strncpy(verdict, "LIKELY AI-GENERATED",  49);
    else if (score <= 35) strncpy(verdict, "LIKELY HUMAN-WRITTEN", 49);
    else                  strncpy(verdict, "UNCERTAIN",            49);

    // Output clean JSON
    printf("{\n");
    printf("  \"totalWords\": %d,\n",        totalWords);
    printf("  \"totalSentences\": %d,\n",    totalSentences);
    printf("  \"uniqueWords\": %d,\n",       uniqueWords);
    printf("  \"fillerWords\": %d,\n",       fillerWords);
    printf("  \"avgSentenceLen\": %.2f,\n",  avgSentenceLen);
    printf("  \"vocabRichness\": %.2f,\n",   vocabRichness);
    printf("  \"fillerRatio\": %.2f,\n",     fillerRatio);
    printf("  \"sentScore\": %.0f,\n",       sentScore < 0 ? 0 : sentScore);
    printf("  \"vocabScore\": %.0f,\n",      vocabScore < 0 ? 0 : vocabScore);
    printf("  \"fillerScore\": %.0f,\n",     fillerScore < 0 ? 0 : fillerScore);
    printf("  \"aiScore\": %.1f,\n",         score);
    printf("  \"verdict\": \"%s\"\n",        verdict);
    printf("}\n");
}

// ═══════════════════════════════════════════
//  MAIN — Reads file path from argument
// ═══════════════════════════════════════════
int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("{\"error\": \"No file provided\"}\n");
        return 1;
    }

    FILE *fp = fopen(argv[1], "r");
    if (!fp) {
        printf("{\"error\": \"Cannot open file\"}\n");
        return 1;
    }

    char text[MAX_TEXT];
    int i = 0; char ch;
    while ((ch = fgetc(fp)) != EOF && i < MAX_TEXT - 1) text[i++] = ch;
    text[i] = '\0';
    fclose(fp);

    analyzeAndPrintJSON(text);
    return 0;
}