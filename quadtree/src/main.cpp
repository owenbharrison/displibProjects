#include <vector>

#include "Engine.h"
#include "maths/Maths.h"
#include "geom/AABB2D.h"
using namespace displib;

template<class T>
void addAll(std::vector<T>& a, const std::vector<T>& b) {
	a.insert(a.end(), b.begin(), b.end());
}

struct quadTree {
	const int cap=1;
	AABB2D bounds;
	std::vector<float2> points;

	quadTree* northWest=nullptr;
	quadTree* northEast=nullptr;
	quadTree* southWest=nullptr;
	quadTree* southEast=nullptr;

	quadTree(AABB2D bounds_) {
		bounds=bounds_;
	}

	void subdivide() {
		float2 ctr=(bounds.min+bounds.max)/2;
		northWest=new quadTree(AABB2D(bounds.min, ctr));
		northEast=new quadTree(AABB2D(float2(ctr.x, bounds.min.y), float2(bounds.max.x, ctr.y)));
		southWest=new quadTree(AABB2D(float2(bounds.min.x, ctr.y), float2(ctr.x, bounds.max.y)));
		southEast=new quadTree(AABB2D(ctr, bounds.max));
	}

	bool insert(float2 pt) {
		if (!bounds.containsPt(pt))
			return false;

		if (northWest==nullptr) {
			if (points.size()<cap) {
				points.push_back(pt);
				return true;
			}
			subdivide();
		}

		if (northWest->insert(pt)) return true;
		if (northEast->insert(pt)) return true;
		if (southWest->insert(pt)) return true;
		if (southEast->insert(pt)) return true;

		return false;
	}

	std::vector<float2> queryRange(AABB2D range) {
		std::vector<float2> pts;
		if (!bounds.overlapAABB(range))
			return pts; // empty list

		for (float2& pt:points) {
			if (range.containsPt(pt)) pts.push_back(pt);
		}

		if (northWest==nullptr) return pts;

		addAll(pts, northWest->queryRange(range));
		addAll(pts, northEast->queryRange(range));
		addAll(pts, southWest->queryRange(range));
		addAll(pts, southEast->queryRange(range));

		return pts;
	}

	void show(Raster& rst) {
		bounds.render(rst);

		if (northWest==nullptr) return;

		northWest->show(rst);
		northEast->show(rst);
		southWest->show(rst);
		southEast->show(rst);
	}

	void showPoints(Raster& rst) {
		for (const float2& pt:points) {
			rst.putPixel(pt);
		}

		if (northWest==nullptr) return;

		northWest->showPoints(rst);
		northEast->showPoints(rst);
		southWest->showPoints(rst);
		southEast->showPoints(rst);
	}
};

class Demo : public Engine {
	public:
	bool down=false, wasDown=false;
	quadTree* mainTree;

	void setup() override {
		mainTree=new quadTree(AABB2D(0, 0, width-1, height-1));
	}

	void update(float dt) override {
		float2 mousePos(mouseX, mouseY);
		down=getKey(VK_SPACE);
		if (down&&!wasDown) {
			mainTree->insert(mousePos);
		}
		wasDown=down;
	}

	void draw(Raster& rst) override {
		rst.setChar(' ');
		rst.fillRect(0, 0, width, height);

		rst.setChar('#');
		mainTree->show(rst);

		rst.setChar(0x2588);
		mainTree->showPoints(rst);
	}
};

int main() {
	srand(time(NULL));

	//init custom graphics engine
	Demo d;
	d.startWindowed(4, 200, 150);

	return 0;
}