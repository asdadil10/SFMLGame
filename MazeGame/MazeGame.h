// MazeGame.h : Include file for standard system include files,
// or project specific include files.

#pragma once

#include <iostream>
#include <time.h>
#include <string>
#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <vector>

struct Enemy {
    sf::CircleShape enemy;
    bool movingLeft = true;
    Enemy(){
        enemy.setFillColor(sf::Color::Red);
        enemy.setPosition({500,100});
        enemy.setRadius(32);
    }
    void update(double dt) {
        sf::Vector2f pos = enemy.getPosition();
        if (0 < pos.x && pos.x < 80) movingLeft = true;
        if (1200 < pos.x && pos.x < 1280) movingLeft = false;
        if (movingLeft) {
            enemy.setPosition(pos + sf::Vector2f({ float_t(255 * dt),0 }));
            enemy.setFillColor({ 255,128,0 });
        }
        else {
            enemy.setPosition(pos + sf::Vector2f({ -float_t(255 * dt),0 }));
            enemy.setFillColor({ 255,0,128 });
        }
    }
    bool checkCollision(sf::Vector2f pos) {
        if (enemy.getGlobalBounds().contains(pos))
            return true;
        else 
            return false;
    }
};

struct Projectile {
    sf::CircleShape bullet;
    Projectile() {
        bullet.setRadius(6);
        bullet.setPointCount(9);
        bullet.setFillColor(sf::Color{ 164,64,64,255 });
        bullet.setOutlineThickness(2);
        bullet.setOutlineColor(sf::Color{ 128,32,64,196 });
        bullet.setOrigin({6, 6});
    }
    void update(double dt) {
        bullet.setPosition(bullet.getPosition() + sf::Vector2f({ 0,-float_t(555*dt) }));
    }
};

void upscale(sf::Image* image)
{
    sf::Color pixels[32][32];
    uint8_t* p = new uint8_t[32 * 32 * 4];
    int px= 0;
    for (uint16_t i = 0; i < 8; i++) // y loop
    {
        for (uint8_t y = 0; y < 4; y++) // for repeating y-pixels 4 times
        {
            for (uint16_t j = 0; j < 8; j++) // x loop
            {
                for (uint8_t x = 0; x < 4; x++) // for repeating x-pixels 4 times
                {
                    pixels[j * 4 + x][i * 4 + x] = image->getPixel(sf::Vector2u{ j,i });
                    p[px] = pixels[j * 4 + x][i * 4 + x].r; px++;
                    p[px] = pixels[j * 4 + x][i * 4 + x].g; px++;
                    p[px] = pixels[j * 4 + x][i * 4 + x].b; px++;
                    p[px] = pixels[j * 4 + x][i * 4 + x].a; px++;
                }
            }
        }
    }
    image->resize({ 32,32 }, p);
    delete[] p;
}
sf::Color** pixelToColorArr(sf::Vector2u dimensions, const uint8_t* pixels)
{
    sf::Color** cPixels = new sf::Color * [dimensions.y];
    for (int i = 0; i < dimensions.y; i++) {
        cPixels[i] = new sf::Color[dimensions.x];
    }
    if (pixels == nullptr)
        return cPixels;
    int count = 0;
    for (int i = 0; i < dimensions.y; i++) {
        for (int j = 0; j < dimensions.x; j++) {
            cPixels[i][j].r = pixels[count]; count++;
            cPixels[i][j].g = pixels[count]; count++;
            cPixels[i][j].b = pixels[count]; count++;
            cPixels[i][j].a = pixels[count]; count++;
        }
    }
    return cPixels;
}

uint8_t* colorToPixelArr(sf::Vector2u dimensions, sf::Color** cPixels)
{
    uint8_t* pixels = new uint8_t[dimensions.x * dimensions.y * 4];
    unsigned int count = 0;
    for (int i = 0; i < dimensions.y; i++) {
        for (int j = 0; j < dimensions.x; j++) {
            pixels[count] = cPixels[i][j].r; count++;
            pixels[count] = cPixels[i][j].g; count++;
            pixels[count] = cPixels[i][j].b; count++;
            pixels[count] = cPixels[i][j].a; count++;
        }
    }
    return pixels;
}
sf::Image* createTiles(const sf::Vector2u dimensions, const uint8_t* pixelData) //32x32 tile
{
    sf::Image* image = new sf::Image;
    sf::Color** Cx = pixelToColorArr(dimensions, nullptr);
    double tileX, tileY;
    tileY = dimensions.y / 32;
    tileX = dimensions.x / 32;
    sf::Color** cPixels = pixelToColorArr({ 32,32 }, pixelData);
    for (int i = 0; i < dimensions.y; i++) {
        for (int j = 0; j < dimensions.x; j++) {
            Cx[i][j] = cPixels[i % 32][j % 32];
        }
    }
    uint8_t* p = colorToPixelArr(dimensions, Cx);
    
    for (int i = 0; i < dimensions.y; i++) { delete[] Cx[i]; } delete[] Cx;
    for (int i = 0; i < 32; i++) { delete[]cPixels[i]; } delete[]cPixels;

    image->resize(dimensions, p);
    return image;
}

double deltaT()
{
    static sf::Clock clock;
    double dt = clock.getElapsedTime().asSeconds();
    clock.restart();
    return dt;
}

void GameOver(sf::RenderWindow * window,bool win)
{
    const sf::Font font("C:/Users/HP/Downloads/Helvetica.ttf");
    sf::Text text(font, "You Won!", 96);
    if (!win) text.setString("You Lost!");
    text.setOrigin(text.getGlobalBounds().getCenter());
    text.setPosition({ (float_t)(window->getSize().x / 2),(float_t)(window->getSize().y / 2) });
    window->draw(text);
}