#ifndef TETRIS_CONTROL_H
#define TETRIS_CONTROL_H

struct tetro {
	float[2] pos_0;
	float[2] pos_1;
	float[2] pos_2;
	float[2] pos_3;
	int type;
	int id;
};

tetro create_block(float origin[2], float c_size, float type) {
	tetro t;
	t.type = type;

	switch(origin){
		case 0: // t shaped
			t.pos_0 = origin;
			t.pos_1 = {origin[0] + c_size, origin[1]};
			t.pos_2 = {origin[0] + (2*c_size), origin[1]};
			t.pos_3 = {origin[0] + c_size, origin[1] + c_size};
			return t;
	}
	return NULL;
}
#endif
