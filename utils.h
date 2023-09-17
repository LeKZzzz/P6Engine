//
// Created by LeK on 2023/5/18.
//

#ifndef P6ENGINE_UTILS_H
#define P6ENGINE_UTILS_H

#include "types.h"
#include "stack"
#include "cmath"
#include <amp.h>

using namespace std;
using namespace concurrency;

//角度转弧度
float degree_to_radian(float degree) {
    return degree * float(atan(1) * 4) / 180;
}

//绕轴旋转
void util_rotate(vertex_v origin, vertex_v &v, angle angles) {
    float x1, y1, z1;
    //绕z轴旋转
    x1 = (v.x - origin.x) * cos(float(angles.z)) - (v.y - origin.y) * sin(float(angles.z));
    y1 = (v.x - origin.x) * sin(float(angles.z)) + (v.y - origin.y) * cos(float(angles.z));
    v.x = origin.x + x1;
    v.y = origin.y + y1;

    //绕x轴旋转
    y1 = (v.z - origin.z) * sin(float(angles.x)) + (v.y - origin.y) * cos(float(angles.x));
    z1 = (v.z - origin.z) * cos(float(angles.x)) - (v.y - origin.y) * sin(float(angles.x));
    v.y = origin.y + y1;
    v.z = origin.z + z1;

    //绕y轴旋转
    x1 = (v.x - origin.x) * cos(float(angles.y)) - (v.z - origin.z) * sin(float(angles.y));
    z1 = (v.x - origin.x) * sin(float(angles.y)) + (v.z - origin.z) * cos(float(angles.y));
    v.x = origin.x + x1;
    v.z = origin.z + z1;
}

void util_rotate(vertex_v origin, vertex_vn &v, angle angles) {
    float x1, y1, z1;
    //绕z轴旋转
    x1 = (v.x - origin.x) * cos(float(angles.z)) - (v.y - origin.y) * sin(float(angles.z));
    y1 = (v.x - origin.x) * sin(float(angles.z)) + (v.y - origin.y) * cos(float(angles.z));
    v.x = origin.x + x1;
    v.y = origin.y + y1;

    //绕x轴旋转
    y1 = (v.z - origin.z) * sin(float(angles.x)) + (v.y - origin.y) * cos(float(angles.x));
    z1 = (v.z - origin.z) * cos(float(angles.x)) - (v.y - origin.y) * sin(float(angles.x));
    v.y = origin.y + y1;
    v.z = origin.z + z1;

    //绕y轴旋转
    x1 = (v.x - origin.x) * cos(float(angles.y)) - (v.z - origin.z) * sin(float(angles.y));
    z1 = (v.x - origin.x) * sin(float(angles.y)) + (v.z - origin.z) * cos(float(angles.y));
    v.x = origin.x + x1;
    v.z = origin.z + z1;
}


//水平FOV和垂直FOV转换
float fov_exchange(float fov, int width, int length, bool cur_flag) {
    if (cur_flag == VERTICAL_FOV)
        return 2 * ::atan((float(length) * ::tan(fov / 2)) / float(width));
    else if (cur_flag == HORIZONTAL_FOV)
        return 2 * ::atan((float(width) * ::tan(fov / 2)) / float(length));
}

//获取距离
float get_distance(vertex_v start, vertex_v end) {
    return sqrt(float(pow(start.x - end.x, 2) + pow(start.y - end.y, 2) + pow(start.z - end.z, 2)));
}

