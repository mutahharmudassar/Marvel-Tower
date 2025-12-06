#pragma once

#include <SFML/Graphics.hpp>
#include "Constants.hpp"
#include "TextureManager.hpp"
#include "GameObjects.hpp"

class GameMap : public sf::Drawable {
private:
    sf::Sprite backgroundSprite;
    sf::Sprite towerSprite;
    PathPoint paths[3][Constants::MAX_PATH_LENGTH];
    int pathLengths[3];
    bool occupied[Constants::GRID_ROWS][Constants::GRID_COLS];
    sf::RectangleShape gridCells[Constants::GRID_ROWS][Constants::GRID_COLS];
    bool showGrid;
    sf::RectangleShape placementPreview;
    bool showPreview, previewValid;

public:
    GameMap() : showGrid(true), showPreview(false), previewValid(false) {
        for (int y = 0; y < Constants::GRID_ROWS; y++) {
            for (int x = 0; x < Constants::GRID_COLS; x++) {
                occupied[y][x] = false;
                gridCells[y][x].setSize(sf::Vector2f(Constants::CELL_SIZE - 2, Constants::CELL_SIZE - 2));
                gridCells[y][x].setPosition(x * Constants::CELL_SIZE + 1, y * Constants::CELL_SIZE + 1);
                gridCells[y][x].setFillColor(sf::Color(0, 255, 0, 20));
                gridCells[y][x].setOutlineColor(sf::Color(255, 255, 255, 30));
                gridCells[y][x].setOutlineThickness(1);
            }
        }
        placementPreview.setSize(sf::Vector2f(Constants::CELL_SIZE - 4, Constants::CELL_SIZE - 4));
        placementPreview.setOutlineThickness(2);
        for (int i = 0; i < 3; i++) pathLengths[i] = 0;
    }
    
    void initialize() {
        backgroundSprite.setTexture(TextureManager::getInstance().getTexture("map_background"));
        towerSprite.setTexture(TextureManager::getInstance().getTexture("tower"));
        sf::FloatRect bounds = towerSprite.getLocalBounds();
        towerSprite.setOrigin(bounds.width / 2, bounds.height / 2);
        towerSprite.setPosition(Constants::TOWER_X, Constants::TOWER_Y);
        
        for (int y = 0; y < Constants::GRID_ROWS; y++)
            for (int x = 0; x < Constants::GRID_COLS; x++)
                occupied[y][x] = false;
        
        float topPath[][2] = {{-50,100},{100,100},{200,100},{300,130},{400,170},{500,220},{600,260},{700,300},{800,330},{Constants::TOWER_X,Constants::TOWER_Y}};
        pathLengths[0] = 10;
        for (int i = 0; i < 10; i++) paths[0][i] = PathPoint(topPath[i][0], topPath[i][1]);
        
        float midPath[][2] = {{-50,350},{100,350},{200,350},{300,350},{400,350},{500,350},{600,350},{700,350},{800,350},{Constants::TOWER_X,Constants::TOWER_Y}};
        pathLengths[1] = 10;
        for (int i = 0; i < 10; i++) paths[1][i] = PathPoint(midPath[i][0], midPath[i][1]);
        
        float botPath[][2] = {{-50,600},{100,600},{200,600},{300,570},{400,530},{500,480},{600,440},{700,400},{800,370},{Constants::TOWER_X,Constants::TOWER_Y}};
        pathLengths[2] = 10;
        for (int i = 0; i < 10; i++) paths[2][i] = PathPoint(botPath[i][0], botPath[i][1]);
        
        markPathsOccupied();
        markTowerOccupied();
    }
    
