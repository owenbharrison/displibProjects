#include <time.h>

#include "Engine.h"
#include "maths/Maths.h"
using namespace displib;

class Demo : public Engine {
	public:
	bool* cellGrid;
	float timer=0;
	float timePerStep=0.05f;

	void setup() override {
		cellGrid=new bool[width*height];
		for (int x=0; x<width; x++) {
			for (int y=0; y<height; y++) {
				bool alive=Maths::random()>0.5;
				cellGrid[ix(x, y)]=alive;
			}
		}
	}

	void update(float dt) override {
		//do timesteps
		if (timer>timePerStep) {
			//reset
			timer=0;

			//timestep
			bool* nextCellGrid=new bool[width*height];
			for (int x=0; x<width; x++) {
				for (int y=0; y<height; y++) {
					int numAlive=0;
					for (int i=-1; i<=1; i++) {
						for (int j=-1; j<=1; j++) {
							//dont check self
							if (!(i==0&&j==0)) {
								//wrap x
								int rx=x+i;
								if (rx==-1) rx=width-1;
								if (rx==width) rx=0;
								//wrap y
								int ry=y+j;
								if (ry==-1) ry=height-1;
								if (ry==height) ry=0;
								//check alive
								if (cellGrid[ix(rx, ry)]) numAlive++;
							}
						}
					}

					//choose next timestep based on neighbors
					bool currAlive=cellGrid[ix(x, y)];
					bool nextAlive=false;
					//Any live cell with:
					if (currAlive) {
						//fewer than two live neighbours dies, as if by underpopulation.
						if (numAlive<2) nextAlive=false;
						//two or three live neighbours lives on to the next generation.
						else if (numAlive==2||numAlive==3) nextAlive=true;
						//more than three live neighbours dies, as if by overpopulation.
						else if (numAlive>3) nextAlive=false;
					}
					//Any dead cell with exactly three live neighbours becomes a live cell, as if by reproduction.
					else if (numAlive==3) nextAlive=true;

					//set next array
					nextCellGrid[ix(x, y)]=nextAlive;
				}
			}

			//copy next to current
			for (int i=0; i<width*height; i++) {
				cellGrid[i]=nextCellGrid[i];
			}

			//clear temp storage
			delete[] nextCellGrid;
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
				rst.setChar(cellGrid[ix(x, y)]?'#':' ');
				rst.putPixel(x, y);
			}
		}
	}
};

int main() {
	srand(time(NULL));

	//init custom graphics engine
	Demo d=Demo();
	d.start(12, 12, true);

	return 0;
}