#include "MyGLCanvas.h"

MyGLCanvas::MyGLCanvas(int x, int y, int w, int h, const char *l) : Fl_Gl_Window(x, y, w, h, l) {
	mode(FL_RGB | FL_ALPHA | FL_DEPTH | FL_DOUBLE);
	wireframe = 0;
	orbits = 1;
	grid = 0;
	rotVec.x = rotVec.y = rotVec.z = 0.0f;
	eyePosition.x = 0.0f;
	eyePosition.y = 1.0f;
	eyePosition.z = -2.0f;
	solarSystem = new SolarSystem();
}

MyGLCanvas::~MyGLCanvas() {
	delete solarSystem;
}

void MyGLCanvas::draw() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (!valid()) {  //this is called when the GL canvas is set up for the first time...
		puts("establishing GL context");

		glViewport(0, 0, w(), h());
		updateCamera(w(), h());

		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glShadeModel(GL_SMOOTH);
		//glShadeModel(GL_FLAT);

		GLfloat light_pos0[] = { eyePosition.x, eyePosition.y, eyePosition.z, 0.0f };
		GLfloat ambient[] = { 0.7f, 0.7f, 0.7f, 1.0f };
		GLfloat diffuse[] = { 0.5f, 0.5f, 0.5f, 1.0f };

		glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
		glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
		glLightfv(GL_LIGHT0, GL_POSITION, light_pos0);

		glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
		glEnable(GL_COLOR_MATERIAL);

		glEnable(GL_LIGHTING);
		glEnable(GL_LIGHT0);

		/**********************************************/
		/*    Enable normalizing normal vectors       */
		/*    (e.g. normals not affected by glScalef) */
		/**********************************************/

		glEnable(GL_NORMALIZE);

		/****************************************/
		/*          Enable z-buferring          */
		/****************************************/

		glEnable(GL_DEPTH_TEST);
		glPolygonOffset(1, 1);
		glFrontFace(GL_CCW); //make sure that the ordering is counter-clock wise
	}

	// Clear the buffer of colors in each bit plane.
	// bit plane - A set of bits that are on or off (Think of a black and white image)
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	// Set the mode so we are modifying our objects.
	glMatrixMode(GL_MODELVIEW);
	// Load the identify matrix which gives us a base for our object transformations
	// (i.e. this is the default state)
	glLoadIdentity();

	//allow for user controlled rotation
	glRotatef(rotVec.x, 1.0, 0.0, 0.0);
	glRotatef(rotVec.y, 0.0, 1.0, 0.0);
	glRotatef(rotVec.z, 0.0, 0.0, 1.0);

	//draw the axes
	glBegin(GL_LINES);
	glColor3f(1.0, 0.0, 0.0);
	glVertex3f(0, 0, 0); glVertex3f(1.0, 0, 0);
	glColor3f(0.0, 1.0, 0.0);
	glVertex3f(0, 0, 0); glVertex3f(0.0, 1.0, 0);
	glColor3f(0.0, 0.0, 1.0);
	glVertex3f(0, 0, 0); glVertex3f(0, 0, 1.0);
	glEnd();


	if (wireframe) {
		glDisable(GL_LIGHTING);
		glDisable(GL_POLYGON_OFFSET_FILL);
		glColor3f(1.0, 1.0, 0.0);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		solarSystem->render();
	}
	else {
		glEnable(GL_LIGHTING);
		glEnable(GL_POLYGON_OFFSET_FILL);
		glColor3f(0.6f, 0.6f, 0.6f);
		glPolygonMode(GL_FRONT, GL_FILL);
		solarSystem->render();
	}

	if (orbits) {
		solarSystem->draw_orbits();
	}

	if (grid) {
		draw_grid();
	}
	//no need to call swap_buffer as it is automatically called
}

void MyGLCanvas::updateCamera(int width, int height) {
	float xy_aspect;
	xy_aspect = (float)width / (float)height;
	// Determine if we are modifying the camera(GL_PROJECITON) matrix(which is our viewing volume)
		// Otherwise we could modify the object transormations in our world with GL_MODELVIEW
	glMatrixMode(GL_PROJECTION);
	// Reset the Projection matrix to an identity matrix
	glLoadIdentity();
	gluPerspective(45.0f, xy_aspect, 0.1f, 10.0f);
	gluLookAt(eyePosition.x, eyePosition.y, eyePosition.z, eyePosition.x, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);
}


int MyGLCanvas::handle(int e) {
	//printf("Event was %s (%d)\n", fl_eventnames[e], e);
	switch (e) {
	case FL_ENTER: cursor(FL_CURSOR_HAND); break;
	case FL_LEAVE: cursor(FL_CURSOR_DEFAULT); break;
	case FL_KEYUP:
		printf("keyboard event: key pressed: %c\n", Fl::event_key());
		switch (Fl::event_key()) {
		case 'w': eyePosition.y += 0.05f;  break;
		case 'a': eyePosition.x += 0.05f; break;
		case 's': eyePosition.y -= 0.05f;  break;
		case 'd': eyePosition.x -= 0.05f; break;
		}
		updateCamera(w(), h());
		break;
	case FL_MOUSEWHEEL:
		printf("mousewheel: dx: %d, dy: %d\n", Fl::event_dx(), Fl::event_dy());
		eyePosition.z += Fl::event_dy() * -0.05f;
		updateCamera(w(), h());
		break;
	}

	return Fl_Gl_Window::handle(e);
}

void MyGLCanvas::resize(int x, int y, int w, int h) {
	Fl_Gl_Window::resize(x, y, w, h);
	puts("resize called");
}


void MyGLCanvas::draw_grid() {
	float grid_size = 10.0f;
	glDisable(GL_LIGHTING);
	glPushMatrix();
	glTranslatef(-grid_size / 2.0f, 0, -grid_size / 2.0f);
	// Draw a grid under the object
	for (int i = 0; i < grid_size; i++) {
		glBegin(GL_LINES);
		glColor3f(1.0f, 1.0f, 1.0f);
		glVertex3f(0.0f, 0.0f, i*1.0f);  glVertex3f(grid_size, 0.0f, i*1.0f); /* X axis grid */
		glVertex3f(i*1.0f, 0.0f, 0.0f);  glVertex3f(i*1.0f, 0.0f, grid_size); /* X axis grid */
		glEnd();
	}

	glBegin(GL_LINES);
	glColor3f(1.0f, 1.0f, 1.0f);
	glVertex3f(0.0f, 0.0f, grid_size);  glVertex3f(grid_size, 0.0f, grid_size); /* X axis grid */
	glVertex3f(grid_size, 0.0f, 0.0f);  glVertex3f(grid_size, 0.0f, grid_size); /* X axis grid */
	glEnd();
	glPopMatrix();
	glEnable(GL_LIGHTING);

}