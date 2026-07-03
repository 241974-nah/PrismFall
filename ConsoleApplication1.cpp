#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <random>
#include <algorithm>
#include <stack> // DSA: Include stack library

using namespace std;

// --- Global Constants ---
const float SCREEN_WIDTH = 1920.0f;
const float SCREEN_HEIGHT = 1080.0f;

// Board Layout Constants
const float RADIUS_PEG = 22.0f;
const float RADIUS_SCORE = 10.0f;
const float SPACING_SLOT = 60.0f;
const float MARGIN_TOP = 100.0f;

// Palette Layout
const float POS_X_PALETTE = 120.0f;
const float SPACING_PALETTE = 70.0f;

// Difficulty Constants
const int DIFF_EASY = 0;
const int DIFF_MEDIUM = 1;
const int DIFF_HARD = 2;

// Game State Constants
const int STATE_MENU = 0;
const int STATE_PLAYING = 1;

// Color ID Constant
const int NO_COLOR = -1;

// --- Utility Functions ---

sf::Color getColorFromId(int id) {
    if (id == 0) return sf::Color(231, 76, 60);    // Red
    if (id == 1) return sf::Color(46, 204, 113);   // Green
    if (id == 2) return sf::Color(52, 152, 219);   // Blue
    if (id == 3) return sf::Color(241, 196, 15);   // Yellow
    if (id == 4) return sf::Color(230, 126, 34);   // Orange
    if (id == 5) return sf::Color(155, 89, 182);   // Purple
    if (id == 6) return sf::Color(26, 188, 156);   // Cyan
    if (id == 7) return sf::Color(255, 105, 180);  // Pink
    if (id == 8) return sf::Color(236, 240, 241);  // White
    if (id == 9) return sf::Color(121, 85, 72);    // Brown
    return sf::Color::Transparent;
}

// --- Classes ---

class SoundSystem {
private:
    sf::Music bgMusic;
    sf::SoundBuffer bClick, bSelect, bUndo, bSubmit, bWin, bLose;
    sf::Sound sClick, sSelect, sUndo, sSubmit, sWin, sLose;

public:
    SoundSystem() {
        // Simplified loading - assumes files exist in folder
        bgMusic.openFromFile("welcome.ogg");
        bgMusic.setLoop(true);

        bClick.loadFromFile("click.wav"); sClick.setBuffer(bClick);
        bSelect.loadFromFile("select.wav"); sSelect.setBuffer(bSelect);
        bUndo.loadFromFile("undo.wav"); sUndo.setBuffer(bUndo);
        bSubmit.loadFromFile("submit.wav"); sSubmit.setBuffer(bSubmit);
        bWin.loadFromFile("win.wav"); sWin.setBuffer(bWin);
        bLose.loadFromFile("lose.wav"); sLose.setBuffer(bLose);
    }

    void playMusic(bool play) {
        if (play && bgMusic.getStatus() != sf::Music::Playing) bgMusic.play();
        else if (!play) bgMusic.stop();
    }

    void playClick() { sClick.play(); }
    void playSelect() { sSelect.play(); }
    void playUndo() { sUndo.play(); }
    void playSubmit() { sSubmit.play(); }
    void playVictory() { sWin.play(); }
    void playDefeat() { sLose.play(); }
};

class Settings {
private:
    int colorCount;
    int patternSize;
    int maxTurns;
    string diffName;

public:
    Settings() { setup(DIFF_MEDIUM); }

    void setup(int difficulty) {
        if (difficulty == DIFF_EASY) {
            colorCount = 5; patternSize = 3; maxTurns = 15; diffName = "EASY";
        }
        else if (difficulty == DIFF_HARD) {
            colorCount = 10; patternSize = 5; maxTurns = 10; diffName = "HARD";
        }
        else {
            colorCount = 8; patternSize = 4; maxTurns = 12; diffName = "MEDIUM";
        }
    }

    int getTotalColors() { return colorCount; }
    int getCodeLength() { return patternSize; }
    int getMaxAttempts() { return maxTurns; }
    string getName() { return diffName; }
};

