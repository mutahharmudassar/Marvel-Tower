#include <SFML/Graphics.hpp>
#include "imgui.h"
#include "imgui-SFML.h"

#include <cstdlib>
#include <ctime>
#include <cstdio>
#include <cstring>
#include <iostream>

#include "Constants.hpp"
#include "DataStructures.hpp"
#include "TextureManager.hpp"
#include "GameObjects.hpp"
#include "GameMap.hpp"

struct WaveConfig {
    int groundEnemies;
    int flyingEnemies;
    bool hasBoss;
};

class Game {
private:
    sf::RenderWindow window;
    sf::Clock deltaClock;
    
    GameState state;
    GameMode mode;
    GameMap gameMap;
    
    LinkedList<Enemy*> enemies;
    LinkedList<Structure*> structures;
    Queue<Enemy*> spawnQueue;
    CombatLog combatLog;
    
    int towerHP, maxTowerHP;
    int coins;
    int totalKills, groundKills, flyingKills, bossKills;
    
    int currentWave, totalWaves;
    float spawnTimer, spawnInterval;
    int nextEnemyId, nextStructureId;
    
    WaveConfig easyWaves[10];
    WaveConfig epicWaves[15];
    
    StructureType selectedBuildType;
    Structure* selectedStructure;
    bool showGrid;
    
public:
    Game() : window(sf::VideoMode(Constants::WINDOW_WIDTH, Constants::WINDOW_HEIGHT),
                    "Marvel: Avenger Tower Defense ",
                    sf::Style::Titlebar | sf::Style::Close),
             state(STATE_MENU), mode(MODE_EASY),
             towerHP(200), maxTowerHP(200), coins(1000),//Coin
             totalKills(0), groundKills(0), flyingKills(0), bossKills(0),
             currentWave(0), totalWaves(10),
             spawnTimer(0), spawnInterval(0.8f),
             nextEnemyId(1), nextStructureId(1),
             selectedBuildType(STRUCT_NONE), selectedStructure(nullptr),
             showGrid(true) {
        
        srand((unsigned)time(nullptr));
        window.setFramerateLimit(60);
        
        if (!ImGui::SFML::Init(window)) {
            throw std::runtime_error("ImGui-SFML init failed!");
        }
        
        initWaveConfigs();
        TextureManager::getInstance(). loadAllAssets();
        gameMap.initialize();
        applyStyle();
        
        std::cout << "Game ready!" << std::endl;
    }
    
    ~Game() {
        ListNode<Enemy*>* eNode = enemies.getHead();
        while (eNode) {
            delete eNode->data;
            eNode = eNode->next;
        }
        
        ListNode<Structure*>* sNode = structures. getHead();
        while (sNode) {
            delete sNode->data;
            sNode = sNode->next;
        }
        
        ImGui::SFML::Shutdown();
    }
    
    void initWaveConfigs() {
        easyWaves[0] = {10, 5, false};
        easyWaves[1] = {15, 8, false};
        easyWaves[2] = {20, 10, false};
        easyWaves[3] = {25, 12, false};
        easyWaves[4] = {20, 10, true};
        easyWaves[5] = {30, 15, false};
        easyWaves[6] = {35, 18, false};
        easyWaves[7] = {40, 20, false};
        easyWaves[8] = {45, 25, false};
        easyWaves[9] = {35, 20, true};
        
        epicWaves[0] = {15, 8, false};
        epicWaves[1] = {20, 12, false};
        epicWaves[2] = {28, 15, false};
        epicWaves[3] = {35, 20, false};
        epicWaves[4] = {30, 15, true};
        epicWaves[5] = {40, 25, false};
        epicWaves[6] = {50, 30, false};
        epicWaves[7] = {60, 35, false};
        epicWaves[8] = {70, 40, false};
        epicWaves[9] = {50, 30, true};
        epicWaves[10] = {80, 50, false};
        epicWaves[11] = {90, 60, false};
        epicWaves[12] = {100, 70, false};
        epicWaves[13] = {110, 80, false};
        epicWaves[14] = {80, 50, true};
    }
    
