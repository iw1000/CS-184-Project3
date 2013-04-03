
#include <vector>
#include <iostream>
#include <fstream>
#include <cmath>
#include <string>
#include <sstream>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/time.h>
#endif

#ifdef OSX
#include <GLUT/glut.h>
#include <OpenGL/glu.h>
#else
#include <GL/glut.h>
#include <GL/glu.h>
#endif

#include <time.h>
#include <math.h>
#include <stdio.h>


#include <GL/glut.h>

#include <glm/glm.hpp>

#define PI 3.14159265  // Should be used from mathlib
inline float sqr(float x) { return x*x; }

using namespace std;

struct controlPoint
{
	glm::vec3 pixelLocation;
};

struct drawPoint {
	glm::vec3 pixelLocation;
	glm::vec3 pixelNormal;
};

struct patch
{
	vector <controlPoint> allControlPoints;
	drawPoint** row;
};

vector <patch> allPatches;

bool* keyStates = new bool[256];
float x_position = 0.0f;
float y_position = 0.0f;
float x_rotation_angle = 0.0f;
float y_rotation_angle = 0.0f;
float zoomFactor = 1.0f;
int numberOfPatches = 0;
int numberOfSubdivisions = 0;
float subdivision = 0;
float error = 0.0f;
bool adaptive = false; 
bool wireToggle = false;
bool flat = true;

class Viewport;
class Viewport {
public:
		int w, h; // width and height
	};
	Viewport	viewport;

	glm::vec3 myNormalize (glm::vec3 input) {

		float x = input[0];
		float y = input[1];
		float z = input[2];

		float normalVectorNormalizingFactorMan = sqrt(sqr(x)+sqr(y)+sqr(z));

		x = x / normalVectorNormalizingFactorMan;
		y = y / normalVectorNormalizingFactorMan;
		z = z / normalVectorNormalizingFactorMan;

		return glm::vec3(x,y,z);
	}

	void myReshape(int w, int h) {
		viewport.w = w;
		viewport.h = h;
	glViewport(0,0,viewport.w,viewport.h);// sets the rectangle that will be the window
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();                // loading the identity matrix for the screen
	glOrtho(-1, 1, -1, 1, 4, -4);    // resize type = stretch
}

void initScene(){
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f); // Clear to black, fully transparent
	myReshape(viewport.w,viewport.h);

	glShadeModel(GL_FLAT);
	glLightModelf(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);

	//Material properties
	GLfloat ambient[] = {0.2f, 0.2f, 0.2f, 1.0f};
	GLfloat diffuse[] = {1.0f, 0.8f, 0.0f, 1.0f};
	GLfloat specular[] = {1.0f, 1.0f, 1.0f, 1.0f};
	GLfloat shine = 100.0f;
	glMaterialfv(GL_FRONT, GL_AMBIENT, ambient);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, specular);
	glMaterialf(GL_FRONT, GL_SHININESS, shine);

	//Add ambient light
	//GLfloat ambientColor[] = {0.2f, 0.2f, 0.2f, 1.0f}; //Color (0.2, 0.2, 0.2)
	//glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambientColor);

	//Add positioned light
	GLfloat lightColor0[] = {0.5f, 0.5f, 0.5f, 1.0f}; //Color (0.5, 0.5, 0.5)
	GLfloat lightPos0[] = {-1.0f, -1.0f, 5.0f, 1.0f}; 
	glLightfv(GL_LIGHT0, GL_DIFFUSE, lightColor0);
	glLightfv(GL_LIGHT0, GL_POSITION, lightPos0);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);	
	glEnable(GL_NORMALIZE); //Automatically normalize normals

}

glm::vec3 generateNormalTriangle(drawPoint pixel1, drawPoint pixel2, drawPoint pixel3) {
	glm::vec3 firstPixel = pixel1.pixelLocation;
	glm::vec3 secondPixel = pixel2.pixelLocation;
	glm::vec3 thirdPixel = pixel3.pixelLocation;
	glm::vec3 normalDirection = glm::normalize(glm::cross((secondPixel - firstPixel),(thirdPixel - firstPixel)));
	return myNormalize(normalDirection);
}

