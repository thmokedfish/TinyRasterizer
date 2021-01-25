//#include<old\tgaimage.h>
//#include<old\model.h>
//#include<old\geometry.h>
#include"tgaimage.h"
#include"model.h"
#include"geometry.h"
#include"my_gl.h"
#include<cmath>
#include<iostream>
/*
#include<boost\geometry\arithmetic\normalize.hpp>
#include<boost\qvm\vec_operations.hpp>
#include<boost\geometry\arithmetic\dot_product.hpp>
#include<boost\geometry\geometries\point.hpp>
#include<boost\geometry.hpp>
*/
using namespace std;

//namespace bg = boost::geometry;
//using pointf3 = bg::model::point<float, 3, bg::cs::cartesian>;

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);
Model* model = NULL;
const int width = 800;
const int height = 800;

//light
Vec3f light(0, 0, -1);
//bg::detail::vec_normalize(light);
struct GouraudShader :public IShader
{
public:
	Matrix uniform_pm;//用于变换的矩阵
	Matrix uniform_pm_it;//变换矩阵的逆转置矩阵(法线变换是逆转置)
	GouraudShader()
	{
		texture.read_tga_file("obj/african_head/african_head_diffuse.tga");
	}
	Vec4f vertex(int iface,int nthvert) override
	{
		Vec2f uv = model->uv(iface,nthvert);
		vert_uvs[0][nthvert] = uv.x;
		vert_uvs[1][nthvert] = uv.y;
		texture_coords[0][nthvert] = uv.x * texture.get_width();
		texture_coords[1][nthvert] = (1 - uv.y) * texture.get_height();


		Vec3f normal = model->normal(iface,nthvert);
		normal.normalize();
		lightIntense[nthvert] =normal * light;

		Vec4f world_coords = embed<4>(model->vert(iface, nthvert));
		Matrix m = Viewport * Projection * ModelView;
		//texture_coords[nthvert] = Vec2f(uv.x * texture.get_width(), (1 - uv.y) * texture.get_height());
		Vec4f screen_coords = m * world_coords;

		screen_coords = screen_coords / screen_coords[3];
		return screen_coords;
	}
	bool fragment(Vec3f bar, TGAColor& color) override
	{
		Vec2f uv = vert_uvs * bar;//像素的uv
		Vec3f normal = model->normal(uv);
		Vec3f trans_light=proj<3>( uniform_pm * embed<4>(light)).normalize();
		Vec3f trans_normal = proj<3>(uniform_pm_it * embed<4>(normal)).normalize();
		//float lightVolume = -(light*normal);
		float lightVolume =-(trans_light *trans_normal);
		//float lightVolume = -bar * lightIntense;
		//背面
		if (lightVolume < 0) { return true; }

		Vec2f texture_coord = texture_coords*bar ;
		color = texture.get(texture_coord.x, texture_coord.y);
		color =color* lightVolume;
		return false;
		
	}
private:
	//矩阵:用于乘重心坐标得出一个二维uv坐标
	//x1,x2,x3
	//y1,y2,y3
	mat<2, 3, float> texture_coords;
	TGAImage texture;
	Vec3f lightIntense;
	mat<2, 3, float> vert_uvs;
};

struct IntervalColoredShader :IShader
{
public:

	virtual Vec4f vertex(int iface, int nthvert)
	{
		Vec4f world_coords = embed<4>(model->vert(iface, nthvert));

		Vec3f normal = model->normal(iface,nthvert);
		normal.normalize();
		lightIntense[nthvert] = normal * light;

		Matrix m = Viewport * Projection * ModelView;
		Vec4f screen_coords = m * world_coords;
		screen_coords = screen_coords / screen_coords[3];
		return screen_coords;
	}
	virtual bool fragment(Vec3f bar, TGAColor& color)
	{
		float light = -(bar * lightIntense);
		if (light < 0) { return true; }
		color = TGAColor(0, int(light / 0.2)*50, 0);
		return false;
	}
private:
	Vec3f lightIntense;

};



int main() {
	model = new Model("obj/african_head/african_head.obj");
	const int texturewidth = 800, textureHeight = 800;
	TGAImage image(width, height, TGAImage::RGB);
	TGAImage texture;
	texture.read_tga_file("obj/african_head/african_head_diffuse.tga");

	light.normalize();


	int length = width * height;
	float* zbuffer = new float[length];
	for (int i = 0; i < length; ++i)
	{
		zbuffer[i] = numeric_limits<int>::min();
	}
	//camera position(0,0,c)
	//int c = 3;
	Vec3f camera = Vec3f(1.5, 1.5, 3);
	Vec3f center = Vec3f(0, 0, 0);
	float c = (camera - center).norm();
	cout <<"norm " << c << endl;
	/// <summary>
	/// 手算了一下 对于列向量（本次代码中的是列向量），后进行的矩阵变换靠左，先进行的矩阵变换靠右
	/// 例如：A*B*v,是先将向量v进行B变换，然后进行A变换
	/// </summary>
	/// <returns></returns>
	frame_convert(center, camera, Vec3f(0, 1, 0));
	projection(c);
	viewport(width, height);


	GouraudShader shader;
	shader.uniform_pm = Projection * ModelView;
	shader.uniform_pm_it = (Projection * ModelView).invert_transpose();


	//for each triangle
	for (int i = 0; i < model->nfaces(); i++) {
		Vec4f screen_coords[3];

		//calculate light volume
		
		

		//for each vertex
		for (int j = 0; j < 3; j++) {




			//convert into screen coordiate(screen width/height equivalents to image width/height)
			//x ranges 0~width,y ranges 0~height
			//screen_coords[j] = Vec3f((world_coords.x + 1.) * width / 2., (world_coords.y + 1.) * height / 2.,world_coords.z);
			screen_coords[j] = shader.vertex(i, j);
		}

		//here draws a triangle
		triangle(screen_coords,zbuffer, image,shader);
	}

	image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
	image.write_tga_file("output.tga");
	delete model;
	delete[] zbuffer;
	return 0;



}