    void applyStyle() {
        ImGuiStyle& style = ImGui::GetStyle();
        ImVec4* colors = style.Colors;
        
        colors[ImGuiCol_WindowBg] = ImVec4(0.1f, 0.1f, 0.12f, 0.95f);
        colors[ImGuiCol_Header] = ImVec4(0.7f, 0.15f, 0.15f, 0.8f);
        colors[ImGuiCol_HeaderHovered] = ImVec4(0.8f, 0.2f, 0.2f, 0.9f);
        colors[ImGuiCol_HeaderActive] = ImVec4(0.9f, 0.25f, 0.25f, 1.0f);
        colors[ImGuiCol_Button] = ImVec4(0.6f, 0.1f, 0.1f, 0.8f);
        colors[ImGuiCol_ButtonHovered] = ImVec4(0.75f, 0.15f, 0.15f, 0.9f);
        colors[ImGuiCol_ButtonActive] = ImVec4(0.9f, 0.2f, 0.2f, 1.0f);
        colors[ImGuiCol_FrameBg] = ImVec4(0.2f, 0.2f, 0.22f, 0.9f);
        colors[ImGuiCol_TitleBg] = ImVec4(0.5f, 0.1f, 0.1f, 1.0f);
        colors[ImGuiCol_TitleBgActive] = ImVec4(0.7f, 0.15f, 0.15f, 1.0f);
        colors[ImGuiCol_PlotHistogram] = ImVec4(0.2f, 0.8f, 0.2f, 1.0f);
        
        style.WindowRounding = 5.0f;
        style. FrameRounding = 3.0f;
        style.WindowPadding = ImVec2(10, 10);
    }
    
    void run() {
        while (window.isOpen()) {
            processEvents();
            sf::Time dt = deltaClock.restart();
            ImGui::SFML::Update(window, dt);
            update(dt. asSeconds());
            render();
        }
    }

private:
    void processEvents() {
        sf::Event event;
        while (window.pollEvent(event)) {
            ImGui::SFML::ProcessEvent(window, event);
            
            if (event.type == sf::Event::Closed) window.close();
            
            if (ImGui::GetIO().WantCaptureMouse || ImGui::GetIO().WantCaptureKeyboard)
                continue;
            
            if (event.type == sf::Event::MouseButtonPressed) {
                handleClick((float)event.mouseButton.x, (float)event.mouseButton.y,
                           event.mouseButton.button);
            }
            
            if (event. type == sf::Event::KeyPressed) {
                handleKey(event.key. code);
            }
        }
    }
    
    void handleClick(float x, float y, sf::Mouse::Button btn) {
        if (state != STATE_WAVE_PREP && state != STATE_WAVE_ACTIVE) return;
        if (x >= Constants::GAME_AREA_WIDTH) return;
        
        if (btn == sf::Mouse::Left) {
            if (selectedBuildType != STRUCT_NONE) {
                if (gameMap.canPlaceAt(x, y)) {
                    int cost = getStructureCost(selectedBuildType);
                    if (coins >= cost) {
                        placeStructure(selectedBuildType, x, y);
                        coins -= cost;
                        
                        char msg[64];
                        sprintf(msg, "Built %s", getStructureName(selectedBuildType));
                        combatLog.add(msg);
                    } else {
                        combatLog.add("Not enough coins!");
                    }
                }
                selectedBuildType = STRUCT_NONE;
                gameMap.setShowPreview(false);
                return;
            }
            
            ListNode<Structure*>* node = structures.getHead();
            while (node) {
                if (node->data->getBounds().contains(x, y)) {
                    if (selectedStructure) selectedStructure->setShowRange(false);
                    selectedStructure = node->data;
                    selectedStructure->setShowRange(true);
                    return;
                }
                node = node->next;
            }
            
            if (selectedStructure) {
                selectedStructure->setShowRange(false);
                selectedStructure = nullptr;
            }
        }
        
        if (btn == sf::Mouse::Right) {
            selectedBuildType = STRUCT_NONE;
            gameMap. setShowPreview(false);
            if (selectedStructure) {
                selectedStructure->setShowRange(false);
                selectedStructure = nullptr;
            }
        }
    }
    
