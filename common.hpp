#ifndef __COMMON_HPP__
#define __COMMON_HPP__

#include <bits/stdc++.h>
#include "constdef.hpp"

namespace mfwu {

#define likely(x)       __builtin_expect(!!(x), 1)
#define unlikely(x)     __builtin_expect(!!(x), 0)

enum class BoardSize : size_t {
    Small = 0,
    Middle = 1,
    Large = 2
};  // endof enum class BoardSize

struct BoardDimension {
    size_t height;
    size_t width;
};  // endof struct BoardDimension

constexpr BoardDimension BoardSize2Dimension[3] = {{12, 9}, {18, 15}, {20, 26}};
constexpr BoardDimension get_board_dimension(BoardSize sz) {
    return BoardSize2Dimension[static_cast<size_t>(sz)];
}

enum class GameMode : size_t {
    Human,
    Robot
};  // endof enum class GameMode

template <typename... Args>
void log_warn(const char* fmt, Args&&... args);

enum class Cover {
    COVERED = 0,
    REVEALED = 1,
    INVALID = 2
};  // endof enum class Cover

enum class Flag {
    NO_FLAG = 0,
    FLAG = 1,
    INVALID = 2
};  // endof enum class Flag

struct Position {
public:
    Position() = default;
    Position(int r, int c) : row(r), col(c) {}
    Position(const Position& p) : row(p.row), col(p.col) {}
    bool operator==(const Position& p) const {
        return p.row == row && p.col == col;
    }
    bool is_near(const Position& pos) const {
        return std::abs(row - pos.row) <= 1 
            && std::abs(col - pos.col) <= 1;
    }

    virtual bool is_tile() const { return false; }
    virtual bool is_mine() const { return false; }
    virtual void set_num(int n) { log_warn("Set num on a Position object"); }
    virtual int get_num() const { return -1; }
    virtual void set_flag() { log_warn("Set flag on a Position object"); }
    virtual Flag get_flag() const { return Flag::INVALID; }
    virtual void reveal() { log_warn("Reveal a Position object"); }
    virtual Cover get_cover() const { return Cover::INVALID; }

    int row = -1;
    int col = -1;
};  // endof struct Position
struct PositionHash {
    size_t operator()(const Position& pos) const {
        return static_cast<size_t>(pos.row * 26 + pos.col);
    }
};  // endof struct PositionHash
struct PositionEqual {
    size_t operator()(const Position& a, const Position& b) const {
        return a == b;
    }
};  // endof struct PositionEqual

struct PositionPair {
public:
    PositionPair(const Position& a, const Position& b) {
        if (a.row > b.row) {
            p1 = a; p2 = b; return ;
        } else if (a.row < b.row) {
            p1 = b; p2 = a; return ;
        } else if (a.col > b.col) {
            p1 = a; p2 = b; return ;
        } else if (a.col < b.col) {
            p1 = b; p2 = a; return ;
        } else {
            p1 = a; p2 = b; return ;
        }
        p1 = a; p2 = b;
    }
    Position p1;
    Position p2;
};  // endof struct PositionPair

struct PositionPairHash {
    size_t operator()(const PositionPair& pos) const {
        return static_cast<size_t>((pos.p1.row * 26 + pos.p1.col) * 26 + 
                                   (pos.p2.row * 26 + pos.p2.col));
    }
};  // endof struct PositionPairHash
struct PositionPairEqual {
    bool operator()(const PositionPair& pos1, const PositionPair& pos2) const {
        return pos1.p1 == pos2.p1 && pos1.p2 == pos2.p2;
    }
};  // endof struct PositionPairEqual

constexpr int MINE = 9;

struct Tile : public Position {
public:
    Tile() : Position(-1, -1), num(-1), cover(Cover::COVERED), flag(Flag::NO_FLAG) {}
    Tile(const Position& pos, int n) : Position(pos), num(n), cover(Cover::COVERED), flag(Flag::NO_FLAG) {}
    Tile(int r, int c, int n) : Position(r, c), num(n), cover(Cover::COVERED), flag(Flag::NO_FLAG) {}
    Tile(const Position& pos, int n, Cover cover_, Flag flag_) 
        : Position(pos), num(n), cover(cover_), flag(flag_) {}
    Tile(int r, int c, int n, Cover cover_, Flag flag_) 
        : Position(r, c), num(n), cover(cover_), flag(flag_) {}

    virtual void set_num(int n) override {
        num = n;
    }
    virtual int get_num() const override { return num; }
    virtual bool is_tile() const override { return true; }
    virtual bool is_mine() const override {
        if (num == MINE) {
            log_warn("Mine constucted in Tile object");
            return true;
        }
        return false;
    }
    virtual void set_flag() override {
        assert(cover == Cover::COVERED && flag != Flag::INVALID);
        flag == Flag::FLAG ? flag = Flag::NO_FLAG : flag = Flag::FLAG;
    }
    virtual Flag get_flag() const override { return flag; }
    virtual void reveal() override {
        assert(cover != Cover::INVALID);
        cover = Cover::REVEALED;
    }
    virtual Cover get_cover() const override { return cover; }
private:
    int num;
    Cover cover;
    Flag flag;
} ;  // endof struct Tile

struct Mine : public Tile {
public:
    Mine() : Tile(-1, -1, MINE, Cover::COVERED, Flag::NO_FLAG) {}
    Mine(const Position& pos) : Tile(pos, MINE, Cover::COVERED, Flag::NO_FLAG) {}
    Mine(int r, int c) : Tile(r, c, MINE, Cover::COVERED, Flag::NO_FLAG) {}
    Mine(const Position& pos, Cover cover_, Flag flag_) : Tile(pos, MINE, cover_, flag_) {}
    Mine(int r, int c, Cover cover_, Flag flag_) : Tile(r, c, MINE, cover_, flag_) {}

