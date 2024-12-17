#include "MyGLCanvas.h"
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/noise.hpp>

#include <glm/gtc/random.hpp>

#include <vector>

MyGLCanvas::MyGLCanvas(int x, int y, int w, int h, const char* l) : Fl_Gl_Window(x, y, w, h, l) {
	mode(FL_OPENGL3 | FL_RGB | FL_ALPHA | FL_DEPTH | FL_DOUBLE);

	eyePosition = glm::vec3(0.0f, 0.0f, 5.0f);
	lookatPoint = glm::vec3(0.0f, 0.0f, 0.0f);
	rotVec = glm::vec3(0.0f, 0.0f, 0.0f);
	rotWorldVec = glm::vec3(0.0f, 0.0f, 0.0f);
	lightPos = eyePosition;

	viewAngle = 60;
	clipNear = 0.01f;
	clipFar = 20.0f;
	scaleFactor = 1.0f;
	lightAngle = 0.0f;
	textureBlend = 0.0f;

    NUM_PLANETS = 3; 

    for (int i = 0; i < NUM_PLANETS; i++) {
        Planet planet = {
            .size = scaleFactor,
            .modelMatrix = glm::mat4(1.0f),
            .position = glm::vec3(1.0f + i, 2.0f + i, -1.0f - i),
            .orbitDirChange = glm::vec3(1.0f + (-1.5f * i)),
            .orbitPaused = false,
            .recentlyDragged = false
        };
        planets.insert(planets.begin() + i, planet);
    }

	useDiffuse = false;

	firstTime = true;

	myTextureManager = new TextureManager();
	myShaderManager = new ShaderManager();
	myObjectPLY = new ply("./data/sphere.ply");
	myEnvironmentPLY = new ply("./data/sphere.ply");


    // Potential sets of ply and ppm files
    std::vector<string> availablePLY = {
        "./data/sphere.ply",
        "./data/cow.ply",
        "./data/teapot.ply",
        "./data/bunny.ply"
    };

    std::vector<string> availablePPM = {
        "./data/sphere-map-castle.ppm",
        "./data/sphere-map-nature.ppm",
        "./data/boccioni.ppm",
        "./data/brick.ppm"
    };

    // // non random planets
    for (int i = 0; i < NUM_PLANETS; i++) {
        switch (i) { 
            case 0: 
                planets[i].plyModel = new ply("./data/milleniumFalcon.ply"); 
                break; 
            case 1: 
                planets[i].plyModel = new ply("./data/cow.ply"); 
                break; 
            case 2: 
                planets[i].plyModel = new ply("./data/sphere.ply"); 
                break;
        }
    }
}

MyGLCanvas::~MyGLCanvas() {
	delete myTextureManager;
	delete myShaderManager;
	delete myObjectPLY;
	delete myEnvironmentPLY;

    for (int i = 0; i < NUM_PLANETS; i++) {
        delete planets[i].plyModel; 
    } 
}

void MyGLCanvas::initShaders() {
	myTextureManager->loadTexture("environMap", "./data/sphere-map-market.ppm");
	myTextureManager->loadTexture("objectTexture", "./data/brick.ppm");

	myShaderManager->addShaderProgram("objectShaders", "shaders/330/object-vert.shader", "shaders/330/object-frag.shader");
	myObjectPLY->buildArrays();
	myObjectPLY->bindVBO(myShaderManager->getShaderProgram("objectShaders")->programID);


    for (int i = 0; i < NUM_PLANETS; i++) {
        std::string ppm_path; 
        switch (i) { 
            case 0: 
                ppm_path = "./data/sphere-map-nature.ppm"; 
                break; 
            case 1: 
                ppm_path = "./data/boccioni.ppm"; 
                break; 
            case 2: 
                ppm_path = "./data/sphere-map-castle.ppm";
                break;
            default: 
                ppm_path = "./data/brick.ppm";
                break; 
        }
        myTextureManager->loadTexture("planetMap" + std::to_string(i), ppm_path); 

        myShaderManager->addShaderProgram("planetShaders", "shaders/330/object-vert.shader", "shaders/330/object-frag.shader");
        planets[i].plyModel->buildArrays();
        planets[i].plyModel->bindVBO(myShaderManager->getShaderProgram("planetShaders")->programID);
    }

	myShaderManager->addShaderProgram("environmentShaders", "shaders/330/environment-vert.shader", "shaders/330/environment-frag.shader");
	myEnvironmentPLY->buildArrays();
	myEnvironmentPLY->bindVBO(myShaderManager->getShaderProgram("environmentShaders")->programID);

    myShaderManager->addShaderProgram("planeShaders", "shaders/330/plane.vert", "shaders/330/plane.frag");
    createPlane(myShaderManager->getShaderProgram("planeShaders")->programID);
}

