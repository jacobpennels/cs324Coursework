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
#include <sstream>

#include "draw_text.h"
#include "lights_material.h"
#include "create_and_compile_shaders.h"
#include "block.h"
//#include "tetris_control.h"

float difficulty = 0.75f;
float speed_up = 1.0f;
bool game_over = false;
int points = 0;

float t_l[2] = {-0.4f, 0.9f};
float t_r[2] = {0.4f, 0.9f};
float b_l[2] = {-0.4f, -0.7f};
float b_r[2] = {0.4f, -0.7f};

float i_colour[4] = {0.0f, 1.0f, 1.0f, 1.0f};
float j_colour[4] = {0.0f, 0.0f, 1.0f, 1.0f};
float l_colour[4] = {1.0f, (170.0f/255.0f), 0.0f, 1.0f};
float o_colour[4] = {1.0f, 1.0f, 0.0f, 1.0f};
float z_colour[4] = {0.0f, 1.0f, 0.0f, 1.0f};
float t_colour[4] = {(153.0f / 255.0f), 0.0f, 1.0f, 1.0f};
float s_colour[4] = {1.0f, 0.0f, 0.0f, 1.0f};

float height = t_l[1] - b_l[1];
float width = t_r[0] - t_l[0];
const int arena_h = 22;
const int arena_w = 10;
int arena[arena_w][arena_h] = { 0 }; // [x][y] 10x20 grid with 2 on top for new pieces to spawn in
const int spawn_point[2] = {4, 20};

float cube_size = (height / 20.0f);

float eye_x = 0.05f;
float eye_y = 0.05f;

int c_pos[2] = {spawn_point[0], spawn_point[1]};
int c_rotation = 0;
int c_piece[4][2] = { 0 };
int c_type = 0;
clock_t begin_time;
int full_lines[arena_h] = { 0 }; // Stores lines that are full and can be cleared 

// Variables for storing keyboard inputs
bool key_pressed = false;
int key_id = 0; // 0 = none, 1 = left, 2 = right

unsigned int g_program_obj = 0;

float g_light0_position[] = {1.0f, 1.0f, 2.0f, 0.0f};

void print_arena() {
	for(int i = 0; i < arena_h; i++) {
		for(int j = 0; j < arena_w; j++) {
			printf("%d", arena[j][arena_h - i - 1]);
		}
		printf("\n");
	}
}

void spawn_block(int t) {
	// 1 = t, 2 = l, 3 = j, 4 = z, 5 = s, 6 = i, 7 = o
	// For the given type, form the coordinates for that block
	switch(t) {
		case 1:
			for(int i = 0; i < 4; i++) {
				for(int j = 0; j < 2; j++) {
					c_piece[i][j] = c_pos[j] + t_block[c_rotation][i][j];
				}
			}
			break;
		case 2:
			for(int i = 0; i < 4; i++) {
				for(int j = 0; j < 2; j++) {
					c_piece[i][j] = c_pos[j] + l_block[c_rotation][i][j];
				}
			}
			break;
		case 3:
			for(int i = 0; i < 4; i++) {
				for(int j = 0; j < 2; j++) {
					c_piece[i][j] = c_pos[j] + j_block[c_rotation][i][j];
				}
			}
			break;
		case 4:
			for(int i = 0; i < 4; i++) {
				for(int j = 0; j < 2; j++) {
					c_piece[i][j] = c_pos[j] + z_block[c_rotation][i][j];
				}
			}
			break;
		case 5:
			for(int i = 0; i < 4; i++) {
				for(int j = 0; j < 2; j++) {
					c_piece[i][j] = c_pos[j] + s_block[c_rotation][i][j];
				}
			}
			break;
		case 6:
			for(int i = 0; i < 4; i++) {
				for(int j = 0; j < 2; j++) {
					c_piece[i][j] = c_pos[j] + i_block[c_rotation][i][j];
				}
			}
			break;
		case 7:
			for(int i = 0; i < 4; i++) {
				for(int j = 0; j < 2; j++) {
					c_piece[i][j] = c_pos[j] + o_block[c_rotation][i][j];
				}
			}
			break;
	}
}

