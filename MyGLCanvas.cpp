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

    planetSpeed = 0.025f;
    NUM_PLANETS = 11; 

    orbitAngle = 0; 

    for (int i = 0; i < NUM_PLANETS; i++) {
        Planet planet = {
            .size = 0.75f * (3 - (i * 0.5f)),
            .modelMatrix = glm::mat4(1.0f),
            .position = glm::vec3(1.0f + i, 2.0f + i, -1.0f - i),
            .orbitDirChange = glm::vec3(1.0f + (-1.5f * i)),
            .orbitPaused = false,
            .recentlyDragged = false
        };
        planets.insert(planets.begin() + i, planet);
        if (i >= 3) {
            planets[i].size = glm::gaussRand(0.25f, 0.1f);
            planets[i].position = glm::vec3(glm::linearRand(-2.5f, 2.5f), glm::linearRand(-2.5f, 2.5f), glm::linearRand(-2.5f, 2.5f));
            planets[i].orbitDirChange = glm::vec3(glm::linearRand(-1.0f, 1.0f), glm::linearRand(-1.0f, 1.0f), glm::linearRand(-1.0f, 1.0f));
        }
        planets[i].scalar = glm::linearRand(0.5f, 1.5f);
    }

	useDiffuse = false;

	firstTime = true;

	myTextureManager = new TextureManager();
	myShaderManager = new ShaderManager();
	myObjectPLY = new ply("./data/sphere.ply");
	myEnvironmentPLY = new ply("./data/sphere.ply");
    myBuddahPLY = new ply("./data/happy.ply");


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

    // make warp planet
    myWarpPLY = new ply("./data/sphere.ply");
}

MyGLCanvas::~MyGLCanvas() {
	delete myTextureManager;
	delete myShaderManager;
	delete myObjectPLY;
	delete myEnvironmentPLY;
    delete myBuddahPLY;

    for (int i = 0; i < NUM_PLANETS; i++) {
        delete planets[i].plyModel; 
    } 

    delete myWarpPLY; 
}

