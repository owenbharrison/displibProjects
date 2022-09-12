#include <time.h>
#include <vector>
#include <algorithm>

#include "Engine.h"
#include "maths/Maths.h"
#include "geom/AABB3D.h"
using namespace displib;

//i really havent a clue where i got this XD
float2 projfloat3(float3 v, float yaw, float pitch, float zoom) {
	return float2(
		sinf(yaw)*v.x-cosf(yaw)*v.z,
		(cosf(yaw)*v.x+sinf(yaw)*v.z)*cosf(pitch)+sinf(pitch)*v.y
	)*zoom;
}

//for some pemdas reason, this cant be a preprocessor define.
int ix(int x, int y, int z, int w, int d) {
	return x+z*w+y*w*d;
}

struct ptc {
	float3 pos, vel, acc;

	ptc() {}

	ptc(float3 pos_) { pos=pos_; }

	void update(float dt) {
		vel+=acc*dt;
		pos+=vel*dt;
		acc*=0;
	}

	void applyForce(float3 f) { acc+=f; }

	void constrainIn(AABB3D a) {
		//bounds detection and bounce
		if (pos.x<a.min.x) { pos.x=a.min.x; vel.x*=-1; }
		if (pos.y<a.min.y) { pos.y=a.min.y; vel.y*=-1; }
		if (pos.z<a.min.z) { pos.z=a.min.z; vel.z*=-1; }
		if (pos.x>a.max.x) { pos.x=a.max.x; vel.x*=-1; }
		if (pos.y>a.max.y) { pos.y=a.max.y; vel.y*=-1; }
		if (pos.z>a.max.z) { pos.z=a.max.z; vel.z*=-1; }
	}
};

struct spr {
	ptc* a=nullptr, * b=nullptr;

	float restLen=0, stiff=0, damp=0;

	spr() {}

	spr(ptc& a_, ptc& b_, float stiff_, float damp_) {
		a=&a_;
		b=&b_;
		restLen=length(getA().pos-getB().pos);
		stiff=stiff_;
		damp=damp_;
	}

	ptc& getA() { return *a; }
	ptc& getB() { return *b; }

	void update() {
		float3 sub=getB().pos-getA().pos;
		float3 dir=normalize(sub);
		float fs=stiff*(length(sub)-restLen);
		float fd=dot(dir, getB().vel-getA().vel)*damp;
		float3 f=dir*(fs+fd);

		getA().applyForce(f);
		getB().applyForce(f*-1);
	}
};

struct tri {
	ptc* a=nullptr, * b=nullptr, * c=nullptr;

	tri() {}

	tri(ptc& a_, ptc& b_, ptc& c_) {
		a=&a_;
		b=&b_;
		c=&c_;
	}

	ptc& getA() { return *a; }
	ptc& getB() { return *b; }
	ptc& getC() { return *c; }

	float3 getNorm() {//just flip the norms, this is getting ridiculous
		return normalize(cross(getB().pos-getA().pos, getC().pos-getA().pos));
	}

	float3 getAvgPos() {
		return (getA().pos+getB().pos+getC().pos)/3;
	}
};

class Demo : public Engine {
	public:
	float3 grav;
	float2 ctr;
	AABB3D bounds;
	int ptcNum=0;
	ptc* ptcs={};
	int sprNum=0;
	spr* sprs={};
	int triNum=0;
	tri* tris;
	float camYaw=2.074755f, camPitch=-1.060102f;
	float camZoom=82.45f;

	bool showOutline=true;
	bool oDown=false;

	float3 lightDir, camPos;
	const char* asciiArr=".,~=#&@";

