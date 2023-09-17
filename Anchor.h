//
// Created by LeK on 2023/5/11.
//

#ifndef P6ENGINE_ANCHOR_H
#define P6ENGINE_ANCHOR_H

#include "Basic.h"
#include "PObject.h"
#include "types.h"
#include "list"
#include "queue"
#include "utils.h"
#include <iostream>

using namespace std;

class PObjectManager;

//同一个根结点的骨骼
struct BoneNode {
    vertex_v v;
    BoneNode *pre;  //父节点
    BoneNode *lnext;    //子节点
    BoneNode *rnext;    //兄弟节点
    PObject *skin;  //蒙皮
    vector<BoneNode *> last_change_node; //上一个骨骼运动的起始节点记录
    increment increments;    //相对于导入时的增量
    vector<angle> angles;   //相对于导入时的旋转记录
};

struct msg_ds {
    BoneNode *base_node_start; //动作起始骨骼的开始节点
    BoneNode *base_node_end;   //动作起始骨骼的结束节点
    int type;   // 消息类型 ROTATE / MOVE
    angle rotation; //旋转角度
    increment increment;    //移动增量
    int speed;  //影响传播速率
    queue<BoneNode *> curnode;  //当前影响已传播到的节点
};

//TODO：存在节点冲突：起始节点不同而终止节点相同，寻找节点时可能会在错误的骨骼链
//根据节点坐标查找骨骼节点
BoneNode *bnfind(BoneNode *root, vertex_v v) {
    stack<BoneNode *> stack;
    BoneNode *tmp;
    stack.push(root);
    while (!stack.empty()) {
        while (stack.top()) stack.push(stack.top()->lnext);
        stack.pop();
        if (!stack.empty()) {
            tmp = stack.top();
            if (tmp->v.x == v.x && tmp->v.y == v.y && tmp->v.z == v.z ) return tmp;
            stack.pop();
            stack.push(tmp->rnext);
        }
    }
    return nullptr;
}

//根据蒙皮查找骨骼节点
BoneNode *bnfind(BoneNode *root, PObject *skin) {
    stack<BoneNode *> stack;
    BoneNode *tmp;
    stack.push(root);
    while (!stack.empty()) {
        while (stack.top()) stack.push(stack.top()->lnext);
        stack.pop();
        if (!stack.empty()) {
            tmp = stack.top();
            if (tmp->skin == skin) return tmp;
            stack.pop();
            stack.push(tmp->rnext);
        }
    }
    return nullptr;
}

//向序列中添加节点
template<typename T>
void push_node(BoneNode *root, T &sequence, int depth) {
    queue<BoneNode *> tmp;
    BoneNode *p;
    unsigned short n;
    if (depth < 1) return;
    p = root;
    if (p->lnext == nullptr) return;
    tmp.push(p->lnext);
    sequence.push(p->lnext);
    p = p->lnext;
    while (p->rnext != nullptr) {
        tmp.push(p->rnext);
        sequence.push(p->rnext);
        p = p->rnext;
    }
    for (int i = 0; i < depth - 1; ++i) {
        n = tmp.size();
        for (int j = 0; j < n; ++j) {
            p = tmp.front();
            tmp.pop();
            if (p->lnext == nullptr) continue;
            tmp.push(p->lnext);
            sequence.push(p->lnext);
            p = p->lnext;
            while (p->rnext != nullptr) {
                tmp.push(p->rnext);
                sequence.push(p->rnext);
                p = p->rnext;
            }
        }
    }
}

//骨骼类
class Bone: public Basic{
public:

    explicit Bone(vertex_v);

    void on(PObject *, vertex_v, vertex_v);  //注册对象

    void update();  //处理消息队列

    void move(vertex_v, vertex_v, increment, int speed = ALL); //根据骨骼节点信息移动
    void move(PObject *, increment, int speed = ALL);    //根据蒙皮信息移动
    void move(increment, int speed = ALL);   //从根节点开始移动

    void rotate(vertex_v, vertex_v, angle, int speed = ALL);   //根据骨骼节点信息旋转
    void rotate(PObject *, angle, int speed = ALL);  //根据蒙皮信息旋转
    void rotate(angle, int speed = ALL); //从根节点开始旋转

    BoneNode *separate(vertex_v, vertex_v); //根据骨骼节点信息分离骨骼
    BoneNode *separate(PObject *);  //根据蒙皮信息分离骨骼

    void setskin(vertex_v, vertex_v, PObject *);    //根据骨骼节点信息设置蒙皮
    void setskin(PObject *, PObject *);    //根据骨骼节点信息设置蒙皮

protected:
    int nodenums;
    int layer;  //骨骼层数
    BoneNode *BoneTree; //节点
    list<msg_ds> msg_list;  //事件消息队列
    list<PObject *> skin_list;  //蒙皮列表
    friend class PObjectManager;
    friend class Camera;
};


