#include "mesh.h"
#include "glew.h"
#include "glut.h"
#include "FreeImage.h"
#include <math.h>
#include <fstream>
#include <iostream>
using namespace std;

unsigned int textobject[100];
int num_of_object_to_text[100][5]; //第一項1: single texture 2: multi 3: cube 0: no 第二項:object數量   第三項: sc,mc  第四項:static第二項

GLhandleARB MyShader;
void LoadShaders();

int scene_obj_num = 0;
int how_many_light = 0;
int ptobject = 0;
mesh *object[100];
int windowSize[2];
////////////////scene declaration///////////////////
GLfloat scale[100][3];
GLfloat rotate_[100][4];   //rotate_[x][0] is Angle
GLfloat translate[100][3];
char singletextname[20][30]; ///single-texture filename 20
int singletex_num = 0;
int max_pos;
////////////////view declaration///////////////////
GLfloat eye[3], vat[3], vup[3], viewport[4];
GLfloat fovy, dnear, dfar;
////////////////light declaration///////////////////
GLfloat light_specular[7][4];
GLfloat light_diffuse[7][4];
GLfloat light_ambient[7][4];
GLfloat light_position[7][4];
GLfloat ambient[3];
////////////////for keyboard use///////////////////
GLfloat distancex;
GLfloat distancey;
GLfloat distancez;
////////////////for mouse use//////////////////////
GLfloat movex;
GLfloat movey;
GLfloat past_x, past_y, rec_x, rec_y;
GLfloat ctr_x[9], ctr_y[9];
////////////////for Shaders use//////////////////////
float hair_length;
int hair_segment;
float hair_gravity;
////////////////function predeclaration////////////
void light();
void handle_scene();
void handle_view();
void handle_light();
void display();
void LoadShaders();
void keyboard(unsigned char, int, int);
void mouse(int, int, int, int);
void mousemove(int, int);
void reshape(GLsizei, GLsizei);
void LoadTexture(char*, int, int);
void render_obj(int);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int hair;

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

	hair = glCreateProgram();

	glAttachShader(hair, vertShader);
	glAttachShader(hair, geomShader);
	glAttachShader(hair, fragShader);

	glLinkProgram(hair);

	printShaderInfoLog(vertShader);
	printShaderInfoLog(geomShader);
	printShaderInfoLog(fragShader);
	printProgramInfoLog(hair);

	//You can use glUseProgram(program) whenever you want to render something with the program.
	//If you want to render with the fixed pipeline, use glUseProgram(0).

}

bool ShaderLoad(GLhandleARB programId, char* shaderSrc, GLenum shaderType)
{
	FILE *fp;
	GLhandleARB h_shader;
	GLcharARB *shader_string;
	GLint str_length, maxLength;
	GLint isCompiled = GL_FALSE, isLinked = GL_FALSE;
	GLcharARB *pInfoLog;

	// open the file of shader source code
	if ((fp = fopen(shaderSrc, "r")) == NULL)
	{
		fprintf(stderr, "Error : Failed to read the OpenGL shader source \"%s\".\n", shaderSrc);
		return false;
	}

	// allocate memory for program string and load it.
	shader_string = (GLcharARB*)malloc(sizeof(GLcharARB) * 65536);
	str_length = (GLint)fread(shader_string, 1, 65536, fp);
	fclose(fp);

	// Create and load shader string.
	h_shader = glCreateShader(shaderType);
	if (h_shader == 0)
	{
		fprintf(stderr, "Error : Failed to create OpenGL shader object \"%s\".\n", shaderSrc);
		return false;
	}
	glShaderSource(h_shader, 1, (const GLcharARB**)&shader_string, &str_length);
	free(shader_string);

	// Compile the vertex shader, print out the compiler log message.
	glCompileShader(h_shader);

	// get compile state information
	glGetObjectParameterivARB(h_shader, GL_OBJECT_COMPILE_STATUS_ARB, &isCompiled);

	if (!isCompiled)
	{
		fprintf(stderr, "Error : Failed to compile OpenGL shader source \"%s\".\n", shaderSrc);
		glGetObjectParameterivARB(h_shader, GL_OBJECT_INFO_LOG_LENGTH_ARB, &maxLength);
		pInfoLog = (GLcharARB *)malloc(maxLength * sizeof(GLcharARB));
		glGetInfoLogARB(h_shader, maxLength, &str_length, pInfoLog);
		fprintf(stderr, "%s\n", pInfoLog);
		free(pInfoLog);
		return false;
	}
	glAttachShader(programId, h_shader);

	// delete the shader object, since we have attached it with the program object.
	glDeleteShader(h_shader);

	// Link the program and print out the linker log message
	glLinkProgram(programId);
	glGetObjectParameterivARB(programId, GL_OBJECT_LINK_STATUS_ARB, &isLinked);

	if (!isLinked)
	{
		fprintf(stderr, "Error : Failed to link OpenGL shader \"%s\".\n", shaderSrc);
		glGetObjectParameterivARB(programId, GL_OBJECT_INFO_LOG_LENGTH_ARB, &maxLength);
		pInfoLog = (GLcharARB *)malloc(maxLength * sizeof(GLcharARB));
		glGetInfoLogARB(programId, maxLength, &str_length, pInfoLog);
		fprintf(stderr, "%s\n", pInfoLog);
		free(pInfoLog);
		return false;
	}

	return true;
}

