#include <iostream>
#include "Projectile.h"

using namespace std;

Projectile::Projectile(float x, float y, float angle) {
	this->x = x;
	this->y = y;
	this->angle = angle;
	distance = 0;
}