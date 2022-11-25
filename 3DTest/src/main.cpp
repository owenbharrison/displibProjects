#include <vector>
#include <strstream> 
#include <fstream>
#include <regex>
#include <algorithm>

#include "Engine.h"
#include "maths/Maths.h"
#include "maths/vector/float3.h"
using namespace displib;

static std::string FILENAME;

//i really havent a clue where i got this
//i believe it is x and z matrix rotations, without z
float2 projfloat3(float3 v, float yaw, float pitch, float zoom) {
	return float2(
		sinf(yaw)*v.x-cosf(yaw)*v.z,
		(cosf(yaw)*v.x+sinf(yaw)*v.z)*cosf(pitch)+sinf(pitch)*v.y
	)*zoom;
}

//i didnt want to use a vector of arrays so, this is the "bodge"
struct triIndex { int a, b, c; };
struct tri {
	float3 a, b, c;

	float3 getNorm() {
		return normalize(cross(b-a, c-a));
	}

	float3 getAvgPos() {
		return (a+b+c)/3;
	}
};

struct mesh {
	std::vector<tri> tris;

	//load obj file
	bool loadFromFile(std::string filename) {
		std::ifstream file(filename);
		std::regex indexRegex("([0-9]+)[/0-9]*");
		if (!file.is_open()) return false;

		std::vector<float3> vtxs;
		std::vector<triIndex> tIxs;

		//get extreme points
		float nx=INFINITY, ny=INFINITY, nz=INFINITY;
		float mx=-INFINITY, my=-INFINITY, mz=-INFINITY;
		//go through each line of the file
		while (!file.eof()) {
			std::string line;
			getline(file, line);
			std::strstream stream;
			stream<<line;
			char junk;

			//if the line is in vertex format
			if (line.find("v ")!=std::string::npos) {
				float x, y, z;
				stream>>junk>>x>>y>>z;
				nx=min(nx, x); ny=min(ny, y); nz=min(nz, z);
				mx=max(mx, x); my=max(my, y); mz=max(mz, z);
				vtxs.push_back(float3(x, y, z));
			}

			//if the line is in face format
			if (line.find("f ")!=std::string::npos) {
				//find all indexes
				std::vector<int> indexes;
				for (auto iter=std::sregex_iterator(line.begin(), line.end(), indexRegex); iter!=std::sregex_iterator(); iter++) {
					std::smatch match;
					match=*iter;
					indexes.push_back(stoi(match.str(1))-1);
				}
				//simple triangulation of face
				for (int i=1; i<indexes.size()-1; i++) {
					tIxs.push_back({indexes[0], indexes[i], indexes[i+1]});
				}
			}
		}

		//center vtxs about (0, 0, 0)
		float3 modelMid((nx+mx)/2, (ny+my)/2, (nz+mz)/2);
		for (auto& v:vtxs) v-=modelMid;

		//get biggest dimension, in one dir (so on 2)
		float maxDim=max(mx-nx, max(my-ny, mz-nz))/2;
		//divide by that, "normalize"?
		for (auto& v:vtxs) v/=maxDim;

		//use real tri ix info
		for (auto& tIx:tIxs) tris.push_back({vtxs[tIx.a], vtxs[tIx.b], vtxs[tIx.c]});
		return true;
	}
};

class Demo : public Engine {
	public:
	float2 ctr;
	mesh mainMesh;
	float3 lightPos, camPos;

	bool showOutline=true, showNorm=false;
	bool oDown=false, nDown=false;

	float camYaw=-0.983478f, camPitch=-1.922638f;
	float camZoom;

	const char* asciiArr=" .,~=#&@";

	void setup() override {
		mainMesh.loadFromFile(FILENAME);

		ctr=float2(width/2, height/2);

		lightPos=float3(60, 40, 90);

		camZoom=height/2.8f;
	}

