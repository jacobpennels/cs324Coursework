#ifdef __APPLE__
#include <GLUT/glut.h> 
#else
#include <GL/glew.h> 
#include <GL/glut.h> 
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <unistd.h>
#include <time.h>

#include "draw_text.h"
#include "lights_material.h"
#include "create_and_compile_shaders.h"
//#include "tetris_control.h"

bool g_spinning = false;
int g_angle = 0;

float t_l[2] = {-0.4f, 0.9f};
float t_r[2] = {0.4f, 0.9f};
float b_l[2] = {-0.4f, -0.7f};
float b_r[2] = {0.4f, -0.7f};

float red[4] = {1.0f, 0.0f, 0.0f, 0.75f};
float green[4] = {0.0f, 1.0f, 0.0f, 0.75f};
float blue[4] = {0.0f, 0.0f, 1.0f, 0.75f};

float height = t_l[1] - b_l[1];
float width = t_r[0] - t_l[0];
const int arena_h = 22;
const int arena_w = 10;
int arena[arena_w][arena_h] = { 0 }; // [x][y] 10x20 grid with 2 on top for new pieces to spawn in
const int spawn_point[2] = {4, 20};

float cube_size = height / 20.0f;

float eye_x = 0.05f;
float eye_y = 0.05f;
// define block shapes, based on point of rotation as origin (rotations are clockwise)
const int t_block[4][4][2] = {
	{{0,0}, {1,0}, {-1,0}, {0,1}},
	{{0,0}, {1,0}, {0,1}, {0,-1}},
	{{0,0}, {1,0}, {-1,0}, {0,-1}},
	{{0,0}, {-1,0}, {0,-1}, {0,1}}
};

int c_y = spawn_point[1];
int c_rotation = 0;
int c_piece[4][2] = { 0 };
int c_type = 1;
clock_t begin_time;

unsigned int g_program_obj = 0;

float g_light0_position[] = {1.0f, 1.0f, 2.0f, 0.0f};

void spawn_block(int t) {
	switch(t) {
		case 1:
			for(int i = 0; i < 4; i++) {
				for(int j = 0; j < 2; j++) {
					c_piece[i][j] = spawn_point[j] + t_block[c_rotation][i][j];
				}
			}
	}
}

void draw_arena(float c_size) {
	// Takes the array and draws the blocks that are currently in play
	for(int i = 0; i < arena_w; i++) {
		for(int j = 0; j < arena_h; j++) {
			switch(arena[i][j]) {
				case 1: glColor4fv(red); break;
				case 2: glColor4fv(green); break;
				case 3: glColor4fv(blue); break;
				case 8: glColor4fv(blue); break;
				default: continue;
			}
			glPushMatrix();
				glTranslatef(c_size * i, c_size * j, 0.0f);
				glutSolidCube(c_size);
				/*
				glColor4f(0.0f, 0.0f, 0.0f, 1.0f);
				glLineWidth(2.0f);
				glutWireCube(c_size - 0.01);
				*/
			glPopMatrix();
		}
	}
}

void update_arena() { // Moves down the current piece, checking there is space to do so
	bool clear = true;
	int temp_x = 0;
	int temp_y = 0;
	int block_below = 0;
	for(int i = 0; i < 4; i++) {
		temp_x = c_piece[i][0];
		temp_y = c_piece[i][1];
		block_below = arena[temp_x][temp_y - 1];
		printf("At point (%d, %d), the block below is %d\n", temp_x, temp_y, block_below);
		if(block_below != 0 && block_below != 8) {
			clear = false;
			c_type = 0;
			c_y = spawn_point[1];
		}
	}

	if(clear) {
		for(int i = 0; i < 4; i++) {
			arena[c_piece[i][0]][c_piece[i][1]] = 0;
			arena[c_piece[i][0]][c_piece[i][1] - 1] = 8;
			c_piece[i][1] -= 1;
			c_y -= 1;
		}
	}
	draw_arena(cube_size);
}

void idle() {
	if(float(clock() - begin_time) / CLOCKS_PER_SEC > 0.1f) {
		//printf("A second has passed\n");
		begin_time = clock();
		if(c_type != 0) {
			update_arena();
		} else {
			draw_arena(cube_size);
		}
		glutPostRedisplay();
	}
	usleep(10);
}

