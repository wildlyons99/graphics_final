#ifndef SOLAR_SYSTEM_H
#define SOLAR_SYSTEM_H

#include <math.h>
#include <glm/glm.hpp>
#include <vector>

#define PI 3.141592653589793238462643383279502


class SolarSystem {
public:
    
    struct Body {
        float radius;
        glm::mat4 local_transform;
        glm::mat4 orbit_rotation;
        glm::vec3 color;
        float translation;
        glm::vec3 axis;
        int numChildren;
        Body **satellites;
    };
    std::vector<int> sattilite_counts = {0, 4, 4};
    int planets = 4;
    Body **system;

    SolarSystem();

    ~SolarSystem();

    void Draw(Body *parent, Body *body);

    void DeleteBody(Body *body);

    Body *GenRandomBody(Body *parent, int depth = 1);

    void render();

    void DrawBodyOrbits(Body *body);
    void DrawCircleWithAxis(float translation, glm::vec3 axis, glm::vec3 color);

    void draw_orbits();

    std::vector<glm::vec3> random_okhcl_color(int count);
    glm::vec3 okhcl_to_rgb(glm::vec3 okhcl);
    
private:
    void draw_circle(float radius, glm::vec3 color);
};

#endif