void drawPatches(){

	for (int numPatch = 0; numPatch < numberOfPatches; numPatch++){

		patch singlePatch = allPatches[numPatch];

		for (int i = 0; i <= numberOfSubdivisions - 1; i++) {

			for (int j = 0; j <= numberOfSubdivisions - 1; j++) {

				drawPoint point1 = singlePatch.row[i][j];
				drawPoint point2 = singlePatch.row[i][j+1];
				drawPoint point3 = singlePatch.row[i+1][j+1];
				drawPoint point4 = singlePatch.row[i+1][j];


				if (flat) {

					glShadeModel(GL_FLAT);
					glBegin(GL_TRIANGLES);
					glm::vec3 triangleNormal = generateNormalTriangle(point1, point2, point3);
					glNormal3f(triangleNormal[0], triangleNormal[1], triangleNormal[2]);
					glVertex3f(point1.pixelLocation[0], point1.pixelLocation[1], point1.pixelLocation[2]);
					glVertex3f(point2.pixelLocation[0], point2.pixelLocation[1], point2.pixelLocation[2]);
					glVertex3f(point3.pixelLocation[0], point3.pixelLocation[1], point3.pixelLocation[2]);
					glEnd();


					glBegin(GL_TRIANGLES);
					triangleNormal = generateNormalTriangle(point3, point4, point1);
					glNormal3f(triangleNormal[0], triangleNormal[1], triangleNormal[2]);
					glVertex3f(point3.pixelLocation[0], point3.pixelLocation[1], point3.pixelLocation[2]);
					glVertex3f(point4.pixelLocation[0], point4.pixelLocation[1], point4.pixelLocation[2]);
					glVertex3f(point1.pixelLocation[0], point1.pixelLocation[1], point1.pixelLocation[2]);
					glEnd();

				} else {

					glShadeModel(GL_SMOOTH);

					glBegin(GL_TRIANGLES);

					glNormal3f(point1.pixelNormal[0], point1.pixelNormal[1], point1.pixelNormal[2]);
					glVertex3f(point1.pixelLocation[0], point1.pixelLocation[1], point1.pixelLocation[2]);

					glNormal3f(point2.pixelNormal[0], point2.pixelNormal[1], point2.pixelNormal[2]);
					glVertex3f(point2.pixelLocation[0], point2.pixelLocation[1], point2.pixelLocation[2]);

					glNormal3f(point3.pixelNormal[0], point3.pixelNormal[1], point3.pixelNormal[2]);
					glVertex3f(point3.pixelLocation[0], point3.pixelLocation[1], point3.pixelLocation[2]);

					glEnd();



					glBegin(GL_TRIANGLES);

					glNormal3f(point3.pixelNormal[0], point3.pixelNormal[1], point3.pixelNormal[2]);
					glVertex3f(point3.pixelLocation[0], point3.pixelLocation[1], point3.pixelLocation[2]);

					glNormal3f(point4.pixelNormal[0], point4.pixelNormal[1], point4.pixelNormal[2]);
					glVertex3f(point4.pixelLocation[0], point4.pixelLocation[1], point4.pixelLocation[2]);

					glNormal3f(point1.pixelNormal[0], point1.pixelNormal[1], point1.pixelNormal[2]);
					glVertex3f(point1.pixelLocation[0], point1.pixelLocation[1], point1.pixelLocation[2]);
					
					glEnd();



				}
			}
		}
	}
}


void drawPatchesAdaptive() {

	for (int numPatch = 0; numPatch < numberOfPatches; numPatch++){
		patch singlePatch = allPatches[numPatch];

		for (int i = 0; i < 4; i++) {
			//drawPoint point1 = 

		}
		
	}

}

void reDrawMesh(){

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	glPushMatrix();

	glTranslatef(x_position, y_position, 0.0f);
	glRotatef(y_rotation_angle, 0.0f, 1.0f, 0.0f);
	glRotatef(x_rotation_angle, 1.0f, 0.0f, 0.0f);

	glColor3f(0.0f,0.65f,0.65f);
	drawPatches();

	glPopMatrix();
	glFlush();
	glutSwapBuffers();

}

