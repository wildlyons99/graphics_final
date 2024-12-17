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

private:
	void draw();
	void drawScene();

	void initShaders();

    void createPlane(unsigned int programID);
    void createIcosphereVAO(int recursionLevel);
    void drawWarp(glm::mat4 modelMatrix, glm::mat4 viewMatrix, float myTime); 
    unsigned int planeVAO;
    unsigned int planevertices;
    unsigned int icosphereVAO;
    unsigned int icosphereVertices;

    int handle(int);
	void resize(int x, int y, int w, int h);
	void updateCamera(int width, int height);

    /* The intersect function accepts three input parameters:
        (1) the eye point (in world coordinate)
        (2) the ray vector (in world coordinate)
        (3) the transform matrix that would be applied to there sphere to transform it from object coordinate to world coordinate

        The function should return:
        (1) a -1 if no intersection is found
        (2) OR, the "t" value which is the distance from the origin of the ray to the (nearest) intersection point on the sphere
    */
    float intersect(glm::vec3 eyePointP, glm::vec3 rayV, glm::mat4 objToWorld);

    /* The generateRay function accepts the mouse click coordinates
        (in x and y, which will be integers between 0 and screen width and 0 and screen height respectively).
        The function returns the ray
    */
    glm::vec3 generateRay(int pixelX, int pixelY);

    /* The getIsectPointWorldCoord function accepts three input parameters:
        (1) the eye point (in world coordinate)
        (2) the ray vector (in world coordinate)
        (3) the "t" value

        The function should return the intersection point on the sphere
    */
    glm::vec3 getIsectPointWorldCoord(glm::vec3 eye, glm::vec3 ray, float t);

	TextureManager* myTextureManager;
	ShaderManager* myShaderManager;
	ply* myObjectPLY;
    ply* myEnvironmentPLY;
    ply* myBuddahPLY;
    ply* myWarpPLY;

    struct Planet {
        ply* plyModel;
        std::string texturePath;
        float size;
        float scalar;
        glm::mat4 modelMatrix;
        glm::vec3 position; // world space
        glm::vec3 orbitDirChange;
        bool orbitPaused;
        bool recentlyDragged;
        unsigned int vao;
        unsigned int vertices;
    };

    std::vector<Planet> planets;

    float oldT;
    float planetSpeed;
    int NUM_PLANETS;

    glm::vec3 oldRayV;

	glm::mat4 perspectiveMatrix;
    glm::mat4 viewMatrix;

	bool firstTime;

    // todo Delet
    float orbitAngle; 
};

#endif // !MYGLCANVAS_H