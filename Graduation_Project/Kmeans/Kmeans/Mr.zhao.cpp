#include <iostream>
#include <gdal_priv.h>
#include<cmath>
#include<ctime>
#include <iomanip>
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
	GDALClose(poIn);
}

//double** initail_center(unsigned short** image, int k, int p)//总体功能,不进入循环体的，不用太担心内存泄漏的问题
//{
//	step01 申请内存center[][]，最终返回结果，二维数组，类别中心，p行k列，行表示波段数，列表示某某类，第几类
//	double** center = (double**)malloc(sizeof(double*) * p);//p行k列，单实际上是开辟了p+1个一位数组
//	for (int i = 0; i < p; i++)
//	{
//		center[i] = (double*)malloc(sizeof(double) * k);
//		memset(center[i], 0, sizeof(double) * k);//每一行的元素值置零
//	}
//	step02 把影像文件image[][]里面的前k个像元，它们的波段值拷贝、复制给center[][]
//	for (int i = 0; i < p; i++)//i在image里表示第几个像元，i在center表示第几类
//	{
//		for (int j = 0; j < k; j++)//j表示第几个波段
//		{
//			center[i][j] = image[i][j];//赋值语句，就是把image对应元素赋值给center
//		}
//	}
//	return center;
//}
void randperm(vector<int>& temp, int Num, int width, int heght)
{
	srand((unsigned int)time(NULL));
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
	cout << "随机生成数：";
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
	for (int i = 0; i < k; i++)
	{
		kpcenter[i] = new double[bands];
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
	free(kpcenter);
}
//double** initail_center(unsigned short** image, int k, int p)//总体功能,不进入循环体的，不用太担心内存泄漏的问题
//{
//	//Step01 申请内存center[][]，最终返回结果，二维数组，类别中心，P行K列，行表示波段数，列表示某某类，第几类
//		double** center = (double**)malloc(sizeof(double*) * p);//p行k列，单实际上是开辟了p+1个一位数组
//	for (int i = 0; i < p; i++)
//	{
//		center[i] = (double*)malloc(sizeof(double) * k);
//		memset(center[i], 0, sizeof(double) * k);//每一行的元素值置零
//	}
//	//Step02 把影像文件image[][]里面的前k个像元，它们的波段值拷贝、复制给center[][]
//		for (int i = 0; i < p; i++)//i在image里表示第几个像元，i在center表示第几类
//		{
//			for (int j = 0; j < k; j++)//j表示第几个波段
//			{
//				center[i][j] = image[i][j];//赋值语句，就是把image对应元素赋值给center
//			}
//		}
//	return center;
//}

double dist(double* vect1, double* vect2, int p)//计算两个一位数组的距离，欧式距离
{
	double diff = 0, sum = 0, rt = 0;//都做了置零处理
	for (int i = 0; i < p; i++)
	{
		diff = vect1[i] - vect2[i];
		sum += pow(diff, 2);
	}
	rt = sqrt(sum);
	return rt;
}

void dist_vect(double* dist_M, double** center, double* pixel, int k, int p)//外部一定要事先创建好dist_M
{
	////step01 创建存储距离的数组dist_m
	//double* dist_m = (double*)malloc(sizeof(double) * k);
	//memset(dist_m, 0, sizeof(double) * k);//初始化以下，把dist_m每一个元素都归零

	//Step02 创建一个一位数组，临时存储center里某一列，某一类的类别中心
	double* temp_vect = (double*)malloc(sizeof(double) * p);
	memset(temp_vect, 0, sizeof(double) * p);//初始化以下，把dist_M每一个元素都归零

	//pixel到center里面每一个中心的距离，并存储到距离数组dist_M
	for (int i = 0; i < k; i++)//i在center里应该是表示列，i对应的也是dist_M里的索引
	{
		for (int j = 0; j < p; j++)
		{
			temp_vect[j] = center[i][j];//要把center里面的第i列，存放给temp_vect
		}
		dist_M[i] = dist(pixel, temp_vect, p);
	}
	free(temp_vect);
}

char min_label(double* dist_M, int k)
{
	double* min_V = dist_M;
	unsigned short* min_L = new unsigned short[k];
	for (int i = 0; i < k; i++)
	{
		min_L[i] = (unsigned short)i;
	}
	for (int i = 0; i < k - 1; i++)
	{
		//冒泡求最小
		for (int j = 0; j < k - i - 1; j++)
		{
			if (min_V[j] > min_V[j + 1])
			{
				double m = 0;
				m = min_V[j];
				min_V[j] = min_V[j + 1];
				min_V[j + 1] = m;
				char n = 0;
				n = min_L[j];
				min_L[j] = min_L[j + 1];
				min_L[j + 1] = n;
			}

		}
	}
	return min_L[0];
	free(min_L);
	free(min_V);
}

//创建了类别中心，给我一个像元，我能返回char变量，确定分类结果

void classify(unsigned short* class_rt, unsigned short** image, double** center, int m, int n, int p, int k)
//定义成char*，一定要记得，在循环体内，在迭代过程中，一定要释放掉
{
	//double* dist_M;//创建dist_M
	double* dist_M = (double*)malloc(sizeof(double) * k);
	memset(dist_M, 0, sizeof(double) * k);//初始化以下，把dist_M每一个元素都归零

	double* pixel = (double*)malloc(sizeof(double) * p);//创建pixel	
	memset(dist_M, 0, sizeof(double) * p);


	for (int i = 0; i < m * n; i++)//循环每一个像元
	{
		//把这个像元的波段值传给pixel
		for (int j = 0; j < p; j++)
		{
			pixel[j] = (double)image[j][i];
			/*cout << pixel[j] << "  ";*/
		}
		dist_vect(dist_M, center, pixel, k, p);//给我一个像元，给我类别中心，我去修改dist_m，存储像元到各类别中心的距离
		class_rt[i] = min_label(dist_M, k);
		//cout << class_rt[i] << "  ";
	}
	//如果在函数内，在堆区申请了内存，这段内存又没有用了，记得free掉
}

void fresh_center(double** new_center, unsigned short* class_rt, unsigned short** image, int k, int p, int m, int n)
{
	for (int i = 0; i < k; i++)
	{
		new_center[i] = new double[p];
		memset(new_center[i], 0, sizeof(double) * p);
	}
	int** kk = new int* [k];
	for (int i = 0; i < k; i++)
	{
		kk[i] = new int[p];
		memset(kk[i], 0, sizeof(int) * p);
	}
	int* count = new int[k];
	memset(count, 0, sizeof(int) * k);
	for (int i = 0; i < m * n; i++)
	{
		for (int j = 0; j < p; j++)
		{
			kk[class_rt[i]][j] += image[j][i];
		}
		count[class_rt[i]] += 1;

	}
	/*for (int i = 0; i < k; i++)
	{
		cout << count[i] << endl;
	}*/

	for (int i = 0; i < k; i++)
	{
		for (int j = 0; j < p; j++)
		{
			new_center[i][j] = (double)kk[i][j] / count[i];
			cout << new_center[i][j] << "  ";
		}
		cout << endl;
	}
	free(count);
}

bool infer_rt(double** new_center, double** old_center, int k, int p, double n)
{
	double* nember = new double[k];
	memset(nember, 0, sizeof(double) * k);
	cout << "新老中心差值：" << endl;
	for (int i = 0; i < k; i++)
	{
		for (int j = 0; j < p; j++)
		{
			nember[i] += fabs(new_center[i][j] - old_center[i][j]);
			cout << fabs(new_center[i][j] - old_center[i][j]) << "  ";
		}
		cout << endl;
		nember[i] = sqrt(nember[i]);
	}
	double sign = 0;
	for (int i = 0; i < k; i++)
	{
		sign += nember[i];
	}
	sign = sign / k;
	cout << sign << endl;
	if (sign <= n)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
	free(nember);
}

void ciff_color(unsigned short** color, int k)
{
	for (int i = 0; i < k; i++)
	{
		color[i] = new unsigned short[3];
		memset(color[i], 0, sizeof(unsigned short) * 3);
	}

	unsigned short max = 256;
	int n = 0;
	for (int m = 0; m < k / 6 + 1; m++)
	{
		if (m > 1)
		{
			max = max + max / (m + 1);
		}
		else
		{
			max = max / (m + 1);
		}

		for (int j = 0; j < 3; j++)
		{
			color[n][j] = max - 1;
			n++;
			if (n >= k)
			{
				break;
			}
		}
		if (n >= k)
		{
			break;
		}
		for (int i = 0; i < 3; i++)
		{
			color[n][i] = max - 1;
			if ((i + 1) == 3)
			{
				color[n][0] = max - 1;

			}
			else
			{
				color[n][i + 1] = max - 1;
			}
			n++;
			if (n >= k)
			{
				break;
			}
		}
		if (n >= k)
		{
			break;
		}

	}

}

void add_color(unsigned char** w_data, unsigned short** color, unsigned short* class_rt, int m, int n)
{
	for (int i = 0; i < 3; i++)
	{
		w_data[i] = new unsigned char[m * n];
		memset(w_data[i], 0, sizeof(unsigned char) * m * n);
	}
	for (int i = 0; i < m * n; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			w_data[j][i] = (unsigned char)color[class_rt[i]][j];
			//cout << w_data[j][i] << "  ";
		}
	}
}

