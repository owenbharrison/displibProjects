#include "Raster.h"

namespace displib {
	Raster::Raster() : Raster::Raster(10, 10) {}

	Raster::Raster(int w, int h) {
		this->width=w;
		this->height=h;

		//screen bfr
		this->charBuffer=new CHAR_INFO[this->width*this->height];

		//set default char to space
		this->setChar(32);

		//set default color to white
		this->setColor(WHITE);
	}

	void Raster::setChar(short c) { this->currChar.Char.UnicodeChar=c; }

	void Raster::setColor(short c) { this->currChar.Attributes=c; }

	void Raster::_putPixel(int x, int y) {
		if (x>=0&&x<this->width) {//in range of x
			if (y>=0&&y<this->height) {//in range of y
				this->charBuffer[x+y*this->width]=this->currChar;
			}
		}
	}
	void Raster::putPixel(float x, float y) { this->_putPixel(round(x), round(y)); }
	void Raster::putPixel(float2 v) { this->putPixel(v.x, v.y); }

	void Raster::_drawLine(int x1, int y1, int x2, int y2) {
		int x, y, dx, dy, dx1, dy1, px, py, xe, ye, i;
		dx=x2-x1; dy=y2-y1;
		if (dx==0) {
			if (y2<y1) std::swap(y1, y2);
			for (y=y1; y<=y2; y++) this->_putPixel(x1, y);
			return;
		}
		if (dy==0) {
			if (x2<x1) std::swap(x1, x2);
			for (x=x1; x<=x2; x++) this->_putPixel(x, y1);
			return;
		}
		dx1=abs(dx); dy1=abs(dy);
		px=2*dy1-dx1;	py=2*dx1-dy1;
		if (dy1<=dx1) {
			if (dx>=0) {
				x=x1; y=y1; xe=x2;
			}
			else {
				x=x2; y=y2; xe=x1;
			}
			this->_putPixel(x, y);
			for (i=0; x<xe; i++) {
				x++;
				if (px<0) px+=2*dy1;
				else {
					y+=((dx<0&&dy<0)||(dx>0&&dy>0))?1:-1;
					px+=2*(dy1-dx1);
				}
				this->_putPixel(x, y);
			}
		}
		else {
			if (dy>=0) {
				x=x1; y=y1; ye=y2;
			}
			else {
				x=x2; y=y2; ye=y1;
			}
			this->_putPixel(x, y);
			for (i=0; y<ye; i++) {
				y++;
				if (py<=0) py+=2*dx1;
				else {
					x+=((dx<0&&dy<0)||(dx>0&&dy>0))?1:-1;
					py+=2*(dx1-dy1);
				}
				this->_putPixel(x, y);
			}
		}
	}
	void Raster::drawLine(float x1, float y1, float x2, float y2) { this->_drawLine(round(x1), round(y1), round(x2), round(y2)); }
	void Raster::drawLine(float2 v1, float2 v2) { this->drawLine(v1.x, v1.y, v2.x, v2.y); }

	void Raster::_drawTriangle(int x1, int y1, int x2, int y2, int x3, int y3) {
		this->drawLine(x1, y1, x2, y2);
		this->drawLine(x2, y2, x3, y3);
		this->drawLine(x3, y3, x1, y1);
	}
	void Raster::drawTriangle(float x1, float y1, float x2, float y2, float x3, float y3) { this->_drawTriangle(round(x1), round(y1), round(x2), round(y2), round(x3), round(y3)); }
	void Raster::drawTriangle(float2 v1, float2 v2, float2 v3) { this->drawTriangle(v1.x, v1.y, v2.x, v2.y, v3.x, v3.y); }

