#pragma once

#include <string>

namespace displib {
	class Raster {
		private:
		wchar_t* charBuffer;
		wchar_t currChar;

		public:
		int width, height;

		Raster();//dont use

		Raster(int w, int h);

		void setChar(int c);

		void putPixel(int x, int y);

		void drawLine(int x1, int y1, int x2, int y2);

		void drawTriangle(int x1, int y1, int x2, int y2, int x3, int y3);
		void fillTriangle(int x1, int y1, int x2, int y2, int x3, int y3);

		void drawCircle(int xc, int yc, int r);
		void fillCircle(int xc, int yc, int r);

		void drawRect(int x, int y, int w, int h);
		void fillRect(int x, int y, int w, int h);

		void drawString(int x_, int y, std::string str);

		wchar_t* getBuffer();
	};
}