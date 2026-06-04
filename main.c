#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>

// ─── CONSTANTS ───────────────────────────────────────────
#define MAX_TEXT       10000
#define MAX_WORDS      2000
#define MAX_WORD_LEN   50
#define MAX_SENTENCES  500
#define MAX_FILLERS    20

// ─── STRUCT ──────────────────────────────────────────────
typedef struct {
    char text[MAX_TEXT];
    char words[MAX_WORDS][MAX_WORD_LEN];
    int  word_count;
    int  sentence_count;
    int  sentence_lengths[MAX_SENTENCES];
    float ai_probability;
} TextData;

// ─── FUNCTION DECLARATIONS ───────────────────────────────
void  read_file(char *filename, TextData *data);
void  preprocess(TextData *data);
void  tokenize(TextData *data);
float sentence_variance_score(TextData *data);
float ttr_score(TextData *data);
float filler_word_score(TextData *data);
float calculate_final_score(float s1, float s2, float s3);
void  generate_report(TextData *data, float s1, float s2, float s3);

// ─── MAIN ─────────────────────────────────────────────────
int main() {
    TextData data;
    char filename[100];

    printf("===========================================\n");
    printf("   AI CONTENT DETECTION TOOL v1.0\n");
    printf("===========================================\n");
    printf("Enter filename to analyze: ");
    scanf("%s", filename);

    read_file(filename, &data);
    preprocess(&data);
    tokenize(&data);

    float s1 = sentence_variance_score(&data);
    float s2 = ttr_score(&data);
    float s3 = filler_word_score(&data);

    data.ai_probability = calculate_final_score(s1, s2, s3);
    generate_report(&data, s1, s2, s3);

    return 0;
}

// ─── READ FILE ────────────────────────────────────────────
void read_file(char *filename, TextData *data) {
    FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        printf("ERROR: Could not open file '%s'\n", filename);
        exit(1);
    }
    int i = 0;
    char ch;
    while ((ch = fgetc(fp)) != EOF && i < MAX_TEXT - 1) {
        data->text[i++] = ch;
    }
    data->text[i] = '\0';
    fclose(fp);
    printf("\n[✓] File loaded successfully. (%d characters)\n", i);
}

// ─── PREPROCESS ───────────────────────────────────────────
void preprocess(TextData *data) {
    // Convert to lowercase
    for (int i = 0; data->text[i]; i++) {
        data->text[i] = tolower(data->text[i]);
    }
    printf("[✓] Text preprocessed.\n");
}

// ─── TOKENIZE ─────────────────────────────────────────────
void tokenize(TextData *data) {
    // Count words
    data->word_count = 0;
    char temp[MAX_TEXT];
    strcpy(temp, data->text);
    char *token = strtok(temp, " \t\n\r.,!?;:");
    while (token != NULL && data->word_count < MAX_WORDS) {
        strcpy(data->words[data->word_count], token);
        data->word_count++;
        token = strtok(NULL, " \t\n\r.,!?;:");
    }

    // Count sentences
    data->sentence_count = 0;
    int len = 0;
    for (int i = 0; data->text[i]; i++) {
        len++;
        if (data->text[i] == '.' ||
            data->text[i] == '!' ||
            data->text[i] == '?') {
            if (data->sentence_count < MAX_SENTENCES) {
                data->sentence_lengths[data->sentence_count] = len;
                data->sentence_count++;
                len = 0;
            }
        }
    }
    printf("[✓] Tokenized: %d words, %d sentences.\n",
           data->word_count, data->sentence_count);
}

// ─── SIGNAL 1: SENTENCE VARIANCE ─────────────────────────
float sentence_variance_score(TextData *data) {
    if (data->sentence_count == 0) return 0.0;

    // Calculate mean
    float mean = 0;
    for (int i = 0; i < data->sentence_count; i++)
        mean += data->sentence_lengths[i];
    mean /= data->sentence_count;

    // Calculate std deviation
    float variance = 0;
    for (int i = 0; i < data->sentence_count; i++) {
        float diff = data->sentence_lengths[i] - mean;
        variance += diff * diff;
    }
    variance /= data->sentence_count;
    float std_dev = sqrt(variance);

    // Low std_dev = uniform = AI-like → high score
    // We normalize: if std_dev < 10 → very AI-like
    float score = 0.0;
    if (std_dev < 5)       score = 1.0;
    else if (std_dev < 10) score = 0.8;
    else if (std_dev < 20) score = 0.5;
    else if (std_dev < 35) score = 0.3;
    else                   score = 0.1;

    printf("[✓] Signal 1 (Sentence Variance): std_dev=%.2f score=%.2f\n",
           std_dev, score);
    return score;
}