Bone::Bone(vertex_v base) {
    nodenums = 1;
    layer = 1;
    BoneTree = new BoneNode;
    BoneTree->v = base;
    BoneTree->pre = BoneTree->lnext = BoneTree->rnext = nullptr;
    BoneTree->increments = {0, 0, 0};
    BoneTree->skin = nullptr;
}


void Bone::on(PObject *skin, vertex_v start_v, vertex_v end_v) {
    auto *newbone = new BoneNode;
    BoneNode *tmp;
    newbone->v = end_v;
    newbone->lnext = newbone->rnext = nullptr;
    newbone->skin = skin;
    newbone->increments = {0, 0, 0};
    skin_list.push_back(skin);

    tmp = bnfind(BoneTree, start_v);
    newbone->pre = tmp;

    if (tmp->lnext == nullptr) {
        tmp->lnext = newbone;
        layer++;
    } else {
        while (tmp->rnext) tmp = tmp->rnext;
        tmp->rnext = newbone;
    }

    skin->move(newbone->pre->increments);
    for (int i = 0; i < newbone->pre->last_change_node.size(); ++i) //重复旋转过程
        skin->rotate(newbone->pre->last_change_node[i]->v, newbone->pre->angles[i]);
    newbone->last_change_node = newbone->pre->last_change_node;
    newbone->angles = newbone->pre->angles;

    nodenums++;
}


BoneNode *Bone::separate(vertex_v start, vertex_v end) {
    BoneNode *tmp, *p;
    tmp = bnfind(BoneTree, start);
    tmp = bnfind(tmp, end);
    if (tmp->pre->lnext == tmp) {
        if (tmp->rnext != nullptr) {
            tmp->pre->lnext = tmp->rnext;
        } else {
            tmp->pre->lnext = nullptr;
        }
    } else {
        p = tmp->pre->lnext;
        while (p->rnext != tmp) p = p->rnext;
        p->rnext = tmp->rnext;
    }
    tmp->pre = nullptr;
    tmp->rnext = nullptr;
    return tmp;
}

BoneNode *Bone::separate(PObject *skin) {
    BoneNode *tmp, *p;
    tmp = bnfind(BoneTree, skin);
    if (tmp->pre->lnext == tmp) {
        if (tmp->rnext != nullptr) {
            tmp->pre->lnext = tmp->rnext;
        } else {
            tmp->pre->lnext = nullptr;
        }
    } else {
        p = tmp->pre->lnext;
        while (p->rnext != tmp) p = p->rnext;
        p->rnext = tmp->rnext;
    }
    tmp->pre = nullptr;
    tmp->rnext = nullptr;
    return tmp;
}

void Bone::move(vertex_v start_v, vertex_v end_v, increment increments, int speed) {
    msg_ds msg;
    msg.base_node_start = bnfind(BoneTree, start_v);
    msg.base_node_end = bnfind(msg.base_node_start, end_v);
    msg.type = MOVE;
    msg.increment = increments;
    msg.speed = speed;
    if (speed == ALL)
        push_node(msg.base_node_start, msg.curnode, layer);
    else
        push_node(msg.base_node_start, msg.curnode, speed);
    msg_list.push_back(msg);
}

void Bone::move(PObject *skin, increment increments, int speed) {
    msg_ds msg;
    msg.base_node_end = bnfind(BoneTree, skin);
    msg.base_node_start = msg.base_node_end->pre;
    msg.type = MOVE;
    msg.increment = increments;
    msg.speed = speed;
    if (speed == ALL)
        push_node(msg.base_node_start, msg.curnode, layer);
    else
        push_node(msg.base_node_start, msg.curnode, speed);
    msg_list.push_back(msg);
}

void Bone::move(increment increments, int speed) {
    msg_ds msg;
    msg.base_node_start = BoneTree;
    msg.type = MOVE;
    msg.increment = increments;
    msg.speed = speed;
    if (speed == ALL)
        push_node(msg.base_node_start, msg.curnode, layer);
    else
        push_node(msg.base_node_start, msg.curnode, speed);
    msg_list.push_back(msg);
}

void Bone::rotate(vertex_v start_v, vertex_v end_v, angle angles, int speed) {
    msg_ds msg;
    msg.base_node_start = bnfind(BoneTree, start_v);
    msg.base_node_end = bnfind(msg.base_node_start, end_v);
    msg.type = ROTATE;
    msg.rotation = angles;
    msg.speed = speed;
    if (speed == ALL)
        push_node(msg.base_node_start, msg.curnode, layer);
    else
        push_node(msg.base_node_start, msg.curnode, speed);
    msg_list.push_back(msg);
}

void Bone::rotate(PObject *skin, angle angles, int speed) {
    msg_ds msg;
    msg.base_node_end = bnfind(BoneTree, skin);
    msg.base_node_start = msg.base_node_end->pre;
    msg.type = ROTATE;
    msg.rotation = angles;
    msg.speed = speed;
    if (speed == ALL)
        push_node(msg.base_node_start, msg.curnode, layer);
    else
        push_node(msg.base_node_start, msg.curnode, speed);
    msg_list.push_back(msg);
}

