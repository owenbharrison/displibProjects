#include <vector>

#include "Engine.h"
#include "maths/Maths.h"
#include "maths/vector/float3.h"
using namespace displib;

#define EPSILON 0.0001f

struct Ray {
	float3 origin, dir;

	Ray() {}

	Ray(float3 origin_, float3 dir_) {
		origin=origin_;
		dir=dir_;
	}
};

struct Hit {
	Ray ray;
	float dist=0;
	float3 pos, norm;
	short col=0x000F;
	bool reflective=false;

	Hit() {}

	Hit(Ray ray_, float dist_, float3 pos_, float3 norm_, short col_, bool reflective_) {
		ray=ray_;
		dist=dist_;
		pos=pos_;
		norm=norm_;
		col=col_;
		reflective=reflective_;
	}
};

struct Shape {
	short col=0x000F;
	bool reflective=false;

	Shape() {}

	virtual float intersectRay(Ray r) { return 0; }

	virtual bool getIntersection(Ray r, Hit& hitOut) { return false; }
};

struct Sphere : Shape {
	float3 pos;
	float rad=0;

	Sphere(float3 pos_, float rad_, short col_, bool reflective_) {
		pos=pos_;
		rad=rad_;
		col=col_;
		reflective=reflective_;
	}

	float intersectRay(Ray r) override {
		float3 oc=r.origin-pos;
		float a=dot(r.dir, r.dir);
		float b=2*dot(r.dir, oc);
		float c=dot(oc, oc)-rad*rad;
		float disc=b*b-4*a*c;
		if (disc<EPSILON) return -1;
		//solve quadratic, + & -
		float sq=sqrtf(disc);
		float num=-b-sq;
		if (num>EPSILON) return num/(2*a);
		num=-b+sq;
		if (num>EPSILON) return num/(2*a);
		return -1;
	}

	bool getIntersection(Ray r, Hit& hitOut) override {
		float dist=intersectRay(r);
		//invalid
		if (dist<0) return false;

		//use dist to march dir to hitpos
		float3 hitPos=r.origin+r.dir*dist;
		//to get norm of UNIFORM surface
		float3 hitNorm=normalize(hitPos-pos);
		hitOut=Hit(r, dist, hitPos, hitNorm, col, reflective);
		return true;
	}
};

struct Tri : Shape {
	float3 v[3];

	Tri(float3 a, float3 b, float3 c, short col_, bool reflective_) {
		v[0]=a;
		v[1]=b;
		v[2]=c;
		col=col_;
		reflective=reflective_;
	}

	float3 getNorm() {
		return normalize(cross(v[1]-v[0], v[2]-v[0]));
	}

	float intersectRay(Ray r) override {
		float3 e1=v[1]-v[0];
		float3 e2=v[2]-v[0];
		float3 h=cross(r.dir, e2);
		float n=dot(e1, h);
		//ray parallel
		if (n>-EPSILON&&n<EPSILON) return -1;
		float f=1/n;
		float3 s=r.origin-v[0];
		float u=f*dot(s, h);
		if (u<0||u>1) return -1;
		float3 q=cross(s, e1);
		float v_=f*dot(r.dir, q);
		if (v_<0||u+v_>1) return -1;
		float t_=f*dot(e2, q);
		if (t_>EPSILON) return t_;
		return -1;
	}

	bool getIntersection(Ray r, Hit& hitOut) override {
		float dist=intersectRay(r);
		//invalid
		if (dist<0) return false;

		//use dist to march dir to hitpos
		float3 hitPos=r.origin+r.dir*dist;
		hitOut=Hit(r, dist, hitPos, getNorm(), col, reflective);
		return true;
	}
};

class Demo : public Engine {
	public:
	std::vector<Shape*> shapes;

	float3 camPos, sunPos;
	float FOV;
	float camYaw, camPitch;
	int maxBounces=25;

	const char* asciiArr=".,~=#&@";

	void dirToUV(float3 dir, float& uOut, float& vOut) {
		uOut=0.5f+atan2f(dir.x, dir.z)/Maths::TAU;
		vOut=0.5f-asinf(dir.y)/Maths::PI;
	}

	//https://math.stackexchange.com/a/13263
	float3 reflectVec(float3 d, float3 n) {
		return normalize(d-n*dot(d, n)*2);
	}

