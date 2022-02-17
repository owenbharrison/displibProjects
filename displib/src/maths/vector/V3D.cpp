#include "V3D.h"

namespace displib {
	V3D::V3D() {
		this->x=0.0f;
		this->y=0.0f;
		this->z=0.0f;
	}
	V3D::V3D(float x_, float y_, float z_) {
		this->x=x_;
		this->y=y_;
		this->z=z_;
	}

	V3D V3D::add(V3D v) {
		this->x+=v.x;
		this->y+=v.y;
		this->z+=v.z;
		return *this;
	}
	V3D V3D::add(const V3D& a, const V3D& b) {
		V3D c=a;
		return c.add(b);
	}

	V3D V3D::sub(V3D v) {
		this->x-=v.x;
		this->y-=v.y;
		this->z-=v.z;
		return *this;
	}
	V3D V3D::sub(const V3D& a, const V3D& b) {
		V3D c=a;
		return c.sub(b);
	}

	V3D V3D::mult(float f) {
		this->x*=f;
		this->y*=f;
		this->z*=f;
		return *this;
	}
	V3D V3D::mult(const V3D& v, float f) {
		V3D r=v;
		return r.mult(f);
	}

	V3D V3D::div(float f) {
		this->x/=f;
		this->y/=f;
		this->z/=f;
		return *this;
	}
	V3D V3D::div(const V3D& v, float f) {
		V3D r=v;
		return r.div(f);
	}

	float V3D::dot(V3D v) {
		return this->x*v.x+this->y*v.y+this->z*v.z;
	}
	float V3D::dot(V3D a, V3D b) {
		return a.dot(b);
	}

	V3D V3D::cross(V3D v) {
		return V3D(
			this->y*v.z-this->z*v.y,
			this->z*v.x-this->x*v.z,
			this->x*v.y-this->y*v.x
		);
	}
	V3D V3D::cross(V3D a, V3D b) {
		return V3D(
			a.y*b.z-a.z*b.y,
			a.z*b.x-a.x*b.z,
			a.x*b.y-a.y*b.x
		);
	}

	V3D V3D::normal() {
		float f=this->mag();
		if (f!=0.0f) {//NEVER DIVIDE BY ZERO THIS TOOK ME 3 HRS TO FIX, if - something wrong
			this->div(f);
		}
		return *this;
	}
	V3D V3D::normal(const V3D& v) {
		V3D r=v;
		return r.normal();
	}

	float V3D::mag() {
		return sqrt(this->dot(*this));
	}

	std::string V3D::toStr() {
		return "[x: "+std::to_string(this->x)+", y: "+std::to_string(this->y)+", z: "+std::to_string(this->z)+"]";
	}

	V3D V3D::operator +(const V3D& v) {
		return V3D::add(*this, v);
	}

	V3D V3D::operator -(const V3D& v) {
		return V3D::sub(*this, v);
	}

	V3D V3D::operator *(float f) {
		return V3D::mult(*this, f);
	}

	V3D V3D::operator /(float f) {
		return V3D::div(*this, f);
	}

	V3D& V3D::operator +=(const V3D& v) {
		this->add(v);
		return *this;
	}

	V3D& V3D::operator -=(const V3D& v) {
		this->sub(v);
		return *this;
	}

	V3D& V3D::operator *=(float f) {
		this->mult(f);
		return *this;
	}

	V3D& V3D::operator /=(float f) {
		this->div(f);
		return *this;
	}

	V3D& V3D::operator =(const V3D& v) {
		this->x=v.x;
		this->y=v.y;
		this->z=v.z;
		return *this;
	}
}