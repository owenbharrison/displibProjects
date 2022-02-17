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

	void AABB2D::render(Raster& gfx) {
		gfx.drawLine((int)this->min.x, (int)this->min.y, (int)this->max.x, (int)this->min.y);
		gfx.drawLine((int)this->max.x, (int)this->min.y, (int)this->max.x, (int)this->max.y);
		gfx.drawLine((int)this->max.x, (int)this->max.y, (int)this->min.x, (int)this->max.y);
		gfx.drawLine((int)this->min.x, (int)this->max.y, (int)this->min.x, (int)this->min.y);
	}
}