#ifndef __MODEL_H__
#define __MODEL_H__

#include <vector>
#include "geometry.h"

class Model {
private:
	std::vector<Vec3f> verts_;//ÿ������һ����������
	std::vector<Vec2f> vts_;//ÿ������һ��������������
	std::vector<std::vector<int> > faces_;//ÿ������һ�飨3����һ���棩������������
	std::vector < std::vector<int>> vtfaces_;//ÿ������һ�飨3����һ���棩����������������
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
