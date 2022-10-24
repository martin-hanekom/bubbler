#include <iostream>
#include <cmath>
#include <vector>
#include <SFML/Graphics.hpp>

#define WIN_W 800
#define WIN_H 600
#define FS 30

#define HEALTH_RATIO 10
#define P_HEALTH 100.0
#define P_RADIUS 10.0
#define P_SPEED 60.0
#define GUN_W 3
#define GUN_H 15
#define GUN_COOLDOWN 0.2
#define BULLET_W 5
#define BULLET_H 1
#define BULLET_SPEED 400.0

float vAbs(sf::Vector2f vec) {
  return sqrt(vec.x * vec.x + vec.y * vec.y);
}

float vAngle(sf::Vector2f pos) {
  sf::Vector2f unitPos(1, 0);
  float angle = acos((pos.x * unitPos.x + pos.y * unitPos.y) / (vAbs(pos) * vAbs(unitPos)));
  return angle * copysignf(1.0, pos.y);
}

class Player {
  public:
    sf::CircleShape body;
    sf::RectangleShape gun;
    float health = P_HEALTH;
    float radius = P_RADIUS;
    float gunCooldown = 0;
    sf::Vector2f pos;
    sf::Color color;

    Player(sf::Vector2f pos): pos(pos), color(sf::Color::Red) {
      body.setRadius(radius);
      body.setOrigin(radius, radius);
      body.setPosition(pos);
      body.setFillColor(color);

      gun.setSize(sf::Vector2f(GUN_W, GUN_H));
      gun.setOrigin(-radius, 0);
      gun.setPosition(pos);
      gun.setFillColor(sf::Color::White);
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

    void draw(sf::RenderWindow &window) {
      window.draw(body);
      window.draw(gun);
    }

    sf::Vector2f getGunPosition() {
      float gunAngle = gun.getRotation() * M_PI / 180.0;
      return pos + sf::Vector2f(radius * cos(gunAngle), radius * sin(gunAngle));
    }
};

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
      pos.x += BULLET_SPEED * dt * cos(angle);
      pos.y += BULLET_SPEED * dt * sin(angle);
      //std::cout << "pos: " << pos.x << ", " << pos.y << std::endl;
      body.setPosition(pos);
    }

    void draw(sf::RenderWindow &window) {
      window.draw(body);
    }

    bool outOfBounds() {
      return (pos.x < 0 || pos.x > WIN_W || pos.y < 0 || pos.y > WIN_H);
    }
};

int main() {
  // Create window
  sf::RenderWindow window(sf::VideoMode(WIN_W, WIN_H), "Bubbler", sf::Style::Titlebar | sf::Style::Close);
  window.setVerticalSyncEnabled(true);
  window.setFramerateLimit(FS);
  sf::Clock clock;
  float cooldown = 0;

  // Create player
  Player player(sf::Vector2f(WIN_W / 2, WIN_H / 2));
  std::vector<Bullet> bullets;

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
          bullets.push_back(Bullet(player.getGunPosition(), sf::Vector2f(sf::Mouse::getPosition(window))));
          break;
        default:
          break;
      }
    }

    // update
    float dt = clock.restart().asSeconds();
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

    // draw
    window.clear();
    player.draw(window);
    for (auto &bullet : bullets) {
      bullet.draw(window);
    }
    window.display();
  }
  return 0;
}
