
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


void reDrawMesh();

struct controlPoint
{
	glm::vec3 pixelLocation;
};

struct adaptiveTriangle {

	controlPoint point1;
	glm::vec3 point1PixelNormal;
	float point1U;
	float point1V;

	controlPoint point2;
	glm::vec3 point2PixelNormal;
	float point2U;
	float point2V;

	controlPoint point3;
	glm::vec3 point3PixelNormal;
	float point3U;
	float point3V;

	controlPoint midpointBetween12;
	controlPoint midpointBetween23;
	controlPoint midpointBetween31;

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

vector <adaptiveTriangle> adaptiveTrianglesToRender;

bool* keyStates = new bool[256];
float x_position = 0.0f;
float y_position = 0.0f;
float x_rotation_angle = 0.0f;
float y_rotation_angle = 0.0f;
float zoomFactor = 1.0f;
float max_point = 0.0f;
int numberOfPatches = 0;
int numberOfSubdivisions = 0;
float subdivision = 0;
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



	glOrtho(-1*max_point, 1*max_point, -1*max_point, 1*max_point, 4, -4);    // resize type = stretch

	reDrawMesh();
}

void initScene(){
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f); // Clear to black, fully transparent
	myReshape(viewport.w,viewport.h);

	glShadeModel(GL_FLAT);
	glLightModelf(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);

	//Material properties
	GLfloat ambient[] = {0.2f, 0.2f, 0.2f, 1.0f};
	GLfloat diffuse[] = {0.5f, 0.7f, 0.9f, 1.0f};
	GLfloat specular[] = {1.0f, 1.0f, 1.0f, 1.0f};
	GLfloat shine = 50.0f;
	glMaterialfv(GL_FRONT, GL_AMBIENT, ambient);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, specular);
	glMaterialf(GL_FRONT, GL_SHININESS, shine);

	//Add ambient light
	//GLfloat ambientColor[] = {0.2f, 0.2f, 0.2f, 1.0f}; //Color (0.2, 0.2, 0.2)
	//glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambientColor);

	//Add positioned light
	GLfloat lightColor0[] = {0.5f, 0.5f, 0.5f, 1.0f}; //Color (0.5, 0.5, 0.5)
	GLfloat lightPos0[] = {-3.0f, -3.0f, 0.0f, 1.0f}; 
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


glm::vec3 generateNormalTriangle(controlPoint pixel1, controlPoint pixel2, controlPoint pixel3) {
	glm::vec3 firstPixel = pixel1.pixelLocation;
	glm::vec3 secondPixel = pixel2.pixelLocation;
	glm::vec3 thirdPixel = pixel3.pixelLocation;
	glm::vec3 normalDirection = glm::normalize(glm::cross((secondPixel - firstPixel),(thirdPixel - firstPixel)));
	return myNormalize(normalDirection);
}