	void setup() override {
		//plane ease of use functs
		auto planeXY=[](Tri*& a, Tri*& b, float3 p, float s, short c, bool r) {
			s/=2;
			float3 v0=float3(-s, -s, 0)+p;
			float3 v1=float3(-s, s, 0)+p;
			float3 v2=float3(s, -s, 0)+p;
			float3 v3=float3(s, s, 0)+p;
			a=new Tri(v0, v1, v2, c, r);
			b=new Tri(v1, v3, v2, c, r);
		};
		auto planeYZ=[](Tri*& a, Tri*& b, float3 p, float s, short c, bool r) {
			s/=2;
			float3 v0=float3(0, -s, -s)+p;
			float3 v1=float3(0, -s, s)+p;
			float3 v2=float3(0, s, -s)+p;
			float3 v3=float3(0, s, s)+p;
			a=new Tri(v0, v1, v2, c, r);
			b=new Tri(v1, v3, v2, c, r);
		};
		auto planeZX=[](Tri*& a, Tri*& b, float3 p, float s, short c, bool r) {
			s/=2;
			float3 v0=float3(-s, 0, -s)+p;
			float3 v1=float3(s, 0, -s)+p;
			float3 v2=float3(-s, 0, s)+p;
			float3 v3=float3(s, 0, s)+p;
			a=new Tri(v0, v1, v2, c, r);
			b=new Tri(v1, v3, v2, c, r);
		};

		//add sphere
		shapes.push_back(new Sphere(float3(-5, 0, 0), 2.2f, Raster::DARK_RED, false));

		//add tetrahedron
		float ttSz=1.7f;
		float3 vt(0, ttSz, 0);
		float3 vf(0, -ttSz, ttSz);
		float3 vbl(-ttSz, -ttSz, -ttSz);
		float3 vbr(ttSz, -ttSz, -ttSz);
		shapes.push_back(new Tri(vt, vf, vbr, Raster::GREEN, false));
		shapes.push_back(new Tri(vt, vbr, vbl, Raster::GREEN, false));
		shapes.push_back(new Tri(vt, vbl, vf, Raster::GREEN, false));
		shapes.push_back(new Tri(vf, vbl, vbr, Raster::GREEN, false));

		//groundplane
		Tri* gn[2];
		planeZX(gn[0], gn[1], float3(0, -4, 0), 8, Raster::MAGENTA, false);
		gn[1]->col=Raster::DARK_YELLOW;
		//add
		shapes.push_back(gn[0]); shapes.push_back(gn[1]);

		//side mirror
		Tri* mr[2];
		planeXY(mr[0], mr[1], float3(0, 0, 4), 8, Raster::WHITE, true);
		//add
		shapes.push_back(mr[0]); shapes.push_back(mr[1]);

		//blue cube
		Tri* cb[6][2];
		float sz=2;
		float3 cbCtr(5, 0, 0);
		planeZX(cb[0][0], cb[0][1], float3(0, sz/2, 0)+cbCtr, sz, Raster::DARK_CYAN, false);//front
		planeZX(cb[1][0], cb[1][1], float3(0, -sz/2, 0)+cbCtr, sz, Raster::DARK_CYAN, false);//back
		planeYZ(cb[2][0], cb[2][1], float3(sz/2, 0, 0)+cbCtr, sz, Raster::DARK_CYAN, false);//left
		planeYZ(cb[3][0], cb[3][1], float3(-sz/2, 0, 0)+cbCtr, sz, Raster::DARK_CYAN, false);//right
		planeXY(cb[4][0], cb[4][1], float3(0, 0, sz/2)+cbCtr, sz, Raster::DARK_CYAN, false);//top
		planeXY(cb[5][0], cb[5][1], float3(0, 0, -sz/2)+cbCtr, sz, Raster::DARK_CYAN, false);//bottom
		//add
		for (int f=0; f<6; f++) { shapes.push_back(cb[f][0]); shapes.push_back(cb[f][1]); }

		//initialize other stuff
		camPos=float3(0, 0, -5);
		sunPos=float3(0, 4, -1.2f);
		FOV=Maths::PI/2;
		camYaw=Maths::PI/2;
		camPitch=Maths::PI/2;
	}

	void update(float dt) override {
		//movement
		//move up down
		float speed=4.67f;
		if (getKey(VK_SPACE)) camPos.y+=speed*dt;
		if (getKey(VK_SHIFT)) camPos.y-=speed*dt;

		//move forward back
		float3 fbDir(cosf(camYaw), 0, sinf(camYaw));
		if (getKey('W')) camPos+=fbDir*speed*dt;
		if (getKey('S')) camPos-=fbDir*speed*dt;

		//move left right
		float3 lrDir(cosf(camYaw+Maths::PI/2), 0, sinf(camYaw+Maths::PI/2));
		if (getKey('A')) camPos+=lrDir*speed*dt;
		if (getKey('D')) camPos-=lrDir*speed*dt;

		//change view dir
		//look up down
		if (getKey(VK_UP)) camPitch-=dt;
		if (getKey(VK_DOWN)) camPitch+=dt;

		//look left right
		if (getKey(VK_LEFT)) camYaw+=dt;
		if (getKey(VK_RIGHT)) camYaw-=dt;

		//set sunpos if enter pressed
		if (getKey(VK_RETURN)) sunPos=camPos;

		camPitch=Maths::clamp(camPitch, EPSILON, Maths::PI-EPSILON);
	}

