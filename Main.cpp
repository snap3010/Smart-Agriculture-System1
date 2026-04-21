/*
 * Smart Agriculture Crop Recommendation System
 * Backend: Pure C++ (no external libraries)
 * DSA Used: vector, unordered_map, queue, stack
 * Compile: g++ -o agri main.cpp -std=c++17
 * Run:     ./agri "Sandy" 6.5 28 "Medium" 12 2.5
 *      OR  ./agri  (reads from input.txt)
 */

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <queue>
#include <stack>
#include <algorithm>
#include <cmath>
#include <string>
#include <iomanip>

// ─────────────────────────────────────────────
//  DATA MODEL
// ─────────────────────────────────────────────
struct Crop {
    std::string name;
    std::string soilType;     // Loamy, Sandy, Clay, Silty, Peaty
    double phMin, phMax;
    double tempMin, tempMax;
    std::string waterReq;     // Low, Medium, High
    double windTolerance;     // max km/h it can handle well
    std::string emoji;
    std::string description;
};

// Input request bundled together
struct AgriInput {
    std::string soilType;
    double soilPH;
    double temperature;
    std::string waterAvailability;
    double windVelocity;
    double area;
};

struct CropResult {
    std::string name;
    double score;
    double percentage;
    std::string emoji;
    std::string description;
};

// ─────────────────────────────────────────────
//  CROP DATASET  (20 crops)
// ─────────────────────────────────────────────
std::vector<Crop> buildCropDatabase() {
    return {
        {"Rice",        "Clay",   5.5, 7.0,  20, 35, "High",   20, "🌾", "Staple grain; thrives in waterlogged paddy fields."},
        {"Wheat",       "Loamy",  6.0, 7.5,  15, 25, "Medium", 30, "🌿", "Cool-season cereal; ideal for roti and bread production."},
        {"Maize",       "Sandy",  5.8, 7.0,  18, 35, "Medium", 40, "🌽", "Versatile cereal used for food, feed and biofuel."},
        {"Sugarcane",   "Loamy",  6.0, 8.0,  20, 40, "High",   25, "🎋", "High-yield sugar crop requiring hot & humid climate."},
        {"Cotton",      "Sandy",  6.0, 8.0,  25, 40, "Low",    35, "🌸", "Cash crop suited for dry, warm semi-arid regions."},
        {"Groundnut",   "Sandy",  6.0, 7.0,  25, 35, "Low",    30, "🥜", "Legume rich in protein; fixes atmospheric nitrogen."},
        {"Soybean",     "Loamy",  6.0, 7.0,  20, 30, "Medium", 25, "🫘", "Protein powerhouse; excellent nitrogen-fixing legume."},
        {"Tomato",      "Loamy",  6.0, 7.0,  18, 30, "Medium", 20, "🍅", "Popular vegetable; needs well-drained fertile soil."},
        {"Potato",      "Sandy",  4.8, 6.0,  15, 25, "Medium", 20, "🥔", "Cool-season tuber crop with high starch content."},
        {"Onion",       "Loamy",  6.0, 7.5,  13, 30, "Medium", 20, "🧅", "Bulb vegetable grown widely as culinary staple."},
        {"Banana",      "Loamy",  5.5, 7.0,  22, 38, "High",   20, "🍌", "Tropical fruit needing warm humid conditions."},
        {"Mango",       "Sandy",  5.5, 7.5,  24, 42, "Low",    25, "🥭", "King of fruits; thrives in hot tropical climate."},
        {"Chickpea",    "Sandy",  6.0, 9.0,  15, 30, "Low",    30, "🫛", "Rabi legume with high drought tolerance."},
        {"Mustard",     "Loamy",  6.0, 7.5,  10, 25, "Low",    35, "🌼", "Oil-seed crop suited for cool winters."},
        {"Sunflower",   "Loamy",  6.5, 8.0,  20, 35, "Low",    45, "🌻", "Drought-tolerant oilseed with high wind tolerance."},
        {"Tea",         "Peaty",  4.5, 5.5,  15, 28, "High",   15, "🍃", "Perennial shrub preferring acidic hilly soil."},
        {"Coffee",      "Loamy",  6.0, 6.5,  15, 28, "Medium", 10, "☕", "Shade-loving tropical crop for highland regions."},
        {"Turmeric",    "Clay",   5.5, 7.0,  20, 35, "High",   15, "🌿", "Spice crop needing warm humid shaded conditions."},
        {"Lentil",      "Sandy",  6.0, 8.0,  15, 25, "Low",    25, "🫘", "Cool-season pulse; excellent source of protein."},
        {"Watermelon",  "Sandy",  6.0, 7.0,  25, 40, "Low",    30, "🍉", "Warm season fruit loving sandy, well-drained soil."},
    };
}

