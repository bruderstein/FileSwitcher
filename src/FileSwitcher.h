#ifndef _FILESWITCHER_H
#define _FILESWITCHER_H

#include "EditFile.h"
#include "Globals.h"
#include "Notepad_plus_msgs.h"
#include "../common/PluginInterface.h"
#include <map>
#include <string>
#include <algorithm>

void redisplaySwitchDialog();
void saveColumnWidths();
void loadFiles(tstring root, int maxFiles);

// ANSI compatibility for std::regex and dirent.h
#ifdef UNICODE
# define tregex wregex

# define topendir _wopendir
# define treaddir _wreaddir
# define tclosedir _wclosedir
# define TDIR _WDIR
# define tdirent _wdirent
#else
# define tregex regex

# define topendir opendir
# define treaddir readdir
# define tclosedir closedir
# define TDIR DIR
# define tdirent dirent
#endif

#endif
