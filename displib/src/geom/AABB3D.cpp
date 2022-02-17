#include "AABB3D.h"

namespace displib {
	AABB3D::AABB3D() {
		this->min=V3D();
		this->max=V3D();
	}

	AABB3D::AABB3D(float minx, float miny, float minz, float maxx, float maxy, float maxz) {
		this->min=V3D(minx, miny, minz);
		this->max=V3D(maxx, maxy, maxz);
	}

	AABB3D::AABB3D(V3D min_, V3D max_) {
		this->min=min_;
		this->max=max_;
	}

	bool AABB3D::containsPt(V3D pt) {
		bool xOverlap=pt.x>=this->min.x&&pt.x<=this->max.x;
		bool yOverlap=pt.y>=this->min.y&&pt.y<=this->max.y;
		bool zOverlap=pt.z>=this->min.z&&pt.z<=this->max.z;
		return xOverlap&&yOverlap&&zOverlap;
	}

	bool AABB3D::overlapAABB(AABB3D aabb) {
		bool xOverlap=this->min.x<=aabb.max.x&&this->max.x>=aabb.min.x;
		bool yOverlap=this->min.y<=aabb.max.y&&this->max.y>=aabb.min.y;
		bool zOverlap=this->min.z<=aabb.max.z&&this->max.z>=aabb.min.z;
		return xOverlap&&yOverlap&&zOverlap;
	}
}