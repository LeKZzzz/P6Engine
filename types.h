//
// Created by LeK on 2023/5/11.
//

#ifndef P6ENGINE_TYPES_H
#define P6ENGINE_TYPES_H

#include "vector"
#include "regex"
#include "string"
#include <Windows.h>

using namespace std;

//呈现方式
#define PADDING 0   //填充
#define FRAME 1 //边框
//透明度
#define OPAQUE 0 //不透明的
#define TRANSPARENT 1   //透明

#define ALL (-1)
#define ROTATE 0
#define MOVE 1

#define KEEP_AUTO 1
#define CANCEL_AUTO 0

//FOV类型
#define VERTICAL_FOV 0
#define HORIZONTAL_FOV 1

//平面
#define XOY 0
#define XOZ 1
#define YOZ 2

struct vertex_v {   //模型顶点
    float x;
    float y;
    float z;
};

struct vertex_vt{   //uv贴图顶点
    float x;
    float y;
    float z;
};

struct vertex_vn{   //法向量
    float x;
    float y;
    float z;
};

//模型面
struct surface {
    vector<int> v; //v顶点索引
    vector<int> vt; //vt顶点索引
    vector<int> vn;   //vn顶点索引
    int show_mehtod; //面呈现方式
    int transparency;   //面透明度
    char mtl; //面材质
    int s; //平滑组
};

//旋转角度(弧度制)
struct angle{
    float x;
    float y;
    float z;
};

//增量
struct increment{
    float x;
    float y;
    float z;
};

//显示节点
struct buffer_node{
    COORD site; //坐标
    float distance; //距离
    char mtl;   //材质
};

//UI结点
struct ui_node{
    COORD begin; //起始坐标
    COORD end; //结束坐标
    char mtl;   //材质
    int priority;   //优先级
};

#endif //P6ENGINE_TYPES_H
