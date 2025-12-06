#pragma once

#include <SFML/Graphics.hpp>
#include <iostream>
#include <cstring>
#include "Constants.hpp"

class TextureManager {
private:
    static const int MAX_TEXTURES = 20;
    sf::Texture textures[MAX_TEXTURES];
    char textureNames[MAX_TEXTURES][32];
    int textureCount;
    
    TextureManager() : textureCount(0) {
        for (int i = 0; i < MAX_TEXTURES; i++) textureNames[i][0] = '\0';
    }

public:
    static TextureManager& getInstance() {
        static TextureManager instance;
        return instance;
    }
    
    TextureManager(const TextureManager&) = delete;
    TextureManager& operator=(const TextureManager&) = delete;
    
    int findTextureIndex(const char* name) const {
        for (int i = 0; i < textureCount; i++) {
            if (strcmp(textureNames[i], name) == 0) return i;
        }
        return -1;
    }
    
    void addTexture(const char* name, const sf::Texture& tex) {
        if (textureCount >= MAX_TEXTURES) return;
        int existing = findTextureIndex(name);
        if (existing >= 0) {
            textures[existing] = tex;
            return;
        }
        textures[textureCount] = tex;
        strcpy(textureNames[textureCount], name);
        textureCount++;
    }
    
    sf::Texture& getTexture(const char* name) {
        int idx = findTextureIndex(name);
        if (idx >= 0) return textures[idx];
        return textures[0];
    }
    
    void createPlaceholders() {
        auto createColorTexture = [this](const char* name, sf::Color color, unsigned int size) {
            sf::Image img;
            img. create(size, size, color);
            for (unsigned int i = 0; i < size; i++) {
                img.setPixel(i, 0, sf::Color::Black);
                img.setPixel(i, size-1, sf::Color::Black);
                img.setPixel(0, i, sf::Color::Black);
                img.setPixel(size-1, i, sf::Color::Black);
            }
            sf::Texture tex;
            tex.loadFromImage(img);
            addTexture(name, tex);
        };
        
        createColorTexture("enemy_ground", sf::Color(139, 69, 19), 40);
        createColorTexture("enemy_flying", sf::Color(100, 149, 237), 40);
        createColorTexture("enemy_boss", sf::Color(178, 34, 34), 64);
        createColorTexture("struct_shooter", sf::Color(34, 139, 34), 50);
        createColorTexture("struct_hawkeye", sf::Color(148, 0, 211), 50);
        createColorTexture("struct_ironman", sf::Color(220, 20, 60), 50);
        createColorTexture("struct_drstrange", sf::Color(255, 140, 0), 50);
        createColorTexture("struct_thor", sf::Color(30, 144, 255), 50);
        createColorTexture("tower", sf::Color(255, 215, 0), 80);
        
        sf::Image mapImg;
        mapImg.create(Constants::GAME_AREA_WIDTH, Constants::WINDOW_HEIGHT, sf::Color(34, 49, 34));
        sf::Texture mapTex;
        mapTex.loadFromImage(mapImg);
        addTexture("map_background", mapTex);
    }
    
    void loadAllAssets() {
        createPlaceholders();
    }
};