void get_lookahead(float c_size) {
	// Take current piece and draw lookahead outline
	// Select the right colour
	switch(c_type) {
		case 1: glColor4fv(t_colour); break;
		case 2: glColor4fv(l_colour); break;
		case 3: glColor4fv(j_colour); break;
		case 4: glColor4fv(z_colour); break;
		case 5: glColor4fv(s_colour); break;
		case 6: glColor4fv(i_colour); break;
		case 7: glColor4fv(o_colour); break;
	}

	int min_distance = 23;
	for(int i = 0; i < 4; i++) {
		int t = c_piece[i][1];
		if(t < min_distance) { // reached bottom of arena
			min_distance = t;
		}
		for(int y = t - 1; y >= 0; y--) {
			if(arena[c_piece[i][0]][y] != 0 && arena[c_piece[i][0]][y] != 8) { 
				// Reached the closest block below
				int dis = (t - y) - 1;
				if(dis < min_distance) {
					min_distance = dis;
				}
				break;
			}	
		}
	}

	for(int i = 0; i < 4; i++) {
		glPushMatrix();
			glTranslatef(c_size * c_piece[i][0], c_size * (c_piece[i][1] - min_distance), 0.0f);
			glutWireCube(c_size - 0.01);
		glPopMatrix();
	}
}

void draw_arena(float c_size) {
	// Takes the array and draws the blocks that are currently in play
	bool block_in_arena = false;
	for(int i = 0; i < arena_w; i++) {
		for(int j = 0; j < arena_h; j++) {
			int current_type = arena[i][j];
			if(current_type == 8) {
				current_type = c_type;
				block_in_arena = true;
			}
			switch(current_type) {
				case 1: glColor4fv(t_colour); break;
				case 2: glColor4fv(l_colour); break;
				case 3: glColor4fv(j_colour); break;
				case 4: glColor4fv(z_colour); break;
				case 5: glColor4fv(s_colour); break;
				case 6: glColor4fv(i_colour); break;
				case 7: glColor4fv(o_colour); break;
				default: continue;
			}
			glPushMatrix();
				glTranslatef(c_size * i, c_size * j, 0.0f);
				glutSolidCube(c_size - 0.01);
				/*
				glColor4f(0.0f, 0.0f, 0.0f, 1.0f);
				glLineWidth(2.0f);
				glutWireCube(c_size - 0.01);
				*/
			glPopMatrix();
		}
	}
	if(block_in_arena) {
		get_lookahead(c_size);
	}
}

void update_arena() { // Moves down the current piece, checking there is space to do so, if not stopping the piece
	bool clear = true;
	int temp_x = 0;
	int temp_y = 0;
	int block_below = 0;
	for(int i = 0; i < 4; i++) {
		temp_x = c_piece[i][0];
		temp_y = c_piece[i][1];
		block_below = arena[temp_x][temp_y - 1];
		//printf("At point (%d, %d), the block below is %d\n", temp_x, temp_y, block_below);
		if(block_below != 0 && block_below != 8) {
			clear = false;
			//c_type = 0;
			c_pos[0] = spawn_point[0];
			c_pos[1] = spawn_point[1];
		}
		if(temp_y - 1 < 0) {
			clear = false;
			//c_type = 0;
			c_pos[0] = spawn_point[0];
			c_pos[1] = spawn_point[1];
		}
	}

	if(clear) {
		for(int i = 0; i < 4; i++) {
			arena[c_piece[i][0]][c_piece[i][1]] = 0;
		}
		for(int i = 0; i < 4; i++) {
			arena[c_piece[i][0]][c_piece[i][1] - 1] = 8;
			c_piece[i][1] -= 1;	
		}
		c_pos[1] -= 1;
	} else {
		for(int i = 0; i < 4; i++) {
			arena[c_piece[i][0]][c_piece[i][1]] = c_type;
		}
		c_type = 0;
	}
}

