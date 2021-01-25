#ifndef __MODEL_H__
#define __MODEL_H__

#include <vector>
#include "geometry.h"
#include"tgaimage.h"

class Model {
private:
	std::vector<Vec3f> verts_;//每个代表一个顶点坐标
	std::vector<Vec2f> uvs_;//每个代表一个顶点纹理坐标
	std::vector<std::vector<Vec2i> > faces_;//每个代表一组（3个，一个面）顶点坐标索引/顶点纹理坐标索引
	std::vector<Vec3f> normals_; //vertix normal
	TGAImage normalmap_;
	TGAImage diffusemap_;
	TGAImage specularmap_;
public:
	void load_texture(std::string filename, const char* suffix, TGAImage& img);
	Model(const char *filename);
	~Model();
	int nverts();
	int nfaces();
	int nvts();
	Vec3f vert(int i);
	Vec3f vert(int iface, int nthvert);
	Vec2f uv(int i);
	Vec2f uv(int iface, int nthvert); 
	Vec3f normal(Vec2f uvf);
	Vec3f normal(int i);
	Vec3f normal(int iface, int nthvert);
	std::vector<Vec2i> face(int idx);
	
};

#endif //__MODEL_H__
