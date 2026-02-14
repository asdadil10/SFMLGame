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
#include <math.h>
#include <algorithm>
float BulletSpeed = 555; // pixels per second

void upscale(sf::Image*);




class ProgressBar : public sf::RectangleShape {
	const int max = 100;
    float progress = 0;
	sf::RectangleShape bgBar;
    void update() {
        setSize({ bgBar.getSize().x * (progress / 100),bgBar.getSize().y });
	}
public:
    ProgressBar(float_t width = 150, float_t height = 25, float progress = 100.f) :progress(progress) {
        setSize({ width,height });
		bgBar.setSize(getSize());
        setOrigin({ 0.f,height / 2.f }); //origin in left middle side
		bgBar.setOrigin(getOrigin());
        setFillColor({ 255,64,0 });
		bgBar.setFillColor({ 64,64,64 });
    }
    void setProgress(float progress) {
        this->progress = progress;
        update();
	}
    void draw(sf::RenderWindow* w) {
        if (bgBar.getPosition() != getPosition()) {
            bgBar.setSize({ bgBar.getSize().x,getSize().y});
            bgBar.setPosition(getPosition());
        }
        w->draw(bgBar);
        w->draw(*this);
	}
};


struct Entity : sf::RectangleShape {
    sf::Image* Image;
    sf::Texture* Tex;
    short health = 100;
	sf::ConvexShape collisionShape;
    bool Alive = true;
    Entity(sf::Vector2f size, std::string ImagePath) {
        setSize(size);
        Image = new sf::Image(ImagePath);
        Tex = new sf::Texture(*Image);
        upscale(Image);
        setTexture(Tex);
        setOrigin({ size.x / 2,size.y / 2 });
        collisionShape.setOrigin(getOrigin());
        collisionShape.setPointCount(4);
		collisionShape.setPoint(0, { 0,size.y/2 });
		collisionShape.setPoint(1, { size.x/2,0 });
		collisionShape.setPoint(2, { size.x,size.y/2 });
		collisionShape.setPoint(3, { size.x/2,size.y });
        health = 100;
    }
    //Check collision with a point, default dmg is 0
    bool checkCollision(sf::Vector2f pos, int dmg = 0,bool Convex = false ) {
        sf::FloatRect bounds;
        if(!Convex)
        bounds = getGlobalBounds();
        else
        {
            collisionShape.setPosition(getPosition());
            bounds = collisionShape.getGlobalBounds();
        }
        if (bounds.contains(pos)) {
            health -= dmg;
            if (health < 1) {
                Alive = false;
                health = 0;
            }
            return true;
        }
        else {
            return false;
        }
    }
    void setHealth(uint8_t health) { this->health = health; }
    uint8_t getHealth() { return health; }
    ~Entity() {
        delete Image;
        delete Tex;
    }
};

struct Player : Entity {
	sf::CircleShape booster;
    Player() : Entity({ 32,32 }, "src/player.png") {
        health = 100;
		booster.setPointCount(3);
        booster.setRadius(8);
        booster.setOrigin({ 8,24 });
        booster.rotate(sf::degrees(180));
		booster.setFillColor({ 255, 165, 0 });
    }
    bool checkCollision(sf::Vector2f pos) { return Entity::checkCollision(pos, 20); }
};

struct Enemy : Entity{
	bool movingLeft = true;
    Enemy() : Entity({ 64,64 }, "src/EnemySprite.png") {
        health = 100;
		setPosition({ 1200,200 });
	}
	//enemy move left and right, and rotate when moving
    void update(double dt) {
		sf::Vector2f pos = getPosition();
		static float time = 0;
        time += dt;
        if (time > 0.25) {
            rotate(sf::degrees(BulletSpeed / (dt*60*24)));
            time = 0;
        }
        if (0 < pos.x && pos.x < 80) movingLeft = true;
        if (1200 < pos.x && pos.x < 1280) movingLeft = false;
        if (movingLeft) {
            setPosition(pos + sf::Vector2f({ float_t(255 * dt),0 }));
        }
        else {
            setPosition(pos + sf::Vector2f({ -float_t(255 * dt),0 }));
            
		}
    }
    bool checkCollision(sf::Vector2f pos) { return Entity::checkCollision(pos, 4 ,true); }
};

struct Bullet : sf::CircleShape {
    sf::Vector2f direction; 
    int speed; //speed pixels perscon 
    Bullet(const sf::Vector2f& dir = sf::Vector2f(0.f, -1.f)) : direction(dir) {
        setRadius(6);
        setPointCount(10);
        setFillColor({ 164, 64, 64, 255 });
        setOutlineThickness(2);
        setOutlineColor({ 128, 32, 64, 196 });
        setOrigin({ 6, 6 });
        speed = BulletSpeed;
        direction = direction.normalized();
    }
    void update(double dt) {
        speed = BulletSpeed;
        setPosition(getPosition() + direction * float(speed * dt));
    }
};

