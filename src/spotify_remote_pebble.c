#include <pebble.h>
#include "model.h"
#include "loading_window.h"
#include "playlists_menu_window.h"

static Window *loading_window;
static Window *playlists_menu_window;
static SpotifyPlaylist *playlists_head;

static const int KEY_MESSAGE = 1;
static const int KEY_PLAYLIST_NAME = 2;
static const int KEY_PLAYLIST_ID = 3;
static const int KEY_PLAY_PLAYLIST = 4;

static const int  VALUE_CONNECT = 1;
static const int  VALUE_MESSAGE_RESET = 2;
static const int  VALUE_DID_RESET = 3;
static const int  VALUE_DID_RECEIVE_ALL_PLAYLISTS = 4;

// App Communication

static char *translate_error(AppMessageResult result) {
  switch (result) {
    case APP_MSG_OK: return "APP_MSG_OK";
    case APP_MSG_SEND_TIMEOUT: return "APP_MSG_SEND_TIMEOUT";
    case APP_MSG_SEND_REJECTED: return "APP_MSG_SEND_REJECTED";
    case APP_MSG_NOT_CONNECTED: return "APP_MSG_NOT_CONNECTED";
    case APP_MSG_APP_NOT_RUNNING: return "APP_MSG_APP_NOT_RUNNING";
    case APP_MSG_INVALID_ARGS: return "APP_MSG_INVALID_ARGS";
    case APP_MSG_BUSY: return "APP_MSG_BUSY";
    case APP_MSG_BUFFER_OVERFLOW: return "APP_MSG_BUFFER_OVERFLOW";
    case APP_MSG_ALREADY_RELEASED: return "APP_MSG_ALREADY_RELEASED";
    case APP_MSG_CALLBACK_ALREADY_REGISTERED: return "APP_MSG_CALLBACK_ALREADY_REGISTERED";
    case APP_MSG_CALLBACK_NOT_REGISTERED: return "APP_MSG_CALLBACK_NOT_REGISTERED";
    case APP_MSG_OUT_OF_MEMORY: return "APP_MSG_OUT_OF_MEMORY";
    case APP_MSG_CLOSED: return "APP_MSG_CLOSED";
    case APP_MSG_INTERNAL_ERROR: return "APP_MSG_INTERNAL_ERROR";
    default: return "UNKNOWN ERROR";
  }
}

static void out_sent_handler(DictionaryIterator *sent, void *context) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Delivered outgoing message!");
}

static void out_failed_handler(DictionaryIterator *failed, AppMessageResult reason, void *context) {
  // outgoing message failed
  APP_LOG(APP_LOG_LEVEL_DEBUG, translate_error(reason));
}

static void in_received_handler(DictionaryIterator *received, void *context) {
  Tuple *message_tuple = dict_find(received, KEY_MESSAGE);
  if (message_tuple) {
    int value = (int)message_tuple->value->uint32;
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Message received with value: %d", value);

    if (value == VALUE_DID_RECEIVE_ALL_PLAYLISTS) {
        playlists_menu_window = playlists_window_create(playlists_head);
      window_stack_push(playlists_menu_window, true);
      window_stack_remove(loading_window, false);
    }

    return;
  }

  // Check for fields you expect to receive
  Tuple *id_tuple = dict_find(received, KEY_PLAYLIST_ID);
  Tuple *name_tuple = dict_find(received, KEY_PLAYLIST_NAME);

  // Act on the found fields received
  if (id_tuple && name_tuple) {
    SpotifyPlaylist *playlist = spotify_playlist_create(id_tuple->value->int32, &(name_tuple->value->cstring)[0]);
    playlists_head = spotify_playlist_add_to_list(playlists_head, playlist);
  }

}

static void in_dropped_handler(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, translate_error(reason));
}

static void setup_app_message() {
  app_message_register_inbox_received(in_received_handler);
  app_message_register_inbox_dropped(in_dropped_handler);
  app_message_register_outbox_sent(out_sent_handler);
  app_message_register_outbox_failed(out_failed_handler);

  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
}

static void send_initial_message() {
  DictionaryIterator *iterator;
  app_message_outbox_begin(&iterator);
  Tuplet app_init_tuplet = TupletInteger(KEY_MESSAGE, VALUE_CONNECT);
  dict_write_tuplet(iterator, &app_init_tuplet);
  app_message_outbox_send();
}

//

int main(void) {
  loading_window = loading_window_create();

  window_stack_push(loading_window, true /* Animated */);

  setup_app_message();
  send_initial_message();

  app_event_loop();

  window_destroy(loading_window);

  if (playlists_menu_window != NULL) {
    window_destroy(playlists_menu_window);
  }
}
