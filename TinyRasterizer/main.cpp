//#include<old\tgaimage.h>
//#include<old\model.h>
//#include<old\geometry.h>
#include"tgaimage.h"
#include"model.h"
#include"geometry.h"
#include<cmath>
#include<vector>
#include<algorithm>
#include<iostream>
#include<boost\geometry\arithmetic\normalize.hpp>
#include<boost\qvm\vec_operations.hpp>
#include<boost\geometry\arithmetic\dot_product.hpp>
#include<boost\geometry\geometries\point.hpp>
#include<boost\geometry.hpp>
using namespace std;

namespace bg = boost::geometry;
using pointf3 = bg::model::point<float, 3, bg::cs::cartesian>;

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);
Model* model = NULL;
const int width = 800;
const int height = 800;

void line(int x0, int y0, int x1, int y1, TGAImage& image, const TGAColor& color) {
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
void line(Vec2f t0, Vec2f t1, TGAImage& image, const TGAColor& color) {
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

void triangleOutline(Vec2f t0, Vec2f t1, Vec2f t2, TGAImage& image, const TGAColor& color)
{
	line(t0, t1, image, color);
	line(t1, t2, image, color);
	line(t2, t0, image, color);

}

void oldtriangle(Vec3i t0, Vec3i t1, Vec3i t2, float* zbuffer,TGAImage& image, const TGAColor& color)
{
	if (t1.y > t2.y) { swap(t2, t1); }
	if (t0.y > t1.y) { swap(t0, t1); }
	if (t1.y > t2.y) { swap(t1, t2); }
	if (t0.y == t2.y) { return; }
	//1/斜率
	float s12, s01;
	s12 = t2.y != t1.y ? ((float)(t2.x - t1.x) / (t2.y - t1.y)) : 0;
	s01 = t0.y != t1.y ? ((float)(t1.x - t0.x) / (t1.y - t0.y)) : 0;
	float ttx = t2.x - t0.x; float tty = t2.y - t0.y;
	float s02 = (float)(t2.x - t0.x) / (t2.y - t0.y);
	int x02, x1;
	for (int i = t0.y; i <= t2.y; ++i)
	{
		x02 = t0.x + (i - t0.y) * s02;
		x1 = i > t1.y ? (t1.x + (i - t1.y) * s12) : (t0.x + (i - t0.y) * s01);

		if (x02 > x1) { swap(x02, x1); }
		for (int j = x02; j < x1; ++j)
		{
			image.set(j, i, color);
		}
	}
}

//叉乘模长
inline float Edgefunction(Vec2i vec0, Vec2i vec1)
{
	return abs(vec0[0] * vec1[1] - vec1[0] * vec0[1]);
}
//return:barycentric coordinate
Vec3f barycentric(Vec2i p0, Vec2i p1, Vec2i p2, Vec2i point)
{
	Vec2i v10 = p0 - p1;
	Vec2i v21 = p1 - p2;
	Vec2i v02 = p2 - p0;
	Vec2i v0p = point - p0;
	Vec2i v1p = point - p1;
	Vec2i v2p = point - p2;
	float e02p = Edgefunction(v02, v0p);
	float e10p = Edgefunction(v10, v1p);
	float e21p = Edgefunction(v21, v2p);
	float total = e02p + e10p + e21p;
	return Vec3f(e21p / total, e02p / total, e10p / total);
	
}

Matrix v2m(Vec3f v)
{
	Matrix m = Matrix::identity();
	m[0][0] = v.x;
	m[1][0] = v.y;
	m[2][0] = v.z;
	m[3][0] = 1.f;
	return m;
}

Vec3f m2v(Matrix m)
{
	return Vec3f(m[0][0]/m[3][0], m[1][0]/m[3][0], m[2][0]/m[3][0]);
}

//vt:texture coordinates
void triangle(Vec3f* vertex, float* zbuffer, TGAImage& image,TGAImage& texture,Vec2f* vt, float lightVolume) {
	Vec2i v[3];
	float z[3];
	int i = 0;
	for (; i < 3; ++i)
	{
		v[i].x = vertex[i].x;
		v[i].y = vertex[i].y;
		z[i] = vertex[i].z;
	}
	if (v[1].y > v[2].y) { swap(v[2], v[1]); swap(z[2], z[1]); swap(vt[2], vt[1]); }
	if (v[0].y > v[1].y) { swap(v[0], v[1]); swap(z[0], z[1]); swap(vt[0], vt[1]);}
	if (v[1].y > v[2].y) { swap(v[1], v[2]); swap(z[1], z[2]); swap(vt[2], vt[1]);}
	if (v[0].y == v[2].y) { return; }
	//1/斜率
	float s12, s01;
	s12 = v[2].y != v[1].y ? ((float)(v[2].x - v[1].x) / (v[2].y - v[1].y)) : 0;
	s01 = v[0].y != v[1].y ? ((float)(v[1].x - v[0].x) / (v[1].y - v[0].y)) : 0;
	float ttx = v[2].x - v[0].x; float tty = v[2].y - v[0].y;
	float s02 = (float)(v[2].x - v[0].x) / (v[2].y - v[0].y);
	int x02, x1;

	for (int i = v[0].y; i <= v[2].y; ++i)
	{
		if (i >= height||i<0 ) { continue; }
		x02 = v[0].x + (i - v[0].y) * s02;
		x1 = i > v[1].y ? (v[1].x + (i - v[1].y) * s12) : (v[0].x + (i - v[0].y) * s01);

		if (x02 > x1) { swap(x02, x1); }
		//每个像素
		for (int j = x02; j < x1; ++j)
		{
			if (j >= width||j<0 ) { continue; }
			int index = j + i * width;
			Vec3f barycentricCoo = barycentric(v[0], v[1], v[2], Vec2i(j, i));
			float zval = z[0] * barycentricCoo[0] + z[1] * barycentricCoo[1] + z[2] * barycentricCoo[2];
			if (zbuffer[index] < zval)
			{
				zbuffer[index] = zval;

				float x_texture_coord = 0;
				float y_texture_coord = 0;
				for (int t = 0; t < 3; ++t)
				{
					x_texture_coord += barycentricCoo[t] * vt[t].x;
					y_texture_coord += barycentricCoo[t] * vt[t].y;
				}
				TGAColor color=texture.get(x_texture_coord, y_texture_coord);
				color = color * lightVolume;
				image.set(j, i,color);
			}
		}
	}
}
//assume world coordinate ranges -1~1
Matrix world2screen(int width,int height)
{
	Matrix m = Matrix::identity();
	m[0][0] = m[0][3] =width / 2;
	m[1][1] = m[1][3] = height / 2;
	return m;
}

Matrix OrthogonalConvert(float cameraZ)
{
	Matrix m = Matrix::identity();
	m[3][2] = -1 / cameraZ;
	return m;
}


int main() {
	/*
	//三角形
	TGAImage image(width, height, TGAImage::RGB);
	float* zbuffer = new float[width * height];
	Vec3f t0[3] = { Vec3f(550,600,12), Vec3f(50,400,10),Vec3f(750,450,11) };
	triangle(t0,zbuffer, image, red);
	image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
	image.write_tga_file("triangle.tga");
	return 0;
	*/
	
	//光照模型
	//light
	pointf3 light(0, 0, -1);
	bg::detail::vec_normalize(light);
	
	int length = width * height;
	float* zbuffer = new float[length];
	for (int i =0;i<length;++i)
	{
		zbuffer[i] = numeric_limits<int>::min();
	}
	model = new Model("obj/african_head/african_head.obj");
	const int texturewidth = 800, textureHeight = 800;
	TGAImage image(width, height, TGAImage::RGB);
	TGAImage texture;
	texture.read_tga_file("obj/african_head/african_head_diffuse.tga");



	//camera position(0,0,c)
	int c = 3;
	/// <summary>
	/// 手算了一下 对于列向量（本次代码中的是列向量），后进行的矩阵变换靠左，先进行的矩阵变换靠右
	/// 例如：A*B*v,是先将向量v进行B变换，然后进行A变换
	/// </summary>
	/// <returns></returns>
	Matrix m = world2screen(width, height)*OrthogonalConvert(c);

	//for each triangle
	for (int i = 0; i < model->nfaces(); i++) {
		std::vector<int> face = model->face(i);
		std::vector<int> vtface = model->vtface(i);
		Vec3f screen_coords[3];
		Vec2f screen_texture_coords[3];
		float lightVolume;

		//calculate light volume
		Vec3f diff1 = model->vert(face[0]) - model->vert(face[1]);
		Vec3f diff2 = model->vert(face[2]) - model->vert(face[1]);

		pointf3 d1(diff1.x, diff1.y, diff1.z);
		pointf3 d2(diff2.x, diff2.y, diff2.z);
		pointf3 cross = bg::cross_product(d1, d2);
		bg::detail::vec_normalize(cross);

		lightVolume = bg::dot_product(cross, light);//0~1
		//not lighted
		if (lightVolume < 0) { continue; }
		int lv = lightVolume * 255;

		

		//for each vertex
		for (int j = 0; j < 3; j++) {
			Vec3f world_coords = model->vert(face[j]);
			Vec2f uv = model->vt(vtface[j]);


			//translate screen coords into orthogonal projection
		//	for (int j = 0; j < 3; ++j)
			{
		//		world_coords.x /= (1 - world_coords.z / c);
		//		world_coords.y /= (1 - world_coords.z / c);
			}

			//convert into screen coordiate(screen width/height equivalents to image width/height)
			//x ranges 0~width,y ranges 0~height
			//screen_coords[j] = Vec3f((world_coords.x + 1.) * width / 2., (world_coords.y + 1.) * height / 2.,world_coords.z);
			screen_coords[j] = m2v(m* v2m(world_coords));
			screen_texture_coords[j] = Vec2f(uv.x*texture.get_width(),(1-uv.y)*texture.get_height());
		}

		//here draws a triangle
		triangle(screen_coords,zbuffer, image,texture,screen_texture_coords,lightVolume);
	}

	image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
	image.write_tga_file("output.tga");
	delete model;
	delete[] zbuffer;
	return 0;

	











	/*
	线框模型
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
		image.write_tga_file("output2.tga");
		delete model;
		return 0;
	}
	*/

}