class TurnResult {
private:
    vector<int> pegs;
    int exactMatches;
    int colorMatches;

public:
    // Pass by value (copy) is simpler for students than const reference here
    TurnResult(vector<int> p, int red, int white) {
        pegs = p;
        exactMatches = red;
        colorMatches = white;
    }

    vector<int> getPegs() { return pegs; }
    int getRed() { return exactMatches; }
    int getWhite() { return colorMatches; }
};

class PrismFall {
private:
    Settings currentSettings;
    int difficultyId;

    // DSA: Vector Declaration
    vector<int> secretCode;
    vector<int> sortedCode;
    vector<TurnResult> history;

    // DSA: Stack Declaration
    stack<int> undoStack;

    vector<int> currentInput;

    int turnCount;
    bool victory;
    bool defeat;

    // DSA: Binary Search Implementation
    // We use "const vector<int>&" (Address) here because copying the vector 
    // every time we search is inefficient. This is a "Best Practice".
    bool binarySearch(const vector<int>& sortedArr, int target) {
        int left = 0;
        int right = sortedArr.size() - 1;

        while (left <= right) {
            int mid = left + (right - left) / 2;

            if (sortedArr[mid] == target) {
                return true; // Found
            }
            if (sortedArr[mid] < target) {
                left = mid + 1;
            }
            else {
                right = mid - 1;
            }
        }
        return false; // Not found
    }

public:
    PrismFall() {
        difficultyId = DIFF_MEDIUM;
        currentSettings.setup(DIFF_MEDIUM);
        restartGame();
    }

    void setDifficulty(int diff) {
        difficultyId = diff;
        currentSettings.setup(diff);
        restartGame();
    }

    void restartGame() {
        history.clear();

        // DSA: Clearing the Stack
        while (!undoStack.empty()) {
            undoStack.pop();
        }

        currentInput.clear();
        turnCount = 0;
        victory = false;
        defeat = false;
        generateCode();
    }

    void generateCode() {
        secretCode.clear();
        vector<int> pool;

        for (int i = 0; i < currentSettings.getTotalColors(); i++) {
            pool.push_back(i);
        }

        random_device rd;
        mt19937 g(rd());
        shuffle(pool.begin(), pool.end(), g);

        for (int i = 0; i < currentSettings.getCodeLength(); i++) {
            secretCode.push_back(pool[i]);
        }

        // DSA: Prepare for Binary Search
        sortedCode = secretCode;
        sort(sortedCode.begin(), sortedCode.end());

        cout << "Secret Code: ";
        for (int c : secretCode) cout << c << " ";
        cout << endl;
    }

    // Returns true if peg was successfully placed (for audio trigger)
    bool placePeg(int colorId) {
        if (currentInput.size() >= currentSettings.getCodeLength()) return false;

        // Linear search for duplicates
        for (int c : currentInput) {
            if (c == colorId) return false;
        }

        // DSA: Stack Push
        undoStack.push(colorId);

        // DSA: Vector Push
        currentInput.push_back(colorId);
        return true;
    }

    bool undoMove() {
        if (undoStack.empty()) return false;

        // DSA: Stack Pop
        undoStack.pop();
        currentInput.pop_back();
        return true;
    }

    bool confirmTurn() {
        if (currentInput.size() != currentSettings.getCodeLength()) return false;

        int red = 0;
        int white = 0;

        for (int i = 0; i < currentSettings.getCodeLength(); i++) {
            if (currentInput[i] == secretCode[i]) {
                red++;
            }
            else {
                // DSA: Using Binary Search
                if (binarySearch(sortedCode, currentInput[i])) {
                    white++;
                }
            }
        }

        TurnResult res(currentInput, red, white);
        history.push_back(res);
        turnCount++;

        if (red == currentSettings.getCodeLength()) {
            victory = true;
        }
        else if (turnCount >= currentSettings.getMaxAttempts()) {
            defeat = true;
        }

        while (!undoStack.empty()) undoStack.pop();
        currentInput.clear();
        return true;
    }

