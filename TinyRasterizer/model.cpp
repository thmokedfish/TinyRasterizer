#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include "model.h"

Model::Model(const char* filename) : verts_(), faces_(), vtfaces_() {
	std::ifstream in;
	in.open(filename, std::ifstream::in);
	if (in.fail()) return;
	std::string line;
	while (!in.eof()) {
		std::getline(in, line);
		std::istringstream iss(line.c_str());
		char trash;
		if (!line.compare(0, 2, "v ")) {
			iss >> trash;
			Vec3f v;
			for (int i = 0; i < 3; i++)  iss >> v[i];  
			verts_.push_back(v);
			continue;
		}
		if(!line.compare(0,2,"vt"))
		{
			iss >> trash>>trash;
			Vec2f vt;
			for (int i = 0; i < 2; i++)  iss >> vt[i]; 
			vts_.push_back(vt);
			continue;
		}
		if (!line.compare(0, 2, "f ")) {
			std::vector<int> f;
			std::vector<int> t;
			int itrash, idx, ivt;
			iss >> trash;
			while (iss >> idx >> trash >> ivt >> trash >> itrash) {
				idx--; // in wavefront obj all indices start at 1, not zero
				ivt--;
				f.push_back(idx);
				t.push_back(ivt);
			}
			faces_.push_back(f);
			vtfaces_.push_back(t);
			continue;
		}
		if (!line.compare(0, 2, "vn"))
		{
			iss >> trash >> trash;
			Vec3f vn;
			for (int i = 0; i < 3; i++)  iss >> vn[i];
			vns_.push_back(vn);
			continue;
		}
	}
	std::cerr << "# v# " << verts_.size() << " f# " << faces_.size() << std::endl;
}

Model::~Model() {
}

int Model::nverts() {
	return (int)verts_.size();
}
int Model::nvts() {
	return (int)vts_.size();
}

int Model::nfaces() {
	return (int)faces_.size();
}

std::vector<int> Model::face(int idx) {
	return faces_[idx];
}
std::vector<int> Model::vtface(int ivt)
{
	return vtfaces_[ivt];
}

Vec3f Model::vert(int i) {
	return verts_[i];
}
Vec2f Model::vt(int i)
{
	return vts_[i];
}

Vec3f Model::vn(int i)
{
	return vns_[i];
}