#include "loading_window.h"
#include <stdlib.h>

static GBitmap *icon_bitmap;
static BitmapLayer *icon_bitmap_layer;

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_frame(window_layer);

  icon_bitmap = gbitmap_create_with_resource(RESOURCE_ID_LARGE_ICON);
  icon_bitmap_layer = bitmap_layer_create(layer_get_frame(window_layer));
  bitmap_layer_set_bitmap(icon_bitmap_layer, icon_bitmap);
  layer_add_child(window_layer, bitmap_layer_get_layer(icon_bitmap_layer));
}

static void window_unload(Window *window) {
  bitmap_layer_destroy(icon_bitmap_layer);
  gbitmap_destroy(icon_bitmap);
}

Window *loading_window_create() {
  Window *window = window_create();

  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });

  return window;
}