    // SIMPLIFIED GETTERS: Return by Value (Copy) instead of Reference (&)
    Settings getSettings() { return currentSettings; }
    vector<TurnResult> getHistory() { return history; }
    vector<int> getCurrentInput() { return currentInput; }
    vector<int> getSecretCode() { return secretCode; }

    int getTurnCount() { return turnCount; }
    int getDifficulty() { return difficultyId; }
    bool isWon() { return victory; }
    bool isLost() { return defeat; }
};

class UiButton {
private:
    sf::RectangleShape rect;
    sf::Text label;

public:
    // We MUST use "sf::Font&" (Address) here.
    UiButton(float x, float y, float w, float h, string text, sf::Font& font, sf::Color color) {
        rect.setPosition(x, y);
        rect.setSize(sf::Vector2f(w, h));
        rect.setFillColor(color);

        label.setFont(font);
        label.setString(text);
        label.setCharacterSize(24);
        label.setFillColor(sf::Color::White);

        sf::FloatRect bounds = label.getLocalBounds();
        label.setOrigin(bounds.left + bounds.width / 2.0f, bounds.top + bounds.height / 2.0f);
        label.setPosition(x + w / 2.0f, y + h / 2.0f);
    }

    void render(sf::RenderWindow& win) {
        win.draw(rect);
        win.draw(label);
    }

    sf::RectangleShape& getShape() { return rect; }

    bool checkClick(sf::Event e, sf::RenderWindow& win) {
        if (e.type == sf::Event::MouseButtonPressed && e.mouseButton.button == sf::Mouse::Left) {
            sf::Vector2i mPos = sf::Mouse::getPosition(win);
            sf::Vector2f worldPos = win.mapPixelToCoords(mPos);
            if (rect.getGlobalBounds().contains(worldPos)) {
                return true;
            }
        }
        return false;
    }
};

