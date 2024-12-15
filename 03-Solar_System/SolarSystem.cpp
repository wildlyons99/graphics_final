#define GLM_ENABLE_EXPERIMENTAL
#include <FL/gl.h>
#include <FL/glut.h>
#include "SolarSystem.h"
#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/type_ptr.inl>
#include <glm/gtc/random.hpp>
#include <glm/gtx/quaternion.hpp>
#include <vector>
#include <iostream>

#define SEGMENTS 20

std::vector<glm::vec3> SolarSystem::random_okhcl_color(int count) {
    std::vector<glm::vec3> colors;
    for (int i = 0; i < count; i++) {
        float hue = glm::linearRand(0.0f, 360.0f);
        float chroma = glm::linearRand(0.1f, 0.25f);
        float luminance = glm::linearRand(0.6f, 0.99f);
        colors.emplace_back(hue, chroma, luminance);
    }
    return colors;
}

const double Xn = 0.95047;
const double Yn = 1.00000;
const double Zn = 1.08883;

double gamma_correction(double c) {
    return c >= 0.0031308 ? 1.055 * std::pow(c, 1.0 / 2.4) - 0.055 : 12.92 * c;
}
glm::vec3 SolarSystem::okhcl_to_rgb(glm::vec3 okhcl) {
    float H = okhcl.x;
    float C = okhcl.y;
    double L, a, b;
    L = okhcl.z;
    a = C * cos(H * PI / 180.0);
    b = C * sin(H * PI / 180.0);

    float l = L + a * +0.3963377774 + b * +0.2158037573;
    float m = L + a * -0.1055613458 + b * -0.0638541728;
    float s = L + a * -0.0894841775 + b * -1.2914855480;
    l = l * l * l; m = m * m * m; s = s * s * s;
    float R = l * +4.0767416621 + m * -3.3077115913 + s * +0.2309699292;
    float G = l * -1.2684380046 + m * +2.6097574011 + s * -0.3413193965;
    float B = l * -0.0041960863 + m * -0.7034186147 + s * +1.7076147010;

    R = gamma_correction(R);
    G = gamma_correction(G);
    B = gamma_correction(B);
    //print lab values
    printf("L: %f, a: %f, b: %f\n", L, a, b);
    printf("R: %f, G: %f, B: %f\n", R, G, B);
    return glm::vec3(R, G, B);
}

