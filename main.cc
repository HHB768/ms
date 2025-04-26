// #define __GUI_MODE__
#define __CMD_MODE__

#ifdef __GUI_MODE__
#undef __CMD_MODE__
#endif  // __GUI_MODE__

#include "common.hpp"
#include "GameController.hpp"
using namespace mfwu;

int main() {

    while (true) {
#ifdef __CMD_MODE__
        const BoardSize size = cmd_get_size_helper();
        const GameMode  mode = cmd_get_mode_helper();
#       define __BOARD__ CmdBoard
#else  // __GUI_MODE__
        const BoardSize size = gui_get_size_helper();
        const GameMode  mode = gui_get_mode_helper();
#       define __Board__ GuiBoard
#endif  // __CMD_MODE__

        using __ROBOT__ = DebugRobot;
        
        std::unique_ptr<GameController_base> game = nullptr;

        switch (size) {
        case BoardSize::Small : {
            switch (mode) {
            case GameMode::Human : {
                game = std::make_unique<GameController<__BOARD__<BoardSize::Small>, HumanPlayer>>();
            } break;
            case GameMode::Robot : {
                game = std::make_unique<GameController<__BOARD__<BoardSize::Small>, __ROBOT__>>();
            } break;
            default : 
                gc_error_exit(size, mode);
            }
        } break;
        case BoardSize::Middle : {
            switch (mode) {
            case GameMode::Human : {
                game = std::make_unique<GameController<__BOARD__<BoardSize::Middle>, HumanPlayer>>();
            } break;
            case GameMode::Robot : {
                game = std::make_unique<GameController<__BOARD__<BoardSize::Middle>, __ROBOT__>>();
            } break;
            default : 
                gc_error_exit(size, mode);
            }
        } break;
        case BoardSize::Large : {
            switch (mode) {
            case GameMode::Human : {
                game = std::make_unique<GameController<__BOARD__<BoardSize::Large>, HumanPlayer>>();
            } break;
            case GameMode::Robot : {
                game = std::make_unique<GameController<__BOARD__<BoardSize::Large>, __ROBOT__>>();
            } break;
            default : 
                gc_error_exit(size, mode);
            }
        } break;
        default :
            gc_error_exit(size, mode);
        }

        bool ret2menu_flag = false;
        while (!ret2menu_flag) {
            GameStatus status = game->start();
            switch (status) {
            case GameStatus::RESTART : {
                game->restart_game_init(status);
            } break;
            case GameStatus::NORMAL : {
                std::cout << HELPER_PRESS_ANY_KEY << "\n";
                sleep(1);  // TODO: del this after transfering to win platf
                fgetc(stdin);
                game->restart_game_init(status);
            } break;
            case GameStatus::MENU : {
                ret2menu_flag = true;
                game->abrupt_flush(status);
                break;
            } break;
            case GameStatus::QUIT : {
                game->abrupt_flush(status);
                exit(0);
            } break;
            default:
                game->abrupt_flush(status);
                logerr_unknown_game_status();
                exit(-1);
            }
        }
    }
    
    return 0;
}