void LoadShaders()
{
	MyShader = glCreateProgram();
	if (MyShader != 0)
	{
		ShaderLoad(MyShader, "../Examples/PhongShading/PhongShading.vert", GL_VERTEX_SHADER);
		ShaderLoad(MyShader, "../Examples/PhongShading/PhongShading.frag", GL_FRAGMENT_SHADER);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////






int main(int argc, char** argv)
{
	hair_length = 1;
	hair_segment = 10;
	hair_gravity = -0.1f;
	handle_scene();
	handle_view();
	handle_light();
	glutInit(&argc, argv);

	glutInitWindowSize(viewport[2], viewport[3]);
	glutInitWindowPosition(250, 100);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutCreateWindow("Hair Salon");
	glewInit();
	FreeImage_Initialise();

	glGenTextures(1, &textobject[0]);
	LoadTexture(singletextname[0], 0, 1);

	FreeImage_DeInitialise();

	setShaders();/////////vertex normal shading
	LoadShaders();////////phong shading

	distancex = vat[0] - eye[0];
	distancey = vat[1] - eye[1];
	distancez = vat[2] - eye[2];
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyboard);
	glutMouseFunc(mouse);
	glutMotionFunc(mousemove);

	glutMainLoop();

	return 0;
}

void LoadTexture(char* filename, int k, int textnum){
	FIBITMAP* pimage = FreeImage_Load(FreeImage_GetFileType(filename, 0), filename);
	FIBITMAP* p32bitsImage = FreeImage_ConvertTo32Bits(pimage);
	int iwidth = FreeImage_GetWidth(p32bitsImage);
	int iheight = FreeImage_GetHeight(p32bitsImage);

	glBindTexture(GL_TEXTURE_2D, textobject[k]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, iwidth, iheight, 0, GL_BGRA, GL_UNSIGNED_BYTE, (void*)FreeImage_GetBits(p32bitsImage));
	glGenerateMipmap(GL_TEXTURE_2D);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	FreeImage_Unload(p32bitsImage);
	FreeImage_Unload(pimage);
}

void display()
{
	// clear the buffer
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);      //清除用color
	glClearDepth(1.0f);                        // Depth Buffer (就是z buffer) Setup
	glEnable(GL_DEPTH_TEST);                   // Enables Depth Testing
	glDepthFunc(GL_LEQUAL);                    // The Type Of Depth Test To Do
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);//這行把畫面清成黑色並且清除z buffer
	// viewport transformation
	//-----------------------------------------------------------------------

	glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);

	// projection transformation
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(fovy, viewport[2] / viewport[3], dnear, dfar);
	// viewing and modeling transformation
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	cout << "X: " << -distancex << " y: " << -distancey << " Z: " << -distancez << endl;
	gluLookAt(eye[0], eye[1], eye[2],		// eye
		vat[0], vat[1], vat[2],     // center
		vup[0], vup[1], vup[2]);    // up
	//-----------------------------------------------------------------------
	light();

	//Draw object

	glUseProgram(MyShader);
	glUniform1i(glGetUniformLocation(MyShader, "colorTexture"), 0);
	glUniform1i(glGetUniformLocation(MyShader, "lightnumber"), how_many_light);
	render_obj(1); //////畫臉

	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//Draw sphere's normal line
	glUseProgram(hair);
	cout << "Length: " << hair_length << endl;
	glUniform1f(glGetUniformLocation(hair, "length"), hair_length);
	cout << "Segment: " << hair_segment << endl;
	glUniform1i(glGetUniformLocation(hair, "segment"), hair_segment);
	cout << "Gravity: " << hair_gravity << endl;
	glUniform3f(glGetUniformLocation(hair, "gravity"), 0.0f, hair_gravity, 0.0f);

	glUniform4f(glGetUniformLocation(hair, "color_in"), 0.0f ,0.0f ,0.0f ,1.0f);
	glDepthMask(GL_FALSE);
	render_obj(0);  /////畫頭髮
	glDepthMask(GL_TRUE);


	glFlush();
	glutSwapBuffers();
}

