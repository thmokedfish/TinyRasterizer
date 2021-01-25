#pragma once
#include"tgaimage.h"
#include"geometry.h"

extern Matrix ModelView;
extern Matrix Viewport;
extern Matrix Projection;
extern const int width;
extern const int height;

struct IShader
{
	virtual ~IShader();
	//iface:face(三角形)的index，nthvert:三角形中顶点的index[0~2]
	virtual Vec4f vertex(int iface, int nthvert) = 0;
	//bar：重心坐标
	//return true则丢弃像素
	virtual bool fragment(Vec3f bar, TGAColor& color) = 0;
};

void viewport(int width, int height);
void projection(float cameraZ);
void frame_convert(Vec3f center, Vec3f camera, Vec3f up);
void triangle(const Vec4f* vertex, float* zbuffer, TGAImage& image,IShader& shader);