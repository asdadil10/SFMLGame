#include "MazeGame.h"

void updateFps(sf::Text *fps,double dt) {
    fps->setString(std::to_string(int(1/dt)));
}

// Future Idea : make these functions and update with updated speed

const float BulletAccumulator = 55.5/BulletSpeed; // seconds between each bullet update
const float EnemyShootAccumulator = 138.75/BulletSpeed; // seconds between each enemy shot

enum GameState { MENU,GAME,OPTION,SOUND,EXIT };

uint8_t Menu(sf::RenderWindow &window,sf::Font font) {

	bool moving = false;
	double dtheta = 25;
    // Bg image
    sf::Image tile("src/grass1.bmp");
    upscale(&tile); // 4x upscaling
    tile = *createTiles({ 1280,720 }, tile.getPixelsPtr());
    const sf::Texture texture(tile);
    sf::Sprite sprite(texture);

	sf::RectangleShape PlayButton({ 200,80 });
	sf::Text playText(font, "Play", 48);
	PlayButton.setOrigin(PlayButton.getSize() / 2.f);
	playText.setOrigin(playText.getLocalBounds().getCenter());
	PlayButton.setPosition({ window.getSize().x / 1.25f, window.getSize().y / 2.f });
	playText.setPosition(PlayButton.getPosition());
	playText.setFillColor({ 255,0,0,255 });
	PlayButton.setFillColor({ 64,64,64,255 });

    sf::Image CursorImg = getCursorImage({ 25,25 });
    sf::RectangleShape cursor({ 25,25 });
	cursor.setOrigin({ 12.5f,12.5f });
	cursor.setTexture(new sf::Texture(CursorImg));
    float dt;
	bool expanding = true;
    // Menu Text
    sf::Text title(font, "SpaceMane", 96);
    title.setFillColor({ 255,0,0,255 });
    title.setOrigin(title.getLocalBounds().getCenter());
    title.setPosition(sf::Vector2f{ window.getSize().x / 3.f, window.getSize().y / 3.f });
    while (window.isOpen())
    {
        dt = deltaT();
		moving = false;
        // Process events
        while (const std::optional event = window.pollEvent())
        {
            // Close window: exit
            if (event->is<sf::Event::Closed>())
                return EXIT;
            if ((sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LAlt) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::RAlt)) && sf::Keyboard::isKeyPressed(sf::Keyboard::Key::F4)) {
                return EXIT;
            }
            else
                cursor.setFillColor(sf::Color::White);

            cursor.setPosition(sf::Vector2f(sf::Mouse::getPosition() - window.getPosition()));
            if (event->getIf<sf::Event::MouseMovedRaw>()) {
				dtheta += dt * 720; // adjust the multiplier to control rotation speed
                if(dtheta > 720.f) {
                    dtheta = 720.f;
				}
                else if (dtheta < 75.f) {
                    dtheta += 75.f;
                }
				moving = true;
            }
            if (PlayButton.getGlobalBounds().contains(cursor.getPosition())) {
                cursor.setFillColor(sf::Color::Green);
				PlayButton.setFillColor({ 64,128,64,255 });
				playText.setFillColor({ 0,255,0,255 });
                if (event->getIf<sf::Event::MouseButtonReleased>() != nullptr && event->getIf<sf::Event::MouseButtonReleased>()->button == sf::Mouse::Button::Left) {
                    return GAME;
                }
            }
            else
            {
                PlayButton.setFillColor({ 64,64,64,255 });
                playText.setFillColor({ 255,0,0,255 });
            }
        }
        if (expanding) { // simple animation
            title.setScale(title.getScale() + sf::Vector2f({ float_t(0.15f * dt),float_t(0.15f * dt) }));
            if(title.getScale().x > 1.15f) {
                expanding = false;
			}
        }
        else{
            title.setScale(title.getScale() - sf::Vector2f({ float_t(0.15f * dt),float_t(0.15f * dt) }));
            if (title.getScale().x < 1.f) {
                expanding = true;
            }
		}

        if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)) {
            moving = true;
            cursor.setFillColor(sf::Color::Red);
            dtheta = -720.f; // max rotation speed when clicking
        }

        if (!moving) {
            if(dtheta> 75.f) {
                dtheta -= dt * 720;
            }
            else {
                dtheta += dt * 720;
            }
        }
        cursor.rotate(sf::degrees(dt * dtheta));
        // Clear screen
        window.clear();
        // Draw the text
		window.draw(sprite);
        window.draw(title);
		window.draw(PlayButton);
		window.draw(playText);
        window.draw(cursor);
        
        // Update the window
        window.display();
	}
}
void Game(sf::RenderWindow &window,sf::Font font) {

    // Bg image
    sf::Image tile("src/grass.bmp");
    upscale(&tile); // 4x upscaling
    tile = *createTiles({ 1280,720 }, tile.getPixelsPtr());
    const sf::Texture texture(tile);
    sf::Sprite sprite(texture);

    // FPS Text
    sf::Text fps(font, "", 44);
    fps.setFillColor({ 255,0,0,255 });
    sf::Text speedText = fps;
    speedText.setPosition({ 110,0 });

    // Player
    Player player;
    sf::CircleShape& booster = player.booster; // Instead of #define

    // Enemy
    Enemy enemy;

    // Health Bar
    ProgressBar PlayerHealth(300);
    ProgressBar EnemyHealth(300);
    PlayerHealth.setFillColor({ 0,64,255 });
    PlayerHealth.setPosition({ 50,100 });
    EnemyHealth.setPosition({ 50,150 });

    // Projectile
    std::vector<Bullet> pBullets;
    std::vector<Bullet> eBullets;

    // Mouse
    sf::Vector2i mousePos;

    // Delta time
    double dt;
    double speedTimer = 0.0;
    bool slowed = false;
    bool playerMoved = false;

    while (window.isOpen())
    {
        // get delta time
        dt = deltaT();

        // Process events
        static double holdTime = 0, enemyBulletTime = 0;
        holdTime += dt;
        enemyBulletTime += dt;
        speedTimer += dt;
        if (player.Alive)
        {
            // Move player with mouse
            if (sf::Mouse::getPosition() - window.getPosition() != mousePos) {
                mousePos = sf::Mouse::getPosition() - window.getPosition();
                player.setPosition(sf::Vector2f(mousePos));
                booster.setPosition(sf::Vector2f(mousePos));

                if (enemy.Alive)
                    if (enemy.checkCollision(sf::Vector2f(mousePos))) {
                        player.setHealth(0);
                        player.Alive = false;
                        PlayerHealth.setProgress(0);
                    }
            }

            // handle Bullets
            if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)) {
                if (holdTime > BulletAccumulator) {
                    Bullet bullet;
                    bullet.setPosition(sf::Vector2f(mousePos) + sf::Vector2f{ 0.f,-16.f });
                    bullet.setFillColor({ 64,64,255 }), bullet.setOutlineColor({ 32,32,192,192 });
                    pBullets.push_back(bullet);
                    holdTime = 0;
                }
            }

        }

        // update fps
        static float fpsUpdateThreshold;
        fpsUpdateThreshold += dt;
        if (fpsUpdateThreshold > (1 / 10.0)) {
            speedText.setString("Bullet Speed: " + std::to_string(BulletSpeed));
            updateFps(&fps, dt);
            fpsUpdateThreshold = 0;
        }

        // Handle events
        while (const std::optional event = window.pollEvent())
        {
            // Close window: exit
            if (event->is<sf::Event::Closed>())
                return;
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape)) {
                return;
            }

            // HANDLE PLAYER TURNING SIDEWAYS AND BOOSTER
            if (const auto* const mouseMovedRaw = event->getIf<sf::Event::MouseMovedRaw>())
            {
                static bool MovedLeft = true;
                static sf::Texture* pTex;
                if (mouseMovedRaw->delta.x != 0) {
                    if (mouseMovedRaw->delta.x < 0) {
                        MovedLeft = true;
                    }
                    else {
                        MovedLeft = false;
                    }
                    delete pTex;
                    pTex = texShade(MovedLeft, player.Tex, mouseMovedRaw->delta.x);
                    player.setTexture(pTex);
                }
                player.setScale(sf::Vector2f({ float_t(1.f / (abs(mouseMovedRaw->delta.x / 8.f) + 1)),1.f }));

                static uint8_t a;
                if (mouseMovedRaw->delta.y < 0) {
                    if (a < 196) {
                        a += 4;
                    }
                }
                else
                {
                    if (a > 32) {
                        a -= 4;
                    }
                }
                booster.setFillColor({ 255, 165, 0 , a });
                playerMoved = true;
            }
        }

        // Bullet Time Slowdown
        if (speedTimer > 5 && player.Alive && enemy.Alive) {
            if (speedTimer > 5 + 2) {
                if (BulletSpeed < 555) {
                    BulletSpeed += dt * 555;
                }
                else {
                    BulletSpeed = 555;
                    speedTimer = 0.0;
                }
                slowed = false;
            }
            else if (!slowed) {
                BulletSpeed = 125;
                slowed = true;
            }
        }

        // Update if player is still, IDLE position
        if (!playerMoved) {
            bool r = rand() % 2;
            uint8_t a = booster.getFillColor().a;
            int b = 500 * dt;
            if (a > 156) {
                a -= b;
            }
            else if (a < 64) {
                a += b;
            }
            if (r)
                booster.setFillColor({ 255, 165, 0 , uint8_t(a + b) });
            else
                booster.setFillColor({ 255, 165, 0 , uint8_t(a - b) });
            float x = player.getScale().x;
            if (x < 1.f) {
                x += 5 * dt;
            }
            else
                x = 1.f;
            player.setScale(sf::Vector2f({ float_t(x),1.f }));
            player.setTexture(player.Tex);
        }
        playerMoved = false;

        // Update Enemy
        if (enemy.Alive) {
            enemy.update(dt);
            if (enemyBulletTime > EnemyShootAccumulator) {
                sf::Vector2f enemyPos = enemy.getPosition();
                sf::Vector2f direction = enemyPos - sf::Vector2f(mousePos);  // Vector from enemy to player

                Bullet eBullet(direction);  // Pass direction by value
                eBullet.setPosition(enemyPos);
                eBullets.push_back(eBullet);  // No need for new/delete

                enemyBulletTime = 0;
            }
        }

        // Clear screen
        window.clear();

        // Draw the sprite
        window.draw(sprite);

        // Draw player
        if (player.Alive) {
            window.draw(booster);
            window.draw(player);
        }

        // Enemy
        if (enemy.Alive) {
            // Bulllets by enemy
            for (int i = 0; i < eBullets.size(); i++) {
                eBullets[i].update(-dt);
                sf::Vector2f pos = eBullets[i].getPosition();
                if (player.Alive) {
                    if (player.checkCollision(pos)) {
                        eBullets.erase(eBullets.begin() + i);
                        // Update Helath
                        PlayerHealth.setProgress(player.getHealth());
                        i--; //to avoid skipping the next bullet after erasing
                        continue;
                    }
                }
                window.draw(eBullets[i]);
                if (pos.y > 720) {
                    eBullets.erase(eBullets.begin() + i);
                }
            }
            window.draw(enemy);
        }

        // Bulllets by player
        for (int i = 0; i < pBullets.size(); i++) {
            pBullets[i].update(dt);
            sf::Vector2f pos = pBullets[i].getPosition();
            if (enemy.Alive) {
                if (enemy.checkCollision(pos)) {
                    pBullets.erase(pBullets.begin() + i);
                    EnemyHealth.setProgress(enemy.getHealth());
                    i--; //to avoid skipping the next bullet after erasing
                    continue;
                }
            }
            window.draw(pBullets[i]);
            if (pos.y < 0) {
                pBullets.erase(pBullets.begin() + i);
            }
        }

        if (!enemy.Alive)
            GameOverText(&window, true, font);
        else if (!player.Alive)
            GameOverText(&window, false, font);

        // Fps
        window.draw(fps);

        // Bullet Speed
        window.draw(speedText);

        // Health Bars
        PlayerHealth.draw(&window);
        EnemyHealth.draw(&window);

        // Update the window
        window.display();
    }
    window.setMouseCursorVisible(true);
}
int main()
{
    sf::ContextSettings contextSettings;
    contextSettings.antiAliasingLevel = 8;

    // Main Window
    sf::RenderWindow window(sf::VideoMode(sf::Vector2u{ 1280,720 }), "Game", sf::State::Fullscreen, contextSettings);
    //window.setFramerateLimit(60);
    window.setMouseCursorVisible(false);
    window.setVerticalSyncEnabled(true);
    const sf::Font font("C:/Users/HP/Downloads/Helvetica.ttf");
    for (bool exit = false; !exit;) {
        switch (Menu(window, font)) {
        case GAME:
            Game(window, font);
            break;
        case EXIT:
            exit = true;
            break;
        }
    }
    window.close();
	return 0;
}
