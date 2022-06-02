#include "Engine.h"
#include "maths/Maths.h"
#include "geom/AABB3D.h"
#include <time.h>

#include "physics/Spring.h"
using namespace displib;

//i really havent a clue where i got this XD
V2D projV3D(V3D v, float yaw, float pitch, float zoom) {
	return V2D(
		sinf(yaw)*v.x-cosf(yaw)*v.z,
		(cosf(yaw)*v.x+sinf(yaw)*v.z)*cosf(pitch)+sinf(pitch)*v.y
	)*zoom;
}

//for some pemdas reason, this cant be a preprocessor define.
int ix(int x, int y, int z, int w, int d) {
	return x+z*w+y*w*d;
}

class Demo : public Engine {
	public:
	V3D grav;
	V2D ctr;
	AABB3D bounds;
	int particleNum=0;
	int springNum=0;
	Particle* particles={};
	Spring* springs={};
	float camYaw=2.074755f, camPitch=-1.060102f;
	float camZoom=1;

	void setup() override {
		ctr=V2D(width/2, height/2);
		
		float sz=50;
		bounds=AABB3D(-sz, -sz, -sz, sz, sz, sz);

		float stiff=240.47f;
		float damp=6.23f;
		int w=3;
		int h=5;
		int d=3;
		AABB3D bodyBnds(-sz/3, -sz*2/3, -sz/3, sz/3, sz*2/3, sz/3);//body container

		particleNum=w*h*d;
		particles=new Particle[particleNum];
		for (int x=0; x<w; x++) {
			for (int y=0; y<h; y++) {
				for (int z=0; z<d; z++) {
					float rx=Maths::map(x, 0, w-1, bodyBnds.min.x, bodyBnds.max.x)+Maths::random(-0.2, 0.2);
					float ry=Maths::map(y, 0, h-1, bodyBnds.min.y, bodyBnds.max.y)+Maths::random(-0.2, 0.2);
					float rz=Maths::map(z, 0, d-1, bodyBnds.min.z, bodyBnds.max.z)+Maths::random(-0.2, 0.2);
					particles[ix(x, y, z, w, d)]=Particle(V3D(rx, ry, rz));
				}
			}
		}

		//connect axis aligned springs
		int k=0;
		springNum=13*w*h*d-9*(w*h+h*d+d*w)+6*(w+h+d)-4;
		springs=new Spring[springNum];
		for (int x=0; x<w; x++) {
			for (int y=0; y<h; y++) {
				for (int z=0; z<d; z++) {
					if (x<w-1) springs[k++]=Spring(particles[ix(x, y, z, w, d)], particles[ix(x+1, y, z, w, d)], stiff, damp);
					if (y<h-1) springs[k++]=Spring(particles[ix(x, y, z, w, d)], particles[ix(x, y+1, z, w, d)], stiff, damp);
					if (z<d-1) springs[k++]=Spring(particles[ix(x, y, z, w, d)], particles[ix(x, y, z+1, w, d)], stiff, damp);
				}
			}
		}
		//for x, set zy diags
		for (int x=0; x<w; x++) {
			for (int y=0; y<h-1; y++) {
				for (int z=0; z<d-1; z++) {
					springs[k++]=Spring(particles[ix(x, y, z, w, d)], particles[ix(x, y+1, z+1, w, d)], stiff, damp);
					springs[k++]=Spring(particles[ix(x, y, z+1, w, d)], particles[ix(x, y+1, z, w, d)], stiff, damp);
				}
			}
		}
		//for y, set zx diags
		for (int y=0; y<h; y++) {
			for (int z=0; z<d-1; z++) {
				for (int x=0; x<w-1; x++) {
					springs[k++]=Spring(particles[ix(x, y, z, w, d)], particles[ix(x+1, y, z+1, w, d)], stiff, damp);
					springs[k++]=Spring(particles[ix(x+1, y, z, w, d)], particles[ix(x, y, z+1, w, d)], stiff, damp);
				}
			}
		}
		//for z, set xy diags
		for (int z=0; z<d; z++) {
			for (int x=0; x<w-1; x++) {
				for (int y=0; y<h-1; y++) {
					springs[k++]=Spring(particles[ix(x, y, z, w, d)], particles[ix(x+1, y+1, z, w, d)], stiff, damp);
					springs[k++]=Spring(particles[ix(x+1, y, z, w, d)], particles[ix(x, y+1, z, w, d)], stiff, damp);
				}
			}
		}
		//total diags
		for (int x=0; x<w-1; x++) {
			for (int y=0; y<h-1; y++) {
				for (int z=0; z<d-1; z++) {
					springs[k++]=Spring(particles[ix(x, y, z, w, d)], particles[ix(x+1, y+1, z+1, w, d)], stiff, damp);
					springs[k++]=Spring(particles[ix(x, y, z+1, w, d)], particles[ix(x+1, y+1, z, w, d)], stiff, damp);
					springs[k++]=Spring(particles[ix(x, y+1, z, w, d)], particles[ix(x+1, y, z+1, w, d)], stiff, damp);
					springs[k++]=Spring(particles[ix(x, y+1, z+1, w, d)], particles[ix(x+1, y, z, w, d)], stiff, damp);
				}
			}
		}
	}

