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
void triangle(const Vec3f* vertex, float* zbuffer, TGAImage& image,TGAImage& texture,const Vec2f* texture_coords,const float* vertexLightIntense) {
	Vec2i v[3];
	struct vertexInfo
	{
		Vec2i pos;
		float zbuff;
		Vec2f vt;
		float lightVol;
	};

	vertexInfo infos[3];
	
	for (int i=0; i < 3; ++i)
	{
		infos[i].pos.x = vertex[i].x;
		infos[i].pos.y = vertex[i].y;
		infos[i].zbuff = vertex[i].z;
		infos[i].vt = texture_coords[i];
		infos[i].lightVol = vertexLightIntense[i];
	}
	if (infos[1].pos.y > infos[2].pos.y) { swap(infos[2], infos[1]);  }
	if (infos[0].pos.y > infos[1].pos.y) { swap(infos[0],infos[1]);}
	if (infos[1].pos.y > infos[2].pos.y) { swap(infos[1],infos[2]);}
	if (infos[0].pos.y == infos[2].pos.y) { return; }
	//1/斜率
	float s12, s01;
	s12 = infos[2].pos.y != infos[1].pos.y ? ((float)(infos[2].pos.x - infos[1].pos.x) / (infos[2].pos.y - infos[1].pos.y)) : 0;
	s01 = infos[0].pos.y != infos[1].pos.y ? ((float)(infos[1].pos.x - infos[0].pos.x) / (infos[1].pos.y - infos[0].pos.y)) : 0;
	float ttx = infos[2].pos.x - infos[0].pos.x; float tty = infos[2].pos.y - infos[0].pos.y;
	float s02 = (float)(infos[2].pos.x - infos[0].pos.x) / (infos[2].pos.y - infos[0].pos.y);
	int x02, x1;

	for (int i = infos[0].pos.y; i <= infos[2].pos.y; ++i)
	{
		if (i >= height||i<0 ) { continue; }
		x02 = infos[0].pos.x + (i - infos[0].pos.y) * s02;
		x1 = i > infos[1].pos.y ? (infos[1].pos.x + (i - infos[1].pos.y) * s12) : (infos[0].pos.x + (i - infos[0].pos.y) * s01);

		if (x02 > x1) { swap(x02, x1); }
		//每个像素
		for (int j = x02; j < x1; ++j)
		{
			if (j >= width||j<0 ) { continue; }
			int index = j + i * width;
			//重心坐标
			Vec3f barycentricCoo = barycentric(infos[0].pos, infos[1].pos, infos[2].pos, Vec2i(j, i));

			float lightVolume = infos[0].lightVol * barycentricCoo[0] +
				infos[1].lightVol * barycentricCoo[1] + infos[2].lightVol * barycentricCoo[2];
			lightVolume *= -1;
			if (lightVolume < 0) { continue; }
			float zval = infos[0].zbuff * barycentricCoo[0] + infos[1].zbuff * barycentricCoo[1] + infos[2].zbuff * barycentricCoo[2];
			if (zbuffer[index] < zval)
			{
				zbuffer[index] = zval;

				float x_texture_coord = 0;
				float y_texture_coord = 0;
				for (int t = 0; t < 3; ++t)
				{
					x_texture_coord += barycentricCoo[t] * infos[t].vt.x;
					y_texture_coord += barycentricCoo[t] * infos[t].vt.y;
				}
				TGAColor color=texture.get(x_texture_coord, y_texture_coord);
				color = color * lightVolume;
				image.set(j, i,color);
			}
		}
	}
}
//assume world coordinate ranges -1~1
//viewport matrix
Matrix world2screen(int width,int height)
{
	Matrix m = Matrix::identity();
	m[0][0] = m[0][3] =width / 2;
	m[1][1] = m[1][3] = height / 2;
	return m;
}

Matrix projection_matrix(float cameraZ)
{
	Matrix m = Matrix::identity();
	m[3][2] = -1 / cameraZ;
	return m;
}

//用这三个参数得出摄像机所在坐标系(原点center)的三个轴向量x,y,z
//求坐标系之间过渡矩阵的逆矩阵M
//要进行变换，需要点P减去center坐标之后乘M的逆矩阵
Matrix frame_convert(Vec3f center, Vec3f camera, Vec3f up)
{
	Vec3f z = (camera - center).normalize();
	Vec3f x = cross(up, z).normalize();
	Vec3f y = cross(z, x).normalize();
	Matrix m = Matrix::identity();
	/*
	* 从原坐标系到相机坐标系的过渡矩阵:
	* x.x y.x z.x 0
	* x.y y.y z.y 0
	* x.z y.z z.z 0
	*  0   0   0  1
	* 过渡矩阵满足正交矩阵 所以逆矩阵=转置矩阵 下面直接构造逆的
	*/
	Matrix Tr = Matrix::identity();
	for (int i = 0; i < 3; ++i)
	{
		m[0][i] = x[i];
		m[1][i] = y[i];
		m[2][i] = z[i];
		Tr[i][3] = -center[i];//矩阵Tr代表"P减去center坐标"这一变换
	}
	return m * Tr;
}


int main() {
	
	model = new Model("obj/african_head/african_head.obj");
	const int texturewidth = 800, textureHeight = 800;
	TGAImage image(width, height, TGAImage::RGB);
	TGAImage texture;
	texture.read_tga_file("obj/african_head/african_head_diffuse.tga");



	//light
	Vec3f light(0, 0, -1);
	light.normalize();
	//bg::detail::vec_normalize(light);

	int length = width * height;
	float* zbuffer = new float[length];
	for (int i = 0; i < length; ++i)
	{
		zbuffer[i] = numeric_limits<int>::min();
	}
	//camera position(0,0,c)
	//int c = 3;
	Vec3f camera = Vec3f(-3, -3, 5);
	Vec3f center = Vec3f(-1, -1, 0);
	float c = (camera - center).norm();
	cout <<"norm " << c << endl;
	/// <summary>
	/// 手算了一下 对于列向量（本次代码中的是列向量），后进行的矩阵变换靠左，先进行的矩阵变换靠右
	/// 例如：A*B*v,是先将向量v进行B变换，然后进行A变换
	/// </summary>
	/// <returns></returns>
	Matrix m = world2screen(width, height)*projection_matrix(c)*frame_convert(center,camera,Vec3f(0,1,0));

	//for each triangle
	for (int i = 0; i < model->nfaces(); i++) {
		std::vector<int> face = model->face(i);
		std::vector<int> vtface = model->vtface(i);
		Vec3f screen_coords[3];
		Vec2f texture_coords[3];

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
			texture_coords[j] = Vec2f(uv.x*texture.get_width(),(1-uv.y)*texture.get_height());
		}

		//here draws a triangle
		triangle(screen_coords,zbuffer, image,texture,texture_coords,lightIntense);
	}

	image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
	image.write_tga_file("output.tga");
	delete model;
	delete[] zbuffer;
	return 0;



}