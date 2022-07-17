#include <vector>

#include "Engine.h"
#include "maths/Maths.h"
using namespace displib;

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

static std::string FILENAME;

class Demo : public Engine {
	public:
	int imgWidth, imgHeight, imgChannels;
	unsigned char* imgData;
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

	short rgbToCol(int r, int g, int b) {
		return colors[(r>127)*4+(g>127)*2+(b>127)];
	}

	int ix(int i, int j) {
		return (i+j*width)*3;
	}

	void setup() override {
		setTitle("Floyd Steinberg Dithering");

		//load image
		unsigned char* origImg=stbi_load(FILENAME.c_str(), &imgWidth, &imgHeight, &imgChannels, 0);

		//resize image
		imgData=new unsigned char[width*height*3];
		for (int x=0; x<width; x++) {
			for (int y=0; y<height; y++) {
				//map x,y to pixel in image i,j
				int i=Maths::map(x, 0, width, 0, imgWidth);
				int j=Maths::map(y, 0, height, 0, imgHeight);
				int origZ=(i+j*imgWidth)*imgChannels;
				int newZ=ix(x, y);
				imgData[newZ]=origImg[origZ];
				imgData[newZ+1]=origImg[origZ+1];
				imgData[newZ+2]=origImg[origZ+2];
			}
		}
		delete[] origImg;

		//dither it
		for (int y=0; y<height-1; y++) {
			for (int x=1; x<width-1; x++) {
				int i=ix(x, y);
				int oR=imgData[i];
				int oG=imgData[i+1];
				int oB=imgData[i+2];
				int nR=(oR>127)*255;
				int nG=(oG>127)*255;
				int nB=(oB>127)*255;
				imgData[i]=nR;
				imgData[i+1]=nG;
				imgData[i+2]=nB;

				int eR=oR-nR;
				int eG=oG-nG;
				int eB=oB-nB;

				int ir=ix(x+1, y);
				imgData[ir]+=eR*0.4375f;
				imgData[ir+1]+=eG*0.4375f;
				imgData[ir+2]+=eB*0.4375f;

				int ibl=ix(x-1, y+1);
				imgData[ibl]+=eR*0.1875f;
				imgData[ibl+1]+=eG*0.1875f;
				imgData[ibl+2]+=eB*0.1875f;

				int ib=ix(x, y+1);
				imgData[ib]+=eR*0.3125f;
				imgData[ib+1]+=eG*0.3125f;
				imgData[ib+2]+=eB*0.3125f;

				int ibr=ix(x+1, y+1);
				imgData[ibr]+=eR*0.0625f;
				imgData[ibr+1]+=eG*0.0625f;
				imgData[ibr+2]+=eB*0.0625f;
			}
		}
	}

	void update(float dt) override {

	}

	void draw(Raster& rst) override {
		//background
		rst.setChar(' ');
		rst.fillRect(0, 0, width, height);

		rst.setChar(0x2588);
		for (int x=0; x<width; x++) {
			for (int y=0; y<height; y++) {
				int z=ix(x, y);
				rst.setColor(rgbToCol(imgData[z], imgData[z+1], imgData[z+2]));
				rst.putPixel(x, y);
			}
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

		d.startWindowed(4, 240, 135);
	}

	return 0;
}