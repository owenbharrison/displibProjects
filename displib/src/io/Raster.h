#define _WIN32_WINNT 0x0500

#include <windows.h>
#include <string>

#include "../maths/vector/V2D.h"

namespace displib {
#pragma once
	class Raster {
		private:
		CHAR_INFO* charBuffer;
		CHAR_INFO currChar;

		void _putPixel(int x, int y);

		void _drawLine(int x1, int y1, int x2, int y2);

		void _drawTriangle(int x1, int y1, int x2, int y2, int x3, int y3);

		void _fillTriangle(int x1, int y1, int x2, int y2, int x3, int y3);

		void _drawCircle(int xc, int yc, int r);

		void _fillCircle(int xc, int yc, int r);

		void _drawRect(int x, int y, int w, int h);

		void _fillRect(int x, int y, int w, int h);

		void _drawString(int x_, int y, std::string str);

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

		void putPixel(float x, float y), putPixel(V2D v);

		void drawLine(float x1, float y1, float x2, float y2), drawLine(V2D v1, V2D v2);

		void drawTriangle(float x1, float y1, float x2, float y2, float x3, float y3), drawTriangle(V2D v1, V2D v2, V2D v3);

		void fillTriangle(float x1, float y1, float x2, float y2, float x3, float y3), fillTriangle(V2D v1, V2D v2, V2D v3);

		void drawCircle(float x, float y, float r), drawCircle(V2D v, float r);

		void fillCircle(float x, float y, float r), fillCircle(V2D v, float r);

		void drawRect(float x, float y, float w, float h), drawRect(V2D v, float w, float h);

		void fillRect(float x, float y, float w, float h), fillRect(V2D v, float w, float h);

		void drawString(float x, float y, std::string str), drawString(V2D v, std::string str);

		CHAR_INFO* getBuffer();
	};
}