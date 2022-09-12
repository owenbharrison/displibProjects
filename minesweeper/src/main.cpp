#include <time.h>
#include <vector>

#include "Engine.h"
#include "maths/Maths.h"
using namespace displib;

struct Cell {
	bool bomb=false;
	bool revealed=false;
	bool flagged=false;
	int numBombs=0;
	//must store ptrs, else size is inf
	std::vector<Cell*> neighbors;

	void flag() {
		//only flag if not revealed
		if (!revealed) flagged=!flagged;
	}

	void reveal() {
		//if flagged, unflag
		if (flagged) flagged=false;
		else if (bomb) {
			//print lose and end program
			printf("GAME OVER! You revealed a bomb.");
			exit(0);
		}
		else {
			revealed=true;

			//reveal all neighbors if this is 0
			if (numBombs==0) {
				for (Cell*& nptr:neighbors) {
					Cell& neighbor=*nptr;
					//only reveal if not revealed
					if (!neighbor.revealed) neighbor.reveal();
				}
			}
		}
	}
};

class Demo : public Engine {
	public:
	Cell* cellGrid=nullptr;
	int res, cols, rows;
	int mouseI=0, mouseJ=0;
	int numTotalBombs=0;
	bool flagKeyDown=false, revealKeyDown=false, started=false;
	//"classic" number coloring in minesweeper
	short colors[8]={
		Raster::DARK_CYAN,
		Raster::GREEN,
		Raster::RED,
		Raster::DARK_YELLOW,
		Raster::DARK_RED,
		Raster::CYAN,
		Raster::GREY,
		Raster::WHITE
	};

	void setup() override {
		//plenty of tiles
		res=5;
		cols=width/res;
		rows=height/res;
		cellGrid=new Cell[cols*rows];
	}

	int cellGridInit(int i, int j) {
		//random bomb placement
		for (int i=0; i<cols; i++) {
			for (int j=0; j<rows; j++) {
				cellGrid[i+j*cols].bomb=Maths::random()<0.18f;
			}
		}

		//make sure the [i, j] can show a starting point, to make it fair
		for (int i_=-1; i_<=1; i_++) {
			for (int j_=-1; j_<=1; j_++) {
				int ri=i+i_;
				int rj=j+j_;
				if (ri>=0&&rj>=0&&ri<=cols-1&&rj<=rows-1) {
					cellGrid[ri+rj*cols].bomb=false;
				}
			}
		}

		//init cells with neighbors
		for (int i=0; i<cols; i++) {
			for (int j=0; j<rows; j++) {
				Cell& cell=cellGrid[i+j*cols];
				int numBombs=0;
				//loop through surrounding cells
				for (int i_=-1; i_<=1; i_++) {
					for (int j_=-1; j_<=1; j_++) {
						int ri=i+i_;
						int rj=j+j_;
						//if in grid "range" init cell with neighbor ptrs
						if (ri>=0&&rj>=0&&ri<=cols-1&&rj<=rows-1) {
							Cell& neighbor=cellGrid[ri+rj*cols];
							cell.neighbors.push_back(&neighbor);
							if (neighbor.bomb) numBombs++;
						}
					}
				}
				cell.numBombs=numBombs;
			}
		}

		//save number of total bombs
		int numBombs=0;
		for (int i=0; i<cols; i++) {
			for (int j=0; j<rows; j++) {
				if (cellGrid[i+j*cols].bomb) numBombs++;
			}
		}
		return numBombs;
	}

	void update(float dt) override {
		//get key states
		bool flagKey=getKey('F');
		bool revealKey=getKey(VK_SPACE);

		//map mouse pos to nearest cell
		mouseI=mouseX/res;
		mouseJ=mouseY/res;

		//only after inital reveal
		if (started) {
			//to make sure we cant hold down the flag key
			if (!flagKeyDown&&flagKey) {
				flagKeyDown=true;
				cellGrid[mouseI+mouseJ*cols].flag();
			}
			if (flagKeyDown&&!flagKey) flagKeyDown=false;

			//to make sure we cant hold down the reveal key
			if (!revealKeyDown&&revealKey) {
				revealKeyDown=true;
				cellGrid[mouseI+mouseJ*cols].reveal();
			}
			if (revealKeyDown&&!revealKey) revealKeyDown=false;

			//check win
			bool allFlagged=true;
			//basically asd long as all cells bomb and flagged state are the same.
			for (int i=0; i<cols; i++) {
				for (int j=0; j<rows; j++) {
					Cell& cell=cellGrid[i+j*cols];
					if (cell.bomb!=cell.flagged) {
						allFlagged=false;
						break;
					}
				}
			}
			if (allFlagged) {
				//print win and end program
				printf("YOU WIN! All the bombs were flagged.");
				exit(0);
			}
		}
		//check for initial reveal
		else if (revealKey) {
			//make certain we cant immediately lose
			numTotalBombs=cellGridInit(mouseI, mouseJ);

			//reveal that inital cell
			cellGrid[mouseI+mouseJ*cols].reveal();

			//"start" the game
			started=true;
		}
	}

	void draw(Raster& rst) override {
		//clear screen
		rst.setChar(' ');
		rst.fillRect(0, 0, width, height);

		//show grid
		int numFlagged=0;
		for (int i=0; i<cols; i++) {
			for (int j=0; j<rows; j++) {
				float x=i*res;
				float y=j*res;
				Cell& cell=cellGrid[i+j*cols];
				if (cell.revealed) {
					rst.setChar(0x2588);
					//index checker method
					bool checker=(i%2)==(j%2);
					//switch between colors
					rst.setColor(checker?Raster::DARK_GREY:Raster::WHITE);
					rst.fillRect(x, y, res, res);

					if (cell.numBombs>0) {
						//show number
						rst.setChar('0'+cell.numBombs);
						//in the appropriate coloring
						rst.setColor(colors[cell.numBombs-1]);
						//as char in center of cell
						rst.putPixel(x+res/2, y+res/2);
					}
				}
				else {
					//this is a mine"field" so make it green
					rst.setChar(0x2588);
					rst.setColor(Raster::GREEN);
					rst.fillRect(x, y, res, res);
					if (cell.flagged) {
						//draw "flag"
						rst.setChar('!');
						rst.setColor(Raster::RED);
						//as char in center of cell
						rst.fillRect(x+1, y+1, res-2, res-2);
					}
				}

				//highlight cell if mouse inside
				if (i==mouseI&&j==mouseJ) {
					rst.setChar(0x2588);
					rst.setColor(Raster::BLACK);
					rst.drawRect(x, y, res, res);
				}

				if (cell.flagged) numFlagged++;
			}
		}

		//this isnt really correct, but giving the true info would make the game easier at the end
		rst.setColor(Raster::RED);
		rst.drawString(0, 0, "Bombs Left: "+std::to_string(numTotalBombs-numFlagged));
	}
};

int main() {
	srand(time(NULL));

	//init custom graphics engine
	Demo d=Demo();
	d.startFullscreen(12);

	return 0;
}