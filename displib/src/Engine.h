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

		//displays windows box for a message
		static void showPopupBox(std::string title, std::string content) {
			MessageBoxA(0, content.c_str(), title.c_str(), MB_OK);
		}
		
		Engine();

		//new fullscreen console with specific square sized chars
		void startFullscreen(int cz);

		//new console window with specific square sized chars, sized accordingly.
		void startWindowed(int cz, int w, int h);

		//this is called at the start of the program, must extend it.
		virtual void setup();

		//this is called as fast as possible, giving the delta time since the last frame, must extend it.
		virtual void update(float dt);

		//this is called as fast as possible, giving the raster in which to draw/render on, must extend it.
		virtual void draw(Raster& rst);

		//is this key pressed?
		bool getKey(int k);

		HWND& getWindowHandle();

		//set console title.
		void setTitle(std::string str);
	};
}