#pragma once

#ifndef MYGLCANVAS_H
#define MYGLCANVAS_H

#if defined(__APPLE__)
#  include <OpenGL/gl3.h> // defines OpenGL 3.0+ functions
#else
#  if defined(WIN32)
#    define GLEW_STATIC 1
#  endif
#  include <GL/glew.h>
#endif
#include <FL/glut.H>
#include <FL/glu.h>
#include <glm/glm.hpp>
#include <time.h>
#include <iostream>

#include "TextureManager.h"
#include "ShaderManager.h"
#include "ply.h"
#include "gfxDefs.h"


class MyGLCanvas : public Fl_Gl_Window {
public:
	glm::vec3 eyePosition;
	glm::vec3 rotVec;
	glm::vec3 lookatPoint;
	glm::vec3 lightPos;
	glm::vec3 rotWorldVec;
    glm::vec3 cameraPosition;

	int useDiffuse;
	float lightAngle; //used to control where the light is coming from
	int viewAngle;
	float clipNear;
	float clipFar;
	float scaleFactor;
	float textureBlend;

	MyGLCanvas(int x, int y, int w, int h, const char* l = 0);
	~MyGLCanvas();

	void loadPLY(std::string filename);
	void loadEnvironmentTexture(std::string filename);
	void loadObjectTexture(std::string filename);
	void reloadShaders();

private:
	void draw();
	void drawScene();

	void initShaders();

    void createPlane(unsigned int programID);
    unsigned int planeVAO;
    unsigned int planevertices;

    int handle(int);
	void resize(int x, int y, int w, int h);
	void updateCamera(int width, int height);

    float clickIntersect(glm::vec3 eyePointP, glm::vec3 rayV, glm::mat4 objToWorld);
    glm::vec3 generateRay(int pixelX, int pixelY);

	TextureManager* myTextureManager;
	ShaderManager* myShaderManager;
	ply* myObjectPLY;
    ply* myEnvironmentPLY;

    std::vector<ply *> planets; 
    std::vector<glm::mat4> planetMatrices; 
    std::vector<float> planetOrbitAngle;
    std::vector<bool> planetOrbitPaused;

    // saving the PLY and PLL path for each planet
    std::vector<string> planetPLYFilenames;
    std::vector<string> planetTextureFilenames;
	

    float initialOrbitAngle; 
    bool orbitPaused;
    float mouseLastXPos;
    int NUM_PLANETS;

	glm::mat4 perspectiveMatrix;
    glm::mat4 viewMatrix;

	bool firstTime;
};

#endif // !MYGLCANVAS_H