SolarSystem::SolarSystem() {
    srand(time(0));
    float theta = 45;
    glm::vec3 point(1, 1, 1);
    glm::mat4 rotx = glm::rotate(glm::mat4(1.0f), glm::radians(theta), glm::vec3(1.0f, 0.0f, 0.0f));
    glm::mat4 roty = glm::rotate(glm::mat4(1.0f), glm::radians(theta), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 rotz = glm::rotate(glm::mat4(1.0f), glm::radians(theta), glm::vec3(0.0f, 0.0f, 1.0f));
    glm::vec3 rotatedx = glm::vec3(rotx * glm::vec4(point, 1.0f));
    glm::vec3 rotatedy = glm::vec3(roty * glm::vec4(point, 1.0f));
    glm::vec3 rotatedz = glm::vec3(rotz * glm::vec4(point, 1.0f));
    std::cout << "Rotated x: " << rotatedx.x << ", " << rotatedx.y << ", " << rotatedx.z << std::endl;
    std::cout << "Rotated y: " << rotatedy.x << ", " << rotatedy.y << ", " << rotatedy.z << std::endl;
    std::cout << "Rotated z: " << rotatedz.x << ", " << rotatedz.y << ", " << rotatedz.z << std::endl;
    
    static float rotationSpeed = 0.1;
    static float orbitSpeed = 0.15;
    static float moonOrbitX = 0.1;
    static float moonOrbitY = 0.1;
    static float moonOrbitSpeed = 1;
    system = new Body *[planets];
    std::vector<glm::vec3> colors = random_okhcl_color(planets);
    for (int p = 0; p < planets; p++) {
        system[p] = GenRandomBody(nullptr);
        system[p]->color = okhcl_to_rgb(colors[p]);
    }
}

SolarSystem::~SolarSystem() {
    for (int p = 0; p < planets; p++) {
        DeleteBody(system[p]);
    }
    delete [] system;
}

void SolarSystem::Draw(Body *parent, Body *body) {
    if (body == nullptr) return;
    glPushMatrix();
    body->local_transform = body->orbit_rotation * body->local_transform;
    glMultMatrixf(glm::value_ptr(body->local_transform));
    glColor3fv(glm::value_ptr(body->color));
    glutSolidSphere(0.2f, SEGMENTS, SEGMENTS);
    for (int i = 0; i < body->numChildren; i++) {
        Draw(body, body->satellites[i]);
    }
    glPopMatrix();
}

void SolarSystem::DeleteBody(Body *body) {
    if (body == nullptr) return;
    for (int i = 0; i < body->numChildren; i++) {
        DeleteBody(body->satellites[i]);
    }
    delete [] body->satellites;
    delete body;
}

SolarSystem::Body *SolarSystem::GenRandomBody(Body *parent, int depth) {
    if (depth > 3) { return nullptr; }
    Body *body = new Body;
    body->radius = abs(glm::gaussRand(0.34f, 0.1f)) + 0.01;

    glm::vec3 axis = glm::sphericalRand(1.0f);
    glm::vec3 standard_up = (parent) ? parent->axis : glm::vec3(0, 1, 0);
    while (glm::dot(axis, standard_up) < 0.95f and glm::dot(axis, standard_up) > -0.95f) {
        axis = glm::sphericalRand(1.0f);
    }
    body->axis = axis;

    body->orbit_rotation = glm::rotate(glm::mat4(1.0), 0.01f * depth * depth, axis);

    glm::vec3 orthogonal = glm::normalize(glm::vec3(axis.z, 0, -axis.x));

    glm::vec3 translation;
    // if (parent) {
    //     translation = glm::linearRand(glm::sqrt(parent->radius), 0.75f) * orthogonal;
    // } else {
    //     translation = glm::linearRand(0.25f, 0.9f) * orthogonal;
    // }
    float offset = glm::linearRand(0.25f, 0.85f);
    translation = offset * orthogonal;
    glm::mat4 composite = glm::translate(glm::mat4(1.0f), translation);
    composite = glm::scale(composite, glm::vec3(body->radius));
    body->local_transform = composite;
    body->translation = offset;
    body->numChildren = sattilite_counts[depth];
    if (body->numChildren == 0) {
        body->satellites = nullptr;
        return body;
    }
    body->satellites = new Body *[body->numChildren];
    std::vector<glm::vec3> colors = random_okhcl_color(body->numChildren);
    for (int i = 0; i < body->numChildren; i++) {
        body->satellites[i] = GenRandomBody(body, depth + 1);
        body->satellites[i]->color = okhcl_to_rgb(colors[i]);
    }
    return body;
}


// Render this with push and pop operations
// or alternatively implement helper functions
// for each of the planets with parameters.
void SolarSystem::render() {
    // Some ideas for constants that can be used for
    // rotating the planets.
    static float rotation = 0.1;
    static float orbitSpeed = 0.15;
    static float moonOrbitX = 0.1;
    static float moonOrbitY = 0.1;
    static float moonOrbitSpeed = 1;

    glPushMatrix();

    // The Sun
    glPushMatrix();
    glRotatef(rotation, 0, 1, 0);
    glColor3f(0.96f, 0.85f, 0.26f);
    glutSolidSphere(0.12, SEGMENTS, SEGMENTS);
    glPopMatrix();


    // Add more planets, moons, and rings here!
    for (int p = 0; p < planets; p++) {
        Body *planet = system[p];
        Draw(nullptr, planet);
    }

    glPopMatrix();

    rotation += 0.01;
    orbitSpeed += 0.2;
    moonOrbitX += moonOrbitSpeed;
    moonOrbitY += moonOrbitSpeed;
}

void SolarSystem::DrawBodyOrbits(Body *body) {
    if (!body) return;
    glPushMatrix();
    DrawCircleWithAxis(body->translation, body->axis, body->color);
    glMultMatrixf(glm::value_ptr(body->local_transform));
    for (int i = 0; i < body->numChildren; i++) {
        DrawBodyOrbits(body->satellites[i]);
    }
    glPopMatrix();
}

void SolarSystem::DrawCircleWithAxis(float translation, glm::vec3 axis, glm::vec3 color) {
    glm::vec3 orthogonal(axis.y, -axis.x, 0);
    glm::quat rotation = glm::rotation(glm::vec3(0,1,0), axis);
    glm::mat4 RotationMatrix = glm::toMat4(rotation);
    glPushMatrix();
    glMultMatrixf(glm::value_ptr(RotationMatrix)); //expects this matrix to be popped
    draw_circle(translation, color);
    glPopMatrix();
}

void SolarSystem::draw_circle(float radius, glm::vec3 color) {
    float x, y;

    glDisable(GL_LIGHTING);
    glBegin(GL_LINES);
    glColor4f(color.x, color.y, color.z, 0.8f);

    x = (float) radius * cos(359 * PI / 180.0f);
    y = (float) radius * sin(359 * PI / 180.0f);
    for (int j = 0; j < 360; j++) {
        glVertex3f(x, 0, y);
        x = (float) radius * cos(j * PI / 180.0f);
        y = (float) radius * sin(j * PI / 180.0f);
        glVertex3f(x, 0, y);
    }
    glEnd();
    glEnable(GL_LIGHTING);
}

void SolarSystem::draw_orbits() {
    for (int p = 0; p < planets; p++) {
        Body *planet = system[p];
        DrawBodyOrbits(planet);
    }
}

#define GLM_ENABLE_EXPERIMENTAL
#include <FL/gl.h>
#include <FL/glut.h>
#include "SolarSystem.h"
#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/type_ptr.inl>
#include <glm/gtc/random.hpp>
#include <glm/gtx/quaternion.hpp>
#include <vector>
#include <iostream>




#define SEGMENTS 20

std::vector<glm::vec3> SolarSystem::random_okhcl_color(int count) {
    std::vector<glm::vec3> colors;
    for (int i = 0; i < count; i++) {
        float hue = glm::linearRand(0.0f, 360.0f);
        float chroma = glm::linearRand(0.1f, 0.25f);
        float luminance = glm::linearRand(0.6f, 0.99f);
        colors.emplace_back(hue, chroma, luminance);
    }
    return colors;
}

const double Xn = 0.95047;
const double Yn = 1.00000;
const double Zn = 1.08883;

double gamma_correction(double c) {
    return c >= 0.0031308 ? 1.055 * std::pow(c, 1.0 / 2.4) - 0.055 : 12.92 * c;
}
glm::vec3 SolarSystem::okhcl_to_rgb(glm::vec3 okhcl) {
    float H = okhcl.x;
    float C = okhcl.y;
    double L, a, b;
    L = okhcl.z;
    a = C * cos(H * PI / 180.0);
    b = C * sin(H * PI / 180.0);

    float l = L + a * +0.3963377774 + b * +0.2158037573;
    float m = L + a * -0.1055613458 + b * -0.0638541728;
    float s = L + a * -0.0894841775 + b * -1.2914855480;
    l = l * l * l; m = m * m * m; s = s * s * s;
    float R = l * +4.0767416621 + m * -3.3077115913 + s * +0.2309699292;
    float G = l * -1.2684380046 + m * +2.6097574011 + s * -0.3413193965;
    float B = l * -0.0041960863 + m * -0.7034186147 + s * +1.7076147010;

    R = gamma_correction(R);
    G = gamma_correction(G);
    B = gamma_correction(B);
    //print lab values
    printf("L: %f, a: %f, b: %f\n", L, a, b);
    printf("R: %f, G: %f, B: %f\n", R, G, B);
    return glm::vec3(R, G, B);
}

SolarSystem::SolarSystem() {
    srand(time(0));
    float theta = 45;
    glm::vec3 point(1, 1, 1);
    glm::mat4 rotx = glm::rotate(glm::mat4(1.0f), glm::radians(theta), glm::vec3(1.0f, 0.0f, 0.0f));
    glm::mat4 roty = glm::rotate(glm::mat4(1.0f), glm::radians(theta), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 rotz = glm::rotate(glm::mat4(1.0f), glm::radians(theta), glm::vec3(0.0f, 0.0f, 1.0f));
    glm::vec3 rotatedx = glm::vec3(rotx * glm::vec4(point, 1.0f));
    glm::vec3 rotatedy = glm::vec3(roty * glm::vec4(point, 1.0f));
    glm::vec3 rotatedz = glm::vec3(rotz * glm::vec4(point, 1.0f));
    std::cout << "Rotated x: " << rotatedx.x << ", " << rotatedx.y << ", " << rotatedx.z << std::endl;
    std::cout << "Rotated y: " << rotatedy.x << ", " << rotatedy.y << ", " << rotatedy.z << std::endl;
    std::cout << "Rotated z: " << rotatedz.x << ", " << rotatedz.y << ", " << rotatedz.z << std::endl;
    
    static float rotationSpeed = 0.1;
    static float orbitSpeed = 0.15;
    static float moonOrbitX = 0.1;
    static float moonOrbitY = 0.1;
    static float moonOrbitSpeed = 1;
    system = new Body *[planets];
    std::vector<glm::vec3> colors = random_okhcl_color(planets);
    for (int p = 0; p < planets; p++) {
        system[p] = GenRandomBody(nullptr);
        system[p]->color = okhcl_to_rgb(colors[p]);
    }
}

SolarSystem::~SolarSystem() {
    for (int p = 0; p < planets; p++) {
        DeleteBody(system[p]);
    }
    delete [] system;
}

void SolarSystem::Draw(Body *parent, Body *body) {
    if (body == nullptr) return;
    glPushMatrix();
    body->local_transform = body->orbit_rotation * body->local_transform;
    glMultMatrixf(glm::value_ptr(body->local_transform));
    glColor3fv(glm::value_ptr(body->color));
    glutSolidSphere(0.2f, SEGMENTS, SEGMENTS);
    for (int i = 0; i < body->numChildren; i++) {
        Draw(body, body->satellites[i]);
    }
    glPopMatrix();
}

void SolarSystem::DeleteBody(Body *body) {
    if (body == nullptr) return;
    for (int i = 0; i < body->numChildren; i++) {
        DeleteBody(body->satellites[i]);
    }
    delete [] body->satellites;
    delete body;
}

SolarSystem::Body *SolarSystem::GenRandomBody(Body *parent, int depth) {
    if (depth > 3) { return nullptr; }
    Body *body = new Body;
    body->radius = abs(glm::gaussRand(0.34f, 0.1f)) + 0.01;

    glm::vec3 axis = glm::sphericalRand(1.0f);
    glm::vec3 standard_up = (parent) ? parent->axis : glm::vec3(0, 1, 0);
    while (glm::dot(axis, standard_up) < 0.95f and glm::dot(axis, standard_up) > -0.95f) {
        axis = glm::sphericalRand(1.0f);
    }
    body->axis = axis;

    body->orbit_rotation = glm::rotate(glm::mat4(1.0), 0.01f * depth * depth, axis);

    glm::vec3 orthogonal = glm::normalize(glm::vec3(axis.z, 0, -axis.x));

    glm::vec3 translation;
    // if (parent) {
    //     translation = glm::linearRand(glm::sqrt(parent->radius), 0.75f) * orthogonal;
    // } else {
    //     translation = glm::linearRand(0.25f, 0.9f) * orthogonal;
    // }
    float offset = glm::linearRand(0.25f, 0.85f);
    translation = offset * orthogonal;
    glm::mat4 composite = glm::translate(glm::mat4(1.0f), translation);
    composite = glm::scale(composite, glm::vec3(body->radius));
    body->local_transform = composite;
    body->translation = offset;
    body->numChildren = sattilite_counts[depth];
    if (body->numChildren == 0) {
        body->satellites = nullptr;
        return body;
    }
    body->satellites = new Body *[body->numChildren];
    std::vector<glm::vec3> colors = random_okhcl_color(body->numChildren);
    for (int i = 0; i < body->numChildren; i++) {
        body->satellites[i] = GenRandomBody(body, depth + 1);
        body->satellites[i]->color = okhcl_to_rgb(colors[i]);
    }
    return body;
}


// Render this with push and pop operations
// or alternatively implement helper functions
// for each of the planets with parameters.
void SolarSystem::render() {
    // Some ideas for constants that can be used for
    // rotating the planets.
    static float rotation = 0.1;
    static float orbitSpeed = 0.15;
    static float moonOrbitX = 0.1;
    static float moonOrbitY = 0.1;
    static float moonOrbitSpeed = 1;

    glPushMatrix();

    // The Sun
    glPushMatrix();
    glRotatef(rotation, 0, 1, 0);
    glColor3f(0.96f, 0.85f, 0.26f);
    glutSolidSphere(0.12, SEGMENTS, SEGMENTS);
    glPopMatrix();


    // Add more planets, moons, and rings here!
    for (int p = 0; p < planets; p++) {
        Body *planet = system[p];
        Draw(nullptr, planet);
    }

    glPopMatrix();

    rotation += 0.01;
    orbitSpeed += 0.2;
    moonOrbitX += moonOrbitSpeed;
    moonOrbitY += moonOrbitSpeed;
}

void SolarSystem::DrawBodyOrbits(Body *body) {
    if (!body) return;
    glPushMatrix();
    DrawCircleWithAxis(body->translation, body->axis, body->color);
    glMultMatrixf(glm::value_ptr(body->local_transform));
    for (int i = 0; i < body->numChildren; i++) {
        DrawBodyOrbits(body->satellites[i]);
    }
    glPopMatrix();
}
void SolarSystem::DrawCircleWithAxis(float translation, glm::vec3 axis, glm::vec3 color) {
    glm::vec3 orthogonal(axis.y, -axis.x, 0);
    glm::quat rotation = glm::rotation(glm::vec3(0,1,0), axis);
    glm::mat4 RotationMatrix = glm::toMat4(rotation);
    glPushMatrix();
    glMultMatrixf(glm::value_ptr(RotationMatrix)); //expects this matrix to be popped
    draw_circle(translation, color);
    glPopMatrix();
}
void SolarSystem::draw_circle(float radius, glm::vec3 color) {
    float x, y;

    glDisable(GL_LIGHTING);
    glBegin(GL_LINES);
    glColor4f(color.x, color.y, color.z, 0.8f);

    x = (float) radius * cos(359 * PI / 180.0f);
    y = (float) radius * sin(359 * PI / 180.0f);
    for (int j = 0; j < 360; j++) {
        glVertex3f(x, 0, y);
        x = (float) radius * cos(j * PI / 180.0f);
        y = (float) radius * sin(j * PI / 180.0f);
        glVertex3f(x, 0, y);
    }
    glEnd();
    glEnable(GL_LIGHTING);
}

void SolarSystem::draw_orbits() {
    for (int p = 0; p < planets; p++) {
        Body *planet = system[p];
        DrawBodyOrbits(planet);
    }
}
