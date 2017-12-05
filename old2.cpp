#ifdef __APPLE__
#include <GLUT/glut.h> 
#else
#include <GL/glut.h> 
#endif

#include <iostream>

#include <stdlib.h>
#include <stddef.h>
#include <unistd.h>

#include "create_and_compile_shaders.h"

// properties of some material
float mat_ambient[] = {1.0, 0.0, 0.0, 1.0};
float mat_diffuse[] = {0.75, 0.75, 0.75, 1.0};
float mat_specular[] = {0.1, 0.1, 0.1, 0.1};
float mat_shininess[] = {50.0};

float t_l[2] = {-0.4f, 0.9f};
float t_r[2] = {0.4f, 0.9f};
float b_l[2] = {-0.4f, -0.7f};
float b_r[2] = {0.4f, -0.7f};

float height = t_l[1] - b_l[1];
float width = t_r[0] - t_l[0];

float cube_size = height / 20.0f;

float eye_x = 0.0f;
float eye_y = 0.0f;

int arena[10][20];

unsigned int shader = 0;

// Define some cube bits
float cube_vertices[8][3] = {
		{0.0f, 0.0f, -(cube_size / 2.0f)},
		{cube_size, 0.0f, -(cube_size / 2.0f)},
		{cube_size, cube_size, -(cube_size / 2.0f)},
		{0.0f, cube_size, -(cube_size / 2.0f)},
		{0.0f, 0.0f, (cube_size / 2.0f)}, 
		{cube_size, 0.0f, (cube_size / 2.0f)},
		{cube_size, cube_size, (cube_size / 2.0f)},
		{0.0f, cube_size, (cube_size / 2.0f)}
};

// indices into verices
size_t cube_faces[6][4] = {
		{0, 1, 2, 3},
		{5, 4, 7, 6},
		{4, 0, 3, 7},
		{1, 5, 6, 2},
		{4, 5, 1, 0},
		{3, 2, 6, 7} 
};

int g_spin = 0;
bool g_spinning = false;

void idle()
{
	usleep(300000);
	g_spin += 5;
	glutPostRedisplay();
}

void display()
{
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT); 
	
	// position and orient camera
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(eye_x, eye_y, 3, // eye position
			  0, 0, 0, // reference point
			  0, 1, 0  // up vector
		);

	// add in transparent geometry in front of teapot
	glDisable(GL_LIGHTING);

	glColor4f(0.0f, 1.0f, 0.0f, 0.25f);
	glLineWidth(0.1f);
	glBegin(GL_QUADS);
		glVertex3f(t_l[0], t_l[1], 0.05f);
		glVertex3f(b_l[0], b_l[1], 0.05f);
		glVertex3f(b_l[0], b_l[1], -0.05f);
		glVertex3f(t_l[0], t_l[1], -0.05f);
		
		glVertex3f(b_l[0], b_l[1], 0.05f);
		glVertex3f(b_r[0], b_r[1], 0.05f);
		glVertex3f(b_r[0], b_r[1], -0.05f);
		glVertex3f(b_l[0], b_l[1], -0.05f);
	
		glVertex3f(b_r[0], b_r[1], 0.05f);
		glVertex3f(t_r[0], t_r[1], 0.05f);
		glVertex3f(t_r[0], t_r[1], -0.05f);
		glVertex3f(b_r[0], b_r[1], -0.05f);
	glEnd();
	
	glBegin(GL_LINES);
		glColor4f(1.0f, 1.0f, 1.0f, 0.25f);

		for(int i = 1; i < 10; i++) {
			glVertex3f(b_l[0] + (i * (width / 10)), b_l[1], 0.0f);
			glVertex3f(b_l[0] + (i * (width / 10)), t_l[1], 0.0f);
		}
		for(int i = 1; i < 20; i++) {
			glVertex3f(b_l[0], b_l[1] + (i * (height / 20)), 0.0f);
			glVertex3f(b_r[0], b_l[1] + (i * (height / 20)), 0.0f);
		}
	glEnd();
	glEnable(GL_LIGHTING);
	
	glColor4f(1.0f, 0.0f, 0.0f, 1.0f);

	glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
	glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
	
	glPushMatrix();
		glTranslatef(cube_size / 2, cube_size / 2, 0);
		glTranslatef(b_l[0], b_l[1], 0.0f);
		glutSolidCube(cube_size);
		glPushMatrix();
			glTranslatef(cube_size, 0.0f, 0.0f);
			glutSolidCube(cube_size);
		glPopMatrix();
	glPopMatrix();
	glutSwapBuffers(); 
}

void keyboard(unsigned char key, int, int)
{
	switch (key)
	{
		case 'q': exit(1); break;

		case ' ': g_spinning = !g_spinning;
				if (g_spinning)
					glutIdleFunc(idle);
				else
					glutIdleFunc(NULL);
				break;
		case 'w': eye_y += 0.1f; break;
		case 's': eye_y -= 0.1f; break;
		case 'd': eye_x += 0.1f; break;
		case 'a': eye_x -= 0.1f; break;
	}
	glutPostRedisplay();
}

void reshape(int w, int h)
{
	glViewport(0, 0, w, h); 
	glMatrixMode(GL_PROJECTION); 
	glLoadIdentity();
	gluPerspective(40.0, 1.0f, 1.0, 5.0);

	glutPostRedisplay();
}

void init()
{
	float light_ambient[] = {0.1, 0.1, 0.1, 1.0};
	float light_diffuse[] = {0.5, 0.5, 0.5, 1.0};
	glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);

	float light_position[] = {0.0, 1.0, 2.0, 0.0};
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	glEnable(GL_DEPTH_TEST);
	glShadeModel(GL_SMOOTH); 

	// turn on blending and set a blending function
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	if (argc==3)
		g_program_obj = create_and_compile_shaders(argv[1], NULL, argv[2]);
	else
	if (argc==4)
		g_program_obj = create_and_compile_shaders(argv[1], argv[2], argv[3]);

	if (g_program_obj) {
		set_geometry_shader_params(g_program_obj, GL_LINES, GL_LINE_STRIP, 4);
	}
}

int main(int argc, char* argv[])
{
	glutInit(&argc, argv); 
	
	glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGBA|GLUT_DEPTH); 

	glutInitWindowSize(512, 512); 
	glutInitWindowPosition(50, 50); 

	glutCreateWindow("Blending"); 

	glutKeyboardFunc(keyboard); 
	glutReshapeFunc(reshape); 
	glutDisplayFunc(display); 

	init(); 

	glutMainLoop(); 

	return 0; 
}