int main() {
    sf::RenderWindow window(sf::VideoMode(SCREEN_WIDTH, SCREEN_HEIGHT), "PrismFall", sf::Style::Default);
    window.setFramerateLimit(60);

    sf::Font font;
    font.loadFromFile("arial.ttf");

    SoundSystem audio;
    PrismFall game;

    int currentState = STATE_MENU;
    bool showDiffMenu = false;
    bool showRules = false;

    int selectedPaletteIndex = 0;

    audio.playMusic(true);

    // --- RULES DATA (Hardcoded) ---
    vector<string> rulesTextLines = {
        "OBJECTIVE: Guess the secret color code before you run out of turns!",
        "",
        "NO DUPLICATES: The secret code never uses the same color twice.",
        "",
        "CONTROLS: Click colors OR use Arrow Keys + Spacebar.",
        "  - Backspace: Undo last peg.",
        "  - Enter: Submit your guess.",
        "",
        "HOW TO WIN: Watch the small pins for clues!",
        "  - Red Pin: Correct Color in the Correct Slot.",
        "  - White Pin: Correct Color, but in the Wrong Slot."
    };

    // --- MENU BACKGROUND (Menu Only) ---
    sf::Texture menuBgTex;
    sf::Sprite menuBgSprite;
    bool menuBgLoaded = menuBgTex.loadFromFile("menu_bg.png");
    if (menuBgLoaded) {
        menuBgSprite.setTexture(menuBgTex);
        sf::FloatRect tr = menuBgSprite.getLocalBounds();
        menuBgSprite.setOrigin(tr.width / 2, tr.height / 2);
        menuBgSprite.setPosition(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2);
    }

    // --- GAME BACKGROUNDS (Distinct for Each Difficulty) ---
    // Ensure these files exist: "bg_easy.jpg", "bg_medium.jpg", "bg_hard.jpg"
    sf::Texture bgEasyTex, bgMedTex, bgHardTex;
    sf::Sprite bgEasySprite, bgMedSprite, bgHardSprite;
    bool bgEasyLoaded = bgEasyTex.loadFromFile("bg_easy.png");
    bool bgMedLoaded = bgMedTex.loadFromFile("bg_medium.png");
    bool bgHardLoaded = bgHardTex.loadFromFile("bg_hard.png");

    if (bgEasyLoaded) {
        bgEasySprite.setTexture(bgEasyTex);
    }
    if (bgMedLoaded) {
        bgMedSprite.setTexture(bgMedTex);
    }
    if (bgHardLoaded) {
        bgHardSprite.setTexture(bgHardTex);
    }

    sf::Text authorText("A game by: Mubashar and Ahtesham", font, 20);
    authorText.setPosition(30, SCREEN_HEIGHT - 50);
    authorText.setFillColor(sf::Color(150, 150, 150));

    float btnW = 300, btnH = 80;
    float midX = SCREEN_WIDTH / 2 - btnW / 2;

    // Calculating centered position for the block of 3 buttons (Play, Diff, Fin)
    // Total height of buttons block = (3 * 80) + (2 * 40 spacing) = 320
    float totalMenuHeight = 320.0f;
    float menuY = (SCREEN_HEIGHT - totalMenuHeight) / 2.0f;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) window.close();

            if (currentState == STATE_MENU) {
                // --- MENU EVENT HANDLING ---

                UiButton btnRules(SCREEN_WIDTH - 330, SCREEN_HEIGHT - 110, 300, 80, "RULES", font, sf::Color(52, 152, 219));
                UiButton btnCloseRules(SCREEN_WIDTH / 2 + 500, SCREEN_HEIGHT / 2 - 350, 60, 60, "X", font, sf::Color(231, 76, 60));

                if (showRules) {
                    if (btnCloseRules.checkClick(event, window)) {
                        audio.playClick();
                        showRules = false;
                    }
                }
                else {
                    UiButton btnPlay(midX, menuY, btnW, btnH, "PLAY", font, sf::Color(46, 204, 113));
                    UiButton btnDiff(midX, menuY + 120, btnW, btnH, "DIFFICULTY", font, sf::Color(241, 196, 15));
                    UiButton btnFin(midX, menuY + 240, btnW, btnH, "FIN", font, sf::Color(231, 76, 60));

                    if (btnRules.checkClick(event, window)) {
                        audio.playClick();
                        showRules = true;
                    }

                    if (btnPlay.checkClick(event, window)) {
                        audio.playClick();
                        audio.playMusic(false);
                        currentState = STATE_PLAYING;
                        selectedPaletteIndex = 0;
                    }
                    if (btnFin.checkClick(event, window)) {
                        audio.playClick();
                        window.close();
                    }
                    if (btnDiff.checkClick(event, window)) {
                        audio.playClick();
                        showDiffMenu = !showDiffMenu;
                    }

                    if (showDiffMenu) {
                        float optX = midX + btnW + 30;
                        UiButton btnEasy(optX, menuY + 120 - 90, 250, 60, "EASY", font, sf::Color(46, 204, 113));
                        UiButton btnMed(optX, menuY + 120, 250, 60, "MEDIUM", font, sf::Color(241, 196, 15));
                        UiButton btnHard(optX, menuY + 120 + 90, 250, 60, "HARD", font, sf::Color(231, 76, 60));

                        if (btnEasy.checkClick(event, window)) {
                            audio.playClick();
                            game.setDifficulty(DIFF_EASY);
                            showDiffMenu = false;
                        }
                        if (btnMed.checkClick(event, window)) {
                            audio.playClick();
                            game.setDifficulty(DIFF_MEDIUM);
                            showDiffMenu = false;
                        }
                        if (btnHard.checkClick(event, window)) {
                            audio.playClick();
                            game.setDifficulty(DIFF_HARD);
                            showDiffMenu = false;
                        }
                    }
                }
            }
            else if (currentState == STATE_PLAYING) {
                // Game Over Return Click
                if (game.isWon() || game.isLost()) {
                    if (event.type == sf::Event::MouseButtonPressed) {
                        audio.playClick();
                        currentState = STATE_MENU;
                        game.restartGame();
                        audio.playMusic(true);
                    }
                }
                else {
                    // --- Input Handling ---

                    // Keyboard Navigation
                    if (event.type == sf::Event::KeyPressed) {
                        Settings set = game.getSettings();
                        int maxColors = set.getTotalColors();

                        if (event.key.code == sf::Keyboard::Up) {
                            selectedPaletteIndex--;
                            if (selectedPaletteIndex < 0) selectedPaletteIndex = maxColors - 1;
                        }
                        else if (event.key.code == sf::Keyboard::Down) {
                            selectedPaletteIndex++;
                            if (selectedPaletteIndex >= maxColors) selectedPaletteIndex = 0;
                        }
                        else if (event.key.code == sf::Keyboard::Space) {
                            if (game.placePeg(selectedPaletteIndex)) {
                                audio.playSelect();
                            }
                        }
                        else if (event.key.code == sf::Keyboard::BackSpace) {
                            if (game.undoMove()) {
                                audio.playUndo();
                            }
                        }
                        else if (event.key.code == sf::Keyboard::Enter) {
                            if (game.confirmTurn()) {
                                audio.playSubmit();
                                if (game.isWon()) audio.playVictory();
                                if (game.isLost()) audio.playDefeat();
                            }
                        }
                    }

                    // Mouse Interaction
                    Settings set = game.getSettings();

                    float totalH = set.getTotalColors() * SPACING_PALETTE;
                    float startY = (SCREEN_HEIGHT - totalH) / 2.0f;

                    for (int i = 0; i < set.getTotalColors(); i++) {
                        float px = POS_X_PALETTE;
                        float py = startY + i * SPACING_PALETTE;

                        sf::Vector2f mPos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
                        float dx = mPos.x - px;
                        float dy = mPos.y - py;

                        if (dx * dx + dy * dy <= RADIUS_PEG * RADIUS_PEG) {
                            if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
                                audio.playSelect();
                                game.placePeg(i);
                                selectedPaletteIndex = i;
                            }
                        }
                    }

                    UiButton btnMenu(30, 30, 150, 50, "MENU", font, sf::Color(231, 76, 60));
                    UiButton btnUndo(SCREEN_WIDTH - 200, SCREEN_HEIGHT - 150, 150, 50, "Undo", font, sf::Color(230, 126, 34));
                    UiButton btnSubmit(SCREEN_WIDTH - 200, SCREEN_HEIGHT - 80, 150, 50, "SUBMIT", font, sf::Color(52, 152, 219));

                    if (btnMenu.checkClick(event, window)) {
                        audio.playClick();
                        currentState = STATE_MENU;
                        audio.playMusic(true);
                        game.restartGame();
                    }
                    if (btnUndo.checkClick(event, window)) {
                        audio.playUndo();
                        game.undoMove();
                    }
                    if (btnSubmit.checkClick(event, window)) {
                        if (game.confirmTurn()) {
                            audio.playSubmit();
                            if (game.isWon()) audio.playVictory();
                            if (game.isLost()) audio.playDefeat();
                        }
                    }
                }
            }
        }

        // --- Rendering ---
        window.clear(sf::Color(30, 30, 30));

        if (currentState == STATE_MENU) {
            // Draw Menu Background
            if (menuBgLoaded) {
                window.draw(menuBgSprite);
            }
            window.draw(authorText);

            UiButton btnPlay(midX, menuY, btnW, btnH, "PLAY", font, sf::Color(46, 204, 113));
            UiButton btnDiff(midX, menuY + 120, btnW, btnH, "DIFFICULTY", font, sf::Color(241, 196, 15));
            UiButton btnFin(midX, menuY + 240, btnW, btnH, "FIN", font, sf::Color(231, 76, 60));
            UiButton btnRules(SCREEN_WIDTH - 330, SCREEN_HEIGHT - 110, 300, 80, "RULES", font, sf::Color(52, 152, 219));

            if (game.getDifficulty() == DIFF_MEDIUM && !showDiffMenu) {
                sf::RectangleShape border = btnDiff.getShape();
                border.setFillColor(sf::Color::Transparent);
                border.setOutlineThickness(5);
                border.setOutlineColor(sf::Color::White);
                window.draw(border);
            }

            btnPlay.render(window);
            btnDiff.render(window);
            btnFin.render(window);
            btnRules.render(window);

            if (showDiffMenu) {
                float optX = midX + btnW + 30;
                UiButton btnEasy(optX, menuY + 120 - 90, 250, 60, "EASY", font, sf::Color(46, 204, 113));
                UiButton btnMed(optX, menuY + 120, 250, 60, "MEDIUM", font, sf::Color(241, 196, 15));
                UiButton btnHard(optX, menuY + 120 + 90, 250, 60, "HARD", font, sf::Color(231, 76, 60));

                sf::Text txtEasy("5 Colors, 3 Pattern", font, 14); txtEasy.setPosition(optX + 10, menuY + 120 - 90 + 65);
                sf::Text txtMed("8 Colors, 4 Pattern", font, 14); txtMed.setPosition(optX + 10, menuY + 120 + 65);
                sf::Text txtHard("10 Colors, 5 Pattern", font, 14); txtHard.setPosition(optX + 10, menuY + 120 + 90 + 65);

                btnEasy.render(window); window.draw(txtEasy);
                btnMed.render(window);  window.draw(txtMed);
                btnHard.render(window); window.draw(txtHard);
            }

            // --- RULES POPUP ---
            if (showRules) {
                sf::RectangleShape dim(sf::Vector2f(SCREEN_WIDTH, SCREEN_HEIGHT));
                dim.setFillColor(sf::Color(0, 0, 0, 150));
                window.draw(dim);

                sf::RectangleShape popBg(sf::Vector2f(1200, 800));
                popBg.setOrigin(600, 400);
                popBg.setPosition(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2);
                popBg.setFillColor(sf::Color(200, 200, 200));
                popBg.setOutlineThickness(5);
                popBg.setOutlineColor(sf::Color(52, 152, 219));
                window.draw(popBg);

                UiButton btnCloseRules(SCREEN_WIDTH / 2 + 500, SCREEN_HEIGHT / 2 - 350, 60, 60, "X", font, sf::Color(231, 76, 60));
                btnCloseRules.render(window);

                sf::Text rulesTitle("How to Play PRISMFALL:", font, 40);
                rulesTitle.setFillColor(sf::Color::Black);
                rulesTitle.setPosition(SCREEN_WIDTH / 2 - 500, SCREEN_HEIGHT / 2 - 350);
                window.draw(rulesTitle);

                float textY = SCREEN_HEIGHT / 2 - 250;
                for (const string& line : rulesTextLines) {
                    sf::Text lineText(line, font, 30);
                    lineText.setFillColor(sf::Color::Black);
                    lineText.setPosition(SCREEN_WIDTH / 2 - 500, textY);
                    window.draw(lineText);
                    textY += 45;
                }
            }
        }
        else if (currentState == STATE_PLAYING) {

            // --- DRAW GAME BACKGROUND ---
            int diff = game.getDifficulty();
            if (diff == DIFF_EASY && bgEasyLoaded) {
                window.draw(bgEasySprite);
            }
            else if (diff == DIFF_MEDIUM && bgMedLoaded) {
                window.draw(bgMedSprite);
            }
            else if (diff == DIFF_HARD && bgHardLoaded) {
                window.draw(bgHardSprite);
            }

            // SIMPLIFIED: No reference '&'
            Settings set = game.getSettings();

            float wGuess = (set.getCodeLength() * SPACING_SLOT) + 40.0f;
            float wScore = (set.getCodeLength() * (RADIUS_SCORE * 3.5f)) + 30.0f;
            float wTotal = wGuess + wScore;
            float hTotal = set.getMaxAttempts() * SPACING_SLOT;

            float xBoard = (SCREEN_WIDTH - wTotal) / 2.0f;
            float yBoard = (SCREEN_HEIGHT - hTotal) / 2.0f;

            // Board Backgrounds
            sf::RectangleShape bgGuess(sf::Vector2f(wGuess, hTotal));
            bgGuess.setPosition(xBoard, yBoard);
            bgGuess.setFillColor(sf::Color(20, 20, 20, 230)); // Slightly transparent board to see BG
            bgGuess.setOutlineThickness(2);
            bgGuess.setOutlineColor(sf::Color(80, 80, 80));
            window.draw(bgGuess);

            sf::RectangleShape bgScore(sf::Vector2f(wScore, hTotal));
            bgScore.setPosition(xBoard + wGuess, yBoard);
            bgScore.setFillColor(sf::Color(50, 50, 50, 230)); // Slightly transparent
            bgScore.setOutlineThickness(2);
            bgScore.setOutlineColor(sf::Color(80, 80, 80));
            window.draw(bgScore);

            // Draw Board Rows
            vector<TurnResult> history = game.getHistory();
            vector<int> input = game.getCurrentInput();

            for (int r = 0; r < set.getMaxAttempts(); r++) {
                float yRow = yBoard + r * SPACING_SLOT + (SPACING_SLOT / 2.0f);
                float xGuessStart = xBoard + (SPACING_SLOT / 2.0f) + 20.0f;
                float xScoreStart = xBoard + wGuess + 20.0f;

                for (int c = 0; c < set.getCodeLength(); c++) {
                    sf::CircleShape slot(RADIUS_PEG);
                    slot.setOrigin(RADIUS_PEG, RADIUS_PEG);
                    slot.setFillColor(sf::Color::Black);
                    slot.setPosition(xGuessStart + c * SPACING_SLOT, yRow);
                    window.draw(slot);
                }

                float scoreSpace = RADIUS_SCORE * 3.0f;
                for (int s = 0; s < set.getCodeLength(); s++) {
                    sf::CircleShape slot(RADIUS_SCORE);
                    slot.setOrigin(RADIUS_SCORE, RADIUS_SCORE);
                    slot.setFillColor(sf::Color::Black);
                    slot.setPosition(xScoreStart + s * scoreSpace, yRow);
                    window.draw(slot);
                }

                if (r < history.size()) {
                    TurnResult& turn = history[r];
                    vector<int> pegs = turn.getPegs();

                    for (int c = 0; c < pegs.size(); c++) {
                        sf::CircleShape p(RADIUS_PEG);
                        p.setOrigin(RADIUS_PEG, RADIUS_PEG);
                        p.setFillColor(getColorFromId(pegs[c]));
                        p.setPosition(xGuessStart + c * SPACING_SLOT, yRow);
                        window.draw(p);
                    }

                    int drawn = 0;
                    for (int i = 0; i < turn.getRed(); i++) {
                        sf::CircleShape p(RADIUS_SCORE);
                        p.setOrigin(RADIUS_SCORE, RADIUS_SCORE);
                        p.setFillColor(sf::Color::Red);
                        p.setPosition(xScoreStart + drawn * scoreSpace, yRow);
                        window.draw(p);
                        drawn++;
                    }
                    for (int i = 0; i < turn.getWhite(); i++) {
                        sf::CircleShape p(RADIUS_SCORE);
                        p.setOrigin(RADIUS_SCORE, RADIUS_SCORE);
                        p.setFillColor(sf::Color::White);
                        p.setPosition(xScoreStart + drawn * scoreSpace, yRow);
                        window.draw(p);
                        drawn++;
                    }
                }
                else if (r == game.getTurnCount() && !game.isWon() && !game.isLost()) {
                    for (int c = 0; c < input.size(); c++) {
                        sf::CircleShape p(RADIUS_PEG);
                        p.setOrigin(RADIUS_PEG, RADIUS_PEG);
                        p.setFillColor(getColorFromId(input[c]));
                        p.setPosition(xGuessStart + c * SPACING_SLOT, yRow);
                        window.draw(p);
                    }

                    sf::RectangleShape high(sf::Vector2f(wGuess, SPACING_SLOT));
                    high.setPosition(xBoard, yBoard + r * SPACING_SLOT);
                    high.setFillColor(sf::Color::Transparent);
                    high.setOutlineThickness(2);
                    high.setOutlineColor(sf::Color::Yellow);
                    window.draw(high);
                }
            }

            // Draw Palette
            float totalPalH = set.getTotalColors() * SPACING_PALETTE;
            float yPalStart = (SCREEN_HEIGHT - totalPalH) / 2.0f;

            for (int i = 0; i < set.getTotalColors(); i++) {
                sf::CircleShape p(RADIUS_PEG);
                p.setOrigin(RADIUS_PEG, RADIUS_PEG);
                p.setFillColor(getColorFromId(i));
                p.setPosition(POS_X_PALETTE, yPalStart + i * SPACING_PALETTE);
                p.setOutlineThickness(3);

                if (i == selectedPaletteIndex) {
                    p.setOutlineColor(sf::Color::Yellow);
                    p.setOutlineThickness(5);
                }
                else {
                    p.setOutlineColor(sf::Color::White);
                    p.setOutlineThickness(3);
                }

                window.draw(p);
            }

            // UI Elements
            sf::Text txtInfo;
            txtInfo.setFont(font);
            txtInfo.setCharacterSize(20);
            txtInfo.setPosition(SCREEN_WIDTH - 250, 50);
            stringstream ss;
            ss << "Difficulty: " << set.getName() << "\n";
            ss << "Turn: " << (game.getTurnCount() + 1) << " / " << set.getMaxAttempts();
            txtInfo.setString(ss.str());
            window.draw(txtInfo);

            UiButton btnMenu(30, 30, 150, 50, "MENU", font, sf::Color(231, 76, 60));
            UiButton btnUndo(SCREEN_WIDTH - 200, SCREEN_HEIGHT - 150, 150, 50, "Undo", font, sf::Color(230, 126, 34));
            UiButton btnSubmit(SCREEN_WIDTH - 200, SCREEN_HEIGHT - 80, 150, 50, "SUBMIT", font, sf::Color(52, 152, 219));

            btnMenu.render(window);
            btnUndo.render(window);
            btnSubmit.render(window);

            if (game.isWon() || game.isLost()) {
                sf::RectangleShape overlay(sf::Vector2f(SCREEN_WIDTH, SCREEN_HEIGHT));
                overlay.setFillColor(sf::Color(0, 0, 0, 220));
                window.draw(overlay);

                sf::Text msg;
                msg.setFont(font);
                msg.setCharacterSize(80);
                if (game.isWon()) {
                    msg.setString("VICTORY!");
                    msg.setFillColor(sf::Color::Green);
                }
                else {
                    msg.setString("GAME OVER");
                    msg.setFillColor(sf::Color::Red);
                }
                sf::FloatRect b = msg.getLocalBounds();
                msg.setOrigin(b.width / 2, b.height / 2);
                msg.setPosition(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 - 100);
                window.draw(msg);

                sf::Text sub("Click anywhere to return", font, 30);
                sub.setPosition(SCREEN_WIDTH / 2 - 180, SCREEN_HEIGHT / 2 + 100);
                window.draw(sub);

                if (game.isLost()) {
                    sf::Text codeT("The Secret Code Was:", font, 30);
                    codeT.setPosition(SCREEN_WIDTH / 2 - 150, SCREEN_HEIGHT / 2 - 10);
                    window.draw(codeT);

                    vector<int> code = game.getSecretCode();
                    float cx = SCREEN_WIDTH / 2 - ((set.getCodeLength() * 60) / 2);
                    for (int c : code) {
                        sf::CircleShape cs(20);
                        cs.setFillColor(getColorFromId(c));
                        cs.setPosition(cx, SCREEN_HEIGHT / 2 + 40);
                        window.draw(cs);
                        cx += 60;
                    }
                }
            }
        }

        window.display();
    }

    return 0;
}