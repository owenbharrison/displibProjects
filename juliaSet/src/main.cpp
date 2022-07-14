#include "Engine.h"
#include "maths/Maths.h"
using namespace displib;

class Demo : public Engine {
	public:
	int iter, maxIter=100;
	bool animate=true;

	float animTimer=0;
	bool toggleAnim=false, wasToggleAnim=false;

	short* colorArr=new short[10]{
		Raster::DARK_RED,
		Raster::RED,
		Raster::DARK_YELLOW,
		Raster::GREEN,
		Raster::CYAN,
		Raster::DARK_CYAN,
		Raster::BLUE,
		Raster::DARK_BLUE,
		Raster::MAGENTA,
		Raster::DARK_MAGENTA
	};
	float minx, miny, maxx, maxy;
	V2D scrMin, scrMax;
	bool set=false, wasSet=false;
	bool validSet;

	void setup() override {
		minx=-2, miny=-1.75f, maxx=2, maxy=1.75f;
		resetBnd();
		iter=maxIter;
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

		toggleAnim=getKey(' ');
		if (toggleAnim&&!wasToggleAnim) animate=!animate;
		wasToggleAnim=toggleAnim;

		//option to reset.
		if (getKey('R')) setup();

		if (animate) {
			iter=Maths::map(sinf(animTimer), -1, 1, 0, maxIter);
			animTimer+=dt;
		}

		//update title
		std::string animText=animate?"Animated ":"";
		setTitle(animText+"Julia Set Explorer @ "+std::to_string((int)framesPerSecond)+"fps");
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

				//the iterations
				float z=0;
				int n;
				for (n=0; n<iter; n++) {
					float aa=a*a-b*b;
					float bb=2*a*b;
					a=aa-0.8f;
					b=bb+0.156f;
					if (a*a+b*b>16) break;
				}

				//int to 0-1 bright val
				float pct=n/(float)iter;
				//some lighting fix
				pct=sqrt(pct);
				int csi=Maths::clamp(pct*10, 0, 9);
				//if it reached, make it dark
				rst.setChar(n==iter?' ':0x2588);
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
	d.startWindowed(4, 240, 180);

	return 0;
}