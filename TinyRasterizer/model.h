#ifndef __MODEL_H__
#define __MODEL_H__

#include <vector>
#include "geometry.h"

class Model {
private:
	std::vector<Vec3f> verts_;//每个代表一个顶点坐标
	std::vector<Vec2f> vts_;//每个代表一个顶点纹理坐标
	std::vector<std::vector<int> > faces_;//每个代表一组（3个，一个面）顶点坐标索引
	std::vector < std::vector<int>> vtfaces_;//每个代表一组（3个，一个面）顶点纹理坐标索引
	std::vector<Vec3f> vns_; //vertix normal
public:
	Model(const char *filename);
	~Model();
	int nverts();
	int nfaces();
	int nvts();
	Vec3f vert(int i);
	Vec2f vt(int i);
	Vec3f vn(int i);
	std::vector<int> face(int idx);
	std::vector<int> vtface(int ivt);
	
};

#endif //__MODEL_H__
