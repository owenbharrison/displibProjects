#include <vector>

#include "Engine.h"
#include "geom/AABB2D.h"
#include "maths/Maths.h"
using namespace displib;

float2 rayRayIntersect(float2 a, float2 b, float2 c, float2 d) {
	float denom=(a.x-b.x)*(c.y-d.y)-(a.y-b.y)*(c.x-d.x);
	return float2(
		(a.x*b.y-a.y*b.x)*(c.x-d.x)-(a.x-b.x)*(c.x*d.y-c.y*d.x),
		(a.x*b.y-a.y*b.x)*(c.y-d.y)-(a.y-b.y)*(c.x*d.y-c.y*d.x)
	)/denom;
}

struct Edge {
	float2 a, b;

	Edge() {}

	Edge(float2 a_, float2 b_) {
		a=a_;
		b=b_;
	}

	//same edge or flipped
	static bool isSimilar(Edge a, Edge b) {
		bool norm=a.a==b.a&&a.b==b.b;
		bool flip=a.a==b.b&&a.b==b.a;
		//cant both be true
		return norm!=flip;
	}
};

struct Circle {
	float2 pos;
	float rad=0;

	Circle() {}

	Circle(float2 pos_, float rad_) {
		pos=pos_;
		rad=rad_;
	}

	bool containsPt(float2 point) {
		//is point closer than rad? if so inside!
		float dist=length(point-pos);
		return dist<rad;
	}
};

struct Triangle {
	Edge edges[3];

	Triangle() {}

	Triangle(float2 a, float2 b, float2 c) {
		edges[0]=Edge(a, b);
		edges[1]=Edge(b, c);
		edges[2]=Edge(c, a);
	}

	Triangle(Edge e, float2 v) {
		edges[0]=e;
		edges[1]=Edge(e.b, v);
		edges[2]=Edge(v, e.a);
	}

	static bool similar(Triangle t0, Triangle t1) {
		//consider tri abc, and tri def.
		float2 a=t0.edges[0].a, b=t0.edges[1].a, c=t0.edges[2].a;
		float2 d=t1.edges[0].a, e=t1.edges[1].a, f=t1.edges[2].a;

		//abc is normal
		bool norm0=a==d&&b==e&&c==f;//abc==def?
		bool norm1=a==e&&b==f&&c==d;//abc==efd?
		bool norm2=a==f&&b==d&&c==e;//abc==fde?
		bool norm=norm0||norm1||norm2;

		//abc is flipped, or cba
		bool flip0=c==d&&b==e&&a==f;//cba==def?
		bool flip1=c==e&&b==f&&a==d;//cba==efd?
		bool flip2=c==f&&b==d&&a==e;//cba==fde?
		bool flip=flip0||flip1||flip2;

		return norm!=flip;
	}

	Circle getCircumCircle() {
		//find normals and middles of two edges
		float2 mid0=(edges[0].a+edges[0].b)/2;
		float2 tang0=edges[0].b-edges[0].a;
		float2 norm0(-tang0.y, tang0.x);
		norm0=normalize(norm0);

		float2 mid1=(edges[1].a+edges[1].b)/2;
		float2 tang1=edges[1].b-edges[1].a;
		float2 norm1(-tang1.y, tang1.x);
		norm1=normalize(norm1);

		//the center is the ray intersect of the middle of the edges.
		float2 circPos=rayRayIntersect(mid0, mid0+norm0, mid1, mid1+norm1);
		//and rad is dist to any pt.
		float circRad=length(edges[0].a-circPos);

		return Circle(circPos, circRad);
	}
};