    virtual bool is_tile() const override { return true; }
    virtual bool is_mine() const override { return true; }
    
};  // endof struct Mine

enum class CommandType : size_t {
    REVEAL = 0,
    FLAG = 1,
    RESTART = 2,
    MENU = 3,
    QUIT = 4,
    INVALID = 5,
    XQ4MS = 6
};  // endof enum class CommandType
const std::unordered_map<size_t, std::string> CommandTypeDescription = {
    {0, "REVEAL"}, {1, "FLAG"}, {2, "RESTART"},
    {3, "MENU"}, {4, "QUIT"}, {5, "INVALID"}, {6, "XQ4MS"}
};

struct Command {
public:
    CommandType cmdtype;
    Position pos;
};  // endof struct Command

enum class GameStatus : size_t {
    NORMAL = 0,
    RESTART = 2,
    MENU = 3,
    QUIT = 4,
    INVALID = 5,
    XQ4MS = 6
};  // endof enum class GameStatus
const std::unordered_map<size_t, std::string> GameStatusDescription = {
    {0, "NORMAL"}, {2, "RESTART"}, {3, "MENU"},
    {4, "QUIT"}, {5, "INVALID"}, {6, "XQ4GB"}
};

std::vector<std::pair<int, int>> dirs = {{1, 0}, {0, 1}, {1, 1}, {1, -1},
                                         {-1, 0}, {0, -1}, {-1, -1}, {-1, 1}};
std::vector<std::pair<int, int>> dir_dirs = {{1, 0}, {0, 1}, {-1, 0}, {0, -1}};

inline void append_time_info(std::string& str) {
    time_t now = time(0);
    tm* lt = localtime(&now);
    str += std::to_string(1900 + lt->tm_year);
    str += '-'; str += std::to_string(1 + lt->tm_mon);
    str += '-'; str += std::to_string(lt->tm_mday);
    str += '_'; str += std::to_string(lt->tm_hour);
    str += 'h'; str += std::to_string(lt->tm_min);
    str += 'm'; str += std::to_string(lt->tm_sec); 
    str += 's';
}

inline void tolower(std::string& str) {
    std::transform(str.begin(), str.end(), str.begin(), ::tolower);
}
inline void toupper(std::string& str) {
    std::transform(str.begin(), str.end(), str.begin(), ::toupper);
}

inline bool is_digit(char c) {
    return c <= '9' and c >= '0';
}
inline bool is_uppercase(char c) {
    return c <= 'Z' and c >= 'A';
}
inline bool is_lowercase(char c) {
    return c <= 'z' and c >= 'a';
}

inline BoardSize cmd_get_size_helper() {
    cmd_clear();
    std::cout << HELPER_SELECT_SIZE << "\n";
    std::string boardsize;
    signed char size = -1;
    while (size < 0) {
        std::cin >> boardsize;
        if (boardsize.size() == 0) {
            return BoardSize::Small;
        }
        if (boardsize.size() >= 2) {
            std::cout << HELPER_INVALIDSIZE_1 << "\n";
            size = -1; continue;
        }
        size = boardsize[0];
        if (is_digit(size)) {
            size -= '0';
        } else if (is_uppercase(size)) {
            size -= 'A' - 1;
        } else if (is_lowercase(size)) {
            size -= 'a' - 1;
        } else {
            std::cout << HELPER_INVALIDSIZE_2 << "\n";
            size = -2; continue;
        }
        switch (size) {
            case 1 : return BoardSize::Small;
            case 2 : return BoardSize::Middle;
            case 3 : return BoardSize::Large;
            default : {
                std::cout << HELPER_INVALIDSIZE_3 << "\n";
                size = -3; continue;
            }
        }
    }
    return BoardSize::Large;
}
inline GameMode  cmd_get_mode_helper() {
    cmd_clear();
    std::cout << HELPER_SELECT_MODE << "\n";
    std::string gamemode;
    signed char mode = -1;
    while (mode < 0) {
        std::cin >> gamemode;
        if (gamemode.size() == 0) {
            return GameMode::Human;
        }
        if (gamemode.size() >= 2) {
            std::cout << HELPER_INVALIDMODE_1 << "\n";
            mode = -1; continue;
        }
        mode = gamemode[0];
        if (is_digit(mode)) {
            mode -= '0';
        } else if (is_uppercase(mode)) {
            mode -= 'A' - 1;
        } else if (is_lowercase(mode)) {
            mode -= 'a' - 1;
        } else {
            std::cout << HELPER_INVALIDMODE_2 << "\n";
            mode = -2; continue;
        }
        
        switch (mode) {
            case 1 : return GameMode::Human;
            case 2 : return GameMode::Robot;
            default : {
                std::cout << HELPER_INVALIDMODE_3 << "\n";
                mode = -3; continue;
            }
        }
    }
    
    return GameMode::Robot;
}

inline BoardSize gui_get_size_helper() {
    return BoardSize::Small;
}
inline GameMode  gui_get_mode_helper() {
    return GameMode::Human;
}

}  // endof namespace mfwu

#endif  // __COMMON_HPP__