	bool traceRay(Ray rayToUse, std::vector<Shape*> shapeSetToUse, short& charOut, short& colorOut) {
		//sort
		Hit chosenHit;
		float record=INFINITY;
		bool shapeFound=false;
		for (Shape* s:shapeSetToUse) {//we love auto!
			Hit ix;
			if (s->getIntersection(rayToUse, ix)) {
				if (ix.dist<record) {
					record=ix.dist;
					chosenHit=ix;
					shapeFound=true;
				}
			}
		}

		//color pixel accordingly
		if (shapeFound) {
			//use closest shape
			colorOut=chosenHit.col;

			//diffuse shade
			float3 sunDir=normalize(sunPos-chosenHit.pos);
			//abs so tris are same from each side
			float diffuse=abs(dot(sunDir, chosenHit.norm));
			int asi=Maths::clamp(diffuse*7, 0, 6);
			charOut=diffuse<0?' ':asciiArr[asi];

			//shadows, find any shape inbetween the hit and the sun
			Ray shadowRay=Ray(chosenHit.pos, sunDir);
			for (Shape* s:shapes) {
				float dist=s->intersectRay(shadowRay);
				if (dist>EPSILON) {//is there an intersection?
					//so make the shadow dark!
					charOut=' ';
					return true;
				}
			}

			if (chosenHit.reflective) {
				traceRay(Ray(chosenHit.pos, reflectVec(rayToUse.dir, chosenHit.norm)), shapeSetToUse, charOut, colorOut);
			}
			return true;
		}
		else {
			//make some circular pattern for the rays that hit no shapes.
			float u, v;
			dirToUV(rayToUse.dir, u, v);

			int iu=u*26;
			int iv=v*20;
			charOut='a'+iu;

			bool checker=iu%2==iv%2;
			colorOut=checker?Raster::WHITE:Raster::DARK_GREY;
			return false;
		}
	}

	void draw(Raster& rst) override {
		//background
		rst.setChar(' ');
		rst.fillRect(0, 0, width, height);

		//https://en.wikipedia.org/wiki/Ray_tracing_(graphics)
		//calc viewport
		float3 vUp(0, 1, 0);
		float3 camDir=float3(//cam dir [3d polar to cartesian]
			cosf(camYaw)*sinf(camPitch),
			cosf(camPitch),
			sinf(camYaw)*sinf(camPitch)
		);
		float3 b=cross(vUp, camDir);
		float3 tn=normalize(camDir);
		float3 bn=normalize(b);
		float3 vn=cross(tn, bn);

		//calc viewport sizes
		float gx=tanf(FOV/2);
		float gy=gx*((height-1.0f)/(width-1.0f));

		//stepping vectors
		float3 qx=bn*(2*gx/(width-1));
		float3 qy=vn*(2*gy/(height-1));
		float3 p1m=tn-bn*gx-vn*gy;

		bool* hitGrid=new bool[width*height];
		for (int i=0, x=0; i<width; i++, x++) {
			//y flipped
			for (int j=0, y=height-1; j<height; j++, y--) {
				float3 pij=p1m+qx*i+qy*j;
				Ray ray=Ray(camPos, normalize(pij));

				//calculate color and symbol to use for this pixel
				short charToUse;
				short colorToUse;
				hitGrid[x+y*width]=traceRay(ray, shapes, charToUse, colorToUse);
				//set pixel
				rst.setChar(charToUse);
				rst.setColor(colorToUse);
				//and draw it
				rst.putPixel(x, y);
			}
		}

		//edge detection
		rst.setChar(0x2588);
		rst.setColor(Raster::WHITE);
		for (int x=0; x<width; x++) {
			for (int y=0; y<height; y++) {
				bool diff=false;
				bool curr=hitGrid[x+y*width];
				//"highlight" any differences between pixels
				if (x>1) diff|=(curr&&!hitGrid[x-1+y*width]);//left or
				if (y>1) diff|=(curr&&!hitGrid[x+y*width-width]);//up or
				if (x<width-2) diff|=(curr&&!hitGrid[x+1+y*width]);//right or
				if (y<height-2) diff|=(curr&&!hitGrid[x+y*width+width]);//down
				if (diff) rst.putPixel(x, y);
			}
		}
		delete[] hitGrid;

		//show fps
		rst.setChar(' ');
		rst.fillRect(0, 0, 16, 4);
		rst.setColor(Raster::WHITE);
		rst.drawString(0, 0, "FPS: "+std::to_string((int)framesPerSecond));
		rst.drawString(0, 1, "yaw: "+std::to_string(camYaw));
		rst.drawString(0, 2, "pitch: "+std::to_string(camPitch));
	}
};

int main() {
	//init custom graphics engine
	Demo d;
	d.startFullscreen(8);

	return 0;
}