struct HSV {
    float h; // 0 - 360
    float s; // 0.0 - 1.0
    float v; // 0.0 - 1.0
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

void GameOver(sf::RenderWindow* window, bool win, const sf::Font& font)
{
    // create per-call text (cheap enough)
    sf::Text text(font, win ? "You Won!" : "You Lost!", 96);

    // better use local bounds for origin computation
    sf::FloatRect lb = text.getLocalBounds();
    text.setOrigin(lb.getCenter());

    sf::Vector2f wCenter{ window->getSize().x / 2.f, window->getSize().y / 2.f };
    text.setPosition(wCenter);
    window->draw(text);
}

sf::Color hsvToRgb(HSV hsv) {
    // Wrap hue, clamp sat/val
    int hue = hsv.h;
    float saturation = hsv.s;
    float value = hsv.v;
    hue = hue % 360;
    if (hue < 0) hue += 360;
    saturation = std::max(0.0f, std::min(1.0f, saturation));
    value = std::max(0.0f, std::min(1.0f, value));

    float h = hue / 60.0f;
    int i = static_cast<int>(h);
    float f = h - i;
    float p = value * (1.0f - saturation);
    float q = value * (1.0f - saturation * f);
    float t = value * (1.0f - saturation * (1.0f - f));

    float r = 0, g = 0, b = 0;
    switch (i) {
    case 0: r = value; g = t; b = p; break;
    case 1: r = q; g = value; b = p; break;
    case 2: r = p; g = value; b = t; break;
    case 3: r = p; g = q; b = value; break;
    case 4: r = t; g = p; b = value; break;
    default: r = value; g = p; b = q; break;
    }

    return sf::Color(
        static_cast<uint8_t>(r * 255),
        static_cast<uint8_t>(g * 255),
        static_cast<uint8_t>(b * 255)
    );
}

HSV rgbToHsv(sf::Color color) {
    HSV out;
    float r = color.r / 255.0f;
    float g = color.g / 255.0f;
    float b = color.b / 255.0f;

    float minVal = std::min({ r, g, b });
    float maxVal = std::max({ r, g, b });
    float delta = maxVal - minVal;

    // Value (Brightness)
    out.v = maxVal;

    // Saturation
    if (maxVal > 0.0f) {
        out.s = (delta / maxVal);
    }
    else {
        // R = G = B = 0
        out.s = 0.0f;
        out.h = 0.0f; // Hue is undefined, but usually set to 0
        return out;
    }

    // Hue
    if (delta == 0.0f) {
        out.h = 0.0f; // Grayscale
    }
    else {
        if (maxVal == r) {
            out.h = 60.0f * (fmod(((g - b) / delta), 6.0f));
        }
        else if (maxVal == g) {
            out.h = 60.0f * (((b - r) / delta) + 2.0f);
        }
        else { // maxVal == b
            out.h = 60.0f * (((r - g) / delta) + 4.0f);
        }

        if (out.h < 0.0f) {
            out.h += 360.0f;
        }
    }

    return out;
}

sf::Texture* texShade(bool movedLeft, const sf::Texture* tex, float deltaX) {
    if (!tex) return nullptr;
    sf::Image img = tex->copyToImage();
    sf::Vector2u size = img.getSize();
    if (size.x == 0 || size.y == 0) return nullptr;

    deltaX = std::abs(deltaX);
    //if (deltaX < 1e-4f) deltaX = 1.0f; // avoid division by zero / huge factors

    std::vector<float> width(size.x);

    // build a simple left->right gradient (1..something) and modulate by deltaX
    for (unsigned int x = 0; x < size.x; ++x) {
        float gradient = 1.0f - (float)x / float(std::max<unsigned int>(1, size.x - 1)); // 1..0 [1-(0..1)]
         //adjust constants to taste; clamp to avoid extreme brightening
        width[x] = std::clamp(1.0f + gradient * (deltaX * 0.5f + 1.0f)/4, 0.0f, 4.0f);
    }
    
    if (movedLeft) std::reverse(width.begin(), width.end());

    for (unsigned int y = 0; y < size.y; ++y) {
        for (unsigned int x = 0; x < size.x; ++x) {
            sf::Color c = img.getPixel({x, y});
            HSV h = rgbToHsv(c);
            h.v = std::clamp(h.v * width[x], 0.0f, 1.0f);
            img.setPixel({ x, y }, { hsvToRgb(h).r,hsvToRgb(h).g,hsvToRgb(h).b,c.a });
        }
    }

    return new sf::Texture(img);
}