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
	bool compare(const matrix<float, 0, 1> &f, const std::vector<matrix<rgb_pixel>> &s, double t)
	{
		for (auto v : net(s))
		{
			double d = length(v - f);
			if (d < t)
			{
				return true;
			}
		}
		return false;
	}

	void feature(std::vector<matrix<float, 0, 1>> &fs, const std::vector<matrix<rgb_pixel>> &s)
	{
		fs = net(s);
	}
};

// load resnet v1.
resnet dnnNet;

int main()
try
{
	matrix<rgb_pixel> img, cmp;
	load_image(img, "src.jpg");
	load_image(cmp, "dst.jpg");
	std::vector<matrix<rgb_pixel>> src, dst;
	dnnNet.detect(src, img);
	dnnNet.detect(dst, cmp);
	std::vector<matrix<float, 0, 1>> f;
	dnnNet.feature(f, src);
	if (!f.empty())
	{
		auto r = dnnNet.compare(f[0], dst, 0.6);
		cout << r << endl;
	}
	else
	{
		cout << "can't find any face in src image." << endl;
	}
	cin.get();
}
catch (std::exception &e)
{
	cout << e.what() << endl;
	cin.get();
}
