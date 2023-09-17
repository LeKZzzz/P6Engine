//
// Created by LeK on 2023/5/11.
//


#ifndef P6ENGINE_POBJECTMANAGER_H
#define P6ENGINE_POBJECTMANAGER_H

#include "Basic.h"
#include "PObject.h"
#include "Anchor.h"
#include "types.h"
#include "iostream"
#include "list"
#include "utils.h"

using namespace std;

//�����������ʹ���ͬ�������Ϊһ�����������й���
class PObjectManager: public Basic{
public:
    //���ݶȣ���������ж���������Ƿ��ڹ�����ʱ��Ч����ֵԽ��Խ�����ױ��������ʧ����������۴�
    float tolerance_radian= degree_to_radian(25);

    PObjectManager(bool logic = false);

    void on(PObject *);    //ע�����
    void on(Bone *);    //ע�����

    void off(Basic *, bool= CANCEL_AUTO);

    void on_only_auto(Basic *);  //��ע����߼�����/�������Ը��¶���

    void move(increment, int speed = ALL); //�ƶ�

    void rotate(angle, int speed = ALL);   //��ת

    void update();  //״̬����

    void set_tolerance(float set_tolerance) { tolerance_radian= degree_to_radian(set_tolerance); }

    vertex_v get_origin() { return emptyobject->get_origin(); }

protected:
    int objnums, bnums;
    EmptyObjet *emptyobject;   //�ն����
    list<Basic *> autolist; //�Ը��¶���
    friend class PWorld;
    friend class Camera;
};


PObjectManager::PObjectManager(bool logic) {
    emptyobject = new EmptyObjet;
    objnums = 0;
    bnums = 0;
    logic_flag = logic;
}


void PObjectManager::on(PObject *obj) {
    //�������ƶ����ʵ��ľ�������
    vertex_v origin = emptyobject->get_origin();
    angle angles = emptyobject->get_angle();
    increment increments = {origin.x, origin.y, origin.z};
    if (origin.x != 0 || origin.y != 0 || origin.z != 0)
        obj->move(increments);
    if (angles.x != 0 || angles.y != 0 || angles.z != 0)
        obj->rotate(origin, angles);
    emptyobject->on(obj);   //����ն������
    if (obj->logic_flag) autolist.push_back(obj);   //�����Ը��¶���
}


void PObjectManager::on(Bone *bone) {
    emptyobject->on(bone);
    if (bone->logic_flag) autolist.push_back(bone);
}

void PObjectManager::off(Basic *obj, bool flag) {
    emptyobject->off(obj);
    if (flag == CANCEL_AUTO)
        for (auto i = autolist.begin(); i != autolist.end(); i++)
            if (*i == obj) autolist.erase(i);
}

void PObjectManager::move(increment increments, int speed) {
    emptyobject->move(increments, speed);
}

void PObjectManager::rotate(angle angles, int speed) {
    emptyobject->rotate(angles, speed);
}

void PObjectManager::on_only_auto(Basic *obj) {
    autolist.push_back(obj);
}

void PObjectManager::update() {
    for (auto i = autolist.begin(); i != autolist.end(); i++)
        (*i)->logic();
    emptyobject->update_bones();
    if (logic_flag)
        logic();
}

#endif //P6ENGINE_POBJECTMANAGER_H
