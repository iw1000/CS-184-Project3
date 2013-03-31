
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
	float xValue;
	float yValue;
	float zValue;
};

struct patch
{
	vector <controlPoint> allControlPoints;
};

vector <patch> allPatches;

int numberOfPatches = 0;
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


void initScene(){

}


//****************************************************
// reshape viewport if the window is resized
//****************************************************
void myReshape(int w, int h) {
	viewport.w = w;
	viewport.h = h;

	glViewport (0,0,viewport.w,viewport.h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0, viewport.w, 0, viewport.h);

}


//****************************************************
// A routine to set a pixel by drawing a GL point.  This is not a
// general purpose routine as it assumes a lot of stuff specific to
// this example.
//****************************************************

//****************************************************
// Draw a filled circle.  
//****************************************************


void circle(float centerX, float centerY, float radius) {
	// Draw inner circle
	glBegin(GL_POINTS);

	int iStep,jStep;  // Pixel indices
	int minI = max(0,(int)floor(centerX-radius));
	int maxI = min(viewport.w-1,(int)ceil(centerX+radius));
	glEnd();

}
//****************************************************
// function that does the actual drawing of stuff
//***************************************************
void myDisplay() {
	glClear(GL_COLOR_BUFFER_BIT);				// clear the color buffer
	glMatrixMode(GL_MODELVIEW);			        // indicate we are specifying camera transformations
	glLoadIdentity();				        // make sure transformation is "zero'd"
	// Start drawing
	circle(viewport.w / 2.0 , viewport.h / 2.0 , min(viewport.w, viewport.h) / 3.0);
	glFlush();
	glutSwapBuffers();					// swap buffers (we earlier set double buffer)
}



void printPoint(controlPoint p) {
	cout << "Xvalue = " << p.xValue <<  "     Yvalue = " << p.yValue <<  "     Zvalue = " << p.zValue << endl;
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
				singleControl.xValue=x;
				singleControl.yValue=y;
				singleControl.zValue=z;
				tempControlPoints.push_back(singleControl);

				x = atof(splitline[3].c_str());
				y = atof(splitline[4].c_str());
				z = atof(splitline[5].c_str());

				controlPoint singleControl1 = {};
				singleControl1.xValue=x;
				singleControl1.yValue=y;
				singleControl1.zValue=z;
				tempControlPoints.push_back(singleControl1);

				x = atof(splitline[6].c_str());
				y = atof(splitline[7].c_str());
				z = atof(splitline[8].c_str());

				controlPoint singleControl2 = {};
				singleControl2.xValue=x;
				singleControl2.yValue=y;
				singleControl2.zValue=z;
				tempControlPoints.push_back(singleControl2);

				x = atof(splitline[9].c_str());
				y = atof(splitline[10].c_str());
				z = atof(splitline[11].c_str());

				controlPoint singleControl3 = {};
				singleControl3.xValue=x;
				singleControl3.yValue=y;
				singleControl3.zValue=z;
				tempControlPoints.push_back(singleControl3);
			}

			count++;
		}
	}

}


int main(int argc, char *argv[]) {   // first argument is the program running
	//This initializes glut
	glutInit(&argc, argv);

	//string testName = "test.bez";//argv[1];

	string testName = argv[1];

	loadScene(testName);

	string division = argv[2];
	subdivision = atof(division.c_str());

	string flag = argv[3];
	if (flag == "-a") {
		adaptive = true;
	}


	cout << adaptive << endl;
	cout << subdivision << endl;

	//printPatch(allPatches[1]);


	/*

	//This tells glut to use a double-buffered window with red, green, and blue channels 
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);

	// Initalize theviewport size
	viewport.w = 400;
	viewport.h = 400;

	//The size and position of the window
	glutInitWindowSize(viewport.w, viewport.h);
	glutInitWindowPosition(0,0);
	glutCreateWindow(argv[0]);
	glutKeyboardFunc(keyPressed);


	initScene();							// quick function to set up scene

	glutDisplayFunc(myDisplay);				// function to run when its time to draw something
	glutReshapeFunc(myReshape);				// function to run when the window gets resized

	glutMainLoop();							// infinite loop that will keep drawing and resizing
	// and whatever else

	 */

	return 0;
}