#include "Engine.h"

namespace displib {
	Engine::Engine() {
		this->raster=Raster();
		this->windowRect={0, 0, 1, 1};
		this->consoleHandle=CreateConsoleScreenBuffer(GENERIC_READ|GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
		this->windowHandle=GetConsoleWindow();

		//prevent user from resizing window
		SetWindowLong(this->windowHandle, GWL_STYLE, GetWindowLong(this->windowHandle, GWL_STYLE)&~WS_MAXIMIZEBOX&~WS_SIZEBOX);
	}

	void Engine::start() {
		//raster setup
		this->raster=Raster(this->width, this->height);

		//timing
		this->lastCallTime=std::chrono::system_clock::now();

		//MAIN
		this->setup();

		while (!this->getKey(VK_ESCAPE)) {
			//timing
			std::chrono::duration<float> elapsedTime=std::chrono::system_clock::now()-this->lastCallTime;
			this->lastCallTime=std::chrono::system_clock::now();
			float dt=elapsedTime.count();

			//mouse
			POINT pt;
			GetCursorPos(&pt);
			ScreenToClient(this->windowHandle, &pt);
			this->mouseX=pt.x/this->charSize;
			this->mouseY=pt.y/this->charSize;

			//update
			this->update(dt);

			//ease of use
			this->framesPerSecond=1/dt;
			this->updateCount++;
			this->totalDeltaTime+=dt;

			//draws
			this->draw(this->raster);

			//show chars to screen
			WriteConsoleOutput(this->consoleHandle, this->raster.getBuffer(), {(short)this->width, (short)this->height}, {0, 0}, &windowRect);
		}
	}

	void Engine::startFullscreen(int cz) {
		this->charSize=cz;

		//set console handle
		SetConsoleActiveScreenBuffer(this->consoleHandle);

		//set console font size
		CONSOLE_FONT_INFOEX cfi;
		cfi.cbSize=sizeof(cfi);
		cfi.nFont=0;
		cfi.dwFontSize.X=this->charSize;
		cfi.dwFontSize.Y=this->charSize;
		cfi.FontFamily=FF_DONTCARE;
		cfi.FontWeight=FW_NORMAL;
		wcscpy_s(cfi.FaceName, L"Consolas");
		SetCurrentConsoleFontEx(this->consoleHandle, TRUE, &cfi);

		//make fullscreen
		SetConsoleDisplayMode(this->consoleHandle, CONSOLE_FULLSCREEN_MODE, 0);

		//set the "pixel sizings"
		CONSOLE_SCREEN_BUFFER_INFO csbi;
		GetConsoleScreenBufferInfo(this->consoleHandle, &csbi);
		this->width=(csbi.srWindow.Right-csbi.srWindow.Left)+1;
		this->height=(csbi.srWindow.Bottom-csbi.srWindow.Top)+1;
		this->windowRect={0, 0, (short)(this->width-1), (short)(this->height-1)};

		//actually start the thing
		this->start();
	}

	void Engine::startWindowed(int cz, int w, int h) {
		this->charSize=cz;
		this->width=w;
		this->height=h;

		//lets make sure its not too big...
		RECT desktop;
		const HWND hDesktop=GetDesktopWindow();
		GetWindowRect(hDesktop, &desktop);
		int screenWidth=desktop.right;
		int screenHeight=desktop.bottom;
		//too wide
		if (this->charSize*this->width>screenWidth) {
			this->width=(screenWidth-64)/this->charSize;
			printf("Window too wide. width now=%d\n", this->width);
		}
		//too tall
		if (this->charSize*this->height>screenHeight) {
			this->height=(screenHeight-64)/this->charSize;
			printf("Window too tall. height now=%d\n", this->height);
		}
		//if still cant fit, or too small, just end prog.
		if (this->width<1||this->height<1) {
			printf("Window too big, screen too small. ending execution.");
			exit(1);
		}

		// below the actual visual size
		this->windowRect={0, 0, 1, 1};
		SetConsoleWindowInfo(this->consoleHandle, TRUE, &this->windowRect);

		// Set the size of the screen buffer
		COORD coord={(short)this->width, (short)this->height};
		SetConsoleScreenBufferSize(this->consoleHandle, coord);

		// Assign screen buffer to the console
		SetConsoleActiveScreenBuffer(this->consoleHandle);

		// Set the font size now that the screen buffer has been assigned to the console
		CONSOLE_FONT_INFOEX cfi;
		cfi.cbSize=sizeof(cfi);
		cfi.nFont=0;
		cfi.dwFontSize.X=this->charSize;
		cfi.dwFontSize.Y=this->charSize;
		cfi.FontFamily=FF_DONTCARE;
		cfi.FontWeight=FW_NORMAL;
		wcscpy_s(cfi.FaceName, L"Consolas");
		SetCurrentConsoleFontEx(this->consoleHandle, TRUE, &cfi);

		// Set Physical Console Window Size
		this->windowRect={0, 0, (short)(this->width-1), (short)(this->height-1)};
		SetConsoleWindowInfo(this->consoleHandle, TRUE, &this->windowRect);

		//actually start the thing
		this->start();
	}

	void Engine::setup() {}

	void Engine::update(float dt) {}

	void Engine::draw(Raster& rst) {}

	bool Engine::getKey(int k) { return GetAsyncKeyState(k); }

	HWND& Engine::getWindowHandle() { return this->windowHandle; }

	void Engine::setTitle(std::string str) { SetConsoleTitleA(str.c_str()); }
}