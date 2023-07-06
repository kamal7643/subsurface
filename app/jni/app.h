

#ifndef APP_H
#define APP_H

bool load_app();
bool init_app(int width, int height);
void update_app(float dt);
void render_app(float dt);
void free_app();
void on_pointer_down(float x, float y);
void on_pointer_up(float x, float y);

#endif