#include "V2D.h"
#include <cmath>

namespace displib {
	V2D::V2D() {
		this->x=0.0f;
		this->y=0.0f;
	}
	V2D::V2D(float x_, float y_) {
		this->x=x_;
		this->y=y_;
	}

	V2D V2D::add(V2D v) {
		this->x+=v.x;
		this->y+=v.y;
		return *this;
	}
	V2D V2D::add(const V2D& a, const V2D& b) {
		V2D c=a;
		return c.add(b);
	}

	V2D V2D::sub(V2D v) {
		this->x-=v.x;
		this->y-=v.y;
		return *this;
	}
	V2D V2D::sub(const V2D& a, const V2D& b) {
		V2D c=a;
		return c.sub(b);
	}

	V2D V2D::mult(float f) {
		this->x*=f;
		this->y*=f;
		return *this;
	}
	V2D V2D::mult(const V2D& v, float f) {
		V2D r=v;
		return r.mult(f);
	}

	V2D V2D::div(float f) {
		this->x/=f;
		this->y/=f;
		return *this;
	}
	V2D V2D::div(const V2D& v, float f) {
		V2D r=v;
		return r.div(f);
	}

	float V2D::dot(V2D v) {
		return this->x*v.x+this->y*v.y;
	}
	float V2D::dot(V2D a, V2D b) {
		return a.dot(b);
	}

	V2D V2D::normal() {
		float f=this->mag();
		if (f!=0.0f) {//NEVER DIVIDE BY ZERO THIS TOOK ME 3 HRS TO FIX, if - something wrong
			this->div(f);
		}
		return *this;
	}
	V2D V2D::normal(const V2D& v) {
		V2D r=v;
		return r.normal();
	}

	float V2D::heading() {
		return atan2f(this->y, this->x);
	};

	float V2D::mag() {
		return sqrtf(this->dot(*this));
	}

	V2D V2D::fromAngle(float f) {
		return V2D(cosf(f), sinf(f));
	}

	std::string V2D::toStr() {
		return "[x: "+std::to_string(this->x)+", y: "+std::to_string(this->y)+"]";
	}

	V2D V2D::operator +(const V2D& v) {
		return V2D::add(*this, v);
	}

	V2D V2D::operator -(const V2D& v) {
		return V2D::sub(*this, v);
	}

	V2D V2D::operator *(float f) {
		return V2D::mult(*this, f);
	}

	V2D V2D::operator /(float f) {
		return V2D::div(*this, f);
	}

	V2D& V2D::operator +=(const V2D& v) {
		this->add(v);
		return *this;
	}

	V2D& V2D::operator -=(const V2D& v) {
		this->sub(v);
		return *this;
	}

	V2D& V2D::operator *=(float f) {
		this->mult(f);
		return *this;
	}

	V2D& V2D::operator /=(float f) {
		this->div(f);
		return *this;
	}

	V2D& V2D::operator =(const V2D& v) {
		this->x=v.x;
		this->y=v.y;
		return *this;
	}
}