	void update(float dt) override {
		//moving
		float amt=Maths::PI/2;
		//slow
		if (getKey(VK_SHIFT)) amt/=5;

		if (getKey('A')) camYaw-=amt*dt;
		if (getKey('D')) camYaw+=amt*dt;
		if (getKey('W')) camPitch-=amt*dt;
		if (getKey('S')) camPitch+=amt*dt;
		camPitch=Maths::clamp(camPitch, -Maths::PI, 0);

		if(getKey('R')) this->setup();

		if (getKey(VK_UP)) camZoom+=amt*dt;
		if (getKey(VK_DOWN)) camZoom-=amt*dt;
		if (camZoom<1) camZoom=1;
		
		//softbody updating

		//update springs first
		for (int i=0; i<springNum; i++) {
			springs[i].update();
		}

		//add grav, update all particles
		for (int i=0; i<particleNum; i++) {
			particles[i].applyForce(grav);
			particles[i].update(dt);
			particles[i].constrainIn(bounds);
		}

		grav.y=Maths::map(sinf(totalDeltaTime), -1, 1, 6, -42);
	}

	void draw(Raster& rst) override {
		//background
		rst.setChar(' ');
		rst.fillRect(0, 0, width, height);

		//show springs
		rst.setChar('.');
		for (int i=0; i<springNum; i++) {
			Spring& s=springs[i];
			V2D a=projV3D(s.getA().pos, camYaw, camPitch, camZoom)+ctr;
			V2D b=projV3D(s.getB().pos, camYaw, camPitch, camZoom)+ctr;
			drawLine(rst, a, b);
		}

		//show particles
		rst.setChar('@');
		for (int i=0; i<particleNum; i++) {
			Particle& p=particles[i];
			V2D proj=projV3D(p.pos, camYaw, camPitch, camZoom)+ctr;
			rst.putPixel(proj.x, proj.y);
		}

		//draw bounds
		float nx=bounds.min.x, ny=bounds.min.y, nz=bounds.min.z;
		float xx=bounds.max.x, xy=bounds.max.y, xz=bounds.max.z;
		V2D nnn=projV3D(V3D(nx, ny, nz), camYaw, camPitch, camZoom)+ctr;
		V2D nnx=projV3D(V3D(nx, ny, xz), camYaw, camPitch, camZoom)+ctr;
		V2D nxn=projV3D(V3D(nx, xy, nz), camYaw, camPitch, camZoom)+ctr;
		V2D nxx=projV3D(V3D(nx, xy, xz), camYaw, camPitch, camZoom)+ctr;
		V2D xnn=projV3D(V3D(xx, ny, nz), camYaw, camPitch, camZoom)+ctr;
		V2D xnx=projV3D(V3D(xx, ny, xz), camYaw, camPitch, camZoom)+ctr;
		V2D xxn=projV3D(V3D(xx, xy, nz), camYaw, camPitch, camZoom)+ctr;
		V2D xxx=projV3D(V3D(xx, xy, xz), camYaw, camPitch, camZoom)+ctr;
		//top
		drawLine(rst, nnn, nnx);
		drawLine(rst, nnx, xnx);
		drawLine(rst, xnx, xnn);
		drawLine(rst, xnn, nnn);
		//verts
		drawLine(rst, nnn, nxn);
		drawLine(rst, nnx, nxx);
		drawLine(rst, xnx, xxx);
		drawLine(rst, xnn, xxn);
		//bottom
		drawLine(rst, nxn, nxx);
		drawLine(rst, nxx, xxx);
		drawLine(rst, xxx, xxn);
		drawLine(rst, xxn, nxn);

		//show fps
		setTitle("3D SoftBody Sim @ "+std::to_string((int)framesPerSecond)+"fps");
	}

	void drawLine(Raster& rst, V2D a, V2D b) {
		rst.drawLine(a.x, a.y, b.x, b.y);
	}
};

int main() {
	srand(time(NULL));
	//init custom graphics engine
	Demo d=Demo();
	d.startWindowed(6, 90, 110);

	return 0;
}