void write_image(const char* file, tiffdata& datatiff, unsigned short* class_rt, int k)
{
	GDALDriver* podriver = (GDALDriver*)GDALGetDriverByName("GTiff");
	GDALDataset* po_outname = podriver->Create(file, datatiff.width, datatiff.heght, 3, GDT_Byte, NULL);
	po_outname->SetGeoTransform(datatiff.adf);
	po_outname->SetProjection(datatiff.psz);

	unsigned short** color = new unsigned short* [k];
	ciff_color(color, k);
	cout << "颜色类型：" << endl;
	for (int i = 0; i < k; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			cout << color[i][j] << " ";
		}
		cout << endl;
	}
	unsigned char** w_data = new unsigned char* [3];
	add_color(w_data, color, class_rt, datatiff.width, datatiff.heght);
	for (int i = 0; i < 3; i++)
	{
		po_outname->GetRasterBand(i + 1)->RasterIO(GF_Write, 0, 0, datatiff.width, datatiff.heght, w_data[i], datatiff.width, datatiff.heght, GDT_Byte, 0, 0);//数据的存储
	}

	GDALClose(po_outname);
}


int main()
{
	const char* f = "test01.tif";
	tiffdata datatiff;
	unsigned short** data = readtiff(f, datatiff);
	int k;
	double n;
	cout << "请输入分类数：";
	cin >> k;
	cout << "请输入阈值：";
	cin >> n;
	cout << endl;
	double** old_center = inital_center(data, k, datatiff.bands, datatiff.width, datatiff.heght);
	cout << "初始中心：" << endl;
	for (int i = 0; i < k; i++)
	{
		for (int j = 0; j < datatiff.bands; j++)
		{
			cout << old_center[i][j] << "   ";
		}
		cout << endl;
	}


	int count = 1;
	double** new_center = new double* [k];
	unsigned short* class_result = new unsigned short[datatiff.width * datatiff.heght];

	int sign = 1;
	cout << fixed << setprecision(5);
	while (true)
	{
		classify(class_result, data, old_center, datatiff.width, datatiff.heght, datatiff.bands, k);
		cout << "第" << sign << "次迭代中心：" << endl;
		fresh_center(new_center, class_result, data, k, datatiff.bands, datatiff.width, datatiff.heght);
		sign++;

		if (count > 50 || infer_rt(new_center, old_center, k, datatiff.bands, n) == true)
		{
			cout << "最后的中心：" << endl;
			for (int i = 0; i < k; i++)
			{
				for (int j = 0; j < datatiff.bands; j++)
				{
					cout << new_center[i][j] << "   ";
				}
				cout << endl;
			}
			break;
		}
		else {
			count++;
			for (int i = 0; i < k; i++)
			{
				for (int j = 0; j < datatiff.bands; j++)
				{
					old_center[i][j] = new_center[i][j];
				}
			}
		}
	}

	/*const char* file = "image//kmean.tif";
	write_image(file, datatiff, class_result, k);*/
	cout << "图像已在image文件中生成！！！" << endl;

	return 0;
}