void render_obj(int not_to_draw){

	int text = 0;

	for (int gg = 0; gg < scene_obj_num; ++gg){

		if (gg == not_to_draw) continue;
		if (gg == 0) text = 1;

		int lastMaterial = -1;
		glPushMatrix();
		glTranslatef(translate[gg][0] - ctr_x[gg], translate[gg][1] + ctr_y[gg], translate[gg][2]);
		glRotatef(rotate_[gg][0], rotate_[gg][1], rotate_[gg][2], rotate_[gg][3]);
		glScalef(scale[gg][0], scale[gg][1], scale[gg][2]);
		for (size_t i = 0; i < object[gg]->fTotal; ++i)
		{
			// set material property if this face used different material
			if (lastMaterial != object[gg]->faceList[i].m)
			{
				lastMaterial = (int)object[gg]->faceList[i].m;
				glMaterialfv(GL_FRONT, GL_AMBIENT, object[gg]->mList[lastMaterial].Ka);
				glMaterialfv(GL_FRONT, GL_DIFFUSE, object[gg]->mList[lastMaterial].Kd);
				glMaterialfv(GL_FRONT, GL_SPECULAR, object[gg]->mList[lastMaterial].Ks);
				glMaterialfv(GL_FRONT, GL_SHININESS, &object[gg]->mList[lastMaterial].Ns);

				if (text == 1){
					glActiveTexture(GL_TEXTURE0);
					glEnable(GL_TEXTURE_2D);
					glBindTexture(GL_TEXTURE_2D, textobject[0]);
					glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE); /////////////////////////////////
				}
			}
			glBegin(GL_TRIANGLES);
			for (size_t j = 0; j < 3; ++j){
				if (text == 0){
					//textex corrd. object[gg]->tList[object[gg]->faceList[i][j].t].ptr
					glNormal3fv(object[gg]->nList[object[gg]->faceList[i][j].n].ptr);
					glVertex3fv(object[gg]->vList[object[gg]->faceList[i][j].v].ptr);
				}
				else if (text == 1){

					//textex corrd. object[gg]->tList[object[gg]->faceList[i][j].t].ptr
					glTexCoord2fv(object[gg]->tList[object[gg]->faceList[i][j].t].ptr);
					glNormal3fv(object[gg]->nList[object[gg]->faceList[i][j].n].ptr);
					glVertex3fv(object[gg]->vList[object[gg]->faceList[i][j].v].ptr);
				}
			}
			glEnd();
		}

		if (text == 1){
			glActiveTexture(GL_TEXTURE0);
			glDisable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, 0);
		}
		glPopMatrix();
	}
}


