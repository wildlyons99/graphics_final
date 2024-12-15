#pragma once

#ifndef MYGLCANVAS_H
#define MYGLCANVAS_H

#include <FL/gl.h>
#include <FL/glut.h>
#include <FL/glu.h>
#include <glm/glm.hpp>

#include "SolarSystem.h"

class MyGLCanvas : public Fl_Gl_Window {
public:
	int wireframe, orbits, grid;
	glm::vec4 rotVec;
	glm::vec4 eyePosition;
	SolarSystem* solarSystem;

	MyGLCanvas(int x, int y, int w, int h, const char *l = 0);
	~MyGLCanvas();
	void draw_grid();

private:
	void draw();
	int handle(int);
	void resize(int x, int y, int w, int h);
	void updateCamera(int width, int height);
};

#endif // !MYGLCANVAS_H