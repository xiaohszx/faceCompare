
#ifdef __cplusplus
extern "C"
{
#endif

#ifdef BUILD_DLL
#define MY_API __declspec(dllexport)
#else
#define MY_API __declspec(dllimport)
#endif

    // 人脸比对函数
    bool MY_API faceCompare(const unsigned char *src, int w1, int h1, int r1,
                            const unsigned char *cmp, int w2, int h2, int r2, double t);

#ifdef __cplusplus
}
#endif
