#ifndef __BOARD_HPP__
#define __BOARD_HPP__

#include "common.hpp"
#include "Displayer.hpp"
#include "Logger.hpp"

namespace mfwu {

class Board_base {
public:
    Board_base() {}
    virtual ~Board_base() {}

    virtual Command get_command() = 0;
    virtual void show() const = 0;
    virtual void show_mine_num() const = 0;
    virtual void show_without_log() const = 0;
    virtual void refresh() = 0;
    virtual void update(const Command& cmd) = 0;
    virtual bool is_valid(int row, int col) const = 0;

    virtual size_t height() const = 0;
    virtual size_t width() const = 0;

    virtual std::shared_ptr<const Position> get_pos(int row, int col) const = 0;
    virtual bool all_clear(const PositionPair& pp) const = 0;

protected:
    Tile last_tile_;
    bool status_;
};  // endof class Board_base


template <BoardSize Size=BoardSize::Small>
class Board : public Board_base {
public:
    static constexpr BoardDimension dims = get_board_dimension(Size);
    static constexpr size_t height_ = dims.height;
    static constexpr size_t width_  = dims.width;
    size_t height() const override { return height_; }
    size_t width() const override { return width_; }
    static constexpr size_t num_of_tile_ = width_ * height_;
    static constexpr size_t num_of_mine_ = num_of_tile_ * MINE_POS_RATIO;

    Board() : board_() {
        _init_resize();
        _init_board();        
    }
    Board(const std::vector<std::vector<bool>>& mines_pos) {
        _init_resize();
        _init_board(mines_pos);
    }
    Board(const Board& board) = default;
    Board(Board&& board) = default;
    Board& operator=(const Board& board) = default;
    Board& operator=(Board&& board) = default;

    virtual ~Board() {}

    virtual void reset() {
        this->_init_board();
        mine_count_down_ = num_of_mine_;
        tile_count_down_ = num_of_tile_ - num_of_mine_;
    }

    void update(const Command& cmd) override {
        int row = cmd.pos.row, col = cmd.pos.col;
        if (cmd.cmdtype == CommandType::FLAG) {
            assert(board_[row][col]->get_cover() == Cover::COVERED);
            board_[row][col]->set_flag();
            if (board_[row][col]->get_flag() == Flag::FLAG) {
                mine_count_down_--;
            } else if (board_[row][col]->get_flag() == Flag::NO_FLAG) {
                mine_count_down_++;
            } else {
                log_warn("Invalid flag status, cannot be flagged");
                return ;
            }
            if (board_[row][col]->is_mine()) {
                log_debug("Flag a real mine");
            } else {
                log_debug("Flag a tile without mine");
            }
        } else if (cmd.cmdtype == CommandType::REVEAL) {
            std::unordered_set<Position, PositionHash, PositionEqual> found;
            reveal(cmd.pos, found);
        }
    }

    // TODO: return enum
    int is_end(int row, int col) {
        if (board_[row][col]->is_mine() 
            && board_[row][col]->get_cover() == Cover::REVEALED) { return 1; }  // failures
        if (tile_count_down_ == 0) { return 2; }  // victory
        return 0;  // unfinished
    }

    static size_t get_height() {
        return height_;
    }
    static size_t get_width() {
        return width_;
    }

    std::shared_ptr<const Position> get_pos(int row, int col) const override {
        return board_[row][col];
    }
    
    virtual void winner_display(int res) {
        if (res == 1) {
            cmd_clear();
            std::cout << "failure\n";
        } else if (res == 2) {
            cmd_clear();
            std::cout << "victory\n";
        } else {
            assert(1 == 0);
        }
    }
    