	void Raster::_fillTriangle(int x1, int y1, int x2, int y2, int x3, int y3) {
		auto drawline=[&](int sx, int ex, int ny) { for (int i=sx; i<=ex; i++) this->_putPixel(i, ny); };

		int t1x, t2x, y, minx, maxx, t1xp, t2xp;
		bool changed1=false;
		bool changed2=false;
		int signx1, signx2, dx1, dy1, dx2, dy2;
		int e1, e2;
		if (y1>y2) { std::swap(y1, y2); std::swap(x1, x2); }
		if (y1>y3) { std::swap(y1, y3); std::swap(x1, x3); }
		if (y2>y3) { std::swap(y2, y3); std::swap(x2, x3); }

		t1x=t2x=x1; y=y1;
		dx1=(int)(x2-x1); if (dx1<0) { dx1=-dx1; signx1=-1; }
		else signx1=1;
		dy1=(int)(y2-y1);

		dx2=(int)(x3-x1); if (dx2<0) { dx2=-dx2; signx2=-1; }
		else signx2=1;
		dy2=(int)(y3-y1);

		if (dy1>dx1) {
			std::swap(dx1, dy1);
			changed1=true;
		}
		if (dy2>dx2) {
			std::swap(dy2, dx2);
			changed2=true;
		}

		e2=(int)(dx2>>1);
		if (y1==y2) goto next;
		e1=(int)(dx1>>1);

		for (int i=0; i<dx1;) {
			t1xp=0; t2xp=0;
			if (t1x<t2x) { minx=t1x; maxx=t2x; }
			else { minx=t2x; maxx=t1x; }
			while (i<dx1) {
				i++;
				e1+=dy1;
				while (e1>=dx1) {
					e1-=dx1;
					if (changed1) t1xp=signx1;
					else goto next1;
				}
				if (changed1) break;
				else t1x+=signx1;
			}
			next1:
			while (true) {
				e2+=dy2;
				while (e2>=dx2) {
					e2-=dx2;
					if (changed2) t2xp=signx2;
					else goto next2;
				}
				if (changed2) break;
				else t2x+=signx2;
			}
			next2:
			if (minx>t1x) minx=t1x; if (minx>t2x) minx=t2x;
			if (maxx<t1x) maxx=t1x; if (maxx<t2x) maxx=t2x;
			drawline(minx, maxx, y);
			if (!changed1) t1x+=signx1;
			t1x+=t1xp;
			if (!changed2) t2x+=signx2;
			t2x+=t2xp;
			y+=1;
			if (y==y2) break;

		}
		next:
		dx1=(int)(x3-x2); if (dx1<0) { dx1=-dx1; signx1=-1; }
		else signx1=1;
		dy1=(int)(y3-y2);
		t1x=x2;

		if (dy1>dx1) {
			std::swap(dy1, dx1);
			changed1=true;
		}
		else changed1=false;

		e1=(int)(dx1>>1);

		for (int i=0; i<=dx1; i++) {
			t1xp=0; t2xp=0;
			if (t1x<t2x) { minx=t1x; maxx=t2x; }
			else { minx=t2x; maxx=t1x; }
			while (i<dx1) {
				e1+=dy1;
				while (e1>=dx1) {
					e1-=dx1;
					if (changed1) { t1xp=signx1; break; }
					else goto next3;
				}
				if (changed1) break;
				else t1x+=signx1;
				if (i<dx1) i++;
			}
			next3:
			while (t2x!=x3) {
				e2+=dy2;
				while (e2>=dx2) {
					e2-=dx2;
					if (changed2) t2xp=signx2;
					else goto next4;
				}
				if (changed2) break;
				else t2x+=signx2;
			}
			next4:
			if (minx>t1x) minx=t1x; if (minx>t2x) minx=t2x;
			if (maxx<t1x) maxx=t1x; if (maxx<t2x) maxx=t2x;
			drawline(minx, maxx, y);
			if (!changed1) t1x+=signx1;
			t1x+=t1xp;
			if (!changed2) t2x+=signx2;
			t2x+=t2xp;
			y+=1;
			if (y>y3) return;
		}
	}
	void Raster::fillTriangle(float x1, float y1, float x2, float y2, float x3, float y3) { this->_fillTriangle(round(x1), round(y1), round(x2), round(y2), round(x3), round(y3)); }
	void Raster::fillTriangle(float2 v1, float2 v2, float2 v3) { this->fillTriangle(v1.x, v1.y, v2.x, v2.y, v3.x, v3.y); }

