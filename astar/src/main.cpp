#include <vector>

#include "Engine.h"
using namespace displib;

struct pt {
	int x=0, y=0;

	pt() {}

	pt(int x_, int y_) : x(x_), y(y_) {}

	bool operator==(pt o) {
		return x==o.x&&y==o.y;
	}
};

struct node {
	pt pos;
	node* parent=nullptr;
	bool walkable=true;
	std::vector<node*> neighbors;
	float fCost=0, gCost=0, hCost=0;

	node() {}

	node(pt pos_) :pos(pos_) {}
};

class Demo : public Engine {
	public:
	bool running=false;
	bool down=false, wasDown=false;

	std::vector<node*> path;

	node* nodeGrid=nullptr;
	node* startNode=nullptr, * endNode=nullptr;
	std::vector<node*> openSet, closedSet;

	float timer=0;

	bool inRange(int i, int j) {
		return i>=0&&j>=0&&i<width&& j<height;
	}

	int ix(int i, int j) {
		return i+j*width;
	}

	float distance(node a, node b) {
		float dx=abs(a.pos.x-b.pos.x);
		float dy=abs(a.pos.y-b.pos.y);
		if (dx>dy) return 14*dy+10*(dx-dy);
		return 14*dx+10*(dy-dx);
	}

	void setup() override {
		//init mem
		nodeGrid=new node[width*height];
		//init each node
		for (int x=0; x<width; x++) {
			for (int y=0; y<height; y++) {
				nodeGrid[ix(x, y)]=node(pt(x, y));

				int dx=x-width/2;
				int dy=y-height/2;
				float dist=sqrtf(dx*dx+dy*dy);
				if (dist<width*.33f) nodeGrid[ix(x, y)].walkable=false;
			}
		}
		//init each node's neighborhood
		for (int x=0; x<width; x++) {
			for (int y=0; y<height; y++) {
				node& n=nodeGrid[ix(x, y)];
				for (int i=-1; i<=1; i++) {
					for (int j=-1; j<=1; j++) {
						//dont check self
						if (i==0&&j==0) continue;

						int rx=x+i;
						int ry=y+j;
						if (inRange(rx, ry)) {
							n.neighbors.push_back(&nodeGrid[ix(rx, ry)]);
						}
					}
				}
			}
		}

		startNode=&nodeGrid[ix(width/8, height/8)];
		endNode=&nodeGrid[ix(width*7/8, height*7/8)];
		openSet.push_back(startNode);
	}

	void update(float dt) override {
		down=getKey(VK_RETURN);
		if (down&&!wasDown) {
			path.clear();
			openSet.clear();
			closedSet.clear();

			if (!running) {
				openSet.push_back(startNode);
			}
			running=!running;
		}
		wasDown=down;

		//if try change, pause
		bool setStart=getKey('S');
		bool setEnd=getKey('E');
		bool setWall=getKey('W');
		bool setFloor=getKey('F');
		bool toClear=getKey('C');
		if (setStart||setEnd||setWall||setFloor||toClear) {
			running=false;

			path.clear();
			openSet.clear();
			closedSet.clear();

			//mouse change options
			if (inRange(mouseX, mouseY)) {
				node& mouseNode=nodeGrid[ix(mouseX, mouseY)];
				if (setStart) startNode=&mouseNode;
				if (setEnd) endNode=&mouseNode;
				if (setWall) mouseNode.walkable=false;
				if (setFloor) mouseNode.walkable=true;
			}

			//make entire grid walkable
			if (toClear) {
				for (int x=0; x<width; x++) {
					for (int y=0; y<height; y++) {
						nodeGrid[ix(x, y)].walkable=true;
					}
				}
			}
		}

		//every so often...
		if (timer>.01f) {
			timer=0;

			if (running&&path.size()==0&&openSet.size()>0) {
				//curr = node in OPEN with lowest f
				node* currNode=openSet.at(0);
				for (int i=1; i<openSet.size(); i++) {
					node* check=openSet.at(i);
					bool fLess=check->fCost<currNode->fCost;
					bool fEqual=check->fCost==currNode->fCost;
					bool hLess=check->hCost<currNode->hCost;
					if (fLess||fEqual&&hLess) {
						currNode=check;
					}
				}

				//remove curr from OPEN
				openSet.erase(find(openSet.begin(), openSet.end(), currNode));

				//add curr to CLOSED
				closedSet.push_back(currNode);

				if (currNode==endNode) {
					running=false;

					//path found
					openSet.clear();
					closedSet.clear();

					//trace back using parents
					node* c=endNode;
					while (c!=startNode) {
						path.push_back(c);
						c=c->parent;
					}

					return;
				}

				//foreach neighbor of the curr node
				for (auto n:currNode->neighbors) {
					//if neighbor is NOT walkable OR neighbor is in CLOSED
					auto nInClosed=find(closedSet.begin(), closedSet.end(), n);
					if (!n->walkable||nInClosed!=closedSet.end()) {
						//skip to next neighbor
						continue;
					}

					//if new path to neighbor is shorter OR neighbor is NOT in OPEN
					float newGCost=currNode->gCost+distance(*currNode, *n);
					auto nInOpen=find(openSet.begin(), openSet.end(), n);
					if (newGCost<n->gCost||nInOpen==openSet.end()) {
						//set f of neighbor
						n->gCost=newGCost;
						n->hCost=distance(*n, *endNode);
						n->fCost=n->gCost+n->hCost;

						//set parent of neighbor to curr
						n->parent=currNode;

						//if neighbor is NOT in OPEN...
						if (nInOpen==openSet.end()) {
							//...add neighbor to OPEN
							openSet.push_back(n);
						}
					}
				}
			}
		}
		timer+=dt;

		//update title
		std::string fpsStr=std::to_string((int)framesPerSecond)+"fps";
		std::string runStr=running?"running":"not running";
		setTitle("A* Simulation ["+runStr+"] @ "+fpsStr);
	}

	void draw(Raster& rst) override {
		for (int x=0; x<width; x++) {
			for (int y=0; y<height; y++) {
				node& n=nodeGrid[ix(x, y)];
				rst.setChar(n.walkable?' ':0x2588);
				rst.putPixel(x, y);
			}
		}

		//draw all in open
		rst.setChar(0x2588);
		rst.setColor(Raster::GREEN);
		for (const auto n:openSet) {
			rst.putPixel(n->pos.x, n->pos.y);
		}

		//draw all in closed
		rst.setColor(Raster::RED);
		for (const auto n:closedSet) {
			rst.putPixel(n->pos.x, n->pos.y);
		}

		//draw path
		rst.setColor(Raster::BLUE);
		for (const auto n:path) {
			rst.putPixel(n->pos.x, n->pos.y);
		}

		//show start node
		rst.setChar('S');
		rst.setColor(Raster::WHITE);
		rst.putPixel(startNode->pos.x, startNode->pos.y);

		//show end node
		rst.setChar('E');
		rst.putPixel(endNode->pos.x, endNode->pos.y);
	}
};

int main() {
	//init custom graphics engine
	Demo d;
	d.startWindowed(8, 60, 60);

	return 0;
}