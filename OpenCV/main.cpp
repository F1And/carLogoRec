#include <iostream>
#include <fstream>
#include <opencv.hpp>
#include <highgui.h>
#include <cxcore.h>
#include <cstring>
#include <ml.h>
#include <vector>
#include <iomanip>
//#define TRAIN 
#define TEST
using namespace std;
using namespace cv;


//------------------------ ȫ�ֱ���-----------------------------------------
#define  F_NUM     1764     //7*7*9*4  ��������ά��
#define  N_SAMPLE  1000     //ѵ����������
#define  CLASSNUM  5        //�������� 
float Data[N_SAMPLE][F_NUM];
float Label[N_SAMPLE][CLASSNUM];
vector<float> descriptor;   // HOG�����������

//-----------------------------ȫ�ֺ���------------------------------------------
// ��ȡͼ��hog����
void getHOG(Mat& img) {
	HOGDescriptor *hog = new HOGDescriptor(
		Size(64, 64),      //win_size  ��ⴰ�ڴ�С�����Ｔ������ͼ
		Size(16, 16),      //block_size  ���С
		Size(8, 8),        //blackStride ����
		Size(8, 8),        //cell_size  ϸ�����С
		9                   //9��bin
	);
	hog->compute(           //��ȡHOG��������
		img,
		descriptor,          //�����������
		Size(64, 64),            //��������
		Size(0, 0)
	);
	delete hog;
	hog = NULL;
}

// �з��ַ���
vector<string> split(string s, char token) {
	istringstream iss(s);
	string word;
	vector<string> vs;
	while (getline(iss, word, token)) {
		vs.push_back(word);
	}
	return vs;
}

// װ������
void packTrainData(Mat &img, int label, int counter) {
	getHOG(img);// ��ȡͼƬHOG����
	int cur = 0;
	for (auto d : descriptor)
		Data[counter][cur++] = d;
	Label[counter][label] = 1.0;
}

//------------------------------������--------------------------------------------
class myNeuralNetwork {
public:
	myNeuralNetwork() {};
	myNeuralNetwork(char *str) { this->bp.load(str); }
	void initialNN();     // ��ʼ���������
	void train(float(&data)[N_SAMPLE][F_NUM], float(&label)[N_SAMPLE][CLASSNUM]);  //ѵ��
	int  predict(Mat &img);
private:
	CvANN_MLP_TrainParams params;  // �������
	CvANN_MLP bp;		// bp����
};

void myNeuralNetwork :: initialNN() {
	//term_crit��ֹ���� ��������������������������Сֵ(CV_TERMCRIT_ITER + CV_TERMCRIT_EPS),һ����һ���ﵽ��������ֹ
	params.term_crit = cvTermCriteria(CV_TERMCRIT_ITER + CV_TERMCRIT_EPS, 10000, 0.001);  //��������7000,��С���0.001

	//train_methodѵ������,opencv�����ṩ����������һ���Ǿ���ķ��򴫲��㷨BP,һ���ǵ��Է����㷨RPROP
	params.train_method = CvANN_MLP_TrainParams::BACKPROP;   //ѵ���������򴫲�

	//bp_moment_scaleȨֵ���³���
	params.bp_moment_scale = 0.1;
	//bp_dw_scaleȨֵ������
	params.bp_dw_scale = 0.1;

	Mat layerSizes = (Mat_<int>(1, 3) << F_NUM, 48, CLASSNUM);  //3��������
	bp.create(layerSizes, CvANN_MLP::SIGMOID_SYM);          //�����sigmoid
}

void myNeuralNetwork::train(float(&data)[N_SAMPLE][F_NUM], float(&label)[N_SAMPLE][CLASSNUM]) {
	Mat trainData(N_SAMPLE, F_NUM, CV_32FC1, data);
	Mat trainLabel(N_SAMPLE, CLASSNUM, CV_32FC1, label);
	bp.train(trainData, trainLabel, Mat(), Mat(), params);  //��ʼѵ��
	bp.save("carClassifieryu.xml");
}

int myNeuralNetwork::predict(Mat &img) {
	::getHOG(img);
	float testData[1][F_NUM];
	int cur = 0;
	for (auto d : descriptor) 
		testData[0][cur++] = d;

	Mat nearest(1, CLASSNUM, CV_32FC1, Scalar(0));
	Mat charFeature(1, F_NUM, CV_32FC1, testData);
	bp.predict(charFeature, nearest);
	Point maxLoc; minMaxLoc(nearest, NULL, NULL, NULL, &maxLoc);
	int result = maxLoc.x;
	return result;
}

int main() {
#ifdef TRAIN
	ifstream in("trainpath.txt");
	string s;

	int label;
	int counter = 0;
	while (in >> s) {
		// ��ȡlabel
		vector<string> fp = split(s, '/');
		label = fp[3].c_str()[0] - '0';
		// ��ȡͼƬ��Ϣ
		Mat imge = imread(s), img;
		if (imge.empty())
		{
			cout << "image load error!" << endl;
			system("pause");
			return 0;
		}
		resize(imge, img, Size(64, 64));
		packTrainData(img, label,counter++);
	}
	cout << "����ѵ�������!" << endl;
	cout << "��ʼѵ��........." <<endl;
	myNeuralNetwork ann;
	ann.initialNN();
	ann.train(Data, Label);
	cout << "ѵ����ɣ�" << endl;
#endif
#ifdef TEST
	ifstream test_in("testpath.txt");
	string ss;
	int true_label;
	int counter = 0;
	int wrong = 0;
	string carlog[5] = { "ѩ����","����","һ��","����","����" };
	cout << "��ʼ����..." << endl;
	cout << "����: 0.00%";
	while (test_in >> ss) {
		// ��ȡlabel
		vector<string> fp = split(ss, '/');
		true_label = fp[3].c_str()[0] - '0';
		// ��ȡͼƬ��Ϣ
		Mat imge = imread(ss), img;
		if (imge.empty())
		{
			cout << "image load error!" << endl;
			system("pause");
			return 0;
		}
		resize(imge, img, Size(64, 64));
		myNeuralNetwork predictor("carClassifieryu.xml");
		int predict_label = predictor.predict(img);
		if (predict_label != true_label)
			wrong++;
		counter++;
		cout<<"\r����: "<< setprecision(2) << fixed << counter*1.0 / 5 << "%" ;

		
	}
	cout << endl;
	cout << "----------------------" << endl;
	cout << "���Խ��" << endl;
	cout << "----------------------" << endl;
	cout << "Ŀ����Ŀ��" << counter << endl;
	cout << "��ȷ������" << counter-wrong << endl;
	cout << "��ȷ�ʣ�" << setprecision(2)<<fixed<<(counter-wrong)*1.0/counter*100 <<"%"<< endl;
	cout << "----------------------" << endl;
#endif
	system("pause");
	return 0;
}