#include <time.h>
#include <vector>

#include "Engine.h"
#include "maths/Maths.h"
using namespace displib;

struct ptc {
	float2 pos, vel, acc;
	float age=0, lifeSpan;

	ptc(float2 pos_, float2 vel_, float lifeSpan_) {
		pos=pos_;
		vel=vel_;
		lifeSpan=lifeSpan_;
	}

	static ptc random(float x, float y) {
		float angle=Maths::random(-Maths::PI, Maths::PI);
		float speed=Maths::random(3, 5);
		return ptc(float2(x, y), float2(cosf(angle), sinf(angle))*speed, Maths::random(1, 2));
	}

	void update(float dt) {
		//euler explicit
		vel+=acc*dt;
		pos+=vel*dt;
		acc*=0;

		//age with change in time
		age+=dt;
	}

	//too "old"
	bool isDead() { return age>lifeSpan; }

	void render(Raster& rst) {
		//ramp to show how "young" or "vibrant"
		float pct=Maths::map(age, 0, lifeSpan, 1, 0);
		int asi=Maths::clamp(pct*7, 0, 6);
		rst.setChar(".,~=#&@"[asi]);
		rst.putPixel(pos);
	}
};

class Demo : public Engine {
	public:
	int score=0;
	int lines=0;
	int level=1;
	float gravTimer=0;
	int cols=10, rows=22;
	bool lost=false;
	float endTimer=0;
	int endStage=0;
	int combo=-1;
	float comboTimer=0;

	char* field;
	bool* nextTetro=new bool[16];
	bool* currTetro=new bool[16];
	bool* heldTetro=new bool[16];
	bool* tetros=new bool[112]{
		0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0,//|
		0, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0,//z
		0, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0,//s
		0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0,//[]
		0, 0, 1, 0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0,//t
		0, 0, 1, 0, 0, 0, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0,//J
		0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0//L
	};
	int* stats=new int[7];
	short* colors=new short[7]{
		Raster::DARK_CYAN,
		Raster::GREEN,
		Raster::DARK_RED,
		Raster::WHITE,
		Raster::DARK_MAGENTA,
		Raster::DARK_YELLOW,
		Raster::DARK_BLUE
	};
	int currX, currY, currRot, currNum, nextNum, heldNum;

	//key stuff
	float keyStartTime=0.19f, keyRepeatTime=0.058f;
	float downTime=0, leftTime=0, rightTime=0;
	bool down=false, drop=false, left=false, right=false, cw=false, ccw=false, held=false;
	bool wasDown=false, wasDrop=false, wasLeft=false, wasRight=false, wasCw=false, wasCcw=false, wasHeld=false;
	bool canSwitchHold=false;
	bool heldEmpty=true;

	std::vector<ptc> particles;

	int mod(int n, int m) {
		return ((n%m)+m)%m;
	}

	int fIX(int i, int j) {
		return i+j*cols;
	}

	//rotatable tetro index
	int tIX(int i, int j, int r) {
		switch (r) {
			case 0: return i+j*4;
			case 1: return 3+i*4-j;
			case 2: return 15-i-j*4;
			case 3: return 12-i*4+j;
			default: return -1;
		}
	}

	int numToCol(int num) {
		return (num+level/3*2)%7;
	}

	//determines if conceptual piece can fit in grid
	bool tetroCanFit(int x, int y, int r) {
		for (int i=0; i<4; i++) {
			for (int j=0; j<4; j++) {
				int ri=i+x;
				int rj=j+y;
				if (currTetro[tIX(i, j, r)]) {
					//offscurrRoteen
					if (ri<0||rj<0||ri>=cols||rj>=rows) return false;
					//colliding
					if (field[fIX(ri, rj)]>=0) return false;
				}
			}
		}
		return true;
	}

	void resetPiece() {
		//put in the middle, no rotation
		currX=cols/2-2;
		currY=0;
		currRot=0;

		//reset key repeat stuff
		downTime=0, leftTime=0, rightTime=0;
		canSwitchHold=true;
	}

	void getNewPiece() {
		//set curr to next
		currNum=nextNum;
		memcpy(currTetro, tetros+sizeof(bool)*16*currNum, sizeof(bool)*16);

		//change next
		while (nextNum==currNum) nextNum=Maths::clamp(Maths::random(7), 0, 6);
		stats[nextNum]++;
		memcpy(nextTetro, tetros+sizeof(bool)*16*nextNum, sizeof(bool)*16);
	}

