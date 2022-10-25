#include <iostream>
#include <cmath>
#include <cstdlib>
#include <time.h>
#include <vector>
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>

#define WIN_W 1600
#define WIN_H 900
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
#define GUN_COOLDOWN 0.2
#define BULLET_W 5
#define BULLET_H 1
#define BULLET_SPEED 300
#define BULLET_ALT 5
#define BULLET_DAMAGE 10
#define AMMO_COST 15
#define AMMO_AMOUNT 30
#define BUBBLE_SPEED 30
#define BUBBLE_COOLDOWN 2
#define BUBBLE_DAMAGE 20
#define BUBBLE_ATTACK 1

#define ST_INIT 0
#define ST_PLAY 1
#define ST_PAUSE 2

#define NUM_CD 3
#define CD_BUBBLE 0
#define CD_GUN 1

#define NUM_TX 5
#define TX_CASH 0
#define TX_HEALTH 1
#define TX_KILLED 2
#define TX_BULLETS 3
#define TX_SPLASH 4

#define NUM_SOUNDS 5
#define SOUND_POP 0
#define SOUND_RELOAD 1
#define SOUND_OW 2
#define SOUND_SHOT 3
#define SOUND_EMPTY 4

// struct definitions
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

typedef struct {
  sf::CircleShape body;
  sf::Vector2f pos;
  int health;
} Package;

// global variables
int gameState = ST_INIT;
Player player;
Gun gun;
std::vector<Bullet> bullets;
std::vector<Bubble> bubbles;
std::vector<Package> package;
sf::Text textBoxes[NUM_TX] = {sf::Text(), sf::Text(), sf::Text(), sf::Text()};
sf::Font font;
std::string soundFiles[NUM_SOUNDS] = {"lib/pop.wav", "lib/reload.wav", "lib/ow.wav", "lib/shot.wav", "lib/empty.wav"};
sf::SoundBuffer soundBuffers[NUM_SOUNDS];
sf::Sound sounds[NUM_SOUNDS];
float cooldown[NUM_CD] = {BUBBLE_COOLDOWN, GUN_COOLDOWN};

// helper functions
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

void setTextBoxes() {
  char msg[64];
  snprintf(msg, sizeof(msg), "Cash: $%d", player.cash);
  textBoxes[TX_CASH].setString(msg);
  snprintf(msg, sizeof(msg), "Health: %d", player.health);
  textBoxes[TX_HEALTH].setString(msg);
  snprintf(msg, sizeof(msg), "Killed: %d", player.killed);
  textBoxes[TX_KILLED].setString(msg);
  snprintf(msg, sizeof(msg), "Bullets: %d", player.bullets);
  textBoxes[TX_BULLETS].setString(msg);
}

sf::Vector2f gunPosition() {
  float angle = gun.body.getRotation() * M_PI / 180.0;
  float r = radius(player.health);
  return player.pos + sf::Vector2f(r * cos(angle), r * sin(angle));
}

void resizePlayer() {
  float r = radius(player.health);
  player.body.setRadius(r);
  player.body.setOrigin(r, r);
  player.body.setPosition(player.pos);
  player.body.setFillColor(sf::Color::Red);
  gun.body.setOrigin(-radius(player.health), 0);
  gun.body.setPosition(player.pos);
}

