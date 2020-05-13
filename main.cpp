#include "dnn_face_recognition_ex.h"
#include <time.h>
#include <atlimage.h>

// 上下翻转图像
BYTE *flipud(BYTE *src, int h, int r)
{
	BYTE *buf = new BYTE[r];
	for (int i = 0; i < h / 2; ++i)
	{
		memcpy(buf, src + i * r, r);
		memcpy(src + i * r, src + (h - 1 - i) * r, r);
		memcpy(src + (h - 1 - i) * r, buf, r);
	}
	delete[] buf;
	return src;
}

int main(int argc, const char *argv[])
{
	const char *im1 = "src.jpg", *im2 = "dst.jpg";
	double threshold = argc < 4 ? 0.6 : atof(argv[3]);
	if (argc == 3)
	{
		im1 = argv[1];
		im2 = argv[2];
	}
	else
	{
		printf("Compare images on '%s' and '%s'. Threshold is %f.\n", im1, im2, threshold);
	}
	CImage img, cmp;
	if (img.Load(im1) == S_OK &&
		cmp.Load(im2) == S_OK)
	{
		const BYTE *src = flipud((BYTE *)img.GetBits() + (img.GetHeight() - 1) * img.GetPitch(),
								 img.GetHeight(), abs(img.GetPitch()));
		const BYTE *dst = flipud((BYTE *)cmp.GetBits() + (cmp.GetHeight() - 1) * cmp.GetPitch(),
								 cmp.GetHeight(), abs(cmp.GetPitch()));
		clock_t t = clock();
		bool r = faceCompare(src, img.GetWidth(), img.GetHeight(), abs(img.GetPitch()),
							 dst, cmp.GetWidth(), cmp.GetHeight(), abs(cmp.GetPitch()), threshold);
		printf("The result of faceCompare is %d. Using %d ms.\n", r, int(clock() - t));
	}
	else
	{
		printf("Load images failed.\n");
	}
	getchar();
}
