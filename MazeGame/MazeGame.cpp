#include "MazeGame.h"

void updateFps(sf::Text *fps,double dt) {
    fps->setString(std::to_string(int(1/dt)));
}

int main()
{
    sf::ContextSettings contextSettings;
    contextSettings.antiAliasingLevel = 8;
    // Main Window
    sf::RenderWindow window(sf::VideoMode(sf::Vector2u{1280,720}), "Maze", sf::State::Fullscreen,contextSettings);
    window.setFramerateLimit(60);
    window.setMouseCursorVisible(false);
    window.setVerticalSyncEnabled(true);
    // Bg image
    sf::Image tile("src/grass.bmp");
    upscale(&tile); // 4x upscaling
    tile = *createTiles({ 1280,720 }, tile.getPixelsPtr());
    const sf::Texture texture(tile);
    sf::Sprite sprite(texture);
    //delete createTiles({800,800},pixels);

    // Font
    const sf::Font font("C:/Users/HP/Downloads/Helvetica.ttf");

    sf::Text fps(font, "", 44);
    fps.setFillColor({ 255,0,0,255 });
    
    //sf::Music music("C:/Users/HP/Downloads/Aura - Shubh.mp3");
    //music.play();
    
    // Player
    sf::RectangleShape player({ 32,32 });
    sf::Image pImage("src/player.png");
    upscale(&pImage);
    sf::Texture pTex(pImage);
    player.setTexture(&pTex);
    player.setOrigin({ 16,16 });
    sf::Vector2f playerPos = { 0,0 };
    sf::Vector2i mousePos;
    bool playerAlive = true;

    // Projectile
    std::vector<Projectile> bullets;
    double dt;

    // Enemy
    Enemy enemy;
    bool enemyAlive = true;
    //bool playerMoved = false;
    while (window.isOpen())
    {
        // get delta time
        dt = deltaT();

        // update fps
        static float fpsUpdateThreshold;
        fpsUpdateThreshold += dt;
        if (fpsUpdateThreshold > (1 / 10.0)) {
            updateFps(&fps, dt);
            fpsUpdateThreshold = 0;
        }

        // Process events
        static double holdTime = 0;
        holdTime += dt;

        while (const std::optional event = window.pollEvent())
        {
            // Close window: exit
            if (event->is<sf::Event::Closed>())
                window.close();
            if ((sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LAlt) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::RAlt)) && sf::Keyboard::isKeyPressed(sf::Keyboard::Key::F4)) {
                window.close();
            }

            /*if (event->is<sf::Event::KeyPressed>()) {
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)) {playerPos += {0 ,-1}; playerMoved=true;}
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) {playerPos += {-1, 0}; playerMoved=true;}
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)) {playerPos += {0 , 1} ; playerMoved=true;}
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) {playerPos += {1 , 0} ; playerMoved=true;}
            }*/
        }
        /*if (playerMoved) {
            player.setPosition(playerPos);
            playerMoved = false;
        }
        */
        if (playerAlive)
        {
            // Move player with mouse
            if (sf::Mouse::getPosition() - window.getPosition() != mousePos) {
                mousePos = sf::Mouse::getPosition() - window.getPosition();
                player.setPosition(sf::Vector2f(mousePos));
                if(enemyAlive)
                    if (enemy.checkCollision(sf::Vector2f(mousePos))) {
                        playerAlive = false;
                        GameOver(&window, false);
                    }
            }

            // handle Bullets
            if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)) {
                if (holdTime > (1 / 10.0)) {
                    Projectile* bullet = new Projectile;
                    bullet->bullet.setPosition(sf::Vector2f(mousePos)+sf::Vector2f{0.f,-16.f});
                    bullets.push_back(*bullet);
                    delete bullet;
                    holdTime = 0;
                }
            }
        }
        // Update Enemy
        if (enemyAlive)
            enemy.update(dt);
        // Clear screen
        window.clear();

        // Draw the sprite
        window.draw(sprite);
        
        // Draw player
        if (playerAlive)
            window.draw(player);
        
        // Enemy
        if (enemyAlive)
            window.draw(enemy.enemy);

        // Bulllets
        for (int i = 0; i < bullets.size();i++) {
            bullets[i].update(dt);
            sf::Vector2f pos = bullets[i].bullet.getPosition();
            if (enemyAlive)
                if (enemy.checkCollision(pos))
                    enemyAlive = false;
            window.draw(bullets[i].bullet);
            if (pos.y < 0) {
                bullets.erase(bullets.begin() + i);
            }
        }

        if (!enemyAlive)
            GameOver(&window, true);
        else if (!playerAlive)
            GameOver(&window, false);
        // Fps
        window.draw(fps);

        // Update the window
        window.display();
    }
}