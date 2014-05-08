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

#endif
