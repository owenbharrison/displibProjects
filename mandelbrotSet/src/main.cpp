#include "Engine.h"
#include "maths/Maths.h"
using namespace displib;

class Demo : public Engine {
	public:
	int maxIter=100;
	short* colorArr=new short[8]{
		Raster::DARK_BLUE,
		Raster::BLUE,
		Raster::DARK_CYAN,
		Raster::CYAN,
		Raster::GREEN,
		Raster::DARK_YELLOW,
		Raster::RED,
		Raster::DARK_RED
	};
	float minx, miny, maxx, maxy;
	V2D scrMin, scrMax;
	bool set=false, wasSet=false;
	bool validSet;

	void setup() override {
		minx=-2, miny=-1.5f, maxx=1, maxy=1.5f;
		resetBnd();
	}

	void resetBnd() {
		scrMin=V2D(-1, -1);
		scrMax=V2D(-2, -2);
		validSet=false;
	}

	void update(float dt) override {
		//user input for setting
		V2D mp(mouseX, mouseY);
		if (getKey('1')) scrMin=mp;
		if (getKey('2')) scrMax=mp;
		validSet=(scrMax.x>scrMin.x&&scrMax.y>scrMin.y);

		//for bound setting
		set=getKey(VK_RETURN);
		if (set&&!wasSet) {
			if (validSet) {
				float newminx=Maths::map(scrMin.x, 0, width, minx, maxx);
				float newmaxx=Maths::map(scrMax.x, 0, width, minx, maxx);
				minx=newminx;
				maxx=newmaxx;

				float newminy=Maths::map(scrMin.y, 0, height, miny, maxy);
				float newmaxy=Maths::map(scrMax.y, 0, height, miny, maxy);
				miny=newminy;
				maxy=newmaxy;
				
				resetBnd();
			}
		}
		wasSet=set;

		//option to reset.
		if (getKey('R')) setup();

		//update title
		setTitle("Mandelbrot Set Explorer @ "+std::to_string((int)framesPerSecond)+"fps");
	}

	void draw(Raster& rst) override {
		//show background
		rst.setChar(' ');
		rst.fillRect(0, 0, width, height);

		//show set
		for (int x=0; x<width; x++) {
			for (int y=0; y<height; y++) {
				float a=Maths::map(x, 0, width, minx, maxx);
				float b=Maths::map(y, 0, height, miny, maxy);
				float ca=a;
				float cb=b;

				//the iterations
				float z=0;
				int n;
				for(n=0;n<maxIter;n++){
					float asq=a*a-b*b;
					float bb=2*a*b;
					a=asq+ca;
					b=bb+cb;
					if (a*a+b*b>16) {
						break;
					}
				}
				
				//int to 0-1 bright val
				float pct=n/(float)maxIter;
				//some lighting fix
				pct=sqrt(pct);
				int csi=Maths::clamp(pct*8, 0, 7);
				//if it reached, make it dark
				rst.setChar(n==maxIter?' ':0x2588);
				//coloring
				rst.setColor(colorArr[csi]);
				rst.putPixel(x, y);
			}
		}

		//show box
		rst.setColor(Raster::WHITE);
		rst.setChar(0x2588);
		if (validSet) {
			//show "new" bounds
			V2D s=scrMax-scrMin;
			rst.drawRect(scrMin, s.x, s.y);
		}

		//show pts
		rst.setColor(validSet?Raster::GREEN:Raster::DARK_RED);
		rst.fillCircle(scrMin, 2);
		rst.fillCircle(scrMax, 2);
	}
};

int main() {
	//init custom graphics engine
	Demo d;
	d.startWindowed(3, 240, 240);

	return 0;
}