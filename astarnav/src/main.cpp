#include <list>
#include <algorithm>

#include "Engine.h"
using namespace displib;

#define NODE_SIZE 2.49f
#define ROAD_SIZE 2

struct node {
	float2 pos;
	float gCost=0, hCost=0, fCost=0;
	node* parent=nullptr;
	std::list<node*> neighbors;

	node() {}

	node(float2 pos_) : pos(pos_) {}
};

class Demo : public Engine {
	public:
	std::list<node> nodes;
	node* startNode=nullptr, * endNode=nullptr;

	bool running=false;
	bool pausePlay=false, wasPausePlay=false;
	bool addNode=false, wasAddNode=false;
	bool connect=false, wasConnect=false;
	node* connectStart=nullptr;
	bool hold=false, wasHold=false;
	node* heldNode=nullptr;

	void update(float dt) override {
		float2 mousePos(mouseX, mouseY);

		//for modifying
		connect=getKey('C');
		bool connectChanged=connect!=wasConnect;
		if (connectChanged) {
			if (connect) {//on key down
				//clear connection.
				connectStart=nullptr;
				for (node& n:nodes) {
					float dist=length(mousePos-n.pos);
					if (dist<NODE_SIZE) {
						connectStart=&n;
					}
				}
			}
			else {//on key up
				node* connectEnd=nullptr;
				for (node& n:nodes) {
					float dist=length(mousePos-n.pos);
					if (dist<NODE_SIZE) {
						connectEnd=&n;
					}
				}
				//dont connect to nothing from nothing or self.
				if (connectStart!=nullptr&&connectEnd!=nullptr&&connectEnd!=connectStart) {
					connectStart->neighbors.push_back(connectEnd);
				}
				//clear connection.
				connectStart=nullptr;
			}
		}
		wasConnect=connect;

		addNode=getKey('A');
		bool toAdd=addNode&&!wasAddNode;//on key down
		wasAddNode=addNode;
		bool toDelete=getKey('D');
		bool toSetStart=getKey('S');
		bool toSetEnd=getKey('E');
		if (toAdd||toDelete||toSetStart||toSetEnd||connectChanged) {
			//stop if modify
			running=false;

			//add at mouse if far from others
			if (toAdd) {
				startNode=nullptr;
				endNode=nullptr;

				bool farAway=true;
				for (node& n:nodes) {
					float dist=length(mousePos-n.pos);
					if (dist<NODE_SIZE) {
						farAway=false;
						break;
					}
				}
				if (farAway) {
					nodes.push_back(node(mousePos));
				}
			}
			//removes at mouse
			if (toDelete) {
				std::list<node>::iterator it=nodes.begin();
				while (it!=nodes.end()) {
					node& n=*it;
					float dist=length(mousePos-n.pos);
					if (dist<NODE_SIZE) {
						for (std::list<node>::iterator oit=nodes.begin(); oit!=nodes.end();oit++) {
							node& o=*oit;
							o.neighbors.remove(&n);
						}
						//remove node
						it=nodes.erase(it);
					}
					else it++;
				}
			}
			//sets random in range.
			if (toSetStart) {
				for (node& n:nodes) {
					float dist=length(mousePos-n.pos);
					if (dist<NODE_SIZE) {
						startNode=&n;
					}
				}
			}
			//sets random in range.
			if (toSetEnd) {
				for (node& n:nodes) {
					float dist=length(mousePos-n.pos);
					if (dist<NODE_SIZE) {
						endNode=&n;
					}
				}
			}
		}

		//for pause/play
		pausePlay=getKey(VK_SPACE);
		if (pausePlay&&!wasPausePlay) {
			connectStart=nullptr;
			heldNode=nullptr;
			if (running) running=false;
			else {
				bool startValid=startNode!=nullptr;
				bool endValid=endNode!=nullptr;
				if (!startValid) showPopupBox("Error", "Choose start!");
				if (!endValid) showPopupBox("Error", "Choose end!");
				if (startValid&&endValid) running=true;
			}
		}
		wasPausePlay=pausePlay;

		//mouse interact
		hold=getKey(VK_MBUTTON);
		if (hold!=wasHold) {//on key change
			//reset
			heldNode=nullptr;
			if (hold) {//on key down
				//find point to "hold"
				for (node& p:nodes) {
					float dist=length(mousePos-p.pos);
					if (dist<NODE_SIZE) {
						heldNode=&p;
					}
				}
			}
		}
		wasHold=hold;

		if (heldNode!=nullptr) {
			heldNode->pos=mousePos;
		}

		//update title
		std::string fpsStr=std::to_string((int)framesPerSecond)+"fps";
		std::string runStr=running?"running":"not running";
		setTitle("A* Navigation ["+runStr+"] @ "+fpsStr);
	}

