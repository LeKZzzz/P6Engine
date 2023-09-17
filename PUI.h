//
// Created by LeK on 2023/5/11.
//

#ifndef P6ENGINE_PUI_H
#define P6ENGINE_PUI_H

#include "types.h"
#include "fstream"
#include "sstream"

class PUI {
public:
    PUI(int set_priority = 1) { priority = set_priority; };

    void load(const string &);  //导入UI

    void clear() { ui.clear(); }

    void set_priority(int set_priority) { priority = set_priority; }

protected:
    vector<ui_node> ui;
    int priority;
    friend class Camera;
};

void PUI::load(const string &filename) {
    ui_node newnode;
    char pathbuff[1024];
    string filepath, line;
    ifstream ifile;
    stringstream sstream;
    regex rsplit(" ");

    _getcwd(pathbuff, 1024);
    filepath = pathbuff;
    ifile.open(filepath + "\\" + filename, ios::in);
    if (!ifile.is_open()) {
        ::perror("文件打开失败");
        exit(1);
    }

    clear();
    while (getline(ifile, line)) {
        sregex_token_iterator beg(line.begin(), line.end(), rsplit, -1); //传入-1，代表对匹配到的分隔符之间的内容感兴趣

        sstream << beg->str();
        sstream >> newnode.begin.X;
        beg++;
        sstream.clear();
        sstream << beg->str();
        sstream >> newnode.begin.Y;
        beg++;
        sstream.clear();
        sstream << beg->str();
        sstream >> newnode.end.X;
        beg++;
        sstream.clear();
        sstream << beg->str();
        sstream >> newnode.end.Y;
        beg++;
        sstream.clear();
        sstream << beg->str();
        sstream >> newnode.mtl;
        beg++;
        sstream.clear();

        newnode.priority = priority;
        ui.push_back(newnode);
    }
    ifile.close();
}

#endif P6ENGINE_PUI_H