void move_left() {
	// moves the current piece to left
	bool clear = true;
	for(int i = 0; i < 4; i++) {
		int temp_x = c_piece[i][0];
		int temp_y = c_piece[i][1];
		int left_neighbour = arena[temp_x - 1][temp_y];

		if(left_neighbour != 0 && left_neighbour != 8) {
			clear = false;
		}
		if(temp_x - 1 < 0) {
			clear = false;
		}
	}

	if(clear) {
		for(int i = 0; i < 4; i++) {
			arena[c_piece[i][0]][c_piece[i][1]] = 0;
		}

		for(int i = 0; i < 4; i++) {
			c_piece[i][0] -= 1;
			arena[c_piece[i][0]][c_piece[i][1]] = 8;
		}
		c_pos[0] -= 1;
	}
	//printf("Moved left\n");
	//print_arena();
	//glutPostRedisplay();
}

void move_right() {
	// moves current piece to right
	bool clear = true;
	for(int i = 0; i < 4; i++) {
		int temp_x = c_piece[i][0];
		int temp_y = c_piece[i][1];
		int left_neighbour = arena[temp_x + 1][temp_y];

		if(left_neighbour != 0 && left_neighbour != 8) {
			clear = false;
		}
		if(temp_x + 1 > 9) {
			clear = false;
		}
	}

	if(clear) {
		for(int i = 0; i < 4; i++) {
			arena[c_piece[i][0]][c_piece[i][1]] = 0;
		}

		for(int i = 0; i < 4; i++) {
			c_piece[i][0] += 1;
			arena[c_piece[i][0]][c_piece[i][1]] = 8;
		}
		c_pos[0] += 1;
	}
	//printf("Moved right\n");
	//print_arena();
	//glutPostRedisplay();
}

void rotate_block(int dir) {
	// Follows SRS rotation rules, implements the 'wallkick' feature found in traditional tetris
	if(dir) {
		// Clockwise
		c_rotation += 1;
		if(c_rotation > 3) {
			c_rotation = 0;
		}
	} else {
		c_rotation -= 1;
		if(c_rotation < 0) {
			c_rotation = 3;
		}
	}
	for(int i = 0; i < 4; i++) {
		arena[c_piece[i][0]][c_piece[i][1]] = 0;
	}

	spawn_block(c_type);
	int temp_x = c_pos[0];
	bool unsafe = true;
	// Check the block is safe where it is, and perform wall kick if necessary
	do {
		// Checks and moves the block until it is all in the arena
		unsafe = false;
		for(int i = 0; i < 4; i++) { // Do the wallkick
			if(c_piece[i][0] > 9) {
				c_pos[0] -= 1;
				unsafe = true;
				break;
			}
			if(c_piece[i][0] < 0) {
				c_pos[0] += 1;
				unsafe = true;
				break;
			}
		}
		spawn_block(c_type);
	} while(unsafe);

	if(c_pos[0] != temp_x) {
		spawn_block(c_type);
	}

	unsafe = false;
	for(int i = 0; i < 4; i++) {
		int block = arena[c_piece[i][0]][c_piece[i][1]];
		if(block != 8 && block != 0) {
			// moved into another block, so rotation is illegal
			unsafe = true;
		}
	}

	if(unsafe) {
		// Whole rotation was illegal, so reset to original position (Reverse whole rotation, as if nothing happened)
		c_pos[0] = temp_x;
		if(!dir) {
			c_rotation += 1;
			if(c_rotation > 3) {
				c_rotation = 0;
			}
		} else {
			c_rotation -= 1;
			if(c_rotation < 0) {
				c_rotation = 3;
			}
		}
	}
	spawn_block(c_type);
	for(int i = 0; i < 4; i++) {
		arena[c_piece[i][0]][c_piece[i][1]] = 8;
	}
}

