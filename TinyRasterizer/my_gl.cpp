#include"my_gl.h"
#include<algorithm>
#include<cmath>
#include<iostream>

Matrix ModelView;
Matrix Viewport;
Matrix Projection;

IShader::~IShader(){}

//assume world coordinate ranges -1~1
//viewport matrix
void viewport(int width, int height)
{
	Matrix m = Matrix::identity();
	m[0][0] = m[0][3] = width / 2;
	m[1][1] = m[1][3] = height / 2;
	Viewport=m;
}
void projection(float cameraZ)
{
	Matrix m = Matrix::identity();
	m[3][2] = -1 / cameraZ;
	Projection=m;
}

//用这三个参数得出摄像机所在坐标系(原点center)的三个轴向量x,y,z
//求坐标系之间过渡矩阵的逆矩阵M
//要进行变换，需要点P减去center坐标之后乘M的逆矩阵
void frame_convert(Vec3f center, Vec3f camera, Vec3f up)
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
	ModelView= m * Tr;
}


//叉乘模长
inline float Edgefunction(Vec2i vec0, Vec2i vec1)
{
	return vec0[0] * vec1[1] - vec1[0] * vec0[1];
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
	float total = std::abs(e02p) + std::abs(e10p) + std::abs(e21p);
	if (Edgefunction(v02,v21)<0) { total *= -1; }
	//if (e02p >= 0 && e10p >= 0 && e21p >= 0) { std::cout << "ok "; }
	//if (e02p <= 0 && e10p <= 0 && e21p <= 0) { std::cout << "ok "; }
	//std::cout << e21p << " " << e02p << " " << e10p << std::endl;
	return Vec3f(e21p / total, e02p / total, e10p / total);


}
/*
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
*/
//vt:texture coordinates
void triangle(const Vec4f* vertex, float* zbuffer, TGAImage& image,  IShader& shader) {
	TGAColor color;
	Vec2f bboxmin(std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
	Vec2f bboxmax(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());
	Vec2i pos[3];
	for (int i = 0; i < 3; ++i)
	{
		pos[i] = Vec2i(vertex[i][0], vertex[i][1]);
	}
	for (int i = 0; i < 3; ++i)
	{
		for (int j = 0; j < 2; ++j)
		{
			//除以vertex最后一项意义不明
			bboxmin[j] = std::min(bboxmin[j], vertex[i][j]/vertex[i][3]);
			bboxmax[j] = std::max(bboxmax[j], vertex[i][j]/vertex[i][3]);
		}
	}
	for (int i = bboxmin.y; i < bboxmax.y; ++i)
	{
		if (i >= height || i < 0) { continue; }
		//每个像素
		for (int j = bboxmin.x; j < bboxmax.x; ++j)
		{
			if (j >= width || j < 0) { continue; }
			//重心坐标
			Vec3f bar = barycentric(pos[0], pos[1], pos[2], Vec2i(j, i));
			if (bar.x < 0 || bar.y < 0 || bar.z < 0) {  continue; }
			float zval = vertex[0][2]* bar[0] + vertex[1][2]* bar[1] + vertex[2][2] * bar[2];

			int index = j + i * width;
			if (zbuffer[index] >zval) { continue; }

			bool discard = shader.fragment(bar, color);
			if (!discard)
			{
				zbuffer[index] = zval;
				image.set(j, i, color);
			}

		}
	}
}