    void handleKey(sf::Keyboard::Key key) {
        if (key == sf::Keyboard::Escape) {
            if (state == STATE_WAVE_PREP || state == STATE_WAVE_ACTIVE)
                state = STATE_PAUSED;
            else if (state == STATE_PAUSED)
                state = STATE_WAVE_PREP;
        }
        if (key == sf::Keyboard::G) {
            showGrid = !showGrid;
            gameMap.setShowGrid(showGrid);
        }
        if (key == sf::Keyboard::Space && state == STATE_WAVE_PREP) {
            startWave();
        }
    }
    
    void placeStructure(StructureType type, float x, float y) {
        float snappedX, snappedY;
        gameMap.snapToGrid(x, y, snappedX, snappedY);
        
        Structure* s = new Structure();
        s->initialize(nextStructureId++, type, snappedX, snappedY);
        structures. pushBack(s);
        gameMap.setOccupied(snappedX, snappedY, true);
    }
    
    void startGame(GameMode m) {
        mode = m;
        
        ListNode<Enemy*>* eNode = enemies.getHead();
        while (eNode) {
            ListNode<Enemy*>* next = eNode->next;
            delete eNode->data;
            enemies.remove(eNode);
            eNode = next;
        }
        
        ListNode<Structure*>* sNode = structures. getHead();
        while (sNode) {
            ListNode<Structure*>* next = sNode->next;
            delete sNode->data;
            structures.remove(sNode);
            sNode = next;
        }
        
        Enemy* e;
        while (spawnQueue.dequeue(e)) {
            delete e;
        }
        
        combatLog.clear();
        gameMap.initialize();
        
        if (mode == MODE_EASY) {
            coins = Constants::EASY_START_COINS;
            towerHP = maxTowerHP = Constants::EASY_TOWER_HP;
            totalWaves = Constants::EASY_WAVES;
        } else {
            coins = Constants::EPIC_START_COINS;
            towerHP = maxTowerHP = Constants::EPIC_TOWER_HP;
            totalWaves = Constants::EPIC_WAVES;
        }
        
        currentWave = 0;
        totalKills = groundKills = flyingKills = bossKills = 0;
        nextEnemyId = nextStructureId = 1;
        selectedBuildType = STRUCT_NONE;
        selectedStructure = nullptr;
        
        state = STATE_WAVE_PREP;
        combatLog.add("Game started!");
    }
    
    void startWave() {
        currentWave++;
        state = STATE_WAVE_ACTIVE;
        spawnTimer = 0;
        
        WaveConfig config;
        if (mode == MODE_EASY) {
            config = easyWaves[currentWave - 1];
        } else {
            config = epicWaves[currentWave - 1];
        }
        
        for (int i = 0; i < config.groundEnemies; i++) {
            Enemy* enemy = new Enemy();
            enemy->initialize(nextEnemyId++, ENEMY_GROUND, currentWave);
            int lane = rand() % 3;
            int pathLen;
            PathPoint* path = gameMap.getPath(lane, pathLen);
            enemy->setPath(path, pathLen);
            spawnQueue.enqueue(enemy);
        }
        
        for (int i = 0; i < config.flyingEnemies; i++) {
            Enemy* enemy = new Enemy();
            enemy->initialize(nextEnemyId++, ENEMY_FLYING, currentWave);
            int lane = rand() % 3;
            int pathLen;
            PathPoint* path = gameMap. getPath(lane, pathLen);
            enemy->setPath(path, pathLen);
            spawnQueue.enqueue(enemy);
        }
        
        if (config.hasBoss) {
            Enemy* boss = new Enemy();
            boss->initialize(nextEnemyId++, ENEMY_BOSS, currentWave);
            int pathLen;
            PathPoint* path = gameMap.getPath(1, pathLen);
            boss->setPath(path, pathLen);
            spawnQueue.enqueue(boss);
        }
        
        char msg[64];
        sprintf(msg, "Wave %d started!", currentWave);
        combatLog.add(msg);
    }
    
