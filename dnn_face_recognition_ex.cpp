// The contents of this file are in the public domain. See LICENSE_FOR_EXAMPLE_PROGRAMS.txt
/*
    This is an example illustrating the use of the deep learning tools from the dlib C++
    Library.  In it, we will show how to do face recognition.  This example uses the
    pretrained dlib_face_recognition_resnet_model_v1 model which is freely available from
    the dlib web site.  This model has a 99.38% accuracy on the standard LFW face
    recognition benchmark, which is comparable to other state-of-the-art methods for face
    recognition as of February 2017. 
    
    In this example, we will use dlib to do face clustering.  Included in the examples
    folder is an image, bald_guys.jpg, which contains a bunch of photos of action movie
    stars Vin Diesel, The Rock, Jason Statham, and Bruce Willis.   We will use dlib to
    automatically find their faces in the image and then to automatically determine how
    many people there are (4 in this case) as well as which faces belong to each person.
    
    Finally, this example uses a network with the loss_metric loss.  Therefore, if you want
    to learn how to train your own models, or to get a general introduction to this loss
    layer, you should read the dnn_metric_learning_ex.cpp and
    dnn_metric_learning_on_images_ex.cpp examples.
*/

#include <dlib/dnn.h>
#include <dlib/string.h>
#include <dlib/image_io.h>
#include <dlib/image_processing/frontal_face_detector.h>
#include "dnn_face_recognition_ex.h"

using namespace dlib;
using namespace std;

// ----------------------------------------------------------------------------------------

// The next bit of code defines a ResNet network.  It's basically copied
// and pasted from the dnn_imagenet_ex.cpp example, except we replaced the loss
// layer with loss_metric and made the network somewhat smaller.  Go read the introductory
// dlib DNN examples to learn what all this stuff means.
//
// Also, the dnn_metric_learning_on_images_ex.cpp example shows how to train this network.
// The dlib_face_recognition_resnet_model_v1 model used by this example was trained using
// essentially the code shown in dnn_metric_learning_on_images_ex.cpp except the
// mini-batches were made larger (35x15 instead of 5x5), the iterations without progress
// was set to 10000, and the training dataset consisted of about 3 million images instead of
// 55.  Also, the input layer was locked to images of size 150.
template <template <int, template <typename> class, int, typename> class block, int N, template <typename> class BN, typename SUBNET>
using residual = add_prev1<block<N, BN, 1, tag1<SUBNET>>>;

template <template <int, template <typename> class, int, typename> class block, int N, template <typename> class BN, typename SUBNET>
using residual_down = add_prev2<avg_pool<2, 2, 2, 2, skip1<tag2<block<N, BN, 2, tag1<SUBNET>>>>>>;

template <int N, template <typename> class BN, int stride, typename SUBNET>
using block = BN<con<N, 3, 3, 1, 1, relu<BN<con<N, 3, 3, stride, stride, SUBNET>>>>>;

template <int N, typename SUBNET>
using ares = relu<residual<block, N, affine, SUBNET>>;
template <int N, typename SUBNET>
using ares_down = relu<residual_down<block, N, affine, SUBNET>>;

template <typename SUBNET>
using alevel0 = ares_down<256, SUBNET>;
template <typename SUBNET>
using alevel1 = ares<256, ares<256, ares_down<256, SUBNET>>>;
template <typename SUBNET>
using alevel2 = ares<128, ares<128, ares_down<128, SUBNET>>>;
template <typename SUBNET>
using alevel3 = ares<64, ares<64, ares<64, ares_down<64, SUBNET>>>>;
template <typename SUBNET>
using alevel4 = ares<32, ares<32, ares<32, SUBNET>>>;

using anet_type = loss_metric<fc_no_bias<128, avg_pool_everything<
												  alevel0<
													  alevel1<
														  alevel2<
															  alevel3<
																  alevel4<
																	  max_pool<3, 3, 2, 2, relu<affine<con<32, 7, 7, 2, 2, input_rgb_image_sized<150>>>>>>>>>>>>>;

// ----------------------------------------------------------------------------------------

// 对内存图像的描述, 定义参考自: dlib\image_loader\jpeg_loader.h
class image : noncopyable
{
public:
	const unsigned char *data; // 图像数据
	int width_;				   // 图像宽度
	int height_;			   // 图像高度
	int output_components_;	   // 每行字节数
	int channel_;			   // 每像素字节

	image(const unsigned char *src, int w, int h, int r, int ch = 3)
		: data(src), width_(w), height_(h), output_components_(r), channel_(ch) {}
	matrix<rgb_pixel> &cvtToMatrix(matrix<rgb_pixel> &img) const;
	bool is_gray() const { return channel_ == 1; }
	bool is_rgb() const { return channel_ == 3; }
	bool is_rgba() const { return channel_ == 4; }
	const unsigned char *get_row(unsigned long i) const
	{
		return data + i * output_components_;
	}
};

