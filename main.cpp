#include <iostream>
#include <cmath>
#include <cstdlib>
#include <time.h>
#include <vector>
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include "main.hpp"

// global variables
Game game;
Player player;
Gun gun;
std::vector<Bullet> bullets;
std::vector<Grenade> grenades;
std::vector<Bubble> bubbles;
std::vector<Package> packages;
Wall walls[NUM_WALLS];
std::vector<Blast> blasts;
sf::Text textBoxes[NUM_TX] = {sf::Text(), sf::Text(), sf::Text(), sf::Text()};
sf::Font font;
sf::SoundBuffer soundBuffers[NUM_SOUNDS];
sf::Sound sounds[NUM_SOUNDS];

// TODO: hat
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

float bubbleCooldown() {
  float newCooldown = BUBBLE_COOLDOWN_MAX - WAVE_COOLDOWN * game.wave;
  return (newCooldown > BUBBLE_COOLDOWN_MIN) ? newCooldown : BUBBLE_COOLDOWN_MIN;
}

void setTextBoxes() {
  char msg[128];
  snprintf(msg, sizeof(msg), "Cash: $%d", player.cash);
  textBoxes[TX_CASH].setString(msg);
  snprintf(msg, sizeof(msg), "Health: %d", player.health);
  textBoxes[TX_HEALTH].setString(msg);
  snprintf(msg, sizeof(msg), "Wave: %d", game.wave);
  textBoxes[TX_WAVE].setString(msg);
  snprintf(msg, sizeof(msg), "Killed: %d", player.killed);
  textBoxes[TX_KILLED].setString(msg);
  snprintf(msg, sizeof(msg), "Bullets: %d", player.bullets);
  textBoxes[TX_BULLETS].setString(msg);
  textBoxes[TX_INFO].setString("Ammo [R]: $15, Grenade [F]: $20");
}

void displaySplash(std::string msg, float duration) {
  textBoxes[TX_SPLASH].setString(msg);
  sf::FloatRect size = textBoxes[TX_SPLASH].getLocalBounds();
  textBoxes[TX_SPLASH].setOrigin(size.width / 2, size.height / 2);
  game.splash = duration;
}

