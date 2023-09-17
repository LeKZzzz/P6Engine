//
// Created by LeK on 2023/5/11.
//

#ifndef P6ENGINE_CAMERA_H
#define P6ENGINE_CAMERA_H

#include "Basic.h"
#include "types.h"
#include "utils.h"
#include "PObjectManager.h"
#include "vector"
#include "PObjectManager.h"
#include <Windows.h>
#include <algorithm>

class Camera : public Basic {
public:

    Camera(int, int, float, vertex_v, float, int buffer_nums = 2, bool fov_type = VERTICAL_FOV, bool logic = false,
           vertex_vn set_vn = {0, 0, 1});

    ~Camera();

    void set_fov(float, bool fov_type = VERTICAL_FOV);  //设置fov

    void update(list<PObjectManager *> *, list<PUI *> *);  //更新缓冲区

    void show(list<PObjectManager *> *, list<PUI *> *, HANDLE);    //成像

    void move(increment);   //移动摄像机

    void rotate(angle); //旋转摄像机

    void clear_screen(HANDLE);  //清空缓冲区

protected:

    float fov_vertical, fov_horizontal;  //视场角
    int screen_length, screen_width;   //光屏长宽
    float visibility;   //能见度
    vertex_v origin;    //摄像机原点
    vertex_vn vn;   //光屏法向量
    float z_rotation;   //z轴旋转角度
    int buffer_num, buffer_index;
    vector<buffer_node> *frame_buffer, **back_up;
    HANDLE *buffer_handle;
    COORD base;

    friend class CameraManager;
};

Camera::Camera(int length, int width, float fov, vertex_v v, float set_visibility, int buffer_nums, bool fov_type,
               bool logic, vertex_vn set_vn) {
    screen_length = length;
    screen_width = width;
    buffer_num = buffer_nums;
    buffer_index = 0;
    frame_buffer = new vector<buffer_node>[width];
    back_up = new vector<buffer_node> *[buffer_num];
    buffer_handle = new HANDLE[buffer_num];
    for (int i = 0; i < buffer_num; ++i) {
        back_up[i] = new vector<buffer_node>[width];
        buffer_handle[i] = nullptr;
    }
    if (fov_type == VERTICAL_FOV) {
        fov_vertical = degree_to_radian(fov);
        fov_horizontal = fov_exchange(fov_vertical, width, length, VERTICAL_FOV);
    } else if (fov_type == HORIZONTAL_FOV) {
        fov_horizontal = degree_to_radian(fov);
        fov_vertical = fov_exchange(fov_horizontal, width, length, HORIZONTAL_FOV);
    }
    origin = v;
    logic_flag = logic;
    base = {0, 0};
    visibility = set_visibility;
    vn = set_vn;
    z_rotation = 0;
}

void Camera::set_fov(float fov, bool fov_type) {
    if (fov_type == VERTICAL_FOV) {
        fov_vertical = fov;
        fov_horizontal = fov_exchange(fov, screen_width, screen_length, VERTICAL_FOV);
    } else if (fov_type == HORIZONTAL_FOV) {
        fov_horizontal = fov;
        fov_vertical = fov_exchange(fov, screen_width, screen_length, HORIZONTAL_FOV);
    }
}

