
本项目拷贝自: http://dlib.net

并进行修改。

编译器：Visual Studio 2015 x64

附加依赖项: jpeg.lib, libpng.lib, z.lib.（来自Anaconda3\Library\lib）

需手动解压"dlib.7z"到项目所在目录，或者自行进行编译生成：

dlib19.19.0_debug_64bit_msvc1900.lib

dlib19.19.0_release_64bit_msvc1900.lib

注意事项：

（1）需安装jpeg和png库:

sudo apt install libpng-dev
sudo apt install libjpeg-dev

（2）Makefile需指定“dlib-19.19/dlib/all/source.cpp”代码位置.

2020-5-12
