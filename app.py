import os
import uuid
import json
import subprocess
import re

from flask import Flask, request, render_template
from duckduckgo_search import DDGS

app = Flask(__name__)
UPLOAD_FOLDER = 'uploads'
os.makedirs(UPLOAD_FOLDER, exist_ok=True)

# ── Filler phrases (must match result.html heatmap logic) ──────────────────
FILLER_PHRASES = [
    "in conclusion", "it is important", "it is worth noting",
    "furthermore", "moreover", "in summary", "as mentioned",
    "it should be noted", "in today's world", "in the realm of",
    "needless to say", "it goes without saying", "at the end of the day",
    "with that being said", "in the context of", "delve", "utilize",
    "leverage", "facilitate", "it is evident", "plays a crucial role",
    "it can be seen", "as we can see", "there is no doubt",
    "it is clear that"
]

def is_ai_sentence(sentence):
    """Mirror of the JS heatmap logic in result.html"""
    sentence = sentence.strip()

    if len(sentence) < 40:
        return False

    words = sentence.split()
    word_count = len(words)
    sentence_lower = sentence.lower()

    # Skip header-like lines (more than 60% capitalized words)
    capitalized = sum(1 for w in words if w and w[0].isupper())
    if word_count > 0 and (capitalized / word_count) > 0.6:
        return False

    has_filler = any(phrase in sentence_lower for phrase in FILLER_PHRASES)

    if has_filler or word_count > 40:
        return True
    return False


def search_sources(sentence, max_results=2):
    """Search DuckDuckGo for a sentence and return top URLs"""
    try:
        with DDGS() as ddgs:
            results = list(ddgs.text(
                f'"{sentence}"',
                max_results=max_results
            ))
        urls = [r['href'] for r in results if 'href' in r]
        return urls
    except Exception:
        return []


def get_ai_sentence_sources(full_text):
    """
    Split text into sentences, find AI-likely ones,
    search each, return dict { sentence: [urls] }
    """
    raw_sentences = re.split(r'(?<=[.!?])\s+', full_text.strip())
    sources = {}

    for sentence in raw_sentences:
        sentence = sentence.strip()
        if is_ai_sentence(sentence):
            urls = search_sources(sentence)
            if urls:
                sources[sentence] = urls

    return sources


@app.route('/')
def index():
    return render_template('index.html')


@app.route('/analyze', methods=['POST'])
def analyze():
    text = ''
    tmp_path = None

    try:
        # ── Get text from upload or paste ──────────────────────────────
        if 'file' in request.files and request.files['file'].filename != '':
            file = request.files['file']
            filename = file.filename.lower()

            if filename.endswith('.pdf'):
                import fitz
                file_bytes = file.read()
                doc = fitz.open(stream=file_bytes, filetype='pdf')
                text = ''.join(page.get_text() for page in doc)

            elif filename.endswith('.docx'):
                from docx import Document
                import io
                doc = Document(io.BytesIO(file.read()))
                text = '\n'.join(p.text for p in doc.paragraphs)

            else:
                text = file.read().decode('utf-8', errors='ignore')

        elif 'text' in request.form and request.form['text'].strip():
            text = request.form['text']

        else:
            return "No input provided.", 400

        # ── No truncation — full document is processed ─────────────────
        full_text    = text
        tmp_filename = f"{uuid.uuid4().hex}.txt"
        tmp_path     = os.path.join(UPLOAD_FOLDER, tmp_filename)

        with open(tmp_path, 'w', encoding='utf-8') as f:
            f.write(full_text)

        # ── Run C detector ─────────────────────────────────────────────
        result = subprocess.run(
            ['./detector.exe', tmp_path],
            capture_output=True, text=True, timeout=60
        )
        output = result.stdout.strip()
        parsed = json.loads(output)

        # ── Build data dict ────────────────────────────────────────────
        data = {
            'ai_score':            parsed.get('aiScore', 0),
            'word_count':          parsed.get('totalWords', 0),
            'sentence_count':      parsed.get('totalSentences', 0),
            'avg_sentence_length': parsed.get('avgSentenceLen', 0),
            'vocab_richness':      parsed.get('vocabRichness', 0),
            'raw_text':            full_text
        }

        # ── Search sources for AI-flagged sentences ────────────────────
        sources      = get_ai_sentence_sources(full_text)
        sources_json = json.dumps(sources)

        return render_template('result.html', data=data, sources_json=sources_json)

    except json.JSONDecodeError:
        return "Error parsing detector output.", 500
    except subprocess.TimeoutExpired:
        return "Detector timed out — document may be too large.", 500
    except Exception as e:
        return f"Error: {str(e)}", 500

    finally:
        if tmp_path and os.path.exists(tmp_path):
            os.remove(tmp_path)


if __name__ == '__main__':
    app.run(debug=True)