    void update(float dt) {
        if (selectedBuildType != STRUCT_NONE) {
            sf::Vector2i mousePixel = sf::Mouse::getPosition(window);
            gameMap.updatePreview((float)mousePixel.x, (float)mousePixel. y);
            gameMap.setShowPreview(true);
        }
        
        if (state == STATE_WAVE_ACTIVE) {
            spawnTimer += dt;
            if (spawnTimer >= spawnInterval && !spawnQueue. isEmpty()) {
                Enemy* enemy;
                if (spawnQueue. dequeue(enemy)) {
                    enemies.pushBack(enemy);
                }
                spawnTimer = 0;
            }
            
            ListNode<Enemy*>* eNode = enemies.getHead();
            while (eNode) {
                Enemy* e = eNode->data;
                e->update(dt);
                
                if (e->hasReachedEnd() && e->isAlive()) {
                    towerHP -= e->getTowerDamage();
                    char msg[64];
                    sprintf(msg, "Tower hit!  -%d HP", e->getTowerDamage());
                    combatLog.add(msg);
                    e->takeDamage(9999);
                }
                eNode = eNode->next;
            }
            
            ListNode<Structure*>* sNode = structures.getHead();
            while (sNode) {
                Structure* s = sNode->data;
                s->updateCooldown(dt);
                
                if (s->canFire()) {
                    Enemy* target = findTarget(s);
                    if (target) {
                        processAttack(s, target);
                        s->fire();
                    }
                }
                sNode = sNode->next;
            }
            
            cleanupDeadEnemies();
            
            if (enemies.isEmpty() && spawnQueue.isEmpty()) {
                int bonus = Constants::WAVE_COMPLETE_BONUS;
                if (currentWave % 5 == 0) bonus *= 2;
                coins += bonus;
                
                char msg[64];
                sprintf(msg, "Wave complete! +%d coins", bonus);
                combatLog.add(msg);
                
                if (currentWave >= totalWaves) {
                    state = STATE_VICTORY;
                } else {
                    state = STATE_WAVE_PREP;
                }
            }
            
            if (towerHP <= 0) {
                state = STATE_DEFEAT;
            }
        }
    }
    
    Enemy* findTarget(Structure* s) {
        Enemy* closest = nullptr;
        float closestDist = 9999.0f;
        
        ListNode<Enemy*>* node = enemies.getHead();
        while (node) {
            Enemy* e = node->data;
            if (e->isAlive() && s->canTarget(e->getType()) && s->isInRange(e->getPosition())) {
                sf::Vector2f sPos = s->getPosition();
                sf::Vector2f ePos = e->getPosition();
                float dist = calcDistance(sPos. x, sPos. y, ePos. x, ePos. y);
                if (dist < closestDist) {
                    closestDist = dist;
                    closest = e;
                }
            }
            node = node->next;
        }
        return closest;
    }
    
    void processAttack(Structure* s, Enemy* target) {
        int damage = s->getDamage();
        target->takeDamage(damage);
        s->addDamage(damage);
        s->setAttackVisual(target->getPosition());
        
        if (! target->isAlive()) {
            coins += target->getCoinReward();
            s->addKill();
            totalKills++;
            
            switch(target->getType()) {
                case ENEMY_GROUND: groundKills++; break;
                case ENEMY_FLYING: flyingKills++; break;
                case ENEMY_BOSS: bossKills++; break;
            }
        }
    }
    
    void cleanupDeadEnemies() {
        ListNode<Enemy*>* node = enemies.getHead();
        while (node) {
            ListNode<Enemy*>* next = node->next;
            if (! node->data->isAlive()) {
                delete node->data;
                enemies.remove(node);
            }
            node = next;
        }
    }
    
