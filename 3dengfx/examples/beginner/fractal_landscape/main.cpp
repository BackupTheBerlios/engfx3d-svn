/*
 * fractal_landscape
 * This example demonstrates how to use the 3dengfx geometry generation
 * functions to create a fractal landscape.
 */

#include <iostream>
#include "3dengfx/3dengfx.hpp"

using namespace std;

bool init();
void clean_up();
void update_gfx();
void key_handler(int key);

GraphicsInitParameters *gip;
Scene *scene;

ntimer timer;

int main(int argc, char **argv) {
	if(!init()) {
		return -1;
	}

	// pass control to 3dengfx
	return fxwt::main_loop();
}

bool init() {
	// use this function to load a configuration file
	gip = load_graphics_context_config("3dengfx.conf");
	if (!gip)
	{
		// alternatively, one can set the params by hand
		GraphicsInitParameters params;
		params.x = 640;
		params.y = 480;
		params.fullscreen = false;
		params.dont_care_flags = DONT_CARE_BPP | DONT_CARE_DEPTH | DONT_CARE_STENCIL;
		gip = &params;
	}
	
	// create graphics context
	if(!create_graphics_context(*gip)) {
		return false;
	}

	fxwt::set_window_title("3dengfx example: fractal_landscape");

	fxwt::set_display_handler(update_gfx);
	fxwt::set_idle_handler(update_gfx);
	fxwt::set_keyboard_handler(key_handler);
	atexit(clean_up);

	scene = new Scene;
	
	Camera *cam = new TargetCamera(Vector3(0, 5, 0), Vector3(0, 0, 0));
	scene->add_camera(cam);

	MotionController c(CTRL_SIN, TIME_FREE);
	c.set_sin_func(1.0, 10);
	c.set_control_axis(CTRL_X);
	cam->add_controller(c, CTRL_TRANSLATION);

	c.set_sin_func(1.0, 10, half_pi);
	c.set_control_axis(CTRL_Z);
	cam->add_controller(c, CTRL_TRANSLATION);

	Light *lt = new PointLight(Vector3(-20, 20, -20));
	scene->add_light(lt);

	Object *obj = new Object;
	create_landscape(obj->get_mesh_ptr(), Vector2(10, 10), 100, 4, 500);
	
	obj->set_dynamic(false);
	cout << "polygons: " << obj->get_triangle_count() << endl;
	scene->add_object(obj);

	timer_reset(&timer);
	timer_start(&timer);

	return true;
}

void update_gfx() {
	scene->render(timer_getmsec(&timer));
	
	flip();
}

void clean_up() {
	destroy_graphics_context();
}

void key_handler(int key) {
	// exit when escape or 'q' is pressed
	switch(key) {
	case fxwt::KEY_ESCAPE:
	case 'q':
		exit(0);
	default:
		break;
	}
}

