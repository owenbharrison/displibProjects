#define _WIN32_WINNT 0x0500

#include <windows.h>
#include <string>

namespace displib {
#pragma once
	class Raster {
		private:
		CHAR_INFO* charBuffer;
		CHAR_INFO currChar;

		public:
		enum COLORS {
			BLACK=0x0000,
			DARK_BLUE=0x0001,
			DARK_GREEN=0x0002,
			DARK_CYAN=0x0003,
			DARK_RED=0x0004,
			DARK_MAGENTA=0x0005,
			DARK_YELLOW=0x0006,
			GREY=0x0007,
			DARK_GREY=0x0008,
			BLUE=0x0009,
			GREEN=0x000A,
			CYAN=0x000B,
			RED=0x000C,
			MAGENTA=0x000D,
			YELLOW=0x000E,
			WHITE=0x000F
		};

		int width, height;

		Raster();//dont use

		Raster(int w, int h);

		void setChar(short c);

		void setColor(short c);

		void putPixel(int x, int y);

		void drawLine(int x1, int y1, int x2, int y2);

		void drawTriangle(int x1, int y1, int x2, int y2, int x3, int y3);
		void fillTriangle(int x1, int y1, int x2, int y2, int x3, int y3);

		void drawCircle(int xc, int yc, int r);
		void fillCircle(int xc, int yc, int r);

		void drawRect(int x, int y, int w, int h);
		void fillRect(int x, int y, int w, int h);

		void drawString(int x_, int y, std::string str);

		CHAR_INFO* getBuffer();
	};
}