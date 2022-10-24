#include <iostream>
#include <cmath>
#include <SFML/Graphics.hpp>

#define WIN_W 800
#define WIN_H 600
#define FS 30

#define HEALTH_RATIO 10
#define P_HEALTH 100.0
#define P_SPEED 60.0
#define GUN_W 3
#define GUN_H 15

float vAbs(sf::Vector2f vec) {
  return sqrt(vec.x * vec.x + vec.y * vec.y);
}

class Entity {
  public:
    float health;
    sf::Vector2f pos;
    sf::Color color;

    Entity(float health, sf::Color color, sf::Vector2f pos):
      health(health), color(color), pos(pos) { }

    float getRadius() {
      return health / HEALTH_RATIO;
    }

    virtual void update(sf::Time dt) { }

    virtual void draw(sf::RenderWindow window) { }
};

class Player: public Entity {
  public:
    sf::CircleShape body;
    sf::RectangleShape gun;

    Player(float health, sf::Color color, sf::Vector2f pos):
      Entity(health, color, pos) {
      body.setFillColor(color);
      gun.setFillColor(sf::Color::White);
      resize();
    }

    void resize() {
      float radius = getRadius();
      body.setPosition(pos);
      body.setRadius(radius);
      body.setOrigin(sf::Vector2f(radius, radius));
      gun.setPosition(pos);
      gun.setSize(sf::Vector2f(GUN_W, GUN_H));
      gun.setOrigin(sf::Vector2f(-radius, 0));
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
      float radius = getRadius();
      float gunAngle = gun.getRotation() * M_PI / 180.0;
      sf::Vector2f relPos = mousePos - pos - sf::Vector2f(radius * cos(gunAngle), radius * sin(gunAngle));
      if (vAbs(relPos) > 2 * radius) {
        //std::cout << "relPos: " << relPos.x << "," << relPos.y << std::endl;
        sf::Vector2f unitPos(1, 0);
        float angle = acos((relPos.x * unitPos.x + relPos.y * unitPos.y) / (vAbs(relPos) * vAbs(unitPos))) * 180 / M_PI;
        angle = angle * copysignf(1.0, relPos.y) - 90.0;
        gun.setRotation(angle);
      }
    }

    void draw(sf::RenderWindow &window) {
      window.draw(body);
      window.draw(gun);
    }
};

int main() {
  // Create window
  sf::RenderWindow window(sf::VideoMode(WIN_W, WIN_H), "Bubbler", sf::Style::Titlebar | sf::Style::Close);
  window.setVerticalSyncEnabled(true);
  window.setFramerateLimit(FS);
  sf::Clock clock;

  // Create player
  Player player(P_HEALTH, sf::Color::Red, sf::Vector2f(WIN_W / 2, WIN_H / 2));

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
        default:
          break;
      }
    }
    player.update(clock.restart().asSeconds());
    window.clear();
    player.draw(window);
    window.display();
  }

  return 0;
}
