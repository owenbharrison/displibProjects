#include "io/Raster.h"
#include <chrono>

namespace displib {
#pragma once
	class Engine {
		private:
		Raster raster;
		HANDLE consoleHandle;
		HWND windowHandle;
		SMALL_RECT windowRect;
		int charSize=0;
		std::chrono::time_point<std::chrono::system_clock> lastCallTime;

		void start();

		public:
		int width=0, height=0;
		int updateCount=0;
		int mouseX=0, mouseY=0;
		float framesPerSecond=0, totalDeltaTime=0;

		static void showPopupBox(std::string title, std::string content) {
			MessageBoxA(0, content.c_str(), title.c_str(), MB_OK);
		}

		Engine();

		void startFullscreen(int cz);

		void startWindowed(int cz, int w, int h);

		virtual void setup();

		virtual void update(float dt);

		virtual void draw(Raster& rst);

		bool getKey(int k);

		HWND& getWindowHandle();

		void setTitle(std::string str);
	};
}