// 将src转换为矩阵形式.参考自: dlib\image_loader\jpeg_loader.h
matrix<rgb_pixel> &image::cvtToMatrix(matrix<rgb_pixel> &img) const
{
	image_view<matrix<rgb_pixel>> t(img);
	t.set_size(height_, width_);
	for (unsigned n = 0; n < height_; n++)
	{
		const unsigned char *v = get_row(n);
		for (unsigned m = 0; m < width_; m++)
		{
			if (is_gray())
			{
				unsigned char p = v[m];
				assign_pixel(t[n][m], p);
			}
			else if (is_rgba())
			{
				rgb_alpha_pixel p;
				p.red = v[m * 4];
				p.green = v[m * 4 + 1];
				p.blue = v[m * 4 + 2];
				p.alpha = v[m * 4 + 3];
				assign_pixel(t[n][m], p);
			}
			else // if ( is_rgb() )
			{
				rgb_pixel p;
				p.red = v[m * 3];
				p.green = v[m * 3 + 1];
				p.blue = v[m * 3 + 2];
				assign_pixel(t[n][m], p);
			}
		}
	}
	return img;
}

// resnet dlib_face_recognition_resnet_model_v1.
class resnet
{
private:
	frontal_face_detector detector;
	shape_predictor sp;
	anet_type net;

public:
	resnet()
	{
		// The first thing we are going to do is load all our models.  First, since we need to
		// find faces in the image we will need a face detector:
		detector = get_frontal_face_detector();
		// We will also use a face landmarking model to align faces to a standard pose.
		// (see face_landmark_detection_ex.cpp for an introduction)
		deserialize("shape_predictor_5_face_landmarks.dat") >> sp;
		// And finally we load the DNN responsible for face recognition.
		deserialize("dlib_face_recognition_resnet_model_v1.dat") >> net;
	}
	// Run the face detector on the image of our action heroes, and for each face extract a
	// copy that has been normalized to 150x150 pixels in size and appropriately rotated
	// and centered.
	// 提取图像img中的人像，保存到faces.
	void detect(std::vector<matrix<rgb_pixel>> &faces, const matrix<rgb_pixel> &img)
	{
		for (auto face : detector(img))
		{
			auto shape = sp(img, face);
			matrix<rgb_pixel> face_chip;
			extract_image_chip(img, get_face_chip_details(shape, 150, 0.25), face_chip);
			faces.push_back(move(face_chip));
		}
	}
	// This call asks the DNN to convert each face image in faces into a 128D vector.
	// In this 128D vector space, images from the same person will be close to each other
	// but vectors from different people will be far apart.  So we can use these vectors to
	// identify if a pair of images are from the same person or from different people.
	// 提取一组人像s（尺寸150x150）中的特征，和f进行比较，如果存在小于阈值t的人像，返回true.
	bool compare(const matrix<float, 0, 1> &f, const std::vector<matrix<rgb_pixel>> &s, double t)
	{
		for (auto v : net(s))
		{
			double d = length(v - f);
#ifdef _DEBUG
			cout << "Distance of two images: " << d << endl;
#endif
			if (d < t)
			{
				return true;
			}
		}
		return false;
	}
	// 提取一组图像s（尺寸150x150）中的人像特征，保存为fs.
	void feature(std::vector<matrix<float, 0, 1>> &fs, const std::vector<matrix<rgb_pixel>> &s)
	{
		fs = net(s);
	}
	// 对两幅图像进行人像比较，如果在cmp中找到了img里的人像，返回true，否则返回false.
	bool compare(const matrix<rgb_pixel> &img, const matrix<rgb_pixel> &cmp, double t)
	{
		if (img.size() == 0 || cmp.size() == 0)
			return false;
		std::vector<matrix<rgb_pixel>> src, dst;
		detect(src, img);
		detect(dst, cmp);
		std::vector<matrix<float, 0, 1>> f;
		feature(f, src);
		return !f.empty() ? compare(f[0], dst, t) : false;
	}
	// 对两幅图像进行人像比较，如果在cmp中找到了img里的人像，返回true，否则返回false.
	bool compare(const image &img, const image &cmp, double t)
	{
		matrix<rgb_pixel> m1, m2;
		img.cvtToMatrix(m1);
		cmp.cvtToMatrix(m2);
		return compare(m1, m2, t);
	}
};

// load resnet v1.
resnet dnnNet;

#ifndef BUILD_DLL

int main()
try
{
	matrix<rgb_pixel> img, cmp;
	load_image(img, "src.jpg");
	load_image(cmp, "dst.jpg");
	cout << "compare result of two images:" << dnnNet.compare(img, cmp, 0.6) << endl;
	cin.get();
	return 0;
}
catch (std::exception &e)
{
	cout << e.what() << endl;
	cin.get();
	return -1;
}

#else

int main()
{
	cout << "Load faceCompare.lib succeed." << endl;
	return 0;
}

#endif

// faceCompare 用于导出的人像比对API.
bool faceCompare(const unsigned char *src, int w1, int h1, int r1,
				 const unsigned char *cmp, int w2, int h2, int r2, double t)
{
	return dnnNet.compare(image(src, w1, h1, r1, r1 / w1), image(cmp, w2, h2, r2, r2 / w2), t);
}