void keyOperations(void) {
	if (keyStates[' ']) {
		exit(0);
	}
	else if(keyStates['+']) {
		zoomFactor -= 0.1f;

		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(zoomFactor*-1, zoomFactor*1, zoomFactor*-1, zoomFactor*1, 4, -4);

		reDrawMesh();

	}
	else if(keyStates['-']) {
		zoomFactor += 0.1f;

		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(zoomFactor*-1, zoomFactor*1, zoomFactor*-1, zoomFactor*1, 4, -4);

		reDrawMesh();
	}
	else if(keyStates['s']) {
		if (!flat) {
			flat = true;
		}
		else {
			flat = false;
		}
		reDrawMesh();
		keyStates['s'] = false;
	}
	else if(keyStates['w']) {
		if (!wireToggle) {
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			wireToggle = true;
		}
		else {
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			wireToggle = false;
		}
		reDrawMesh();
		keyStates['w'] = false;
	}
	else if (keyStates[GLUT_KEY_LEFT] and keyStates[GLUT_ACTIVE_SHIFT]) {
		x_position -= 0.05f;
		reDrawMesh();
	}
	else if (keyStates[GLUT_KEY_RIGHT] and keyStates[GLUT_ACTIVE_SHIFT]) {
		x_position += 0.05f;
		reDrawMesh();
	}
	else if (keyStates[GLUT_KEY_UP] and keyStates[GLUT_ACTIVE_SHIFT]) {
		y_position += 0.05f;
		reDrawMesh();
	}
	else if (keyStates[GLUT_KEY_DOWN] and keyStates[GLUT_ACTIVE_SHIFT]) {
		y_position -= 0.05f;
		reDrawMesh();
	}
	else if (keyStates[GLUT_KEY_LEFT]) {
		y_rotation_angle -= 1.0f;
		reDrawMesh();
	}
	else if (keyStates[GLUT_KEY_RIGHT]) {
		y_rotation_angle += 1.0f;
		reDrawMesh();
	}
	else if (keyStates[GLUT_KEY_UP]) {
		x_rotation_angle += 1.0f;
		reDrawMesh();
	}
	else if (keyStates[GLUT_KEY_DOWN]) {
		x_rotation_angle -= 1.0f;
		reDrawMesh();
	}

}

void keyPressed (unsigned char key, int x, int y) {  
	if (key == 'w' || key == 's'){
		keyStates[key] = false;
	}
	else {
		keyStates[key] = true;
	}
}

void keyPressedUp(unsigned char key, int x, int y) {
	if (key == 'w' || key == 's'){
		keyStates[key] = true;
	}
	else {
		keyStates[key] = false;
	}
}

void keySpecial(int key, int x, int y){
	int mod = glutGetModifiers();
	keyStates[key] = true;
	keyStates[mod] = true;
}

void keySpecialUp(int key, int x, int y){
	int mod = glutGetModifiers();
	keyStates[key] = false;
	keyStates[mod] = false;
}

bool init = true;
void myDisplay() {
	keyOperations();

	if (init) {
		glClear(GL_COLOR_BUFFER_BIT);                // clear the color buffer (sets everything to black)
		glMatrixMode(GL_MODELVIEW);                  // indicate we are specifying camera transformations
		glLoadIdentity();                            // make sure transformation is "zero'd"

		glColor3f(0.0f,0.65f,0.65f);                   // setting the color to pure red 90% for the rect

		if (adaptive) {
			drawPatchesAdaptive();
		}
		else {
			drawPatches();
		}		

		glFlush();
		glutSwapBuffers(); 

		init = false;
	}

	if(zoomFactor < 0.0f) {
		zoomFactor = 0.0f;
	}

	if(x_rotation_angle > 360.0f) {
		x_rotation_angle -= 360.0f;
	}
	else if (x_rotation_angle < 0.0f) {
		x_rotation_angle += 360.0f;
	}

	if (y_rotation_angle > 360.0f) {
		y_rotation_angle -= 360.0f;
	}
	else if (y_rotation_angle < 0.0f) {
		y_rotation_angle += 360.0f;
	}

}

void printPoint(controlPoint p) {
	cout << "Xvalue = " << p.pixelLocation[0] <<  "     Yvalue = " << p.pixelLocation[1] <<  "     Zvalue = " << p.pixelLocation[2] << endl;
}

void printPatch(patch p) {
	for (int i = 0; i < p.allControlPoints.size(); i++ ) {
		printPoint(p.allControlPoints[i]);
	}
}