void nextRound() {
  game.wave++;
  game.numBubbles = WAVE_OFFSET + WAVE_INC * game.wave;
  game.numSpawned = 0;
  char msg[64];
  snprintf(msg, sizeof(msg), "Wave %d", game.wave);
  displaySplash(msg, WAVE_SPLASH_LEN);
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
  player.hat.setRadius(HAT_RADIUS);
  player.hat.setOrigin(HAT_RADIUS, HAT_RADIUS);
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

void createGrenade(Grenade &grenade, sf::Vector2f &mousePos) {
  grenade.pos = gunPosition();
  grenade.body.setRadius(GRENADE_RADIUS);
  grenade.body.setOrigin(GRENADE_RADIUS, GRENADE_RADIUS);
  grenade.body.setPosition(grenade.pos);
  grenade.body.setFillColor(sf::Color::Cyan);
  sf::Vector2f dirPos = mousePos - grenade.pos;
  grenade.angle = vAngle(dirPos);
}

void createBlast(Blast &blast, sf::Vector2f pos) {
  blast.pos = pos;
  blast.alpha = 128;
  blast.body.setRadius(GRENADE_BLAST);
  blast.body.setOrigin(GRENADE_BLAST, GRENADE_BLAST);
  blast.body.setPosition(blast.pos);
  blast.body.setFillColor(sf::Color(BLAST_COLOR[0], BLAST_COLOR[1], BLAST_COLOR[2])); 
}

void createBubble(Bubble &bubble, const Bubble *parentBubble = NULL) {
  bubble.attack = 0;
  float r = 0;
  if (!parentBubble) {
    bubble.health = rand() % 50 + BUBBLE_HEALTH_OFFSET;
    r = radius(bubble.health);
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
  } else {
    bubble.health = parentBubble->origHealth;
    bubble.pos = parentBubble->pos + sf::Vector2f(rand() % 40 - 20, rand() % 40 - 20);
    r = radius(bubble.health);
  }
  bubble.origHealth = bubble.health;
  bubble.body.setRadius(r);
  bubble.body.setOrigin(r, r);
  bubble.body.setPosition(bubble.pos);
  bubble.body.setFillColor(sf::Color::Blue);
  bubble.hat.setRadius(HAT_RADIUS);
  bubble.hat.setOrigin(HAT_RADIUS, HAT_RADIUS);
  bubble.hat.setPosition(bubble.pos);
  if (bubble.modifiers[MOD_PACKAGE] == MOD_PACKAGE) {
    bubble.hat.setFillColor(sf::Color::Green);
  } else if (bubble.modifiers[MOD_DOUBLE] == MOD_DOUBLE) {
    bubble.hat.setFillColor(sf::Color::Black);
  }
}

void createWalls() {
  for (int i = 0; i < NUM_WALLS; i++) {
    sf::Vector2f anchor((i / 2) % 4 * (WIN_W / 4), (i % 2) * (WIN_H / 2));
    walls[i].pos = anchor + sf::Vector2f(rand() % (WIN_W / 4 + 2 * WALL_OVERLAP) - WALL_OVERLAP, rand() % (WIN_H / 2 + 2 * WALL_OVERLAP) - WALL_OVERLAP);
    if (rand() % 2) {
      walls[i].size = sf::Vector2f(rand() % WALL_MAX + WALL_MIN, WALL_BASE);
    } else {
      walls[i].size = sf::Vector2f(WALL_BASE, rand() % WALL_MAX + WALL_MIN);
    }
    walls[i].body.setPosition(walls[i].pos);
    walls[i].body.setSize(walls[i].size);
    walls[i].body.setOrigin(walls[i].size.x / 2, walls[i].size.y / 2);
  }
}

bool wallCollide(sf::Vector2f pos, float radius = 0) {

}

void resizeBubble(Bubble &bubble) {
  if (bubble.health > 0) {
    float r = radius(bubble.health);
    bubble.body.setRadius(r);
    bubble.body.setOrigin(r, r);
  }
}

void createPackage(Package &package, sf::Vector2f pos) {
  package.pos = pos;
  package.health = rand() % 2 * 10 + PACKAGE_MIN_HEALTH;
  package.alpha = 255;
  package.body.setRadius(PACKAGE_RADIUS);
  package.body.setOrigin(PACKAGE_RADIUS, PACKAGE_RADIUS);
  package.body.setPosition(pos);
  package.body.setFillColor(sf::Color(PACKAGE_COLOR[0], PACKAGE_COLOR[1], PACKAGE_COLOR[2]));
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
  if (game.state != ST_PLAY) return;
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

void shootGrenade(sf::Vector2f mousePos) {
  if (game.state != ST_PLAY) return;
  if (player.cash >= GRENADE_COST) {
    player.cash -= GRENADE_COST;
    Grenade grenade;
    createGrenade(grenade, mousePos);
    grenades.push_back(grenade);
  } else {
    sounds[SOUND_EMPTY].play();
  }
}

int update(sf::RenderWindow &window, float dt) {
  if (game.state != ST_PLAY) return 0;

  // Keypress
  if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) {
    if (!wallCollide(player.body)) {
      player.pos.y -= PLAYER_SPEED * dt;
    }
  } else if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) {
    player.pos.y += PLAYER_SPEED * dt;
  }
  if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
    player.pos.x -= PLAYER_SPEED * dt;
  } else if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
    player.pos.x += PLAYER_SPEED * dt;
  }
  game.cooldown[CD_GUN] = (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)) ? game.cooldown[CD_GUN] - dt : GUN_COOLDOWN;
  if (game.cooldown[CD_GUN] <= 0) {
    shoot(sf::Vector2f(sf::Mouse::getPosition(window)));
    game.cooldown[CD_GUN] = GUN_COOLDOWN;
  }
  player.body.setPosition(player.pos);
  gun.body.setPosition(player.pos);
  game.splash = (game.splash > 0) ? game.splash - dt : 0;

  auto package = packages.begin();
  float playerRadius = radius(player.health);
  bool setUi = false;
  while (package != packages.end()) {
    package->alpha -= dt * PACKAGE_FADE;
    package->body.setFillColor(sf::Color(PACKAGE_COLOR[0], PACKAGE_COLOR[1], PACKAGE_COLOR[2], ceil(package->alpha)));
    if (vAbs(player.pos - package->pos) <= playerRadius + PACKAGE_RADIUS) {
      player.health += package->health;
      resizePlayer();
      package = packages.erase(package);
      setUi = true;
      sounds[SOUND_HEAL].play();
    } else if (package->alpha <= 0) {
      package = packages.erase(package);
    } else {
      package++;
    }
  }

  // Cooldown
  if (game.numSpawned < game.numBubbles) {
    game.cooldown[CD_BUBBLE] -= dt;
    if (game.cooldown[CD_BUBBLE] <= 0) {
      Bubble bubble;
      createBubble(bubble);
      bubbles.push_back(bubble);
      game.numSpawned++;
      game.cooldown[CD_BUBBLE] = bubbleCooldown();
    }
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
        resizeBubble(bubble);
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

  auto grenade = grenades.begin();
  while (grenade != grenades.end()) {
    grenade->pos.x += GRENADE_SPEED * dt * cos(grenade->angle);
    grenade->pos.y += GRENADE_SPEED * dt * sin(grenade->angle);
    bool erased = false;
    for (auto &bubble : bubbles) {
      if (vAbs(bubble.pos - grenade->pos) <= radius(bubble.health) + GRENADE_RADIUS) {
        erased = true;
        break;
      }
    }
    if (erased) {
      Blast blast;
      createBlast(blast, grenade->pos);
      blasts.push_back(blast);
      for (auto &bubble : bubbles) {
        if (vAbs(bubble.pos - grenade->pos) <= GRENADE_BLAST) {
          bubble.health -= GRENADE_DAMAGE;
          resizeBubble(bubble);
        }
      }
      sounds[SOUND_BLAST].play();
    }
    if (erased || grenade->pos.x < 0 || grenade->pos.y > WIN_W || grenade->pos.y < 0 || grenade->pos.y > WIN_H) {
      grenade = grenades.erase(grenade);
    } else {
      grenade->body.setPosition(grenade->pos);
      grenade++;
    }
  }
  auto blast = blasts.begin();
  while (blast != blasts.end()) {
    blast->alpha -= dt * 100;
    if (blast->alpha <= 0) {
      blast = blasts.erase(blast);
    } else {
      blast->body.setFillColor(sf::Color(BLAST_COLOR[0], BLAST_COLOR[0], BLAST_COLOR[0], blast->alpha));
      blast++;
    }
  }

  std::vector<Bubble> newBubbles;
  auto bubble = bubbles.begin();
  while (bubble != bubbles.end()) {
    if (bubble->health <= 0) {
      player.cash += ceil(bubble->origHealth / BULLET_DAMAGE) + 1;
      if (bubble->modifiers[MOD_PACKAGE] == MOD_PACKAGE) {
        Package package;
        createPackage(package, bubble->pos);
        packages.push_back(package);
      } else if (bubble->modifiers[MOD_DOUBLE] == MOD_DOUBLE) {
        for (int i = 0; i < 2; i++) {
          Bubble newBubble;
          createBubble(newBubble, &(*bubble));
          newBubbles.push_back(newBubble);
        }
      }
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
      bubble->hat.setPosition(bubble->pos);
      bubble++;
    }
  }
  if (newBubbles.size()) {
    bubbles.insert(bubbles.end(), newBubbles.begin(), newBubbles.end());
  }

  if (game.numBubbles == game.numSpawned && bubbles.size() == 0) {
    nextRound();
  }

  if (player.health < 0) return -1;
  if (setUi) return 1;
  return 0;
}