	void update(float dt) override {
		//update "campos"
		camPos=float3(
			cosf(camYaw)*sinf(camPitch),
			-cosf(camPitch),
			sinf(camYaw)*sinf(camPitch)
		)*120;

		//for switching outline "mode"
		bool oKey=getKey('O');
		if (oKey&&!oDown) {
			oDown=true;
			showOutline=!showOutline;
		}
		if (!oKey&&oDown) oDown=false;

		//for switching norm "mode"
		bool nKey=getKey('N');
		if (nKey&&!nDown) {
			nDown=true;
			showNorm=!showNorm;
		}
		if (!nKey&&nDown) nDown=false;

		//set lighting
		if (getKey('L')) lightPos=camPos;

		//slow
		float amt=Maths::PI/2;
		if (getKey(VK_SHIFT)) amt/=5;
		//look dir
		if (getKey('W')) camPitch+=amt*dt;
		if (getKey('S')) camPitch-=amt*dt;
		if (getKey('A')) camYaw+=amt*dt;
		if (getKey('D')) camYaw-=amt*dt;
		camPitch=Maths::clamp(camPitch, -Maths::PI, 0);
		//zoom
		if (getKey(VK_UP)) camZoom+=25*dt;
		if (getKey(VK_DOWN)) camZoom-=25*dt;
		if (camZoom<1) camZoom=1;

		//update title
		std::string triCt=std::to_string(mainMesh.tris.size())+"tris";
		std::string fpsStr=std::to_string((int)framesPerSecond)+"fps";
		setTitle("3D Testing ["+FILENAME+"] with "+triCt+" @ "+fpsStr);
	}

	void draw(Raster& rst) override {
		//background
		rst.setChar(' ');
		rst.fillRect(0, 0, width, height);

		//"optimization" show only front facing tris
		std::vector<tri> trisToDraw;
		for (auto& t:mainMesh.tris) {
			//culling
			if (dot(t.getNorm(), t.a-camPos)<0) {
				trisToDraw.push_back(t);
			}
		}

		//"painters"-ish algo to sort by what is closer to the "camera"
		sort(trisToDraw.begin(), trisToDraw.end(), [&](tri& a, tri& b) {
			return length(a.getAvgPos()-camPos)>length(b.getAvgPos()-camPos);
		});

		//"project" tris
		for (auto& t:trisToDraw) {
			float3 tPos=t.getAvgPos();
			float3 tNorm=t.getNorm();
			//diffuse is norm vs lightdir, "direct lighting"
			float3 dirToLight=normalize(lightPos-tPos);
			float diffuseShade=dot(tNorm, dirToLight);

			//set tri brightness
			int asi=Maths::clamp(diffuseShade*8, 0, 7);
			rst.setChar(asciiArr[asi]);

			//get projected coords
			float2 a=projfloat3(t.a, camYaw, camPitch, camZoom)+ctr;
			float2 b=projfloat3(t.b, camYaw, camPitch, camZoom)+ctr;
			float2 c=projfloat3(t.c, camYaw, camPitch, camZoom)+ctr;
			rst.fillTriangle(a, b, c);

			//show wireframe
			rst.setChar(0x2588);
			if (showOutline) {
				rst.drawTriangle(a, b, c);
			}

			//show tri normals
			if (showNorm) {
				float2 mid=projfloat3(tPos, camYaw, camPitch, camZoom)+ctr;
				float2 midEx=projfloat3(tPos+tNorm/25, camYaw, camPitch, camZoom)+ctr;
				rst.drawLine(mid, midEx);
			}
		}

		//show stats
		rst.drawString(0, 0, "camYaw[LR]: "+std::to_string(camYaw));
		rst.drawString(0, 1, "camPitch[UD]: "+std::to_string(camPitch));
		rst.drawString(0, 2, "camZoom[FB]: "+std::to_string(camZoom));
	}
};

int main() {
	Demo d=Demo();

	//init file chooser
	OPENFILENAME ofn;
	char szFile[1024];
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize=sizeof(ofn);
	ofn.hwndOwner=d.getWindowHandle();
	ofn.lpstrFile=(LPWSTR)szFile;
	ofn.lpstrFile[0]='\0';
	ofn.nMaxFile=sizeof(szFile);
	ofn.lpstrFilter=L"Wavefront (.obj)\0*.OBJ\0";
	ofn.nFilterIndex=1;
	ofn.lpstrFileTitle=NULL;
	ofn.nMaxFileTitle=0;
	ofn.lpstrInitialDir=NULL;
	ofn.Flags=OFN_PATHMUSTEXIST|OFN_FILEMUSTEXIST;

	//if user chooses file, init engine
	if (GetOpenFileName(&ofn)) {
		std::wstring wstr(ofn.lpstrFile);
		FILENAME=std::string(wstr.begin(), wstr.end());

		d.startWindowed(6, 240, 135);
	}

	return 0;
}