	void setup() override {
		ctr=float2(width/2, height/2);

		lightDir=normalize(float3(1, -1, 1));

		bounds=AABB3D(-0.5f, -0.5f, -0.5f, 0.5f, 0.5f, 0.5f);

		float stiff=1220.67f;
		float damp=4.83f;
		int w=4;
		int h=6;
		int d=4;
		AABB3D bodyBnds(-0.1667f, -0.3333f, -0.1667f, 0.1667f, 0.3333f, 0.1667f);//body container

		ptcNum=w*h*d;
		ptcs=new ptc[ptcNum];
		for (int x=0; x<w; x++) {
			for (int y=0; y<h; y++) {
				for (int z=0; z<d; z++) {
					float rx=Maths::map(x, 0, w-1, bodyBnds.min.x, bodyBnds.max.x)+Maths::random(-0.0001f, 0.0001f);
					float ry=Maths::map(y, 0, h-1, bodyBnds.min.y, bodyBnds.max.y)+Maths::random(-0.0001f, 0.0001f);
					float rz=Maths::map(z, 0, d-1, bodyBnds.min.z, bodyBnds.max.z)+Maths::random(-0.0001f, 0.0001f);
					ptcs[ix(x, y, z, w, d)]=ptc(float3(rx, ry, rz));
				}
			}
		}

		//connect all springs
		int s=0;
		sprNum=13*w*h*d-9*(w*h+h*d+d*w)+6*(w+h+d)-4;
		sprs=new spr[sprNum];
		//connect axis aligned springs
		for (int x=0; x<w; x++) {
			for (int y=0; y<h; y++) {
				for (int z=0; z<d; z++) {
					if (x<w-1) sprs[s++]=spr(ptcs[ix(x, y, z, w, d)], ptcs[ix(x+1, y, z, w, d)], stiff, damp);
					if (y<h-1) sprs[s++]=spr(ptcs[ix(x, y, z, w, d)], ptcs[ix(x, y+1, z, w, d)], stiff, damp);
					if (z<d-1) sprs[s++]=spr(ptcs[ix(x, y, z, w, d)], ptcs[ix(x, y, z+1, w, d)], stiff, damp);
				}
			}
		}
		//for x, set yz diags
		for (int x=0; x<w; x++) {
			for (int y=0; y<h-1; y++) {
				for (int z=0; z<d-1; z++) {
					sprs[s++]=spr(ptcs[ix(x, y, z, w, d)], ptcs[ix(x, y+1, z+1, w, d)], stiff, damp);
					sprs[s++]=spr(ptcs[ix(x, y, z+1, w, d)], ptcs[ix(x, y+1, z, w, d)], stiff, damp);
				}
			}
		}
		//for y, set zx diags
		for (int y=0; y<h; y++) {
			for (int z=0; z<d-1; z++) {
				for (int x=0; x<w-1; x++) {
					sprs[s++]=spr(ptcs[ix(x, y, z, w, d)], ptcs[ix(x+1, y, z+1, w, d)], stiff, damp);
					sprs[s++]=spr(ptcs[ix(x+1, y, z, w, d)], ptcs[ix(x, y, z+1, w, d)], stiff, damp);
				}
			}
		}
		//for z, set xy diags
		for (int z=0; z<d; z++) {
			for (int x=0; x<w-1; x++) {
				for (int y=0; y<h-1; y++) {
					sprs[s++]=spr(ptcs[ix(x, y, z, w, d)], ptcs[ix(x+1, y+1, z, w, d)], stiff, damp);
					sprs[s++]=spr(ptcs[ix(x+1, y, z, w, d)], ptcs[ix(x, y+1, z, w, d)], stiff, damp);
				}
			}
		}
		//total diags
		for (int x=0; x<w-1; x++) {
			for (int y=0; y<h-1; y++) {
				for (int z=0; z<d-1; z++) {
					sprs[s++]=spr(ptcs[ix(x, y, z, w, d)], ptcs[ix(x+1, y+1, z+1, w, d)], stiff, damp);
					sprs[s++]=spr(ptcs[ix(x, y, z+1, w, d)], ptcs[ix(x+1, y+1, z, w, d)], stiff, damp);
					sprs[s++]=spr(ptcs[ix(x, y+1, z, w, d)], ptcs[ix(x+1, y, z+1, w, d)], stiff, damp);
					sprs[s++]=spr(ptcs[ix(x, y+1, z+1, w, d)], ptcs[ix(x+1, y, z, w, d)], stiff, damp);
				}
			}
		}

		//connect all tris
		int t=0;
		triNum=4*(w*h+h*d+d*w-2*(w+h+d)+3);
		tris=new tri[triNum];
		//x=[0, w-1]: set yz tris
		for (int y=0; y<h-1; y++) {
			for (int z=0; z<d-1; z++) {
				//front face
				tris[t++]=tri(ptcs[ix(0, y, z, w, d)], ptcs[ix(0, y+1, z, w, d)], ptcs[ix(0, y, z+1, w, d)]);
				tris[t++]=tri(ptcs[ix(0, y+1, z, w, d)], ptcs[ix(0, y+1, z+1, w, d)], ptcs[ix(0, y, z+1, w, d)]);
				//back face
				tris[t++]=tri(ptcs[ix(w-1, y, z, w, d)], ptcs[ix(w-1, y+1, z+1, w, d)], ptcs[ix(w-1, y+1, z, w, d)]);
				tris[t++]=tri(ptcs[ix(w-1, y, z, w, d)], ptcs[ix(w-1, y, z+1, w, d)], ptcs[ix(w-1, y+1, z+1, w, d)]);
			}
		}
		//y=[0, h-1]: set zx tris
		for (int z=0; z<d-1; z++) {
			for (int x=0; x<w-1; x++) {
				//front face
				tris[t++]=tri(ptcs[ix(x, 0, z, w, d)], ptcs[ix(x, 0, z+1, w, d)], ptcs[ix(x+1, 0, z, w, d)]);
				tris[t++]=tri(ptcs[ix(x, 0, z+1, w, d)], ptcs[ix(x+1, 0, z+1, w, d)], ptcs[ix(x+1, 0, z, w, d)]);
				//back face
				tris[t++]=tri(ptcs[ix(x, h-1, z, w, d)], ptcs[ix(x+1, h-1, z+1, w, d)], ptcs[ix(x, h-1, z+1, w, d)]);
				tris[t++]=tri(ptcs[ix(x, h-1, z, w, d)], ptcs[ix(x+1, h-1, z, w, d)], ptcs[ix(x+1, h-1, z+1, w, d)]);
			}
		}
		//z=[0, d-1]: set xy tris
		for (int x=0; x<w-1; x++) {
			for (int y=0; y<h-1; y++) {
				//front face
				tris[t++]=tri(ptcs[ix(x, y, 0, w, d)], ptcs[ix(x+1, y, 0, w, d)], ptcs[ix(x, y+1, 0, w, d)]);
				tris[t++]=tri(ptcs[ix(x+1, y, 0, w, d)], ptcs[ix(x+1, y+1, 0, w, d)], ptcs[ix(x, y+1, 0, w, d)]);
				//back face
				tris[t++]=tri(ptcs[ix(x, y, d-1, w, d)], ptcs[ix(x+1, y+1, d-1, w, d)], ptcs[ix(x+1, y, d-1, w, d)]);
				tris[t++]=tri(ptcs[ix(x, y, d-1, w, d)], ptcs[ix(x, y+1, d-1, w, d)], ptcs[ix(x+1, y+1, d-1, w, d)]);
			}
		}
	}

