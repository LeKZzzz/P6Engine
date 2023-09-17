//
// Created by LeK on 2023/5/11.
//

#ifndef P6ENGINE_PWORLD_H
#define P6ENGINE_PWORLD_H

#include "Basic.h"
#include "EventManager.h"
#include "PObjectManager.h"
#include "Scene.h"
#include "PUI.h"
#include "CameraManager.h"
#include <Windows.h>

using namespace std;

class PWorld: public Basic{
public:

    PWorld(int, int, int, int buffer_nums = 2);   //默认双缓冲

    void on(PObjectManager *);

    void on(CameraManager *);

    void on(PUI *);

    void clear() {};

    void set_fps(int);

    void run();

    void set_title(LPCSTR lpTitle) { //设置控制台标题
        SetConsoleTitle(lpTitle);
    }


protected:
    int fps, buffer_num, buffer_index = 0, time_interval;    //缓冲数
    HANDLE *out_buffer;
    int screen_length, screen_width;   //控制台长宽
    list<PObjectManager *> objmanager_list;
    list<CameraManager *> cameramanager_list;
    list<PUI *> ui_list;
};

//TODO：也许可以通过win api创建多个窗口
PWorld::PWorld(int length, int width, int myfps, int buffer_nums) {
    HWND hWnd = GetConsoleWindow();  // 获取控制台窗口的句柄
    HMENU hMenu = GetSystemMenu(hWnd, FALSE);  // 获取控制台窗口的菜单句柄
    HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cci;
    cci.bVisible = 0;
    cci.dwSize = 1;
    fps = myfps;
    buffer_num = buffer_nums;
    screen_length = length;
    screen_width = width;
    time_interval = 1000 / fps;
    out_buffer = new HANDLE[buffer_nums];
    for (int j = 0; j < buffer_num; ++j) {
        out_buffer[j] = CreateConsoleScreenBuffer(   //创建缓冲区
                GENERIC_WRITE,  //权限
                FILE_SHARE_WRITE,   //共享模式
                NULL,   //安全级别
                CONSOLE_TEXTMODE_BUFFER,    //标识符
                NULL     //保留,为NULL
        );
        if (out_buffer[j] == INVALID_HANDLE_VALUE) {//创建失败
            cout << "Create error" << endl;
        }
        SetConsoleCursorInfo(out_buffer[j], &cci);   //隐藏光标
        SetConsoleScreenBufferSize(out_buffer[j], {short(length), short(width)});
        //一直无法输出，搞半天是没设置缓冲区大小，无奈了homie，还得是看官方文档TAT
    }
    SetConsoleCursorInfo(hStdout, &cci);   //隐藏光标
    SetConsoleScreenBufferSize(hStdout, {short(length), short(width)});

    //设置窗口大小
    //TODO:修改为窗口模式
//    SMALL_RECT rect={0, 0, short(length), short(width)};
    COORD c = {short(length), short(width)};
//    SetConsoleWindowInfo(hStdout,TRUE,&rect);
//    SetConsoleWindowInfo(out_buffer[0],TRUE,&rect);
//    SetConsoleWindowInfo(out_buffer[1],TRUE,&rect);
    SetConsoleDisplayMode(hStdout, CONSOLE_WINDOWED_MODE, &c);
//    SetConsoleDisplayMode(out_buffer[0],CONSOLE_FULLSCREEN_MODE, &c);
    //SetConsoleDisplayMode(out_buffer[1],CONSOLE_FULLSCREEN_MODE, &c);

    //关闭快速编辑模式
    DWORD mode;
    GetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), &mode);
    mode &= ~ENABLE_QUICK_EDIT_MODE;
    SetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), mode);

    //设置字体
    CONSOLE_FONT_INFOEX cfi;
    cfi.cbSize = sizeof cfi;
    cfi.nFont = 0;
    cfi.dwFontSize.X = 0;
    cfi.dwFontSize.Y = 10;
    cfi.FontFamily = FF_DONTCARE;
    cfi.FontWeight = FW_LIGHT;
    wcscpy_s(cfi.FaceName, L"Raster");
    for (int i = 0; i < buffer_num; ++i)
        SetCurrentConsoleFontEx(out_buffer[i], FALSE, &cfi);
    SetCurrentConsoleFontEx(hStdout, FALSE, &cfi);

    //禁止修改窗口大小
    EnableMenuItem(hMenu, SC_SIZE, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);  // 禁用窗口调整大小功能
    LONG_PTR style = GetWindowLongPtr(hWnd, GWL_STYLE);  // 获取窗口的样式
    SetWindowLongPtr(hWnd, GWL_STYLE, style & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX);  // 修改窗口样式，不允许调整大小
}

void PWorld::set_fps(int newfps) {
    fps = newfps;
    time_interval = 1000 / fps;
}

void PWorld::run() {
    while (true) {
        DWORD lastTime = GetTickCount();
        for (auto it = objmanager_list.begin(); it != objmanager_list.end(); it++) {
            (*it)->update();
        }
        if (logic_flag) logic();
        for (auto it = cameramanager_list.begin(); it != cameramanager_list.end(); it++) {
            (*it)->show(&objmanager_list, &ui_list, out_buffer[buffer_index]);
        }
        //SetConsoleActiveScreenBuffer(out_buffer[buffer_index]);
        buffer_index = (buffer_index + 1) % buffer_num;
        DWORD nowTime = GetTickCount();		//获得当前帧的时间点
        DWORD deltaTime = nowTime - lastTime;  //计算这一帧与上一帧的时间差
        if (deltaTime <= time_interval)
            Sleep(time_interval - deltaTime);   //等待帧同步
    }
}

void PWorld::on(PObjectManager *objmanager) {
    objmanager_list.push_back(objmanager);
}

void PWorld::on(CameraManager *cameramanager) {
    cameramanager_list.push_back(cameramanager);
}

void PWorld::on(PUI *ui) {
    ui_list.push_back(ui);
}


#endif //P6ENGINE_PWORLD_H