void MyGLCanvas::createPlane(unsigned int programID) {
    int rows = 100;
    int cols = 100;
    float spacing = 0.05f;

    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    std::vector<float> normals;
    // Generate vertices and normals
    for (int i = 0; i <= rows; ++i) {
        for (int j = 0; j <= cols; ++j) {
            float x = (j - cols / 2) * spacing; // X position
            float y = 0.0f;        // Y position (flat grid)
            float z = (i - rows / 2)  * spacing; // Z position

            // Add vertex position
            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(z);

            // Add corresponding normal (pointing up in the Y direction)
            normals.push_back(0.0f); // Normal X
            normals.push_back(1.0f); // Normal Y
            normals.push_back(0.0f); // Normal Z
        }
    }

    // Generate indices for triangles
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            // Calculate indices of the corners of the grid cell
            unsigned int topLeft = i * (cols + 1) + j;
            unsigned int topRight = topLeft + 1;
            unsigned int bottomLeft = (i + 1) * (cols + 1) + j;
            unsigned int bottomRight = bottomLeft + 1;

            // First triangle
            indices.push_back(topLeft);
            indices.push_back(bottomLeft);
            indices.push_back(topRight);

            // Second triangle
            indices.push_back(topRight);
            indices.push_back(bottomLeft);
            indices.push_back(bottomRight);
        }
    }
    unsigned int VBO, EBO, NBO;
    glGenVertexArrays(1, &planeVAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    glGenBuffers(1, &NBO);

    // BindplaneVAO 
    glBindVertexArray(planeVAO);

    // Vertex positions
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Vertex normals
    glBindBuffer(GL_ARRAY_BUFFER, NBO);
    glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(float), normals.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_TRUE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);

    // Element indices
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    // UnbindplaneVAO 
    glBindVertexArray(0);
    planevertices = indices.size();

}

