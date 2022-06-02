#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <iostream>
#include <string>

#include "Engine.h"
#include "maths/Maths.h"
using namespace displib;

static std::string FILENAME;

class Demo : public Engine {
	public:
	uint8_t* rgbImage;
	int imgWidth=0, imgHeight=0, bpp=0;
	const char* asciiArr=" .,~=#&@";
	short colorArr[8]={
		Raster::BLACK,
		Raster::BLUE,
		Raster::DARK_GREEN,
		Raster::CYAN,
		Raster::DARK_RED,
		Raster::DARK_MAGENTA,
		Raster::DARK_YELLOW,
		Raster::WHITE
	};

	void setup() override {
		rgbImage=stbi_load(FILENAME.c_str(), &imgWidth, &imgHeight, &bpp, 3);
	}

	void update(float dt) override {
	}

	void draw(Raster& rst) override {
		//background
		rst.setChar(' ');
		rst.fillRect(0, 0, width, height);

		//for each pixel on the screen
		for (int x=0; x<width; x++) {
			for (int y=0; y<height; y++) {
				//get "closest pixel"
				int i=Maths::map(x, 0, width, 0, imgWidth);
				int j=Maths::map(y, 0, height, 0, imgHeight);
				//get index
				int z=(i+j*imgWidth)*3;
				//get pixel colors
				int red=rgbImage[z];
				int green=rgbImage[z+1];
				int blue=rgbImage[z+2];
				int bright=(red+green+blue)/3;
				//set color
				//convert to 8 bit color
				int ci=(red>127)*4+(green>127)*2+(blue>127);
				rst.setColor(colorArr[ci]);
				//set "brightness"
				float pct=bright/255.0f;
				int asi=Maths::clamp(pct*8, 0, 7);
				rst.setChar(asciiArr[asi]);
				//draw the pixel
				rst.putPixel(x, y);
			}
		}

		//update title
		setTitle("Image Testing ["+FILENAME+"] @ "+std::to_string((int)framesPerSecond)+"fps");
	}
};

int main() {
	Demo d=Demo();

	//init ofn
	OPENFILENAME ofn;
	char szFile[1024];
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize=sizeof(ofn);
	ofn.hwndOwner=d.getWindowHandle();
	ofn.lpstrFile=(LPWSTR)szFile;
	ofn.lpstrFile[0]='\0';
	ofn.nMaxFile=sizeof(szFile);
	ofn.lpstrFilter=L"JPEG Image Files (*.jpg)\0*.jpg\0";
	ofn.nFilterIndex=1;
	ofn.lpstrFileTitle=NULL;
	ofn.nMaxFileTitle=0;
	ofn.lpstrInitialDir=NULL;
	ofn.Flags=OFN_PATHMUSTEXIST|OFN_FILEMUSTEXIST;

	//if user chooses file, init engine
	if (GetOpenFileName(&ofn)) {
		std::wstring wstr(ofn.lpstrFile);
		FILENAME=std::string(wstr.begin(), wstr.end());

		d.startWindowed(8, 160, 90);
	}

	return 0;
}