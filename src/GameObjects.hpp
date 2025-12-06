#pragma once

#include <SFML/Graphics.hpp>
#include <cmath>
#include <cstring>
#include "Constants.hpp"
#include "TextureManager.hpp"

enum GameState { STATE_MENU, STATE_WAVE_PREP, STATE_WAVE_ACTIVE, STATE_PAUSED, STATE_VICTORY, STATE_DEFEAT };
enum GameMode { MODE_EASY, MODE_EPIC };
enum EnemyType { ENEMY_GROUND, ENEMY_FLYING, ENEMY_BOSS };
enum StructureType { STRUCT_NONE, STRUCT_SHOOTER, STRUCT_HAWKEYE, STRUCT_IRONMAN, STRUCT_DRSTRANGE, STRUCT_THOR };

struct PathPoint {
    float x, y;
    PathPoint() : x(0), y(0) {}
    PathPoint(float px, float py) : x(px), y(py) {}
};

inline float calcDistance(float x1, float y1, float x2, float y2) {
    return sqrt((x2-x1)*(x2-x1) + (y2-y1)*(y2-y1));
}

inline const char* getStructureName(StructureType type) {
    switch(type) {
        case STRUCT_SHOOTER: return "Shooter";
        case STRUCT_HAWKEYE: return "Hawkeye";
        case STRUCT_IRONMAN: return "Iron Man";
        case STRUCT_DRSTRANGE: return "Dr Strange";
        case STRUCT_THOR: return "Thor";
        default: return "None";
    }
}

inline int getStructureCost(StructureType type) {
    switch(type) {
        case STRUCT_SHOOTER: return Constants::SHOOTER_COST;
        case STRUCT_HAWKEYE: return Constants::HAWKEYE_COST;
        case STRUCT_IRONMAN: return Constants::IRONMAN_COST;
        case STRUCT_DRSTRANGE: return Constants::DRSTRANGE_COST;
        case STRUCT_THOR: return Constants::THOR_COST;
        default: return 0;
    }
}

class Enemy : public sf::Drawable {
private:
    int id;
    EnemyType type;
    sf::Sprite sprite;
    sf::RectangleShape healthBarBg;
    sf::RectangleShape healthBarFill;
    int maxHealth, currentHealth, coinReward, towerDamage;
    float moveSpeed;
    bool alive;
    PathPoint path[Constants::MAX_PATH_LENGTH];
    int pathLength, pathIndex;

public:
    Enemy() : id(0), type(ENEMY_GROUND), maxHealth(50), currentHealth(50),
              coinReward(10), towerDamage(10), moveSpeed(80.0f),
              alive(true), pathLength(0), pathIndex(0) {}
    
    void initialize(int enemyId, EnemyType enemyType, int wave) {
        id = enemyId;
        type = enemyType;
        alive = true;
        pathIndex = 0;
        
        float waveMultiplier = 1.0f + (wave - 1) * 0.15f;
        const char* textureName = "enemy_ground";
        
        switch(type) {
            case ENEMY_GROUND:
                textureName = "enemy_ground";
                maxHealth = (int)(50 * waveMultiplier);
                coinReward = Constants::GROUND_KILL_REWARD;
                towerDamage = 10;
                moveSpeed = 80.0f;
                break;
            case ENEMY_FLYING:
                textureName = "enemy_flying";
                maxHealth = (int)(30 * waveMultiplier);
                coinReward = Constants::FLYING_KILL_REWARD;
                towerDamage = 15;
                moveSpeed = 120.0f;
                break;
            case ENEMY_BOSS:
                textureName = "enemy_boss";
                maxHealth = (int)(500 * waveMultiplier);
                coinReward = Constants::BOSS_KILL_REWARD;
                towerDamage = 50;
                moveSpeed = 40.0f;
                break;
        }
        
        currentHealth = maxHealth;
        sprite.setTexture(TextureManager::getInstance(). getTexture(textureName));
        sf::FloatRect bounds = sprite.getLocalBounds();
        sprite.setOrigin(bounds.width / 2, bounds.height / 2);
        
        float barWidth = (type == ENEMY_BOSS) ? 60.0f : 40.0f;
        healthBarBg.setSize(sf::Vector2f(barWidth, 6));
        healthBarBg.setFillColor(sf::Color(50, 50, 50));
        healthBarFill.setSize(sf::Vector2f(barWidth - 2, 4));
        healthBarFill.setFillColor(sf::Color::Green);
    }
    
