#include <iostream>
#include <fstream>
#include <opencv.hpp>
#include <highgui.h>
#include <cxcore.h>
#include <cstring>
#include <ml.h>
#include <vector>

using namespace std;
using namespace cv;
#define  F_NUM    1764   //7*7*9*4
#define nClass 4
enum carBrd
{
	XTL=0,
	HONDA,
	FAW,
	FUKUDA
};
CvANN_MLP bp;
vector<float> descriptors;               //HOG����������� 
float  f[1][F_NUM];
void getHOG(Mat& img)
{
	HOGDescriptor *hog = new HOGDescriptor(             
			Size(64,64),      //win_size  ��ⴰ�ڴ�С�����Ｔ������ͼ
			Size(16,16),      //block_size  ���С
			Size(8,8),        //blackStride ����
			Size(8,8),        //cell_size  ϸ�����С
			9                   //9��bin
			);
	hog -> compute(           //��ȡHOG��������
		img, 
		descriptors,          //�����������
		Size(64,64),            //��������
		Size(0,0)
		);
	delete hog;
	hog = NULL;
}

void packData()
{
	int p = 0;
	for (vector<float>::iterator it = descriptors.begin(); it != descriptors.end(); it++)
	{
		f[0][p++] = *it;
	}
	descriptors.clear();
}

int classifier(Mat& image)
{
    

	getHOG(image);
	packData();

	Mat nearest(1, nClass, CV_32FC1, Scalar(0));	
	Mat charFeature(1, F_NUM, CV_32FC1,f);

    return  (int)bp.predict(charFeature, nearest);
    Point maxLoc;
    minMaxLoc(nearest, NULL, NULL, NULL, &maxLoc);
    int result = maxLoc.x;
    return result;
}
int main()
{
//	Mat laySize=(Mat_<int>(1,3) << F_NUM,48,4);
//	bp.create(laySize,CvANN_MLP::SIGMOID_SYM);
	bp.load("carClassifier.xml");

	Mat imge,img;

	ifstream in("tpath.txt");
	string s,ss;
	int cls = -1;
	int num=0,c_num=0;
	while( in >> s){
		memset(f,0,sizeof(f));
		if(ss != s.substr(0,44)){
			cls++;
			cout<<cls<<endl;
			//Sleep(2000);
		}
		cout<<s<<endl;
		ss = s.substr(0,44);
		imge = imread(s);
		resize(imge,img,Size(64,64));         //ʹ�����Բ�ֵ
		num++;
		if (classifier(img) == cls)
		{
			c_num++;
		}

	}
	system("cls");
	cout<<"�������"<<endl;
	cout<<"***************************************"<<endl;
	cout<<"*����������"<<num<<endl;
	cout<<"*��ȷ������"<<c_num<<endl;
	cout<<"***************************************"<<endl;
		system("pause");

	
	system("pause");
}
