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
    // test out noise

	viewAngle = 60;
	clipNear = 0.01f;
	clipFar = 20.0f;
	scaleFactor = 1.0f;
	lightAngle = 0.0f;
	textureBlend = 0.0f;

    orbitAngle = 0.0f;
    NUM_PLANETS = 3; 

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

    // for (int i = 0; i < NUM_PLANETS; i++) {
    //     // Random selection of a PLY and texture
    //     int plyIndex = (int) glm::linearRand(0.0f, (float) availablePLY.size() + 0.9f);
    //     int ppmIndex = (int) glm::linearRand(0.0f, (float) availablePPM.size() + 0.9f);

    //     printf("Random Numbers: %d and %d\n", plyIndex, ppmIndex); 

    //     planetPLYFilenames.push_back(availablePLY[plyIndex]);
    //     planetTextureFilenames.push_back(availablePPM[ppmIndex]);

    //     planets.push_back(new ply(planetPLYFilenames[i]));
    // }

    // // non random planets
    for (int i = 0; i < NUM_PLANETS; i++) {
        switch (i) { 
            case 0: 
                planets.push_back(new ply("./data/milleniumFalcon.ply")); 
                break; 
            case 1: 
                planets.push_back(new ply("./data/cow.ply"));
                break; 
            case 2: 
                planets.push_back(new ply("./data/sphere.ply"));
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
        delete planets[i]; 
    } 
}

void MyGLCanvas::initShaders() {
	myTextureManager->loadTexture("environMap", "./data/sphere-map-market.ppm");
	myTextureManager->loadTexture("objectTexture", "./data/brick.ppm");

	myShaderManager->addShaderProgram("objectShaders", "shaders/330/object-vert.shader", "shaders/330/object-frag.shader");
	myObjectPLY->buildArrays();
	myObjectPLY->bindVBO(myShaderManager->getShaderProgram("objectShaders")->programID);


    // to use? 
    // for (int i = 0; i < NUM_PLANETS; i++) {
    //     std::string textureName = "planetMap" + std::to_string(i);
    //     myTextureManager->loadTexture(textureName, "./data/planetfile" + std::to_string(i) + ".ppm");
    // }

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
        // myTextureManager->loadTexture("planetMap" + std::to_string(i), planetTextureFilenames[i]); 

        myShaderManager->addShaderProgram("planetShaders", "shaders/330/object-vert.shader", "shaders/330/object-frag.shader");
        planets[i]->buildArrays();
        planets[i]->bindVBO(myShaderManager->getShaderProgram("planetShaders")->programID);
    }

	myShaderManager->addShaderProgram("environmentShaders", "shaders/330/environment-vert.shader", "shaders/330/environment-frag.shader");
	myEnvironmentPLY->buildArrays();
	myEnvironmentPLY->bindVBO(myShaderManager->getShaderProgram("environmentShaders")->programID);

    myShaderManager->addShaderProgram("planeShaders", "shaders/330/plane.vert", "shaders/330/plane.frag");
    createPlane(myShaderManager->getShaderProgram("planeShaders")->programID);
}

