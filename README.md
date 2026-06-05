# Logic2Impact - AI Content Detection System

## Overview
Logic2Impact is a lightweight AI content detection system built using C and Python integration. It analyzes text using multiple linguistic signals and calculates an AI probability score to classify text.

## Features
- AI vs Human text classification
- Multi-signal detection engine
- PDF report generation
- Web interface for file upload
- Weighted scoring model

## Detection Signals

### Signal 1: Sentence Length Variance
Low sentence variance may indicate AI-generated content.

### Signal 2: Type Token Ratio (TTR)
Measures vocabulary diversity.

```text
TTR = Unique Words / Total Words
```

### Signal 3: Filler Word Detection
Detects words like:
- furthermore
- moreover
- additionally

## Scoring Model

```text
Final Score =
(0.40 × S1) +
(0.30 × S2) +
(0.30 × S3)

AI Probability = Score × 100
```

## Classification

| Score | Result |
|------|------|
| >70% | Likely AI Generated |
| 40–70% | Uncertain |
| <40% | Likely Human Written |

## Tech Stack
- C
- Python
- Flask
- HTML/CSS/JavaScript

## Run Locally

```bash
pip install flask
python app.py
```

## Project Structure

```text
Logic2Impact/
├── app.py
├── main.c
├── detector.h
├── templates/
├── static/
└── uploads/
```

## Limitations
- Rule-based approach
- English text focused
- Not 100% accurate

## Contributors
Logic2Impact Project Team
