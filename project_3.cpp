
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


struct lightSource
{
	float xValue;
	float yValue;
	float zValue;
	float redValue;
	float greenValue;
	float blueValue;
};


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

int numberOfPatches = 0;
int numberOfSubdivisions = 0;
float subdivision = 0;
bool adaptive = false; 

//****************************************************
// Some Classes
//****************************************************

class Viewport;

class Viewport {
public:
		int w, h; // width and height
	};

	Viewport	viewport;


	void myReshape(int w, int h) {
		viewport.w = w;
		viewport.h = h;
  glViewport(0,0,viewport.w,viewport.h);// sets the rectangle that will be the window
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();                // loading the identity matrix for the screen
  glOrtho(-1, 1, -1, 1, 1, -1);    // resize type = stretch
}

void initScene(){
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f); // Clear to black, fully transparent
  myReshape(viewport.w,viewport.h);
}


void myDisplay() {

  glClear(GL_COLOR_BUFFER_BIT);                // clear the color buffer (sets everything to black)
  glMatrixMode(GL_MODELVIEW);                  // indicate we are specifying camera transformations
  glLoadIdentity();                            // make sure transformation is "zero'd"

  //----------------------- code to draw objects --------------------------
  // Rectangle Code
  //glColor3f(red component, green component, blue component);
  glColor3f(1.0f,0.0f,0.0f);                   // setting the color to pure red 90% for the rect

  //glBegin(GL_POLYGON);   

  glBegin(GL_TRIANGLES); 
  //glVertex3f(x val, y val, z val (won't change the point because of the projection type));
  glVertex3f(-0.8f, 0.0f, 0.0f);               // bottom left corner of rectangle
  glVertex3f(-0.8f, 0.5f, 0.0f);               // top left corner of rectangle
  glVertex3f( 0.0f, 0.5f, 0.0f);               // top right corner of rectangle
 // glVertex3f( 0.0f, 0.0f, 0.0f);               // bottom right corner of rectangle
  glEnd();

  glFlush();
  glutSwapBuffers();                           // swap buffers (we earlier set double buffer)
}



void printPoint(controlPoint p) {
	cout << "Xvalue = " << p.pixelLocation[0] <<  "     Yvalue = " << p.pixelLocation[0] <<  "     Zvalue = " << p.pixelLocation[0] << endl;
}

void printPatch(patch p) {
	for (int i = 0; i < p.allControlPoints.size(); i++ ) {
		printPoint(p.allControlPoints[i]);
	}
}


void keyPressed (unsigned char key, int x, int y) {  
	if (key == ' ') {
		exit(0);
	}
}

void loadScene(string file) {
	//store variables and set stuff at the end
	//std::string fname = "output.png";
	cout << file << endl;
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

				singlePatch.row = new drawPoint* [numberOfSubdivisions];
				for(int i = 0; i < numberOfSubdivisions; i++){
    			
    				singlePatch.row[i] = new drawPoint[numberOfSubdivisions];
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

	/// Decastou

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
	toReturn.pixelNormal = cross;
	return toReturn;



}


void generatePatchPoints(){

	patch currentPatch = allPatches[0];

	float u = 0;

	float v = 0;

	for (int i = 0; i < numberOfSubdivisions; i++) {

		u = i * subdivision;

		for (int j = 0; j < numberOfSubdivisions; j++) {

			v = j * subdivision;

			currentPatch.row[i][j] = generatePlanarCurve (currentPatch,u,v);


		
		}

	}


}

int main(int argc, char *argv[]) {   // first argument is the program running
	//This initializes glut
	glutInit(&argc, argv);

	//string testName = "test.bez";//argv[1];

	string testName = argv[1];


	string division = argv[2];
	subdivision = atof(division.c_str());

	string flag = argv[3];
	if (flag == "-a") {
		adaptive = true;
	}


	cout << adaptive << endl;
	cout << subdivision << endl;

	float tempDivision = 1 / subdivision;

	numberOfSubdivisions = (int) tempDivision;

	cout << numberOfSubdivisions << endl;


	loadScene(testName);


	//printPatch(allPatches[1]);

	generatePatchPoints();



  //This tells glut to use a double-buffered window with red, green, and blue channels 
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);

  // Initalize theviewport size
	viewport.w = 400;
	viewport.h = 400;

  //The size and position of the window
	glutInitWindowSize(viewport.w, viewport.h);
	glutInitWindowPosition(0, 0);
	glutCreateWindow("CS184!");

 	initScene();                                 // quick function to set up scene

  glutDisplayFunc(myDisplay);                  // function to run when its time to draw something
  glutReshapeFunc(myReshape);                  // function to run when the window gets resized
  glutIdleFunc(myFrameMove);                   // function to run when not handling any other task
  glutMainLoop();                              // infinite loop that will keep drawing and resizing and whatever else



  return 0;
}