void loadScene(string file) {
	std::ifstream inpfile(file.c_str());

	int count = 0;

	if(!inpfile.is_open()) {
		std::cout << "Unable to open file" << std::endl;
	} else {
		std::string line;
		//MatrixStack mst;
		vector <controlPoint> tempControlPoints;

		while(inpfile.good()) {
			std::vector<std::string> splitline;
			std::string buf;
			std::getline(inpfile,line);
			std::stringstream ss(line);
			while (ss >> buf) {
				splitline.push_back(buf);
			}
			if (count == 0){ // first line
				numberOfPatches = atof(splitline[0].c_str());

			}

			else if(splitline.size() == 0) { // empty line

				patch singlePatch = {};
				singlePatch.allControlPoints = tempControlPoints;
				tempControlPoints.clear();

				singlePatch.row = new drawPoint* [numberOfSubdivisions + 1];
				for(int i = 0; i <= numberOfSubdivisions; i++){

					singlePatch.row[i] = new drawPoint[numberOfSubdivisions + 1 ];
				}	

				allPatches.push_back(singlePatch);

			}
			//Ignore comments
			else if(splitline[0][0] == '#') {
				continue;
			}
			else {

				float x = atof(splitline[0].c_str());
				float y = atof(splitline[1].c_str());
				float z = atof(splitline[2].c_str());


				controlPoint singleControl = {};
				singleControl.pixelLocation =  glm::vec3(x, y, z);
				tempControlPoints.push_back(singleControl);

				x = atof(splitline[3].c_str());
				y = atof(splitline[4].c_str());
				z = atof(splitline[5].c_str());

				controlPoint singleControl1 = {};
				singleControl1.pixelLocation =  glm::vec3(x, y, z);
				tempControlPoints.push_back(singleControl1);	

				x = atof(splitline[6].c_str());
				y = atof(splitline[7].c_str());
				z = atof(splitline[8].c_str());

				controlPoint singleControl2 = {};
				singleControl2.pixelLocation =  glm::vec3(x, y, z);
				tempControlPoints.push_back(singleControl2);	

				x = atof(splitline[9].c_str());
				y = atof(splitline[10].c_str());
				z = atof(splitline[11].c_str());

				controlPoint singleControl3 = {};
				singleControl3.pixelLocation =  glm::vec3(x, y, z);
				tempControlPoints.push_back(singleControl3);	;
			}

			count++;
		}
	}

}

void myFrameMove() {
	//nothing here for now
#ifdef _WIN32
	Sleep(10);                                   //give ~10ms back to OS (so as not to waste the CPU)
#endif
	glutPostRedisplay(); // forces glut to call the display function (myDisplay())
}



drawPoint generateSingleCurve(vector <controlPoint> singleCurve, float u) {

	glm::vec3 A = (singleCurve[0].pixelLocation * (1-u)) +  (singleCurve[1].pixelLocation * u);
	glm::vec3 B = (singleCurve[1].pixelLocation * (1-u)) +  (singleCurve[2].pixelLocation * u);
	glm::vec3 C = (singleCurve[2].pixelLocation * (1-u)) +  (singleCurve[3].pixelLocation * u);
	glm::vec3 D = (A* (1-u)) +  (B* u);
	glm::vec3 E = (B* (1-u)) +  (C* u);
	glm::vec3 P = (D* (1-u)) +  (E* u);
	glm::vec3 derivativeOfP = E-D;
	derivativeOfP = derivativeOfP * (3-u);
	drawPoint toReturn = {};
	toReturn.pixelLocation = P;
	toReturn.pixelNormal = derivativeOfP;

	return toReturn;

}

vector <controlPoint> convertToCurve(patch p, int index1, int index2, int index3, int index4) {
	vector <controlPoint> singleCurve;
	singleCurve.push_back(p.allControlPoints[index1]);
	singleCurve.push_back(p.allControlPoints[index2]);
	singleCurve.push_back(p.allControlPoints[index3]);
	singleCurve.push_back(p.allControlPoints[index4]);
	return singleCurve;
}


vector <controlPoint> convertToCurve(drawPoint u1, drawPoint u2, drawPoint u3, drawPoint u4) {
	vector <controlPoint> singleCurve;

	controlPoint temp1 ={};
	temp1.pixelLocation = u1.pixelLocation;
	singleCurve.push_back(temp1);

	controlPoint temp2 ={};
	temp2.pixelLocation = u2.pixelLocation;
	singleCurve.push_back(temp2);

	controlPoint temp3 ={};
	temp3.pixelLocation = u3.pixelLocation;
	singleCurve.push_back(temp3);

	controlPoint temp4 ={};
	temp4.pixelLocation = u4.pixelLocation;
	singleCurve.push_back(temp4);

	return singleCurve;
}