void MyGLCanvas::draw() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (!valid()) {  //this is called when the GL canvas is set up for the first time or when it is resized...
		printf("establishing GL context\n");

		glViewport(0, 0, w(), h());
		updateCamera(w(), h());
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

		/****************************************/
		/*          Enable z-buferring          */
		/****************************************/

		glEnable(GL_DEPTH_TEST);
		glPolygonOffset(1, 1); 
		if (firstTime == true) {
			firstTime = false;
			initShaders();
		}
	}

	// Clear the buffer of colors in each bit plane.
	// bit plane - A set of bits that are on or off (Think of a black and white image)
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	drawScene();
}
float myTime = 0;
void MyGLCanvas::drawScene() {
	// defining the view 
	viewMatrix = glm::lookAt(eyePosition, lookatPoint, glm::vec3(0.0f, 1.0f, 0.0f));

	viewMatrix = glm::rotate(viewMatrix, TO_RADIANS(rotWorldVec.x), glm::vec3(1.0f, 0.0f, 0.0f));
	viewMatrix = glm::rotate(viewMatrix, TO_RADIANS(rotWorldVec.y), glm::vec3(0.0f, 1.0f, 0.0f));
	viewMatrix = glm::rotate(viewMatrix, TO_RADIANS(rotWorldVec.z), glm::vec3(0.0f, 0.0f, 1.0f));

	glm::mat4 modelMatrix = glm::mat4(1.0);
	modelMatrix = glm::rotate(modelMatrix, TO_RADIANS(rotVec.x), glm::vec3(1.0f, 0.0f, 0.0f));
	modelMatrix = glm::rotate(modelMatrix, TO_RADIANS(rotVec.y), glm::vec3(0.0f, 1.0f, 0.0f));
	modelMatrix = glm::rotate(modelMatrix, TO_RADIANS(rotVec.z), glm::vec3(0.0f, 0.0f, 1.0f));
	modelMatrix = glm::scale(modelMatrix, glm::vec3(scaleFactor, scaleFactor, scaleFactor));

	glm::vec4 lookVec(0.0f, 0.0f, -1.0f, 0.0f);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glEnable(GL_TEXTURE_2D);
	//Pass first texture info to our shader 
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, myTextureManager->getTextureID("environMap"));
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, myTextureManager->getTextureID("objectTexture"));

	//first draw the object sphere
	unsigned int objProgramId =
		myShaderManager->getShaderProgram("objectShaders")->programID;
	glUseProgram(objProgramId);

	glUniform1i(glGetUniformLocation(objProgramId, "environMap"), 0);
	glUniform1i(glGetUniformLocation(objProgramId, "objectTexture"), 1);
	glUniform1i(glGetUniformLocation(objProgramId, "useDiffuse"), useDiffuse ? 1 : 0);
	glUniform1f(glGetUniformLocation(objProgramId, "textureBlend"), textureBlend);
	glm::mat4 inverseViewMatrix = glm::inverse(viewMatrix);
	cameraPosition = glm::vec3(inverseViewMatrix[3]);
	glUniform3fv(glGetUniformLocation(objProgramId, "lightPos"), 1, glm::value_ptr(lightPos));
	glUniform3fv(glGetUniformLocation(objProgramId, "cameraPos"), 1, glm::value_ptr(cameraPosition));
	glUniformMatrix4fv(glGetUniformLocation(objProgramId, "myViewMatrix"), 1, false, glm::value_ptr(viewMatrix));
	glUniformMatrix4fv(glGetUniformLocation(objProgramId, "myModelMatrix"), 1, false, glm::value_ptr(modelMatrix));
	glUniformMatrix4fv(glGetUniformLocation(objProgramId, "myPerspectiveMatrix"), 1, false, glm::value_ptr(perspectiveMatrix));
	myObjectPLY->renderVBO(objProgramId);


    // Draw the planets
    unsigned int planetProgramId = myShaderManager->getShaderProgram("planetShaders")->programID;
    glUseProgram(planetProgramId);
    
    glUniform3fv(glGetUniformLocation(planetProgramId, "cameraPos"), 1, glm::value_ptr(eyePosition));
     glUniformMatrix4fv(glGetUniformLocation(planetProgramId, "myViewMatrix"), 1, false, glm::value_ptr(viewMatrix));
        glUniformMatrix4fv(glGetUniformLocation(planetProgramId, "myPerspectiveMatrix"), 1, false, glm::value_ptr(perspectiveMatrix));

    float planetSpeed = 0.05f;

    for (int i = 0; i < NUM_PLANETS; i++) {
        // load the planetMap shader defined above into the 2nd texture index
        glActiveTexture(GL_TEXTURE0 + 2 + i);
        glBindTexture(GL_TEXTURE_2D, myTextureManager->getTextureID("planetMap" + std::to_string(i)));

        // Set uniforms common to all planets
        glUniform1i(glGetUniformLocation(planetProgramId, "environMap"), 2 + i);
        glUniform1i(glGetUniformLocation(planetProgramId, "objectTexture"), 1);
       
        glm::mat4 planetModelMatrix = glm::mat4(1.0f);

        // Calculate elliptical orbit parameters
        if (!planets[i].orbitPaused) {
            glm::vec3 v = (planets[i].position.x != 0.0f) ? glm::vec3(0, 1, 0) : glm::vec3(1, 0, 0);

            glm::vec3 t0 = glm::cross(planets[i].position, v);

            if (glm::length(t0) < 0.001f) {
                v = glm::vec3(0, 0, 1);
                t0 = glm::cross(planets[i].position, v);
            }

            t0 = glm::normalize(t0);

            float dotDirChangeT0 = glm::dot(planets[i].orbitDirChange, t0);
            float dotDirChangeDirChange = glm::dot(planets[i].orbitDirChange, planets[i].orbitDirChange);
            float lambda = (1.0f - dotDirChangeT0) / dotDirChangeDirChange;

            if (planets[i].recentlyDragged) {
                planets[i].orbitDirChange = glm::normalize(t0 + lambda * planets[i].orbitDirChange);
                planets[i].recentlyDragged = false;
            }
            float radius = glm::length(planets[i].position);
            float speed = planetSpeed / (1.5f * radius);
            glm::vec3 step = planets[i].orbitDirChange * speed;

            glm::vec3 oldPlanetPos = planets[i].position;
            planets[i].position = radius * glm::normalize(oldPlanetPos + step);
            planets[i].orbitDirChange = glm::normalize(planets[i].position - oldPlanetPos);
        }

        planetModelMatrix = glm::translate(planetModelMatrix, planets[i].position);


        // Scale the planets down by a factor of 4
        float planetScale = 0.75f * (NUM_PLANETS - (i * 0.5f));
        planets[i].size = planetScale;
        planetModelMatrix = glm::scale(planetModelMatrix, glm::vec3(planets[i].size));
        planets[i].modelMatrix = planetModelMatrix;

        // Set the model matrix for each planet
        glUniformMatrix4fv(glGetUniformLocation(planetProgramId, "myModelMatrix"), 1, false, glm::value_ptr(planetModelMatrix));
        planets[i].plyModel->renderVBO(planetProgramId);
    }


	// Draw the enviroment sphere
	unsigned int envProgramId =
		myShaderManager->getShaderProgram("environmentShaders")->programID;
	glUseProgram(envProgramId);

	glm::mat4 environmentModelMatrix = glm::mat4(1.0);
	environmentModelMatrix = glm::rotate(environmentModelMatrix, TO_RADIANS(rotWorldVec.x), glm::vec3(1.0f, 0.0f, 0.0f));
	environmentModelMatrix = glm::rotate(environmentModelMatrix, TO_RADIANS(rotWorldVec.y), glm::vec3(0.0f, 1.0f, 0.0f));
	environmentModelMatrix = glm::rotate(environmentModelMatrix, TO_RADIANS(rotWorldVec.z), glm::vec3(0.0f, 0.0f, 1.0f));

	environmentModelMatrix = glm::scale(environmentModelMatrix, glm::vec3(11.0f));
	glUniform1i(glGetUniformLocation(envProgramId, "environMap"), 0);
	glUniform3fv(glGetUniformLocation(envProgramId, "lightPos"), 1, glm::value_ptr(lightPos));
	glUniformMatrix4fv(glGetUniformLocation(envProgramId, "myViewMatrix"), 1, false, glm::value_ptr(viewMatrix));
	glUniformMatrix4fv(glGetUniformLocation(envProgramId, "myModelMatrix"), 1, false, glm::value_ptr(environmentModelMatrix));
	glUniformMatrix4fv(glGetUniformLocation(envProgramId, "myPerspectiveMatrix"), 1, false, glm::value_ptr(perspectiveMatrix));

	myEnvironmentPLY->renderVBO(envProgramId);

    // draw the plane
    unsigned int planeProgramId = myShaderManager->getShaderProgram("planeShaders")->programID;
    glUseProgram(planeProgramId);
    glUniformMatrix4fv(glGetUniformLocation(planeProgramId, "myViewMatrix"), 1, false, glm::value_ptr(viewMatrix));
    glUniformMatrix4fv(glGetUniformLocation(planeProgramId, "myModelMatrix"), 1, false, glm::value_ptr(modelMatrix));
    glUniformMatrix4fv(glGetUniformLocation(planeProgramId, "myPerspectiveMatrix"), 1, false, glm::value_ptr(perspectiveMatrix));
    // pass in the light position
    glUniform3fv(glGetUniformLocation(planeProgramId, "lightPos"), 1, glm::value_ptr(lightPos));
    // pass in the camera position
    glUniform3fv(glGetUniformLocation(planeProgramId, "cameraPos"), 1, glm::value_ptr(cameraPosition));
    // pass in the texture
    glUniform1i(glGetUniformLocation(planeProgramId, "environMap"), 0);
    // pass in the texture blend
    glUniform1f(glGetUniformLocation(planeProgramId, "textureBlend"), textureBlend);
    // pass in time
    glUniform1f(glGetUniformLocation(planeProgramId, "myTime"), myTime);
    myTime += 0.01f;
    glBindVertexArray(planeVAO);
    
    glDrawElements(GL_TRIANGLES, planevertices, GL_UNSIGNED_INT, 0);
}


