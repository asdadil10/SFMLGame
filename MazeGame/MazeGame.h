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
#include <fstream>
// A global variable to represent the speed of bullets in the game, measured in pixels per second. This variable can be used throughout the code to control how fast bullets move across the screen.
float BulletSpeed = 555; // pixels per second

void upscale(sf::Image*);
// A simple structure to represent sound effects in the game, including sound buffers and sound objects for music, shooting, enemy shooting, final shots, player death, enemy death, and hits. The constructor loads the sound files and initializes the sound objects, while the destructor cleans up the dynamically allocated resources.
struct SFX {
    sf::SoundBuffer musicB, shotB, enemyShotB, finalShotB, dieB, enemyDieB, hitB;
    sf::Sound* music = nullptr, * shot = nullptr, * enemyShot = nullptr, * finalShot = nullptr, * die = nullptr, * enemyDie = nullptr, * hit = nullptr;
    SFX() {
        if (!
            musicB.loadFromFile("src/music.mp3")
            || !shotB.loadFromFile("src/shot.wav")
            || !enemyShotB.loadFromFile("src/enemy.wav")
            || !finalShotB.loadFromFile("src/last hit.wav")
            || !dieB.loadFromFile("src/playerdead.wav")
            || !enemyDieB.loadFromFile("src/enemydead.mp3")
            || !hitB.loadFromFile("src/hit.mp3")
            ) {
            std::cerr << "Error loading sound files" << std::endl;
        }
        else {
            music = new sf::Sound(musicB);
            shot = new sf::Sound(shotB);
            enemyShot = new sf::Sound(enemyShotB);
            enemyShot->setVolume(50);
            finalShot = new sf::Sound(finalShotB);
            die = new sf::Sound(dieB);
            enemyDie = new sf::Sound(enemyDieB);
            hit = new sf::Sound(hitB);
            music->setLooping(true);
        }
    }
    ~SFX() {
        delete music;
        delete shot;
        delete enemyShot;
        delete finalShot;
        delete die;
        delete enemyDie;
        delete hit;
    }
};
// A simple structure to represent colors in the HSV color space, with `h` representing the hue (0-360 degrees), `s` representing the saturation (0.0-1.0), and `v` representing the value or brightness (0.0-1.0).
struct HSV {
    float h; // 0 - 360
    float s; // 0.0 - 1.0
    float v; // 0.0 - 1.0
};
// A simple progress bar class that inherits from `sf::RectangleShape` and includes a background bar. The constructor initializes the progress bar's appearance and sets its initial progress. The `setProgress` method updates the progress value and adjusts the size of the progress bar accordingly. The `draw` method renders both the background bar and the progress bar on the provided render window.
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
// A simple entity class that inherits from `sf::RectangleShape` and includes an `sf::Image`, an `sf::Texture`, a health variable, a collision shape, and an alive status. The constructor initializes the entity's appearance and sets its health. The `checkCollision` method checks for collisions with a point, applying damage if a collision occurs. The destructor cleans up the dynamically allocated image and texture resources.
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
// A simple player class that inherits from `Entity` and includes a `sf::CircleShape` member for representing a booster. The constructor initializes the player's appearance and sets its health. The `checkCollision` method checks for collisions with a point, applying damage if a collision occurs.
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
// A simple enemy class that inherits from `Entity` and includes a `movingLeft` boolean to determine the direction of movement. The constructor initializes the enemy's appearance and sets its initial position. The `update` method moves the enemy left and right, rotating it based on the elapsed time `dt`. The enemy changes direction when it reaches certain x-coordinate boundaries. The `checkCollision` method checks for collisions with a point, applying damage if a collision occurs.
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
// A simple bullet class that inherits from `sf::CircleShape` and includes a `direction` vector and a `speed` variable. The constructor initializes the bullet's appearance and sets its speed based on a global `BulletSpeed` variable. The `update` method moves the bullet in the direction of its `direction` vector, scaled by its speed and the elapsed time `dt`. The bullet is represented as a circle with a specific radius, fill color, outline thickness, and outline color.
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
// A simple button class that inherits from `sf::RectangleShape` and includes a `sf::Text` member for displaying text on the button. The button has default and hover colors for both the fill and text, which can be set using the provided methods. The `hover` method changes the button's appearance based on whether it is being hovered over, and the `draw` method renders both the button and its text to a given `sf::RenderWindow`. The constructor initializes the button's size, text, and default colors.
struct Button : sf::RectangleShape {
    sf::Text text;
    sf::Color defaultFill = { 64,64,64,255 }, defaultText = { 0,128,0,255 };
    sf::Color hoverFill = { 0,128,0,255 }, hoverText = { 64,255,64,255 };
    Button(const sf::Font& font, const std::string& str, const sf::Vector2f& size) : sf::RectangleShape(size), text(font, str, size.y / 1.25) {
        setOrigin(getSize() / 2.f);
        text.setOrigin(text.getLocalBounds().getCenter());
        text.setPosition(getPosition());
        setFillColor({ 64,64,64,255 });
        text.setFillColor({ 255,0,0,255 });
    }
    void setPos(const sf::Vector2f& pos) {
        sf::RectangleShape::setPosition(pos);
        text.setPosition(pos);
    }
    void setDefaultColors(const sf::Color& fill, const sf::Color& textColor) {
        defaultFill = fill;
        defaultText = textColor;
    }
    void setHoverColors(const sf::Color& fill, const sf::Color& textColor) {
        hoverFill = fill;
        hoverText = textColor;
    }
    void hover(bool isHovered, float dt) {
        if (isHovered) {
            setFillColor(hoverFill);
            text.setFillColor(hoverText);
            if (getScale().x < 1.05f)
                setScale(getScale() + sf::Vector2f{ 0.6f * dt,0.6f * dt });
        }
        else {
            setFillColor(defaultFill);
            text.setFillColor(defaultText);
            if (getScale().x > 1)
                setScale(getScale() - sf::Vector2f{ 0.6f * dt,0.6f * dt });
        }
    }
    void draw(sf::RenderWindow* w) {
        w->draw(*this);
        w->draw(text);
    }
};
// Upscale a 8x8 image to 32x32 by repeating each pixel 4 times in both dimensions. The input is a pointer to an `sf::Image` object, which is expected to be 8x8 pixels in size. The function creates a new array of pixels where each original pixel is repeated 4 times horizontally and vertically, resulting in a 32x32 image. The new pixel data is then used to resize the original image to 32x32 pixels. After resizing, the dynamically allocated pixel array is deleted to free memory.
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
// Convert a flat array of bytes representing pixel data in RGBA format into a 2D array of `sf::Color` objects. The input is a flat array of bytes where each pixel is represented by four consecutive bytes (red, green, blue, alpha) and the dimensions of the resulting 2D array are specified by `dimensions`. The output is a dynamically allocated 2D array of `sf::Color` objects corresponding to the input pixel data. If the input `pixels` pointer is `nullptr`, an empty 2D array is created. The caller is responsible for deleting the returned 2D array to avoid memory leaks.
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
// Convert a 2D array of `sf::Color` objects into a flat array of bytes representing pixel data in RGBA format. The input is a 2D array of colors with dimensions specified by `dimensions`, and the output is a dynamically allocated array of bytes where each pixel is represented by four consecutive bytes (red, green, blue, alpha). The caller is responsible for deleting the returned array to avoid memory leaks.
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
// Create a new image by tiling a 32x32 pixel pattern across the specified dimensions. The pattern is defined by the provided pixel data, which is expected to be in RGBA format for a 32x32 tile (4096 bytes). The resulting image will have the same dimensions as specified, with the tile repeated as necessary to fill the area.
sf::Image createTiles(const sf::Vector2u dimensions, const uint8_t* pixelData) //32x32 tile
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
	sf::Image img = *image;
	delete image;
    return img;
}
// Convert HSV color to RGB color space
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
// Convert RGB color to HSV color space
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
// Create a new texture with a left-to-right brightness gradient applied, modulated by deltaX. If movedLeft is true, the gradient is reversed (right-to-left).
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
// Draw game over text, win or lose
void GameOverText(sf::RenderWindow* window, bool win, const sf::Font& font)
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
//Delta Time
double deltaT()
{
    static sf::Clock clock;
    double dt = clock.getElapsedTime().asSeconds();
    clock.restart();
    return dt;
}
// X * X = Y px cursor size
// cross pattern
sf::Image getCursorImage(sf::Vector2f dimensions)
{
    const unsigned int W = dimensions.x;
    const unsigned int H = dimensions.y;
    sf::Image img({ W, H }, sf::Color::Transparent);

    for (unsigned int y = 0; y < H; ++y)
    {
        for (unsigned int x = 0; x < W; ++x)
        {
            if (x == W / 2 || y == H / 2) // cross
                img.setPixel({ x, y }, sf::Color::White);
            else if (x == (W / 2)+1 || y == (H / 2) + 1) // cross
                img.setPixel({ x, y }, sf::Color::Blue);
            else if (x == (W / 2) - 1 || y == (H / 2) - 1) // cross
                img.setPixel({ x, y }, sf::Color::Blue);
            else
                img.setPixel({ x, y }, sf::Color::Transparent);
        }
    }

    return img;
}