void draw(sf::RenderWindow &window) {
  window.clear();
  for (auto &wall : walls) {
    window.draw(wall.body);
  }
  for (auto &bubble : bubbles) {
    window.draw(bubble.body);
    for (int i = 0; i < NUM_MODS; i++) {
      if (bubble.modifiers[i] == i) {
        window.draw(bubble.hat);
        break;
      }
    }
  }
  window.draw(player.body);
  window.draw(gun.body);
  for (auto &bullet : bullets) {
    window.draw(bullet.body);
  }
  for (auto &grenade : grenades) {
    window.draw(grenade.body);
  }
  for (auto &package : packages) {
    window.draw(package.body);
  }
  for (auto &blast : blasts) {
    window.draw(blast.body);
  }
  for (int i = 0; i < TX_SPLASH; i++) {
    window.draw(textBoxes[i]);
  }
  if (game.splash > 0) {
    window.draw(textBoxes[TX_SPLASH]);
  }
  window.display();
}

void restart() {
  game.state = ST_INIT;
  game.wave = 0;

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
  packages.clear();
  createWalls();

  for (int i = 0; i < NUM_TX; i++) {
    textBoxes[i].setFont(font);
    textBoxes[i].setCharacterSize(24);
    textBoxes[i].setPosition(sf::Vector2f(10, 10 + 30*i));
    textBoxes[i].setFillColor(sf::Color::White);
  }
  textBoxes[TX_SPLASH].setPosition(sf::Vector2f(WIN_W / 2, WIN_H / 2));
  textBoxes[TX_INFO].setPosition(sf::Vector2f(10, WIN_H - 40));
  setTextBoxes();
  displaySplash(PAUSE_MSG, 0.1);
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
  sounds[SOUND_BLAST].setVolume(20);
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
              if (game.state == ST_INIT) {
                nextRound();
              } else {
                displaySplash(PAUSE_MSG, 0.1);
              }
              game.state = game.state % 2 + 1;
              break;
            case sf::Keyboard::R:
              buyBullets();
              break;
            case sf::Keyboard::F:
              shootGrenade(sf::Vector2f(sf::Mouse::getPosition(window)));
              break;
            case sf::Keyboard::Z:
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
        game.state = ST_INIT;
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
