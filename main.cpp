#include <iostream>
#include <cmath>
#include <cstdlib>
#include <time.h>
#include <vector>
#include <SFML/Graphics.hpp>

#define WIN_W 1024
#define WIN_H 650
#define FS 60

#define HEALTH_DIV 20
#define HEALTH_OFFSET 8
#define SPEED_DIV 2
#define PLAYER_HEALTH 100
#define PLAYER_SPEED 60
#define PLAYER_BULLETS 100
#define GUN_W 3
#define GUN_H 15
#define GUN_COOLDOWN 0.2
#define BULLET_W 5
#define BULLET_H 1
#define BULLET_SPEED 200
#define BULLET_DAMAGE 10
#define BUBBLE_SPEED 30
#define BUBBLE_COOLDOWN 2;

typedef struct {
  sf::RectangleShape body;
  sf::Vector2f pos;
  float angle;
} Bullet;

typedef struct {
  sf::RectangleShape body;
  sf::Vector2f pos;
  int bullets;
} Gun;

typedef struct {
  sf::CircleShape body;
  sf::Vector2f pos;
  int health;
  int killed;
} Player;

typedef struct {
  sf::CircleShape body;
  sf::Vector2f pos;
  int health;
  float speed;
} Bubble;

float vAbs(sf::Vector2f vec) {
  return sqrt(vec.x * vec.x + vec.y * vec.y);
}

float vAngle(sf::Vector2f pos) {
  sf::Vector2f unitPos(1, 0);
  float angle = acos((pos.x * unitPos.x + pos.y * unitPos.y) / (vAbs(pos) * vAbs(unitPos)));
  return angle * copysignf(1.0, pos.y);
}

float radius(int health) {
  return health / HEALTH_DIV + HEALTH_OFFSET;
}

float speed(float maxSpeed, int health) {
  return maxSpeed - health / SPEED_DIV;
}

void createBullet(Bullet &bullet, sf::Vector2f gunPos, sf::Vector2f &mousePos) {
  bullet.pos = gunPos;
  bullet.body.setSize(sf::Vector2f(BULLET_W, BULLET_H));
  bullet.body.setOrigin(BULLET_W / 2, BULLET_H / 2);
  bullet.body.setPosition(bullet.pos);
  bullet.body.setFillColor(sf::Color::Yellow);
  sf::Vector2f dirPos = mousePos - gunPos;
  bullet.angle = vAngle(dirPos);
  bullet.body.setRotation(bullet.angle * 180.0 / M_PI);
}

void createGun(Gun &gun, Player &player) {
  gun.body.setSize(sf::Vector2f(GUN_W, GUN_H));
  gun.body.setOrigin(-radius(player.health), 0);
  gun.body.setPosition(player.pos);
  gun.body.setFillColor(sf::Color::Magenta);
  gun.bullets = PLAYER_BULLETS;
}

sf::Vector2f gunPosition(Player &player, Gun &gun) {
  float angle = gun.body.getRotation() * M_PI / 180.0;
  float r = radius(player.health);
  return player.pos + sf::Vector2f(r * cos(angle), r * sin(angle));
}

void createPlayer(Player &player, sf::Vector2f pos) {
  player.pos = pos;
  player.health = PLAYER_HEALTH;
  float r = radius(player.health);
  player.body.setRadius(r);
  player.body.setOrigin(r, r);
  player.body.setPosition(player.pos);
  player.body.setFillColor(sf::Color::Red);
}

void createBubble(Bubble &bubble) {
  bubble.health = rand() % 20 + 20.0;
  int side = rand() % 4;
  switch (side) {
    case 0:
      bubble.pos = sf::Vector2f(0, rand() % WIN_H);
      break;
    case 1:
      bubble.pos = sf::Vector2f(rand() % WIN_W, 0);
      break;
    case 2:
      bubble.pos = sf::Vector2f(WIN_W, rand() % WIN_H);
      break;
    default:
      bubble.pos = sf::Vector2f(rand() % WIN_W, WIN_H);
      break;
  }
  float r = radius(bubble.health);
  bubble.body.setRadius(r);
  bubble.body.setOrigin(r, r);
  bubble.body.setPosition(bubble.pos);
  bubble.body.setFillColor(sf::Color::Blue);
}

void mouseMove(sf::Vector2f mousePos, Player &player, Gun &gun) {
  sf::Vector2f dirPos = mousePos - gunPosition(player, gun);
  if (vAbs(dirPos) > 2 * radius(player.health)) {
    gun.body.setRotation(vAngle(dirPos) * 180.0 / M_PI - 90.0);
  }
}

