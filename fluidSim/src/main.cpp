#include "Engine.h"
#include "maths/Maths.h"
using namespace displib;

class Demo : public Engine {
	public:
	int iter=23;
	int size;
	float diff=0.001f, visc=0.001f;
	float* u, * v, * uPrev, * vPrev, * dens, * densPrev;

	short stressGrad[8]{
		Raster::DARK_BLUE,
		Raster::BLUE,
		Raster::DARK_CYAN,
		Raster::CYAN,
		Raster::GREEN,
		Raster::DARK_YELLOW,
		Raster::RED,
		Raster::DARK_RED
	};
	short tempGrad[6]{
		Raster::BLACK,
		Raster::RED,
		Raster::DARK_RED,
		Raster::DARK_YELLOW,
		Raster::YELLOW,
		Raster::WHITE
	};
	short coolGrad[6]{
		Raster::DARK_MAGENTA,
		Raster::DARK_BLUE,
		Raster::BLUE,
		Raster::DARK_CYAN,
		Raster::CYAN,
		Raster::WHITE
	};
	int typeRender=1;

	float mouseTimer=0;
	float2 mousePos, oldMousePos;

	int IX(int i, int j) {
		return i+j*(width+2);
	}

	void setup() override {
		size=(width+2)*(height+2);
		u=new float[size];
		v=new float[size];
		dens=new float[size];
		uPrev=new float[size];
		vPrev=new float[size];
		densPrev=new float[size];
	}

	//fluid methods, from http://graphics.cs.cmu.edu/nsp/course/15-464/Fall09/papers/StamFluidforGames.pdf
	void addSource(float* x, float* s, float dt) {
		for (int i=0; i<size; i++) x[i]+=s[i]*dt;
	}

	void diffuse(int b, float* x, float* x0, float diff, float dt) {
		//make each cell more similar to its neighbor
		float a=dt*diff*width*height;
		for (int k=0; k<iter; k++) {
			for (int i=1; i<=width; i++) {
				for (int j=1; j<=height; j++) {
					x[IX(i, j)]=(
						x0[IX(i, j)]+
						a*(
							x[IX(i-1, j)]+
							x[IX(i+1, j)]+
							x[IX(i, j-1)]+
							x[IX(i, j+1)]
							)
						)/(1+4*a);
				}
			}
			setBound(b, x);
		}
	}

	void advect(int b, float* d, float* d0, float* u, float* v, float dt) {
		int i0, j0, i1, j1;
		float x, y, s0, t0, s1, t1, dt0w, dt0h;
		dt0w=dt*width;
		dt0h=dt*height;
		for (int i=1; i<=width; i++) {
			for (int j=1; j<=height; j++) {
				x=i-dt0w*u[IX(i, j)]; y=j-dt0h*v[IX(i, j)];
				if (x<0.5f) x=0.5f; if (x>width+0.5f) x=width+0.5f; i0=(int)x; i1=i0+1;
				if (y<0.5f) y=0.5f; if (y>height+0.5f) y=height+0.5f; j0=(int)y; j1=j0+1;
				s1=x-i0; s0=1-s1; t1=y-j0; t0=1-t1;
				d[IX(i, j)]=s0*(
					t0*d0[IX(i0, j0)]+
					t1*d0[IX(i0, j1)]
					)+
					s1*(
						t0*d0[IX(i1, j0)]+
						t1*d0[IX(i1, j1)]
						);
			}
		}
		setBound(b, d);
	}

	void densStep(float dt) {
		diffuse(0, densPrev, dens, diff, dt);
		advect(0, dens, densPrev, u, v, dt);
	}

	void velStep(float dt) {
		addSource(u, uPrev, dt); addSource(v, vPrev, dt);

		std::swap(uPrev, u); diffuse(1, u, uPrev, visc, dt);
		std::swap(vPrev, v); diffuse(2, v, vPrev, visc, dt);

		project(u, v, uPrev, vPrev);

		std::swap(uPrev, u); std::swap(vPrev, v);
		advect(1, u, uPrev, uPrev, vPrev, dt);
		advect(2, v, vPrev, uPrev, vPrev, dt);

		project(u,v, uPrev, vPrev);
	}

