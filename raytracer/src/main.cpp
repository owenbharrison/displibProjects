#include <vector>

#include "Engine.h"
#include "maths/Maths.h"
#include "shp/Sphere.h"
#include "shp/Tri.h"
using namespace displib;

class Demo : public Engine {
	public:
	std::vector<Shape*> shapes;

	V3D camPos, sunPos;
	float FOV;
	float camYaw, camPitch;
	int maxBounces=25;

	const char* asciiArr=" .,~=#&@";
	const int asciiLen=strlen(asciiArr);

	char asciiCharRamp(float a) {
		//pct between 0-1
		float pct=Maths::clamp(a, 0, 1);
		//index in bounds of asciiArr
		int asi=Maths::clamp(pct*asciiLen, 0, asciiLen-1);
		return asciiArr[asi];
	}

	void dirToUV(V3D& dir, float* uOut, float* vOut) {
		*uOut=0.5f+atan2f(dir.x, dir.z)/Maths::TAU;
		*vOut=0.5f-asinf(dir.y)/Maths::PI;
	}

	//https://math.stackexchange.com/a/13263
	V3D reflectVec(V3D d, V3D n) {
		return V3D::normal(d-n*d.dot(n)*2);
	}

	void setup() override {
		//add spheres
		V3D s1=V3D(-5, 0, 0);
		V3D s2=V3D(4, 0, 0);
		shapes.push_back(new Sphere(s1, 2.2f, Raster::RED, false));
		shapes.push_back(new Sphere(s2, 1.5f, Raster::BLUE, false));

		//add tetrahedron
		float ttSz=1.7f;
		V3D vt(0, ttSz, 0);
		V3D vf(0, -ttSz, ttSz);
		V3D vbl(-ttSz, -ttSz, -ttSz);
		V3D vbr(ttSz, -ttSz, -ttSz);
		shapes.push_back(new Tri(vt, vf, vbr, Raster::GREEN, false));
		shapes.push_back(new Tri(vt, vbr, vbl, Raster::GREEN, false));
		shapes.push_back(new Tri(vt, vbl, vf, Raster::GREEN, false));
		shapes.push_back(new Tri(vf, vbl, vbr, Raster::GREEN, false));

		//add groundPlane
		float sz=4;
		V3D gnn(-sz, -sz, -sz);
		V3D gnp(-sz, -sz, sz);
		V3D gpn(sz, -sz, -sz);
		V3D gpp(sz, -sz, sz);
		V3D gc(0, -sz, 0);
		shapes.push_back(new Tri(gnn, gc, gpn, Raster::DARK_YELLOW, false));
		shapes.push_back(new Tri(gpn, gc, gpp, Raster::DARK_MAGENTA, false));
		shapes.push_back(new Tri(gpp, gc, gnp, Raster::DARK_YELLOW, false));
		shapes.push_back(new Tri(gnp, gc, gnn, Raster::DARK_MAGENTA, false));

		//add side mirror
		V3D mtl(-sz, sz, sz);
		V3D mtr(sz, sz, sz);
		V3D mbl(-sz, -sz, sz);
		V3D mbr(sz, -sz, sz);
		V3D mc(0, 0, sz);
		shapes.push_back(new Tri(mtl, mtr, mc, Raster::WHITE, true));
		shapes.push_back(new Tri(mtr, mbr, mc, Raster::WHITE, true));
		shapes.push_back(new Tri(mbr, mbl, mc, Raster::WHITE, true));
		shapes.push_back(new Tri(mbl, mtl, mc, Raster::WHITE, true));

		//initialize other stuff
		camPos=V3D(0, 0, -5);
		sunPos=V3D(0, 5, 0);

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
		V3D fbDir(cosf(camYaw), 0, sinf(camYaw));
		if (getKey('W')) camPos+=fbDir*speed*dt;
		if (getKey('S')) camPos-=fbDir*speed*dt;

		//move left right
		V3D lrDir(cosf(camYaw+Maths::PI/2), 0, sinf(camYaw+Maths::PI/2));
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

		camPitch=Maths::clamp(camPitch, 0.0001f, Maths::PI-0.0001f);
	}

	bool traceRay(Ray rayToUse, std::vector<Shape*> shapeSetToUse, short& charOut, short& colorOut) {
		//sort
		Hit chosenHit;
		float record=INFINITY;
		bool shapeFound=false;
		for (auto& sptr:shapeSetToUse) {//we love auto!
			Hit ix;
			Shape& shp=*sptr;
			if (shp.getIntersection(rayToUse, ix)) {
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
			V3D sunDir=V3D::normal(sunPos-chosenHit.pos);
			float diffuse=sunDir.dot(chosenHit.norm);
			charOut=diffuse<0?' ':asciiCharRamp(diffuse);

			//shadows, find any shape inbetween the hit and the sun
			Ray shadowRay=Ray(chosenHit.pos, sunDir);
			for (auto& sptr:shapes) {
				float dist=(*sptr).intersectRay(shadowRay);
				if (dist!=-1) {//is there an intersection?
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
			dirToUV(rayToUse.dir, &u, &v);
			charOut=48+u*10;
			colorOut=Raster::DARK_GREY;
			return false;
		}
	}

	void draw(Raster& rst) override {
		//background
		rst.setChar(' ');
		rst.fillRect(0, 0, width, height);

		//https://en.wikipedia.org/wiki/Ray_tracing_(graphics)
		//calc viewport
		V3D vUp(0, 1, 0);
		V3D t=V3D(//cam dir [3d polar to cartesian]
			cosf(camYaw)*sinf(camPitch),
			cosf(camPitch),
			sinf(camYaw)*sinf(camPitch)
		);
		V3D b=vUp.cross(t);
		V3D tn=V3D::normal(t);
		V3D bn=V3D::normal(b);
		V3D vn=tn.cross(bn);

		//calc viewport sizes
		float gx=tanf(FOV/2);
		float gy=gx*((height-1.0f)/(width-1.0f));

		//stepping vectors
		V3D qx=bn*(2*gx/(width-1));
		V3D qy=vn*(2*gy/(height-1));
		V3D p1m=tn-bn*gx-vn*gy;

		bool* hitGrid=new bool[width*height];
		for (int i=0, x=0; i<width; i++, x++) {
			//y flipped
			for (int j=0, y=height-1; j<height; j++, y--) {
				V3D pij=p1m+qx*i+qy*j;
				Ray ray=Ray(camPos, V3D::normal(pij));

				//STARTCUT
				short charToUse;
				short colorToUse;
				hitGrid[x+y*width]=traceRay(ray, shapes, charToUse, colorToUse);
				rst.setChar(charToUse);
				rst.setColor(colorToUse);
				//ENDCUT
				rst.putPixel(x, y);
			}
		}

		//edge detection
		rst.setChar('s');
		rst.setColor(Raster::WHITE);
		for (int x=0; x<width; x++) {
			for (int y=0; y<height; y++) {
				bool diff=false;
				int curr=hitGrid[x+y*width];
				//"highlight" any differences between pixels
				if (x>1) diff|=(curr!=hitGrid[x-1+y*width]);//left or
				if (y>1) diff|=(curr!=hitGrid[x+y*width-width]);//up or
				if (x<width-2) diff|=(curr!=hitGrid[x+1+y*width]);//right or
				if (y<height-2) diff|=(curr!=hitGrid[x+y*width+width]);//down
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
	Demo d=Demo();
	d.start(10, 10, true);

	return 0;
}