    std::string serialize() { return {}; }

protected:
    std::vector<std::vector<std::shared_ptr<Position>>> board_;
    int mine_count_down_ = num_of_mine_;
    int tile_count_down_ = num_of_tile_ - num_of_mine_;
    

private:
    void _init_resize() {
        board_.resize(height_);
        for (int i = 0; i < height_; i++) {
            board_[i].resize(width_);
        }
    }
    void _init_board() {
        init_mines();
        init_tile_num();
    }
    void _init_board(const std::vector<std::vector<bool>>& mines_pos) {
        init_mines(mines_pos);
        init_tile_num();
    }
    void init_mines() {
        // randomly mining
        // 不重复的随机序列
        std::unordered_set<size_t> mines_pos = get_random_mines();
        std::vector<std::vector<size_t>> mines(height_, std::vector<size_t>(width_, 0x0));
        for (int i = 0; i < height_; i++) {
            for (int j = 0; j < width_; j++) {
                if (mines_pos.count(i * width_ + j)) {
                    board_[i][j] = std::make_shared<Mine>(i, j, Cover::COVERED, Flag::NO_FLAG);
                    mines[i][j] = 9;
                } else {
                    board_[i][j] = std::make_shared<Tile>(i, j, 0, Cover::COVERED, Flag::NO_FLAG);
                }
            }
        }
        CmdDisplayer<Size> mine_displayer(mines);
        mine_displayer.log("Mines: ");
    }
    void init_mines(const std::vector<std::vector<bool>>& mines_pos) {
        for (int i = 0; i < height_; i++) {
            for (int j = 0; j < width_; j++) {
                if (mines_pos[i][j] == true) {
                    board_[i][j] = std::make_shared<Mine>(i, j, Cover::COVERED, Flag::NO_FLAG);
                } else {
                    board_[i][j] = std::make_shared<Tile>(i, j, 0, Cover::COVERED, Flag::NO_FLAG);
                }
            }
        }
    }
    void init_tile_num() {
        for (int i = 0; i < height_; i++) {
            for (int j = 0; j < width_; j++) {
                if (!board_[i][j]->is_mine()) {
                    int num = count_mine_num(i, j);
                    board_[i][j]->set_num(num);
                }
            }
        }
    }
    std::unordered_set<size_t> get_random_mines() {
        std::vector<size_t> vec(num_of_tile_);
        for (int i = 0; i < num_of_tile_; i++) {
            vec[i] = i;
        }
        for (int i = 0; i < num_of_tile_; i++) {
            int r = rand() % num_of_tile_;
            std::swap(vec[i], vec[r]);
        }
        return std::unordered_set<size_t>(vec.begin(), vec.begin() + num_of_mine_);
    }

    static bool is_valid_pos(int i, int j) {
        return i >= 0 && i < height_ && j >= 0 && j < width_;
    }
    int count_mine_num(int row, int col) {
        int res = 0;
        for (auto&& [inc_r, inc_c] : dirs) {
            int cur_r = row + inc_r;
            int cur_c = col + inc_c;
            if (!is_valid_pos(cur_r, cur_c)) { continue; }
            if (board_[cur_r][cur_c]->is_mine()) {
                res++;
            }
        }
        return res;
    }

    void reveal(const Position& pos, std::unordered_set<Position, PositionHash, PositionEqual>& found) {
        if (board_[pos.row][pos.col]->get_cover() == Cover::REVEALED) return ;
        if (found.count(pos)) { return ; }
        else { found.insert(pos); }
        board_[pos.row][pos.col]->reveal();
        if (!board_[pos.row][pos.col]->is_mine()) {
            tile_count_down_--;
            if (board_[pos.row][pos.col]->get_num() == 0) {
                for (auto&& [inc_r, inc_c] : dirs) {
                    int cur_r = pos.row + inc_r,
                        cur_c = pos.col + inc_c;
                    if (!is_valid(cur_r, cur_c)) { continue; }
                    assert(!board_[cur_r][cur_c]->is_mine());
                    reveal({cur_r, cur_c}, found);
                }
            }
        } else {
            log_debug("Reveal a mine");
        }
    }

    bool is_valid(int row, int col) const override {
        return row >= 0 && row < height_
               && col >= 0 && col < width_;
    }

    bool all_clear(const PositionPair& pp) const {
        for (auto&& [inc_r, inc_c] : dirs) {
            int cur_r = pp.p1.row + inc_r,
                cur_c = pp.p1.col + inc_c;
            if (!is_valid(cur_r, cur_c)) { continue; }
            if (this->board_[cur_r][cur_c]->get_cover() == Cover::COVERED) { return false; }
        }
        for (auto&& [inc_r, inc_c] : dirs) {
            int cur_r = pp.p2.row + inc_r,
                cur_c = pp.p2.col + inc_c;
            if (!is_valid(cur_r, cur_c)) { continue; }
            if (this->board_[cur_r][cur_c]->get_cover() == Cover::COVERED) { return false; }
        }
        return true;
    }
};  // endof class Board

template <BoardSize Size=BoardSize::Small>
class CmdBoard : public Board<Size> {
public:
    using base_type = Board<Size>;
    static constexpr size_t height_ = base_type::height_;
    static constexpr size_t width_  = base_type::width_; 
    CmdBoard() : Board<Size>(), displayer_() {}
    CmdBoard(const std::vector<std::vector<bool>>& mines_pos) 
        : Board<Size>(mines_pos),
          displayer_(this->board_) {}
    CmdBoard(const CmdBoard& board) = default;
    CmdBoard(CmdBoard&& board) = default;
    CmdBoard& operator=(const CmdBoard& board) = default;
    CmdBoard& operator=(CmdBoard&& board) = default;

    virtual void reset() override {
        base_type::reset();
        displayer_.load_new_board();
    }

