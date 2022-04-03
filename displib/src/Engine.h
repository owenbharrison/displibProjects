#include "io/Raster.h"
#include <chrono>

namespace displib {
#pragma once
	class Engine {
		private:
		Raster raster;
		HANDLE consoleHandle;
		HWND windowHandle;
		DWORD bytesWritten=0;
		int charWidth=0, charHeight=0;
		std::chrono::time_point<std::chrono::system_clock> lastCallTime;

		void displayRasterToConsole();

		public:
		int width=0, height=0;
		int updateCount=0;
		int mouseX=0, mouseY=0;
		float framesPerSecond=0.0f, totalDeltaTime=0.0f;

		Engine();

		void start(int sx, int sy, bool fullScr);

		virtual void setup();

		virtual void shutdown();

		virtual void update(float dt);

		virtual void draw(Raster& rst);

		bool getKey(int k);

		void setTitle(std::string str);
	};
}