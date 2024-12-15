#include "MyGLCanvas.h"
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

MyGLCanvas::MyGLCanvas(int x, int y, int w, int h, const char* l) : Fl_Gl_Window(x, y, w, h, l) {
	mode(FL_OPENGL3 | FL_RGB | FL_ALPHA | FL_DEPTH | FL_DOUBLE);

	eyePosition = glm::vec3(0.0f, 0.0f, 3.0f);
	lookatPoint = glm::vec3(0.0f, 0.0f, 0.0f);
	rotVec = glm::vec3(0.0f, 0.0f, 0.0f);
	rotWorldVec = glm::vec3(0.0f, 0.0f, 0.0f);
	lightPos = eyePosition;
    up = glm::vec3(0.0f, 1.0f, 0.0f);

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
}


void MyGLCanvas::updateCamera(int width, int height) {
	float xy_aspect;
	xy_aspect = (float)width / (float)height;

	perspectiveMatrix = glm::perspective(TO_RADIANS(viewAngle), xy_aspect, clipNear, clipFar);
}

glm::mat4 MyGLCanvas::getInverseModelViewMatrix() {
    glm::vec3 u, v, w;
    w = -glm::normalize(lookatPoint);
    u = glm::normalize(glm::cross(up, w));
    v = glm::normalize(glm::cross(w, u));
    glm::mat4 toCameraBasis = glm::mat4(glm::vec4(u, 0),
        glm::vec4(v, 0),
        glm::vec4(w, 0),
        glm::vec4(0, 0, 0, 1));
    glm::mat4 translateToEye = glm::translate(glm::mat4(1.0), eyePosition);
    return translateToEye * toCameraBasis;
}

/* The generateRay function accepts the mouse click coordinates
	(in x and y, which will be integers between 0 and screen width and 0 and screen height respectively).
   The function returns the ray
*/
glm::vec3 MyGLCanvas::generateRay(int pixelX, int pixelY) {
	glm::vec3 eye = eyePosition;
	float viewAngle = viewAngle;
	float screenWidth = w();
	float screenHeight = h();
	float aspectRatio = screenWidth / screenHeight;

	float ndcX = (2.0f * pixelX) / screenWidth - 1.0f;
	float ndcY = 1.0f - (2.0f * pixelY) / screenHeight;

	float tanHalfViewAngle = glm::tan(glm::radians(viewAngle) / 2.0f);
	float px = ndcX * tanHalfViewAngle * aspectRatio;
	float py = ndcY * tanHalfViewAngle;
	glm::vec3 pixelCameraSpace(px, py, -1.0f);

	glm::mat4 inverseViewMatrix = getInverseModelViewMatrix();
	glm::vec4 pixelWorldSpace = inverseViewMatrix * glm::vec4(pixelCameraSpace, 1.0f);
	glm::vec3 pixelWorldPos = glm::vec3(pixelWorldSpace) / pixelWorldSpace.w;

	glm::vec3 rayDirection = glm::vec3(pixelWorldSpace) - eye;

	return rayDirection;
}

float MyGLCanvas::clickIntersect (glm::vec3 eyePointP, glm::vec3 rayV, glm::mat4 transformMatrix) {
	glm::vec3 eyePObj = glm::inverse(transformMatrix) * glm::vec4(eyePointP, 1.0f);
	glm::vec3 rayObj = glm::inverse(transformMatrix) * glm::vec4(rayV, 0.0f);

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
	//printf("Event was %s (%d)\n", fl_eventnames[e], e);
    int mouseX;
    int mouseY;
    float t;
    int closestObjID;
    int objects = 10; // TODO: delete
    glm::mat4 deleteThis = glm::mat4(1.0); // TODO: delete
	switch (e) {
        case FL_DRAG:
            break;
        case FL_MOVE:
            break;
        case FL_PUSH:
            mouseX = (int)Fl::event_x();
            mouseY = (int)Fl::event_y();
            t = std::numeric_limits<float>::max();
            closestObjID = -1;
            for (int i = 0; i < objects; i++) { // TODO get object IDs
                float currIntersect = clickIntersect(eyePosition, generateRay(mouseX, mouseY), deleteThis); // TODO get transformMatrix
                if (currIntersect != -1.0) {
                    printf("hit!\n");
                    if (currIntersect < t) {
                        t = currIntersect;
                        closestObjID = i;
                    }
                }
            }
            if (closestObjID != -1) {
                printf("ID of closest object clicked: %d\n", closestObjID);
                // zoomIn(closestObjID); // TODO zoomIn function
            } else {
                printf("miss!\n");
            }
            
            return (1);
        case FL_RELEASE:
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