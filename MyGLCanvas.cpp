#include "MyGLCanvas.h"
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/noise.hpp>

MyGLCanvas::MyGLCanvas(int x, int y, int w, int h, const char* l) : Fl_Gl_Window(x, y, w, h, l) {
	mode(FL_OPENGL3 | FL_RGB | FL_ALPHA | FL_DEPTH | FL_DOUBLE);

	eyePosition = glm::vec3(0.0f, 0.0f, 3.0f);
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

	useDiffuse = false;

	firstTime = true;

	myTextureManager = new TextureManager();
	myShaderManager = new ShaderManager();
	myObjectPLY = new ply("./data/sphere.ply");
	mySecondObjectPLY = new ply("./data/sphere.ply");
	myEnvironmentPLY = new ply("./data/sphere.ply");
}

MyGLCanvas::~MyGLCanvas() {
	delete myTextureManager;
	delete myShaderManager;
	delete myObjectPLY;
    delete mySecondObjectPLY; 
	delete myEnvironmentPLY;
}

void MyGLCanvas::initShaders() {
	myTextureManager->loadTexture("environMap", "./data/sphere-map-market.ppm");
	myTextureManager->loadTexture("objectTexture", "./data/brick.ppm");

	myShaderManager->addShaderProgram("objectShaders", "shaders/330/object-vert.shader", "shaders/330/object-frag.shader");
	myObjectPLY->buildArrays();
	myObjectPLY->bindVBO(myShaderManager->getShaderProgram("objectShaders")->programID);

    myShaderManager->addShaderProgram("objectShaders", "shaders/330/object-vert.shader", "shaders/330/object-frag.shader");
	mySecondObjectPLY->buildArrays();
	mySecondObjectPLY->bindVBO(myShaderManager->getShaderProgram("objectShaders")->programID);

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

	/*glm::vec3 tmp = viewMatrix * glm::vec4(eyePosition, 1.0f)*/;

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


    // Draw the orbiting sphere
    // Let's assume you have a variable orbitAngle (float) that you increment each frame.
    orbitAngle += 0.01f;  // update function ? 

    glm::mat4 orbitModelMatrix = glm::mat4(1.0f);

    // Rotate around Y-axis so the sphere orbits horizontally
    orbitModelMatrix = glm::rotate(orbitModelMatrix, orbitAngle, glm::vec3(0.0f, 1.0f, 0.0f));

    // Translate away from the center so it orbits at a radius of, say, 3 units
    orbitModelMatrix = glm::translate(orbitModelMatrix, glm::vec3(3.0f, 0.0f, 0.0f));

    // Scale it down for a smaller orbiting sphere
    orbitModelMatrix = glm::scale(orbitModelMatrix, glm::vec3(0.25f));

    // Use the same object shader program or another program if you have one
    glUseProgram(objProgramId);
    glUniformMatrix4fv(glGetUniformLocation(objProgramId, "myViewMatrix"), 1, false, glm::value_ptr(viewMatrix));
    glUniformMatrix4fv(glGetUniformLocation(objProgramId, "myModelMatrix"), 1, false, glm::value_ptr(orbitModelMatrix));
    glUniformMatrix4fv(glGetUniformLocation(objProgramId, "myPerspectiveMatrix"), 1, false, glm::value_ptr(perspectiveMatrix));
    
    // Render another sphere (assuming you have a second sphere mesh)
    mySecondObjectPLY->renderVBO(objProgramId);



	//second draw the enviroment sphere
	unsigned int envProgramId =
		myShaderManager->getShaderProgram("environmentShaders")->programID;
	glUseProgram(envProgramId);

	glm::mat4 environmentModelMatrix = glm::mat4(1.0);
	environmentModelMatrix = glm::rotate(environmentModelMatrix, TO_RADIANS(rotWorldVec.x), glm::vec3(1.0f, 0.0f, 0.0f));
	environmentModelMatrix = glm::rotate(environmentModelMatrix, TO_RADIANS(rotWorldVec.y), glm::vec3(0.0f, 1.0f, 0.0f));
	environmentModelMatrix = glm::rotate(environmentModelMatrix, TO_RADIANS(rotWorldVec.z), glm::vec3(0.0f, 0.0f, 1.0f));

	environmentModelMatrix = glm::scale(environmentModelMatrix, glm::vec3(7.0f));
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