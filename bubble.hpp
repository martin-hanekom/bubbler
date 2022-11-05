#ifndef BUBBLE_HPP
#define BUBBLE_HPP

#include <SFML/Graphics.hpp>
#include "const.hpp"

typedef struct {
  sf::CircleShape body;
  sf::CircleShape hat;
  sf::Vector2f pos;
  int health;
  int origHealth;
  float speed;
  float attack;
  int modifiers[NUM_MODS] = {rand() % PACKAGE_ODDS, rand() % DOUBLE_ODDS};
} Bubble;

#endif
