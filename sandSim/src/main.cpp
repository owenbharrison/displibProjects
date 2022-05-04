#include <time.h>

#include "Engine.h"
#include "maths/Maths.h"
using namespace displib;

enum ParticleType {
	Barrier,
	Air,
	Sand,
	Water
};

class Demo : public Engine {
	public:
	ParticleType* particleGrid;
	float timer=0;
	float timePerStep=0.01f;

	void setup() override {
		setTitle("Sand & Water Simulation");

		particleGrid=new ParticleType[width*height];

		//initialize the grid
		for (int x=0; x<width; x++) {
			for (int y=0; y<height; y++) {
				//make edges barriers
				if (x==0||y==0||x==width-1||y==height-1) {
					particleGrid[ix(x, y)]=Barrier;
				}
				//all else "empty"
				else particleGrid[ix(x, y)]=Air;
			}
		}
	}

	void update(float dt) override {
		//ez funcs
		auto pgSet=[&](int x, int y, ParticleType pt) { particleGrid[ix(x, y)]=pt; };
		auto pgGet=[&](int x, int y, ParticleType pt) { return particleGrid[ix(x, y)]==pt; };
		//do timesteps
		if (timer>timePerStep) {
			//reset "sim"
			if (getKey('R')) setup();

			//add sand, water, large barriers, remove a lot
			if (mouseX>0&&mouseY>0&&mouseX<width-1&&mouseY<height-1) {
				if (getKey('S')) pgSet(mouseX, mouseY, Sand);
				if (getKey('W')) pgSet(mouseX, mouseY, Water);

				for (int i=-1; i<=1; i++) {
					for (int j=-1; j<=1; j++) {
						int rx=mouseX+i;
						int ry=mouseY+j;
						if (rx>0&&ry>0&&rx<width-1&&ry<height-1) {
							if (getKey('B')) pgSet(rx, ry, Barrier);
							if (getKey('A')) pgSet(rx, ry, Air);
						}
					}
				}
			}

			//timestep
			ParticleType* gridCopy=new ParticleType[width*height];
			memcpy(gridCopy, particleGrid, sizeof(ParticleType)*width*height);//copy "old" to new

			//sand behavior:
			for (int x=0; x<width; x++) {
				for (int y=0; y<height; y++) {
					ParticleType& curr=gridCopy[ix(x, y)];
					if (curr!=Barrier) {//dont "update" barriers
						//sand behavior:
						if (curr==Sand) {//if free move there:
							int l_r=Maths::random()>0.5?-1:1;//some randomness
							if (pgGet(x, y+1, Air)) { pgSet(x, y+1, Sand); pgSet(x, y, Air); }//down
							else if (pgGet(x-l_r, y+1, Air)) { pgSet(x-l_r, y+1, Sand); pgSet(x, y, Air); }//down left or right?
							else if (pgGet(x+l_r, y+1, Air)) { pgSet(x+l_r, y+1, Sand); pgSet(x, y, Air); }//down the other side
						} 
						//water behavior:
						else if (curr==Water) {//if free move there:
							int l_r=Maths::random()>0.5?-1:1;//some randomness
							if (pgGet(x, y+1, Air)) { pgSet(x, y+1, Water); pgSet(x, y, Air); }//down
							else if (pgGet(x-l_r, y+1, Air)) { pgSet(x-l_r, y+1, Water); pgSet(x, y, Air); }//down left or right?
							else if (pgGet(x+l_r, y+1, Air)) { pgSet(x+l_r, y+1, Water); pgSet(x, y, Air); }//down the other side
							else if (pgGet(x-l_r, y, Air)) { pgSet(x-l_r, y, Water); pgSet(x, y, Air); }//left or right?
							else if (pgGet(x+l_r, y, Air)) { pgSet(x+l_r, y, Water); pgSet(x, y, Air); }//the other side
						}
					}
				}
			}

			//clear copy
			delete[] gridCopy;

			//reset
			timer=0;
		}

		//update timer
		timer+=dt;
	}

	int ix(int i, int j) {
		return i+j*width;
	}

	void draw(Raster& rst) override {
		//background
		rst.setChar(' ');
		rst.fillRect(0, 0, width, height);

		//draw cellGrid
		int numBarrier=0;
		int numSand=0;
		int numWater=0;
		for (int x=0; x<width; x++) {
			for (int y=0; y<height; y++) {
				ParticleType& curr=particleGrid[ix(x, y)];
				if (curr==Barrier) {
					numBarrier++;
					rst.setChar('@');
					rst.setColor(Raster::DARK_GREY);
				}
				else if (curr==Air) rst.setChar(' ');
				else if (curr==Sand) {
					numSand++;
					rst.setChar('.');
					rst.setColor(Raster::DARK_YELLOW);
				}
				else if (curr==Water) {
					numWater++;
					rst.setChar('~');
					rst.setColor(Raster::CYAN);
				}
				rst.putPixel(x, y);
			}
		}

		//show fps
		rst.setColor(Raster::WHITE);
		rst.drawString(0, 0, "Barrier: "+std::to_string(numBarrier));
		rst.drawString(0, 1, "Sand: "+std::to_string(numSand));
		rst.drawString(0, 2, "Water: "+std::to_string(numWater));
	}
};

int main() {
	srand(time(NULL));

	//init custom graphics engine
	Demo d=Demo();
	d.startWindowed(6, 160, 90);

	return 0;
}