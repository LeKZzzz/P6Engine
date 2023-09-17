//
// Created by LeK on 2023/9/3.
//

#ifndef P6ENGINE_BASIC_H
#define P6ENGINE_BASIC_H


#include <objbase.h>

class Basic {
public:
    bool logic_flag = false;

    virtual void logic() {};    //对象自身逻辑函数
    Basic() {
        ::CoCreateGuid(&id);
    }

    GUID get_id() const { return id; }

protected:
    void print_id();

private:
    GUID id;    //对象id
};

void Basic::print_id() {
    printf(
            "{%08X-%04X-%04x-%02X%02X-%02X%02X%02X%02X%02X%02X}\n", id.Data1, id.Data2, id.Data3, id.Data4[0],
            id.Data4[1], id.Data4[2], id.Data4[3], id.Data4[4], id.Data4[5], id.Data4[6], id.Data4[7]
    );
}

#endif //P6ENGINE_BASIC_H
