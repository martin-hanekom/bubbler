#include <iostream>
#include <cmath>
#include <cstdlib>
#include <time.h>
#include <vector>
#include <SFML/Graphics.hpp>

#define WIN_W 800
#define WIN_H 600
#define FS 60

#define HEALTH_RATIO 10
#define P_HEALTH 100.0
#define P_RADIUS 10.0
#define P_SPEED 60.0
#define P_BULLETS 100.0
#define GUN_W 3
#define GUN_H 15
#define GUN_COOLDOWN 0.2
#define BULLET_W 5
#define BULLET_H 1
#define BULLET_SPEED 200.0
#define BULLET_DAMAGE 10.0
#define B_SPEED 30.0
#define B_COOLDOWN 2.0;

float vAbs(sf::Vector2f vec) {
  return sqrt(vec.x * vec.x + vec.y * vec.y);
}

float vAngle(sf::Vector2f pos) {
  sf::Vector2f unitPos(1, 0);
  float angle = acos((pos.x * unitPos.x + pos.y * unitPos.y) / (vAbs(pos) * vAbs(unitPos)));
  return angle * copysignf(1.0, pos.y);
}

class Bullet {
  public:
    sf::RectangleShape body;
    sf::Vector2f pos;
    sf::Color color;
    float angle;

    Bullet(sf::Vector2f pos, sf::Vector2f mousePos): pos(pos), color(sf::Color::Yellow) {
      body.setSize(sf::Vector2f(BULLET_W, BULLET_H));
      body.setOrigin(BULLET_W / 2, BULLET_H / 2);
      body.setPosition(pos);
      sf::Vector2f dirPos = mousePos - pos;
      angle = vAngle(dirPos);
      body.setRotation(angle * 180.0 / M_PI);
    }

    void update(float dt) {
      //std::cout << BULLET_SPEED * dt << std::endl;
      pos.x += BULLET_SPEED * dt * cos(angle);
      pos.y += BULLET_SPEED * dt * sin(angle);
      body.setPosition(pos);
    }

    void draw(sf::RenderWindow &window) {
      window.draw(body);
    }

    bool outOfBounds() {
      return (pos.x < 0 || pos.x > WIN_W || pos.y < 0 || pos.y > WIN_H);
    }
};

class Player {
  public:
    sf::CircleShape body;
    sf::RectangleShape gun;
    sf::Text scoreText;
    sf::Text bulletText;
    float health = P_HEALTH;
    float radius = P_RADIUS;
    float num_bullets = P_BULLETS;
    float num_killed = 0;
    sf::Vector2f pos;
    sf::Color color;

    Player(sf::Vector2f pos, sf::Font &font): pos(pos), color(sf::Color::Red) {
      body.setRadius(radius);
      body.setOrigin(radius, radius);
      body.setPosition(pos);
      body.setFillColor(color);

      gun.setSize(sf::Vector2f(GUN_W, GUN_H));
      gun.setOrigin(-radius, 0);
      gun.setPosition(pos);
      gun.setFillColor(sf::Color::White);

      scoreText.setFont(font);
      scoreText.setString("Score: 0");
      scoreText.setPosition(0, 0);
      bulletText.setFont(font);
      bulletText.setString("Bullets: 0");
      bulletText.setPosition(0, 30);
    }

    void update(float dt) {
      if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) {
        pos.y -= P_SPEED * dt;
      } else if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) {
        pos.y += P_SPEED * dt;
      }
      if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
        pos.x -= P_SPEED * dt;
      } else if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
        pos.x += P_SPEED * dt;
      }
      body.setPosition(pos);
      gun.setPosition(pos);
    }

    void mouseMove(sf::Vector2f mousePos) {
      sf::Vector2f dirPos = mousePos - getGunPosition();
      if (vAbs(dirPos) > 2 * radius) {
        gun.setRotation(vAngle(dirPos) * 180.0 / M_PI - 90.0);
      }
    }

    void shoot(sf::Vector2f mousePos, std::vector<Bullet> bullets) {
      if (num_bullets > 0) {
        bullets.push_back(Bullet(getGunPosition(), mousePos));
        num_bullets--;
      }
    }

    void draw(sf::RenderWindow &window) {
      window.draw(body);
      window.draw(gun);
      window.draw(scoreText);
      window.draw(bulletText);
    }

    sf::Vector2f getGunPosition() {
      float gunAngle = gun.getRotation() * M_PI / 180.0;
      return pos + sf::Vector2f(radius * cos(gunAngle), radius * sin(gunAngle));
    }
};