void MyGLCanvas::createPlane(unsigned int programID) {
    int rows = 500;
    int cols = 500;
    float spacing = 0.1f;

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

void MyGLCanvas::drawScene() {
	// defining the view 
	glm::mat4 viewMatrix = glm::lookAt(eyePosition, lookatPoint, glm::vec3(0.0f, 1.0f, 0.0f));

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
	glm::vec3 cameraPosition = glm::vec3(inverseViewMatrix[3]);
	glUniform3fv(glGetUniformLocation(objProgramId, "lightPos"), 1, glm::value_ptr(lightPos));
	glUniform3fv(glGetUniformLocation(objProgramId, "cameraPos"), 1, glm::value_ptr(cameraPosition));
	glUniformMatrix4fv(glGetUniformLocation(objProgramId, "myViewMatrix"), 1, false, glm::value_ptr(viewMatrix));
	glUniformMatrix4fv(glGetUniformLocation(objProgramId, "myModelMatrix"), 1, false, glm::value_ptr(modelMatrix));
	glUniformMatrix4fv(glGetUniformLocation(objProgramId, "myPerspectiveMatrix"), 1, false, glm::value_ptr(perspectiveMatrix));
	myObjectPLY->renderVBO(objProgramId);


    // Draw the planets
    
    // Increment orbitAngle somewhere outside this function or here
    orbitAngle += 0.001f; 


    for (int i = 0; i < NUM_PLANETS; i++) {
        unsigned int planetProgramId = myShaderManager->getShaderProgram("planetShaders")->programID;
        glUseProgram(planetProgramId);

        // load the planetMap shader defined above into the 2nd texture index
        glActiveTexture(GL_TEXTURE0 + 2 + i);
        glBindTexture(GL_TEXTURE_2D, myTextureManager->getTextureID("planetMap" + std::to_string(i)));

        // Set uniforms common to all planets
        glUniform1i(glGetUniformLocation(planetProgramId, "environMap"), 2 + i);
        glUniform1i(glGetUniformLocation(planetProgramId, "objectTexture"), 1);
        glUniform3fv(glGetUniformLocation(planetProgramId, "cameraPos"), 1, glm::value_ptr(eyePosition));
        glUniformMatrix4fv(glGetUniformLocation(planetProgramId, "myViewMatrix"), 1, false, glm::value_ptr(viewMatrix));
        glUniformMatrix4fv(glGetUniformLocation(planetProgramId, "myPerspectiveMatrix"), 1, false, glm::value_ptr(perspectiveMatrix));

        glm::mat4 planetModelMatrix = glm::mat4(1.0f);

        // Give each planet a different orbit radius and angle offset
        // float radius = 2.0f + i * 1.0f;        // Each planet farther out by 1 unit
        // float angle = orbitAngle;      // Each planet shifted by i

        // planetModelMatrix = glm::rotate(planetModelMatrix, angle, glm::vec3(0.0f, 1.0f, 0.0f));
        // planetModelMatrix = glm::translate(planetModelMatrix, glm::vec3(radius, 0.0f, 0.0f))
        
        // Eliptical orbits? 
        // Calculate elliptical orbit parameters
        float angle = orbitAngle + i * 1.0f; // Offset angle for each planet
        float radiusX = 2.0f + i * 0.5f;     // X-axis semi-major radius
        float radiusZ = 1.5f + i * 0.5f;     // Z-axis semi-minor radius

        // Calculate x, y, z positions
        float x = radiusX * cos(angle); // Elliptical x-position
        float z = radiusZ * sin(angle); // Elliptical z-position

        // Add y-axis oscillation up to 50% of the orbit height
        float maxY = 0.5f * radiusX; // Max height of oscillation
        float y = maxY * sin(angle); // Oscillation along y-axis

        // Translate the planet to its elliptical orbit position
        planetModelMatrix = glm::translate(planetModelMatrix, glm::vec3(x, y, z));


        // Scale the planets down by a factor of 4
        planetModelMatrix = glm::scale(planetModelMatrix, glm::vec3(0.75f * (i+1)));

        // Set the model matrix for each planet
        glUniformMatrix4fv(glGetUniformLocation(planetProgramId, "myModelMatrix"), 1, false, glm::value_ptr(planetModelMatrix));
        planets[i]->renderVBO(planetProgramId);
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
    glBindVertexArray(planeVAO);
    glDrawElements(GL_TRIANGLES, planevertices, GL_UNSIGNED_INT, 0);
}


void MyGLCanvas::updateCamera(int width, int height) {
	float xy_aspect;
	xy_aspect = (float)width / (float)height;

	perspectiveMatrix = glm::perspective(TO_RADIANS(viewAngle), xy_aspect, clipNear, clipFar);
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
	//printf("Event was %s (%d)\n", fl_eventnames[e], e);
	switch (e) {
	case FL_DRAG:
	case FL_MOVE:
	case FL_PUSH:
	case FL_RELEASE:
	case FL_KEYUP:
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