	void setup() override {
		//allocate field, make empty
		field=new char[cols*rows];
		memset(field, -1, sizeof(char)*cols*rows);

		//clear heldTetro
		memset(heldTetro, false, sizeof(bool)*16);

		//clear stats
		memset(stats, 0, sizeof(int)*7);

		//first piece
		resetPiece();
		getNewPiece();
		getNewPiece();
	}

	void update(float dt) override {
		if (!lost) {
			//check held
			held=getKey('C');
			if (held&&!wasHeld) {
				if (canSwitchHold) {
					if (heldEmpty) {
						heldEmpty=false;
						heldNum=currNum;
						memcpy(heldTetro, currTetro, sizeof(bool)*16);
						resetPiece();
						getNewPiece();
					}
					else {
						std::swap(heldNum, currNum);
						std::swap(heldTetro, currTetro);
						resetPiece();
					}
					canSwitchHold=false;
				}
			}
			wasHeld=held;

			//movement
			auto moveDown=[&]() { return tetroCanFit(currX, currY+1, currRot)?currY++, 1:0; };
			auto moveLeft=[&]() { tetroCanFit(currX-1, currY, currRot)&&currX--; };
			auto moveRight=[&]() { tetroCanFit(currX+1, currY, currRot)&&currX++; };
			auto moveCw=[&]() { tetroCanFit(currX, currY, mod(currRot+1, 4))&&(currRot=mod(currRot+1, 4)); };
			auto moveCcw=[&]() { tetroCanFit(currX, currY, mod(currRot-1, 4))&&(currRot=mod(currRot-1, 4)); };

			//get key presses
			down=getKey(VK_DOWN);
			drop=getKey(VK_SPACE);
			left=getKey(VK_LEFT);
			right=getKey(VK_RIGHT);
			cw=getKey('Z');
			ccw=getKey(VK_UP);

			//get timing
			downTime=down?downTime+dt:0;
			leftTime=left?leftTime+dt:0;
			rightTime=right?rightTime+dt:0;

			//check each movement
			bool stick=false;

			//on first click movements
			if (down&&!wasDown) { moveDown()?score++:stick=true; }wasDown=down;
			if (left&&!wasLeft) { moveLeft(); }wasLeft=left;
			if (right&&!wasRight) { moveRight(); }wasRight=right;
			if (cw&&!wasCw) { moveCw(); }wasCw=cw;
			if (ccw&&!wasCcw) { moveCcw(); }wasCcw=ccw;
			if (drop&&!wasDrop) {//hard drop
				while (moveDown())score+=2;
				stick=true;
			}
			wasDrop=drop;

			//repeating
			if (downTime>keyStartTime) downTime-=keyRepeatTime, moveDown()?score++:stick;
			if (leftTime>keyStartTime) leftTime-=keyRepeatTime, moveLeft();
			if (rightTime>keyStartTime) rightTime-=keyRepeatTime, moveRight();

			//force down every so often (if can)
			if (gravTimer>0.5f) gravTimer-=0.5f, moveDown()?0:stick=true;

			//should we append the "changes" to the "final" grid?
			if (stick) {
				//place in the grid
				for (int i=0; i<4; i++) {
					for (int j=0; j<4; j++) {
						if (currTetro[tIX(i, j, currRot)]) field[fIX(currX+i, currY+j)]=currNum;
					}
				}

				//reset piece
				resetPiece();
				getNewPiece();

				//if we already cant fit it, game over
				if (!tetroCanFit(currX, currY, currRot)) {
					//end game logic
					lost=true;

					//spawn some particles for every block
					for (int i=0; i<cols; i++) {
						for (int j=0; j<rows; j++) {
							if (field[fIX(i, j)]>=0) {
								//spawn particles at this block
								for (int k=0; k<3; k++) particles.push_back(ptc::random((i+6.5f)*4, (j+1.5f)*4));
							}
						}
					}
				}

				//check for line clearing
				int lns=0;
				for (int j=0; j<rows; j++) {
					bool toClearLine=true;
					for (int i=0; i<cols; i++) {
						if (field[fIX(i, j)]<0) toClearLine=false;
					}
					if (toClearLine) {
						for (int i=0; i<cols; i++) {
							for (int k=0; k<5; k++) particles.push_back(ptc::random((i+6.5f)*4, (j+1.5f)*4));
						}
						//ladder clearing
						for (int i=j; i>0; i--) {
							memcpy(field+i*sizeof(char)*cols, field+(i-1)*sizeof(char)*cols, sizeof(char)*cols);
						}
						//clear top line
						memset(field, -1, sizeof(char)*cols);
						lns++;
					}
				}
				//line scoring
				if (lns>0) {
					switch (lns) {
						case 1: score+=100*level; break;
						case 2: score+=300*level; break;
						case 3: score+=500*level; break;
						case 4: score+=800*level; break;
					}

					combo++;
					comboTimer=0;
					if (combo>0) {
						for (int k=0; k<combo*4; k++) particles.push_back(ptc::random(width/2, height/2));
					}

					score+=50*combo*level;
				}

				//check full board clean
				bool allClear=true;
				for (int i=0; i<cols; i++) {
					for (int j=0; j<rows; j++) {
						if (field[fIX(i, j)]>=0) allClear=false;
					}
				}
				if (allClear) {
					//full clear scoring
					switch (lns) {
						case 1: score+=800*level; break;
						case 2: score+=1200*level; break;
						case 3: score+=1800*level; break;
						case 4: score+=2000*level; break;
					}
				}

				//level updating
				lines+=lns;
				level=lines/10+1;
			}

			//update timers
			gravTimer+=dt;

			//reset combo if too long
			if (comboTimer>2) combo=-1;
			comboTimer+=dt;
		}
		else {
			if (endTimer>0.7f) {
				endTimer-=0.7f;
				endStage++;
			}
			endTimer+=dt;
		}

		//update particles
		for (int i=particles.size()-1; i>=0; i--) {
			ptc& p=particles.at(i);
			p.update(dt);

			if (p.isDead()) {
				particles.erase(particles.begin()+i);
			}
		}

		//change title to show stats.
		setTitle("Tetris! -ish. @ "+std::to_string((int)framesPerSecond)+"fps");
	}

