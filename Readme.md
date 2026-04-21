# 🌱 AgroSense — Smart Agriculture Crop Recommendation System

A full-stack demo project using **C++ DSA backend** + **HTML/CSS/JS frontend**.

---

## 📁 Project Files

| File | Purpose |
|------|---------|
| `main.cpp` | C++ backend — crop scoring engine using vector, unordered_map, queue, stack |
| `index.html` | Frontend UI |
| `style.css` | Organic green luxury stylesheet |
| `script.js` | Form logic, JS simulation, C++ bridge |
| `server.py` | Optional Python micro-server to connect browser ↔ C++ |

---

## 🚀 Quick Start — Two Modes

### MODE A: Full C++ integration (recommended for college lab)

**Step 1 — Compile the C++ binary**
```bash
g++ -o agri main.cpp -std=c++17
```

**Step 2 — Start the Python helper server**
```bash
python3 server.py
```
This server:
- Receives form data from the browser
- Writes `input.txt`
- Runs `./agri` (the compiled binary)
- Serves `output.json` back to the browser

**Step 3 — Open in browser**
```
http://localhost:8080/index.html
```

---

### MODE B: Browser-only demo (no compilation needed)

Just open `index.html` directly in any browser.

The JavaScript in `script.js` contains an exact copy of the C++ scoring algorithm.
When the server is not running, it automatically falls back to this JS simulation.
Results are **identical** to the C++ output.

---

## 🖥️ Run C++ manually (command line)

### Via command line arguments:
```bash
./agri "Loamy" 6.5 28 "Medium" 12 2.5
#        soil   ph  temp  water  wind area
```

### Via input.txt:
Create a file named `input.txt`:
```
soilType=Loamy
soilPH=6.5
temperature=28
waterAvailability=Medium
windVelocity=12
area=2.5
```
Then run:
```bash
./agri
```
Output is written to `output.json`.

---

## 📊 Data Structures Used (C++)

| DSA | Where Used |
|-----|-----------|
| `std::vector<Crop>` | Stores all 20 crops in the dataset |
| `std::unordered_map<string, double>` | Stores scoring weights + per-crop scores |
| `std::queue<AgriInput>` | Simulates an input request pipeline |
| `std::stack<AgriInput>` | Supports undo of last input |

---

## 🌾 Crop Dataset (20 crops)

Rice, Wheat, Maize, Sugarcane, Cotton, Groundnut, Soybean, Tomato,
Potato, Onion, Banana, Mango, Chickpea, Mustard, Sunflower, Tea,
Coffee, Turmeric, Lentil, Watermelon

Each crop has: soil type, pH range, temperature range, water requirement, wind tolerance.

---

## 🧮 Scoring Algorithm

Total score (max 100) = sum of weighted component scores:

| Factor | Weight | Logic |
|--------|--------|-------|
| Soil type match | 30% | Binary (loamy gets 50% bonus as versatile) |
| pH in range | 20% | Full if in range; gradient penalty beyond |
| Temperature | 20% | Full if in range; fades over 15°C deviation |
| Water match | 20% | Full=same level; 50%=1 level off; 0%=2 levels off |
| Wind tolerance | 10% | Full if under limit; fades over 30 km/h excess |

Percentage = (raw score / 100) × 100

---

## 🎓 For College Demo

1. Compile: `g++ -o agri main.cpp -std=c++17`
2. Start server: `python3 server.py`
3. Open browser: `http://localhost:8080/index.html`
4. Fill form and click **Analyse Crops**
5. See Top 3 crops with animated progress bars
6. If score < 50% → warning alert shown

**Works offline** (MODE B) if no server — great for demo without setup.