// ─────────────────────────────────────────────
//  SCORING ENGINE  (using unordered_map)
// ─────────────────────────────────────────────
double scoreCrop(const Crop& crop, const AgriInput& input) {
    // Weight map stored in unordered_map
    std::unordered_map<std::string, double> weights = {
        {"soil",  30.0},
        {"ph",    20.0},
        {"temp",  20.0},
        {"water", 20.0},
        {"wind",  10.0}
    };

    double total = 0.0;

    // 1. Soil match (binary but case-insensitive)
    std::string cropSoil = crop.soilType;
    std::string inSoil   = input.soilType;
    std::transform(cropSoil.begin(), cropSoil.end(), cropSoil.begin(), ::tolower);
    std::transform(inSoil.begin(),   inSoil.end(),   inSoil.begin(),   ::tolower);
    if (cropSoil == inSoil) total += weights["soil"];
    else if (cropSoil == "loamy" || inSoil == "loamy") total += weights["soil"] * 0.5; // loamy is versatile

    // 2. pH range match (gradient)
    if (input.soilPH >= crop.phMin && input.soilPH <= crop.phMax) {
        total += weights["ph"];
    } else {
        double dist = std::min(std::abs(input.soilPH - crop.phMin),
                               std::abs(input.soilPH - crop.phMax));
        double penalty = dist / 2.0; // 2 pH units = 0 score
        double phScore = std::max(0.0, weights["ph"] * (1.0 - penalty));
        total += phScore;
    }

    // 3. Temperature closeness (gradient)
    if (input.temperature >= crop.tempMin && input.temperature <= crop.tempMax) {
        total += weights["temp"];
    } else {
        double dist = std::min(std::abs(input.temperature - crop.tempMin),
                               std::abs(input.temperature - crop.tempMax));
        double penalty = dist / 15.0; // 15°C away = 0 score
        double tempScore = std::max(0.0, weights["temp"] * (1.0 - penalty));
        total += tempScore;
    }

    // 4. Water availability match
    std::unordered_map<std::string, int> waterLevel = {{"Low",1},{"Medium",2},{"High",3}};
    int cropW = waterLevel.count(crop.waterReq)    ? waterLevel[crop.waterReq]    : 2;
    int inW   = waterLevel.count(input.waterAvailability) ? waterLevel[input.waterAvailability] : 2;
    int diff  = std::abs(cropW - inW);
    if (diff == 0) total += weights["water"];
    else if (diff == 1) total += weights["water"] * 0.5;
    // diff==2 → 0

    // 5. Wind tolerance
    if (input.windVelocity <= crop.windTolerance) {
        total += weights["wind"];
    } else {
        double over = input.windVelocity - crop.windTolerance;
        double penalty = over / 30.0;
        double windScore = std::max(0.0, weights["wind"] * (1.0 - penalty));
        total += windScore;
    }

    return total; // max possible = 100
}

// ─────────────────────────────────────────────
//  INPUT PARSING
// ─────────────────────────────────────────────
AgriInput parseArgs(int argc, char* argv[]) {
    AgriInput inp;
    inp.soilType          = argv[1];
    inp.soilPH            = std::stod(argv[2]);
    inp.temperature       = std::stod(argv[3]);
    inp.waterAvailability = argv[4];
    inp.windVelocity      = std::stod(argv[5]);
    inp.area              = std::stod(argv[6]);
    return inp;
}

AgriInput parseFile(const std::string& path) {
    AgriInput inp;
    std::ifstream f(path);
    if (!f.is_open()) {
        std::cerr << "ERROR: Cannot open " << path << "\n";
        exit(1);
    }
    std::string line;
    std::unordered_map<std::string, std::string> kv;
    while (std::getline(f, line)) {
        auto pos = line.find('=');
        if (pos != std::string::npos)
            kv[line.substr(0, pos)] = line.substr(pos + 1);
    }
    inp.soilType          = kv["soilType"];
    inp.soilPH            = std::stod(kv["soilPH"]);
    inp.temperature       = std::stod(kv["temperature"]);
    inp.waterAvailability = kv["waterAvailability"];
    inp.windVelocity      = std::stod(kv["windVelocity"]);
    inp.area              = std::stod(kv["area"]);
    return inp;
}

