#include <iostream>
#include <fstream>
#include <opencv.hpp>
#include <highgui.h>
#include <cxcore.h>
#include <cstring>
#include <ml.h>
#include <vector>

enum  status{
	TRAIN,
	TEST
}sta;

using namespace std;
using namespace cv;

#define  F_NUM    1764     //7*7*9*4
#define  m_NUM    560
#define  CLASSNUM 4

//----------------------------ȫ�ֱ�������---------------------------------
	vector<float> descriptors;               //HOG����������� 
	float    data[m_NUM][F_NUM];             //���������������
	float    f[1][F_NUM];
	float    dataCls[m_NUM][CLASSNUM];       //�����������
	int      mClass ;                        //ѵ�������������
	int      dNum;                           //ѵ����������
/*-------------------------------------------------------------------------*/

//-----------------------------��������------------------------------------
/**************************************************
*���ƣ�init()
*������void
*����ֵ��void
*���ã���ʼ���������
****************************************************/
void  init()
{
	memset(data,0,sizeof(data));
	memset(dataCls,0,sizeof(dataCls));
	 mClass = -1;
	   dNum = 0;
}

/**************************************************
*���ƣ�getHOG()
*������Mat& img
*����ֵ��void
*���ã���ȡͼ���HOG����
****************************************************/
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

/**************************************************
*���ƣ�packData()
*������ö��
*����ֵ��void
*���ã���������������������
****************************************************/
void packData(status sta)
{
	int p = 0;
	if (sta == TRAIN)
	{
		for (vector<float>::iterator it = descriptors.begin(); it != descriptors.end(); it++)
		{
			data[dNum][p++] = *it;
		}
		dataCls[dNum++][mClass] = 1.0;
	}
	else if(sta == TEST)
	{
		for (vector<float>::iterator it = descriptors.begin(); it != descriptors.end(); it++)
		{
			f[0][p++] = *it;
		}
	}
	//descriptors.clear();
}

/**************************************************
*���ƣ�packData()
*������void
*����ֵ��void
*���ã���������������������
****************************************************/
int classifier(Mat& image,CvANN_MLP& bp)
{
    

	getHOG(image);
	packData(sta);

	Mat nearest(1, CLASSNUM, CV_32FC1, Scalar(0));	
	Mat charFeature(1, F_NUM, CV_32FC1,f);

    bp.predict(charFeature, nearest);
    Point maxLoc;
    minMaxLoc(nearest, NULL, NULL, NULL, &maxLoc);
    int result = maxLoc.x;
    return result;
}
int main()
{
	init();
	sta = TRAIN;
	ifstream in("trainpath.txt");
	cout<<"2s��ʼѵ��..."<<endl;
	Sleep(2000);
	system("cls");
	string s,ss;
	while( in >> s){
		if(ss != s.substr(0,44)){
			mClass++;            //�����0��1��2��3
			cout<<mClass<<endl;
		}
		ss = s.substr(0,44);
		 cout<<s<<endl;
//------------------------����ͼ������ͼ��----------------------------
        Mat imge = imread(s),img;  
	
	    if(imge.empty())
	    {
			cout<<"image load error!"<<endl;
			system("pause");
			return 0;
	    }
		resize(imge,img,Size(64,64)); 

//------------------------��ȡHOG������������������---------------------
		getHOG(img);

		packData(sta);        //�������������������

	}
	
//------------------------��BP�����磬��ʼѵ��------------------------
	CvANN_MLP bp;

	CvANN_MLP_TrainParams params;
	params.term_crit=cvTermCriteria(CV_TERMCRIT_ITER+CV_TERMCRIT_EPS,7000,0.001);  //��������7000,��С���0.001
    params.train_method=CvANN_MLP_TrainParams::BACKPROP;   //ѵ���������򴫲�
	params.bp_moment_scale=0.1;
	params.bp_dw_scale=0.1;
   

	Mat layerSizes = (Mat_<int>(1,3) << F_NUM,48,4 );  //3��������
	Mat trainDate(m_NUM,F_NUM,CV_32FC1,data);
	Mat trainLable(m_NUM,CLASSNUM,CV_32FC1,dataCls);
	bp.create(layerSizes, CvANN_MLP::SIGMOID_SYM);          //�����sigmoid
	system("cls");
	cout<<"ѵ����...";
	bp.train(trainDate,trainLable, Mat(),Mat(), params);  //��ʼѵ��
	
	bp.save("carClassifier.xml");
	system("cls");
	cout << "ѵ����ɣ���" <<endl;
	cout <<dNum<<endl;
	
//---------------------------------����ͼ�񣬿�ʼ����--------------------------
	system("cls");
	sta = TEST;
	cout<<"��ʼ����..."<<endl;
	Sleep(2000);
	system("cls");
	Mat imge,img;

	ifstream ins("tpath.txt");

	int cls = -1;
	int num=0,c_num=0;
	while( ins >> s){
		memset(f,0,sizeof(f));
		if(ss != s.substr(0,44)){
			cls++;
			cout<<cls<<endl;
		}
		cout<<s<<endl;
		ss = s.substr(0,44);
		imge = imread(s);
		resize(imge,img,Size(64,64));         //ʹ�����Բ�ֵ
		num++;
		if (classifier(img,bp) == cls)
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



}