	void draw(Raster& rst) override {
		float2 mousePos(mouseX, mouseY);

		//show background
		rst.setChar(' ');
		rst.fillRect(0, 0, width, height);

		//show edge highlight
		rst.setChar(0x2588);
		if (!running) {
			rst.setColor(Raster::DARK_RED);
			rst.drawRect(0, 0, width, height);
		}

		//show connections...
		rst.setColor(Raster::DARK_GREY);
		for (node& n:nodes) {
			for (node* o:n.neighbors) {
				float2 tang=normalize(o->pos-n.pos);
				float2 norm(-tang.y, tang.x);
				//...on one side?
				rst.drawLine(n.pos+norm*ROAD_SIZE, o->pos+norm*ROAD_SIZE);
			}
		}

		//show nodes
		rst.setColor(Raster::WHITE);
		for (node& n:nodes) rst.fillCircle(n.pos, NODE_SIZE);

		//show start, end
		rst.setColor(Raster::MAGENTA);
		if (startNode!=nullptr) {
			rst.setChar('S');
			rst.fillRect(startNode->pos-float2(1), 3, 3);
		}
		if (endNode!=nullptr) {
			rst.setChar('E');
			rst.fillRect(endNode->pos-float2(1), 3, 3);
		}

		//show hover point
		node* possible=nullptr;
		for (node& p:nodes) {
			float dist=length(mousePos-p.pos);
			if (dist<NODE_SIZE) {
				possible=&p;
			}
		}
		if (possible!=nullptr) {
			rst.setChar(0x2588);
			rst.setColor(Raster::GREEN);
			rst.drawCircle(possible->pos, NODE_SIZE);
		}

		//show held point
		if (heldNode!=nullptr) {
			rst.setColor(Raster::CYAN);
			rst.drawCircle(heldNode->pos, NODE_SIZE);
		}

		//calculate and show path
		if (running) {
			std::list<node*> openSet{startNode}, closedSet;
			while (openSet.size()>0) {
				//curr = node in OPEN with lowest f
				std::list<node*>::iterator it=openSet.begin();
				node* currNode=*it;
				for (it++; it!=openSet.end();it++) {
					node* check=*it;
					bool fLess=check->fCost<currNode->fCost;
					bool fEqual=check->fCost==currNode->fCost;
					bool hLess=check->hCost<currNode->hCost;
					if (fLess||fEqual&&hLess) {
						currNode=check;
					}
				}

				//remove curr from OPEN
				openSet.remove(currNode);

				//add curr to CLOSED
				closedSet.push_back(currNode);

				if (currNode==endNode) {
					//running=false;

					//path found
					openSet.clear();
					closedSet.clear();

					//trace back using parents
					node* c=endNode;
					rst.setChar(0x2588);
					rst.setColor(Raster::BLUE);
					while (c!=startNode) {
						float2 tang=normalize(c->pos-c->parent->pos);
						float2 norm(-tang.y, tang.x);
						rst.drawLine(c->pos+norm*ROAD_SIZE, c->parent->pos+norm*ROAD_SIZE);

						c=c->parent;
					}

					return;
				}

				//foreach neighbor of the curr node
				for (node* n:currNode->neighbors) {
					//if neighbor is in CLOSED
					auto nInClosed=find(closedSet.begin(), closedSet.end(), n);
					if (nInClosed!=closedSet.end()) {
						//skip to next neighbor
						continue;
					}

					//if new path to neighbor is shorter OR neighbor is NOT in OPEN
					float currNDist=length(currNode->pos-n->pos);
					float newGCost=currNode->gCost+currNDist;
					auto nInOpen=find(openSet.begin(), openSet.end(), n);
					if (newGCost<n->gCost||nInOpen==openSet.end()) {
						//set f of neighbor
						n->gCost=newGCost;
						n->hCost=length(endNode->pos-n->pos);
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
	}
};

int main() {
	srand(time(NULL));

	//init custom graphics engine
	Demo d;
	d.startWindowed(5, 240, 135);

	return 0;
}