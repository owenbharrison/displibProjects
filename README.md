# displibProjects
These are some of my Visual Studio 2022 "simulations" that I have been working on as of late. They all use a graphics engine, called displib.
This C++ version of displib is not to be confused with my Java Game Engine, this is somewhat of a port of that in C++.
This is similar in that it has simple drawing and display mechanics, and the inspirations are the Processing Foundation and javidx9's olc::ConsoleGameEngine.

This repository has the code of the "simulations" that I have been working on, and a release tag to run the executables in Windows.
These projects incorporate some of my appreciation of Physics and Computer Science.

WARNING: These simulations may potentially trigger seizures for people with photosensitive epilepsy.

## Win32 Usage:
1. Download as ZIP & extract.
2. Using Microsoft Visual Studio with C++, 
3. Open projects.sln.
4. Set project Build Configuration to Release and Win32(x86).
5. Set project startups to "Current Selection".
6. Set "displib" properties to export as Static Library(.lib), and all else to export as Windows Executable(.exe).
7. Select a project and run it!

## Example
The following is a simple example of a program that could be created using the engine.
It draws a number of polygons to the screen, with a random background.

```cpp
#include "Engine.h"
#include "maths/Maths.h"
using namespace displib;

class Demo : public Engine {
	public:
	void drawPolygon(Raster& rst, float2 pos, int sides, float rad) {
		float2* vtxs=new float2[sides];
		for (int i=0; i<sides; i++) {
			float angle=Maths::map(i, 0, sides, 0, Maths::PI*2);
			float x=cosf(angle)*rad+pos.x;
			float y=sinf(angle)*rad+pos.y;
			vtxs[i]=float2(x, y);
		}

		for (int i=0; i<sides; i++) {
			float2 a=vtxs[i];
			float2 b=vtxs[(i+1)%sides];
			rst.drawLine(a, b);
		}
		delete[] vtxs;
	}

	void setup() override {

	}

	void update(float dt) override {
		setTitle("Example @ "+std::to_string((int)framesPerSecond)+"fps");
	}

	void draw(Raster& rst) override {
		if (updateCount%8==0) {
			for (int x=0; x<width; x++) {
				for (int y=0; y<height; y++) {
					int letter=Maths::random(26);
					rst.setChar('a'+letter);
					rst.putPixel(x, y);
				}
			}
		}

		rst.setChar(0x2588);
		int num=32;
		for (int i=0; i<num; i++) {
			float x=Maths::map(i, 0, num, width/12, width*11/12);
			float y=height/2+cosf(totalDeltaTime+i/4.5f)*32;
			float rad=abs(Maths::map(i, 0, num, 12, -12));
			int sides=i/4+3;

			rst.setColor(i%16);
			drawPolygon(rst, float2(x, y) , sides, rad);
		}
	}
};

int main() {
	Demo d;
	d.startWindowed(4, 240, 135);

	return 0;
}
```