drawPoint generatePlanarCurve(patch singlePatch, float u, float v) {

	vector <controlPoint> vCurve;

	drawPoint v1 = generateSingleCurve (convertToCurve(singlePatch, 0,1,2,3), u);
	drawPoint v2 = generateSingleCurve (convertToCurve(singlePatch, 4,5,6,7), u);
	drawPoint v3 = generateSingleCurve (convertToCurve(singlePatch, 8,9,10,11), u);
	drawPoint v4 = generateSingleCurve (convertToCurve(singlePatch, 12,13,14,15), u);


	drawPoint u1 = generateSingleCurve (convertToCurve(singlePatch, 0,4,8,12), v);
	drawPoint u2 = generateSingleCurve (convertToCurve(singlePatch, 1,5,9,13), v);
	drawPoint u3 = generateSingleCurve (convertToCurve(singlePatch, 2,6,10,14), v);
	drawPoint u4 = generateSingleCurve (convertToCurve(singlePatch, 3,7,11,15), v);

	drawPoint onPlaneV = generateSingleCurve(convertToCurve(v1, v2, v3, v4), v);
	drawPoint onPlaneU = generateSingleCurve(convertToCurve(u1, u2, u3, u4), u);

	glm::vec3 cross = glm::cross(onPlaneV.pixelNormal, onPlaneU.pixelNormal);

	drawPoint toReturn = {};

	toReturn.pixelLocation = onPlaneV.pixelLocation;
	toReturn.pixelNormal = glm::normalize(cross);
	return toReturn;



}


void generatePatchPoints(){

	for (int numPatch = 0; numPatch < numberOfPatches; numPatch++) {
		patch currentPatch = allPatches[numPatch];
		float u = 0;
		float v = 0;
		for (int i = 0; i <= numberOfSubdivisions; i++) {
			u = i * subdivision;
			for (int j = 0; j <= numberOfSubdivisions; j++) {
				v = j * subdivision;
				currentPatch.row[i][j] = generatePlanarCurve (currentPatch,u,v);
			}
		}
	}
}

void generateAdaptivePatchPoints() {

	for (int numPatch = 0; numPatch < numberOfPatches; numPatch++) {
		patch currentPatch = allPatches[numPatch];
		float u = 0;
		float v = 0;
		for (int i = 0; i <= numberOfSubdivisions; i++) {
			u = i * subdivision;
			for (int j = 0; j <= numberOfSubdivisions; j++) {
				v = j * subdivision;
				currentPatch.row[i][j] = generatePlanarCurve (currentPatch,u,v);
			}
		}
	}
}

int main(int argc, char *argv[]) {   // first argument is the program running

	glutInit(&argc, argv);

	string testName;	
	string division;
	string flag;

	int commandLine = 1;

	if (commandLine) {
		testName = argv[1];
		division = argv[2];
		subdivision = atof(division.c_str());
		flag = argv[3];
		if (flag == "-a") {
			adaptive = true;
			// error = distance evaluated at midpoints of polygon edges and center of each quadrilateral
			error = subdivision;
		}
	} else {
		adaptive = false; 
		subdivision = .1;
		testName = "test.bez";
	}

	float tempDivision = 1 / subdivision;

	numberOfSubdivisions = (int) tempDivision;

	//cout << adaptive << endl;
	//cout << subdivision << endl;
	//cout << numberOfSubdivisions << endl;

	loadScene(testName);

	if (adaptive) {
		generateAdaptivePatchPoints();
	}
	else {
		generatePatchPoints();
	}
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

	viewport.w = 600;
	viewport.h = 600;

	glutInitWindowSize(viewport.w, viewport.h);
	glutInitWindowPosition(0, 0);
	glutCreateWindow("CS184!");

	initScene();                                 // quick function to set up scene

	glutKeyboardFunc(keyPressed);
	glutKeyboardUpFunc(keyPressedUp);
	glutSpecialFunc(keySpecial);
	glutSpecialUpFunc(keySpecialUp);

	glutDisplayFunc(myDisplay);                  // function to run when its time to draw something
	glutReshapeFunc(myReshape);                  // function to run when the window gets resized
	glutIdleFunc(myFrameMove);                   // function to run when not handling any other task
	glutMainLoop();                              // infinite loop that will keep drawing and resizing and whatever else

	return 0;
}
