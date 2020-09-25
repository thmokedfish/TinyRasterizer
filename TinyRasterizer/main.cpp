#include<old\tgaimage.h>
#include<old\model.h>
#include<old\geometry.h>
#include<cmath>
#include<vector>
#include<algorithm>
#include<iostream>
using namespace std;
const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);
Model* model = NULL;
const int width = 800;
const int height = 800;

void line(int x0, int y0, int x1, int y1, TGAImage& image,const TGAColor& color) {
	bool steep = false;
	if (std::abs(x0 - x1) < std::abs(y0 - y1)) {
		std::swap(x0, y0);
		std::swap(x1, y1);
		steep = true;
	}
	if (x0 > x1) {
		std::swap(x0, x1);
		std::swap(y0, y1);
	}
	int dx = x1 - x0;
	int dy = y1 - y0;
	int derror2 = std::abs(dy) * 2;
	int error2 = 0;
	int y = y0;
	for (int x = x0; x <= x1; x++) {
		if (steep) {
			image.set(y, x, color);
		}
		else {
			image.set(x, y, color);
		}
		error2 += derror2;
		if (error2 > dx) {
			y += (y1 > y0 ? 1 : -1);
			error2 -= dx * 2;
		}
	}
}
void line( Vec2i t0, Vec2i t1, TGAImage& image, const TGAColor& color) {
	bool steep = false;
	if (std::abs(t0.x - t1.x) < std::abs(t0.y - t1.y)) {
		std::swap(t0.x, t0.y);
		std::swap(t1.x, t1.y);
		steep = true;
	}
	if (t0.x > t1.x) {
		std::swap(t0, t1);
	}
	int dx = t1.x - t0.x;
	int dy = t1.y - t0.y;
	int derror2 = std::abs(dy) * 2;
	int error2 = 0;
	int y = t0.y;
	for (int x = t0.x; x <= t1.x; x++) {
		if (steep) {
			image.set(y, x, color);
		}
		else {
			image.set(x, y, color);
		}
		error2 += derror2;
		if (error2 > dx) {
			y += (t1.y > t0.y ? 1 : -1);
			error2 -= dx * 2;
		}
	}
}

void triangleLine(Vec2i t0, Vec2i t1, Vec2i t2, TGAImage& image,const TGAColor& color)
{
	line(t0, t1, image, color);
	line(t1, t2, image, color);
	line(t2, t0, image, color);

}

void triangle(Vec2i t0, Vec2i t1, Vec2i t2, TGAImage& image, const TGAColor& color)
{
	if (t1.y > t2.y) { swap(t2, t1); }
	if (t0.y > t1.y) { swap(t0, t1); }
	if (t1.y > t2.y) { swap(t0, t2); }
	//Ð±ÂÊ
	float s12 = (t2.x - t1.x) / (t2.y - t1.y);
	float s01 = (t1.x - t0.x) / (t1.y - t0.y);
	float s02 = (t2.x - t0.x) / (t2.y - t0.y);

	int x02,x1;
	for (int i = t0.y; i <= t2.y; ++i)
	{
		x02 = t0.x + (i - t0.y) * s02;
		x1 = i > t1.y ? (t1.x+(i-t1.y)*s12):(t0.x + (i - t0.y) * s01);
		if (x02 > x1) { swap(x02, x1); }
		for (int j = x02; j < x1; ++j)
		{
			image.set(j, i, color);
		}
	}

}

int main() {
	model = new Model("F:/CppProjects/Rendering/tinyrenderer-master/include/obj/african_head/african_head.obj");

	TGAImage image(width, height, TGAImage::RGB);
	Vec2i t0[3] = { Vec2i(50,350),Vec2i(350,400),Vec2i(250,300) };
	triangle(t0[0], t0[1], t0[2], image, red);

	image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
	image.write_tga_file("output.tga");
	delete model;
	return 0;
	/*
	model = new Model("F:/CppProjects/Rendering/tinyrenderer-master/include/obj/african_head/african_head.obj");
	
	const TGAColor& rwhite = white;
	TGAImage image(width, height, TGAImage::RGB);
	for (int i = 0; i < model->nfaces(); i++) {
		std::vector<int> face = model->face(i);
		for (int j = 0; j < 3; j++) {
			Vec3f v0 = model->vert(face[j]);
			Vec3f v1 = model->vert(face[(j + 1) % 3]);
			int x0 = (v0.x + 1.) * width / 2.;
			int y0 = (v0.y + 1.) * height / 2.;
			int x1 = (v1.x + 1.) * width / 2.;
			int y1 = (v1.y + 1.) * height / 2.;
			line(x0, y0, x1, y1, image, white);
		}
	}

	image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
	image.write_tga_file("output.tga");
	delete model;
	return 0;
	*/
	
}