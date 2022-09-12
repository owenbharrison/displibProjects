#include "Engine.h"
#include "maths/Maths.h"
using namespace displib;

class Demo : public Engine {
	public:
	int maxIter=250;
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
	long double minx, miny, maxx, maxy;
	int sminx, sminy, smaxx, smaxy;

	bool set=false, wasSet=false;
	bool validSet;

	long double map(long double x, long double a, long double b, long double c, long double d) {
		return (x-a)*(d-c)/(b-a)+c;
	}

	void setup() override {
		minx=-2, miny=-1.5f, maxx=1, maxy=1.5f;
		resetBnd();
	}

	void resetBnd() {
		sminx=-1, sminy=-1;
		smaxx=-2, smaxy-2;
		validSet=false;
	}

	void update(float dt) override {
		//user input for setting
		if (getKey('1')) sminx=mouseX, sminy=mouseY;
		if (getKey('2')) smaxx=mouseX, smaxy=mouseY;
		validSet=(smaxx>sminx&&smaxy>sminy);

		//for bound setting
		set=getKey(' ');
		if (set&&!wasSet) {
			if (validSet) {
				long double newminx=map(sminx, 0, width, minx, maxx);
				long double newmaxx=map(smaxx, 0, width, minx, maxx);
				minx=newminx;
				maxx=newmaxx;

				long double newminy=map(sminy, 0, height, miny, maxy);
				long double newmaxy=map(smaxy, 0, height, miny, maxy);
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

		//get set values
		int minVal=maxIter+1, maxVal=-1;
		int* values=new int[width*height]; 
		for (int x=0; x<width; x++) {
			for (int y=0; y<height; y++) {
				long double a=map(x, 0, width, minx, maxx);
				long double b=map(y, 0, height, miny, maxy);
				long double ca=a;
				long double cb=b;

				//the iterations
				long double z=0;
				int n;
				for (n=0; n<maxIter; n++) {
					long double asq=a*a-b*b;
					long double bb=2*a*b;
					a=asq+ca;
					b=bb+cb;
					if (a*a+b*b>16) break;
				}
				minVal=min(n, minVal);
				maxVal=max(n, maxVal);
				values[x+y*width]=n;
			}
		}

		//fix set values and show them
		for (int x=0; x<width; x++) {
			for (int y=0; y<height; y++) {
				int n=values[x+y*width];
				//int to 0-1 bright val
				long double pct=map(n, minVal, maxVal, 0, 1);
				//some lighting fix
				pct=sqrtl(pct);
				//clamp
				int csi=min(pct*8, 7);
				//if it reached, make it dark
				rst.setChar(n==maxIter?' ':0x2588);
				//coloring
				rst.setColor(colorArr[csi]);
				rst.putPixel(x, y);
			}
		}
		delete[] values;

		//show box
		rst.setColor(Raster::WHITE);
		rst.setChar(0x2588);
		if (validSet) {
			//show "new" bounds
			int w=smaxx-sminx;
			int h=smaxy-sminy;
			rst.drawRect(sminx, sminy, w, h);
		}

		//show pts
		rst.setColor(validSet?Raster::DARK_GREY:Raster::DARK_RED);
		rst.fillCircle(sminx, sminy, 1);
		rst.fillCircle(smaxx, smaxy, 1);
	}
};

int main() {
	//init custom graphics engine
	Demo d;
	d.startWindowed(4, 190, 190);

	return 0;
}