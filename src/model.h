#include "pebble.h"

#ifndef MODEL_H_
#define MODEL_H_

typedef struct SpotifyPlaylist SpotifyPlaylist;
struct SpotifyPlaylist {
  uint32_t id;
  char *name;
  SpotifyPlaylist *next;
};

SpotifyPlaylist *spotify_playlist_create(uint32_t id, char *name);

void spotify_playlist_free(SpotifyPlaylist *playlist);

SpotifyPlaylist *spotify_playlist_add_to_list(SpotifyPlaylist *list_head, SpotifyPlaylist *new_playlist);

int spotify_playlist_get_count(SpotifyPlaylist *list_head);

SpotifyPlaylist *spotify_playlist_get(SpotifyPlaylist *list_head, int index);

#endif
