#include "AverageFaces.h"


using namespace cv;
using namespace std;
using namespace boost::filesystem;

void AlignAndDisplayImage(string filename) {
	Mat out, aligned, face, flandmark, in = imread(filename);
	Rect face_rect;
	if(!FindFace(in,face_rect,face)) 
		cout << "Error couldn't find face";
	else {
		//resize(face,face,Size(WIDTH,HEIGHT));
		int ret1 = AlignFLandmark(in,face_rect,flandmark);
		if(ret1 < 0) {
			cout << "Couldn't find facial landmarks, using slow method" << endl;
			//AlignImage(flandmark,out);
			//flandmark = out;
		}
		imshow("in",in);
		imshow("face",face);
		imshow("landmarks",flandmark);
		waitKey();
	}
}

inline void AddToMat(Mat &r, Mat &g, Mat &b, vector<Mat> &add) {
	Mat_<unsigned char>::iterator pB = add[0].begin<unsigned char>(), pG = add[1].begin<unsigned char>(), pR = add[2].begin<unsigned char>();
	Mat_<float>::iterator pR_F = r.begin<float>(), pG_F = g.begin<float>(), pB_F = b.begin<float>();
	while(pR_F != r.end<float>()) {
		*pR_F += float(*pR);
		*pG_F += float(*pG);
		*pB_F += float(*pB);
		++pR; ++pG; ++pB; ++pR_F, ++pG_F, ++pB_F;
	}
}

inline void DivideMat(Mat &in, int divide) {
	Mat_<float>::iterator p = in.begin<float>();
	while(p != in.end<float>()) {
		*p /= float(divide);
		++p;
	}
}

void AverageImages(string img_dir) {
	Mat total_r = Mat::zeros(HEIGHT,WIDTH,CV_32F), total_g = Mat::zeros(HEIGHT,WIDTH,CV_32F), total_b = Mat::zeros(HEIGHT,WIDTH,CV_32F);
	int count = 0;
	for(directory_iterator i(img_dir), end_iter; i != end_iter; i++) {
		string filename = string(img_dir) + string("/") + (i->path()).filename().string();
		Mat in = imread(filename);
		vector<Mat> rgb;
		split(in,rgb);
		AddToMat(total_r,total_g,total_b,rgb);
		++count;
	}
	DivideMat(total_r,count);
	DivideMat(total_g,count);
	DivideMat(total_b,count);
	vector<Mat> disp;
	disp.resize(3);
	total_r.convertTo(disp[2],CV_8U);
	total_g.convertTo(disp[1],CV_8U);
	total_b.convertTo(disp[0],CV_8U);
	Mat total;
	merge(disp,total);
	imshow("total",total);
	imwrite("average.png",total);
	cvWaitKey();
}

void FindMaxMinMeanDist(string img_dir, string mean_filename) {
	Mat mean = imread(mean_filename);
	map<double,string> diffs;
	for(directory_iterator i(img_dir), end_iter; i != end_iter; i++) {
		string name = (i->path()).filename().string();
		string filename = string(img_dir) + string("/") + name;
		Mat in = imread(filename), diff;
		absdiff(mean,in,diff);
		Scalar total = sum(diff);
		double comp = total[0] + total[1] + total[2];
		pair<double, string> p(comp, name);
		diffs.insert(p);
	}
	map<double,string>::const_iterator p = diffs.begin();
	int i = 0;
	while(p != diffs.end()) {
		cout << i << ": val = " << p->first << ", name = " << p->second << endl;
		++p; ++i;
		//let's wait so we can see the first 20 or so
		if( i == 20)
			cin.get();
	}
	cin.get();
}

void AlignAllImages(string img_dir, string align_dir) {
	for(directory_iterator i(img_dir), end_iter; i != end_iter; i++) {
		string filename = string(img_dir) + string("/") + (i->path()).filename().string();
		Mat face, flandmark, in = imread(filename);
		Rect face_rect;
		if(!FindFace(in,face_rect,face)) 
			cout << "Error couldn't find face";
		else {
			int ret1 = AlignFLandmark(in,face_rect,flandmark);
			if(ret1 < 0) {
				cout << "Couldn't find facial landmarks, using slow method" << endl;
			} else {
				imwrite(string(align_dir + "/" + (i->path()).filename().string()),flandmark);
			}
		}
	}
}

int main(int argc, char* argv[]) {
	//AlignAndDisplayImage(argv[1]);
	assert(argc > 1);
	int last_arg = atoi(argv[argc - 1]);
	if(last_arg == 0)
		AlignAllImages(argv[1],argv[2]);
	else if(last_arg == 1)
		AverageImages(argv[1]);
	else if(last_arg == 2)
		FindMaxMinMeanDist(argv[1],argv[2]);
	return 0;
}
