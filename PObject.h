//
// Created by LeK on 2023/5/12.
//

//基础对象
#ifndef P6ENGINE_POBJECT_H
#define P6ENGINE_POBJECT_H

#include "Basic.h"
#include "string"
#include "sstream"
#include "fstream"
#include "types.h"
#include "vector"
#include "iostream"
#include "cmath"
#include "utils.h"
#include  <direct.h>

using namespace std;

class PObject : public Basic {
public:

    PObject();

    void load(const string &, const string &);  //导入模型

    void clear();   //清空当前对象的蒙皮

    void bind();

    void set_show_method(int);  //整体呈现方式

    void set_transparency(int); //整体透明度

    void set_surface_show_method(int, int); //面呈现方式

    void set_surface_transparency(int, int);    //面透明度

    void move(increment); //移动自身

    void rotate(vertex_v, angle);   //旋转自身

    void display(); //打印模型数据(调试用)


protected:
    string objname;
    vector<vertex_v> v; //v顶点
    vector<vertex_vt> vt;   //vt顶点
    vector<vertex_vn> vn;   //vn顶点
    vector<surface> f;  //面
    int vnums, vtnums, vnnums, fnums, prevnums, prevtnums, prevnnums, prefnums, pres;
    float tolerance;    //宽容度
    vertex_v center;    //中心点
    bool show_method; //整体呈现方式
    bool transparency;   //透明度

    friend class PObjectManager;

    friend class Bone;

    friend class EmptyObjet;

    friend class Camera;
};


PObject::PObject() : center({0, 0, 0}) {
    vnums = vtnums = vnnums = fnums = 0;
    prevnums = prevtnums = prevnnums = prefnums = pres = 0;
    tolerance = 0.0;
    show_method = FRAME;
    transparency = OPAQUE;
}


void PObject::load(const string &filename, const string &objectname) {
    vertex_v lv = {0, 0, 0}; //v顶点
    vertex_vt lvt = {0, 0, 0};   //vt顶点
    vertex_vn lvn = {0, 0, 0};   //vn顶点
    surface lf;  //面
    ifstream ifile, mtl_ifile;
    stringstream sstream;
    string line, checkline, defaultcheckline = "^# object ", type, stmp, filepath, tmpline, tmpstr;
    char pathbuff[1024], premtl;
    int itmp;
    float tmpx, tmpy, tmpz, x = 0.0, y = 0.0, z = 0.0;

    checkline = defaultcheckline + objectname + "$";
    regex rcheckline(checkline), rsplit1(" "), rsplit2("/"), rmtlcheckline("^mtllib.*");

    _getcwd(pathbuff, 1024);
    filepath = pathbuff;
    ifile.open(filepath + "\\" + filename, ios::in);
    if (!ifile.is_open()) {
        ::perror("文件打开失败");
        exit(1);
    }

    clear();
    while (getline(ifile, line)) {
        if (regex_match(line, rmtlcheckline)) {
            sregex_token_iterator beg(line.begin(), line.end(), rsplit1, -1);
            beg++;
            mtl_ifile.open(filepath + "\\" + beg->str());
            continue;
        }
        if (regex_match(line, rcheckline))
            break;
        if (line[0] == 'v') {
            if (line[1] == 't')
                prevtnums++;
            else if (line[1] == 'n')
                prevnnums++;
            else
                prevnums++;
        } else if (line[0] == 'f') prefnums++;
    }

    getline(ifile, line);
    rcheckline.assign(defaultcheckline + ".*");
    while (!regex_match(line, rcheckline) && !ifile.eof()) {
        tmpx = tmpy = tmpz = 0.0;
        sregex_token_iterator beg(line.begin(), line.end(), rsplit1, -1); //传入-1，代表对匹配到的分隔符之间的内容感兴趣
        sregex_token_iterator end; //结束标志
        type = beg->str();
        if (type == "v") {
            beg++;
            beg++;
            sstream << beg->str();
            sstream >> lv.x;
            if (lv.x > tolerance) tolerance = lv.x;
            sstream.clear();
            beg++;
            sstream << beg->str();
            sstream >> lv.y;
            if (lv.y > tolerance) tolerance = lv.y;
            sstream.clear();
            beg++;
            sstream << beg->str();
            sstream >> lv.z;
            if (lv.z > tolerance) tolerance = lv.z;
            sstream.clear();

            v.push_back(lv);
            vnums++;
        } else if (type == "vt") {
            beg++;
            sstream << beg->str();
            sstream >> lvt.x;
            sstream.clear();
            beg++;
            sstream << beg->str();
            sstream >> lvt.y;
            sstream.clear();
            beg++;
            sstream << beg->str();
            sstream >> lvt.z;
            sstream.clear();

            vt.push_back(lvt);
            vtnums++;
        } else if (type == "vn") {
            beg++;
            sstream << beg->str();
            sstream >> lvn.x;
            sstream.clear();
            beg++;
            sstream << beg->str();
            sstream >> lvn.y;
            sstream.clear();
            beg++;
            sstream << beg->str();
            sstream >> lvn.z;
            sstream.clear();

            vn.push_back(lvn);
            vnnums++;
        } else if (type == "f") {
            beg++;
            for (; beg != end; beg++) {
                stmp = beg->str();
                sregex_token_iterator fbeg(stmp.begin(), stmp.end(), rsplit2, -1); //传入-1，代表对匹配到的分隔符之间的内容感兴趣
                sstream << fbeg->str();
                sstream >> itmp;
                itmp > 0 ? lf.v.push_back(itmp - prevnums - 1) : lf.v.push_back(vnums + itmp);
                sstream.clear();
                fbeg++;
                if (fbeg->length() != 0) {
                    sstream << fbeg->str();
                    sstream >> itmp;
                    itmp > 0 ? lf.vt.push_back(itmp - prevtnums - 1) : lf.vt.push_back(vtnums + itmp);
                    sstream.clear();
                }
                fbeg++;
                if (fbeg->length() != 0) {
                    sstream << fbeg->str();
                    sstream >> itmp;
                    itmp > 0 ? lf.vn.push_back(itmp - prevnnums - 1) : lf.vn.push_back(vnnums + itmp);
                    sstream.clear();
                }
            }
            lf.mtl = premtl;
            lf.s = pres;
            lf.show_mehtod = show_method;
            lf.transparency = transparency;
            f.push_back(lf);
            fnums++;
            for (int i: lf.v) {
                tmpx += v[i].x;
                tmpy += v[i].y;
                tmpz += v[i].z;
            }
            x += tmpx / float(lf.v.size());
            y += tmpy / float(lf.v.size());
            z += tmpz / float(lf.v.size());
            lf.v.clear();
            lf.vt.clear();
            lf.vn.clear();
        } else if (type == "s") {
            beg++;
            if (beg->str() == "off") {
                pres = -1;
            } else {
                sstream << beg->str();
                sstream >> pres;
                sstream.clear();
            }

        } else if (type == "usemtl") {
            beg++;
            tmpstr = beg->str();
            while (getline(mtl_ifile, tmpline)) {
                if (tmpline == "newmtl " + tmpstr) {
                    getline(mtl_ifile, tmpline);
                    premtl = tmpline[0];
                    break;
                }

            }
        }
        getline(ifile, line);
    }

    ifile.close();
    mtl_ifile.close();
    objname = objectname;
    center.x = x / float(fnums);
    center.y = y / float(fnums);
    center.z = z / float(fnums);
}


