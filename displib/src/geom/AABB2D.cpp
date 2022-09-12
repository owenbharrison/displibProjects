#include "AABB2D.h"

namespace displib {
	AABB2D::AABB2D() {};

	AABB2D::AABB2D(float minx, float miny, float maxx, float maxy) {
		this->min=float2(minx, miny);
		this->max=float2(maxx, maxy);
	}

	AABB2D::AABB2D(float2 min_, float2 max_) {
		this->min=min_;
		this->max=max_;
	}

	bool AABB2D::containsPt(float2 pt) {
		bool xOverlap=pt.x>=this->min.x&&pt.x<=this->max.x;
		bool yOverlap=pt.y>=this->min.y&&pt.y<=this->max.y;
		return xOverlap&&yOverlap;
	}

	bool AABB2D::overlapAABB(AABB2D aabb) {
		bool xOverlap=(this->min.x<=aabb.max.x&&this->max.x>=aabb.min.x);
		bool yOverlap=(this->min.y<=aabb.max.y&&this->max.y>=aabb.min.y);
		return xOverlap&&yOverlap;
	}

	void AABB2D::render(Raster& rst) {
		float2 tr(this->max.x, this->min.y);
		float2 bl(this->min.x, this->max.y);
		rst.drawLine(this->min, tr);
		rst.drawLine(tr, this->max);
		rst.drawLine(this->max, bl);
		rst.drawLine(bl, this->min);
	}
}