    void setPath(PathPoint* newPath, int length) {
        pathLength = (length > Constants::MAX_PATH_LENGTH) ?  Constants::MAX_PATH_LENGTH : length;
        for (int i = 0; i < pathLength; i++) path[i] = newPath[i];
        if (pathLength > 0) sprite.setPosition(path[0]. x, path[0].y);
    }
    
    void update(float deltaTime) {
        if (!alive) return;
        if (pathIndex < pathLength) {
            float targetX = path[pathIndex].x;
            float targetY = path[pathIndex]. y;
            sf::Vector2f current = sprite.getPosition();
            float dx = targetX - current.x;
            float dy = targetY - current.y;
            float distance = sqrt(dx*dx + dy*dy);
            
            if (distance < 5.0f) {
                pathIndex++;
            } else {
                dx /= distance;
                dy /= distance;
                sprite. move(dx * moveSpeed * deltaTime, dy * moveSpeed * deltaTime);
            }
        }
        
        sf::Vector2f pos = sprite.getPosition();
        float barWidth = healthBarBg.getSize().x;
        healthBarBg.setPosition(pos.x - barWidth/2, pos.y - 35);
        healthBarFill.setPosition(pos.x - barWidth/2 + 1, pos.y - 34);
        
        float healthPercent = (float)currentHealth / maxHealth;
        healthBarFill.setSize(sf::Vector2f((barWidth - 2) * healthPercent, 4));
        
        if (healthPercent > 0.6f) healthBarFill.setFillColor(sf::Color::Green);
        else if (healthPercent > 0.3f) healthBarFill.setFillColor(sf::Color::Yellow);
        else healthBarFill.setFillColor(sf::Color::Red);
    }
    
    void takeDamage(int damage) {
        currentHealth -= damage;
        if (currentHealth <= 0) { currentHealth = 0; alive = false; }
    }
    
    bool hasReachedEnd() const { return pathIndex >= pathLength; }
    bool isAlive() const { return alive; }
    int getCoinReward() const { return coinReward; }
    int getTowerDamage() const { return towerDamage; }
    EnemyType getType() const { return type; }
    sf::Vector2f getPosition() const { return sprite. getPosition(); }
    
    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override {
        if (alive) {
            target.draw(sprite, states);
            target.draw(healthBarBg, states);
            target. draw(healthBarFill, states);
        }
    }
};

class Structure : public sf::Drawable {
private:
    int id;
    StructureType type;
    int level, damage, totalKills, totalDamageDealt;
    float range, fireRate, cooldown;
    bool canTargetAir, canTargetGround, showRange;
    sf::Sprite sprite;
    sf::CircleShape rangeIndicator;
    sf::VertexArray attackLine;
    float attackVisualTimer;

public:
    Structure() : id(0), type(STRUCT_NONE), level(1), damage(0), totalKills(0), totalDamageDealt(0),
                  range(0), fireRate(1.0f), cooldown(0), canTargetAir(false), canTargetGround(true),
                  showRange(false), attackVisualTimer(0), attackLine(sf::Lines, 2) {}
    