	void update(float dt) override {
		//update "campos"
		camPos=float3(
			cosf(camYaw)*sinf(camPitch),
			-cosf(camPitch),
			sinf(camYaw)*sinf(camPitch)
		)*5;

		//for switching outline "mode"
		bool oKey=getKey('O');
		if (oKey&&!oDown) {
			oDown=true;
			showOutline=!showOutline;
		}
		if (!oKey&&oDown) oDown=false;

		//moving
		float amt=Maths::PI/2;
		//slow
		if (getKey(VK_SHIFT)) amt/=5;

		if (getKey('A')) camYaw-=amt*dt;
		if (getKey('D')) camYaw+=amt*dt;
		if (getKey('W')) camPitch-=amt*dt;
		if (getKey('S')) camPitch+=amt*dt;
		camPitch=Maths::clamp(camPitch, -Maths::PI, 0);

		if (getKey('R')) setup();

		if (getKey(VK_UP)) camZoom+=15*dt;
		if (getKey(VK_DOWN)) camZoom-=15*dt;
		if (camZoom<1) camZoom=1;

		//softbody updating

		//update springs first
		for (int i=0; i<sprNum; i++) {
			sprs[i].update();
		}

		//add grav, update all particles
		for (int i=0; i<ptcNum; i++) {
			ptcs[i].applyForce(grav);
			ptcs[i].update(dt);
			ptcs[i].constrainIn(bounds);
		}

		grav.x=Maths::map(sinf(totalDeltaTime), -1, 1, 1, -1);
		grav.y=Maths::map(sinf(totalDeltaTime+0.3f), -1, 1, 4, -4);
		grav.z=Maths::map(sinf(totalDeltaTime-0.3f), -1, 1, 1, -1);
	}