void MyGLCanvas::updateCamera(int width, int height) {
	float xy_aspect;
	xy_aspect = (float)width / (float)height;

	perspectiveMatrix = glm::perspective(TO_RADIANS(viewAngle), xy_aspect, clipNear, clipFar);
}

/* The generateRay function accepts the mouse click coordinates
	(in x and y, which will be integers between 0 and screen width and 0 and screen height respectively).
   The function returns the ray
*/
glm::vec3 MyGLCanvas::generateRay(int pixelX, int pixelY) {
    glm::vec3 cameraRayDir = {-1.0f + 2.0f * pixelX / w() , 1.0f - 2.0f * pixelY / h(), -1.0f};
    glm::vec4 viewRay = glm::inverse(perspectiveMatrix) * glm::vec4(cameraRayDir, 1.0f);
    viewRay.z = -1.0f;
    viewRay.w = 0.0f;
	glm::vec3 worldRayDir = glm::inverse(viewMatrix) * viewRay;
	worldRayDir = glm::normalize(worldRayDir);

	return worldRayDir;
}

glm::vec3 MyGLCanvas::getIsectPointWorldCoord(glm::vec3 eye, glm::vec3 ray, float t) {
	return eye + ray * t;
}

float MyGLCanvas::intersect (glm::vec3 eyePointP, glm::vec3 rayV, glm::mat4 objToWorld) {
    glm::mat4 worldToObj = glm::inverse(objToWorld);
	glm::vec3 eyePObj = glm::vec3(worldToObj * glm::vec4(eyePointP, 1.0f));
	glm::vec3 rayObj = glm::vec3(worldToObj * glm::vec4(rayV, 0.0f));

	float A = glm::dot(rayObj, rayObj);
	float B = 2.0 * glm::dot(rayObj, eyePObj);
	float C = glm::dot(eyePObj, eyePObj) - 0.25;

	float discriminant = pow(B, 2) - 4.0 * A * C;

	// no intersection
	if (discriminant < 0) return -1.0;

	float t1 = (-B - sqrt(discriminant)) / (2.0 * A);
	float t2 = (-B + sqrt(discriminant)) / (2.0 * A);

	// return nearest intersection
	if (t1 > 0 && t2 > 0)
	{
		return std::min(t1, t2);
	}
	else if (t1 > 0)
	{
		return t1;
	}
	else if (t2 > 0)
	{
		return t2;
	}

	return -1.0;
}