	void Raster::_drawCircle(int xc, int yc, int r) {
		//wikipedia
		int x=0;
		int y=r;
		int p=3-2*r;
		if (!r) return;

		while (y>=x) {
			this->_putPixel(xc-x, yc-y);
			this->_putPixel(xc-y, yc-x);
			this->_putPixel(xc+y, yc-x);
			this->_putPixel(xc+x, yc-y);
			this->_putPixel(xc-x, yc+y);
			this->_putPixel(xc-y, yc+x);
			this->_putPixel(xc+y, yc+x);
			this->_putPixel(xc+x, yc+y);
			if (p<0) p+=4*(x++)+6;
			else p+=4*(x++-y--)+10;
		}
	}
	void Raster::drawCircle(float x, float y, float r) { this->_drawCircle(round(x), round(y), round(r)); }
	void Raster::drawCircle(float2 v, float r) { this->drawCircle(v.x, v.y, r); }

	void Raster::_fillCircle(int xc, int yc, int r) {
		//wikipedia
		int x=0;
		int y=r;
		int p=3-2*r;
		if (!r) return;

		auto drawline=[&](int sx, int ex, int ny) {for (int i=sx; i<=ex; i++)this->_putPixel(i, ny); };

		while (y>=x) {
			drawline(xc-x, xc+x, yc-y);
			drawline(xc-y, xc+y, yc-x);
			drawline(xc-x, xc+x, yc+y);
			drawline(xc-y, xc+y, yc+x);
			if (p<0) p+=4*x+++6;
			else p+=4*(x++-y--)+10;
		}
	};
	void Raster::fillCircle(float x, float y, float r) { this->_fillCircle(round(x), round(y), round(r)); }
	void Raster::fillCircle(float2 v, float r) { this->fillCircle(v.x, v.y, r); }

	void Raster::_drawRect(int x, int y, int w, int h) {
		for (int i=1; i<w; i++) this->_putPixel(x+i, y);
		for (int i=1; i<h; i++) this->_putPixel(x+w-1, y+i);
		for (int i=w-1; i>=0; i--) this->_putPixel(x+i, y+h-1);
		for (int i=h-1; i>=0; i--) this->_putPixel(x, y+i);
	}
	void Raster::drawRect(float x, float y, float w, float h) { this->_drawRect(round(x), round(y), round(w), round(h)); }
	void Raster::drawRect(float2 v, float w, float h) { this->drawRect(v.x, v.y, w, h); }

	void Raster::_fillRect(int x, int y, int w, int h) {
		for (int i=0; i<w; i++) {
			for (int j=0; j<h; j++) {
				this->_putPixel(x+i, y+j);
			}
		}
	}
	void Raster::fillRect(float x, float y, float w, float h) { this->_fillRect(round(x), round(y), round(w), round(h)); }
	void Raster::fillRect(float2 v, float w, float h) { this->fillRect(v.x, v.y, w, h); }

	//draws a string starting from the left at the specified point, with the col, @ the char size
	void Raster::_drawString(int x_, int y, std::string str) {
		int x=x_;
		wchar_t cCh=this->currChar.Char.UnicodeChar;
		for (auto ch:str) {
			this->setChar(ch);
			this->_putPixel(x, y);
			x++;
		}
		this->setChar(cCh);
	}
	void Raster::drawString(float x, float y, std::string str) { this->_drawString(round(x), round(y), str); }
	void Raster::drawString(float2 v, std::string str) { this->drawString(v.x, v.y, str); }

	//returns the 2d raster buffer
	CHAR_INFO* Raster::getBuffer() {
		return this->charBuffer;
	}
}