    void markPathsOccupied() {
        for (int lane = 0; lane < 3; lane++) {
            for (int i = 0; i < pathLengths[lane]; i++) {
                int gx = (int)(paths[lane][i].x / Constants::CELL_SIZE);
                int gy = (int)(paths[lane][i].y / Constants::CELL_SIZE);
                for (int dy = -1; dy <= 1; dy++) {
                    for (int dx = -1; dx <= 1; dx++) {
                        int x = gx + dx, y = gy + dy;
                        if (isValidGrid(x, y)) occupied[y][x] = true;
                    }
                }
            }
        }
    }
    
    void markTowerOccupied() {
        int tx = (int)(Constants::TOWER_X / Constants::CELL_SIZE);
        int ty = (int)(Constants::TOWER_Y / Constants::CELL_SIZE);
        for (int dy = -1; dy <= 1; dy++)
            for (int dx = -1; dx <= 1; dx++) {
                int x = tx + dx, y = ty + dy;
                if (isValidGrid(x, y)) occupied[y][x] = true;
            }
    }
    
    bool isValidGrid(int x, int y) const {
        return x >= 0 && x < Constants::GRID_COLS && y >= 0 && y < Constants::GRID_ROWS;
    }
    
    bool canPlaceAt(float worldX, float worldY) const {
        int gx = (int)(worldX / Constants::CELL_SIZE);
        int gy = (int)(worldY / Constants::CELL_SIZE);
        if (! isValidGrid(gx, gy)) return false;
        return !occupied[gy][gx];
    }
    
    void snapToGrid(float worldX, float worldY, float& outX, float& outY) const {
        int gx = (int)(worldX / Constants::CELL_SIZE);
        int gy = (int)(worldY / Constants::CELL_SIZE);
        outX = gx * Constants::CELL_SIZE + Constants::CELL_SIZE / 2;
        outY = gy * Constants::CELL_SIZE + Constants::CELL_SIZE / 2;
    }
    
    void setOccupied(float worldX, float worldY, bool val) {
        int gx = (int)(worldX / Constants::CELL_SIZE);
        int gy = (int)(worldY / Constants::CELL_SIZE);
        if (isValidGrid(gx, gy)) occupied[gy][gx] = val;
    }
    
    void updatePreview(float mouseX, float mouseY) {
        float snappedX, snappedY;
        snapToGrid(mouseX, mouseY, snappedX, snappedY);
        placementPreview.setPosition(snappedX - Constants::CELL_SIZE/2 + 2, snappedY - Constants::CELL_SIZE/2 + 2);
        previewValid = canPlaceAt(mouseX, mouseY);
        if (previewValid) {
            placementPreview.setFillColor(sf::Color(0, 255, 0, 100));
            placementPreview. setOutlineColor(sf::Color::Green);
        } else {
            placementPreview. setFillColor(sf::Color(255, 0, 0, 100));
            placementPreview.setOutlineColor(sf::Color::Red);
        }
    }
    
    PathPoint* getPath(int lane, int& length) {
        if (lane < 0) lane = 0;
        if (lane > 2) lane = 2;
        length = pathLengths[lane];
        return paths[lane];
    }
    
    void setShowGrid(bool show) { showGrid = show; }
    void setShowPreview(bool show) { showPreview = show; }
    
    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override {
        target.draw(backgroundSprite, states);
        
        for (int lane = 0; lane < 3; lane++) {
            if (pathLengths[lane] < 2) continue;
            sf::VertexArray pathLine(sf::LineStrip, pathLengths[lane]);
            for (int i = 0; i < pathLengths[lane]; i++) {
                pathLine[i].position = sf::Vector2f(paths[lane][i].x, paths[lane][i].y);
                pathLine[i]. color = sf::Color(200, 150, 100, 100);
            }
            target.draw(pathLine, states);
        }
        
        if (showGrid) {
            for (int y = 0; y < Constants::GRID_ROWS; y++)
                for (int x = 0; x < Constants::GRID_COLS; x++)
                    if (! occupied[y][x]) target.draw(gridCells[y][x], states);
        }
        
        target.draw(towerSprite, states);
        if (showPreview) target.draw(placementPreview, states);
    }
};