    void initialize(int structId, StructureType structType, float posX, float posY) {
        id = structId;
        type = structType;
        level = 1;
        cooldown = 0;
        totalKills = 0;
        totalDamageDealt = 0;
        
        const char* textureName = "struct_shooter";
        switch(type) {
            case STRUCT_SHOOTER:
                textureName = "struct_shooter";
                damage = 10; range = 100.0f; fireRate = 1.5f;
                canTargetAir = false; canTargetGround = true;
                break;
            case STRUCT_HAWKEYE:
                textureName = "struct_hawkeye";
                damage = 12; range = 140.0f; fireRate = 1.0f;
                canTargetAir = false; canTargetGround = true;
                break;
            case STRUCT_IRONMAN:
                textureName = "struct_ironman";
                damage = 20; range = 160.0f; fireRate = 0.8f;
                canTargetAir = true; canTargetGround = true;
                break;
            case STRUCT_DRSTRANGE:
                textureName = "struct_drstrange";
                damage = 35; range = 150.0f; fireRate = 0.5f;
                canTargetAir = true; canTargetGround = true;
                break;
            case STRUCT_THOR:
                textureName = "struct_thor";
                damage = 50; range = 250.0f; fireRate = 1.4f;
                canTargetAir = true; canTargetGround = true;
                break;
            default: break;
        }
        
        sprite. setTexture(TextureManager::getInstance(). getTexture(textureName));
        sf::FloatRect bounds = sprite.getLocalBounds();
        sprite.setOrigin(bounds.width / 2, bounds.height / 2);
        sprite.setPosition(posX, posY);
        
        rangeIndicator.setRadius(range);
        rangeIndicator.setOrigin(range, range);
        rangeIndicator.setPosition(posX, posY);
        rangeIndicator.setFillColor(sf::Color(100, 200, 100, 40));
        rangeIndicator.setOutlineColor(sf::Color(100, 200, 100, 150));
        rangeIndicator.setOutlineThickness(2);
    }
    
    void updateCooldown(float deltaTime) {
        if (cooldown > 0) cooldown -= deltaTime;
        if (attackVisualTimer > 0) attackVisualTimer -= deltaTime;
    }
    
    bool canFire() const { return cooldown <= 0; }
    void fire() { cooldown = 1.0f / fireRate; }
    
    bool isInRange(sf::Vector2f targetPos) const {
        sf::Vector2f pos = sprite.getPosition();
        return calcDistance(pos.x, pos.y, targetPos.x, targetPos.y) <= range;
    }
    
    bool canTarget(EnemyType enemyType) const {
        if (enemyType == ENEMY_FLYING) return canTargetAir;
        return canTargetGround;
    }
    
    void setAttackVisual(sf::Vector2f targetPos) {
        attackLine[0].position = sprite.getPosition();
        attackLine[1].position = targetPos;
        attackLine[0].color = sf::Color::Yellow;
        attackLine[1].color = sf::Color::Yellow;
        attackVisualTimer = 0.15f;
    }
    
    bool upgrade() {
        if (level >= 3) return false;
        level++;
        damage = (int)(damage * 1.3f);
        range *= 1.15f;
        rangeIndicator.setRadius(range);
        rangeIndicator.setOrigin(range, range);
        return true;
    }
    
    int getUpgradeCost() const {
        if (level >= 3) return 0;
        return (int)(getStructureCost(type) * 0.5f * level);
    }
    
    int getSellValue() const {
        return (int)(getStructureCost(type) * Constants::SELL_REFUND_PERCENT * level);
    }
    
    void addKill() { totalKills++; }
    void addDamage(int dmg) { totalDamageDealt += dmg; }
    void setShowRange(bool show) { showRange = show; }
    
    int getDamage() const { return damage; }
    float getRange() const { return range; }
    int getLevel() const { return level; }
    int getTotalKills() const { return totalKills; }
    int getTotalDamageDealt() const { return totalDamageDealt; }
    StructureType getType() const { return type; }
    sf::Vector2f getPosition() const { return sprite.getPosition(); }
    sf::FloatRect getBounds() const { return sprite.getGlobalBounds(); }
    
    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override {
        if (showRange) target.draw(rangeIndicator, states);
        target.draw(sprite, states);
        if (attackVisualTimer > 0) target.draw(attackLine, states);
    }
};