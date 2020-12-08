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
void triangle(Vec3f* vertex, float* zbuffer, TGAImage& image,TGAImage& texture,Vec2f* vt,float* vertexLightIntense) {
	Vec2i v[3];
	float z[3];
	
	int i = 0;
	for (; i < 3; ++i)
	{
		v[i].x = vertex[i].x;
		v[i].y = vertex[i].y;
		z[i] = vertex[i].z;
	}
	if (v[1].y > v[2].y) { swap(v[2], v[1]); swap(z[2], z[1]); swap(vt[2], vt[1]); swap(vertexLightIntense[2], vertexLightIntense[1]); }
	if (v[0].y > v[1].y) { swap(v[0], v[1]); swap(z[0], z[1]); swap(vt[0], vt[1]); swap(vertexLightIntense[0], vertexLightIntense[1]);}
	if (v[1].y > v[2].y) { swap(v[1], v[2]); swap(z[1], z[2]); swap(vt[2], vt[1]); swap(vertexLightIntense[2], vertexLightIntense[1]);}
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
			//重心坐标
			Vec3f barycentricCoo = barycentric(v[0], v[1], v[2], Vec2i(j, i));

			float lightVolume = vertexLightIntense[0] * barycentricCoo[0] +
				vertexLightIntense[1] * barycentricCoo[1] + vertexLightIntense[2] * barycentricCoo[2];
			lightVolume *= -1;
			if (lightVolume < 0) { continue; }
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
	
	//光照模型
	//light
	Vec3f light(0, 0, -1);
	light.normalize();
	//bg::detail::vec_normalize(light);
	
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

		//calculate light volume
		
		float lightIntense[3];
		

		//for each vertex
		for (int j = 0; j < 3; j++) {
			Vec3f world_coords = model->vert(face[j]);
			Vec2f uv = model->vt(vtface[j]);


			Vec3f normal = model->vn(face[j]);
			normal.normalize();
			lightIntense[j] = normal * light;


			//convert into screen coordiate(screen width/height equivalents to image width/height)
			//x ranges 0~width,y ranges 0~height
			//screen_coords[j] = Vec3f((world_coords.x + 1.) * width / 2., (world_coords.y + 1.) * height / 2.,world_coords.z);
			screen_coords[j] = m2v(m* v2m(world_coords));
			screen_texture_coords[j] = Vec2f(uv.x*texture.get_width(),(1-uv.y)*texture.get_height());
		}

		//here draws a triangle
		triangle(screen_coords,zbuffer, image,texture,screen_texture_coords,lightIntense);
	}

	image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
	image.write_tga_file("output.tga");
	delete model;
	delete[] zbuffer;
	return 0;



}