void Camera::update(list<PObjectManager *> *managerlist, list<PUI *> *ui_list) {
    list<PObjectManager *> perhaps_in_sight;
    vertex_v origin_v;
    float xn, yn, zn;
    float distance;

    for (auto it = managerlist->begin(); it != managerlist->end(); it++) {  //寻找可能成像的管理器
        origin_v = (*it)->get_origin();
        distance = get_distance(origin, origin_v);
        if (distance <= visibility <= visibility) {
            xn = origin_v.x - origin.x;
            yn = origin_v.y - origin.y;
            zn = origin_v.z - origin.z;
            if (xn == 0.0 && yn == 0.0 && zn == 0.0)
                perhaps_in_sight.push_back(*it);
            else if (get_included_angle(vn, {xn, yn, zn}, XOZ) <= (fov_horizontal / 2 + (*it)->tolerance_radian) &&
                     get_included_angle(vn, {xn, yn, zn}, YOZ) <= (fov_vertical / 2 + (*it)->tolerance_radian)) {
                perhaps_in_sight.push_back(*it);
            }
        }
    }

    for (auto it = perhaps_in_sight.begin(); it != perhaps_in_sight.end(); it++) {  //遍历管理器
        for (auto obj = (*it)->emptyobject->objlist.begin();
             obj != (*it)->emptyobject->objlist.end(); obj++) { //遍历管理器中的对象
            for (int i = 0; i < (*obj)->fnums; ++i) {   //遍历对象的面
                for (int j = 0; j < (*obj)->f[i].v.size(); ++j) {   //遍历面的顶点
                    xn = (*obj)->v[(*obj)->f[i].v[j]].x - origin.x;
                    yn = (*obj)->v[(*obj)->f[i].v[j]].y - origin.y;
                    zn = (*obj)->v[(*obj)->f[i].v[j]].z - origin.z;
                    if (get_included_angle(vn, {xn, yn, zn}, XOZ) <= (fov_horizontal / 2 + (*it)->tolerance_radian) &&
                        get_included_angle(vn, {xn, yn, zn}, YOZ) <=
                        (fov_vertical / 2 + (*it)->tolerance_radian)) {    //TODO:顶点在视线范围内(待商榷)

                        draw_surface(frame_buffer, (*obj)->v, (*obj)->f[i], (*obj)->show_method, screen_length,
                                     screen_width, fov_horizontal,
                                     fov_vertical, vn, origin, z_rotation);
                        break;
                    }
                }
            }
        }

        for (auto bone = (*it)->emptyobject->bonelist.begin();
             bone != (*it)->emptyobject->bonelist.end(); bone++) { //遍历管理器中的骨骼
            for (auto skin = (*bone)->skin_list.begin(); skin != (*bone)->skin_list.end(); skin++) {
                for (int i = 0; i < (*skin)->fnums; ++i) {   //遍历蒙皮的面
                    for (int j = 0; j < (*skin)->f[i].v.size();j++) {
                        xn = (*skin)->v[(*skin)->f[i].v[j]].x - origin.x;
                        yn = (*skin)->v[(*skin)->f[i].v[j]].y - origin.y;
                        zn = (*skin)->v[(*skin)->f[i].v[j]].z - origin.z;
                        if (get_included_angle(vn, {xn, yn, zn}, XOZ) <=
                            (fov_horizontal / 2 + (*it)->tolerance_radian) &&
                            get_included_angle(vn, {xn, yn, zn}, YOZ) <= (fov_vertical / 2 + (*it)->tolerance_radian)) {

                            draw_surface(frame_buffer, (*skin)->v, (*skin)->f[i], (*skin)->show_method, screen_length,
                                         screen_width,
                                         fov_horizontal, fov_vertical, vn, origin, z_rotation);
                        }
                    }
                }
            }
        }
    }

    for (auto it = ui_list->begin(); it != ui_list->end(); it++) {
        draw_ui(frame_buffer, &(*it)->ui, screen_length, screen_width);
    }
}

void Camera::show(list<PObjectManager *> *managerlist, list<PUI *> *ui_list, HANDLE out_buffer) {
    DWORD recnum = 0;
    DWORD bytes = 0;

    update(managerlist, ui_list);

    buffer_handle[buffer_index] = out_buffer;
    for (int i = 0; i < screen_width; ++i) {
        if (frame_buffer[i].empty()) continue;
        for (auto it = frame_buffer[i].begin(); it != frame_buffer[i].end(); it++) {
            SetConsoleCursorPosition(out_buffer, (*it).site);
            WriteConsole(out_buffer, &(*it).mtl, 1, &recnum, NULL);
//            WriteConsoleOutputCharacterA(out_buffer, &(*it).mtl, 1, (*it).site, &bytes);
        }
        back_up[buffer_index][i] = frame_buffer[i];
        frame_buffer[i].clear();
    }
    SetConsoleActiveScreenBuffer(out_buffer);
    buffer_index = (buffer_index + 1) % buffer_num;
    if (buffer_handle[buffer_index] != nullptr)
        clear_screen(buffer_handle[buffer_index]);

}

void Camera::move(increment increments) {
    origin.x += increments.x;
    origin.y += increments.y;
    origin.z += increments.z;
}

void Camera::clear_screen(HANDLE out_buffer) {
    DWORD recnum = 0;
    char *cls = " ";
    for (int i = 0; i < screen_width; ++i) {
        if (back_up[buffer_index][i].empty()) continue;
        for (auto it = back_up[buffer_index][i].begin(); it != back_up[buffer_index][i].end(); it++) {
            SetConsoleCursorPosition(out_buffer, (*it).site);
            WriteConsole(out_buffer, cls, 1, &recnum, NULL);
//            WriteConsoleOutputCharacterA(out_buffer, &(*it).mtl, 1, (*it).site, &bytes);
        }
    }
}

void Camera::rotate(angle angles) {
    util_rotate({0, 0, 0}, vn, angles);
    z_rotation += angles.z;
}

Camera::~Camera() {
    delete[]    frame_buffer;
    delete[]    back_up;
}

#endif //P6ENGINE_CAMERA_H
