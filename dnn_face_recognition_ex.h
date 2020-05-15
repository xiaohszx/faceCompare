#ifndef __facecompare
#define __facecompare

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef _WIN32

#ifdef BUILD_DLL
#define MY_API __declspec(dllexport)
#else
#define MY_API __declspec(dllimport)
#endif

#else

#ifdef BUILD_DLL
#define MY_API __attribute__((visibility("default")))
#else
#define MY_API
#endif

#endif

	// 人脸比对函数
	// src, w1, h1, r1: 原始人脸图像信息
	// cmp, w2, h2, r2: 待比对人脸图像信息
	// t: 比对阈值（建议0.6）
	// flip: 是否上下翻转图像
	bool MY_API faceCompare(unsigned char *src, int w1, int h1, int r1,
							unsigned char *cmp, int w2, int h2, int r2, double t, bool flip);

#ifdef __cplusplus
}
#endif
#endif