    void render() {
        window. clear(sf::Color(20, 25, 30));
        
        if (state != STATE_MENU) {
            window.draw(gameMap);
            
            ListNode<Structure*>* sNode = structures.getHead();
            while (sNode) {
                window.draw(*(sNode->data));
                sNode = sNode->next;
            }
            
            ListNode<Enemy*>* eNode = enemies.getHead();
            while (eNode) {
                window.draw(*(eNode->data));
                eNode = eNode->next;
            }
        }
        
        switch (state) {
            case STATE_MENU: renderMenu(); break;
            case STATE_WAVE_PREP:
            case STATE_WAVE_ACTIVE: renderGameUI(); break;
            case STATE_PAUSED: renderGameUI(); renderPauseMenu(); break;
            case STATE_VICTORY: renderVictory(); break;
            case STATE_DEFEAT: renderDefeat(); break;
        }
        
        ImGui::SFML::Render(window);
        window.display();
    }
    
    void renderMenu() {
        ImGui::SetNextWindowPos(ImVec2(Constants::WINDOW_WIDTH/2 - 220, Constants::WINDOW_HEIGHT/2 - 200));
        ImGui::SetNextWindowSize(ImVec2(440, 400));
        
        ImGui::Begin("Marvel Tower Defense", nullptr,
                     ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);
        
        ImGui::Dummy(ImVec2(0, 15));
        ImGui::SetCursorPosX(60);
        ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "M A R V E L   T O W E R   D E F E N S E");
        ImGui::SetCursorPosX(150);
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "~ SFML + ImGui Edition ~");
        
        ImGui::Dummy(ImVec2(0, 30));
        ImGui::SetCursorPosX(70);
        if (ImGui::Button("EASY MODE\n10 Waves | 300 Coins | 200 Tower HP", ImVec2(300, 55))) {
            startGame(MODE_EASY);
        }
        
        ImGui::Dummy(ImVec2(0, 10));
        ImGui::SetCursorPosX(70);
        if (ImGui::Button("EPIC MODE\n15 Waves | 250 Coins | 150 Tower HP", ImVec2(300, 55))) {
            startGame(MODE_EPIC);
        }
        
        ImGui::Dummy(ImVec2(0, 25));
        ImGui::Separator();
        ImGui::Dummy(ImVec2(0, 10));
        
        ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "CONTROLS:");
        ImGui::BulletText("Left Click - Place/Select structure");
        ImGui::BulletText("Right Click - Cancel selection");
        ImGui::BulletText("G - Toggle grid overlay");
        ImGui::BulletText("Space - Start next wave");
        ImGui::BulletText("Esc - Pause game");
        
        ImGui::Dummy(ImVec2(0, 15));
        ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "HEROES:");
        ImGui::Text("Shooter($50) Hawkeye($75) IronMan($100)");
        ImGui::Text("Dr. Strange($200) Thor($250)");
        
        ImGui::Dummy(ImVec2(0, 15));
        ImGui::SetCursorPosX(170);
        if (ImGui::Button("EXIT GAME", ImVec2(100, 30))) {
            window.close();
        }
        
        ImGui::End();
    }
    
    void renderGameUI() {
        renderTopBar();
        renderSidePanel();
    }
    
    void renderTopBar() {
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImVec2((float)Constants::GAME_AREA_WIDTH, 60));
        
        ImGui::Begin("##TopBar", nullptr,
                     ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                     ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar);
        
        ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "WAVE: %d/%d", currentWave, totalWaves);
        
        if (currentWave > 0 && currentWave % 5 == 0) {
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(1.0f, 0.2f, 0.2f, 1.0f), "[BOSS]");
        }
        
        ImGui::SameLine(180);
        ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.0f, 1.0f), "COINS: $%d", coins);
        
        ImGui::SameLine(350);
        ImGui::Text("KILLS: %d", totalKills);
        
        ImGui::SameLine(500);
        ImGui::Text("TOWER:");
        ImGui::SameLine();
        
        float hpPercent = (float)towerHP / maxTowerHP;
        ImVec4 hpColor;
        if (hpPercent > 0.6f) hpColor = ImVec4(0.2f, 0.8f, 0.2f, 1.0f);
        else if (hpPercent > 0.3f) hpColor = ImVec4(0.9f, 0.9f, 0.2f, 1.0f);
        else hpColor = ImVec4(0.9f, 0.2f, 0.2f, 1.0f);
        
        ImGui::PushStyleColor(ImGuiCol_PlotHistogram, hpColor);
        ImGui::ProgressBar(hpPercent, ImVec2(180, 20), "");
        ImGui::PopStyleColor();
        
        ImGui::SameLine();
        ImGui::Text("%d/%d", towerHP, maxTowerHP);
        
        if (state == STATE_WAVE_PREP) {
            ImGui::TextColored(ImVec4(0.2f, 0.8f, 0.2f, 1.0f), 
                              ">> PREPARATION PHASE - Press SPACE to start wave <<");
        } else if (state == STATE_WAVE_ACTIVE) {
            int remaining = enemies.getSize() + spawnQueue.getSize();
            ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.2f, 1.0f), 
                              ">> WAVE IN PROGRESS - Enemies remaining: %d <<", remaining);
        }
        
        ImGui::End();
    }
    
    void renderSidePanel() {
        ImGui::SetNextWindowPos(ImVec2((float)Constants::GAME_AREA_WIDTH, 0));
        ImGui::SetNextWindowSize(ImVec2((float)Constants::UI_PANEL_WIDTH, (float)Constants::WINDOW_HEIGHT));
        
        ImGui::Begin("##SidePanel", nullptr,
                     ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                     ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);
        
        ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "BUILD STRUCTURES");
        ImGui::Separator();
        ImGui::Dummy(ImVec2(0, 5));
        
        renderStructureButton("Shooter", Constants::SHOOTER_COST, STRUCT_SHOOTER, "Ground only, Fast fire");
        renderStructureButton("Hawkeye", Constants::HAWKEYE_COST, STRUCT_HAWKEYE, "Ground, Pierce enemies");
        renderStructureButton("Iron Man", Constants::IRONMAN_COST, STRUCT_IRONMAN, "Air + Ground");
        renderStructureButton("Dr. Strange", Constants::DRSTRANGE_COST, STRUCT_DRSTRANGE, "Air + Ground, Splash");
        renderStructureButton("Thor", Constants::THOR_COST, STRUCT_THOR, "Air + Ground, Chain");
        
        ImGui::Dummy(ImVec2(0, 10));
        ImGui::Separator();
        
        if (selectedStructure) {
            ImGui::Dummy(ImVec2(0, 5));
                        ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "SELECTED STRUCTURE:");
            
            ImGui::Text("%s (Lv.  %d)", getStructureName(selectedStructure->getType()),
                       selectedStructure->getLevel());
            ImGui::Text("Damage: %d", selectedStructure->getDamage());
            ImGui::Text("Range: %. 0f", selectedStructure->getRange());
            ImGui::Text("Kills: %d", selectedStructure->getTotalKills());
            ImGui::Text("Total Damage: %d", selectedStructure->getTotalDamageDealt());
            
            ImGui::Dummy(ImVec2(0, 5));
            
            int upgradeCost = selectedStructure->getUpgradeCost();
            if (upgradeCost > 0) {
                char upgradeLabel[64];
                sprintf(upgradeLabel, "UPGRADE ($%d)", upgradeCost);
                
                bool canAfford = coins >= upgradeCost;
                if (! canAfford) ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.5f);
                
                if (ImGui::Button(upgradeLabel, ImVec2(125, 35))) {
                    if (canAfford) {
                        coins -= upgradeCost;
                        selectedStructure->upgrade();
                        combatLog.add("Structure upgraded!");
                    }
                }
                
                if (!canAfford) ImGui::PopStyleVar();
                ImGui::SameLine();
            }
            
            char sellLabel[64];
            sprintf(sellLabel, "SELL ($%d)", selectedStructure->getSellValue());
            
            if (ImGui::Button(sellLabel, ImVec2(125, 35))) {
                coins += selectedStructure->getSellValue();
                
                sf::Vector2f pos = selectedStructure->getPosition();
                gameMap.setOccupied(pos. x, pos.y, false);
                
                ListNode<Structure*>* node = structures.getHead();
                while (node) {
                    if (node->data == selectedStructure) {
                        delete node->data;
                        structures.remove(node);
                        break;
                    }
                    node = node->next;
                }
                
                selectedStructure = nullptr;
                combatLog.add("Structure sold!");
            }
            
            ImGui::Separator();
        }
        
        ImGui::Dummy(ImVec2(0, 10));
        ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "COMBAT LOG");
        
        ImGui::BeginChild("##CombatLog", ImVec2(255, 140), true);
        for (int i = 0; i < combatLog.getCount(); i++) {
            ImGui::TextWrapped("%s", combatLog.getMessage(i));
        }
        if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY() - 10) {
            ImGui::SetScrollHereY(1.0f);
        }
        ImGui::EndChild();
        
        ImGui::Dummy(ImVec2(0, 10));
        ImGui::Separator();
        ImGui::Dummy(ImVec2(0, 5));
        ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "STATISTICS");
        
        ImGui::Text("Ground Kills: %d", groundKills);
        ImGui::Text("Flying Kills: %d", flyingKills);
        ImGui::Text("Boss Kills: %d", bossKills);
        ImGui::Text("Structures: %d", structures. getSize());
        
        ImGui::End();
    }
    
    void renderStructureButton(const char* name, int cost, StructureType type, const char* desc) {
        char label[128];
        sprintf(label, "%s ($%d)\n%s", name, cost, desc);
        
        bool canAfford = coins >= cost;
        bool isSelected = (selectedBuildType == type);
        
        if (! canAfford) ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.5f);
        if (isSelected) {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.6f, 0.2f, 1.0f));
        }
        
        if (ImGui::Button(label, ImVec2(255, 45))) {
            if (canAfford) {
                selectedBuildType = type;
                if (selectedStructure) {
                    selectedStructure->setShowRange(false);
                    selectedStructure = nullptr;
                }
            }
        }
        
        if (isSelected) ImGui::PopStyleColor();
        if (!canAfford) ImGui::PopStyleVar();
    }
    
    void renderPauseMenu() {
        ImGui::SetNextWindowPos(ImVec2(Constants::WINDOW_WIDTH/2 - 150, Constants::WINDOW_HEIGHT/2 - 100));
        ImGui::SetNextWindowSize(ImVec2(300, 200));
        
        ImGui::Begin("PAUSED", nullptr,
                     ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);
        
        ImGui::Dummy(ImVec2(0, 20));
        
        ImGui::SetCursorPosX(50);
        if (ImGui::Button("RESUME GAME", ImVec2(200, 40))) {
            state = STATE_WAVE_PREP;
        }
        
        ImGui::Dummy(ImVec2(0, 10));
        ImGui::SetCursorPosX(50);
        if (ImGui::Button("MAIN MENU", ImVec2(200, 40))) {
            state = STATE_MENU;
        }
        
        ImGui::Dummy(ImVec2(0, 10));
        ImGui::SetCursorPosX(50);
        if (ImGui::Button("EXIT GAME", ImVec2(200, 40))) {
            window.close();
        }
        
        ImGui::End();
    }
    
    void renderVictory() {
        ImGui::SetNextWindowPos(ImVec2(Constants::WINDOW_WIDTH/2 - 220, Constants::WINDOW_HEIGHT/2 - 180));
        ImGui::SetNextWindowSize(ImVec2(440, 360));
        
        ImGui::Begin("VICTORY!", nullptr,
                     ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);
        
        ImGui::Dummy(ImVec2(0, 10));
        
        ImGui::SetCursorPosX(150);
        ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.2f, 1.0f), "V I C T O R Y !");
        
        ImGui::Dummy(ImVec2(0, 10));
        ImGui::SetCursorPosX(70);
        ImGui::Text("Congratulations! You defended the tower!");
        ImGui::SetCursorPosX(90);
        ImGui::Text("All %d waves have been defeated!", totalWaves);
        
        ImGui::Dummy(ImVec2(0, 15));
        ImGui::Separator();
        ImGui::Dummy(ImVec2(0, 10));
        
        ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "FINAL STATISTICS:");
        ImGui::Text("Total Kills: %d", totalKills);
        ImGui::Text("  Ground: %d", groundKills);
        ImGui::Text("  Flying: %d", flyingKills);
        ImGui::Text("  Bosses: %d", bossKills);
        ImGui::Text("Final Coins: $%d", coins);
        ImGui::Text("Tower HP: %d/%d", towerHP, maxTowerHP);
        ImGui::Text("Structures Built: %d", structures.getSize());
        
        Structure* mvp = nullptr;
        int maxKills = 0;
        ListNode<Structure*>* node = structures. getHead();
        while (node) {
            if (node->data->getTotalKills() > maxKills) {
                maxKills = node->data->getTotalKills();
                mvp = node->data;
            }
            node = node->next;
        }
        
        if (mvp && maxKills > 0) {
            ImGui::Dummy(ImVec2(0, 5));
            ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), 
                              "MVP: %s (Lv. %d) - %d kills! ",
                              getStructureName(mvp->getType()), 
                              mvp->getLevel(), 
                              maxKills);
        }
        
        ImGui::Dummy(ImVec2(0, 20));
        ImGui::SetCursorPosX(120);
        if (ImGui::Button("MAIN MENU", ImVec2(200, 40))) {
            state = STATE_MENU;
        }
        
        ImGui::End();
    }
    
    void renderDefeat() {
        ImGui::SetNextWindowPos(ImVec2(Constants::WINDOW_WIDTH/2 - 200, Constants::WINDOW_HEIGHT/2 - 150));
        ImGui::SetNextWindowSize(ImVec2(400, 300));
        
        ImGui::Begin("GAME OVER", nullptr,
                     ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);
        
        ImGui::Dummy(ImVec2(0, 10));
        
        ImGui::SetCursorPosX(140);
        ImGui::TextColored(ImVec4(1.0f, 0.2f, 0.2f, 1.0f), "G A M E   O V E R");
        
        ImGui::Dummy(ImVec2(0, 10));
        ImGui::SetCursorPosX(90);
        ImGui::Text("Your tower was destroyed on Wave %d", currentWave);
        
        ImGui::Dummy(ImVec2(0, 15));
        ImGui::Separator();
        ImGui::Dummy(ImVec2(0, 10));
        
        ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "STATISTICS:");
        ImGui::Text("Total Kills: %d", totalKills);
        ImGui::Text("  Ground: %d", groundKills);
        ImGui::Text("  Flying: %d", flyingKills);
        ImGui::Text("  Bosses: %d", bossKills);
        ImGui::Text("Waves Survived: %d/%d", currentWave - 1, totalWaves);
        
        ImGui::Dummy(ImVec2(0, 25));
        
        ImGui::SetCursorPosX(55);
        if (ImGui::Button("TRY AGAIN", ImVec2(130, 40))) {
            startGame(mode);
        }
        ImGui::SameLine();
        if (ImGui::Button("MAIN MENU", ImVec2(130, 40))) {
            state = STATE_MENU;
        }
        
        ImGui::End();
    }
};

int main() {
    try {
        Game game;
        game.run();
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    std::cout << "\n";
    std::cout << "+==========================================+\n";
    std::cout << "|  Thanks for playing Marvel Tower Defense!  |\n";
    std::cout << "|  See you next time, Hero!                 |\n";
    std::cout << "+==========================================+\n";
    std::cout << "\n";
    
    return 0;
}