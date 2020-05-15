package main

import (
	"fmt"
	"image"
	"image/png"
	"os"
	"plugin"
	"testing"
)

// dis-布控任务；features-特征值列表；arr-子图列表；ids-子图id列表；level-预警灵敏度.
type alarmCallback func(dis map[string]interface{}, features []interface{}, arr []image.Image, ids []string, level int) bool

// loadPlugin 加载第三方动态库.该动态库实现了形如"AlarmCallback"的告警处理函数.
func loadPlugin(soPath, funName string) (alarmCallback, error) {
	// 打开so文件
	p, err := plugin.Open(soPath)
	if err != nil {
		return nil, fmt.Errorf("open failed(%v)", err)
	}
	// 查找函数
	f, err := p.Lookup(funName)
	if err != nil {
		return nil, fmt.Errorf("lookup failed(%v)", err)
	}
	// 转换类型后调用函数
	if cb, ok := f.(func(map[string]interface{}, []interface{}, []image.Image, []string, int) bool); ok {
		return cb, nil
	}
	return nil, fmt.Errorf("'%s' has no Alarm func", soPath)
}

// test load "goface.so".
func TestLoadGoface(t *testing.T) {
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
	var alarm alarmCallback
	if cb, err := loadPlugin("./goface.so", "AlarmProcess"); err == nil {
		alarm = cb
		fmt.Printf("load 'goface.so' succeed.\n",)
	} else {
		fmt.Printf("load 'goface.so' failed, err= %v.\n", err)
		return
	}

	for i := 0; i < 3; i++ {
		alarm(dis, nil, arr, ids, 2)
	}
}
