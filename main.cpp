#include <iostream>
#include <cmath>
#include <cstdlib>
#include <time.h>
#include <vector>
#include <SFML/Graphics.hpp>

#define WIN_W 1024
#define WIN_H 650
#define FS 60

#define HEALTH_DIV 10
#define HEALTH_OFFSET 8
#define SPEED_DIV 2
#define MIN_SPEED 10
#define PLAYER_HEALTH 100
#define PLAYER_SPEED 90
#define PLAYER_BULLETS 30
#define GUN_W 3
#define GUN_H 15
#define GUN_COOLDOWN 0.3
#define BULLET_W 5
#define BULLET_H 1
#define BULLET_SPEED 200
#define BULLET_DAMAGE 10
#define AMMO_COST 15
#define AMMO_AMOUNT 30
#define BUBBLE_SPEED 30
#define BUBBLE_COOLDOWN 2
#define BUBBLE_DAMAGE 20
#define BUBBLE_ATTACK 1

#define CD_BUBBLE 0
#define CD_GUN 1

typedef struct {
  sf::RectangleShape body;
  sf::Vector2f pos;
  float angle;
} Bullet;

typedef struct {
  sf::RectangleShape body;
  sf::Vector2f pos;
} Gun;

typedef struct {
  sf::CircleShape body;
  sf::Vector2f pos;
  int health;
  int cash;
  int killed;
  int bullets;
} Player;

typedef struct {
  sf::CircleShape body;
  sf::Vector2f pos;
  int health;
  int origHealth;
  float speed;
  float attack;
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
  return MIN_SPEED + (maxSpeed - maxSpeed * health / 100);
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

void createGun(Gun &gun) {
  gun.body.setSize(sf::Vector2f(GUN_W, GUN_H));
  gun.body.setFillColor(sf::Color::Magenta);
}

sf::Vector2f gunPosition(Player &player, Gun &gun) {
  float angle = gun.body.getRotation() * M_PI / 180.0;
  float r = radius(player.health);
  return player.pos + sf::Vector2f(r * cos(angle), r * sin(angle));
}

void createPlayer(Player &player, sf::Vector2f pos) {
  player.pos = pos;
  player.health = PLAYER_HEALTH;
  player.cash = 0;
  player.killed = 0;
  player.bullets = PLAYER_BULLETS;
}

void createBubble(Bubble &bubble) {
  bubble.attack = 0;
  bubble.health = rand() % 50 + 20.0;
  bubble.origHealth = bubble.health;
  float r = radius(bubble.health);
  int side = rand() % 4;
  switch (side) {
    case 0:
      bubble.pos = sf::Vector2f(-r, rand() % WIN_H);
      break;
    case 1:
      bubble.pos = sf::Vector2f(rand() % WIN_W, -r);
      break;
    case 2:
      bubble.pos = sf::Vector2f(WIN_W + r, rand() % WIN_H);
      break;
    default:
      bubble.pos = sf::Vector2f(rand() % WIN_W, WIN_H + r);
      break;
  }
  bubble.body.setRadius(r);
  bubble.body.setOrigin(r, r);
  bubble.body.setPosition(bubble.pos);
  bubble.body.setFillColor(sf::Color::Blue);
}

void resizePlayer(Player &player, Gun &gun) {
  float r = radius(player.health);
  player.body.setRadius(r);
  player.body.setOrigin(r, r);
  player.body.setPosition(player.pos);
  player.body.setFillColor(sf::Color::Red);
  gun.body.setOrigin(-radius(player.health), 0);
  gun.body.setPosition(player.pos);
}

void mouseMove(sf::Vector2f mousePos, Player &player, Gun &gun) {
  sf::Vector2f dirPos = mousePos - gunPosition(player, gun);
  if (vAbs(dirPos) > 2 * radius(player.health)) {
    gun.body.setRotation(vAngle(dirPos) * 180.0 / M_PI - 90.0);
  }
}

void setTextBoxes(std::vector<sf::Text> &textBoxes, Player &player) {
  char msg[24];
  snprintf(msg, sizeof(msg), "Cash: $%d", player.cash);
  textBoxes[0].setString(msg);
  snprintf(msg, sizeof(msg), "Health: %d", player.health);
  textBoxes[1].setString(msg);
  snprintf(msg, sizeof(msg), "Killed: %d", player.killed);
  textBoxes[2].setString(msg);
  snprintf(msg, sizeof(msg), "Bullets: %d", player.bullets);
  textBoxes[3].setString(msg);
}

void buyBullets(Player &player) {
  if (player.cash >= AMMO_COST) {
    player.cash -= AMMO_COST;
    player.bullets += AMMO_AMOUNT;
  }
}

void shoot(sf::Vector2f mousePos, Player &player, Gun &gun, std::vector<Bullet> &bullets) {
  if (player.bullets > 0) {
    Bullet bullet;
    createBullet(bullet, gunPosition(player, gun), mousePos);
    bullets.push_back(bullet);
    player.bullets--;
  }
}

int update(sf::RenderWindow &window, float dt, float cooldown[], Player &player, Gun &gun, std::vector<Bubble> &bubbles, std::vector<Bullet> &bullets) {
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
  cooldown[CD_GUN] = (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)) ? cooldown[CD_GUN] - dt : GUN_COOLDOWN;
  if (cooldown[CD_GUN] <= 0) {
    shoot(sf::Vector2f(sf::Mouse::getPosition(window)), player, gun, bullets);
    cooldown[CD_GUN] = GUN_COOLDOWN;
  }
  player.body.setPosition(player.pos);
  gun.body.setPosition(player.pos);

  // Cooldown
  cooldown[CD_BUBBLE] -= dt;
  if (cooldown[CD_BUBBLE] <= 0) {
    Bubble bubble;
    createBubble(bubble);
    bubbles.push_back(bubble);
    cooldown[CD_BUBBLE] = BUBBLE_COOLDOWN;
  }

  // Velocity-based
  auto bullet = bullets.begin();
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
  }
  bool setUi = false;
  auto bubble = bubbles.begin();
  while (bubble != bubbles.end()) {
    if (bubble->health <= 0) {
      player.cash += ceil(bubble->origHealth / 10.0);
      player.killed++;
      bubble = bubbles.erase(bubble);
      setUi = true;
    } else {
      sf::Vector2f dirPos = player.pos - bubble->pos;
      float angle = vAngle(dirPos);
      float s = speed(BUBBLE_SPEED, bubble->health);
      bubble->pos.x += s * dt * cos(angle);
      bubble->pos.y += s * dt * sin(angle);
      if (vAbs(player.pos - bubble->pos) < (radius(player.health) + radius(bubble->health))) {
        bubble->attack -= dt;
        if (bubble->attack <= 0) {
          bubble->attack = BUBBLE_ATTACK;
          player.health -= BUBBLE_DAMAGE;
          resizePlayer(player, gun);
          setUi = true;
        }
      } else {
        bubble->attack = 0.1;
      }
      bubble->body.setPosition(bubble->pos);
      bubble++;
    }
  }

  if (player.health < 0) return -1;
  if (setUi) return 1;
  return 0;
}

