// EncryptBase.cpp: 定义 DLL 应用程序的导出函数。
//
//
#include "pch.h"
#include "myDll.h"  
#include "framework.h"  

#include <opencv2/opencv.hpp>
#include <opencv2/ml.hpp>
#include<iostream>
#include <gdal_priv.h>
#include<fstream> 
#include<vector>
#include<string>
#include<sstream>
using namespace std;
using namespace cv;
using namespace cv::ml;
#pragma once
#define DLL_API extern "C" _declspec(dllexport)


static struct allsample {
	vector<int>sample_lable;
	vector<vector<int> >sample_m;
};

static struct tiffdata {
	int width = 0;
	int heght = 0;
	int bands = 0;
	double adf[6] = { 0 };
	const char* psz = NULL;
	GDALDataType eDataType;
};
static struct tiffdata1 {
	int width = 0;
	int heght = 0;
	int bands = 0;
};
tiffdata1 datatiff1;


DLL_API int width()
{
	return datatiff1.width;
}
DLL_API int height()
{
	return datatiff1.heght;
}

DLL_API unsigned short** readtiff(const char* file, tiffdata& datatiff)
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

DLL_API void get_lable(vector<int>& sample_lable, vector<vector<int> >& sample)
{
	for (int i = 0; i < sample.size(); i++)
	{
		sample_lable.push_back(sample[i][0]);
	}
}

DLL_API void get_m(vector<vector<int> >& sample_m, vector<vector<int> >& sample)
{
	vector<int> temp;
	for (int i = 0; i < sample.size(); i++)
	{
		for (int j = 1; j < sample[0].size(); j++)
		{
			temp.push_back(sample[i][j]);
		}
		sample_m.push_back(temp);
		temp.clear();
	}
}

DLL_API void get_sample(allsample& data, const char* file)
{
	vector<vector<int> >sample;
	ifstream fs;
	string s;
	fs.open(file);
	if (fs)
	{
		cout << "打开成功！" << endl;
	}
	vector<int>temp;
	while (getline(fs, s))
	{
		istringstream is(s);
		int d;
		while (!is.eof())
		{
			is >> d;
			temp.push_back(d);
		}
		sample.push_back(temp);
		temp.clear();
		s.clear();
	}
	get_lable(data.sample_lable, sample);
	get_m(data.sample_m, sample);
	sample.clear();
}

DLL_API void ciff_color(unsigned short** color, int k)
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

DLL_API void add_color(unsigned char** w_data, unsigned short** color, unsigned short* class_rt, int m, int n)
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

DLL_API void write_image(const char* file, tiffdata& datatiff, unsigned short* class_rt, int k)
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

DLL_API void Ssvm()
{
	const char* f = "D:\\KF\\vs\\Workplace\\svm测试版\\svm测试版\\data\\test01.tif";
	tiffdata datatiff;
	unsigned short** data = readtiff(f, datatiff);
	Mat DataMat(datatiff.width * datatiff.heght, datatiff.bands, CV_32FC1);
	for (int i = 0; i < datatiff.width * datatiff.heght; i++)
	{
		for (int j = 0; j < datatiff.bands; j++)
		{
			DataMat.at<float>(i, j) = (float)data[j][i];
			//cout << data[0][0] << "  ";

		}
		//cout << endl;
	}
	//cout << trainingDataMat << endl;

	const char* samplefile = "D:\\KF\\vs\\Workplace\\svm测试版\\svm测试版\\data\\yangben.txt";
	allsample sample;
	get_sample(sample, samplefile);
	int m = sample.sample_m.size();
	int n = sample.sample_m[0].size();
	//cout << n << endl;

	int* lable = new int[m];
	memset(lable, 0, sizeof(int) * m);
	copy(sample.sample_lable.begin(), sample.sample_lable.end(), lable);
	Mat labelsMat(m, 1, CV_32SC1, lable);

	Mat trainingDataMat(m, n, CV_32FC1);
	for (int i = 0; i < m; i++)
	{
		for (int j = 0; j < n; j++)
		{
			trainingDataMat.at<float>(i, j) = sample.sample_m[i][j];

		}
	}
	//cout << trainingDataMat << endl;

	Ptr<SVM> svm = SVM::create();//创建一个svm对象
	svm->setType(SVM::C_SVC); //设置SVM公式类型
	svm->setKernel(SVM::LINEAR);//设置SVM核函数类型
	svm->setTermCriteria(TermCriteria(TermCriteria::MAX_ITER, 100, 1e-6));//设置SVM训练时迭代终止条件
	svm->train(trainingDataMat, ROW_SAMPLE, labelsMat);//训练数据
	//svm->save("out/svm.xml");
	//cout << svm << endl;

	unsigned short* class_result = new unsigned short[datatiff.width * datatiff.heght];

	for (int i = 0; i < datatiff.width * datatiff.heght; i++)
	{
		int response = svm->predict(DataMat.row(i));  //进行预测，返回1或-1
		class_result[i] = response;
	}

	const char* file = "D:\\KF\\vs\\Workplace\\svm测试版\\svm测试版\\out\\支持向量机.tif";
	write_image(file, datatiff, class_result, 5);
	cout << "图像已在out文件中生成！！！" << endl;
	tiffdata1  datatiff1;
	datatiff1.bands = datatiff.bands;
	datatiff1.heght = datatiff.heght;
	datatiff1.width = datatiff.width;
	//int fff[3] = { datatiff.bands, datatiff.heght ,datatiff.width };
	//cout << fff[0] << endl;
	//return datatiff1;
}

DLL_API int bandss()
{
	return 4;
}
