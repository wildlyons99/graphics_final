/*  =================== File Information =================
    File Name: main.cpp
    Description:
    Author: Michael Shah

    Purpose: Driver for 3D program to load .ply models
    Usage:
    ===================================================== */

#include <FL/Fl.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_File_Chooser.H>
#include <FL/Fl_Gl_Window.H>
#include <FL/Fl_Pack.H>
#include <FL/Fl_Value_Slider.H>
#include <FL/Fl_Window.H>
#include <FL/gl.h>
#include <FL/glu.h>
#include <FL/glut.h>
#include <FL/names.h>
#include <fstream>
#include <iostream>
#include <math.h>
#include <string>

#include "MyGLCanvas.h"

using namespace std;

class MyAppWindow : public Fl_Window {
public:
    Fl_Slider *rotXSlider;
    Fl_Slider *rotYSlider;
    Fl_Slider *rotZSlider;

    Fl_Button  *wireButton;
    Fl_Button  *orbitButton;
    Fl_Button  *gridButton;
    MyGLCanvas *canvas;

public:
    // APP WINDOW CONSTRUCTOR
    MyAppWindow(int W, int H, const char *L = 0);

    static void idleCB(void *userdata) {
        MyAppWindow *win = (MyAppWindow *)userdata;
        win->canvas->redraw();
    }

private:
    // Someone changed one of the sliders
    static void rotateCB(Fl_Widget *w, void *userdata) {
        float value          = ((Fl_Slider *)w)->value();
        *((float *)userdata) = value;
    }

    static void colorCB(Fl_Widget *w, void *userdata) {
        float value          = ((Fl_Slider *)w)->value();
        *((float *)userdata) = value;
    }

    static void toggleCB(Fl_Widget *w, void *userdata) {
        int value = ((Fl_Button *)w)->value();
        printf("value: %d\n", value);
        *((int *)userdata) = value;
    }
};


MyAppWindow::MyAppWindow(int W, int H, const char *L) : Fl_Window(W, H, L) {
    begin();
    // OpenGL window

    canvas = new MyGLCanvas(10, 10, w() - 110, h() - 20);

    Fl_Pack *pack = new Fl_Pack(w() - 100, 30, 100, h(), "Control Panel");
    pack->box(FL_DOWN_FRAME);
    pack->labelfont(1);
    pack->type(Fl_Pack::VERTICAL);
    pack->spacing(0);
    pack->begin();

    wireButton = new Fl_Check_Button(0, 100, pack->w() - 20, 20, "Wireframe");
    wireButton->callback(toggleCB, (void *)(&(canvas->wireframe)));
    wireButton->value(canvas->wireframe);

    orbitButton = new Fl_Check_Button(0, 100, pack->w() - 20, 20, "Orbit");
    orbitButton->callback(toggleCB, (void *)(&(canvas->orbits)));
    orbitButton->value(canvas->orbits);

    gridButton = new Fl_Check_Button(0, 100, pack->w() - 20, 20, "Grid");
    gridButton->callback(toggleCB, (void *)(&(canvas->grid)));
    gridButton->value(canvas->grid);

    // slider for controlling rotation
    Fl_Box *rotXTextbox = new Fl_Box(0, 0, pack->w() - 20, 20, "RotateX");
    rotXSlider          = new Fl_Value_Slider(0, 0, pack->w() - 20, 20, "");
    rotXSlider->align(FL_ALIGN_TOP);
    rotXSlider->type(FL_HOR_SLIDER);
    rotXSlider->bounds(-359, 359);
    rotXSlider->step(1);
    rotXSlider->value(canvas->rotVec.x);
    rotXSlider->callback(rotateCB, (void *)(&(canvas->rotVec.x)));

    Fl_Box *rotYTextbox = new Fl_Box(0, 0, pack->w() - 20, 20, "RotateY");
    rotYSlider          = new Fl_Value_Slider(0, 0, pack->w() - 20, 20, "");
    rotYSlider->align(FL_ALIGN_TOP);
    rotYSlider->type(FL_HOR_SLIDER);
    rotYSlider->bounds(-359, 359);
    rotYSlider->step(1);
    rotYSlider->value(canvas->rotVec.y);
    rotYSlider->callback(rotateCB, (void *)(&(canvas->rotVec.y)));

    Fl_Box *rotZTextbox = new Fl_Box(0, 0, pack->w() - 20, 20, "RotateZ");
    rotZSlider          = new Fl_Value_Slider(0, 0, pack->w() - 20, 20, "");
    rotZSlider->align(FL_ALIGN_TOP);
    rotZSlider->type(FL_HOR_SLIDER);
    rotZSlider->bounds(-359, 359);
    rotZSlider->step(1);
    rotZSlider->value(canvas->rotVec.z);
    rotZSlider->callback(rotateCB, (void *)(&(canvas->rotVec.z)));

    pack->end();

    end();

    resizable(this);
    Fl::add_idle((Fl_Idle_Handler)(void *)idleCB, (void *)this);
}


/**************************************** main() ********************/
int main(int argc, char **argv) {
    MyAppWindow win(600, 500, "Solar System");
    win.show();
    return (Fl::run());
}