	void draw(Raster& rst) override {
		//background
		rst.setChar(0x2588);
		rst.setColor(Raster::DARK_GREY);
		rst.fillRect(0, 0, width, height);
		//edge
		rst.setColor(Raster::WHITE);
		rst.drawRect(0, 0, width, height);

		//field background
		rst.setChar(' ');
		rst.fillRect(24, 4, width-48, height-8);
		//field edge
		rst.setChar(0x2588);
		rst.drawRect(23, 3, width-46, height-6);
		//field text
		rst.drawString(23, 2, "Playing Field");
		//field
		for (int i=0; i<cols; i++) {
			for (int j=0; j<rows; j++) {
				char c=field[fIX(i, j)];
				//if this is a tetro, show it
				if (c>=0) {
					int x=(i+6)*4;
					int y=(j+1)*4;
					rst.setColor(lost?Raster::RED:colors[numToCol(c)]);
					rst.fillRect(x, y, 4, 4);
				}
			}
		}

		if (!lost) {
			//ghost tetro
			int gy;
			for (gy=currY; tetroCanFit(currX, gy, currRot); gy++);
			rst.setChar('?');
			rst.setColor(colors[numToCol(currNum)]);
			for (int i=0; i<4; i++) {
				for (int j=0; j<4; j++) {
					if (currTetro[tIX(i, j, currRot)]) {
						int x=(currX+i+6)*4;
						int y=(gy+j)*4;
						rst.drawRect(x, y, 4, 4);
					}
				}
			}

			//current tetro
			rst.setChar(0x2588);
			for (int i=0; i<4; i++) {
				for (int j=0; j<4; j++) {
					if (currTetro[tIX(i, j, currRot)]) {
						int x=(currX+i+6)*4;
						int y=(currY+j+1)*4;
						rst.fillRect(x, y, 4, 4);
					}
				}
			}
		}

		//held background
		rst.setChar(' ');
		rst.setColor(Raster::WHITE);
		rst.fillRect(4, 4, 16, 16);
		//held edge
		rst.setChar(0x2588);
		rst.drawRect(3, 3, 18, 18);
		//held text
		rst.drawString(3, 2, "Held Piece");
		//held piece
		rst.setChar(0x2588);
		rst.setColor(colors[numToCol(heldNum)]);
		for (int i=0; i<4; i++) {
			for (int j=0; j<4; j++) {
				if (heldTetro[tIX(i, j, 0)]) {
					int x=(i+1)*4;
					int y=(j+1)*4;
					rst.fillRect(x, y, 4, 4);
				}
			}
		}

		//next background
		rst.setChar(' ');
		rst.setColor(Raster::WHITE);
		rst.fillRect(68, 4, 16, 16);
		//next edge
		rst.setChar(0x2588);
		rst.drawRect(67, 3, 18, 18);
		//next text
		rst.drawString(67, 2, "Next Piece");
		//next piece
		rst.setChar(0x2588);
		rst.setColor(colors[numToCol(nextNum)]);
		for (int i=0; i<4; i++) {
			for (int j=0; j<4; j++) {
				if (nextTetro[tIX(i, j, 0)]) {
					int x=(i+17)*4;
					int y=(j+1)*4;
					rst.fillRect(x, y, 4, 4);
				}
			}
		}

		//show game over screen
		if (lost) {
			rst.setColor(Raster::DARK_RED);
			if (endStage%2) {
				rst.setChar(' ');
				rst.fillRect(width/2-12, height/2-2, 24, 5);
				rst.setChar('#');
				rst.drawRect(width/2-12, height/2-2, 24, 5);
				rst.drawString(width/2-5, height/2-1, "GAME OVER");
				rst.drawString(width/2-11, height/2+1, "You ran out of space!!");
			}
		}
		else rst.setColor(Raster::WHITE);

		//show particles 
		for (ptc& p:particles) p.render(rst);

		//score/level/lines
		rst.setChar(' ');
		rst.setColor(Raster::WHITE);
		showInt(rst, 3, 71, score);
		showInt(rst, 3, 79, level);
		showInt(rst, 3, 87, lines);
		//score/level/lines text
		rst.drawString(3, 69, "Score");
		rst.drawString(3, 77, "Level");
		rst.drawString(3, 85, "Lines");

		//statistics background
		rst.fillRect(71, 39, 10, 36);
		//stats edge
		rst.setChar(0x2588);
		rst.drawRect(70, 38, 12, 38);
		for (int n=0, x=72, y=40; n<7; n++) {
			for (int i=0; i<4; i++) {
				for (int j=0; j<4; j++) {
					if (tetros[n*16+tIX(i, j, 0)]) {
						//show tetro
						rst.setColor(colors[numToCol(n)]);
						rst.putPixel(x+i, y+j);
						//info
						rst.setColor(Raster::WHITE);
						rst.drawString(x+5, y, std::to_string(stats[n]));
					}
				}
			}
			y+=5;
		}
		//stats text
		rst.drawString(70, 37, "Statistics");

		//combo
		if (combo>0) {
			rst.fillCircle(10, 44, 7);
			rst.setChar(' ');
			rst.drawCircle(10, 44, 7);
			rst.drawLine(5, 44, 7, 46);
			rst.drawLine(7, 44, 5, 46);
			showInt(rst, 9, 42, combo);
		}
	}

