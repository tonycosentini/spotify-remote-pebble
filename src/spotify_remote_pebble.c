#include <pebble.h>
#include "model.h"
#include "loading_window.h"
#include "playlists_menu_window.h"
#include "error_window.h"

static Window *loading_window;
static Window *playlists_menu_window;
static Window *error_window;
static SpotifyPlaylist *playlists_head;

static const int KEY_MESSAGE = 1;
static const int KEY_PLAYLIST_NAME = 2;
static const int KEY_PLAYLIST_ID = 3;
static const int KEY_PLAY_PLAYLIST = 4;

static const int VALUE_CONNECT = 1;
static const int VALUE_MESSAGE_RESET = 2;
static const int VALUE_DID_RESET = 3;
static const int VALUE_DID_RECEIVE_ALL_PLAYLISTS = 4;
static const int VALUE_MUST_LAUNCH_SPOTIFY = 5;
static const int VALUE_MUST_AUTHORIZE_SPOTIFY = 6; 

// Utils
static void hide_loading_window() {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Hiding loading window!");
  window_stack_remove(loading_window, false);
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Done!");
}

static void show_error_window(char *error_message) {
    error_window = error_window_create(error_message);
    window_stack_push(error_window, true);
}

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
  APP_LOG(APP_LOG_LEVEL_DEBUG, translate_error(reason));

  if (reason == APP_MSG_SEND_TIMEOUT) {
    Tuple *message_tuple = dict_find(failed, KEY_MESSAGE);
    if (message_tuple) {
      int value = (int)message_tuple->value->uint32;
      APP_LOG(APP_LOG_LEVEL_DEBUG, "Did not receive ack for outgoing message with value: %d", value);
    }

    show_error_window("Unable to connect to Playlists Remote Android app. Please make sure the app is running on your device and your Pebble is connected.");
    hide_loading_window();
  }
}

static void in_received_handler(DictionaryIterator *received, void *context) {
  Tuple *message_tuple = dict_find(received, KEY_MESSAGE);
  if (message_tuple) {
    int value = (int)message_tuple->value->uint32;
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Message received with value: %d", value);

    if (value == VALUE_DID_RECEIVE_ALL_PLAYLISTS && window_stack_contains_window(loading_window)) {
      playlists_menu_window = playlists_window_create(playlists_head);
      window_stack_push(playlists_menu_window, true);
      hide_loading_window();
    } else if (value == VALUE_MUST_AUTHORIZE_SPOTIFY) {
      show_error_window("Please authorize your Spotify Account within the Playlists Remote app on your device.");
      hide_loading_window();
    } else if (value == VALUE_MUST_LAUNCH_SPOTIFY) {
      show_error_window("Spotify must be running on your device in order to use Playlists Remote.");
      hide_loading_window();
    }

    return;
  }

  if (window_stack_contains_window(loading_window)) {
    // Check for fields you expect to receive
    Tuple *id_tuple = dict_find(received, KEY_PLAYLIST_ID);
    Tuple *name_tuple = dict_find(received, KEY_PLAYLIST_NAME);

    // Act on the found fields received
    if (id_tuple && name_tuple) {
      SpotifyPlaylist *playlist = spotify_playlist_create(id_tuple->value->int32, &(name_tuple->value->cstring)[0]);
      playlists_head = spotify_playlist_add_to_list(playlists_head, playlist);
    }
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

  window_stack_push(loading_window, false);

  setup_app_message();
  send_initial_message();

  app_event_loop();
}
