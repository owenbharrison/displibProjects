#include <vector>

#include "Engine.h"
#include "maths/Maths.h"
using namespace displib;

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

static std::string FILENAME;

#define thresh 35

//from coding train : steering behaviors coding challenge
struct particle {
	float2 pos, vel, acc, target;
	float maxSpeed=400, maxForce=30;
	short r, g, b;

	particle(float2 pos_, float2 target_, int r_, int g_, int b_) {
		pos=pos_;
		target=target_;
		r=r_;
		g=g_;
		b=b_;
	}

	void update(float dt) {
		//euler explicit
		vel+=acc*dt;
		pos+=vel*dt;
		acc*=0;
	}

	void applyForce(float2 f) { acc+=f; }

	float2 arrive(float2 tg) {
		float2 des=tg-pos;
		float d=length(des);
		float speed=maxSpeed;
		if (d<100) speed=Maths::map(d, 0, 100, 0, maxSpeed);
		//setmag to speed
		des*=speed/d;
		float2 steer=des-vel;
		//limit
		float s=length(steer);
		if (s>maxForce) steer*=maxForce/s;

		//dont move if at target
		return d==0?float2():steer;
	}

	float2 flee(float2 tg) {
		float2 des=tg-pos;
		float d=length(des);
		//opposite
		des*=-maxSpeed/d;
		float2 steer=des-vel;
		float s=length(steer);
		if (s>maxForce) steer*=maxForce/s;
		return steer;
	}
};

class Demo : public Engine {
	public:
	std::vector<particle> particles;
	int imgWidth, imgHeight, imgChannels;
	const char* asciiArr=" .,~=#&@";
	short* colors=new short[8]{
		Raster::BLACK,
		Raster::BLUE,
		Raster::GREEN,
		Raster::CYAN,
		Raster::RED,
		Raster::DARK_MAGENTA,
		Raster::DARK_YELLOW,
		Raster::WHITE
	};

	char rgbToChar(int r, int g, int b) {
		float pct=(r+g+b)/765.0f;
		int asi=Maths::clamp(pct*8, 0, 7);
		return asciiArr[asi];
	}

	short rgbToCol(int r, int g, int b) {
		int csi=(r>127)*4+(g>127)*2+(b>127);
		return colors[csi];
	}

	void setup() override {
		int imgWidth, imgHeight, imgChannels;
		//load image
		unsigned char* origData=stbi_load(FILENAME.c_str(), &imgWidth, &imgHeight, &imgChannels, 0);
		unsigned char* newData=new unsigned char[width*height*3];
		//resize image
		auto ix=[&](int i, int j) { return (i+j*width)*3; };
		for (int x=0; x<width; x++) {
			for (int y=0; y<height; y++) {
				//map x,y to pixel in image i,j
				int i=Maths::map(x, 0, width, 0, imgWidth);
				int j=Maths::map(y, 0, height, 0, imgHeight);
				int origZ=(i+j*imgWidth)*imgChannels;
				int newZ=ix(x, y);
				newData[newZ]=origData[origZ];
				newData[newZ+1]=origData[origZ+1];
				newData[newZ+2]=origData[origZ+2];
			}
		}
		delete[] origData;

		//edge detection
		//similar takes two indexes of the imgData
		auto similar=[&](int a, int b) {
			int ra=newData[a], ga=newData[a+1], ba=newData[a+2];
			int rb=newData[b], gb=newData[b+1], bb=newData[b+2];
			int rs=ra-rb, gs=ga-gb, bs=ba-bb;
			return rs*rs+gs*gs+bs*bs<thresh* thresh;
		};
		float bfr=3.476f;
		for (int x=1; x<width-1; x++) {
			for (int y=1; y<height-1; y++) {
				int z=ix(x, y);
				bool left=similar(z, ix(x-1, y));
				bool right=similar(z, ix(x+1, y));
				bool top=similar(z, ix(x, y-1));
				bool bottom=similar(z, ix(x, y+1));
				if (!left||!right||!top||!bottom) {
					int r=newData[z];
					int g=newData[z+1];
					int b=newData[z+2];
					float rx=Maths::random(bfr, width-bfr);
					float ry=Maths::random(bfr, height-bfr);
					particles.push_back(particle(float2(rx, ry), float2(x, y), r, g, b));
				}
			}
		}
		delete[] newData;
	}

	void update(float dt) override {
		float2 mp(mouseX, mouseY);
		//for all
		for (particle& p:particles) {
			//go towards target
			p.applyForce(p.arrive(p.target)*20);
			//flee mouse
			if (length(mp-p.pos)<7) p.applyForce(p.flee(mp)*50);
			//update
			p.update(dt);
		}

		//update title
		setTitle("Steering Behaviors! showing image["+FILENAME+"] @ "+std::to_string((int)framesPerSecond)+"fps");
	}

	void draw(Raster& rst) override {
		//background
		rst.setChar(' ');
		rst.fillRect(0, 0, width, height);

		//draw all particles
		for (particle& p:particles) {
			//r,g,b into ascii bright ramp
			float bright=(p.r+p.g+p.b)/765.0f;
			int asi=Maths::clamp(bright*8, 0, 7);
			rst.setChar(asciiArr[asi]);
			//r,g,b to console color
			int csi=(p.r>127)*4+(p.g>127)*2+(p.b>127);
			rst.setColor(colors[csi]);
			rst.putPixel(p.pos);
		}
	}
};

int main() {
	Demo d;

	//init file chooser
	OPENFILENAME ofn;
	char szFile[1024];
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize=sizeof(ofn);
	ofn.hwndOwner=d.getWindowHandle();
	ofn.lpstrFile=(LPWSTR)szFile;
	ofn.lpstrFile[0]='\0';
	ofn.nMaxFile=sizeof(szFile);
	ofn.lpstrFilter=L"JPG Images(.jpg)\0*.jpg\0";
	ofn.nFilterIndex=1;
	ofn.lpstrFileTitle=NULL;
	ofn.nMaxFileTitle=0;
	ofn.lpstrInitialDir=NULL;
	ofn.Flags=OFN_PATHMUSTEXIST|OFN_FILEMUSTEXIST;

	//if user chooses file, init engine
	if (GetOpenFileName(&ofn)) {
		std::wstring wstr(ofn.lpstrFile);
		FILENAME=std::string(wstr.begin(), wstr.end());

		d.startWindowed(8, 120, 68);
	}

	return 0;
}