int MyGLCanvas::handle(int e) {
	//static int first = 1;
#ifndef __APPLE__
	if (firstTime && e == FL_SHOW && shown()) {
		firstTime = 0;
		make_current();
		GLenum err = glewInit(); // defines pters to functions of OpenGL V 1.2 and above
		if (GLEW_OK != err) {
			/* Problem: glewInit failed, something is seriously wrong. */
			fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
		}
		else {
			//SHADER: initialize the shader manager and loads the two shader programs
			initShaders();
		}
	}
#endif	
    int mouseX;
    int mouseY;
    float t;
    int closestObjID;
    glm::vec3 rayV;
	switch (e) {
        case FL_DRAG:
            mouseX = (int)Fl::event_x();
            mouseY = (int)Fl::event_y();
            for (int i = 0; i < NUM_PLANETS; i++) {
                if (planets[i].orbitPaused) { // drag planet
                    glm::vec3 eyePointP = cameraPosition;
                    glm::vec3 newRayV = generateRay(mouseX, mouseY);
                    glm::vec3 mousePosChange = newRayV - oldRayV;
                    glm::vec3 eyeToOldPlanet = glm::normalize(planets[i].position - eyePointP);
                    glm::vec3 eyeToNewPlanet = glm::normalize(eyeToOldPlanet + mousePosChange);
                    glm::vec3 newPlanetPos = getIsectPointWorldCoord(eyePointP, eyeToNewPlanet, oldT);

                    planets[i].position = newPlanetPos;
                    oldRayV = newRayV;
                    glm::vec3 dirChange = eyeToNewPlanet - eyeToOldPlanet;
                    planets[i].orbitDirChange = (glm::length(dirChange) > 0.0000f) ? glm::normalize(dirChange) : planets[i].orbitDirChange;
                    planets[i].recentlyDragged = true;
                }
            }
            break;
        case FL_MOVE:
            break;
        case FL_PUSH:
            mouseX = (int)Fl::event_x();
            mouseY = (int)Fl::event_y();
            rayV = generateRay(mouseX, mouseY);
            t = std::numeric_limits<float>::max();
            closestObjID = -1;
            for (int i = 0; i < NUM_PLANETS; i++) { // upon click, find closest planet that was clicked
                glm::mat4 currPlanetMatrix = planets[i].modelMatrix;
                float currIntersect = intersect(cameraPosition, rayV, currPlanetMatrix);
                if (currIntersect != -1.0) {
                    if (currIntersect < t) {
                        t = currIntersect;
                        closestObjID = i;
                    }
                }
            }
            if (closestObjID != -1) { // If a planet is clicked, pause that planet's orbit
                planets[closestObjID].orbitPaused = true;
                oldRayV = rayV;
                oldT = glm::length(planets[closestObjID].position - cameraPosition);
            }
            
            return (1);
        case FL_RELEASE:
            for (int i = 0; i < NUM_PLANETS; i++) { // resume all orbits when click is released
                planets[i].orbitPaused = false;
            }
            break;
        case FL_KEYUP:
            break;
        case FL_MOUSEWHEEL:
            break;
	}
	return Fl_Gl_Window::handle(e);
}