// ─── SIGNAL 2: TYPE-TOKEN RATIO ───────────────────────────
float ttr_score(TextData *data) {
    if (data->word_count == 0) return 0.0;

    int unique = 0;
    int found  = 0;
    char seen[MAX_WORDS][MAX_WORD_LEN];

    for (int i = 0; i < data->word_count; i++) {
        found = 0;
        for (int j = 0; j < unique; j++) {
            if (strcmp(data->words[i], seen[j]) == 0) {
                found = 1;
                break;
            }
        }
        if (!found) {
            strcpy(seen[unique], data->words[i]);
            unique++;
        }
    }

    float ttr = (float)unique / data->word_count;

    // High TTR (wide vocab) = AI-like
    float score = 0.0;
    if (ttr > 0.85)      score = 1.0;
    else if (ttr > 0.70) score = 0.8;
    else if (ttr > 0.55) score = 0.6;
    else if (ttr > 0.40) score = 0.4;
    else                 score = 0.2;

    printf("[✓] Signal 2 (TTR): unique=%d total=%d ttr=%.2f score=%.2f\n",
           unique, data->word_count, ttr, score);
    return score;
}

// ─── SIGNAL 3: FILLER WORD DETECTION ─────────────────────
float filler_word_score(TextData *data) {
    char *fillers[MAX_FILLERS] = {
        "furthermore", "moreover", "additionally",
        "notably", "consequently", "nevertheless",
        "therefore", "thus", "henceforth",
        "in conclusion", "it is important",
        "it is worth", "in summary", "overall",
        "significantly", "essentially", "ultimately",
        "evidently", "clearly", "undoubtedly"
    };
    int filler_count = 0;

    for (int i = 0; i < data->word_count; i++) {
        for (int j = 0; j < MAX_FILLERS; j++) {
            if (strcmp(data->words[i], fillers[j]) == 0) {
                filler_count++;
            }
        }
    }

    float ratio = (float)filler_count / data->word_count;

    float score = 0.0;
    if (ratio > 0.05)      score = 1.0;
    else if (ratio > 0.03) score = 0.8;
    else if (ratio > 0.02) score = 0.6;
    else if (ratio > 0.01) score = 0.4;
    else                   score = 0.2;

    printf("[✓] Signal 3 (Fillers): count=%d ratio=%.3f score=%.2f\n",
           filler_count, ratio, score);
    return score;
}

// ─── FINAL SCORE ──────────────────────────────────────────
float calculate_final_score(float s1, float s2, float s3) {
    // Weights must sum to 1.0
    float w1 = 0.40; // Sentence variance (most reliable)
    float w2 = 0.30; // TTR
    float w3 = 0.30; // Filler words

    float score = (w1 * s1) + (w2 * s2) + (w3 * s3);
    return score * 100.0; // Convert to percentage
}

// ─── REPORT ───────────────────────────────────────────────
void generate_report(TextData *data, float s1, float s2, float s3) {
    printf("\n===========================================\n");
    printf("         ANALYSIS REPORT\n");
    printf("===========================================\n");
    printf("Total Words      : %d\n", data->word_count);
    printf("Total Sentences  : %d\n", data->sentence_count);
    printf("-------------------------------------------\n");
    printf("Signal 1 - Sentence Uniformity : %.2f/1.0\n", s1);
    printf("Signal 2 - Vocabulary Richness  : %.2f/1.0\n", s2);
    printf("Signal 3 - Filler Word Usage    : %.2f/1.0\n", s3);
    printf("-------------------------------------------\n");
    printf("AI Probability Score : %.1f%%\n", data->ai_probability);
    printf("-------------------------------------------\n");

    if (data->ai_probability > 70.0)
        printf("VERDICT: *** LIKELY AI-GENERATED *** \n");
    else if (data->ai_probability >= 40.0)
        printf("VERDICT:  UNCERTAIN - Manual Review Suggested\n");
    else
        printf("VERDICT: ✓ LIKELY HUMAN-WRITTEN\n");

    printf("===========================================\n");
}
