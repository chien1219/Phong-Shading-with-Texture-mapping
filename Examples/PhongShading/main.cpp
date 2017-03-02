#include "../SharedCode/ShaderLoader.h"
#include "../SharedCode/mesh.h"
#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>
#include <GL/glut.h>

mesh *object;

GLhandleARB MyShader;

int WinW,WinH;

void LoadShaders();
void Light();
void Display();
void Reshape(GLsizei , GLsizei );

int main(int argc, char** argv)
{
	object = new mesh("sphere.obj");

	glutInit(&argc, argv);
	glutInitWindowSize(800, 800);
	glutInitWindowPosition(0, 0);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutCreateWindow("Phong Shading");

	GLenum glew_error;
	if((glew_error = glewInit()) != GLEW_OK)return -1;

	LoadShaders();

	glutDisplayFunc(Display);
	glutReshapeFunc(Reshape);
	glutMainLoop();

	return 0;
}

void LoadShaders()
{
	MyShader = glCreateProgram();
	if(MyShader != 0)
	{
		ShaderLoad(MyShader, "../Examples/PhongShading/PhongShading.vert", GL_VERTEX_SHADER);
		ShaderLoad(MyShader, "../Examples/PhongShading/PhongShading.frag", GL_FRAGMENT_SHADER);
	}
}

void Light()
{
	GLfloat light_specular[] = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat light_diffuse[] = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat light_ambient[] = { 0.0, 0.0, 0.0, 1.0 };
	GLfloat light_position[] = { 20.0, 10.0, 5.0, 0.0 };

	GLfloat Ka[] = { 0.2, 0.2, 0.2 };
	GLfloat Kd[] = { 0.6, 0.6, 0.6 };
	GLfloat Ks[] = { 0.6, 0.6, 0.6 };
	GLfloat Ns = 20;

	glShadeModel(GL_SMOOTH);

	// z buffer enable
	glEnable(GL_DEPTH_TEST);

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

void Display()
{
	// projection transformation
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0, (GLfloat)WinW/(GLfloat)WinH, 1.0, 1000.0);
	// viewing and modeling transformation
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(0.0, 0.0, 10.0,     // eye
			  0.0, 0.0, 0.0,     // center
			  0.0, 1.0, 0.0);    // up

	Light();
	// clear the buffer
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);      //清除用color
	glClearDepth(1.0f);                        // Depth Buffer (就是z buffer) Setup
	glEnable(GL_DEPTH_TEST);                   // Enables Depth Testing
	glDepthFunc(GL_LEQUAL);                    // The Type Of Depth Test To Do
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);//這行把畫面清成黑色並且清除z buffer

	glUseProgram(MyShader);

	for (size_t i=0;i < object->fTotal;i++)
	{
		glBegin(GL_POLYGON);
		for (size_t j=0;j<3;j++)
		{
			glNormal3fv(object->nList[object->faceList[i][j].n].ptr);
			glVertex3fv(object->vList[object->faceList[i][j].v].ptr);	
		}
		glEnd();
	}

	glutSwapBuffers();
	glutPostRedisplay();
}

void Reshape(GLsizei w, GLsizei h)
{
	WinW = w;
	WinH = h;
	// viewport transformation
	glViewport(0, 0, WinW,WinH);
}

