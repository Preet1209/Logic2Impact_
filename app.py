import os
import uuid
import json
import subprocess
from flask import Flask, request, render_template

app = Flask(__name__)
UPLOAD_FOLDER = 'uploads'
os.makedirs(UPLOAD_FOLDER, exist_ok=True)

@app.route('/')
def index():
    return render_template('index.html')

@app.route('/analyze', methods=['POST'])
def analyze():
    text_content = ''

    # ── Get text from upload or paste ──
    if 'file' in request.files and request.files['file'].filename != '':
        file = request.files['file']
        filename = str(uuid.uuid4()) + '.txt'
        filepath = os.path.join(UPLOAD_FOLDER, filename)
        file.save(filepath)
        with open(filepath, 'r', encoding='utf-8', errors='ignore') as f:
            text_content = f.read()

    elif 'text' in request.form and request.form['text'].strip() != '':
        text_content = request.form['text'].strip()
        filename = str(uuid.uuid4()) + '.txt'
        filepath = os.path.join(UPLOAD_FOLDER, filename)
        with open(filepath, 'w', encoding='utf-8') as f:
            f.write(text_content)

    else:
        return render_template('index.html')

    # ── Run C detector ──
    try:
        result = subprocess.run(
            ['./detector.exe', filepath],
            capture_output=True,
            text=True,
            timeout=10
        )
        output = result.stdout.strip()
        parsed = json.loads(output)

    except Exception as e:
        if os.path.exists(filepath):
            os.remove(filepath)
        return f"<h2 style='color:red;font-family:monospace'>Error running detector: {e}</h2>"

    # ── Clean up temp file ──
    if os.path.exists(filepath):
        os.remove(filepath)

    # ── Build data dict ──
    data = {
        'ai_score':            parsed.get('aiScore', 0),
        'word_count':          parsed.get('totalWords', 0),
        'sentence_count':      parsed.get('totalSentences', 0),
        'avg_sentence_length': parsed.get('avgSentenceLen', 0),
        'vocab_richness':      parsed.get('vocabRichness', 0),
        'raw_text':            text_content
    }

    return render_template('result.html', data=data)

if __name__ == '__main__':
    app.run(debug=True)
s