/*at this point, it wont return a triangulation that makes
a convex hull of all the points, but whatever.*/
std::vector<Triangle> bowyerWatson(std::vector<float2>& pointList) {
	//pointList is a set of coordinates defining the points to be triangulated
	std::vector<Triangle> triangulation;

	//find bounding box for super triangle
	float nx=INFINITY, ny=INFINITY, mx=-INFINITY, my=-INFINITY;
	for (float2& point:pointList) {
		nx=min(nx, point.x);
		ny=min(ny, point.y);
		mx=max(mx, point.x);
		my=max(my, point.y);
	}
	float w=mx-nx, h=my-ny;
	//must be large enough to completely contain all the points in pointList (with some margin)
	Triangle superTri(float2(nx+w/2, ny-h-1), float2(mx+w/2+1, my+1), float2(ny-w/2-1, my+1));
	triangulation.push_back(superTri);

	//add all the points one at a time to the triangulation
	for (float2& point:pointList) {
		std::vector<Triangle> badTriangles;
		// first find all the triangles that are no longer valid due to the insertion
		for (Triangle& triangle:triangulation) {
			//if point is inside circumcircle of triangle
			if (triangle.getCircumCircle().containsPt(point)) {
				badTriangles.push_back(triangle);
			}
		}

		//find the boundary of the polygonal hole
		std::vector<Edge> polygon;
		for (Triangle& triangle:badTriangles) {
			for (int te=0; te<3; te++) {
				Edge& triangleEdge=triangle.edges[te];
				//if edge is not shared by any OTHER triangles in badTriangles...
				bool shared=false;
				for (Triangle& otherTriangle:badTriangles) {
					//dont check self
					if (&triangle!=&otherTriangle) {
						for (int ote=0; ote<3; ote++) {
							Edge& otherTriangleEdge=otherTriangle.edges[ote];
							if (Edge::isSimilar(triangleEdge, otherTriangleEdge)) {
								shared=true;
							}
						}
					}
				}
				if (!shared) {
					//...then add edge to polygon!
					polygon.push_back(triangleEdge);
				}
			}
		}

		//remove them from the data structure
		for (int t=triangulation.size()-1; t>=0; t--) {
			Triangle& triangle=triangulation.at(t);
			//if similar to any badTriangle, remove it
			bool similar=false;
			for (Triangle& badTriangle:badTriangles) {
				if (Triangle::similar(triangle, badTriangle)) {
					triangulation.erase(triangulation.begin()+t);
				}
			}
		}

		//re-triangulate the polygonal hole
		for (Edge& edge:polygon) {
			triangulation.push_back(Triangle(edge, point));
		}
	}

	// done inserting points, now clean up
	for (int t=triangulation.size()-1; t>=0; t--) {
		Triangle& triangle=triangulation.at(t);
		//if triangle contains A vertex from original super-triangle...
		bool shared=false;
		for (int te=0; te<3; te++) {
			Edge& triangleEdge=triangle.edges[te];
			for (int se=0; se<3; se++) {
				Edge& superEdge=superTri.edges[se];
				if (triangleEdge.a==superEdge.a) {
					//if ANY vertex similar
					shared=true;
				}
			}
		}
		if (shared) {
			//...remove triangle from triangulation
			triangulation.erase(triangulation.begin()+t);
		}
	}

	return triangulation;
}

struct Ball {
	float2 pos, vel;

	Ball() {}

	Ball(float2 pos_, float2 vel_) {
		pos=pos_;
		vel=vel_;
	}

	void update(float dt) {
		pos+=vel*dt;
	}

	void checkAABB(AABB2D a) {
		//constrain and bounce off walls
		if (pos.x<a.min.x) { pos.x=a.min.x; vel.x*=-1; }
		if (pos.y<a.min.y) { pos.y=a.min.y; vel.y*=-1; }
		if (pos.x>a.max.x) { pos.x=a.max.x; vel.x*=-1; }
		if (pos.y>a.max.y) { pos.y=a.max.y; vel.y*=-1; }
	}
};

class Demo : public Engine {
	public:
	AABB2D bounds;
	std::vector<Ball> balls;

	void setup() override {
		float bfr=3.2f;
		bounds=AABB2D(bfr, bfr, width-bfr, height-bfr);

		int num=Maths::random(35, 50);
		for (int i=0; i<num; i++) addRandomBall();
	}

	void addRandomBall() {
		//random x, y
		float x=Maths::random(bounds.min.x, bounds.max.x);
		float y=Maths::random(bounds.min.y, bounds.max.y);
		float2 pos(x, y);

		//random direction with polar to cartesian
		float angle=Maths::random(-Maths::PI, Maths::PI);
		float speed=Maths::random(5, 18);
		float2 vel=float2(cosf(angle), sinf(angle))*speed;

		balls.push_back(Ball(pos, vel));
	}

	void update(float dt) override {
		for (Ball& b:balls) {
			b.update(dt);

			b.checkAABB(bounds);
		}

		setTitle("Delaunay Triangulation @ "+std::to_string((int)framesPerSecond)+"fps");
	}

	void draw(Raster& rst) override {
		rst.setChar(0x2588);
		rst.setColor(Raster::DARK_GREY);
		rst.fillRect(0, 0, width, height);

		std::vector<float2> points;
		for (Ball& b:balls) points.push_back(b.pos);

		std::vector<Triangle> delaunayTriangulation=bowyerWatson(points);
		for (Triangle& t:delaunayTriangulation) {
			float2 a=t.edges[0].a,
				b=t.edges[1].a,
				c=t.edges[2].a;
			rst.setColor(Raster::GREEN);
			rst.drawTriangle(a, b, c);
		}

		rst.setColor(Raster::BLUE);
		for (Ball& b:balls) rst.fillCircle(b.pos, 2);
	}
};

int main() {
	srand(time(NULL));

	//init custom graphics engine
	Demo d;
	d.startWindowed(3, 320, 200);

	return 0;
}