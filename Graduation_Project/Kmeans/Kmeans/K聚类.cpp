#include <iostream>
#include <gdal_priv.h>
#include<cmath>
using namespace std;

struct tiffdata {
	int width = 0;
	int heght = 0;
	int bands = 0;
	double adf[6] = { 0 };
	const char* psz = NULL;
	GDALDataType eDataType;
};

unsigned short** readtiff(const char* file, tiffdata& datatiff)
{
	//注册驱动
	GDALAllRegister();
	//为了支持中文路径
	CPLSetConfigOption("GDAL_FILENAME_IS_UTF8", "NO");
	//打开影像
	GDALDataset* poIn = (GDALDataset*)GDALOpen(file, GA_ReadOnly);

	//获取影像大小
	int src_width = poIn->GetRasterXSize();
	int src_height = poIn->GetRasterYSize();
	cout << "影像的列：" << src_width << endl;
	cout << "影像的行：" << src_height << endl;

	//获取影像波段数
	int InBands = poIn->GetRasterCount();
	cout << "波段数：" << InBands << endl;
	//定义存储影像的空间参考数组
	double adfInGeoTransform[6] = { 0 };
	const char* pszWKT = NULL;
	//获取影像空间参考
	poIn->GetGeoTransform(adfInGeoTransform);
	pszWKT = poIn->GetProjectionRef();

	GDALDataType eDataType = poIn->GetRasterBand(1)->GetRasterDataType();
	cout << "数据类型：" << GDALGetDataTypeName(poIn->GetRasterBand(1)->GetRasterDataType()) << endl;
	cout << "数据类型：" << eDataType << endl;//1：8位的整形
	datatiff.width = src_width;
	datatiff.heght = src_height;
	datatiff.bands = InBands;
	datatiff.psz = pszWKT;
	datatiff.eDataType = eDataType;
	for (int i = 0; i < 6; i++)
	{
		datatiff.adf[i] = adfInGeoTransform[i];
	}
	GDALDriver* poDriver = (GDALDriver*)GDALGetDriverByName("GTiff");
	unsigned short** data = new unsigned short* [InBands];
	for (int i = 0; i < InBands; i++)
	{
		data[i] = new unsigned short[src_height * src_width];
		memset(data[i], 0, sizeof(unsigned short) * src_width * src_height);
		poIn->GetRasterBand(i + 1)->RasterIO(GF_Read, 0, 0, src_width, src_height, data[i], src_width, src_height, eDataType, 0, 0);
	}
	return data;
}

void randperm(vector<int>& temp, int Num, int width, int heght)

{

	for (int i = 0; i < Num; ++i)
	{

		int num = rand() % width * heght;

		for (int i = 0; i < temp.size(); i++)
		{
			if (temp[i] == num)
			{
				continue;
			}
		}
		temp.push_back(num);
	}


	for (int i = 0; i < temp.size(); i++)
	{
		cout << temp[i] << " ";
	}
	cout << endl;
}

double** inital_center(unsigned short** data, int k, int bands, int width, int heght)
{
	//step1 申请内存center[][]，最终返回结果，二维数组，类别中心，p行K列，行表示波段，列表是m*n
	double** kpcenter = new double* [k];
	for (int i = 0; i < bands; i++)
	{
		kpcenter[i] = new double[k];
		memset(kpcenter[i], 0, sizeof(double) * bands);
	}
	//step2 把影响文件image[][]里面的前K个像元，它们的波段值拷贝、赋值
	vector<int> temp;
	randperm(temp, k, width, heght);
	for (int i = 0; i < k; i++)
	{
		for (int j = 0; j < bands; j++)
		{
			kpcenter[i][j] = data[j][temp[i]];
		}

	}
	return kpcenter;
}
double* calculate_dist(unsigned short* pixel, double** center, int k, int bands)
{
	double* sign = new double[k];
	memset(sign, 0, sizeof(double) * k);

	for (int i = 0; i < k; i++)
	{
		double m = 0;
		for (int j = 0; j < bands; j++)
		{
			m = m + fabs((double)pixel[j] - (double)center[i][j]);
			cout << m << endl;
		}
		sign[i] = sqrt(m);
		m = 0;
	}
	return sign;
}
void dist_list(unsigned short** data, unsigned short* ciff, double** center, int k, int bands, int width, int heght, double** final)
{

	ciff = new unsigned short[width * heght];
	memset(ciff, 0, sizeof(unsigned short) * width * heght);

	int* num = new int[k];
	memset(num, 0, sizeof(int) * k);

	final = new double* [k];
	for (int i = 0; i < k; i++)
	{
		final[i] = new double[bands];
		memset(final, 0, sizeof(double) * bands);
	}
	

	unsigned short* pixel = new unsigned short[bands];
	for (int i = 0; i < width * heght; i++)
	{
		for (int j = 0; j < bands; j++)
		{
			pixel[j] = data[j][i];
		}
		double* sign = calculate_dist(pixel, center, k, bands);
		double min = 255;
		int mm = 0;
		for (int m = 0; m < k; m++)
		{
			if (min > sign[m])
			{
				min = sign[m];
				mm = m;
			}
		}
		ciff[i] = mm;
		for (int n = 0; n < bands; n++)
		{
			final[mm][n] = final[mm][n]+data[n][i];
		}
		num[mm]++;
	}
	for (int i = 0; i < k; i++)
	{
		for (int j = 0; j < bands; j++)
		{
			final[i][j] = final[i][j] / num[i];
		}
	}
}
int main()
{
	const char* f = "test_UTM.tif";
	tiffdata datatiff;
	unsigned short** data = readtiff(f, datatiff);
	double** center = inital_center(data, 3, datatiff.bands, datatiff.width, datatiff.heght);

	unsigned short* ciff;
	double** final;
	dist_list(data, ciff, center, 3, datatiff.bands, datatiff.width, datatiff.heght, final);
	
	
	return 0;
}