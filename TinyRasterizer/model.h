#ifndef __MODEL_H__
#define __MODEL_H__

#include <vector>
#include "geometry.h"

class Model {
private:
	std::vector<Vec3f> verts_;
	std::vector<std::vector<int> > faces_;//������������(ÿ��3������ ÿ������1������)
	std::vector < std::vector<int>> vtfaces_;
	std::vector<Vec2f> vts_;
public:
	Model(const char *filename);
	~Model();
	int nverts();
	int nfaces();
	int nvtfaces();
	int nvts();
	Vec3f vert(int i);
	Vec2f vt(int i);
	std::vector<int> face(int idx);
	std::vector<int> vtface(int ivt);
};

#endif //__MODEL_H__