//获取向量夹角
float get_included_angle(vertex_vn vector1, vertex_vn vector2, int type) {
    float length1, length2, result;
    if (type == XOY) {
        length1 = float(::sqrt(pow(vector1.x, 2) + pow(vector1.y, 2)));
        length2 = float(::sqrt(pow(vector2.x, 2) + pow(vector2.y, 2)));
        result = acos((vector1.x * vector2.x + vector1.y * vector2.y) / (length1 * length2));
        return result;
    } else if (type == XOZ) {
        length1 = float(::sqrt(pow(vector1.x, 2) + pow(vector1.z, 2)));
        length2 = float(::sqrt(pow(vector2.x, 2) + pow(vector2.z, 2)));
        result = acos((vector1.x * vector2.x + vector1.z * vector2.z) / (length1 * length2));
        return result;
    } else if (type == YOZ) {
        length1 = float(::sqrt(pow(vector1.y, 2) + pow(vector1.z, 2)));
        length2 = float(::sqrt(pow(vector2.y, 2) + pow(vector2.z, 2)));
        result = acos((vector1.y * vector2.y + vector1.z * vector2.z) / (length1 * length2));
        return result;
    }

}

//三维顶点转二维平面
int space_to_plane(int screen_arg, float fov, vertex_vn screen, vertex_vn v, int type) {
    int result;
    float radian;
    radian = get_included_angle(screen, v, type);
    result = int(round(tan(radian) * float(screen_arg) / tan(fov / 2)));
    return result;
}

//Bresenham算法绘制直线
void
bresenham(vector<buffer_node> *frame_buffer, int buffer_size_x, int buffer_size_y, buffer_node node1, buffer_node node2,
          char mtl, float distance, bool type = FRAME,
          buffer_node begin = {{0, 0}, 0, ' '}, char pad_mtl = ' ') {

    bool flag1 = false, flag2 = false, flag3 = false;
    int dy, dx, incrE, incrNE, x, y, check;
    buffer_node newnode;

    newnode.mtl = mtl;
    newnode.distance = distance;  //TODO:待改进

    if (node1.site.X > node2.site.X) {  //线段位于二三象限
        flag1 = true;
        swap(node1.site.X, node2.site.X);
        swap(node1.site.Y, node2.site.Y);
    }
    if (node2.site.Y < node1.site.Y) {  //线段位于第四象限
        flag2 = true;
        node2.site.Y += 2 * (node1.site.Y - node2.site.Y);
    }
    dy = node2.site.Y - node1.site.Y;
    dx = node2.site.X - node1.site.X;
    if (dy - dx > 0) {    //线段斜率大于1
        flag3 = true;
        node2.site.X -= node1.site.X;
        node2.site.Y -= node1.site.Y;
        swap(node2.site.X, node2.site.Y);
        node2.site.X += node1.site.X;
        node2.site.Y += node1.site.Y;
    }

    dy = node2.site.Y - node1.site.Y;
    dx = node2.site.X - node1.site.X;
    x = node1.site.X;
    y = node1.site.Y;
    check = 2 * dy - dx;
    incrE = 2 * dy;
    incrNE = 2 * (dy - dx);
    //if(dx==0)
    for (int i = 1; i < dx;) {
        if (check < 0) {
            check += incrE;
            x += 1;
        } else {
            check += incrNE;
            x += 1;
            y += 1;
        }
        newnode.site.X = x;
        newnode.site.Y = y;
        if (flag3) {
            newnode.site.X -= node1.site.X;
            newnode.site.Y -= node1.site.Y;
            swap(newnode.site.X, newnode.site.Y);
            newnode.site.X += node1.site.X;
            newnode.site.Y += node1.site.Y;
        }
        if (flag2) newnode.site.Y -= 2 * (newnode.site.Y - node1.site.Y);

        if (type == PADDING)
            bresenham(frame_buffer, buffer_size_x, buffer_size_y, begin, newnode, pad_mtl, distance);  //填充三角面

        if (newnode.site.X < 0 || newnode.site.Y < 0 || newnode.site.X >= buffer_size_x ||
            newnode.site.Y >= buffer_size_y)
            goto done;
        for (auto it = frame_buffer[newnode.site.Y].begin();
             it != frame_buffer[newnode.site.Y].end(); it++) {
            if ((*it).site.Y == newnode.site.Y && (*it).site.X == newnode.site.X) {
                if (newnode.distance < (*it).distance|| (newnode.distance == (*it).distance && (*it).mtl == ' ')) {
                    (*it).distance = newnode.distance;
                    (*it).mtl = newnode.mtl;
                }
                goto done;
            }
        }
        frame_buffer[newnode.site.Y].push_back(newnode);
        done:
        i++;
    }

}