// when adding shaders remember to also add to loadShaders for reload function
void MyGLCanvas::initShaders() {
	myTextureManager->loadTexture("environMap", "./data/sphere-map-market.ppm");
	myTextureManager->loadTexture("objectTexture", "./data/brick.ppm");

    myTextureManager->loadTexture("starsMap", "./data/stars.ppm");

    myTextureManager->loadTexture("noise", "./data/64noise3octaves.ppm");
    myTextureManager->loadTexture("colorMap", "./data/colorMap.ppm");
    myTextureManager->loadTexture("planetNoise", "./data/simpleNoise.ppm");
    myTextureManager->loadTexture("waterNormals", "./data/waterNormals.ppm");

	myShaderManager->addShaderProgram("objectShaders", "shaders/330/object-vert.shader", "shaders/330/object-frag.shader");
	myObjectPLY->buildArrays();
	myObjectPLY->bindVBO(myShaderManager->getShaderProgram("objectShaders")->programID);
    // adding the proceduralPlanet work
    myShaderManager->addShaderProgram("proceduralPlanet", "shaders/330/proceduralPlanet.vert", "shaders/330/proceduralPlanet.frag");
    myShaderManager->addShaderProgram("proceduralPlanet2", "shaders/330/warpPlanetProcedural.vert", "shaders/330/proceduralPlanet.frag");
    createIcosphereVAO(5);
    
    myShaderManager->addShaderProgram("planetShaders", "shaders/330/object-vert.shader", "shaders/330/warpPlanet.frag");
    myShaderManager->addShaderProgram("planetShaders2", "shaders/330/object-vert.shader", "shaders/330/warpPlanetOG.frag");
    for (int i = 0; i < NUM_PLANETS; i++) {
        // for if swicth frag shader ^ to object not used atm
        switch (i) { 
            case 0: 
                planets[i].texturePath = "./data/sphere-map-nature.ppm"; 
                break; 
            case 1: 
                planets[i].texturePath = "./data/boccioni.ppm"; 
                break; 
            case 2: 
                planets[i].texturePath = "./data/sphere-map-castle.ppm";
                break;
            default: 
                planets[i].texturePath = "./data/brick.ppm";
                break; 
        }
        myTextureManager->loadTexture("planetMap" + std::to_string(i), planets[i].texturePath);
        if (!planets[i].plyModel) {
            planets[i].vao = icosphereVAO;
            planets[i].vertices = icosphereVertices;
            continue;
        }
        planets[i].plyModel->buildArrays();
        planets[i].plyModel->bindVBO(myShaderManager->getShaderProgram("planetShaders")->programID);
    }

	myShaderManager->addShaderProgram("environmentShaders", "shaders/330/environment-vert.shader", "shaders/330/environment-frag.shader");
	myEnvironmentPLY->buildArrays();
	myEnvironmentPLY->bindVBO(myShaderManager->getShaderProgram("environmentShaders")->programID);

    // adding the plane shaders
    myShaderManager->addShaderProgram("planeShaders", "shaders/330/plane.vert", "shaders/330/plane.frag", "shaders/330/plane-geo.glsl");
    createPlane(myShaderManager->getShaderProgram("planeShaders")->programID);

    // warp shaders
    myTextureManager->loadTexture("earth", "./data/earth2.ppm");
    myShaderManager->addShaderProgram("warpShaders", "shaders/330/warpPlanet.vert", "shaders/330/warpPlanet.frag");
    myWarpPLY->buildArrays();
	myWarpPLY->bindVBO(myShaderManager->getShaderProgram("warpShaders")->programID);

    myShaderManager->addShaderProgram("warpShaders2", "shaders/330/warpPlanet.vert", "shaders/330/warpPlanet.frag");


    // adding the proceduralPlanet work
    // myShaderManager->addShaderProgram("proceduralPlanet", "shaders/330/proceduralPlanet.vert", "shaders/330/proceduralPlanet.frag");
    // createIcosphereVAO(5);

    myShaderManager->addShaderProgram("buddahShaders", "shaders/330/buddah.vert", "shaders/330/warpPlanetOG.frag");
    // myShaderManager->addShaderProgram("buddahShaders", "shaders/330/buddah.vert", "shaders/330/buddah.frag");
    myBuddahPLY->buildArrays();
    myBuddahPLY->bindVBO(myShaderManager->getShaderProgram("buddahShaders")->programID);
}

struct Vertex {
    glm::vec3 position;  // Vertex position
    glm::vec3 normal;    // Vertex normal
    glm::vec2 uv;        // Texture coordinates
    glm::vec3 tangent;   // Tangent vector
    glm::vec3 bitangent; // Bitangent vector
};


