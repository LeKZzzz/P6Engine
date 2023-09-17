//
// Created by LeK on 2023/5/11.
//

#ifndef P6ENGINE_CAMERAMANAGER_H
#define P6ENGINE_CAMERAMANAGER_H

#include "Basic.h"
#include "Camera.h"
#include "list"
#include "PObjectManager.h"
#include "PUI.h"
#include <Windows.h>

using namespace std;

class CameraManager:public Basic {
public:
    CameraManager();

    void on(Camera *);

    void show(list<PObjectManager *> *, list<PUI *> *, HANDLE);


protected:
    list<Camera *> camera_list;
};

CameraManager::CameraManager() {
}

void CameraManager::on(Camera *new_camera) {
    camera_list.push_back(new_camera);
}

void CameraManager::show(list<PObjectManager *> *manager_list, list<PUI *> *ui_list, HANDLE out_buffer) {
    for (auto it = camera_list.begin(); it != camera_list.end(); it++) {
        if ((*it)->logic_flag)
            (*it)->logic();
        (*it)->show(manager_list,ui_list, out_buffer);
    }
}


#endif //P6ENGINE_CAMERAMANAGER_H