void check_for_lines() {
	// Checks for full lines, and clear them
	// Uses naive gravity - blocks fall only by the number of line cleared below them
	int num_lines = 0; // Keep track of lines cleared in one go
	for(int j = 0; j < arena_h; j++) {
		full_lines[j] = 1;
		for(int i = 0; i < arena_w; i++) {
			if(arena[i][j] == 0) {
				full_lines[j] = 0;
			}
		}
	}

	for(int i = 0; i < arena_h; i++) {
		if(full_lines[i] == 1) {
			num_lines += 1;
			for(int j = 0; j <  arena_w; j++) {
				arena[j][i] = 0; // Clear the layer that needs to be cleared
			}
		}
	}
	//printf("NUMBER OF LINES TO CLEAR IS: %d\n", num_lines);

	for(int i = arena_h - 1; i >= 0; i--) {
		if(full_lines[i] == 1) {
			for(int x = 0; x < arena_w; x++) {
				for(int y = i + 1; y < arena_h; y++) {
					int t = arena[x][y];
					arena[x][y] = 0;
					arena[x][y-1] = t;
				}
			}
		}
	}
	// Calculate score based on number of lines cleared and difficulty level
	int new_score = 0;
	switch(num_lines) {
		case 1:
			new_score = 40;
			break;
		case 2:
			new_score = 100;
			break;
		case 3:
			new_score = 300;
			break;
		case 4:
			new_score = 1200;
			break;
	}
	//printf("%d\n", new_score);
	switch((int)(difficulty * 100)) {
		case 100: // very easy
			points += new_score;
			break;
		case 75:
			points += new_score * 2;
			break;
		case 50:
			points += new_score * 3;
			break;
		case 25:
			points += new_score * 4;
			break;
	}
}

void check_for_defeat() {
	for(int i = 0; i < arena_w; i++) {
		for(int j = 20; j < arena_h; j++) {
			if(arena[i][j] != 0) {
				// Block outside of the play area
				game_over = true;
			}
		}
	}
}

void reset_arena() {
	// Clear arena for new game
	for(int i = 0; i < arena_w; i++) {
		for(int j = 0; j < arena_h; j++) {
			arena[i][j] = 0;
		}
	}
}

