#include "model.h"
#include "pebble.h"
#include <stdio.h>

static char *strdup(const char *s) {
  char *t;
  int len;

  if (!s) return NULL;
  len = strlen(s);
  t = (char *) malloc(len + 1);
  memcpy(t, s, len + 1);
  return t;
}

SpotifyPlaylist *spotify_playlist_create(uint32_t id, char *name) {
  SpotifyPlaylist *playlist = (SpotifyPlaylist *)malloc(sizeof(SpotifyPlaylist));
  playlist->id = id;
  playlist->name = strdup(name);
  return playlist;
}

void spotify_playlist_free(SpotifyPlaylist *playlist) {
  if ((*playlist).next != NULL) {
    spotify_playlist_free((*playlist).next);
  }
  free(playlist->name);
  free(playlist);
}

SpotifyPlaylist *spotify_playlist_add_to_list(SpotifyPlaylist *list_head, SpotifyPlaylist *new_playlist) {
  if (list_head == NULL) {
    return new_playlist;
  }

  SpotifyPlaylist *current_item = list_head;
  while (current_item->next != NULL) {
    current_item = current_item->next;
  }
  current_item->next = new_playlist;

  return list_head;
}

int spotify_playlist_get_count(SpotifyPlaylist *list_head) {
  int count = 0;
  SpotifyPlaylist *current_item = list_head;
  while (current_item != NULL) {
    count++;
    current_item = current_item->next;
  }
  return count;
}

SpotifyPlaylist *spotify_playlist_get(SpotifyPlaylist *list_head, int index) {
  int current_index = 0;
  SpotifyPlaylist *current_item = list_head;

  while (current_index != index) {
    current_item = current_item->next;
    current_index++;
  }

  return current_item;
}