//绘制平面
void draw_surface(vector<buffer_node> *frame_buffer, vector<vertex_v> v, surface f, bool show_method,
                  int screen_length, int screen_width,
                  float fov_horizontal,
                  float fov_vertical, vertex_vn vn, vertex_v origin, float z_rotation) {
    vertex_v origin_v, center = {0, 0, 0};  //面顶点
    float xn, yn, zn;
    int x, y;
    float t;
    buffer_node newnode, begin, pre;

    for (int j = 0; j < f.v.size(); j++) {
        center.x += v[f.v[j]].x;
        center.y += v[f.v[j]].y;
        center.z += v[f.v[j]].z;
    }
    center.x = center.x / f.v.size();
    center.y = center.y / f.v.size();
    center.z = center.z / f.v.size();
    float distance = get_distance(center, origin);

    for (int j = 0; j < f.v.size();) {
        origin_v = v[f.v[j]];
        util_rotate(origin, origin_v, {0, 0, -z_rotation});  //绕z轴旋转  TODO:待改
        xn = origin_v.x - origin.x;
        yn = origin_v.y - origin.y;
        zn = origin_v.z - origin.z;

//        x = space_to_plane(screen_length / 2, fov_horizontal, vn, {xn, yn, zn}, XOZ);
//        y = space_to_plane(screen_width / 2, fov_vertical, vn, {xn, yn, zn}, YOZ);
//
//        if (get_included_angle({0, 1, 0}, vn, YOZ) < get_included_angle({0, 1, 0}, {xn, yn, zn}, YOZ))
//            newnode.site.Y = short(screen_width / 2 - y - 1);
//        else
//            newnode.site.Y = short(screen_width / 2 + y - 1);
//        if (get_included_angle({1, 0, 0}, vn, XOZ) < get_included_angle({1, 0, 0}, {xn, yn, zn}, XOZ))
//            newnode.site.X = short(screen_length / 2 + x - 1);
//        else
//            newnode.site.X = short(screen_length / 2 - x - 1);
        if (vn.z >= 0) t = 1;
        else t = -1;
        //TODO:与光屏相交,完善情况
        //TODO:解决线条遮挡bug
        x = space_to_plane(screen_length / 2, fov_horizontal, vn, {xn, yn, zn}, XOZ);
        y = space_to_plane(screen_width / 2, fov_vertical, vn, {xn, yn, zn}, YOZ);
        if (get_included_angle({0, t, 0}, vn, YOZ) < get_included_angle({0, t, 0}, {xn, yn, zn}, YOZ)) {
            if (get_included_angle(vn, {xn, yn, zn}, YOZ) < fov_vertical / 2)
                newnode.site.Y = short(screen_width / 2 - y - 1);
            else
                newnode.site.Y = short(-1);
        } else {
            if (get_included_angle(vn, {xn, yn, zn}, YOZ) < fov_vertical / 2)
                newnode.site.Y = short(screen_width / 2 + y - 1);
            else
                newnode.site.Y = short(screen_width);
        }

        if (get_included_angle({t, 0, 0}, vn, XOZ) < get_included_angle({t, 0, 0}, {xn, yn, zn}, XOZ)) {
            if (get_included_angle(vn, {xn, yn, zn}, XOZ) < fov_horizontal / 2)
                newnode.site.X = short(screen_length / 2 + x - 1);
            else
                newnode.site.X = short(screen_length);
        } else {
            if (get_included_angle(vn, {xn, yn, zn}, XOZ) < fov_horizontal / 2)
                newnode.site.X = short(screen_length / 2 - x - 1);
            else
                newnode.site.X = short(-1);
        }

        newnode.mtl = f.mtl;
        newnode.distance = get_distance(v[f.v[j]], origin);
        if (newnode.site.X < 0 || newnode.site.Y < 0 || newnode.site.X >= screen_length ||
            newnode.site.Y >= screen_width)
            goto done1;
        for (auto it = frame_buffer[newnode.site.Y].begin();
             it != frame_buffer[newnode.site.Y].end(); it++) {
            if ((*it).site.Y == newnode.site.Y && (*it).site.X == newnode.site.X) {
                if (newnode.distance < (*it).distance|| (newnode.distance == (*it).distance && (*it).mtl == ' ')) {
                    (*it).distance = newnode.distance;
                    (*it).mtl = newnode.mtl;
                }
                goto done1;
            }
        }
        frame_buffer[newnode.site.Y].push_back(newnode);
        done1:
        if (j == 0) begin = newnode;
        else if (j == 1) {
            bresenham(frame_buffer, screen_length, screen_width, pre, newnode, f.mtl, distance);
        } else {
            if (show_method == PADDING) {
                bresenham(frame_buffer, screen_length, screen_width, begin, newnode, f.mtl, distance);
                bresenham(frame_buffer, screen_length, screen_width, pre, newnode, f.mtl, distance, PADDING, begin,
                          f.mtl);
            } else {
                bresenham(frame_buffer, screen_length, screen_width, pre, newnode, f.mtl, distance);
                bresenham(frame_buffer, screen_length, screen_width, pre, newnode, f.mtl, distance, PADDING, begin,
                          ' ');

            }
        }
        pre = newnode;
        ++j;
    }
    bresenham(frame_buffer, screen_length, screen_width, begin, newnode, f.mtl, distance);
}