void keyboard(unsigned char key, int x, int y){

	switch (key) {
	case 'w':			///////zoom in
	case 'W':
		eye[0] = -distancex + (distancex / 80) + vat[0];
		eye[1] = -distancey + (distancey / 80) + vat[1];
		eye[2] = -distancez + (distancez / 80) + vat[2];
		distancex = vat[0] - eye[0];
		distancey = vat[1] - eye[1];
		distancez = vat[2] - eye[2];
		glutPostRedisplay();
		break;
	case 'a':
	case 'A':
		eye[0] = -distancex*cos(0.055) - (-distancez)*sin(0.055);
		eye[2] = -distancez*cos(0.055) + (-distancex)*sin(0.055);
		distancex = -eye[0];
		distancez = -eye[2];
		glutPostRedisplay();
		break;
	case 's':			///////zoom out
	case 'S':
		eye[0] = -distancex - (distancex / 80) + vat[0];
		eye[1] = -distancey - (distancey / 80) + vat[1];
		eye[2] = -distancez - (distancez / 80) + vat[2];
		distancex = vat[0] - eye[0];
		distancey = vat[1] - eye[1];
		distancez = vat[2] - eye[2];
		glutPostRedisplay();
		break;
	case 'd':
	case 'D':
		eye[0] = -distancex*cos(0.055) + (-distancez)*sin(0.055);
		eye[2] = -distancez*cos(0.055) - (-distancex)*sin(0.055);
		distancex = -eye[0];
		distancez = -eye[2];
		glutPostRedisplay();
		break;
	case 'r':
	case 'R':
		hair_length += 0.1f;
		glutPostRedisplay();
		break;
	case 'f':
	case 'F':
		if (hair_length == 0);
		else{
			hair_length -= 0.1f;
			glutPostRedisplay();
		}
		break;
	case 't':
	case 'T':
		hair_segment += 1;
		glutPostRedisplay();
		break;
	case 'g':
	case 'G':
		if (hair_segment == 0);
		else{
			hair_segment -= 1;
			glutPostRedisplay();
		}
		break;
	case 'y':
	case 'Y':
		hair_gravity += 0.1f;
		glutPostRedisplay();
		break;
	case 'h':
	case 'H':
		hair_gravity -= 0.1f;
		glutPostRedisplay();
		break;
	case '1':
		ptobject = 0;
		glutPostRedisplay();
		break;
	case '2':
		ptobject = 1;
		glutPostRedisplay();
		break;
	case '3':
		ptobject = 2;
		glutPostRedisplay();
		break;
	case '4':
		ptobject = 3;
		glutPostRedisplay();
		break;
	case '5':
		ptobject = 4;
		glutPostRedisplay();
		break;
	case '6':
		ptobject = 5;
		glutPostRedisplay();
		break;
	case '7':
		ptobject = 6;
		glutPostRedisplay();
		break;
	case '8':
		ptobject = 7;
		glutPostRedisplay();
		break;
	case '9':
		ptobject = 8;
		glutPostRedisplay();
		break;

	}

}
void mouse(int button, int state, int x, int y){
	if (state){
		rec_x += (x - past_x);
		rec_y += (y - past_y);
	}
	else{
		past_x = x;
		past_y = y;
	}
}
void mousemove(int x, int y){
	ctr_x[ptobject] = (GLfloat)(past_x - x) / 20;
	ctr_y[ptobject] = (GLfloat)(past_y - y) / 20;

	cout << "mouse_x: " << ctr_x[ptobject] << " mouse_y: " << ctr_y[ptobject] << endl;
	glutPostRedisplay();

}
void light(){
	for (int i = 0; i < how_many_light; i++){
		glShadeModel(GL_SMOOTH);
		// z buffer enable
		glEnable(GL_DEPTH_TEST);
		// enable lighting
		glEnable(GL_LIGHTING);
		// set light property
		glEnable(GL_LIGHT0 + (GLfloat)i);
		glLightfv(GL_LIGHT0 + (GLfloat)i, GL_POSITION, light_position[i]);
		glLightfv(GL_LIGHT0 + (GLfloat)i, GL_DIFFUSE, light_diffuse[i]);
		glLightfv(GL_LIGHT0 + (GLfloat)i, GL_SPECULAR, light_specular[i]);
		glLightfv(GL_LIGHT0 + (GLfloat)i, GL_AMBIENT, light_ambient[i]);
		cout << "--------------" << "No. " << i + 1 << " light is created-----------------" << endl;
	}
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient);
}
void handle_scene(){
	int cnt = 0;
	int pos = 0;
	fstream scene_;
	scene_.open("Peter.scene", ios::in);
	char title[30];
	while (scene_ >> title){
		if (!strcmp(title, "model")){
			cnt++;			///
			char name[20];
			scene_ >> name;
			object[scene_obj_num] = new mesh(name);
			for (int i = 0; i < 3; i++){
				scene_ >> (GLfloat)scale[scene_obj_num][i];
			}
			for (int i = 0; i < 4; i++){
				scene_ >> (GLfloat)rotate_[scene_obj_num][i];
			}
			for (int i = 0; i < 3; i++){
				scene_ >> (GLfloat)translate[scene_obj_num][i];
			}
			cout << "in scene " << scene_obj_num << " obj: " << name << " created." << endl;
			scene_obj_num++;
		}
		else if (!strcmp(title, "single-texture")){
			scene_ >> singletextname[singletex_num];
			singletex_num++;
			if (cnt == 0 && pos == 0)
				num_of_object_to_text[pos][1] = 1;
			if (cnt != 0){
				num_of_object_to_text[pos][2] = cnt;
				pos++;
				num_of_object_to_text[pos][1] = 1;
				num_of_object_to_text[pos][3] = singletex_num - 1;
				cnt = 0;
			}
		}

		else if (!strcmp(title, "no-texture")){
			if (cnt == 0 && pos == 0)
				num_of_object_to_text[pos][1] = 0;
			if (cnt != 0){
				num_of_object_to_text[pos][2] = cnt;
				pos++;
				num_of_object_to_text[pos][1] = 0;
				cnt = 0;
			}
		}
		else;
	}
	num_of_object_to_text[pos][2] = cnt;
	pos++;
	for (int i = 0; i < pos; i++){
		//cout << num_of_object_to_text[i][1] << " " << num_of_object_to_text[i][2] << " " << num_of_object_to_text[i][3] <<endl;
		num_of_object_to_text[i][4] = num_of_object_to_text[i][2];
	}
	max_pos = pos;
	cout << "-----------------scene development done----------------------" << endl;
	scene_.close();
}
void handle_view(){

	char cat[10];

	fstream view_;
	view_.open("Peter.view", ios::in);
	while (view_ >> cat){
		if (!strcmp(cat, "eye")){
			for (int i = 0; i < 3; i++){
				view_ >> (GLfloat)eye[i];
			}
		}
		else if (!strcmp(cat, "vat")){
			for (int i = 0; i < 3; i++){
				view_ >> (GLfloat)vat[i];
			}
		}
		else if (!strcmp(cat, "vup")){
			for (int i = 0; i < 3; i++){
				view_ >> (GLfloat)vup[i];
			}
		}
		else if (!strcmp(cat, "fovy")){
			view_ >> (GLfloat)fovy;
		}
		else if (!strcmp(cat, "dnear")){
			view_ >> (GLfloat)dnear;
		}

		else if (!strcmp(cat, "dfar")){
			view_ >> (GLfloat)dfar;
		}
		else if (!strcmp(cat, "viewport")){
			for (int i = 0; i < 4; i++){
				view_ >> (GLfloat)viewport[i];
			}
		}
	}
	cout << "-----------------view development done-----------------------" << endl;
	view_.close();
}
void handle_light(){
	char buffer[10];
	fstream light_;
	light_.open("Peter.light", ios::in);
	while (light_ >> buffer){
		if (!strcmp(buffer, "light")){
			for (int i = 0; i < 3; i++){
				light_ >> (GLfloat)light_position[how_many_light][i];
			}light_position[how_many_light][3] = 1;
			for (int i = 0; i < 3; i++){
				light_ >> (GLfloat)light_ambient[how_many_light][i];
			}light_ambient[how_many_light][3] = 1;
			for (int i = 0; i < 3; i++){
				light_ >> (GLfloat)light_diffuse[how_many_light][i];
			}light_diffuse[how_many_light][3] = 1;
			for (int i = 0; i < 3; i++){
				light_ >> (GLfloat)light_specular[how_many_light][i];
			}light_specular[how_many_light][3] = 1;
			how_many_light++;
		}
		else if (!strcmp(buffer, "ambient")){
			for (int i = 0; i < 3; i++){
				light_ >> (GLfloat)light_ambient[how_many_light][i];
			}
		}
	}
	cout << "-----------------light development done----------------------" << endl;
	light_.close();

}
void reshape(GLsizei w, GLsizei h)
{
	windowSize[0] = w;
	windowSize[1] = h;
}