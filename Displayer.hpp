#ifndef __DISPLAYER_HPP__
#define __DISPLAYER_HPP__

#include "Logger.hpp"

namespace mfwu {

class Displayer_base {
public:
    static constexpr const char empty_position_char = ' ';
    static constexpr const char unrevealed_tile_char = '+';
    static constexpr const char flagged_tile_char = 'F';
    static constexpr const char mine_char = 'X';
    static constexpr const char unknown_status_tile_char = '!';
    static constexpr const char inner_border_char = ' ';
    static constexpr const char outer_border_char = '.';
    static constexpr const char highlight_left_char = '[';
    static constexpr const char highlight_right_char = ']';
    
    virtual const std::vector<std::string>& get_framework() const = 0;
    // virtual void unzip_tbl(const std::string_view& str, bool mode) = 0;
};  // endof class Displayer_base

#define DEFINE_SHAPES \
static constexpr const char empty_position_char = base_type::empty_position_char;\
static constexpr const char unrevealed_tile_char = base_type::unrevealed_tile_char;\
static constexpr const char flagged_tile_char = base_type::flagged_tile_char;\
static constexpr const char mine_char = base_type::mine_char;\
static constexpr const char unknown_status_tile_char = base_type::unknown_status_tile_char;\
static constexpr const char inner_border_char = base_type::inner_border_char;\
static constexpr const char outer_border_char = base_type::outer_border_char;\
static constexpr const char highlight_left_char = base_type::highlight_left_char;\
static constexpr const char highlight_right_char = base_type::highlight_right_char;

template <BoardSize Size>
class Displayer : public Displayer_base {
    /*  
        Mode = true:
        board: 2 * 3
            A B C 
          . . . . .
        A . O + X .
        B . + O + .
          . . . . .

        framework:   row : 1 + 1 + 2 * 1 + 1 = 1 * (3 + 2)
                     col : 2 + 2 + 3 * 2 + 1 = 2 * (3 + 3)

        [i, j] -> [2 + 1 * i, 4 + 2 * j]
    */
public:
    static constexpr BoardDimension dims = get_board_dimension(Size);
    static constexpr size_t height_ = dims.height;
    static constexpr size_t width_  = dims.width;
    static constexpr size_t num_of_tile_ = width_ * height_;
    static constexpr size_t num_of_mine_ = num_of_tile_ * MINE_POS_RATIO;

    static constexpr size_t frwk_height_ = height_ + 3;
    static constexpr size_t frwk_width_  = 2 * (3 + width_);
    using base_type = Displayer_base;
    DEFINE_SHAPES;

    Displayer() : framework_(frwk_height_, std::string(frwk_width_, inner_border_char)) {
        _init_framework();
        // load_empty_board();  // 和gb不同，这里不能这样初始化，因为初始的displayer并不该是empty，而是covered
        load_new_board();
    }
    Displayer(const std::vector<std::vector<size_t>>& board_)
        : framework_(frwk_height_, std::string(frwk_width_, inner_border_char)) {
        _init_framework();
        reconstruct(board_);
    }

    const std::vector<std::string>& get_framework() const override {
        return framework_;
    }

    static size_t get_row_in_framework(int r) {
        return 2 + r;
    }
    static size_t get_col_in_framework(int c) {
        return 2 * (2 + c);
    }
    char& get_pos_ref_in_framework(int r, int c) {
        return framework_[get_row_in_framework(r)][get_col_in_framework(c)];
    }
    static std::pair<size_t, size_t> get_pos_in_framework(int r, int c) {
        return {get_row_in_framework(r), get_col_in_framework(c)};
    }

    void print_empty_position(int r, int c) {
        get_pos_ref_in_framework(r, c) = empty_position_char;
    }
    void print_unrevealed_tile(int r, int c) {
        get_pos_ref_in_framework(r, c) = unrevealed_tile_char;
    }
    void print_flagged_tile(int r, int c) {
        get_pos_ref_in_framework(r, c) = flagged_tile_char;
    }
    void print_mine(int r, int c) {
        get_pos_ref_in_framework(r, c) = mine_char;
    }
    void print_unknown_status_tile(int r, int c) {
        get_pos_ref_in_framework(r, c) = unknown_status_tile_char;
    }
    void print_number(int r, int c, size_t status) {
        assert(1 <= status && status << 8);
        get_pos_ref_in_framework(r, c) = '0' + status;
    }

