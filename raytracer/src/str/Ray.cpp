#include "Ray.h"

Ray::Ray() {
	this->origin=displib::V3D();
	this->dir=displib::V3D();
}

Ray::Ray(displib::V3D origin_, displib::V3D dir_) {
	this->origin=origin_;
	this->dir=dir_;
}