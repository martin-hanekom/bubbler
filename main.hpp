#ifndef MAIN_HPP
#define MAIN_HPP

#include <SFML/Graphics.hpp>
#include "const.hpp"

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
  sf::CircleShape hat;
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
  float alpha;
} Package;

typedef struct {
  sf::RectangleShape body;
  sf::Vector2f pos;
  sf::Vector2f size;
} Wall;

typedef struct {
  int state;
  int wave;
  int numBubbles;
  int numSpawned;
  float splash;
  float cooldown[NUM_CD] = {BUBBLE_COOLDOWN_MAX, GUN_COOLDOWN};
} Game;

#endif
