
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

#define BUILD 1
#define MY_API __attribute__((visibility("default")))

#endif

    // 人脸比对函数
    bool MY_API faceCompare(const unsigned char *src, int w1, int h1, int r1,
                            const unsigned char *cmp, int w2, int h2, int r2, double t);

#ifdef __cplusplus
}
#endif