    // virtual void load_empty_board() {
    //     for (int i = 0; i < height_; i++) {
    //         for (int j = 0; j < width_; j++) {
    //             print_empty_position(i, j);
    //         }
    //     }
    // }
    virtual void load_new_board() {
        for (int i = 0; i < height_; i++) {
            for (int j = 0; j < width_; j++) {
                print_unrevealed_tile(i, j);
            }
        }
    }
    virtual void show() const {
        std::stringstream ss;
        log_debug("Board: ");
        for (const std::string& line : this->framework_) {
            ss << line << "\n";
            log_debug(XQ4MS_TIMESTAMP, line.c_str());
        }
        std::cout << ss.str();
    }
    virtual void show_without_log() const {
        std::stringstream ss;
        for (const std::string& line : this->framework_) {
            ss << line << "\n";
        }
        std::cout << ss.str();
    }
    void log(std::string name="Board: ") const {
        log_debug("%s", name.c_str());
        for (const std::string& line : this->framework_) {
            log_debug(XQ4MS_TIMESTAMP, line.c_str());
        }
    }
    //
    // virtual void reveal_or_flag(Tile last_tile) {
    //     add_sp(last)
    // }

protected:
    void reconstruct(const std::vector<std::vector<size_t>>& board_) {
        for (int i = 0; i < height_; i++) {
            for (int j = 0; j < width_; j++) {
                update_directly(i, j, board_[i][j]);
            }
        }
    }
    void update_directly(int i, int j, size_t status) {
        switch (status) {
            // number     : 0 ~ 8
            // mine       : 9
            // unrevealed : 10
            // flag       : 15
            
        case 0 : {
            print_empty_position(i, j);
        } break;
        case 1 : case 2 : case 3 : 
        case 4 : case 5 : case 6 : case 7 :
        case 8 : {
            print_number(i, j, status);
        } break;
        case 9 : {
            print_mine(i, j);
        } break;
        case 0xA : {
            print_unrevealed_tile(i, j);
        } break;
        case 0xF : {
            print_flagged_tile(i, j);
        } break;
        default : 
            log_error("In %s, %s", __FILE__, __LINE__);
            logerr_unknown_tile_status();
            print_unknown_status_tile(i, j);
        }
    }

    virtual void remove_highlight() {
        for (std::string& line : this->framework_) {
            for (char& c : line) {
                if (c == highlight_left_char 
                    or c == highlight_right_char) {
                    c = inner_border_char;
                }
            }
        }
    }
    virtual void remove_highlight(int r, int c) {
        auto [row, col] = get_pos_in_framework(r, c);
        char* ch = &this->framework_[row][col - 1];
        if (*ch == highlight_left_char) {
            *ch = inner_border_char;
        }
        ch = &this->framework_[row][col + 1];
        if (*ch == highlight_right_char) {
            *ch = inner_border_char;
        }
    }
    virtual void add_highlight(int r, int c) {
        // assert(...)
        auto [row, col] = get_pos_in_framework(r, c);
        // framework_[row - 1][col] = highlight_up_down_char;
        // framework_[row + 1][col] = highlight_up_down_char;
        this->framework_[row][col - 1] = highlight_left_char;
        this->framework_[row][col + 1] = highlight_right_char;
    }


    std::vector<std::string> framework_;

private:
    void _init_framework() {
        for (int j = 0; j < width_; j++) {
            framework_[0][get_col_in_framework(j)] = 'A' + j;
        }
        for (int i = 0; i < height_; i++) {
            framework_[get_row_in_framework(i)][0] = 'A' + i;
        }
        for (int j = 0; j <= width_; j++) {
            get_pos_ref_in_framework(-1, j) = outer_border_char;
        }
        for (int j = 0; j <= width_; j++) {
            get_pos_ref_in_framework((int)height_, j) = outer_border_char;
        }
        for (int i = 0; i <= height_; i++) {
            get_pos_ref_in_framework(i, -1) = outer_border_char;
        }
        for (int i = 0; i <= height_; i++) {
            get_pos_ref_in_framework(i, (int)width_) = outer_border_char;
        }
        get_pos_ref_in_framework(-1, -1) = outer_border_char;
    }

    void advance(int& i, int& j) {
        if (j < width_ - 1) {
            j++;
        } else {
            i++; j = 0;
        }
    }

    mutable bool zip_mode_ = true;

};  // endof class Displayer

#define DEFINE_SIZES \
static constexpr size_t height_ = base_type::height_;\
static constexpr size_t width_  = base_type::width_;\
static constexpr size_t frwk_height_ = base_type::frwk_height_;\
static constexpr size_t frwk_width_  = base_type::frwk_width_;

template <BoardSize Size>
class CmdDisplayer : public Displayer<Size> {
public:
    using base_type = Displayer<Size>;
    DEFINE_SHAPES; DEFINE_SIZES;

    CmdDisplayer() : base_type() {}
    CmdDisplayer(const std::vector<std::vector<size_t>>& board) : base_type(board) {}
    // TODO: BUG 25.04.22

    void update_new_tile(const std::vector<std::vector<size_t>>& board) {
        this->reconstruct(board);
    }

};  // endof class CmdDisplayer

template <BoardSize Size>
class GuiDisplayer : public Displayer<Size> {
    
};  // endof class GuiDisplayer

}  // endof namespace mfwu

#endif  // __DISPLAYER_HPP__