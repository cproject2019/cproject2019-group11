#ifndef PLAYLIST_H
#define PLAYLIST_H

int PlayListInit();
int PlayListDestroy();
char *GetItemFromDefaultPlaylist(int pos);
int GetDefaultPlaylistTotalItem();
int DefaultPlaylistAddItem(char *itempath);
int DefaultPlaylistDeleteItem(int pos);
int DefaultPlaylistSave();

#endif
