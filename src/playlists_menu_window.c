#include "playlists_menu_window.h"

static MenuLayer *playlists_menu_layer;
static SpotifyPlaylist *menu_playlists;

static const int KEY_PLAY_PLAYLIST = 4;

// Menu

#define NUM_MENU_SECTIONS 1
#define NUM_FIRST_MENU_ITEMS 2

static uint16_t menu_get_num_sections_callback(MenuLayer *menu_layer, void *data) {
  return NUM_MENU_SECTIONS;
}

static uint16_t menu_get_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *data) {
  switch (section_index) {
    case 0:
      return spotify_playlist_get_count(menu_playlists);
    default:
      return 0;
  }
}

static int16_t menu_get_header_height_callback(MenuLayer *menu_layer, uint16_t section_index, void *data) {
  return 0;
}

static void menu_draw_header_callback(GContext* ctx, const Layer *cell_layer, uint16_t section_index, void *data) {
  menu_cell_basic_header_draw(ctx, cell_layer, NULL);
}

static void menu_draw_row_callback(GContext* ctx, const Layer *cell_layer, MenuIndex *cell_index, void *data) {
  char *playlist_name = spotify_playlist_get(menu_playlists, cell_index->row)->name;
  menu_cell_basic_draw(ctx, cell_layer, playlist_name, NULL, NULL);
}

void menu_select_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *data) {
  uint32_t playlist_id = spotify_playlist_get(menu_playlists, cell_index->row)->id;

  DictionaryIterator *iterator;
  app_message_outbox_begin(&iterator);
  Tuplet app_init_tuplet = TupletInteger(KEY_PLAY_PLAYLIST, (uint8_t) playlist_id);
  dict_write_tuplet(iterator, &app_init_tuplet);
  app_message_outbox_send();
}

static MenuLayer *playlists_menu_layer_create(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_frame(window_layer);

  MenuLayer *menu_layer = menu_layer_create(bounds);

  menu_layer_set_callbacks(menu_layer, NULL, (MenuLayerCallbacks){
    .get_num_sections = menu_get_num_sections_callback,
    .get_num_rows = menu_get_num_rows_callback,
    .get_header_height = menu_get_header_height_callback,
    .draw_header = menu_draw_header_callback,
    .draw_row = menu_draw_row_callback,
    .select_click = menu_select_callback,
  });

  menu_layer_set_click_config_onto_window(menu_layer, window);

  return menu_layer;
}

// Window

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);

  playlists_menu_layer = playlists_menu_layer_create(window);

  layer_add_child(window_layer, menu_layer_get_layer(playlists_menu_layer));
}

static void window_unload(Window *window) {
  menu_layer_destroy(playlists_menu_layer);
}

Window *playlists_window_create(SpotifyPlaylist *playlists_head) {
  menu_playlists = playlists_head;

  Window *window = window_create();

  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });

  return window;
}