// ─────────────────────────────────────────────
//  JSON ESCAPE HELPER
// ─────────────────────────────────────────────
std::string jsonEscape(const std::string& s) {
    std::string out;
    for (char c : s) {
        if (c == '"')  out += "\\\"";
        else if (c == '\\') out += "\\\\";
        else out += c;
    }
    return out;
}

// ─────────────────────────────────────────────
//  MAIN
// ─────────────────────────────────────────────
int main(int argc, char* argv[]) {

    // ── DSA: queue to simulate input pipeline ──────────────
    std::queue<AgriInput> requestQueue;

    AgriInput userInput;
    if (argc == 7) {
        userInput = parseArgs(argc, argv);
    } else {
        userInput = parseFile("input.txt");
    }
    requestQueue.push(userInput);

    // ── DSA: stack to support undo (demo: pop & re-push) ───
    std::stack<AgriInput> undoStack;
    undoStack.push(userInput);

    // Process front of queue
    AgriInput inp = requestQueue.front();
    requestQueue.pop();

    // ── DSA: vector for crop dataset ───────────────────────
    std::vector<Crop> crops = buildCropDatabase();

    // ── DSA: unordered_map for score accumulation ──────────
    std::unordered_map<std::string, double> scoreMap;
    for (const auto& crop : crops) {
        scoreMap[crop.name] = scoreCrop(crop, inp);
    }

    // Sort crops by score
    std::sort(crops.begin(), crops.end(), [&](const Crop& a, const Crop& b) {
        return scoreMap[a.name] > scoreMap[b.name];
    });

    // Top 3 results
    std::vector<CropResult> top3;
    double maxScore = scoreMap[crops[0].name];
    if (maxScore < 1.0) maxScore = 1.0; // avoid div/0

    for (int i = 0; i < 3 && i < (int)crops.size(); i++) {
        CropResult r;
        r.name        = crops[i].name;
        r.score       = scoreMap[crops[i].name];
        r.percentage  = std::round((r.score / 100.0) * 100.0 * 10.0) / 10.0;
        r.emoji       = crops[i].emoji;
        r.description = crops[i].description;
        top3.push_back(r);
    }

    // ── Write output.json ───────────────────────────────────
    std::ofstream out("output.json");
    out << "{\n";
    out << "  \"status\": \"success\",\n";
    out << "  \"input\": {\n";
    out << "    \"soilType\": \"" << jsonEscape(inp.soilType) << "\",\n";
    out << "    \"soilPH\": " << std::fixed << std::setprecision(1) << inp.soilPH << ",\n";
    out << "    \"temperature\": " << inp.temperature << ",\n";
    out << "    \"waterAvailability\": \"" << jsonEscape(inp.waterAvailability) << "\",\n";
    out << "    \"windVelocity\": " << inp.windVelocity << ",\n";
    out << "    \"area\": " << inp.area << "\n";
    out << "  },\n";
    out << "  \"recommendations\": [\n";
    for (int i = 0; i < (int)top3.size(); i++) {
        const auto& r = top3[i];
        out << "    {\n";
        out << "      \"rank\": " << (i + 1) << ",\n";
        out << "      \"name\": \"" << jsonEscape(r.name) << "\",\n";
        out << "      \"emoji\": \"" << jsonEscape(r.emoji) << "\",\n";
        out << "      \"score\": " << std::fixed << std::setprecision(1) << r.score << ",\n";
        out << "      \"percentage\": " << r.percentage << ",\n";
        out << "      \"description\": \"" << jsonEscape(r.description) << "\"\n";
        out << "    }" << (i < (int)top3.size() - 1 ? "," : "") << "\n";
    }
    out << "  ],\n";
    out << "  \"poorMatch\": " << (top3[0].percentage < 50.0 ? "true" : "false") << "\n";
    out << "}\n";
    out.close();

    // Also print to stdout for debugging
    std::cout << "=== Top 3 Crop Recommendations ===\n";
    for (const auto& r : top3) {
        std::cout << r.emoji << " " << r.name
                  << " — " << r.percentage << "%\n";
    }
    std::cout << "Output written to output.json\n";

    return 0;
}