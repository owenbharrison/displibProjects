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
		auto pgset=[&](int x, int y, ParticleType pt) { particleGrid[ix(x, y)]=pt; };
		auto pgis=[&](int x, int y, ParticleType pt) { return particleGrid[ix(x, y)]==pt; };
		//do timesteps
		if (timer>timePerStep) {
			//key checks:

			//reset "sim"
			if (getKey('R')) setup();

			//add sand, water, large barriers, remove a lot
			if (mouseX>0&&mouseY>0&&mouseX<width-1&&mouseY<height-1) {
				if (getKey('S')) particleGrid[ix(mouseX, mouseY)]=Sand;
				if (getKey('W')) particleGrid[ix(mouseX, mouseY)]=Water;

				for (int i=-1; i<=1; i++) {
					for (int j=-1; j<=1; j++) {
						int rx=mouseX+i;
						int ry=mouseY+j;
						if (rx>0&&ry>0&&rx<width-1&&ry<height-1) {
							if (getKey('B')) particleGrid[ix(rx, ry)]=Barrier;
							if (getKey('A')) particleGrid[ix(rx, ry)]=Air;
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
							if (pgis(x, y+1, Air)) { pgset(x, y+1, Sand); pgset(x, y, Air); }//down
							else if (pgis(x-1, y+1, Air)) { pgset(x-1, y+1, Sand); pgset(x, y, Air); }//down left
							else if (pgis(x+1, y+1, Air)) { pgset(x+1, y+1, Sand); pgset(x, y, Air); }//down right
						}
						//water behavior:
						else if (curr==Water) {//if free move there:
							if (pgis(x, y+1, Air)) { pgset(x, y+1, Water); pgset(x, y, Air); }//down
							else if (pgis(x-1, y+1, Air)) { pgset(x-1, y+1, Water); pgset(x, y, Air); }//down left
							else if (pgis(x+1, y+1, Air)) { pgset(x+1, y+1, Water); pgset(x, y, Air); }//down right
							else if (pgis(x-1, y, Air)) { pgset(x-1, y, Water); pgset(x, y, Air); }//left
							else if (pgis(x+1, y, Air)) { pgset(x+1, y, Water); pgset(x, y, Air); }//right
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
		for (int x=0; x<width; x++) {
			for (int y=0; y<height; y++) {
				ParticleType& curr=particleGrid[ix(x, y)];
				if (curr==Barrier) {
					rst.setChar('@');
					rst.setColor(Raster::DARK_GREY);
				}
				else if (curr==Air) rst.setChar(' ');
				else if (curr==Sand) {
					rst.setChar('.');
					rst.setColor(Raster::DARK_YELLOW);
				}
				else if (curr==Water) {
					rst.setChar('~');
					rst.setColor(Raster::CYAN);
				}
				rst.putPixel(x, y);
			}
		}

		//show fps
		rst.setColor(Raster::WHITE);
		rst.drawString(0, 0, "FPS: "+std::to_string((int)fps));
	}
};

int main() {
	//init custom graphics engine
	Demo d=Demo();
	d.start(12, 12, true);

	return 0;
}