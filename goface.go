package main

/*
#cgo CFLAGS: -I.
#cgo LDFLAGS: -L. -lfaceCompare
#cgo LDFLAGS: -Wl,-rpath=./
#include <stdlib.h>
#include <stdbool.h>
// 人脸比对函数
int faceCompare_s(const unsigned char *src, int w1, int h1, int r1,
                 const unsigned char *cmp, int w2, int h2, int r2, double t, bool flip, bool rgb);
*/
import "C"
import (
	"fmt"
	"image"
	"image/png"
	"os"
	"unsafe"
)

// a plugin must contains main function.
func main() {
	path := "./test.png"
	file, err := os.Open(path)
	if err != nil {
		fmt.Println(err)
		return
	}
	defer file.Close()
	img, err := png.Decode(file)
	if err != nil {
		fmt.Println(err)
		return
	}
	var dis = map[string]interface{}{"image": img}
	var arr = []image.Image{img}
	var ids = []string{path}
	for i := 0; i < 3; i++ {
		AlarmProcess(dis, nil, arr, ids, 2)
	}
}

// getImageInfo 获取图像信息.
func getImageInfo(v image.Image)(src []byte, w,h,r int){
	switch o:=v.(type) {
	case *image.YCbCr:
	case *image.RGBA:
		src = []byte(o.Pix)
		r = o.Stride
	case *image.NRGBA:
		src = []byte(o.Pix)
		r = o.Stride
	case *image.CMYK:
		src = []byte(o.Pix)
		r = o.Stride
	case *image.Gray:
		src = []byte(o.Pix)
		r = o.Stride
	}
	return src, v.Bounds().Dx(), v.Bounds().Dy(), r
}

// AlarmProcess 告警处理单元.
// go build -buildmode=plugin goface.go
func AlarmProcess(dis map[string]interface{}, features []interface{}, arr []image.Image, ids []string, level int) bool {
	var levelThresholdMap = map[int]float64{0: 0.5, 1: 0.5, 2: 0.6, 3: 0.7}
	threshold := levelThresholdMap[level]
	var src, dst []byte
	var w1, h1, r1, w2, h2, r2 int
	if img := dis["image"]; img != nil {
		if v, ok := img.(image.Image); ok {
			src, w1, h1, r1 = getImageInfo(v)
		}
	}
	for _, a := range arr {
		dst, w2, h2, r2 = getImageInfo(a)
		if src == nil || dst == nil{
			continue
		}
		csrc := (*C.uchar)(unsafe.Pointer(&src[0]))
		cdst := (*C.uchar)(unsafe.Pointer(&dst[0]))
		var r = C.faceCompare_s(csrc, C.int(w1), C.int(h1), C.int(r1),
			cdst, C.int(w2), C.int(h2), C.int(r2), C.double(threshold), false, true)
		fmt.Println("result:", r)
		if r == 1 {
			return true
		}
	}

	return false
}