void draw(sf::RenderWindow &window, Player &player, Gun &gun, std::vector<Bubble> &bubbles, std::vector<Bullet> &bullets, std::vector<sf::Text> textBoxes) {
  window.clear();
  for (auto &bubble : bubbles) {
    window.draw(bubble.body);
  }
  window.draw(player.body);
  window.draw(gun.body);
  for (auto &bullet : bullets) {
    window.draw(bullet.body);
  }
  for (auto &textBox : textBoxes) {
    window.draw(textBox);
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
  float cooldown[2] = {BUBBLE_COOLDOWN, GUN_COOLDOWN};

  // Create objects
  Player player;
  Gun gun;
  std::vector<Bullet> bullets;
  std::vector<Bubble> bubbles;
  std::vector<sf::Text> textBoxes;
  createPlayer(player, {WIN_W / 2, WIN_H / 2});
  createGun(gun);
  resizePlayer(player, gun);
  for (int i = 0; i < 4; i++) {
    sf::Text textBox;
    textBox.setFont(font);
    textBox.setCharacterSize(24);
    textBox.setPosition(sf::Vector2f(0, 30*i));
    textBox.setFillColor(sf::Color::White);
    textBoxes.push_back(textBox);
  }
  setTextBoxes(textBoxes, player);

  while (window.isOpen()) {
    sf::Event event;
    while (window.pollEvent(event)) {
      switch (event.type) {
        case sf::Event::Closed:
          window.close();
          break;
        case sf::Event::KeyPressed:
          switch (event.key.code) {
            case sf::Keyboard::Escape:
              window.close();
              break;
            case sf::Keyboard::R:
              buyBullets(player);
              setTextBoxes(textBoxes, player);
              break;
          }
          break;
        case sf::Event::MouseMoved:
          mouseMove(sf::Vector2f(sf::Mouse::getPosition(window)), player, gun);
          break;
        case sf::Event::MouseButtonPressed:
          shoot(sf::Vector2f(sf::Mouse::getPosition(window)), player, gun, bullets);
          setTextBoxes(textBoxes, player);
          break;
        default:
          break;
      }
    }

    float dt = clock.restart().asSeconds();
    int response = update(window, dt, cooldown, player, gun, bubbles, bullets);
    switch (response) {
      case -1:
        std::cout << "Game over! You killed " << player.killed << " bubbles." << std::endl;
        window.close();
        break;
      case 1:
        setTextBoxes(textBoxes, player);
        break;
      default:
        break;
    }
    draw(window, player, gun, bubbles, bullets, textBoxes);
  }
  
  return 0;
}