	bool* digits=new bool[165]{
		1, 1, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1,//0
		0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0,//1
		1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1,//2
		1, 1, 1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 1, 1,//3
		1, 0, 0, 1, 0, 1, 1, 1, 1, 0, 0, 1, 0, 0, 1,//4
		1, 1, 1, 1, 0, 0, 1, 1, 1, 0, 0, 1, 1, 1, 1,//5
		1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 0, 1, 1, 1, 1,//6
		1, 1, 1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 0, 0, 1,//7
		1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1,//8
		1, 1, 1, 1, 0, 1, 1, 1, 1, 0, 0, 1, 0, 0, 1,//9
		1, 0, 0, 1, 0, 1, 1, 1, 0, 1, 0, 1, 1, 0, 1//k
	};

	void showDigit(Raster& rst, int x, int y, int dig) {
		for (int i=0; i<3; i++) {
			for (int j=0; j<5; j++) {
				if (digits[i+j*3+dig*15]) rst.putPixel(x+i, y+j);
			}
		}
	}

	void showInt(Raster& rst, int x, int y, int num_){
		int num=num_;
		int n=num;
		bool showK=false;
		if (num>100000) showK=true, num/=1000, n/=1000;
		if (num==0) showDigit(rst, x, y, 0);

		int count=0, k=1;
		while (n>0) {
			count++;
			n/=10;
			k*=10;
		}
		for (k/=10; k>0; k/=10) {
			int d=num/k%10;
			showDigit(rst, x, y, d);
			x+=4;
		}
		if (showK) showDigit(rst, x, y, 10);
	}
};

int main() {
	srand(time(NULL));

	//init custom graphics engine
	Demo d;
	d.startWindowed(8, 88, 96);

	return 0;
}