	void draw(Raster& rst) override {
		//background
		rst.setChar(' ');
		rst.fillRect(0, 0, width, height);

		//draw bounds
		rst.setChar(0x2588);
		float nx=bounds.min.x, ny=bounds.min.y, nz=bounds.min.z;
		float xx=bounds.max.x, xy=bounds.max.y, xz=bounds.max.z;
		float2 nnn=projfloat3(float3(nx, ny, nz), camYaw, camPitch, camZoom)+ctr;
		float2 nnx=projfloat3(float3(nx, ny, xz), camYaw, camPitch, camZoom)+ctr;
		float2 nxn=projfloat3(float3(nx, xy, nz), camYaw, camPitch, camZoom)+ctr;
		float2 nxx=projfloat3(float3(nx, xy, xz), camYaw, camPitch, camZoom)+ctr;
		float2 xnn=projfloat3(float3(xx, ny, nz), camYaw, camPitch, camZoom)+ctr;
		float2 xnx=projfloat3(float3(xx, ny, xz), camYaw, camPitch, camZoom)+ctr;
		float2 xxn=projfloat3(float3(xx, xy, nz), camYaw, camPitch, camZoom)+ctr;
		float2 xxx=projfloat3(float3(xx, xy, xz), camYaw, camPitch, camZoom)+ctr;
		//top
		rst.drawLine(nnn, nnx);
		rst.drawLine(nnx, xnx);
		rst.drawLine(xnx, xnn);
		rst.drawLine(xnn, nnn);
		//verts
		rst.drawLine(nnn, nxn);
		rst.drawLine(nnx, nxx);
		rst.drawLine(xnx, xxx);
		rst.drawLine(xnn, xxn);
		//bottom
		rst.drawLine(nxn, nxx);
		rst.drawLine(nxx, xxx);
		rst.drawLine(xxx, xxn);
		rst.drawLine(xxn, nxn);

		//"optimization" show only front facing tris
		std::vector<tri> trisToDraw;
		for (int i=0; i<triNum; i++) {
			tri& t=tris[i];
			//culling
			if (dot(t.getNorm(), t.getA().pos-camPos)<0) {
				trisToDraw.push_back(t);
			}
		}

		//"painters"-ish algo to sort by what is closer to the "camera"
		sort(trisToDraw.begin(), trisToDraw.end(), [&](tri& a, tri& b) {
			return length(a.getAvgPos()-camPos)>length(b.getAvgPos()-camPos);
		});

		//"project" tris
		for (tri& t:trisToDraw) {
			float3 tPos=t.getAvgPos();
			float3 tNorm=t.getNorm();
			//diffuse is norm vs lightdir, "direct lighting"
			float diffuseShade=dot(tNorm, lightDir);

			//set tri brightness
			int asi=Maths::clamp(diffuseShade*7, 0, 6);
			rst.setChar(asciiArr[asi]);

			//get projected coords
			float2 a=projfloat3(t.getA().pos, camYaw, camPitch, camZoom)+ctr;
			float2 b=projfloat3(t.getB().pos, camYaw, camPitch, camZoom)+ctr;
			float2 c=projfloat3(t.getC().pos, camYaw, camPitch, camZoom)+ctr;
			rst.fillTriangle(a, b, c);

			if (showOutline) {
				rst.setChar(0x2588);
				rst.drawTriangle(a, b, c);
			}
		}

		//show fps
		setTitle("3D SoftBody Sim @ "+std::to_string((int)framesPerSecond)+"fps");
	}
};

int main() {
	srand(time(NULL));

	//init custom graphics engine
	Demo d=Demo();
	d.startWindowed(6, 90, 110);

	return 0;
}