void display() {
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT); 
	
	// position and orient camera
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(eye_x, eye_y, 1.0f, // eye position
			  0, 0, 0, // reference point
			  0, 1, 0  // up vector
		);
	
	glEnable(GL_LIGHTING);
	glUseProgram(g_program_obj);
	glColor4f(1.0f, 0.0f, 0.0f, 1.0f);

	glPushMatrix();
		glTranslatef(cube_size / 2, cube_size / 2, 0); // Draws from centre, so translates so draws from bottom left corner
		glTranslatef(b_l[0], b_l[1], 0.0f); // put cursor in bottom left corner of arena
		draw_arena(cube_size);
	glPopMatrix();
	glUseProgram(0);

	glDisable(GL_LIGHTING); // Draw arena borders
	/*
	glBegin(GL_QUADS);
		glColor4f(0.66f, 0.66f, 0.66f, 1.0f);

		glVertex3f(b_r[0], b_r[1], -0.05f);
		glVertex3f(b_l[0], b_l[1], -0.05f);
		glVertex3f(t_l[0], t_l[1], -0.05f);
		glVertex3f(t_r[0], t_r[1], -0.05f);
	glEnd();
	*/
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

	glEnable(GL_LIGHTING);

	glutSwapBuffers();
}

void reshape(int w, int h) {
	glViewport(0, 0, w, h); 
	glMatrixMode(GL_PROJECTION); 
	glLoadIdentity();
	//gluPerspective(40.0, 1.0f, 1.0, 5.0);
	glOrtho(-1.2, 1.2, -1.2, 1.2, -4.0, 4.0);

	glutPostRedisplay();
}

void keyboard(unsigned char key, int, int) {
	switch (key) {
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
	printf("x: %f, y: %f", eye_x, eye_y);
	glutPostRedisplay();
}

void init(int argc, char* argv[]) {
	if (argc==3)
		g_program_obj = create_and_compile_shaders(argv[1], NULL, argv[2]);
	else
	if (argc==4)
		g_program_obj = create_and_compile_shaders(argv[1], argv[2], argv[3]);

	// re-link geometry shader if change the primitive assembly type
	if (g_program_obj)
	{
		set_geometry_shader_params(g_program_obj, GL_LINES, GL_LINE_STRIP, 4);
	}

	init_lights(GL_SMOOTH);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);

	//init_material();

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
}

int main(int argc, char* argv[]) {
	/*
	for(int i = 0; i < 10; i++) {
		for(int j = 0; j< 20; j++) {
			printf("%d\n", arena[i][j]);
		}
	}
	*/
	// test blocks
	spawn_block(1);

	for(int i = 0; i < 4; i++) {
		arena[c_piece[i][0]][c_piece[i][1]] = 8;
	}
	

	arena[0][0] = 1;
	arena[1][0] = 1;
	arena[2][0] = 1;
	arena[1][1] = 1;

	arena[9][0] = 2;
	arena[9][1] = 2;
	arena[9][2] = 2;
	arena[9][3] = 2;

	arena[3][0] = 3;
	arena[4][0] = 3;
	arena[3][1] = 3;
	arena[2][1] = 3;
	begin_time = clock();

	glutInit(&argc, argv); 

	glutInitDisplayMode(
			// GLUT_3_2_CORE_PROFILE| // this will give you GLSL 1.5 on -- be careful what you wish for
			GLUT_DOUBLE|GLUT_RGBA|GLUT_DEPTH); 

	glutInitWindowSize(768, 768); 
	glutInitWindowPosition(50, 50); 

	glutCreateWindow("Tetris"); 

#ifndef __APPLE__ 
	GLenum err = glewInit();
	if (GLEW_OK!=err)
	{
		fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
		exit(1);
	}

	fprintf(stderr, "Using GLEW %s\n", glewGetString(GLEW_VERSION));
#endif

	glutKeyboardFunc(keyboard); 
	glutReshapeFunc(reshape); 
	glutDisplayFunc(display); 
	glutIdleFunc(idle); 

	fprintf(stderr, "Open GL Engine = %s\nVersion =  %s\n", 
				glGetString(GL_RENDERER), 
				glGetString(GL_VERSION)
			);

	init(argc, argv); 

	glutMainLoop(); 

	return 0; 
}
