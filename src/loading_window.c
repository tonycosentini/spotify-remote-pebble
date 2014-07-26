#include "loading_window.h"
#include <stdlib.h>

static TextLayer *text_layer;

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_frame(window_layer);

  text_layer = text_layer_create(bounds);
  text_layer_set_text(text_layer, "Loading...");

  layer_add_child(window_layer, text_layer_get_layer(text_layer));
}

static void window_unload(Window *window) {
  text_layer_destroy(text_layer);
}

Window *loading_window_create() {
  Window *window = window_create();

  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });

  return window;
}