void MyGLCanvas::resize(int x, int y, int w, int h) {
	Fl_Gl_Window::resize(x, y, w, h);
	puts("resize called");
}

void MyGLCanvas::reloadShaders() {
	myShaderManager->resetShaders();

	myShaderManager->addShaderProgram("objectShaders", "shaders/330/object-vert.shader", "shaders/330/object-frag.shader");
	myObjectPLY->bindVBO(myShaderManager->getShaderProgram("objectShaders")->programID);

	myShaderManager->addShaderProgram("environmentShaders", "shaders/330/environment-vert.shader", "shaders/330/environment-frag.shader");
	myEnvironmentPLY->bindVBO(myShaderManager->getShaderProgram("environmentShaders")->programID);

	invalidate();
}

void MyGLCanvas::loadPLY(std::string filename) {
	delete myObjectPLY;
	myObjectPLY = new ply(filename);
	myObjectPLY->buildArrays();
	myObjectPLY->bindVBO(myShaderManager->getShaderProgram("objectShaders")->programID);
}

void MyGLCanvas::loadEnvironmentTexture(std::string filename) {
	myTextureManager->deleteTexture("environMap");
	myTextureManager->loadTexture("environMap", filename);
}

void MyGLCanvas::loadObjectTexture(std::string filename) {
	myTextureManager->deleteTexture("objectTexture");
	myTextureManager->loadTexture("objectTexture", filename);
}