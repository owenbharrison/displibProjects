#include "Hit.h"

Hit::Hit() {
	this->ray=Ray();
	this->dist=0;
	this->pos=displib::V3D();
	this->norm=displib::V3D();
	this->col=0x000F;
}

Hit::Hit(Ray& ray_, float dist_, displib::V3D& pos_, displib::V3D& norm_, short col_) {
	this->ray=ray_;
	this->dist=dist_;
	this->pos=pos_;
	this->norm=norm_;
	this->col=col_;
}