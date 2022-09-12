#define _WIN32_WINNT 0x0500

#include <windows.h>
#include <string>

#include "../maths/vector/float2.h"

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
		//list of console colors, thanks javidx9
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

		//construct new buffer for raster
		Raster(int w, int h);

		//set current console char.
		void setChar(short c);

		//set current console char color.
		void setColor(short c);

		//renders current char at [x, y]
		void putPixel(float x, float y), putPixel(float2 v);

		//renders line of current char from [x1, y1] to [x2, y2]
		void drawLine(float x1, float y1, float x2, float y2), drawLine(float2 v1, float2 v2);

		//renders triangle using specified coordinates.
		void drawTriangle(float x1, float y1, float x2, float y2, float x3, float y3), drawTriangle(float2 v1, float2 v2, float2 v3);

		//renders filled triangle using specified coordinates.
		void fillTriangle(float x1, float y1, float x2, float y2, float x3, float y3), fillTriangle(float2 v1, float2 v2, float2 v3);

		//renders circle using specified coordinates.
		void drawCircle(float x, float y, float r), drawCircle(float2 v, float r);

		//renders filled circle using specified coordinates.
		void fillCircle(float x, float y, float r), fillCircle(float2 v, float r);

		//renders rectangle using specified coordinates.
		void drawRect(float x, float y, float w, float h), drawRect(float2 v, float w, float h);

		//renders filled rectangle using specified coordinates.
		void fillRect(float x, float y, float w, float h), fillRect(float2 v, float w, float h);

		//renders string from left to right at specified coordinates.
		void drawString(float x, float y, std::string str), drawString(float2 v, std::string str);

		//returns the buffer data.
		CHAR_INFO* getBuffer();
	};
}