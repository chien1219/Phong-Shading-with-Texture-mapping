#include <stdio.h>			//C standard IO
#include <stdlib.h>			//C standard lib
#include <string.h>			//C string lib

#include <GL/glew.h>			//GLEW lib
#include <GL/glut.h>			//GLUT lib

int program;

//Function from: http://www.evl.uic.edu/aej/594/code/ogl.cpp
//Read in a textfile (GLSL program)
// we need to pass it as a string to the GLSL driver
char *textFileRead(char *fn)
{
	FILE *fp;
	char *content = NULL;

	int count = 0;

	if (fn != NULL) {

		fp = fopen(fn, "rt");

		if (fp != NULL) {

			fseek(fp, 0, SEEK_END);
			count = ftell(fp);
			rewind(fp);

			if (count > 0) {
				content = (char *)malloc(sizeof(char) * (count + 1));
				count = fread(content, sizeof(char), count, fp);
				content[count] = '\0';
			}
			fclose(fp);

		}
	}

	return content;
}

//Got this from http://www.lighthouse3d.com/opengl/glsl/index.php?oglinfo
// it prints out shader info (debugging!)
void printShaderInfoLog(GLuint obj)
{
	int infologLength = 0;
	int charsWritten = 0;
	char *infoLog;
	glGetShaderiv(obj, GL_INFO_LOG_LENGTH, &infologLength);
	if (infologLength > 0)
	{
		infoLog = (char *)malloc(infologLength);
		glGetShaderInfoLog(obj, infologLength, &charsWritten, infoLog);
		printf("printShaderInfoLog: %s\n", infoLog);
		free(infoLog);
	}
	else{
		printf("Shader Info Log: OK\n");
	}
}

//Got this from http://www.lighthouse3d.com/opengl/glsl/index.php?oglinfo
// it prints out shader info (debugging!)
void printProgramInfoLog(GLuint obj)
{
	int infologLength = 0;
	int charsWritten = 0;
	char *infoLog;
	glGetProgramiv(obj, GL_INFO_LOG_LENGTH, &infologLength);
	if (infologLength > 0)
	{
		infoLog = (char *)malloc(infologLength);
		glGetProgramInfoLog(obj, infologLength, &charsWritten, infoLog);
		printf("printProgramInfoLog: %s\n", infoLog);
		free(infoLog);
	}
	else{
		printf("Program Info Log: OK\n");
	}
}

int gw, gh;				//Keep track of window width and height

//GLUT callback fx
// called when window size changes
void changeSize(int w, int h) {
	//stores the width and height
	gw = w;
	gh = h;
}

void Light()
{
	GLfloat light_specular[] = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat light_diffuse[] = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat light_ambient[] = { 0.0, 0.0, 0.0, 1.0 };
	GLfloat light_position[] = { 10.0, 10.0, 5.0, 0.0 };

	GLfloat Ka[] = { 0.2, 0.2, 0.2 };
	GLfloat Kd[] = { 0.6, 0.6, 0.6 };
	GLfloat Ks[] = { 0.6, 0.6, 0.6 };
	GLfloat Ns = 20;

	glShadeModel(GL_SMOOTH);

	// enable lighting
	glEnable(GL_LIGHTING);
	// set light property
	glEnable(GL_LIGHT0);
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
	glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);

	glMaterialfv(GL_FRONT, GL_AMBIENT, Ka);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, Kd);
	glMaterialfv(GL_FRONT, GL_SPECULAR, Ks);
	glMaterialf(GL_FRONT, GL_SHININESS, Ns);
}

void renderScene(void)
{
	//Set the clear color (black) and clear depth
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glClearDepth(1.0f);
	//Clear the color buffer and depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	// z buffer test enable
	glEnable(GL_DEPTH_TEST);

	//stretch to screen
	glViewport(0, 0, gw, gh);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0, gw / gh, 1.0, 1000.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(0.0, 5.0, 20.0,     // eye
		0.0, 0.0, 0.0,     // center
		0.0, 1.0, 0.0);    // up
	Light();

	//Draw sphere's normal line
	glUseProgram(program);
	glutSolidSphere(3, 8, 8);

	//Draw object
	glUseProgram(0);
	glutSolidSphere(3, 8, 8);
	//swap buffers (double buffering)
	glutSwapBuffers();

}

//GLUT callback fx
// this is for processing key commands (setup a exit key)
void processNormalKeys(unsigned char key, int x, int y)
{
	if (key == 27)
		exit(0);
}


void setShaders()
{
	int vertShader, geomShader, fragShader;
	char *vertSource = NULL, *geomSource = NULL, *fragSource = NULL;

	//First, create our shaders 
	vertShader = glCreateShader(GL_VERTEX_SHADER);
	geomShader = glCreateShader(GL_GEOMETRY_SHADER);
	fragShader = glCreateShader(GL_FRAGMENT_SHADER);

	//Read in the programs
	vertSource = textFileRead("../Examples/VertexNormalVisualizer/VertexNormalVisualizer.vert");
	geomSource = textFileRead("../Examples/VertexNormalVisualizer/VertexNormalVisualizer.geom");
	fragSource = textFileRead("../Examples/VertexNormalVisualizer/VertexNormalVisualizer.frag");

	//Setup a few constant pointers for below
	const char *vv = vertSource;
	const char *gg = geomSource;
	const char *ff = fragSource;

	glShaderSource(vertShader, 1, &vv, NULL);
	glShaderSource(geomShader, 1, &gg, NULL);
	glShaderSource(fragShader, 1, &ff, NULL);

	free(vertSource);
	free(geomSource);
	free(fragSource);

	glCompileShader(vertShader);
	glCompileShader(geomShader);
	glCompileShader(fragShader);

	program = glCreateProgram();

	glAttachShader(program, vertShader);
	glAttachShader(program, geomShader);
	glAttachShader(program, fragShader);

	glLinkProgram(program);

	//You can use glUseProgram(program) whenever you want to render something with the program.
	//If you want to render with the fixed pipeline, use glUseProgram(0).

	printShaderInfoLog(vertShader);
	printShaderInfoLog(geomShader);
	printShaderInfoLog(fragShader);
	printProgramInfoLog(program);
}

int main(int argc, char **argv)
{

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(0, 0);
	glutInitWindowSize(800, 800);
	glutCreateWindow("Vertex Normal Visualizer");

	glutDisplayFunc(renderScene);
	glutReshapeFunc(changeSize);
	glutKeyboardFunc(processNormalKeys);

	glewInit();
	if (glewIsSupported("GL_VERSION_2_1"))
		printf("Ready for OpenGL 2.1\n");
	else {
		printf("OpenGL 2.1 not supported\n");
		exit(1);
	}
	if (GLEW_ARB_vertex_shader && GLEW_ARB_fragment_shader && GL_EXT_geometry_shader4)
		printf("Ready for GLSL - vertex, fragment, and geometry units\n");
	else {
		printf("Not totally ready :( \n");
		exit(1);
	}

	setShaders();

	glutMainLoop();

	// just for compatibiliy purposes
	return 0;

}