// Generate an icosphere
void generateIcosphere(int recursionLevel, std::vector<Vertex>& vertices, std::vector<unsigned int>& indices) {
    const float GOLDEN_RATIO = (1.0f + std::sqrt(5.0f)) / 2.0f;
    // Create initial icosahedron vertices
    std::vector<glm::vec3> positions = {
        {-1, GOLDEN_RATIO, 0}, {1, GOLDEN_RATIO, 0}, {-1, -GOLDEN_RATIO, 0}, {1, -GOLDEN_RATIO, 0},
        {0, -1, GOLDEN_RATIO}, {0, 1, GOLDEN_RATIO}, {0, -1, -GOLDEN_RATIO}, {0, 1, -GOLDEN_RATIO},
        {GOLDEN_RATIO, 0, -1}, {GOLDEN_RATIO, 0, 1}, {-GOLDEN_RATIO, 0, -1}, {-GOLDEN_RATIO, 0, 1}
    };

    for (auto& pos : positions) {
        pos = normalize(pos);
    }

    // Initial indices for the icosahedron
    std::vector<unsigned int> initialIndices = {
        0, 11, 5, 0, 5, 1, 0, 1, 7, 0, 7, 10, 0, 10, 11,
        1, 5, 9, 5, 11, 4, 11, 10, 2, 10, 7, 6, 7, 1, 8,
        3, 9, 4, 3, 4, 2, 3, 2, 6, 3, 6, 8, 3, 8, 9,
        4, 9, 5, 2, 4, 11, 6, 2, 10, 8, 6, 7, 9, 8, 1
    };

    // Subdivide function
    auto midpoint = [&](unsigned int v1, unsigned int v2) {
        glm::vec3 mid = (positions[v1] + positions[v2]) * 0.5f;
        mid = normalize(mid);
        positions.push_back(mid);
        return static_cast<unsigned int>(positions.size() - 1);
    };

    // Subdivide the icosahedron to create an icosphere
    for (int i = 0; i < recursionLevel; ++i) {
        std::vector<unsigned int> newIndices;
        for (size_t j = 0; j < initialIndices.size(); j += 3) {
            unsigned int a = midpoint(initialIndices[j], initialIndices[j + 1]);
            unsigned int b = midpoint(initialIndices[j + 1], initialIndices[j + 2]);
            unsigned int c = midpoint(initialIndices[j + 2], initialIndices[j]);

            newIndices.push_back(initialIndices[j]);
            newIndices.push_back(a);
            newIndices.push_back(c);

            newIndices.push_back(initialIndices[j + 1]);
            newIndices.push_back(b);
            newIndices.push_back(a);

            newIndices.push_back(initialIndices[j + 2]);
            newIndices.push_back(c);
            newIndices.push_back(b);

            newIndices.push_back(a);
            newIndices.push_back(b);
            newIndices.push_back(c);
        }
        initialIndices = newIndices;
    }

    indices = initialIndices;

    // Generate vertices
    for (const auto& pos : positions) {
        Vertex vertex;
        vertex.position[0] = pos.x;
        vertex.position[1] = pos.y;
        vertex.position[2] = pos.z;
        vertex.normal[0] = pos.x;
        vertex.normal[1] = pos.y;
        vertex.normal[2] = pos.z;
        vertex.uv[0] = 0.5f + std::atan2(pos.z, pos.x) / (2.0f * PI);
        vertex.uv[1] = 0.5f - std::asin(pos.y) / PI;
        vertex.tangent = glm::vec3(0.0f, 0.0f, 0.0f);
        vertex.bitangent = glm::vec3(0.0f, 0.0f, 0.0f);
        vertices.push_back(vertex);
    }

    for (size_t i = 0; i < indices.size(); i += 3) {
        Vertex& v0 = vertices[indices[i]];
        Vertex& v1 = vertices[indices[i + 1]];
        Vertex& v2 = vertices[indices[i + 2]];

        // Compute edge vectors and delta UVs
        glm::vec3 edge1 = v1.position - v0.position;
        glm::vec3 edge2 = v2.position - v0.position;
        glm::vec2 deltaUV1 = v1.uv - v0.uv;
        glm::vec2 deltaUV2 = v2.uv - v0.uv;

        float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

        glm::vec3 tangent, bitangent;

        tangent.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
        tangent.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
        tangent.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);

        bitangent.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
        bitangent.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
        bitangent.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);

        v0.tangent += tangent;
        v1.tangent += tangent;
        v2.tangent += tangent;

        v0.bitangent += bitangent;
        v1.bitangent += bitangent;
        v2.bitangent += bitangent;
    }

    // Normalize the tangents and bitangents
    for (auto& vertex : vertices) {
        vertex.tangent = glm::normalize(vertex.tangent);
        vertex.bitangent = glm::normalize(vertex.bitangent);
    }
    
}

// Create VAO for the icosphere
void MyGLCanvas::createIcosphereVAO(int recursionLevel) {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    generateIcosphere(recursionLevel, vertices, indices);

    GLuint vao, vbo, ebo;

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    glBindVertexArray(vao);

    // Upload vertex data
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

    // Upload index data
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    // Specify vertex attributes
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));
    glEnableVertexAttribArray(2);

    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, tangent));
    glEnableVertexAttribArray(3);

    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, bitangent));
    glEnableVertexAttribArray(4);

    glBindVertexArray(0);

    icosphereVAO = vao;
    icosphereVertices = indices.size();
}