void Bone::rotate(angle angles, int speed) {
    msg_ds msg;
    msg.base_node_start = BoneTree;
    msg.type = ROTATE;
    msg.rotation = angles;
    msg.speed = speed;
    if (speed == ALL)
        push_node(msg.base_node_start, msg.curnode, layer);
    else
        push_node(msg.base_node_start, msg.curnode, speed);
    msg_list.push_back(msg);
    update();
}

void Bone::setskin(vertex_v start, vertex_v end, PObject *newskin) {
    BoneNode *tmp;
    tmp = bnfind(BoneTree, start);
    tmp = bnfind(tmp, end);
    tmp->skin = newskin;
    newskin->move(tmp->increments);
    for (int i = 0; i < tmp->last_change_node.size(); ++i) //重复旋转过程
        newskin->rotate(tmp->last_change_node[i]->v, tmp->angles[i]);
}

void Bone::setskin(PObject *oldskin, PObject *newskin) {
    BoneNode *tmp;
    tmp = bnfind(BoneTree, oldskin);
    tmp->skin = newskin;
    newskin->move(tmp->increments);
    for (int i = 0; i < tmp->last_change_node.size(); ++i) //重复旋转过程
        newskin->rotate(tmp->last_change_node[i]->v, tmp->angles[i]);
}

void Bone::update() {
    if (msg_list.empty()) return;
    auto it1 = msg_list.begin();    //消息
    unsigned int n;
    int t = 0;
    BoneNode *it2;
    for (; it1 != msg_list.end(); it1++) {  //遍历消息
        n = it1->curnode.size();
        if (it1->type == MOVE) {
            for (int i = 0; i < n; ++i) {
                it2 = it1->curnode.front(); //骨骼节点
                it2->v.x += it1->increment.x;
                it2->v.y += it1->increment.y;
                it2->v.z += it1->increment.z;
                it2->skin->move(it1->increment);
                it2->increments.x += it1->increment.x;
                it2->increments.y += it1->increment.y;
                it2->increments.z += it1->increment.z;
                it1->curnode.pop();
            }
        } else if (it1->type == ROTATE) {
            for (int i = 0; i < n; ++i) {
                it2 = it1->curnode.front(); //骨骼节点
                util_rotate(it1->base_node_start->v, it2->v, it1->rotation);
                it2->angles.push_back(it1->rotation);
                it2->last_change_node.push_back(it1->base_node_start);
                it2->skin->rotate(it1->base_node_start->v, it1->rotation);
                it1->curnode.pop();
            }
        }
        push_node(it2, it1->curnode, it1->speed);
        if (it1->curnode.empty()) t++;
    }
    //删除已结束的消息
    for (int i = 0; i < t; ++i) {
        for (auto j = msg_list.begin(); j != msg_list.end(); j++) {
            if (j->curnode.empty()) {
                msg_list.erase(j);
                break;
            }
        }
    }
}





//空对象

class EmptyObjet:public Basic {
public:
    EmptyObjet();

    //注册对象/骨骼
    void on(PObject *);

    void on(Bone *);

    vertex_v get_origin() { return origin; }

    angle get_angle() { return origin_angles; }

    void off(Basic *);

    void move(increment, int speed = ALL); //移动所有的对象和骨骼

    void rotate(angle, int speed = ALL);   //旋转所有的对象和骨骼

    void update_bones();    //更新骨骼状态

protected:
    vertex_v origin;
    angle origin_angles{0, 0, 0};
    int objnums, bonenums;
    list<PObject *> objlist;
    list<Bone *> bonelist;
    friend class Camera;
};


EmptyObjet::EmptyObjet() : origin({0, 0, 0}) {
    objnums = bonenums = 0;
    origin_angles = {0, 0, 0};
}


void EmptyObjet::on(Bone *bone) {
    bonelist.push_back(bone);
    bonenums++;
}


void EmptyObjet::on(PObject *object) {
    objlist.push_back(object);
    objnums++;
}

//TODO:记得更新骨骼
void EmptyObjet::update_bones() {
    for (auto i = bonelist.begin(); i != bonelist.end(); i++)
        (*i)->update();
}

void EmptyObjet::off(Basic *obj) {
    for (auto i = bonelist.begin(); i != bonelist.end(); i++)
        if ((*i)->get_id() == obj->get_id()) bonelist.erase(i);
}

void EmptyObjet::move(increment increments, int speed) {
    for (auto i = objlist.begin(); i != objlist.end(); i++)
        (*i)->move(increments);
    for (auto i = bonelist.begin(); i != bonelist.end(); i++)
        (*i)->move(increments, speed);
    origin.x += increments.x;
    origin.y += increments.y;
    origin.z += increments.z;
}

void EmptyObjet::rotate(angle angles, int speed) {
    for (auto i = objlist.begin(); i != objlist.end(); i++)
        (*i)->rotate(origin, angles);
    for (auto i = bonelist.begin(); i != bonelist.end(); i++)
        (*i)->rotate(angles, speed);
    origin_angles = angles;
}


#endif //P6ENGINE_ANCHOR_H