//绘制UI
void draw_ui(vector<buffer_node> *frame_buffer, vector<ui_node> *ui, int screen_length, int screen_width) {
    buffer_node newnode1, newnode2;
    for (auto it = ui->begin(); it != ui->end(); it++) {
        newnode1.site = (*it).begin;
        newnode2.site = (*it).end;
        newnode1.mtl = (*it).mtl;
        newnode2.mtl = (*it).mtl;
        newnode1.distance = float(-1 * (*it).priority);
        newnode2.distance = float(-1 * (*it).priority);

        for (auto it2 = frame_buffer[newnode1.site.Y].begin();
             it2 != frame_buffer[newnode1.site.Y].end(); it2++) {
            if ((*it2).site.Y == newnode1.site.Y && (*it2).site.X == newnode1.site.X) {
                if (newnode1.distance < (*it2).distance|| (newnode1.distance == (*it2).distance && (*it2).mtl == ' ')) {
                    (*it2).distance = newnode1.distance;
                    (*it2).mtl = newnode1.mtl;
                    goto done2;
                }
            }
        }
        frame_buffer[newnode1.site.Y].push_back(newnode1);
        done2:
        for (auto it2 = frame_buffer[newnode2.site.Y].begin();
             it2 != frame_buffer[newnode2.site.Y].end(); it2++) {
            if ((*it2).site.Y == newnode2.site.Y && (*it2).site.X == newnode2.site.X) {
                if (newnode2.distance < (*it2).distance || (newnode2.distance == (*it2).distance && (*it2).mtl == ' ')) {
                    (*it2).distance = newnode2.distance;
                    (*it2).mtl = newnode2.mtl;
                    goto done3;
                }
            }
        }
        frame_buffer[newnode2.site.Y].push_back(newnode2);
        done3:
        bresenham(frame_buffer, screen_length, screen_width, newnode1, newnode2, newnode1.mtl, newnode1.distance);
    }
}

#endif //P6ENGINE_UTILS_H
