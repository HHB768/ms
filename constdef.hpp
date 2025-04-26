#ifndef __CONSTDEF_HPP__
#define __CONSTDEF_HPP__

#include <iostream>
#include <unistd.h>

namespace mfwu {

// constexpr const char* CMD_CLEAR = "clear screen\n";
constexpr const char* CMD_CLEAR = "\033[2J\033[1;1H";
inline void cmd_clear() { std::cout << CMD_CLEAR; }
constexpr float MINE_POS_RATIO = 0.2F;
constexpr float eps = 0.01F;
constexpr const time_t XQ4MS_TIMESTAMP = 1741792500;

constexpr const char* QUIT_CMD1 = "\\QUIT";
constexpr const char* QUIT_CMD2 = "\\Q";
constexpr const char* QUIT_CMD3 = "\\quit";
constexpr const char* RESTART_CMD1 = "\\RESTART";
constexpr const char* RESTART_CMD2 = "\\R";
constexpr const char* RESTART_CMD3 = "\\restart";
constexpr const char* MENU_CMD1 = "\\MENU";
constexpr const char* MENU_CMD2 = "\\M";
constexpr const char* MENU_CMD3 = "\\menu";
constexpr const char* XQ4MS_CMD = "\\XQ4MS";


constexpr const char* HELPER_RETURN2MENU = "Key in \\RESTART or \\MENU or \\QUIT if you want";
constexpr const char* HELPER_PLACE_TILE  = "Key in R(eveal)/F(lag) and a pair of character to play, \n"
                                           "e.g., RAB for revealing the first row & the second col";
constexpr const char* HELPER_SELECT_MODE = "Plz key in your game mode: \n"
                                           "A.1. Human (default), B.2. Robot";
constexpr const char* HELPER_SELECT_SIZE = "Plz key in your scale of board: \n"
                                           "A.1. Small (default), B.2. Middle, C.3. Large";
constexpr const char* HELPER_PRESS_ANY_KEY = "Press any key to continue...";

constexpr const char* HELPER_INVALIDMODE_1 = "Invalid mode selection: input includes multiple chars T.T \n"
                                             "Just key in A/B or 1/2";
constexpr const char* HELPER_INVALIDMODE_2 = "Invalid mode selection: input is not a digit or character >.< \n"
                                             "Just key in A/B or 1/2";
constexpr const char* HELPER_INVALIDMODE_3 = "Invalid mode selection: input is not in alternative options -.- \n"
                                             "Just key in A/B or 1/2";
constexpr const char* HELPER_INVALIDSIZE_1 = "Invalid size selection: input includes multiple chars T.T \n"
                                             "Just key in A/B/C or 1/2/3";
constexpr const char* HELPER_INVALIDSIZE_2 = "Invalid size selection: input is not a digit or character >.< \n"
                                             "Just key in A/B/C or 1/2/3";
constexpr const char* HELPER_INVALIDSIZE_3 = "Invalid size selection: input is not in alternative options -.- \n"
                                             "Just key in A/B/C or 1/2/3";
constexpr const char* HELPER_REVEALED_POSITION = "Invalid position: already revealed";
constexpr const char* HELPER_INVALID_POSITION  = "Invalid position, plz try again";
                                             
constexpr const char* ERROR_NEW_GC = "An error occurs when we new GameController()";
constexpr const char* ERROR_UNKNOWN_COMMAND_TYPE = "Unknown CommandType";
constexpr const char* ERROR_UNKNOWN_TILE_STATUS = "Unknown tile status";
constexpr const char* ERROR_UNKNOWN_GAME_STATUS  = "Unknown game status";

}  // endof namespace mfwu

#endif  // __CONSTDEF_HPP__