#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#define MAX_TEXT       500000
#define MAX_WORD       100
#define MAX_WORDS_LIST 5000

// ═══════════════════════════════════════════
//  FUNCTION 1: Count Sentences
// ═══════════════════════════════════════════
int countSentences(char *text) {
    int count = 0;
    for (int i = 0; text[i]; i++)
        if (text[i] == '.' || text[i] == '!' || text[i] == '?') count++;
    return count == 0 ? 1 : count;
}

// ═══════════════════════════════════════════
//  FUNCTION 2: Count Words
// ═══════════════════════════════════════════
int countWords(char *text) {
    int count = 0, inWord = 0;
    for (int i = 0; text[i]; i++) {
        if (isspace((unsigned char)text[i])) inWord = 0;
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

    // Use heap allocation to avoid stack overflow with large text
    char *temp = (char *)malloc(MAX_TEXT);
    if (!temp) return 0;

    strncpy(temp, text, MAX_TEXT - 1);
    temp[MAX_TEXT - 1] = '\0';

    char *token = strtok(temp, " ,.!?;\n\t\"'()[]{}:/-");
    while (token != NULL) {
        for (int i = 0; token[i]; i++)
            token[i] = tolower((unsigned char)token[i]);

        if (strlen(token) < 2) {
            token = strtok(NULL, " ,.!?;\n\t\"'()[]{}:/-");
            continue;
        }

        int found = 0;
        for (int i = 0; i < total; i++)
            if (strcmp(words[i], token) == 0) { found = 1; break; }

        if (!found && total < MAX_WORDS_LIST) {
            strncpy(words[total], token, MAX_WORD - 1);
            words[total][MAX_WORD - 1] = '\0';
            total++;
        }
        token = strtok(NULL, " ,.!?;\n\t\"'()[]{}:/-");
    }

    free(temp);
    return total;
}

// ═══════════════════════════════════════════
//  FUNCTION 4: Count AI Filler Words
// ═══════════════════════════════════════════
int countFillerWords(char *text) {
    char *fillers[] = {
        "furthermore", "additionally", "notably", "therefore",
        "thus", "hence", "however", "moreover", "in conclusion",
        "it is important", "it is worth", "one must",
        "it should be noted", "in summary", "to summarize",
        "as a result", "consequently", "in addition",
        "this ensures", "plays a crucial", "plays a vital",
        "pivotal", "delve", "utilizes", "leverages",
        "facilitate", "demonstrate", "comprehensively", "significantly",
        "it is essential", "it is clear", "it can be seen",
        "in this context", "with respect to", "in terms of",
        "it is noteworthy", "it is evident", "overall",
        "underscore", "paramount", "intricate", "multifaceted",
        "robust", "seamlessly", "streamline", "harness",
        "cutting-edge", "state-of-the-art", "groundbreaking"
    };
    int fillerCount = sizeof(fillers) / sizeof(fillers[0]);
    int count = 0;

    // Use heap allocation to avoid stack overflow with large text
    char *temp = (char *)malloc(MAX_TEXT);
    if (!temp) return 0;

    strncpy(temp, text, MAX_TEXT - 1);
    temp[MAX_TEXT - 1] = '\0';

    for (int i = 0; temp[i]; i++)
        temp[i] = tolower((unsigned char)temp[i]);

    for (int i = 0; i < fillerCount; i++) {
        char *pos = temp;
        while ((pos = strstr(pos, fillers[i])) != NULL) { count++; pos++; }
    }

    free(temp);
    return count;
}

// ═══════════════════════════════════════════
//  FUNCTION 5: Measure Sentence Uniformity
//  (AI tends to write very uniform sentence lengths)
// ═══════════════════════════════════════════
float measureSentenceUniformity(char *text) {
    int lengths[5000];
    int sentCount = 0;
    int wordCount = 0;

    for (int i = 0; text[i] && sentCount < 4999; i++) {
        if (!isspace((unsigned char)text[i]) &&
            text[i] != '.' && text[i] != '!' && text[i] != '?')
            wordCount++;
        if (text[i] == '.' || text[i] == '!' || text[i] == '?') {
            if (wordCount > 0) {
                lengths[sentCount++] = wordCount;
                wordCount = 0;
            }
        }
    }

    if (sentCount < 2) return 0.0f;

    // Calculate mean
    float mean = 0;
    for (int i = 0; i < sentCount; i++) mean += lengths[i];
    mean /= sentCount;

    // Calculate std deviation
    float variance = 0;
    for (int i = 0; i < sentCount; i++) {
        float diff = lengths[i] - mean;
        variance += diff * diff;
    }
    variance /= sentCount;
    float stddev = (float)sqrt((double)variance);

    // Low stddev = high uniformity = AI-like
    float uniformity = 100.0f - (stddev * 10.0f);
    if (uniformity < 0)   uniformity = 0;
    if (uniformity > 100) uniformity = 100;
    return uniformity;
}

// ═══════════════════════════════════════════
//  FUNCTION 6: Detect Passive Voice Patterns
// ═══════════════════════════════════════════
int countPassiveVoice(char *text) {
    char *passiveMarkers[] = {
        "is used", "are used", "was used", "were used",
        "is known", "are known", "is considered", "are considered",
        "is defined", "is shown", "is found", "is seen",
        "can be", "may be", "must be", "should be",
        "is provided", "are provided", "is given", "is made"
    };
    int markerCount = sizeof(passiveMarkers) / sizeof(passiveMarkers[0]);
    int count = 0;

    // Use heap allocation to avoid stack overflow with large text
    char *temp = (char *)malloc(MAX_TEXT);
    if (!temp) return 0;

    strncpy(temp, text, MAX_TEXT - 1);
    temp[MAX_TEXT - 1] = '\0';

    for (int i = 0; temp[i]; i++)
        temp[i] = tolower((unsigned char)temp[i]);

    for (int i = 0; i < markerCount; i++) {
        char *pos = temp;
        while ((pos = strstr(pos, passiveMarkers[i])) != NULL) { count++; pos++; }
    }

    free(temp);
    return count;
}

// ═══════════════════════════════════════════
//  FUNCTION 7: Score Text & Output JSON
// ═══════════════════════════════════════════
void analyzeAndPrintJSON(char *text) {
    int   totalSentences = countSentences(text);
    int   totalWords     = countWords(text);
    int   uniqueWords    = countUniqueWords(text);
    int   fillerWords    = countFillerWords(text);
    int   passiveCount   = countPassiveVoice(text);
    float uniformity     = measureSentenceUniformity(text);

    float avgSentenceLen = (float)totalWords / totalSentences;
    float vocabRichness  = (float)uniqueWords / totalWords * 100.0f;
    float fillerRatio    = (float)fillerWords  / totalWords * 100.0f;

    float score = 0;

    // ── Signal 1: Sentence length (AI: 15–22 words avg)
    float sentScore = 0;
    if      (avgSentenceLen >= 15 && avgSentenceLen <= 22) sentScore = 25;
    else if (avgSentenceLen > 22  && avgSentenceLen <= 30) sentScore = 15;
    else if (avgSentenceLen > 30)                          sentScore = 5;
    else                                                   sentScore = 0;
    score += sentScore;

    // ── Signal 2: Vocabulary richness
    float vocabScore = 0;
    if      (vocabRichness >= 55 && vocabRichness < 75) vocabScore = 20;
    else if (vocabRichness < 55)                        vocabScore = 10;
    else                                                vocabScore = 5;
    score += vocabScore;

    // ── Signal 3: AI filler words (strongest signal)
    float fillerScore = 0;
    if      (fillerWords >= 5) fillerScore = 35;
    else if (fillerWords == 4) fillerScore = 28;
    else if (fillerWords == 3) fillerScore = 20;
    else if (fillerWords == 2) fillerScore = 12;
    else if (fillerWords == 1) fillerScore = 6;
    else                       fillerScore = 0;
    score += fillerScore;

    // ── Signal 4: Sentence uniformity
    float uniformScore = 0;
    if      (uniformity >= 70) uniformScore = 15;
    else if (uniformity >= 40) uniformScore = 8;
    else                       uniformScore = 0;
    score += uniformScore;

    // ── Signal 5: Passive voice usage
    float passiveScore = 0;
    if      (passiveCount >= 4) passiveScore = 5;
    else if (passiveCount >= 2) passiveScore = 2;
    else                        passiveScore = 0;
    score += passiveScore;

    if (score < 0)   score = 0;
    if (score > 100) score = 100;

    char verdict[50];
    if      (score >= 60) strncpy(verdict, "LIKELY AI-GENERATED",  49);
    else if (score >= 35) strncpy(verdict, "UNCERTAIN",            49);
    else                  strncpy(verdict, "LIKELY HUMAN-WRITTEN", 49);
    verdict[49] = '\0';

    printf("{\n");
    printf("  \"totalWords\": %d,\n",       totalWords);
    printf("  \"totalSentences\": %d,\n",   totalSentences);
    printf("  \"uniqueWords\": %d,\n",      uniqueWords);
    printf("  \"fillerWords\": %d,\n",      fillerWords);
    printf("  \"avgSentenceLen\": %.2f,\n", avgSentenceLen);
    printf("  \"vocabRichness\": %.2f,\n",  vocabRichness);
    printf("  \"fillerRatio\": %.2f,\n",    fillerRatio);
    printf("  \"sentScore\": %.0f,\n",      sentScore);
    printf("  \"vocabScore\": %.0f,\n",     vocabScore);
    printf("  \"fillerScore\": %.0f,\n",    fillerScore);
    printf("  \"uniformScore\": %.0f,\n",   uniformScore);
    printf("  \"passiveScore\": %.0f,\n",   passiveScore);
    printf("  \"uniformity\": %.2f,\n",     uniformity);
    printf("  \"passiveCount\": %d,\n",     passiveCount);
    printf("  \"aiScore\": %.1f,\n",        score);
    printf("  \"verdict\": \"%s\"\n",       verdict);
    printf("}\n");
}

// ═══════════════════════════════════════════
//  MAIN
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

    // ── Heap allocate large buffer (500k on stack would crash) ────────
    char *text = (char *)malloc(MAX_TEXT);
    if (!text) {
        printf("{\"error\": \"Memory allocation failed\"}\n");
        fclose(fp);
        return 1;
    }

    int i = 0;
    int ch;
    while ((ch = fgetc(fp)) != EOF && i < MAX_TEXT - 1)
        text[i++] = (char)ch;
    text[i] = '\0';
    fclose(fp);

    analyzeAndPrintJSON(text);

    free(text);
    return 0;
}
