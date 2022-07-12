#include "AABB2D.h"

namespace displib {
	AABB2D::AABB2D() {
		this->min=V2D();
		this->max=V2D();
	}

	AABB2D::AABB2D(float minx, float miny, float maxx, float maxy) {
		this->min=V2D(minx, miny);
		this->max=V2D(maxx, maxy);
	}

	AABB2D::AABB2D(V2D min_, V2D max_) {
		this->min=min_;
		this->max=max_;
	}

	bool AABB2D::containsPt(V2D pt) {
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
		V2D tr(this->max.x, this->min.y);
		V2D bl(this->min.x, this->max.y);
		rst.drawLine(this->min, tr);
		rst.drawLine(tr, this->max);
		rst.drawLine(this->max, bl);
		rst.drawLine(bl, this->min);
	}
}