class Bubble {
  public:
    sf::CircleShape body;
    float health;
    float radius;
    sf::Vector2f pos;
    sf::Color color;

    Bubble(sf::Color color): color(color) {
      health = rand() % 20 + 20.0;
      int side = rand() % 4;
      switch (side) {
        case 0:
          pos = sf::Vector2f(0, rand() % WIN_H);
          break;
        case 1:
          pos = sf::Vector2f(rand() % WIN_W, 0);
          break;
        case 2:
          pos = sf::Vector2f(WIN_W, rand() % WIN_H);
          break;
        default:
          pos = sf::Vector2f(rand() % WIN_W, WIN_H);
          break;
      }
      radius = health / 5.0 + 10.0;
      body.setRadius(radius);
      body.setOrigin(radius, radius);
      body.setPosition(pos);
      body.setFillColor(color);
    }

    bool update(float dt, Player &player, std::vector<Bullet> &bullets) {
      sf::Vector2f dirPos = player.pos - pos;
      if (vAbs(dirPos) < P_RADIUS) {
        std::cout << "Game over" << std::endl;
        return true;
      }
      float angle = vAngle(dirPos);
      float speed = B_SPEED - health / 2;
      pos.x += speed * dt * cos(angle);
      pos.y += speed * dt * sin(angle);
      body.setPosition(pos);
      auto bullet = bullets.begin();
      while (bullet != bullets.end()) {
        if (vAbs(pos - bullet->pos) <= radius) {
          bleed();
          bullet = bullets.erase(bullet);
        } else {
          bullet++;
        }
      }
      return false;
    }

    void bleed() {
      health -= BULLET_DAMAGE;
      radius = health / 5.0 + 15.0;
      body.setRadius(radius);
    }

    void draw(sf::RenderWindow &window) {
      window.draw(body);
    }
};

int main() {
  // Create window
  srand(time(0));
  sf::RenderWindow window(sf::VideoMode(WIN_W, WIN_H), "Bubbler", sf::Style::Titlebar | sf::Style::Close);
  window.setVerticalSyncEnabled(true);
  window.setFramerateLimit(FS);
  sf::Clock clock;
  sf::Font font;
  if (!font.loadFromFile("lib/arial.ttf")) {
    std::cout << "Loading font error!" << std::endl;
  }
  float cooldown = B_COOLDOWN;

  // Create player
  Player player(sf::Vector2f(WIN_W / 2, WIN_H / 2), font);
  std::vector<Bullet> bullets;
  std::vector<Bubble> bubbles;

  while (window.isOpen()) {
    sf::Event event;
    while (window.pollEvent(event)) {
      switch (event.type) {
        case sf::Event::Closed:
          window.close();
          break;
        case sf::Event::MouseMoved:
          player.mouseMove(sf::Vector2f(event.mouseMove.x, event.mouseMove.y));
          break;
        case sf::Event::MouseButtonPressed:
          player.shoot(sf::Vector2f(sf::Mouse::getPosition(window)), bullets);
          break;
        default:
          break;
      }
    }

    // update
    float dt = clock.restart().asSeconds();
    cooldown -= dt;
    if (cooldown <= 0) {
      bubbles.push_back(Bubble(sf::Color::Blue));
      cooldown = B_COOLDOWN;
    }
    /*cooldown = (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)) ? cooldown - dt : GUN_COOLDOWN;
    if (cooldown <= 0) {
      std::cout << "shoot" << std::endl;
      bullets.push_back(Bullet(player.getGunPosition(), sf::Vector2f(sf::Mouse::getPosition(window))));
    }*/
    player.update(dt);
    auto bullet = bullets.begin();
    while (bullet != bullets.end()) {
      bullet->update(dt);
      if (bullet->outOfBounds()) {
        bullet = bullets.erase(bullet);
      } else {
        bullet++;
      }
    }
    auto bubble = bubbles.begin();
    while (bubble != bubbles.end()) {
      if (bubble->update(dt, player, bullets)) {
        window.close();
      }
      if (bubble->health <= 0) {
        bubble = bubbles.erase(bubble);
        player.num_killed++;
      } else {
        bubble++;
      }
    }

    // draw
    window.clear();
    player.draw(window);
    for (auto &bullet : bullets) {
      bullet.draw(window);
    }
    for (auto &bubble : bubbles) {
      bubble.draw(window);
    }
    window.display();
  }
  return 0;
}