    void update(const Command& cmd) override {
        // rm_last_sp();
        update_new_tile(cmd);
    }
    Command get_command() override {
        std::string input_str;
        std::cout << HELPER_RETURN2MENU << "\n";
        std::cout << HELPER_PLACE_TILE << "\n";
        std::cin >> input_str;
        Command ret = CmdBoard::validate_input(input_str);
        if (ret.cmdtype == CommandType::INVALID
            || ((ret.cmdtype == CommandType::REVEAL || 
                 ret.cmdtype == CommandType::FLAG)
                && (ret.pos.row < 0 or ret.pos.col < 0))) {
            std::cout << HELPER_INVALID_POSITION << "\n";
            ret = this->get_command();
        }
        return ret;
    }
    void show() const override {
        cmd_clear();
        show_board();
        show_mine_num();
    }
    void show_without_log() const override {
        cmd_clear();
        show_board_without_log();
        show_mine_num();
    }
    void refresh() override {
        show();
    }

    void show_mine_num() const override {
        std::cout << "Mine num: " << this->mine_count_down_ << "\n";
        log_debug("Mine num: %d", this->mine_count_down_);
    }
    // void winner_display() override {
    //     // TODO
    // }
private:
    std::vector<std::vector<size_t>> snap() {
        std::vector<std::vector<size_t>> ret(
            height_, std::vector<size_t>(width_)
        );
        for (int i = 0; i < height_; i++) {
            for (int j = 0; j < width_; j++) {
                if (this->board_[i][j]->get_cover() == Cover::COVERED) {
                    if (this->board_[i][j]->get_flag() == Flag::FLAG) {
                        ret[i][j] = 0xF; continue;
                    } else if (this->board_[i][j]->get_flag() == Flag::NO_FLAG) {
                        ret[i][j] = 0xA; continue;
                    }  
                } else if (0 <= this->board_[i][j]->get_num()
                        && 9 >= this->board_[i][j]->get_num()) {
                    ret[i][j] = this->board_[i][j]->get_num(); continue;
                }
                // invalid
                ret[i][j] = 0x3F;
            }
        }
        return ret;
    }
    void update_new_tile(const Command& cmd) {
        Board<Size>::update(cmd);
        // displayer_.update_new_tile(cmd);
        // 由于update可能会调用自己，displayer直接读取新结果
        displayer_.update_new_tile(this->snap());
    }
    Command validate_input(const std::string& rstr) {
        if (rstr.size() > 10) return Command{CommandType::INVALID, {}};
        std::string str = rstr;
        toupper(str);
        if (str == std::string(QUIT_CMD1)
            || str == std::string(QUIT_CMD2)) {
            return Command{CommandType::QUIT, {}};
        } else if (str == std::string(MENU_CMD1)
            || str == std::string(MENU_CMD2)) {
            return Command{CommandType::MENU, {}};
        } else if (str == std::string(RESTART_CMD1)
            || str == std::string(RESTART_CMD2)) {
            return Command{CommandType::RESTART, {}};
        } else if (str == std::string(XQ4MS_CMD)) {
            return Command{CommandType::XQ4MS, {}};
        }

        if (str.size() != 3 or (str[0] != 'R' && str[0] != 'F' && str[0] != 'A'))  {
            return Command{CommandType::INVALID, {}};
        }
        CommandType cmdtype = str[0] == 'F' 
                              ? CommandType::FLAG
                              : CommandType::REVEAL;
        Position pos = {get_row(str[1]), get_col(str[2])};
        auto ret = Command{cmdtype, pos};
        if (pos.row == -1 or pos.col == -1) {
            return ret;
        }
        if ((cmdtype == CommandType::REVEAL || cmdtype == CommandType::FLAG)
            && this->board_[pos.row][pos.col]->get_cover() == Cover::REVEALED) {
            ret.pos.row = ret.pos.col = -1;
            std::cout << HELPER_REVEALED_POSITION << "\n";
        }
        return ret;
    }
    static int get_row(char c) {
        int res = get_int(c);
        if (res >= height_) { return -1; }
        return res;
    }
    static int get_col(char c) {
        int res = get_int(c);
        if (res >= width_) { return -1; }
        return res;
    }
    static int get_int(char c) {
        if (is_lowercase(c)) {
            return c - 'a';
        } else if (is_uppercase(c)) {
            return c - 'A';
        } 
        return -1;
    }
    
    void show_board() const {
        displayer_.show();
    }
    void show_board_without_log() const {
        displayer_.show_without_log();
    }
    CmdDisplayer<Size> displayer_;
};  // endof class CmdBoard

template <BoardSize Size=BoardSize::Small>
class GuiBoard : public Board<Size> {
public:
    GuiBoard() : Board<Size>() {}
    GuiBoard(const std::vector<std::vector<bool>>& mines_pos) : Board<Size>(mines_pos) {}
private:
    GuiDisplayer<Size> displayer_;
};  // endof class GuiBoard

}  // endof namespace mfwu

#endif  // __BOARD_HPP__