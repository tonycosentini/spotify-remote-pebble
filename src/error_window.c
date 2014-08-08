#include "error_window.h"

static TextLayer *error_text_layer;
static char *window_error_message;

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_frame(window_layer);

  error_text_layer = text_layer_create(bounds);
  text_layer_set_text(error_text_layer, window_error_message);
  text_layer_set_text_alignment(error_text_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(error_text_layer));
}

static void window_unload(Window *window) {
  text_layer_destroy(error_text_layer);
  window_destroy(window);
}

Window *error_window_create(char *error_message) {
  Window *window = window_create();
  window_error_message = error_message;

  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });

  return window;
}