// create dynamic objects
void createBullet(Bullet &bullet, sf::Vector2f &mousePos) {
  bullet.pos = gunPosition();
  bullet.body.setSize(sf::Vector2f(BULLET_W, BULLET_H));
  bullet.body.setOrigin(BULLET_W / 2, BULLET_H / 2);
  bullet.body.setPosition(bullet.pos);
  bullet.body.setFillColor(sf::Color::Yellow);
  sf::Vector2f dirPos = mousePos - bullet.pos;
  bullet.angle = vAngle(dirPos);
  bullet.body.setRotation(bullet.angle * 180.0 / M_PI);
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

// input methods
void mouseMove(sf::Vector2f mousePos) {
  sf::Vector2f dirPos = mousePos - gunPosition();
  if (vAbs(dirPos) > 2 * radius(player.health)) {
    gun.body.setRotation(vAngle(dirPos) * 180.0 / M_PI - 90.0);
  }
}

void buyBullets() {
  if (player.cash >= AMMO_COST) {
    player.cash -= AMMO_COST;
    player.bullets += AMMO_AMOUNT;
    setTextBoxes();
    sounds[SOUND_RELOAD].play();
  }
}

void shoot(sf::Vector2f mousePos) {
  if (gameState != ST_PLAY) return;
  if (player.bullets > 0) {
    Bullet bullet;
    createBullet(bullet, mousePos);
    bullets.push_back(bullet);
    player.bullets--;
    setTextBoxes();
    sounds[SOUND_SHOT].play();
  } else {
    sounds[SOUND_EMPTY].play();
  }
}

int update(sf::RenderWindow &window, float dt) {
  if (gameState != ST_PLAY) return 0;

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
    shoot(sf::Vector2f(sf::Mouse::getPosition(window)));
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
    sf::Vector2f altPos = bullet->pos - sf::Vector2f(BULLET_ALT * cos(bullet->angle), BULLET_ALT * sin(bullet->angle));
    bool erased = false;
    for (auto &bubble : bubbles) {
      float r = radius(bubble.health);
      if (vAbs(bubble.pos - bullet->pos) <= r || vAbs(bubble.pos - altPos) <= r) {
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
      sounds[SOUND_POP].play();
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
          resizePlayer();
          setUi = true;
          sounds[SOUND_OW].play();
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

void draw(sf::RenderWindow &window) {
  window.clear();
  for (auto &bubble : bubbles) {
    window.draw(bubble.body);
  }
  window.draw(player.body);
  window.draw(gun.body);
  for (auto &bullet : bullets) {
    window.draw(bullet.body);
  }
  for (int i = 0; i < NUM_TX - 1; i++) {
    window.draw(textBoxes[i]);
  }
  if (gameState == ST_INIT || gameState == ST_PAUSE) {
    window.draw(textBoxes[NUM_TX - 1]);
  }
  window.display();
}

void restart() {
  gameState = ST_INIT;
  player.pos = sf::Vector2f(WIN_W / 2, WIN_H / 2);
  player.health = PLAYER_HEALTH;
  player.cash = 0;
  player.killed = 0;
  player.bullets = PLAYER_BULLETS;

  gun.body.setSize(sf::Vector2f(GUN_W, GUN_H));
  gun.body.setFillColor(sf::Color::Magenta);
  resizePlayer();

  bullets.clear();
  bubbles.clear();

  for (int i = 0; i < NUM_TX; i++) {
    textBoxes[i].setFont(font);
    textBoxes[i].setCharacterSize(24);
    textBoxes[i].setPosition(sf::Vector2f(0, 30*i));
    textBoxes[i].setFillColor(sf::Color::White);
  }
  textBoxes[TX_SPLASH].setPosition(sf::Vector2f(WIN_W / 2 - 200, WIN_H / 2 - 15));
  textBoxes[TX_SPLASH].setString("Press Spacebar to Play, C to reload and R to restart!");
  setTextBoxes();
}

int main() {
  srand(time(0));
  sf::RenderWindow window(sf::VideoMode(WIN_W, WIN_H), "Bubbler", sf::Style::Titlebar | sf::Style::Close);
  window.setVerticalSyncEnabled(true);
  window.setFramerateLimit(FS);
  sf::Clock clock;
  if (!font.loadFromFile("lib/arial.ttf")) {
    std::cout << "Loading asset error!" << std::endl;
    return 1;
  }
  for (int i = 0; i < NUM_SOUNDS; i++) {
    if (!soundBuffers[i].loadFromFile(soundFiles[i])) {
      std::cout << "Loading sound from " << soundFiles[i] << " failed!" << std::endl;
      return 1;
    }
    sounds[i].setBuffer(soundBuffers[i]);
  }
  sounds[SOUND_OW].setVolume(10);
  sounds[SOUND_SHOT].setVolume(10);
  sounds[SOUND_EMPTY].setVolume(10);
  restart();

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
            case sf::Keyboard::Space:
              gameState = gameState % 2 + 1;
              break;
            case sf::Keyboard::C:
              buyBullets();
              break;
            case sf::Keyboard::R:
              restart();
              break;
          }
          break;
        case sf::Event::MouseMoved:
          mouseMove(sf::Vector2f(sf::Mouse::getPosition(window)));
          break;
        case sf::Event::MouseButtonPressed:
          shoot(sf::Vector2f(sf::Mouse::getPosition(window)));
          break;
        default:
          break;
      }
    }

    float dt = clock.restart().asSeconds();
    int response = update(window, dt);
    switch (response) {
      case -1:
        std::cout << "Game over! You killed " << player.killed << " bubbles." << std::endl;
        restart();
        gameState = ST_INIT;
        break;
      case 1:
        setTextBoxes();
        break;
      default:
        break;
    }
    draw(window);
  }
  
  return 0;
}
