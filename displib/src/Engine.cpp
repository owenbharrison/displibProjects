#include "Engine.h"

namespace displib {
	Engine::Engine() {
		this->raster=Raster();
		this->consoleHandle=GetStdHandle(STD_OUTPUT_HANDLE);
		this->windowHandle=GetConsoleWindow();
	}

	//call this to start
	void Engine::start(int sx, int sy, bool fullScr) {
		this->charWidth=sx;
		this->charHeight=sy;

		//set console handle
		this->consoleHandle=CreateConsoleScreenBuffer(GENERIC_READ|GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
		SetConsoleActiveScreenBuffer(this->consoleHandle);

		//set console font size
		CONSOLE_FONT_INFOEX cfi;
		cfi.cbSize=sizeof(cfi);
		cfi.nFont=0;
		cfi.dwFontSize.X=this->charWidth;
		cfi.dwFontSize.Y=this->charHeight;
		cfi.FontFamily=FF_DONTCARE;
		cfi.FontWeight=FW_NORMAL;
		wcscpy_s(cfi.FaceName, L"Consolas");
		SetCurrentConsoleFontEx(this->consoleHandle, false, &cfi);

		//make fullscreen
		if (fullScr) {
			SetConsoleDisplayMode(this->consoleHandle, CONSOLE_FULLSCREEN_MODE, 0);
		}

		//set the "pixel sizings"
		CONSOLE_SCREEN_BUFFER_INFO csbi;
		GetConsoleScreenBufferInfo(this->consoleHandle, &csbi);
		this->width=(csbi.srWindow.Right-csbi.srWindow.Left)+1;
		this->height=(csbi.srWindow.Bottom-csbi.srWindow.Top)+1;

		//raster setup
		this->raster=Raster(this->width, this->height);

		//some windows nonsense
		this->bytesWritten=0;

		//set window handle for mouse
		this->windowHandle=GetConsoleWindow();

		//timing
		this->lastCallTime=std::chrono::system_clock::now();

		//MAIN
		this->setup();

		while (!this->getKey(27)) {
			//timing
			std::chrono::duration<float> elapsedTime=std::chrono::system_clock::now()-this->lastCallTime;
			this->lastCallTime=std::chrono::system_clock::now();
			float dt=elapsedTime.count();

			//mouse
			POINT pt;
			GetCursorPos(&pt);
			ScreenToClient(this->windowHandle, &pt);
			this->mouseX=pt.x/this->charWidth;
			this->mouseY=pt.y/this->charHeight;

			//update
			this->update(dt);

			//ease of use
			this->framesPerSecond=1/dt;
			this->updateCount++;
			this->totalDeltaTime+=dt;

			//draws
			this->draw(this->raster);

			//show chars to screen
			this->displayRasterToConsole();
		}
	}

	//mains (extend these)
	void Engine::setup() {}

	void Engine::update(float dt) {}

	void Engine::draw(Raster& rst) {}

	//ease of use
	bool Engine::getKey(int k) { return GetAsyncKeyState(k); }

	void Engine::setTitle(std::string str) { SetConsoleTitleA(str.c_str()); }

	//displays raster in text to console window
	void Engine::displayRasterToConsole() {
		SMALL_RECT windowRect={0, 0, this->width-1, this->height-1};
		WriteConsoleOutput(this->consoleHandle, this->raster.getBuffer(), {(short)this->width, (short)this->height}, {0, 0}, &windowRect);
	}
}