void timer(int) {
	// This is run once a second and provides basic clock for moving the piece down
	game_over = false;
	if(c_type != 0) {
		if(speed_up < 1.0f) {
			// Soft drop
			points += 1;
		}
		update_arena();
	} else {
		check_for_defeat();
		if(!game_over) {
			check_for_lines();
			c_pos[0] = spawn_point[0];
			c_pos[1] = spawn_point[1];
			c_rotation = 0;
			c_type = rand() % 7 + 1;
			//printf("%d\n", c_type);
			spawn_block(c_type);
			speed_up = 1.0f; // Reset to normal fall rate
		} else { // Game over
			printf("GAME OVER\n");
			reset_arena();
			glutPostRedisplay();
			glutTimerFunc(5000, timer, 0);
			points = 0;
			return;
		}
	}
	glutPostRedisplay();
	glutTimerFunc((int)(1000.0f * difficulty * speed_up), timer, 0);
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

	if(game_over) {
		glDisable(GL_LIGHTING);
	        // put some help on the screen
	        draw_text(250, 550, "GAME OVER");

	        std::stringstream oss;
        	oss << "You scored " << points;
        	draw_text(250, 450, (char*)oss.str().c_str());
		glEnable(GL_LIGHTING);
	} else {
		glPushMatrix();
			glRotatef(-20.0f, 0.0f, 1.0f, 0.0f); // Adjust to show 3D
			glRotatef(-15.0f, 1.0f, 0.0f, 0.0f);

			glEnable(GL_LIGHTING);
			glUseProgram(g_program_obj);
			glColor4f(1.0f, 0.0f, 0.0f, 1.0f);

			glPushMatrix();
				glTranslatef(cube_size / 2, cube_size / 2, 0); // Draws from centre, so translates so draws from bottom left corner
				glTranslatef(b_l[0], b_l[1], 0.0f); // put cursor in bottom left corner of arena
				draw_arena(cube_size); // Draws the whole arena
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
			glColor4f(0.0f, 1.0f, 0.0f, 0.5f);
			glLineWidth(0.1f);
			glBegin(GL_QUADS);
				glVertex3f(t_l[0] - 0.01, t_l[1] + 0.01, 0.05f);
				glVertex3f(b_l[0] - 0.01, b_l[1] - 0.01, 0.05f);
				glVertex3f(b_l[0] - 0.01, b_l[1] - 0.01, -0.05f);
				glVertex3f(t_l[0] - 0.01, t_l[1] + 0.01, -0.05f);
				
				glVertex3f(b_l[0] - 0.01, b_l[1] - 0.01, 0.05f);
				glVertex3f(b_r[0] + 0.01, b_r[1] - 0.01, 0.05f);
				glVertex3f(b_r[0] + 0.01, b_r[1] - 0.01, -0.05f);
				glVertex3f(b_l[0] - 0.01, b_l[1] - 0.01, -0.05f);
			
				glVertex3f(b_r[0] + 0.01, b_r[1] - 0.01, 0.05f);
				glVertex3f(t_r[0] + 0.01, t_r[1] + 0.01, 0.05f);
				glVertex3f(t_r[0] + 0.01, t_r[1] + 0.01, -0.05f);
				glVertex3f(b_r[0] + 0.01, b_r[1] - 0.01, -0.05f);
			glEnd();
		glPopMatrix();

		// show score
	    std::stringstream oss;
        oss << "POINTS: " << points;
        draw_text(80, 80, (char*)oss.str().c_str());

		glEnable(GL_LIGHTING);
}

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

void keyboard(unsigned char key, int, int) { // Debugging stuff for finding best pseudo 3d angle
	switch (key) {
		case 'q': exit(1); break;
		case 'w': eye_y += 0.1f; break;
		case 's': eye_y -= 0.1f; break;
		case 'd': eye_x += 0.1f; break;
		case 'a': eye_x -= 0.1f; break;
		case ' ': 
			//printf("Spacebar pressed\n"); 
			speed_up = 0.1f;
			break;
	}
	//printf("x: %f, y: %f", eye_x, eye_y);
	glutPostRedisplay();
}

void keyboard_up(unsigned char key, int, int) {
	switch(key) {
		case ' ': 
			//printf("Spacebar released\n");
			speed_up = 1.0f;
			break;
	}
}

void special_up(int key, int, int) {
	// Games tend to react to key up
	switch(key) {
		case GLUT_KEY_LEFT:
			//printf("Left key released\n");
			move_left();
			glutPostRedisplay();
			break;
		case GLUT_KEY_RIGHT:
			//printf("Right key released\n");
			move_right();
			glutPostRedisplay();
			break;
		case GLUT_KEY_UP:
			// Rotate clockwise
			rotate_block(1);
			glutPostRedisplay();
			break;
		case GLUT_KEY_DOWN:
			rotate_block(0);
			glutPostRedisplay();
			break;
	}
}

void special(int key, int, int) {
	switch(key) {
		case GLUT_KEY_LEFT:
			key_id = 1;
			//printf("Left arrow pressed\n");
			//printf("%d\n", key_id);
			break;
		case GLUT_KEY_RIGHT:
			key_id = 2;
			//printf("Right arrow pressed\n");
			//printf("%d\n", key_id);
			break;
		default:
			key_id = 0;
	}
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

	init_material();

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

	// seed random generator
	printf("#################################\n\n");
	printf("        WELCOME TO TETRIS!       \n\n");
	printf("#################################\n\n");
	printf("Pick a difficulty:\n");
	printf("1:	Very Easy\n");
	printf("2:	Easy\n");
	printf("3:	Medium\n");
	printf("4:	Hard\n");
	int a;
	bool valid = false;

	do {
		printf("Enter difficulty: ");
		scanf("%u", &a);

		switch(a) {
			case 1:
				difficulty = 1.0f;
				valid = true;
				break;
			case 2:
				difficulty = 0.75f;
				valid = true;
				break;
			case 3:
				difficulty = 0.5f;
				valid = true;
				break;
			case 4:
				difficulty = 0.25f;
				valid = true;
				break;
		}
		if(!valid)
			printf("Not a valid difficulty!\n");
	} while(!valid);

	srand(time(0));

	// test blocks
	c_type = rand() % 7 + 1;
	spawn_block(c_type);

	begin_time = clock();

	glutInit(&argc, argv); 

	glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGBA|GLUT_DEPTH); 

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
	glutKeyboardUpFunc(keyboard_up);
	glutSpecialFunc(special);
	glutSpecialUpFunc(special_up); // Detect key release
	glutReshapeFunc(reshape); 
	glutDisplayFunc(display); 
	glutIdleFunc(NULL); 
	glutTimerFunc((int)(1000.0f * difficulty * speed_up), timer, 0);
	glutIgnoreKeyRepeat(1);

	fprintf(stderr, "Open GL Engine = %s\nVersion =  %s\n", 
				glGetString(GL_RENDERER), 
				glGetString(GL_VERSION)
			);

	init(argc, argv); 

	glutMainLoop(); 

	return 0; 
}