void shoot(sf::Vector2f mousePos, Player &player, Gun &gun, std::vector<Bullet> &bullets) {
  if (gun.bullets > 0) {
    Bullet bullet;
    createBullet(bullet, gunPosition(player, gun), mousePos);
    bullets.push_back(bullet);
    gun.bullets--;
  }
}

float update(float dt, float cooldown, Player &player, Gun &gun, std::vector<Bubble> &bubbles, std::vector<Bullet> &bullets) {
  // Keypress
  if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) {
    player.pos.y -= PLAYER_SPEED * dt;
  } else if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) {
    player.pos.y += PLAYER_SPEED * dt;
  }
  if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
    player.pos.x -= PLAYER_SPEED * dt;
  } else if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
    player.pos.x += PLAYER_SPEED * dt;
  }
  player.body.setPosition(player.pos);
  gun.body.setPosition(player.pos);

  // Cooldown
  //cooldown -= dt;
  if (cooldown <= 0) {
    Bubble bubble;
    createBubble(bubble);
    bubbles.push_back(bubble);
    cooldown = BUBBLE_COOLDOWN;
  }

  // Velocity-based
  /*auto bullet = bullets.begin();
  while (bullet != bullets.end()) {
    bullet->pos.x += BULLET_SPEED * dt * cos(bullet->angle);
    bullet->pos.y += BULLET_SPEED * dt * sin(bullet->angle);
    bool erased = false;
    for (auto &bubble : bubbles) {
      if (vAbs(bubble.pos - bullet->pos) <= radius(bubble.health)) {
        bubble.health -= BULLET_DAMAGE;
        bubble.body.setRadius(radius(bubble.health));
        erased = true;
        break;
      }
    }
    if (erased || bullet->pos.x < 0 || bullet->pos.x > WIN_W || bullet->pos.y < 0 || bullet->pos.y > WIN_H) {
      bullet = bullets.erase(bullet);
    } else {
      bullet->body.setPosition(bullet->pos);
      bullet++;
    }
  }*/
  auto bubble = bubbles.begin();
  while (bubble != bubbles.end()) {
    if (bubble->health <= 0) {
      bubble = bubbles.erase(bubble);
    } else {
      sf::Vector2f dirPos = player.pos - bubble->pos;
      float angle = vAngle(dirPos);
      float s = speed(BUBBLE_SPEED, bubble->health);
      std::cout << s << std::endl;
      bubble->pos.x = s * dt * cos(angle);
      bubble->pos.y = s * dt * sin(angle);
      if (vAbs(player.pos - bubble->pos) < radius(player.health)) {
        std::cout << "Game over" << std::endl;
        return -1;
      }
      bubble->body.setPosition(bubble->pos);
      bubble++;
    }
  }

  return cooldown;
}

void draw(sf::RenderWindow &window, Player &player, Gun &gun, std::vector<Bubble> &bubbles, std::vector<Bullet> &bullets) {
  window.clear();
  for (auto &bubble : bubbles) {
    window.draw(bubble.body);
  }
  window.draw(player.body);
  window.draw(gun.body);
  for (auto &bullet : bullets) {
    window.draw(bullet.body);
  }
  window.display();
}

int main() {
  srand(time(0));
  sf::RenderWindow window(sf::VideoMode(WIN_W, WIN_H), "Bubbler", sf::Style::Titlebar | sf::Style::Close);
  window.setVerticalSyncEnabled(true);
  window.setFramerateLimit(FS);
  sf::Clock clock;
  sf::Font font;
  if (!font.loadFromFile("lib/arial.ttf")) {
    std::cout << "Loading font error!" << std::endl;
    return 1;
  }
  float cooldown = 0;//BUBBLE_COOLDOWN;

  // Create objects
  Player player;
  Gun gun;
  std::vector<Bullet> bullets;
  std::vector<Bubble> bubbles;
  createPlayer(player, {WIN_W / 2, WIN_H / 2});
  createGun(gun, player);

  while (window.isOpen()) {
    sf::Event event;
    while (window.pollEvent(event)) {
      switch (event.type) {
        case sf::Event::Closed:
          window.close();
          break;
        case sf::Event::MouseMoved:
          mouseMove(sf::Vector2f(sf::Mouse::getPosition(window)), player, gun);
          break;
        case sf::Event::MouseButtonPressed:
          shoot(sf::Vector2f(sf::Mouse::getPosition(window)), player, gun, bullets);
          break;
        default:
          break;
      }
    }

    cooldown = update(clock.restart().asSeconds(), cooldown, player, gun, bubbles, bullets);
    if (cooldown < 0) {
      window.close();
    }
    draw(window, player, gun, bubbles, bullets);
  }
  
  return 0;
}
