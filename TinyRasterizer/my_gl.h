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
	//iface:face(������)��index��nthvert:�������ж����index[0~2]
	virtual Vec4f vertex(int iface, int nthvert) = 0;
	//bar����������
	//return true��������
	virtual bool fragment(Vec3f bar, TGAColor& color) = 0;
};

void viewport(int width, int height);
void projection(float cameraZ);
void frame_convert(Vec3f center, Vec3f camera, Vec3f up);
void triangle(const Vec4f* vertex, float* zbuffer, TGAImage& image,IShader& shader);