#ifndef MAIN_HPP
#define MAIN_HPP

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
#define BULLET_SPEED 250
#define BULLET_ALT 10
#define BULLET_DAMAGE 10
#define AMMO_COST 15
#define AMMO_AMOUNT 30

#define GRENADE_RADIUS 10
#define GRENADE_SPEED 150
#define GRENADE_COST 10
#define GRENADE_BLAST 100
#define GRENADE_DAMAGE 40

#define BUBBLE_SPEED 30
#define BUBBLE_COOLDOWN_MAX 2
#define BUBBLE_COOLDOWN_MIN 0.5
#define BUBBLE_DAMAGE 20
#define BUBBLE_ATTACK 1
#define PACKAGE_RADIUS 10

#define WAVE_OFFSET 10
#define WAVE_INC 10
#define WAVE_COOLDOWN 0.1
#define WAVE_SPLASH_LEN 3

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
#define INIT_MSG "Press 1 for singleplayer, 2 for multiplayer"
#define PAUSE_MSG "Press Spacebar to Play, R to reload and Z to restart!"

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
  sf::CircleShape body;
  sf::Vector2f pos;
  float angle;
} Grenade;

typedef struct {
  sf::CircleShape body;
  sf::Vector2f pos;
  float alpha;
} Blast;

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
  float alpha;
} Package;

typedef struct {
  int state;
  int wave;
  int numBubbles;
  int numSpawned;
  float splash;
  float cooldown[NUM_CD] = {BUBBLE_COOLDOWN_MAX, GUN_COOLDOWN};
} Game;

const int PACKAGE_COLOR[] = {0, 255, 0};
const int BLAST_COLOR[] = {255, 255, 255};

#endif