	void project(float* u, float* v, float* p, float* div) {
		int i, j, k;
		for (i=1; i<=width; i++) {
			for (j=1; j<=height; j++) {
				div[IX(i, j)]=-(
					u[IX(i+1, j)]-u[IX(i-1, j)]+
					v[IX(i, j+1)]-v[IX(i, j-1)]
					)/(width+height);
				p[IX(i, j)]=0;
			}
		}
		setBound(0, div); setBound(0, p);

		for (k=0; k<iter; k++) {
			for (i=1; i<=width; i++) {
				for (j=1; j<=height; j++) {
					p[IX(i, j)]=(
						div[IX(i, j)]+
						p[IX(i-1, j)]+
						p[IX(i+1, j)]+
						p[IX(i, j-1)]+
						p[IX(i, j+1)]
						)/4;
				}
			}
			setBound(0, p);
		}

		for (i=1; i<=width; i++) {
			for (j=1; j<=height; j++) {
				u[IX(i, j)]-=0.5f*(
					p[IX(i+1, j)]-
					p[IX(i-1, j)]
					)*width;
				v[IX(i, j)]-=0.5f*(
					p[IX(i, j+1)]-
					p[IX(i, j-1)]
					)*height;
			}
		}
		setBound(1, u); setBound(2, v);
	}

	void setBound(int b, float* x) {
		//for every y do left/right (x norm plane)
		for (int i=1; i<=height; i++) {
			x[IX(0, i)]=b==1?
				-x[IX(1, i)]:
				x[IX(1, i)];
			x[IX(width+1, i)]=b==1?
				-x[IX(width, i)]:
				x[IX(width, i)];
		}

		//for every x do top/bottom (y norm plane)
		for (int i=1; i<=width; i++) {
			x[IX(i, 0)]=b==2?
				-x[IX(i, 1)]:
				x[IX(i, 1)];
			x[IX(i, height+1)]=b==2?
				-x[IX(i, height)]:
				x[IX(i, height)];
		}

		//corner cases
		x[IX(0, 0)]=0.5f*(x[IX(1, 0)]+x[IX(0, 1)]);
		x[IX(0, height+1)]=0.5f*(x[IX(1, height+1)]+x[IX(0, height)]);
		x[IX(width+1, 0)]=0.5f*(x[IX(width, 0)]+x[IX(width+1, 1)]);
		x[IX(width+1, height+1)]=0.5f*(x[IX(width, height+1)]+x[IX(width+1, height)]);
	}

	void update(float dt) override {

		//mousetry
		if (mouseTimer>0.075f) {
			mouseTimer=0;

			oldMousePos=mousePos;
			mousePos=float2(mouseX, mouseY);
		}
		mouseTimer+=dt;

		//user input
		if (getKey(VK_SPACE)) {
			//check bounds
			if (mouseX>=0&&mouseY>=0&&mouseX<width&&mouseY<height) {
				//calc index
				int i=mouseX+1;
				int j=mouseY+1;

				//add density
				dens[IX(i, j)]+=15*dt;

				//add velocity
				float2 vel=(mousePos-oldMousePos)/2;
				u[IX(i, j)]+=vel.x;
				v[IX(i, j)]+=vel.y;
			}
		}

		//"drag?", or just gradual "removal of fluid", dv/dt=-cv
		for (int i=1; i<=width; i++) {
			for (int j=1; j<=height; j++) {
				dens[IX(i, j)]-=dens[IX(i, j)]*0.0937f*dt;
			}
		}

		//update fluid
		velStep(dt);
		densStep(dt);

		//which coloring scheme to use
		for (int i=1; i<=3; i++) if (getKey(48+i)) typeRender=i;

		//update title
		setTitle("Jos Stam Fluid Sim @ "+std::to_string((int)framesPerSecond)+"fps");
	}

	void draw(Raster& rst) override {
		//clear background
		rst.setChar(' ');
		rst.fillRect(0, 0, width, height);

		//show fluid on screen
		rst.setChar(0x2588);
		for (int i=1; i<=width; i++) {
			for (int j=1; j<=height; j++) {
				//calc pos on screen 
				int x=i-1;
				int y=j-1;

				//use color ramp to show cell
				float densPct=dens[IX(i, j)]*10;
				//based on chosen scheme
				switch (typeRender) {
					case 1: rst.setColor(stressGrad[(int)Maths::clamp(densPct*8, 0, 7)]); break;
					case 2: rst.setColor(tempGrad[(int)Maths::clamp(densPct*6, 0, 5)]); break;
					case 3: rst.setColor(coolGrad[(int)Maths::clamp(densPct*6, 0, 5)]); break;
				}
				rst.putPixel(x, y);
			}
		}
	}
};

int main() {
	//init custom graphics engine
	Demo d;
	d.startWindowed(8, 108, 60);

	return 0;
}