void drawPatches(){


	if (adaptive) {

		for (int i = 0; i < adaptiveTrianglesToRender.size(); i++) { 

			
			adaptiveTriangle tempTri = adaptiveTrianglesToRender[i];


			if (flat) {

				glShadeModel(GL_FLAT);
				glBegin(GL_TRIANGLES);
				glm::vec3 triangleNormal = generateNormalTriangle(tempTri.point1, tempTri.point2, tempTri.point3);
				glNormal3f(triangleNormal[0], triangleNormal[1], triangleNormal[2] );
				glVertex3f(tempTri.point1.pixelLocation[0], tempTri.point1.pixelLocation[1], tempTri.point1.pixelLocation[2]);
				glVertex3f(tempTri.point2.pixelLocation[0], tempTri.point2.pixelLocation[1], tempTri.point2.pixelLocation[2]);
				glVertex3f(tempTri.point3.pixelLocation[0], tempTri.point3.pixelLocation[1], tempTri.point3.pixelLocation[2]);
				glEnd();

			}else {

				glShadeModel(GL_SMOOTH);
				glBegin(GL_TRIANGLES);

				glNormal3f(tempTri.point1PixelNormal[0], tempTri.point1PixelNormal[1], tempTri.point1PixelNormal[2]);
				glVertex3f(tempTri.point1.pixelLocation[0], tempTri.point1.pixelLocation[1], tempTri.point1.pixelLocation[2]);

				glNormal3f(tempTri.point2PixelNormal[0], tempTri.point2PixelNormal[1], tempTri.point2PixelNormal[2]);
				glVertex3f(tempTri.point2.pixelLocation[0], tempTri.point2.pixelLocation[1], tempTri.point2.pixelLocation[2]);

				glNormal3f(tempTri.point3PixelNormal[0], tempTri.point3PixelNormal[1], tempTri.point3PixelNormal[2]);
				glVertex3f(tempTri.point3.pixelLocation[0], tempTri.point3.pixelLocation[1], tempTri.point3.pixelLocation[2]);

				glEnd();


			}
		}

	}
	else {


		for (int numPatch = 0; numPatch < numberOfPatches; numPatch++){

			patch singlePatch = allPatches[numPatch];


			int temp = ceil (1 / subdivision) - 1;

			for (int i = 0; i <= temp; i++) {

				for (int j = 0; j <= temp; j++) {

					drawPoint point1 = singlePatch.row[i][j];
					drawPoint point2 = singlePatch.row[i][j+1];
					drawPoint point3 = singlePatch.row[i+1][j+1];
					drawPoint point4 = singlePatch.row[i+1][j];



					if (flat) {

						glShadeModel(GL_FLAT);

						glBegin(GL_QUADS);
						glm::vec3 triangleNormal = generateNormalTriangle(point1, point2, point3);
						glNormal3f(triangleNormal[0], triangleNormal[1], triangleNormal[2] );
						glVertex3f(point1.pixelLocation[0], point1.pixelLocation[1], point1.pixelLocation[2]);
						glVertex3f(point2.pixelLocation[0], point2.pixelLocation[1], point2.pixelLocation[2]);
						glVertex3f(point3.pixelLocation[0], point3.pixelLocation[1], point3.pixelLocation[2]);
						glVertex3f(point4.pixelLocation[0], point4.pixelLocation[1], point4.pixelLocation[2]);
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
		glOrtho(zoomFactor*-1*max_point, zoomFactor*1*max_point, zoomFactor*-1*max_point, zoomFactor*1*max_point, 4, -4);


		reDrawMesh();

	}
	else if(keyStates['-']) {
		zoomFactor += 0.1f;

		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(zoomFactor*-1*max_point, zoomFactor*1*max_point, zoomFactor*-1*max_point, zoomFactor*1*max_point, 4, -4);
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
		y_rotation_angle -= 3.0f;
		reDrawMesh();
	}
	else if (keyStates[GLUT_KEY_RIGHT]) {
		y_rotation_angle += 3.0f;
		reDrawMesh();
	}
	else if (keyStates[GLUT_KEY_UP]) {
		x_rotation_angle += 3.0f;
		reDrawMesh();
	}
	else if (keyStates[GLUT_KEY_DOWN]) {
		x_rotation_angle -= 3.0f;
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

	if (init) {

		reDrawMesh();

		init = false;
	}

	keyOperations();

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

void printPoint(drawPoint p) {
	cout << "Xvalue = " << p.pixelLocation[0] <<  "     Yvalue = " << p.pixelLocation[1] <<  "     Zvalue = " << p.pixelLocation[2] << endl;
}


void printPoint(controlPoint p) {
	cout << "Xvalue = " << p.pixelLocation[0] <<  "     Yvalue = " << p.pixelLocation[1] <<  "     Zvalue = " << p.pixelLocation[2] << endl;
}


void printPatch(patch p) {
	for (int i = 0; i < p.allControlPoints.size(); i++ ) {
		printPoint(p.allControlPoints[i]);
	}

}

void printTriangle(adaptiveTriangle p) {
	printPoint(p.point1);
	printPoint(p.point2);
	printPoint(p.point3);
}

void myFrameMove() {
		//nothing here for now
#ifdef _WIN32
		Sleep(10);                                   //give ~10ms back to OS (so as not to waste the CPU)
#endif
		glutPostRedisplay(); 
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

	float calculateMagnitude(glm::vec3 vect) {
		return sqrt(sqr(vect[0]) + sqr(vect[1]) + sqr(vect[2]));
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

				int temp = ceil (1 / subdivision) + 1;

				singlePatch.row = new drawPoint* [temp];  /// 3 now   /// numberOfSub = 1

				for(int i = 0; i <= ceil (1 / subdivision); i++){

					singlePatch.row[i] = new drawPoint[temp];
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
				if (max_point < calculateMagnitude(singleControl.pixelLocation)) {
					max_point = calculateMagnitude(singleControl.pixelLocation);
				}
				tempControlPoints.push_back(singleControl);

				x = atof(splitline[3].c_str());
				y = atof(splitline[4].c_str());
				z = atof(splitline[5].c_str());

				controlPoint singleControl1 = {};
				singleControl1.pixelLocation =  glm::vec3(x, y, z);

				if (max_point < calculateMagnitude(singleControl1.pixelLocation)) {
					max_point = calculateMagnitude(singleControl1.pixelLocation);
				}

				tempControlPoints.push_back(singleControl1);	

				x = atof(splitline[6].c_str());
				y = atof(splitline[7].c_str());
				z = atof(splitline[8].c_str());

				controlPoint singleControl2 = {};
				singleControl2.pixelLocation =  glm::vec3(x, y, z);

				if (max_point < calculateMagnitude(singleControl2.pixelLocation)) {
					max_point = calculateMagnitude(singleControl2.pixelLocation);
				}

				tempControlPoints.push_back(singleControl2);	

				x = atof(splitline[9].c_str());
				y = atof(splitline[10].c_str());
				z = atof(splitline[11].c_str());

				controlPoint singleControl3 = {};
				singleControl3.pixelLocation =  glm::vec3(x, y, z);

				if (max_point < calculateMagnitude(singleControl3.pixelLocation)) {
					max_point = calculateMagnitude(singleControl3.pixelLocation);
				}

				tempControlPoints.push_back(singleControl3);


			}

			count++;
		}
	}

}



bool pointsEqual(controlPoint point1, controlPoint point2) {
	return (point1.pixelLocation[0] == point2.pixelLocation[0] && point1.pixelLocation[1] == point2.pixelLocation[1] && point1.pixelLocation[2] == point2.pixelLocation[2]);
}


void pointTest(adaptiveTriangle temp) {

	if (pointsEqual(temp.point1, temp.point2)){
		cout << "1 and 2 are the same" << endl;
	} else if (pointsEqual(temp.point1, temp.point3)) {
		cout << "1 and 3 are the same" << endl;
	} else if (pointsEqual(temp.point2, temp.point3)) {
		cout << "2 and 3 are the same" << endl;
	}
}

adaptiveTriangle generateTriangle(controlPoint first, controlPoint second, controlPoint third) {

	adaptiveTriangle toReturn = {};
	toReturn.point1 = first;
	toReturn.point2 = second;
	toReturn.point3 = third;

	glm::vec3 mid1(((toReturn.point1.pixelLocation[0] + toReturn.point2.pixelLocation[0])/2), 
		((toReturn.point1.pixelLocation[1] + toReturn.point2.pixelLocation[1])/2), 
		((toReturn.point1.pixelLocation[2] + toReturn.point2.pixelLocation[2])/2));

	toReturn.midpointBetween12.pixelLocation = mid1;

	glm::vec3 mid2(((toReturn.point2.pixelLocation[0] + toReturn.point3.pixelLocation[0])/2), 
		((toReturn.point2.pixelLocation[1] + toReturn.point3.pixelLocation[1])/2), 
		((toReturn.point2.pixelLocation[2] + toReturn.point3.pixelLocation[2])/2));

	toReturn.midpointBetween23.pixelLocation = mid2;

	glm::vec3 mid3(((toReturn.point3.pixelLocation[0] + toReturn.point1.pixelLocation[0])/2), 
		((toReturn.point3.pixelLocation[1] + toReturn.point1.pixelLocation[1])/2), 
		((toReturn.point3.pixelLocation[2] + toReturn.point1.pixelLocation[2])/2));

	toReturn.midpointBetween31.pixelLocation = mid3;

	return toReturn;

}


bool closeEnough(drawPoint onPlane, controlPoint triangleMid) {

	float total = 0;
	total = total + abs (onPlane.pixelLocation[0] - triangleMid.pixelLocation[0]);
	total = total + abs (onPlane.pixelLocation[1] - triangleMid.pixelLocation[1]);
	total = total + abs (onPlane.pixelLocation[2] - triangleMid.pixelLocation[2]);

	return (total < subdivision);
}


controlPoint fromDrawToControl(drawPoint temp) {
	controlPoint toReturn = {};
	toReturn.pixelLocation = temp.pixelLocation;
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


bool cornerTestFine (patch singlePatch, adaptiveTriangle triangleToSplit) {

	drawPoint corner1 = generatePlanarCurve(singlePatch, triangleToSplit.point1U, triangleToSplit.point1V);
	drawPoint corner2 = generatePlanarCurve(singlePatch, triangleToSplit.point2U, triangleToSplit.point2V);
	drawPoint corner3 = generatePlanarCurve(singlePatch, triangleToSplit.point3U, triangleToSplit.point3V);

	float total1 = 0;
	total1 = total1 + abs(corner1.pixelLocation[0] - triangleToSplit.point1.pixelLocation[0]);
	total1 = total1 + abs(corner1.pixelLocation[1] - triangleToSplit.point1.pixelLocation[1]);
	total1 = total1 + abs(corner1.pixelLocation[2] - triangleToSplit.point1.pixelLocation[2]);

	float total2 = 0;
	total2 = total2 + abs(corner2.pixelLocation[0] - triangleToSplit.point2.pixelLocation[0]);
	total2 = total2 + abs(corner2.pixelLocation[1] - triangleToSplit.point2.pixelLocation[1]);
	total2 = total2 + abs(corner2.pixelLocation[2] - triangleToSplit.point2.pixelLocation[2]);

	float total3 = 0;
	total3 = total3 + abs(corner3.pixelLocation[0] - triangleToSplit.point3.pixelLocation[0]);
	total3 = total3 + abs(corner3.pixelLocation[1] - triangleToSplit.point3.pixelLocation[1]);
	total3 = total3 + abs(corner3.pixelLocation[2] - triangleToSplit.point3.pixelLocation[2]);

	if ((total1 + total2 + total3) != 0) {
		cout << "total 1:  "<< total1 << "   total 2:  "<< total2 << "   total 3:  "<< total3<< endl;
		cout << "triangle" << endl;
		return false;
	}
	return true;

}

void recursivelySplit(patch singlePatch, adaptiveTriangle triangleToSplit, int depth){

	if (cornerTestFine(singlePatch, triangleToSplit) == false) {
		return;
	}

	drawPoint midTest1 =  generatePlanarCurve(singlePatch, ((triangleToSplit.point1U + triangleToSplit.point2U) / 2) , ((triangleToSplit.point1V + triangleToSplit.point2V) / 2));
	drawPoint midTest2 =  generatePlanarCurve(singlePatch, ((triangleToSplit.point2U + triangleToSplit.point3U) / 2) , ((triangleToSplit.point2V + triangleToSplit.point3V) / 2));
	drawPoint midTest3 =  generatePlanarCurve(singlePatch, ((triangleToSplit.point3U + triangleToSplit.point1U) / 2) , ((triangleToSplit.point3V + triangleToSplit.point1V) / 2));

	bool firstMidCheck = closeEnough(midTest1, triangleToSplit.midpointBetween12);
	bool secondMidCheck = closeEnough(midTest2, triangleToSplit.midpointBetween23);
	bool thirdMidCheck = closeEnough(midTest3, triangleToSplit.midpointBetween31);

	if (firstMidCheck == false) {

		if (secondMidCheck == false) {

			if (thirdMidCheck == false) {  

					/// all three wrong

				adaptiveTriangle firstSplit =  generateTriangle(triangleToSplit.point1, fromDrawToControl(midTest1),  fromDrawToControl(midTest3));
				firstSplit.point1U = triangleToSplit.point1U;
				firstSplit.point1V = triangleToSplit.point1V;
				firstSplit.point2U = ((triangleToSplit.point1U + triangleToSplit.point2U) / 2);
				firstSplit.point2V = ((triangleToSplit.point1V + triangleToSplit.point2V) / 2);
				firstSplit.point3U = ((triangleToSplit.point3U + triangleToSplit.point1U) / 2);
				firstSplit.point3V = ((triangleToSplit.point3V + triangleToSplit.point1V) / 2);
				recursivelySplit(singlePatch, firstSplit, depth + 1);

				adaptiveTriangle secondSplit =  generateTriangle(fromDrawToControl(midTest1), triangleToSplit.point2, fromDrawToControl(midTest2));
				secondSplit.point1U = ((triangleToSplit.point1U + triangleToSplit.point2U) / 2);
				secondSplit.point1V = ((triangleToSplit.point1V + triangleToSplit.point2V) / 2);
				secondSplit.point2U = triangleToSplit.point2U;
				secondSplit.point2V = triangleToSplit.point2V;
				secondSplit.point3U = ((triangleToSplit.point2U + triangleToSplit.point3U) / 2);
				secondSplit.point3V = ((triangleToSplit.point2V + triangleToSplit.point3V) / 2);
				recursivelySplit(singlePatch, secondSplit , depth + 1);

				adaptiveTriangle thirdSplit =  generateTriangle(fromDrawToControl(midTest3), fromDrawToControl(midTest2), triangleToSplit.point3);
				thirdSplit.point1U = ((triangleToSplit.point3U + triangleToSplit.point1U) / 2);
				thirdSplit.point1V = ((triangleToSplit.point3V + triangleToSplit.point1V) / 2);
				thirdSplit.point2U = ((triangleToSplit.point2U + triangleToSplit.point3U) / 2);
				thirdSplit.point2V = ((triangleToSplit.point2V + triangleToSplit.point3V) / 2);
				thirdSplit.point3U = triangleToSplit.point3U;
				thirdSplit.point3V = triangleToSplit.point3V;
				recursivelySplit(singlePatch, thirdSplit, depth + 1 );

				adaptiveTriangle middleSplit =  generateTriangle(fromDrawToControl(midTest1), fromDrawToControl(midTest2), fromDrawToControl(midTest3));
				middleSplit.point1U = ((triangleToSplit.point1U + triangleToSplit.point2U) / 2);
				middleSplit.point1V = ((triangleToSplit.point1V + triangleToSplit.point2V) / 2);
				middleSplit.point2U = ((triangleToSplit.point2U + triangleToSplit.point3U) / 2);
				middleSplit.point2V = ((triangleToSplit.point2V + triangleToSplit.point3V) / 2);
				middleSplit.point3U = ((triangleToSplit.point3U + triangleToSplit.point1U) / 2);
				middleSplit.point3V = ((triangleToSplit.point3V + triangleToSplit.point1V) / 2);
				recursivelySplit(singlePatch, middleSplit, depth + 1);

			} else {

					// 1 and 2 are wrong

				adaptiveTriangle firstSplit =  generateTriangle(triangleToSplit.point1, fromDrawToControl(midTest1),  fromDrawToControl(midTest2));
				firstSplit.point1U = triangleToSplit.point1U;
				firstSplit.point1V = triangleToSplit.point1V;
				firstSplit.point2U = ((triangleToSplit.point1U + triangleToSplit.point2U) / 2);
				firstSplit.point2V = ((triangleToSplit.point1V + triangleToSplit.point2V) / 2);
				firstSplit.point3U = ((triangleToSplit.point2U + triangleToSplit.point3U) / 2);
				firstSplit.point3V = ((triangleToSplit.point2V + triangleToSplit.point3V) / 2);
				recursivelySplit(singlePatch, firstSplit, depth + 1);

				adaptiveTriangle secondSplit =  generateTriangle(fromDrawToControl(midTest1), triangleToSplit.point2, fromDrawToControl(midTest2));
				secondSplit.point1U = ((triangleToSplit.point1U + triangleToSplit.point2U) / 2);
				secondSplit.point1V = ((triangleToSplit.point1V + triangleToSplit.point2V) / 2);
				secondSplit.point2U = triangleToSplit.point2U;
				secondSplit.point2V = triangleToSplit.point2V;
				secondSplit.point3U = ((triangleToSplit.point2U + triangleToSplit.point3U) / 2);
				secondSplit.point3V = ((triangleToSplit.point2V + triangleToSplit.point3V) / 2);
				recursivelySplit(singlePatch, secondSplit, depth + 1);

				adaptiveTriangle thirdSplit =  generateTriangle(triangleToSplit.point1, fromDrawToControl(midTest2), triangleToSplit.point3);
				thirdSplit.point1U = triangleToSplit.point1U;
				thirdSplit.point1V = triangleToSplit.point1V;
				thirdSplit.point2U = ((triangleToSplit.point2U + triangleToSplit.point3U) / 2);
				thirdSplit.point2V = ((triangleToSplit.point2V + triangleToSplit.point3V) / 2);
				thirdSplit.point3U = triangleToSplit.point3U;
				thirdSplit.point3V = triangleToSplit.point3V;
				recursivelySplit(singlePatch, thirdSplit, depth + 1);

			}

		} else if (thirdMidCheck == false) {

				// 1 and 3 are wrong

			adaptiveTriangle firstSplit =  generateTriangle(triangleToSplit.point1, fromDrawToControl(midTest1),  fromDrawToControl(midTest3));
			firstSplit.point1U = triangleToSplit.point1U;
			firstSplit.point1V = triangleToSplit.point1V;
			firstSplit.point2U = ((triangleToSplit.point1U + triangleToSplit.point2U) / 2);
			firstSplit.point2V = ((triangleToSplit.point1V + triangleToSplit.point2V) / 2);
			firstSplit.point3U = ((triangleToSplit.point3U + triangleToSplit.point1U) / 2);
			firstSplit.point3V = ((triangleToSplit.point3V + triangleToSplit.point1V) / 2);
			recursivelySplit(singlePatch, firstSplit, depth + 1);

			adaptiveTriangle secondSplit =  generateTriangle(fromDrawToControl(midTest1), triangleToSplit.point2, triangleToSplit.point3);
			secondSplit.point1U = ((triangleToSplit.point1U + triangleToSplit.point2U) / 2);
			secondSplit.point1V = ((triangleToSplit.point1V + triangleToSplit.point2V) / 2);
			secondSplit.point2U = triangleToSplit.point2U;
			secondSplit.point2V = triangleToSplit.point2V;
			secondSplit.point3U = triangleToSplit.point3U;
			secondSplit.point3V = triangleToSplit.point3V;
			recursivelySplit(singlePatch, secondSplit, depth + 1);

			adaptiveTriangle thirdSplit =  generateTriangle(fromDrawToControl(midTest3), fromDrawToControl(midTest1), triangleToSplit.point3);
			thirdSplit.point1U = ((triangleToSplit.point3U + triangleToSplit.point1U) / 2);
			thirdSplit.point1V = ((triangleToSplit.point3V + triangleToSplit.point1V) / 2);
			thirdSplit.point2U = ((triangleToSplit.point1U + triangleToSplit.point2U) / 2);
			thirdSplit.point2V = ((triangleToSplit.point1V + triangleToSplit.point2V) / 2);
			thirdSplit.point3U = triangleToSplit.point3U;
			thirdSplit.point3V = triangleToSplit.point3V;
			recursivelySplit(singlePatch, thirdSplit, depth + 1);

		} else {

				// just 1 is wrong

			adaptiveTriangle firstSplit =  generateTriangle(triangleToSplit.point1, fromDrawToControl(midTest1), triangleToSplit.point3);
			firstSplit.point1U = triangleToSplit.point1U;
			firstSplit.point1V = triangleToSplit.point1V;
			firstSplit.point2U = ((triangleToSplit.point1U + triangleToSplit.point2U) / 2);
			firstSplit.point2V = ((triangleToSplit.point1V + triangleToSplit.point2V) / 2);
			firstSplit.point3U = triangleToSplit.point3U;
			firstSplit.point3V = triangleToSplit.point3V;
			recursivelySplit(singlePatch, firstSplit, depth + 1);

			adaptiveTriangle secondSplit =  generateTriangle(fromDrawToControl(midTest1), triangleToSplit.point2,triangleToSplit.point3);
			secondSplit.point1U = ((triangleToSplit.point1U + triangleToSplit.point2U) / 2);
			secondSplit.point1V = ((triangleToSplit.point1V + triangleToSplit.point2V) / 2);
			secondSplit.point2U = triangleToSplit.point2U;
			secondSplit.point2V = triangleToSplit.point2V;
			secondSplit.point3U = triangleToSplit.point3U;
			secondSplit.point3V = triangleToSplit.point3V;
			recursivelySplit(singlePatch, secondSplit, depth + 1);

		}

	} else if (secondMidCheck == false) {

		if (thirdMidCheck == false) {
				// 2 and 3 are wrong

			adaptiveTriangle firstSplit =  generateTriangle(triangleToSplit.point1, triangleToSplit.point2, fromDrawToControl(midTest3));
			firstSplit.point1U = triangleToSplit.point1U;
			firstSplit.point1V = triangleToSplit.point1V;
			firstSplit.point2U =  triangleToSplit.point2U;
			firstSplit.point2V =  triangleToSplit.point2V;
			firstSplit.point3U = ((triangleToSplit.point3U + triangleToSplit.point1U) / 2);
			firstSplit.point3V = ((triangleToSplit.point3V + triangleToSplit.point1V) / 2);
			recursivelySplit(singlePatch, firstSplit, depth + 1);

			adaptiveTriangle secondSplit =  generateTriangle(fromDrawToControl(midTest3), triangleToSplit.point2, fromDrawToControl(midTest2));
			secondSplit.point1U = ((triangleToSplit.point3U + triangleToSplit.point1U) / 2);
			secondSplit.point1V = ((triangleToSplit.point3V + triangleToSplit.point1V) / 2);
			secondSplit.point2U = triangleToSplit.point2U;
			secondSplit.point2V = triangleToSplit.point2V;
			secondSplit.point3U = ((triangleToSplit.point2U + triangleToSplit.point3U) / 2);
			secondSplit.point3V = ((triangleToSplit.point2V + triangleToSplit.point3V) / 2);
			recursivelySplit(singlePatch, secondSplit, depth + 1);

			adaptiveTriangle thirdSplit =  generateTriangle(fromDrawToControl(midTest3), fromDrawToControl(midTest2), triangleToSplit.point3);
			thirdSplit.point1U = ((triangleToSplit.point3U + triangleToSplit.point1U) / 2);
			thirdSplit.point1V = ((triangleToSplit.point3V + triangleToSplit.point1V) / 2);
			thirdSplit.point2U = ((triangleToSplit.point2U + triangleToSplit.point3U) / 2);
			thirdSplit.point2V = ((triangleToSplit.point2V + triangleToSplit.point3V) / 2);
			thirdSplit.point3U = triangleToSplit.point3U;
			thirdSplit.point3V = triangleToSplit.point3V;
			recursivelySplit(singlePatch, thirdSplit, depth + 1);

		} else {
				/// just 2 is wrong

			adaptiveTriangle firstSplit =  generateTriangle(triangleToSplit.point1, triangleToSplit.point2, fromDrawToControl(midTest2));
			firstSplit.point1U = triangleToSplit.point1U;
			firstSplit.point1V = triangleToSplit.point1V;
			firstSplit.point2U =  triangleToSplit.point2U;
			firstSplit.point2V =  triangleToSplit.point2V;
			firstSplit.point3U = ((triangleToSplit.point2U + triangleToSplit.point3U) / 2);
			firstSplit.point3V = ((triangleToSplit.point2V + triangleToSplit.point3V) / 2);
			recursivelySplit(singlePatch, firstSplit, depth + 1);

			adaptiveTriangle secondSplit =  generateTriangle(triangleToSplit.point1, fromDrawToControl(midTest2) , triangleToSplit.point3);
			secondSplit.point1U = triangleToSplit.point1U;
			secondSplit.point1V = triangleToSplit.point1V;
			secondSplit.point2U = ((triangleToSplit.point2U + triangleToSplit.point3U) / 2);
			secondSplit.point2V = ((triangleToSplit.point2V + triangleToSplit.point3V) / 2);
			secondSplit.point3U = triangleToSplit.point3U;
			secondSplit.point3V = triangleToSplit.point3V;
			recursivelySplit(singlePatch, secondSplit, depth + 1);

		}

	}  else if (thirdMidCheck == false) {
			//just third is wrong

		adaptiveTriangle firstSplit =  generateTriangle(triangleToSplit.point1, triangleToSplit.point2, fromDrawToControl(midTest3));
		firstSplit.point1U = triangleToSplit.point1U;
		firstSplit.point1V = triangleToSplit.point1V;
		firstSplit.point2U =  triangleToSplit.point2U;
		firstSplit.point2V =  triangleToSplit.point2V;
		firstSplit.point3U = ((triangleToSplit.point3U + triangleToSplit.point1U) / 2);
		firstSplit.point3V = ((triangleToSplit.point3V + triangleToSplit.point1V) / 2);
		recursivelySplit(singlePatch, firstSplit, depth + 1);

		adaptiveTriangle secondSplit =  generateTriangle(fromDrawToControl(midTest3), triangleToSplit.point2, triangleToSplit.point3);
		secondSplit.point1U = ((triangleToSplit.point3U + triangleToSplit.point1U) / 2);
		secondSplit.point1V = ((triangleToSplit.point3V + triangleToSplit.point1V) / 2);
		secondSplit.point2U =  triangleToSplit.point2U;
		secondSplit.point2V =  triangleToSplit.point2V;
		secondSplit.point3U = triangleToSplit.point3U;
		secondSplit.point3V = triangleToSplit.point3V;
		recursivelySplit(singlePatch, secondSplit, depth + 1);

	} else {
			///everything fine

		//calculate the 3 normals

		drawPoint point1Normals = generatePlanarCurve(singlePatch, triangleToSplit.point1U, triangleToSplit.point1V);
		drawPoint point2Normals = generatePlanarCurve(singlePatch, triangleToSplit.point2U, triangleToSplit.point2V);
		drawPoint point3Normals = generatePlanarCurve(singlePatch, triangleToSplit.point3U, triangleToSplit.point3V);

		triangleToSplit.point1PixelNormal = point1Normals.pixelNormal;
		triangleToSplit.point2PixelNormal = point2Normals.pixelNormal;
		triangleToSplit.point3PixelNormal = point3Normals.pixelNormal;

		adaptiveTrianglesToRender.push_back(triangleToSplit);

	}
}


void generateAdaptiveTesselation() {

	for (int i = 0; i < numberOfPatches; i++) {

		patch singlePatch = allPatches[i];

		controlPoint topLeft = singlePatch.allControlPoints[0];
		controlPoint topRight = singlePatch.allControlPoints[3];
		controlPoint bottomLeft = singlePatch.allControlPoints[12];
		controlPoint bottomRight = singlePatch.allControlPoints[15];

		adaptiveTriangle firstTri = generateTriangle(topLeft, bottomRight, topRight);
		firstTri.point1U = 0;
		firstTri.point1V = 0;
		firstTri.point2U = 1;
		firstTri.point2V = 1;
		firstTri.point3U = 1;
		firstTri.point3V = 0;
		recursivelySplit(singlePatch, firstTri, 0);

		adaptiveTriangle secondTri = generateTriangle(topLeft, bottomLeft, bottomRight);
		secondTri.point1U = 0;
		secondTri.point1V = 0;
		secondTri.point2U = 0;
		secondTri.point2V = 1;
		secondTri.point3U = 1;
		secondTri.point3V = 1;
		recursivelySplit(singlePatch, secondTri, 0);

	}

}




void generatePatchPoints(){

	int temp = ceil (1 / subdivision) + 1;

	for (int numPatch = 0; numPatch < numberOfPatches; numPatch++) {
		patch currentPatch = allPatches[numPatch];
		float u = 0;
		float v = 0;
		for (int i = 0; i < temp;  i++) {
			u = i * subdivision;
			u = min (u, 1.0f);
			for (int j = 0; j <  temp; j++) {
				v = j * subdivision;
				v = min (v, 1.0f);
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



	testName = argv[1];
	division = argv[2];
	subdivision = atof(division.c_str());
	flag = argv[3];
	if (flag == "-a") {
		adaptive = true;
	}

	float tempDivision = 1 / subdivision;
	numberOfSubdivisions = (int) tempDivision;

	loadScene(testName);



	if (adaptive) {
		cout << "adapative"  << endl;
		generateAdaptiveTesselation();
		cout << adaptiveTrianglesToRender.size() << endl;
	}else {
		generatePatchPoints();
	}

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	viewport.w = 600;
	viewport.h = 600;


	glutInitWindowSize(viewport.w, viewport.h);
	glutInitWindowPosition(0, 0);
	glutCreateWindow("CS184 Project 3");

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
