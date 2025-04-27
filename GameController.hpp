#ifndef __GAMECONTROLLER_HPP__
#define __GAMECONTROLLER_HPP__

#include "common.hpp"
#include "Board.hpp"
#include "Player.hpp"
#include "Displayer.hpp"
#include "Archive.hpp"
#include "Logger.hpp"

namespace mfwu {

class GameController_base {
public:
    virtual GameStatus start() = 0;
    virtual void restart_game_init(GameStatus status) = 0;
    virtual void abrupt_flush(GameStatus status) = 0;

};  // endof class GameController_base

template <typename Board_type, typename Player_type>
class GameController : public GameController_base {
public:
    GameController() 
        : board_(std::make_shared<Board_type>()), 
          player_(std::make_shared<Player_type>(board_)) {
        _gc_init_();
    }  // CHECK
    ~GameController() {}

    GameStatus start() override {
        log_new_game(board_->height(), board_->width());
        this->board_->show();
        CommandType cmd_type;
        this->game_play_task(cmd_type);
        switch (cmd_type) {
        case CommandType::REVEAL : 
        case CommandType::FLAG : {
            return GameStatus::NORMAL;
        } break;
        case CommandType::RESTART : {
            return GameStatus::RESTART;
        } break;
        case CommandType::MENU : {
            return GameStatus::MENU;
        } break;
        case CommandType::QUIT: {
            return GameStatus::QUIT;
        } break;
        default:
            log_error("Game end with cmd type: %lu",
                       static_cast<size_t>(cmd_type));
        }
        return GameStatus::INVALID;
    }

    void restart_game_init(GameStatus status) {
        log_end_game(status);
        archive_.flush(status);
        log_new_game(board_->height(), board_->width());
        board_->reset();
    }

    void abrupt_flush(GameStatus status) {
        log_end_game(status);
        archive_.flush(status);
    }

private:
    void _gc_init_() {
        log_info("game controller inits...");
        if (archive_.get_status() == true) {
            log_debug("archive status: online");
        } else {
            log_error("archive status: offline");
            log_error(XQ4MS_TIMESTAMP, "your archive may be lost");
        }
        log_debug("------------------------------");
    }
    void game_play_task(CommandType& cmd_type) {
        Command cmd;
        int res = 0;
        do {
            cmd = this->advance();
            cmd_type = cmd.cmdtype;
            if (cmd_type != CommandType::REVEAL
                and cmd_type != CommandType::FLAG) {
                if (cmd_type == CommandType::XQ4MS) {
                    // log_new_game();
                    archive_.flush(GameStatus::XQ4MS);
                    execl("./xq4ms", "xq4ms", NULL);
                    exit(0x3F3F3F3F);
                }
                return ;
            }
        } while ((res = check_end(cmd)) == 0);
        board_->winner_display(res);
    }

    Command advance() {
        Command cmd = player_->play();
        CommandType cmd_type = cmd.cmdtype;
        if (cmd_type == CommandType::FLAG
            or cmd_type == CommandType::REVEAL) {
            board_->refresh();
            archive_.record(board_->serialize());
        }
        return cmd;
    }

    int check_end(Command cmd) const {
        if (cmd.cmdtype != CommandType::REVEAL) {
            return 0;
        }
        return board_->is_end(cmd.pos.row, cmd.pos.col);
    }

    std::shared_ptr<Board_base> board_;
    std::shared_ptr<Player> player_;
    Archive<Board_type> archive_;

};  // endof class GameController

}  // endof namespace mfwu

#endif  // __GAMECONTROLLER_HPP__
