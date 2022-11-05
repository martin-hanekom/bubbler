#include <iostream>
#include "const.hpp"
#include "bubble.hpp"

void createBubble(Bubble &bubble, const Bubble *parentBubble = NULL) {
  bubble.attack = 0;
  float r = 0;
  if (!parentBubble) {
    bubble.health = rand() % 50 + BUBBLE_HEALTH_OFFSET;
    r = radius(bubble.health);
    switch (rand() % 4) {
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
