#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/System.hpp>
#include <SFML/Audio.hpp>
#include <iostream>
#include <vector>

using namespace std;
using namespace sf;

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;
const float BALLOON_SPEED = 0.2 / 3.0f;  // Reduced the speed by 3
const int ARROW_SPEED = 500; // Increased arrow speed
const int MAX_ARROWS = 5;
const int ARROWS_REPLENISH_THRESHOLD = 3; // Number of hits needed to replenish arrows

class Balloon {
public:
    Sprite sprite;
    bool isAlive;

    Balloon(Texture& texture, float x, float y) {
        sprite.setTexture(texture);
        sprite.setPosition(x, y);
        isAlive = true;
    }

    void update() {
        if (isAlive) {
            sprite.move(BALLOON_SPEED, 0);
        }
    }

    void draw(RenderWindow& window) {
        if (isAlive) {
            window.draw(sprite);
        }
    }
};

class Arrow {
public:
    Sprite sprite;
    bool isFlying;
    float verticalSpeed; // Added vertical speed
    Clock animationClock; // Added animation clock

    Arrow(Texture& texture) {
        sprite.setTexture(texture);
        isFlying = false;
        verticalSpeed = ARROW_SPEED;
    }

    void shoot(float x, float y) {
        sprite.setPosition(x, y);
        isFlying = true;
        animationClock.restart(); // Restart animation clock when shooting
    }

    void update() {
        if (isFlying) {
            float deltaTime = animationClock.restart().asSeconds(); // Get time since last restart
            sprite.setPosition(sprite.getPosition().x, sprite.getPosition().y - verticalSpeed * deltaTime);
        }
    }

    void reset() {
        sprite.setPosition(-100, -100); // Offscreen position
        isFlying = false;
    }

    void draw(RenderWindow& window) {
        if (isFlying) {
            window.draw(sprite);
        }
    }
};

int main() {
    RenderWindow window(VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Duck Hunter");

    // Load textures
    Texture balloonTexture, arrowTexture, crossbowTexture;
    if (!balloonTexture.loadFromFile("C:\\Users\\jones\\source\\repos\\CGT215 Lab8\\images2\\duck.png") ||
        !arrowTexture.loadFromFile("C:\\Users\\jones\\source\\repos\\CGT215 Lab8\\images2\\arrow.png") ||
        !crossbowTexture.loadFromFile("C:\\Users\\jones\\source\\repos\\CGT215 Lab8\\images2\\crossbow.png")) {
        cout << "Failed to load textures" << endl;
        return -1;
    }


    // Set up balloons and arrows
    vector<Balloon> balloons;
    vector<Arrow> arrows(MAX_ARROWS, Arrow(arrowTexture)); // Initialize multiple arrows with the same texture

    unsigned int currentArrow = 0; // Track the index of the current arrow
    int hitCount = 0; // Track the number of successful hits

    // Set up crossbow
    Sprite crossbow(crossbowTexture);
    crossbow.setPosition(WINDOW_WIDTH / 2 - crossbow.getGlobalBounds().width / 2, WINDOW_HEIGHT - crossbow.getGlobalBounds().height - 20);

    // Set up fonts and texts
    Font font;
    // Use the default Arial font from Windows
font.loadFromFile("C:/Users/jones/source/repos/CGT215 Lab8/images2/arial.ttf");
    Text scoreText("arial.ttf", font, 30);
    scoreText.setFillColor(Color::White);
    scoreText.setPosition(WINDOW_WIDTH - 150, WINDOW_HEIGHT - 50);

    Text arrowsText("arial.ttf", font, 30);
    arrowsText.setFillColor(Color::White);
    arrowsText.setPosition(20, WINDOW_HEIGHT - 50);

    Text gameOverText("arial.ttf", font, 60); // Added Game Over text
    gameOverText.setFillColor(Color::Red);
    gameOverText.setPosition(WINDOW_WIDTH / 2 - 200, WINDOW_HEIGHT / 2 - 30);

    // Game loop
    Clock clock;
    int score = 0;

    while (window.isOpen()) {
        Event event;
        while (window.pollEvent(event)) {
            if (event.type == Event::Closed) {
                window.close();
            }

            // Change the event handling to space bar for shooting arrows
            if (event.type == Event::KeyPressed && event.key.code == Keyboard::Space && !arrows[currentArrow].isFlying) {
                arrows[currentArrow].shoot(crossbow.getPosition().x + crossbow.getGlobalBounds().width / 2, crossbow.getPosition().y);
                currentArrow = (currentArrow + 1) % MAX_ARROWS; // Move to the next arrow in a circular manner
            }
        }

        // Update
        Time deltaTime = clock.restart();
        float dt = deltaTime.asSeconds();

        // Spawn balloons (reduce the spawn rate)
        if (rand() % 11000 == 0) {
            balloons.push_back(Balloon(balloonTexture, -100, rand() % (WINDOW_HEIGHT - 100)));
        }

        // Update balloons
        for (auto& balloon : balloons) {
            balloon.update();
        }

        // Update arrows
        for (auto& arrow : arrows) {
            arrow.update();
        }

        // Check for collisions
        for (auto& arrow : arrows) {
            for (auto it = balloons.begin(); it != balloons.end(); ) {
                if (arrow.isFlying && arrow.sprite.getGlobalBounds().intersects(it->sprite.getGlobalBounds())) {
                    it = balloons.erase(it);
                    score++;
                    arrow.reset();
                    hitCount++;

                    // Replenish arrows when hitCount reaches the threshold
                    if (hitCount >= ARROWS_REPLENISH_THRESHOLD) {
                        hitCount = 0;
                        arrows[currentArrow].reset();
                    }

                    break; // Break the inner loop after the first collision
                }
                else {
                    ++it;
                }
            }
        }

        // Remove off-screen balloons
        balloons.erase(
            remove_if(balloons.begin(), balloons.end(),
                [](const Balloon& balloon) {
                    return balloon.sprite.getPosition().x > WINDOW_WIDTH;
                }),
            balloons.end()
                    );

        // Draw
        window.clear();

        for (auto& balloon : balloons) {
            balloon.draw(window);
        }

        for (auto& arrow : arrows) {
            arrow.draw(window);
        }

        window.draw(crossbow);

        // Display score and arrows left
        scoreText.setString("Score: " + to_string(score));
        arrowsText.setString("Arrows: " + to_string(MAX_ARROWS - currentArrow));

        window.draw(scoreText);
        window.draw(arrowsText);

        // Display Game Over if no arrows left
        if (arrows[currentArrow].isFlying && balloons.empty()) {
            window.draw(gameOverText);
        }

        window.display();
    }

    return 0;
}