void PObject::bind() {

}


void PObject::clear() {
    vnums = vtnums = vnnums = fnums = 0;
    prevnums = prevtnums = prevnnums = prefnums = pres = 0;
    center = {0, 0, 0};
    tolerance = 0.0;
    show_method = FRAME;
    transparency = OPAQUE;
    v.clear();
    vt.clear();
    vn.clear();
    f.clear();
}

void PObject::set_show_method(int new_method) {
    show_method = new_method;
}

void PObject::set_transparency(int new_transparency) {
    transparency = new_transparency;
}


void PObject::set_surface_show_method(int n, int method) {
    f[n - 1].show_mehtod = method;
}

void PObject::set_surface_transparency(int n, int new_transparency) {
    f[n - 1].transparency = new_transparency;
}


void PObject::display() {
    print_id();
    cout << "v:" << endl;
    for (int i = 0; i < vnums; ++i) {
        cout << v[i].x << " " << v[i].y << " " << v[i].z << endl;
    }
    cout << "vt:" << endl;
    for (int i = 0; i < vtnums; ++i) {
        cout << vt[i].x << " " << vt[i].y << " " << vt[i].z << endl;
    }
    cout << "vn:" << endl;
    for (int i = 0; i < vnnums; ++i) {
        cout << vn[i].x << " " << vn[i].y << " " << vn[i].z << endl;
    }
    cout << "f:" << endl;
    for (int i = 0; i < fnums; ++i) {
        for (int j = 0; j < (f[i].v.end() - f[i].v.begin()); ++j) {
            cout << f[i].v[j] << " ";
        }
        cout << endl << "mtl:" << f[i].mtl << endl;
        cout << "s:" << f[i].s << endl;
    }
    cout << "center:" << endl;
    cout << center.x << " " << center.y << " " << center.z << endl;
}

void PObject::move(increment increments) {
    for (int i = 0; i < vnums; ++i) {
        v[i].x += increments.x;
        v[i].y += increments.y;
        v[i].z += increments.z;
    }
}

void PObject::rotate(vertex_v origin, angle angles) {
    for (int i = 0; i < vnums; ++i) {
        util_rotate(origin, v[i], angles);
    }
}

#endif //P6ENGINE_POBJECT_H