void MyGLCanvas::createPlane(unsigned int programID) {
    int rows = 63;
    int cols = 63;
    float length = 16.0f;
    float spacing = length / cols;

    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    std::vector<float> normals;
    std::vector<float> uvs; // To store UV coordinates

    // Generate vertices, normals, and UV coordinates
    for (int i = 0; i <= rows; ++i) {
        for (int j = 0; j <= cols; ++j) {
            float x = (j - cols / 2) * spacing; // X position
            float y = 0.0f;                     // Y position (flat grid)
            float z = (i - rows / 2) * spacing; // Z position

            // Add vertex position
            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(z);

            // Add corresponding normal (pointing up in the Y direction)
            normals.push_back(0.0f); // Normal X
            normals.push_back(1.0f); // Normal Y
            normals.push_back(0.0f); // Normal Z

            // Add UV coordinates (normalized to [0, 1])
            float u = static_cast<float>(j) / cols;
            float v = static_cast<float>(i) / rows;
            uvs.push_back(u);
            uvs.push_back(v);
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

    unsigned int VBO, EBO, NBO, UVBO;
    glGenVertexArrays(1, &planeVAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    glGenBuffers(1, &NBO);
    glGenBuffers(1, &UVBO);

    // Bind planeVAO
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

    // UV coordinates
    glBindBuffer(GL_ARRAY_BUFFER, UVBO);
    glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(float), uvs.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(2);

    // Element indices
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    // Unbind planeVAO
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
    
    glActiveTexture(GL_TEXTURE8);
    glBindTexture(GL_TEXTURE_2D, myTextureManager->getTextureID("earth"));

    // add stars shader
    glActiveTexture(GL_TEXTURE10);
	glBindTexture(GL_TEXTURE_2D, myTextureManager->getTextureID("starsMap"));
    // add additional shaders
    glActiveTexture(GL_TEXTURE11);
    glBindTexture(GL_TEXTURE_2D, myTextureManager->getTextureID("noise"));
    glActiveTexture(GL_TEXTURE13);
    glBindTexture(GL_TEXTURE_2D, myTextureManager->getTextureID("planetNoise"));
    glActiveTexture(GL_TEXTURE14);
    glBindTexture(GL_TEXTURE_2D, myTextureManager->getTextureID("waterNormals"));
    glActiveTexture(GL_TEXTURE18);
    glBindTexture(GL_TEXTURE_2D, myTextureManager->getTextureID("colorMap"));
	//first draw the object sphere
	// unsigned int objProgramId =
	// 	myShaderManager->getShaderProgram("objectShaders")->programID;
	// glUseProgram(objProgramId);
	//
	// glUniform1i(glGetUniformLocation(objProgramId, "environMap"), 0);
	// glUniform1i(glGetUniformLocation(objProgramId, "objectTexture"), 1);
	// glUniform1i(glGetUniformLocation(objProgramId, "useDiffuse"), useDiffuse ? 1 : 0);
	// glUniform1f(glGetUniformLocation(objProgramId, "textureBlend"), textureBlend);
	glm::mat4 inverseViewMatrix = glm::inverse(viewMatrix);
	cameraPosition = glm::vec3(inverseViewMatrix[3]);
	// glUniform3fv(glGetUniformLocation(objProgramId, "lightPos"), 1, glm::value_ptr(lightPos));
	// glUniform3fv(glGetUniformLocation(objProgramId, "cameraPos"), 1, glm::value_ptr(cameraPosition));
	// glUniformMatrix4fv(glGetUniformLocation(objProgramId, "myViewMatrix"), 1, false, glm::value_ptr(viewMatrix));
	// glUniformMatrix4fv(glGetUniformLocation(objProgramId, "myModelMatrix"), 1, false, glm::value_ptr(modelMatrix));
	// glUniformMatrix4fv(glGetUniformLocation(objProgramId, "myPerspectiveMatrix"), 1, false, glm::value_ptr(perspectiveMatrix));
	// myObjectPLY->renderVBO(objProgramId);


    // Draw the planets
    unsigned int planetProgramId = myShaderManager->getShaderProgram("planetShaders")->programID;
    glUseProgram(planetProgramId);
    
    glUniform3fv(glGetUniformLocation(planetProgramId, "cameraPos"), 1, glm::value_ptr(eyePosition));
    glUniformMatrix4fv(glGetUniformLocation(planetProgramId, "myViewMatrix"), 1, false, glm::value_ptr(viewMatrix));
    glUniformMatrix4fv(glGetUniformLocation(planetProgramId, "myPerspectiveMatrix"), 1, false, glm::value_ptr(perspectiveMatrix));
    glUniform1f(glGetUniformLocation(planetProgramId, "myTime"), myTime);

    for (int i = 0; i < NUM_PLANETS; i++) {
        if (i == 9) {
            planetProgramId = myShaderManager->getShaderProgram("planetShaders2")->programID;
            glUseProgram(planetProgramId);
            glUniform3fv(glGetUniformLocation(planetProgramId, "cameraPos"), 1, glm::value_ptr(eyePosition));
            glUniformMatrix4fv(glGetUniformLocation(planetProgramId, "myViewMatrix"), 1, false, glm::value_ptr(viewMatrix));
            glUniformMatrix4fv(glGetUniformLocation(planetProgramId, "myPerspectiveMatrix"), 1, false, glm::value_ptr(perspectiveMatrix));
            glUniform1f(glGetUniformLocation(planetProgramId, "myTime"), myTime);
        }
        // if (i == 10) {
        //     planetProgramId = myShaderManager->getShaderProgram("proceduralPlanet2")->programID;
        //     glUseProgram(planetProgramId);
        //     glUniform3fv(glGetUniformLocation(planetProgramId, "cameraPos"), 1, glm::value_ptr(eyePosition));
        //     glUniformMatrix4fv(glGetUniformLocation(planetProgramId, "myViewMatrix"), 1, false, glm::value_ptr(viewMatrix));
        //     glUniformMatrix4fv(glGetUniformLocation(planetProgramId, "myPerspectiveMatrix"), 1, false, glm::value_ptr(perspectiveMatrix));
        //     glUniform1f(glGetUniformLocation(planetProgramId, "myTime"), myTime);
        // }
        if (i >= 5) {
            planetProgramId = myShaderManager->getShaderProgram("warpShaders2")->programID;
            glUseProgram(planetProgramId);
            glUniform3fv(glGetUniformLocation(planetProgramId, "cameraPos"), 1, glm::value_ptr(eyePosition));
            glUniformMatrix4fv(glGetUniformLocation(planetProgramId, "myViewMatrix"), 1, false, glm::value_ptr(viewMatrix));
            glUniformMatrix4fv(glGetUniformLocation(planetProgramId, "myPerspectiveMatrix"), 1, false, glm::value_ptr(perspectiveMatrix));
            glUniform1f(glGetUniformLocation(planetProgramId, "myTime"), myTime);
        }
        
        // load the planetMap shader defined above into the 2nd texture index
        glActiveTexture(GL_TEXTURE0 + 25 + i);
        glBindTexture(GL_TEXTURE_2D, myTextureManager->getTextureID("planetMap" + std::to_string(i)));

        // Set uniforms common to all planets
        glUniform1i(glGetUniformLocation(planetProgramId, "environMap"), 2 + i);
        glUniform1i(glGetUniformLocation(planetProgramId, "objectTexture"), 1);

        float scaler = (i==0)?1.0f:i*0.5f;

        if (i >= 3) {
            scaler = planets[i].scalar;
        }

        glUniform1f(glGetUniformLocation(planetProgramId, "scaler"), scaler);
       
        glm::mat4 planetModelMatrix = modelMatrix;
        
        // Calculate elliptical orbit parameters
        if (!planets[i].orbitPaused) {
            glm::vec3 v = (planets[i].position.x != 0.0f) ? glm::vec3(0, 1, 0) : glm::vec3(1, 0, 0);
            glm::vec3 t0 = glm::cross(planets[i].position, v); // arbitrary vec orthogonal to radius

            if (glm::length(t0) < 0.001f) { // assert t0 not 0 in length
                v = glm::vec3(0, 0, 1);
                t0 = glm::cross(planets[i].position, v);
            }

            t0 = glm::normalize(t0);
            float dotDirChangeT0 = glm::dot(planets[i].orbitDirChange, t0);
            float dotDirChangeDirChange = glm::dot(planets[i].orbitDirChange, planets[i].orbitDirChange);
            float lambda = (1.0f - dotDirChangeT0) / dotDirChangeDirChange;

            if (planets[i].recentlyDragged) { // upon drag, make sure new orbit direction is orthogonal to radius
                planets[i].orbitDirChange = glm::normalize(t0 + lambda * planets[i].orbitDirChange);
                planets[i].recentlyDragged = false;
            }

            float radius = glm::length(planets[i].position);
            float speed = planetSpeed / (1.5f * radius); // adjust speed based on radius
            glm::vec3 step = planets[i].orbitDirChange * speed;

            // adjust orbit direction with each time step
            glm::vec3 oldPlanetPos = planets[i].position;
            planets[i].position = radius * glm::normalize(oldPlanetPos + step);
            planets[i].orbitDirChange = glm::normalize(planets[i].position - oldPlanetPos);
        }

        planetModelMatrix = glm::translate(planetModelMatrix, planets[i].position);
        planetModelMatrix = glm::scale(planetModelMatrix, glm::vec3(planets[i].size));
        planets[i].modelMatrix = planetModelMatrix;

        // Set the model matrix for each planet
        glUniformMatrix4fv(glGetUniformLocation(planetProgramId, "myModelMatrix"), 1, false, glm::value_ptr(planetModelMatrix));
        if (planets[i].plyModel) {
            planets[i].plyModel->renderVBO(planetProgramId);
        } else {
            glBindVertexArray(planets[i].vao);
            glDrawElements(GL_TRIANGLES, planets[i].vertices, GL_UNSIGNED_INT, 0);
        }
    }


	// Draw the enviroment sphere
	unsigned int envProgramId =
		myShaderManager->getShaderProgram("environmentShaders")->programID;
	glUseProgram(envProgramId);

	glm::mat4 environmentModelMatrix = glm::mat4(1.0);
	environmentModelMatrix = glm::rotate(environmentModelMatrix, TO_RADIANS(rotWorldVec.x), glm::vec3(1.0f, 0.0f, 0.0f));
	environmentModelMatrix = glm::rotate(environmentModelMatrix, TO_RADIANS(rotWorldVec.y), glm::vec3(0.0f, 1.0f, 0.0f));
	environmentModelMatrix = glm::rotate(environmentModelMatrix, TO_RADIANS(rotWorldVec.z), glm::vec3(0.0f, 0.0f, 1.0f));

    // add back orbit angle ?? 
    // float rotateAngle = orbitAngle * -20;      // Each planet shifted by i

    glm::mat4 starsModelMatrix = glm::mat4(1.0f);
    // starsModelMatrix = glm::rotate(starsModelMatrix, rotateAngle, glm::vec3(0.0f, 1.0f, 0.0f));

	starsModelMatrix = glm::scale(starsModelMatrix, glm::vec3(11.0f));

	glUniform1i(glGetUniformLocation(envProgramId, "environMap"), 10);
	glUniform3fv(glGetUniformLocation(envProgramId, "lightPos"), 1, glm::value_ptr(lightPos));
	glUniformMatrix4fv(glGetUniformLocation(envProgramId, "myViewMatrix"), 1, false, glm::value_ptr(viewMatrix));
	glUniformMatrix4fv(glGetUniformLocation(envProgramId, "myModelMatrix"), 1, false, glm::value_ptr(starsModelMatrix));
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
    // pass in the noise texture
    glUniform1i(glGetUniformLocation(planeProgramId, "noiseTexture"), 11);
    // pass in the color map
    glUniform1i(glGetUniformLocation(planeProgramId, "colorTexture"), 18);
    glBindVertexArray(planeVAO);
    
    glDrawElements(GL_TRIANGLES, planevertices, GL_UNSIGNED_INT, 0);

    
    unsigned int proceduralPlanetProgramId = myShaderManager->getShaderProgram("proceduralPlanet")->programID;
    glUseProgram(proceduralPlanetProgramId);
    glUniformMatrix4fv(glGetUniformLocation(proceduralPlanetProgramId, "myViewMatrix"), 1, false, glm::value_ptr(viewMatrix));
    glUniformMatrix4fv(glGetUniformLocation(proceduralPlanetProgramId, "myModelMatrix"), 1, false, glm::value_ptr(modelMatrix));
    glUniformMatrix4fv(glGetUniformLocation(proceduralPlanetProgramId, "myPerspectiveMatrix"), 1, false, glm::value_ptr(perspectiveMatrix));
    glUniform1f(glGetUniformLocation(proceduralPlanetProgramId, "myTime"), myTime);
    glUniform3fv(glGetUniformLocation(proceduralPlanetProgramId, "lightPos"), 1, glm::value_ptr(lightPos));
    glUniform3fv(glGetUniformLocation(proceduralPlanetProgramId, "cameraPos"), 1, glm::value_ptr(cameraPosition));
    glUniform1f(glGetUniformLocation(proceduralPlanetProgramId, "oceanLevel"), 0.1f);
    glUniform1i(glGetUniformLocation(proceduralPlanetProgramId, "noiseTexture"), 13);
    glUniform1i(glGetUniformLocation(proceduralPlanetProgramId, "planeNoiseTexture"), 11);
    glUniform1i(glGetUniformLocation(proceduralPlanetProgramId, "colorTexture"), 18);
    glUniform1i(glGetUniformLocation(proceduralPlanetProgramId, "waterNormals"), 14);
    glBindVertexArray(icosphereVAO);
    glDrawElements(GL_TRIANGLES, icosphereVertices, GL_UNSIGNED_INT, 0);

    unsigned int buddahProgramId = myShaderManager->getShaderProgram("buddahShaders")->programID;
    glUseProgram(buddahProgramId);
    glUniformMatrix4fv(glGetUniformLocation(buddahProgramId, "myViewMatrix"), 1, false, glm::value_ptr(viewMatrix));
    glUniformMatrix4fv(glGetUniformLocation(buddahProgramId, "myModelMatrix"), 1, false, glm::value_ptr(modelMatrix));
    glUniformMatrix4fv(glGetUniformLocation(buddahProgramId, "myPerspectiveMatrix"), 1, false, glm::value_ptr(perspectiveMatrix));
    // glUniform3fv(glGetUniformLocation(buddahProgramId, "lightPos"), 1, glm::value_ptr(lightPos));
    glUniform3fv(glGetUniformLocation(buddahProgramId, "cameraPos"), 1, glm::value_ptr(cameraPosition));
    glUniform1f(glGetUniformLocation(buddahProgramId, "myTime"), myTime);
    // glUniform1f(glGetUniformLocation(buddahProgramId, "scaler"), 0.6);

    glUniform1i(glGetUniformLocation(buddahProgramId, "noiseTexture"), 11);
    myBuddahPLY->renderVBO(buddahProgramId);
    
    // draw the warp planet
    //drawWarp(modelMatrix, viewMatrix, myTime); 
}

void MyGLCanvas::drawWarp(glm::mat4 modelMatrix, glm::mat4 viewMatrix, float myTime) {
    int i = 3; 

    unsigned int planetProgramId = myShaderManager->getShaderProgram("warpShaders")->programID;
    glUseProgram(planetProgramId);
    
    glUniform3fv(glGetUniformLocation(planetProgramId, "cameraPos"), 1, glm::value_ptr(eyePosition));
    glUniformMatrix4fv(glGetUniformLocation(planetProgramId, "myViewMatrix"), 1, false, glm::value_ptr(viewMatrix));
    glUniformMatrix4fv(glGetUniformLocation(planetProgramId, "myPerspectiveMatrix"), 1, false, glm::value_ptr(perspectiveMatrix));
    glUniform1f(glGetUniformLocation(planetProgramId, "myTime"), myTime);


    glActiveTexture(GL_TEXTURE8);
    glBindTexture(GL_TEXTURE_2D, myTextureManager->getTextureID("earth"));

    // Set uniforms common to all planets
    // glUniform1i(glGetUniformLocation(planetProgramId, "noise"), 13);
    glUniform1i(glGetUniformLocation(planetProgramId, "objectTexture"), 1);

    glUniform1i(glGetUniformLocation(planetProgramId, "scaler"), 1);
    
    glm::mat4 planetModelMatrix = modelMatrix;
    
    // copied from above
    // Calculate elliptical orbit parameters
    // orbitAngle 
    // float angle = orbitAngle; // Offset angle for each planet
    // float radiusX = 2.0f + 1.5 * 0.5f;     // X-axis semi-major radius
    // float radiusZ = 1.5f + 0.5f;     // Z-axis semi-minor radius

    // // Calculate x, y, z positions
    // float x = radiusX * cos(angle); // Elliptical x-position
    // float z = radiusZ * sin(angle); // Elliptical z-position

    // // Add y-axis oscillation up to 50% of the orbit height
    // float maxY = 0.5f * radiusX; // Max height of oscillation
    // float y = maxY * sin(angle); // Oscillation along y-axis

    // // Translate the planet to its elliptical orbit position
    // planetModelMatrix = glm::translate(planetModelMatrix, glm::vec3(x, y, z));

    // // Scale the planets down 
    // planetModelMatrix = glm::scale(planetModelMatrix, glm::vec3(0.75f));

    // Set the model matrix for each planet
    // glUniformMatrix4fv(glGetUniformLocation(planetProgramId, "myModelMatrix"), 1, false, glm::value_ptr(planetModelMatrix));
    glUniform3fv(glGetUniformLocation(planetProgramId, "lightPos"), 1, glm::value_ptr(lightPos));
    glUniform1i(glGetUniformLocation(planetProgramId, "noise"), 11);
    
    glUniformMatrix4fv(glGetUniformLocation(planetProgramId, "myModelMatrix"), 1, false, glm::value_ptr(modelMatrix));
    // myWarpPLY->renderVBO(planetProgramId);
    glBindVertexArray(icosphereVAO);
    glDrawElements(GL_TRIANGLES, icosphereVertices, GL_UNSIGNED_INT, 0);
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
        case FL_DRAG: // if planet has been clicked, drag it
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
        case FL_PUSH: // upon click, if planet(s) clicked, pause the closest clicked planet's orbit
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
        // printf("keyboard event: key pressed: %c\n", Fl::event_key()); 
        switch (Fl::event_key()) {
            case 'w': eyePosition.y += 0.05f;  break;
            case 'a': eyePosition.x += 0.05f; break;
            case 's': eyePosition.y -= 0.05f;  break;
            case 'd': eyePosition.x -= 0.05f; break;
		}
		updateCamera(w(), h());
        break;
        case FL_MOUSEWHEEL:
        float MAX_SCALE = 2; 
        if (Fl::event_dy() > 0) {
            // Scrolling up
            // scaleFactor += 0.1f; 
            scaleFactor = (scaleFactor >= MAX_SCALE) ? MAX_SCALE : scaleFactor + 0.1f; 
        } else {
            // Scrolling down
            scaleFactor = (scaleFactor <= 0.1f) ? 0.1f : scaleFactor - 0.1f; 
        }
            break;
	}
	return Fl_Gl_Window::handle(e);
}

void MyGLCanvas::resize(int x, int y, int w, int h) {
	Fl_Gl_Window::resize(x, y, w, h);
	puts("resize called");
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