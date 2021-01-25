#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include "model.h"

Model::Model(const char* filename) : verts_(), faces_(),normalmap_() {
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
			uvs_.push_back(vt);
			continue;
		}
		if (!line.compare(0, 2, "f ")) {
			std::vector<Vec2i> f;
			int itrash, idx, ivt;
			iss >> trash;
			while (iss >> idx >> trash >> ivt >> trash >> itrash) {
				idx--; // in wavefront obj all indices start at 1, not zero
				ivt--;
				f.push_back(Vec2i(idx,ivt));
			}
			faces_.push_back(f);
			continue;
		}
		if (!line.compare(0, 2, "vn"))
		{
			iss >> trash >> trash;
			Vec3f vn;
			for (int i = 0; i < 3; i++)  iss >> vn[i];
			normals_.push_back(vn);
			continue;
		}
	}
	std::cerr << "# v# " << verts_.size() << " f# " << faces_.size() << std::endl;
	load_texture(filename, "_nm.tga", normalmap_);
}

void Model::load_texture(std::string filename, const char* suffix, TGAImage& img) {
	std::string texfile(filename);
	size_t dot = texfile.find_last_of(".");
	if (dot != std::string::npos) {
		texfile = texfile.substr(0, dot) + std::string(suffix);
		std::cerr << "texture file " << texfile << " loading " << (img.read_tga_file(texfile.c_str()) ? "ok" : "failed") << std::endl;
		img.flip_vertically();
	}
}

Model::~Model() {
}

int Model::nverts() {
	return (int)verts_.size();
}
int Model::nvts() {
	return (int)uvs_.size();
}

int Model::nfaces() {
	return (int)faces_.size();
}

std::vector<Vec2i> Model::face(int idx) {
	return faces_[idx];
}
Vec3f Model::vert(int i) {
	return verts_[i];
}
Vec3f Model::vert(int iface, int nthvert)
{
	return this->verts_[face(iface)[nthvert].x];
}
Vec2f Model::uv(int i)
{
	return uvs_[i];
}

Vec2f Model::uv(int iface, int nthvert)
{
	return this->uvs_[face(iface)[nthvert].y];
}

Vec3f Model::normal(Vec2f uvf) {
	Vec2i uv(uvf[0] * normalmap_.get_width(), uvf[1] * normalmap_.get_height());
	TGAColor c = normalmap_.get(uv[0], uv[1]);
	Vec3f res;
	for (int i = 0; i < 3; i++)
		res[2 - i] = (float)c[i] / 255.f * 2.f - 1.f;
	return res;
}

Vec3f Model::normal(int i)
{
	return normals_[i];
}

Vec3f Model::normal(int iface, int nthvert)
{
	return this->normals_[face(iface)[nthvert].x].normalize();
}