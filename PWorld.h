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

    PWorld(int, int, int, int buffer_nums = 2);   //Ĭ��˫����

    void on(PObjectManager *);

    void on(CameraManager *);

    void on(PUI *);

    void clear() {};

    void set_fps(int);

    void run();

    void set_title(LPCSTR lpTitle) { //���ÿ���̨����
        SetConsoleTitle(lpTitle);
    }


protected:
    int fps, buffer_num, buffer_index = 0, time_interval;    //������
    HANDLE *out_buffer;
    int screen_length, screen_width;   //����̨����
    list<PObjectManager *> objmanager_list;
    list<CameraManager *> cameramanager_list;
    list<PUI *> ui_list;
};

//TODO��Ҳ�����ͨ��win api�����������
PWorld::PWorld(int length, int width, int myfps, int buffer_nums) {
    HWND hWnd = GetConsoleWindow();  // ��ȡ����̨���ڵľ��
    HMENU hMenu = GetSystemMenu(hWnd, FALSE);  // ��ȡ����̨���ڵĲ˵����
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
        out_buffer[j] = CreateConsoleScreenBuffer(   //����������
                GENERIC_WRITE,  //Ȩ��
                FILE_SHARE_WRITE,   //����ģʽ
                NULL,   //��ȫ����
                CONSOLE_TEXTMODE_BUFFER,    //��ʶ��
                NULL     //����,ΪNULL
        );
        if (out_buffer[j] == INVALID_HANDLE_VALUE) {//����ʧ��
            cout << "Create error" << endl;
        }
        SetConsoleCursorInfo(out_buffer[j], &cci);   //���ع��
        SetConsoleScreenBufferSize(out_buffer[j], {short(length), short(width)});
        //һֱ�޷�������������û���û�������С��������homie�������ǿ��ٷ��ĵ�TAT
    }
    SetConsoleCursorInfo(hStdout, &cci);   //���ع��
    SetConsoleScreenBufferSize(hStdout, {short(length), short(width)});

    //���ô��ڴ�С
    //TODO:�޸�Ϊ����ģʽ
//    SMALL_RECT rect={0, 0, short(length), short(width)};
    COORD c = {short(length), short(width)};
//    SetConsoleWindowInfo(hStdout,TRUE,&rect);
//    SetConsoleWindowInfo(out_buffer[0],TRUE,&rect);
//    SetConsoleWindowInfo(out_buffer[1],TRUE,&rect);
    SetConsoleDisplayMode(hStdout, CONSOLE_WINDOWED_MODE, &c);
//    SetConsoleDisplayMode(out_buffer[0],CONSOLE_FULLSCREEN_MODE, &c);
    //SetConsoleDisplayMode(out_buffer[1],CONSOLE_FULLSCREEN_MODE, &c);

    //�رտ��ٱ༭ģʽ
    DWORD mode;
    GetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), &mode);
    mode &= ~ENABLE_QUICK_EDIT_MODE;
    SetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), mode);

    //��������
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

    //��ֹ�޸Ĵ��ڴ�С
    EnableMenuItem(hMenu, SC_SIZE, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);  // ���ô��ڵ�����С����
    LONG_PTR style = GetWindowLongPtr(hWnd, GWL_STYLE);  // ��ȡ���ڵ���ʽ
    SetWindowLongPtr(hWnd, GWL_STYLE, style & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX);  // �޸Ĵ�����ʽ�������������С
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
        DWORD nowTime = GetTickCount();		//��õ�ǰ֡��ʱ���
        DWORD deltaTime = nowTime - lastTime;  //������һ֡����һ֡��ʱ���
        if (deltaTime <= time_interval)
            Sleep(time_interval - deltaTime);   //�ȴ�֡ͬ��
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
