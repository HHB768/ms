#ifndef __PLAYER_HPP__
#define __PLAYER_HPP__

#include "Board.hpp"

namespace mfwu {

class Player {
public:
    Player() : board_(nullptr) {}
    Player(std::shared_ptr<Board_base> board) : board_(board) {}

    virtual Command play() = 0;
    virtual void place(const Command& cmd) {
        this->board_->update(cmd);
    }

    virtual void reset() {}

protected:
    // Position last_pos_;  // place之后直接更新也不错，就不需要下一回合查看上一回合的反馈了
    // 有一说一这个挺麻烦的，因为这一轮
    std::shared_ptr<Board_base> board_;
};  // endof class Player


class HumanPlayer : public Player {
public:
    HumanPlayer() : Player() {}
    HumanPlayer(std::shared_ptr<Board_base> board) : Player(board) {}

    virtual Command play() override {
        Command cmd = this->board_->get_command();
        log_debug("Human player puts cmd: %s",
                  CommandTypeDescription.at(static_cast<size_t>(cmd.cmdtype)).c_str());
        switch (cmd.cmdtype) {
        case CommandType::REVEAL :
        case CommandType::FLAG : {
            this->place(cmd);
            log_debug("Human's pos: [%d, %d]", cmd.pos.row, cmd.pos.col);
        } break;
        case CommandType::RESTART :
        case CommandType::MENU :
        case CommandType::QUIT : {
        } break;
        case CommandType::XQ4MS : {
            log_info(                 "XQ41-MS cheater begins...");
            log_info(XQ4MS_TIMESTAMP, ">>>>>>>>>>>>>>>>>>>>>>>>>");
            log_info(XQ4MS_TIMESTAMP, ":D :) XD TT >.< -^- (: zz");
            sleep(2);
        } break;
        default : 
            logerr_unknown_cmdtype();
        }
        return cmd;
    }

    void place(const Command& cmd) {
        return Player::place(cmd);
    }

};  // endof class HumanPlayer


class RobotPlayer : public Player {
public:
    RobotPlayer() : Player() {}
    RobotPlayer(std::shared_ptr<Board_base> board) : Player(board) {}

    virtual Command play() override {
        Command cmd = this->get_best_cmd();
        if (cmd.cmdtype != CommandType::FLAG
            && cmd.cmdtype != CommandType::REVEAL) {
            log_info("Invalid cmd type returned from get_best_cmd()");
            return Command{CommandType::INVALID, {}};
        }
        if (cmd.pos.row < 0 or cmd.pos.col < 0) {
            return Command{CommandType::INVALID, {}};
        }
        log_info("Robot's cmd: %s", 
                 CommandTypeDescription.at(static_cast<size_t>(cmd.cmdtype)).c_str());
        log_info(XQ4MS_TIMESTAMP, "Robot's pos: [%d, %d]", 
                 cmd.pos.row, cmd.pos.col);
        this->place(cmd);
        return cmd;
    }

    void place(const Command& cmd) override {
        Player::place(cmd);
    }

protected:
    virtual Command get_best_cmd() = 0;
};  // endof class RobotPlayer


class DebugRobot : public RobotPlayer {
public:
    DebugRobot() : RobotPlayer() {}
    DebugRobot(std::shared_ptr<Board_base> board) : RobotPlayer(board) {}
private:
    Command get_best_cmd() override {
        size_t height = this->board_->height();
        size_t width = this->board_->width();

        int row = -1, col = -1;
        while (is_valid(row, col) == false) {
            row = rand() % height;
            col = rand() % width;
        }
        sleep(1);
        return {CommandType::REVEAL, {row, col}};
    }

    bool is_valid(int row, int col) const {
        if (row < 0 || row >= this->board_->height()) return false;
        if (col < 0 || col >= this->board_->width()) return false;
        if (this->board_->get_pos(row, col)
            ->get_cover() == Cover::REVEALED) return false;
        return true;
    }
};  // endof class DebugRobot

class HumanLikeRobot : public RobotPlayer {
public:
    HumanLikeRobot() : RobotPlayer() {}
    HumanLikeRobot(std::shared_ptr<Board_base> board) : RobotPlayer(board) {}
private:
    void reset() override {
        is_in_opening_ = true;
        cmd_queue_.clear();
        std::queue<PositionPair> empty_queue{};
        std::swap(check_queue_, empty_queue);
        queue_menbers_.clear();
        all_possible_pairs_.clear();
    }

    Command play() override {
        Command cmd = {CommandType::INVALID, {}};
        if (is_in_opening_) {
            if (is_good_opening()) {
                is_in_opening_ = false;
                return this->play();
            }
            log_info("Robot randomly reveals:");
            cmd = this->randomly_reveal();
            if (cmd.cmdtype != CommandType::REVEAL) {
                log_info("Invalid cmd type returned from randomly_reveal()");
                return Command{CommandType::INVALID, {}};
            }
            if (cmd.pos.row < 0 or cmd.pos.col < 0) {
                return Command{CommandType::INVALID, {}};
            }
            log_info("Robot's cmd: %s", 
                    CommandTypeDescription.at(static_cast<size_t>(cmd.cmdtype)).c_str());
            log_info(XQ4MS_TIMESTAMP, "Robot's pos: [%d, %d]", 
                    cmd.pos.row, cmd.pos.col);
            this->place(cmd);
        } else {
            cmd = RobotPlayer::play();
        }
        if (cmd.cmdtype == CommandType::REVEAL 
            and this->board_->get_pos(cmd.pos.row, cmd.pos.col)->get_cover() == Cover::REVEALED
            and this->board_->get_pos(cmd.pos.row, cmd.pos.col)->get_num() == 0) {
            std::unordered_set<Position, PositionHash, PositionEqual> found;
            recursive_update_deduction(cmd.pos, found);
        } else {
            update_deduction(cmd.pos);
        }
        return cmd;
    }

    void recursive_update_deduction(const Position& pos, 
        std::unordered_set<Position, PositionHash, PositionEqual>& found) {
        assert(this->board_->get_pos(pos.row, pos.col)->get_cover() == Cover::REVEALED);
        update_deduction(pos);
        if (this->board_->get_pos(pos.row, pos.col)->get_num() == 0) {
            for (auto&& [inc_r, inc_c] : dirs) {
                int cur_r = pos.row + inc_r,
                    cur_c = pos.col + inc_c;
                if (!this->board_->is_valid(cur_r, cur_c)) { continue; }
                assert(this->board_->get_pos(cur_r, cur_c)->get_cover() == Cover::REVEALED);
                Position p = {cur_r, cur_c};
                if (!found.count(p)) {
                    found.insert(p);
                    recursive_update_deduction(p, found);
                }
            }
        }
    }
    
    bool is_good_opening() const {
        size_t height = this->board_->height();
        size_t width  = this->board_->width();
        int revealed_tile_num = 0;
        for (int i = 0; i < height; i++) {
            for (int j = 0; j < width; j++) {
                if (this->board_->get_pos(i, j)->get_cover() == Cover::REVEALED) {
                    revealed_tile_num++;
                }
            }
        }
        return revealed_tile_num > 30 or (float)revealed_tile_num / (height * width) > 0.1F; 
    }
    Command randomly_reveal() const {
        int row = -1, col = -1;
        while ((row < 0 || row >= this->board_->height()) 
            or (col < 0 || col >= this->board_->width()) 
            or (this->board_->get_pos(row, col)->get_cover() == Cover::REVEALED)
            or (this->board_->get_pos(row, col)->get_flag() == Flag::FLAG)) {
            row = rand() % this->board_->height();
            col = rand() % this->board_->width();
        }
        // sleep(1);
        return {CommandType::REVEAL, {row, col}};
    }
    bool is_covered(const Position& pos) const {
        return this->board_->get_pos(pos.row, pos.col)->get_cover() == Cover::COVERED;
    }
    void record(const PositionPair& pp) {
        if (is_covered(pp.p1) || is_covered(pp.p2)) return ;
        if (this->board_->all_clear(pp)) return ;
        if (!queue_menbers_.count(pp)) {
            queue_menbers_.insert(pp);
            check_queue_.emplace(pp);
        }
        if (!all_possible_pairs_.count(pp)) {
            all_possible_pairs_.emplace(pp);
        }
    }
    void init_deduction() {
        size_t height = this->board_->height();
        size_t width  = this->board_->width();
        for (int i = 0; i < height; i++) {
            for (int j = 0; j < width; j++) {
                if (this->board_->get_pos(i, j)->get_cover() == Cover::REVEALED) {
                    record({{i, j}, {i, j}});
                    for (auto&& [inc_r, inc_c] : ldirs) {  //
                        int cur_r = i + inc_r,
                            cur_c = j + inc_c;
                        if (!this->board_->is_valid(cur_r, cur_c)) { continue; }
                        record({{i, j}, {cur_r, cur_c}});
                    }
                }
            }
        }
    }
    void update_deduction(const Position& pos) {
        record({pos, pos});
        for (auto&& [inc_r, inc_c] : dirs) {
            int cur_r = pos.row + inc_r,
                cur_c = pos.col + inc_c;
            if (!this->board_->is_valid(cur_r, cur_c)) { continue; }
            record({{cur_r, cur_c}, {cur_r, cur_c}});
            record({pos, {cur_r, cur_c}});
            for (auto&& [inc_r1, inc_c1] : dirs) {
                int cur_r1 = cur_r + inc_r1,
                    cur_c1 = cur_c + inc_c1;
                if (!this->board_->is_valid(cur_r1, cur_c1)) { continue; }
                record({{cur_r, cur_c}, {cur_r1, cur_c1}});
                record({pos, {cur_r1, cur_c1}});
            }
        }
    }

    Command get_best_cmd() override {
        Command cmd = {CommandType::INVALID, {}};
        if (!cmd_queue_.empty()) {
            cmd = std::move(cmd_queue_.back());
            cmd_queue_.pop_back();
        } else {
            size_t height = this->board_->height();
            size_t width  = this->board_->width();
            if (!check_queue_.empty()) {
                // 查询是否有能确定的，有则直接 return，如果没有确定的，挑可能性最高的 return 
                float max_p = 0.0F;
                bool opt_flag = false;
                // Command cmp_tmp = {CommandType::INVALID, {}};
                while (!check_queue_.empty()) {
                    PositionPair pp = check_queue_.front();
                    // check_queue_.pop();  // 不要在这里 pop
                    if (this->board_->all_clear(pp)) {
                        all_possible_pairs_.erase(pp);
                        queue_menbers_.erase(pp);
                        check_queue_.pop();
                    } else {
                        std::pair<float, Command> res = calc_prob(pp);
                        if (res.first + eps >= 1.0F) {
                            cmd = res.second;
                            opt_flag = true;
                            // 有确定的先不 pop，再查一遍
                            // 有了cmd_queue以后好像没有必要了
                            // 但又好像还有点必要，因为他们动了以后pp包受影响的，后面又会加回来，没必要pop了
                            break;
                        } 
                        if (max_p < res.first) {
                            max_p = res.first;
                            cmd = res.second;
                        }
                        queue_menbers_.erase(pp);
                        check_queue_.pop();
                    }
                }
                if (!is_once_ && !opt_flag) {
                    if (!all_possible_pairs_.empty()) {
                        // 遍历所有可能的 PositionPair，插入到待检查序列中
                        // 还有一种可能，cmd点开的是 0，应当检查这一片连通域的结果  // play 里面检查过了
                        // 有没有可能， all_possible_pairs 里面有确定解，但不在check_queue里面，导致忽略了
                        // 搬过来试试（暂时没有发现，先注释掉，后面发现了再说）
                        // 不过我发现，all_possible_pairs_好像没有单独出现的地方，也就是它好像没有用 5.6
                        // 还是有用的，可能 check_queue里面有些内容一开始是不可解的，但后面时机成熟了已经被pop掉了
                        // 另外，用在调试时也很不错，方便查看有什么pair可以考虑
                        for (auto&& pp : all_possible_pairs_) {
                            check_queue_.emplace(pp);
                        }
                        queue_menbers_ = all_possible_pairs_;
                        cmd = get_best_cmd_once();
                    }
                    if (check_queue_.empty()) {
                        log_info("Uncertain next move, asking for human intervention");
                        // debug
                        // for (auto&& pp : all_possible_pairs_) {
                        //     std::cout << "[" << pp.p1.row << ", " << pp.p1.col << "]"
                        //               << " [" << pp.p2.row << ", " << pp.p2.col << "]" << "\n";
                        // }
                        cmd = this->board_->get_command();
                        log_info("Command type: %s, pos: [%d, %d]", 
                            CommandTypeDescription.at(static_cast<size_t>(cmd.cmdtype)).c_str(), 
                            cmd.pos.row, cmd.pos.col);
                    }
                }
                assert(cmd.cmdtype != CommandType::INVALID);
            } else if (!is_once_ && !all_possible_pairs_.empty()) {
                // 可能 all_possible_pairs 里面有确定解，但不在check_queue里面，导致忽略了
                for (auto&& pp : all_possible_pairs_) {
                    check_queue_.emplace(pp);
                }
                queue_menbers_ = all_possible_pairs_;
                cmd = get_best_cmd_once();
            } else {
                // 说明刚开始分析，此时应该有没记录的 pair，不然不会调用 get_best_cmd
                assert(is_good_opening());
                init_deduction();
                cmd = get_best_cmd();
            }
        }
        
        // 在返回 cmd 之前，先把涉及的周围位置都纳入 check_queue_
        // update_deduction(cmd.pos);
        // 此时还没更新，不好
        return cmd;
    }

    // 新概念 goto
    Command get_best_cmd_once() {
        is_once_ = true;
        Command cmd = get_best_cmd();
        is_once_ = false;
        return cmd;
    }

    // NOTE: not equal to DebugRobot::is_valid
    bool is_valid(int row, int col) const {
        if (!this->board_->is_valid(row, col)) return false;
        // if (row < 0 || row >= this->board_->height()) return false;
        // if (col < 0 || col >= this->board_->width()) return false;
        if (this->board_->get_pos(row, col)
            ->get_cover() == Cover::COVERED) return false;
        return true;
    }
    bool is_rest(int row, int col) const {
        if (!this->board_->is_valid(row, col)) return false;
        auto cur = this->board_->get_pos(row, col);
        return cur->get_cover() == Cover::COVERED
            && cur->get_flag() == Flag::NO_FLAG;
    }

    void count_pq(std::shared_ptr<const Position> p, std::shared_ptr<const Position>q, 
                  int& p_flag_cnt, int& q_flag_cnt, int& c_flag_cnt, 
                  int& p_revealed_cnt, int& q_revealed_cnt, int& c_revealed_cnt, 
                  int& p_rest_cnt, int& q_rest_cnt, int& c_rest_cnt) const {
        for (auto&& [inc_r, inc_c] : dirs) {
            int cur_r = p->row + inc_r,
                cur_c = p->col + inc_c;
            if (!this->board_->is_valid(cur_r, cur_c)) {
                if (q->is_near({cur_r, cur_c})) {
                    c_revealed_cnt++;
                } else {
                    p_revealed_cnt++;
                }
                continue;
            }
            auto cur = this->board_->get_pos(cur_r, cur_c);
            if (q->is_near({cur_r, cur_c})) {
                if (cur->get_cover() == Cover::REVEALED) {
                    c_revealed_cnt++;
                } else if (cur->get_flag() == Flag::FLAG) {
                    c_flag_cnt++;
                } else {
                    c_rest_cnt++;
                }
            } else {
                if (cur->get_cover() == Cover::REVEALED) {
                    p_revealed_cnt++;
                } else if (cur->get_flag() == Flag::FLAG) {
                    p_flag_cnt++;
                } else {
                    p_rest_cnt++;
                }
            }
        }
        if (q->is_near(*p)) c_revealed_cnt--;  // 去掉q本身
        for (auto&& [inc_r, inc_c] : dirs) {
            int cur_r = q->row + inc_r,
                cur_c = q->col + inc_c;
            if (!this->board_->is_valid(cur_r, cur_c)) {
                if (p->is_near({cur_r, cur_c})) {
                    // hello world
                } else {
                    q_revealed_cnt++;
                }
                continue;
            }
            auto cur = this->board_->get_pos(cur_r, cur_c);
            if (p->is_near({cur_r, cur_c})) {
                // hello world
            } else {
                if (cur->get_cover() == Cover::REVEALED) {
                    q_revealed_cnt++;
                } else if (cur->get_flag() == Flag::FLAG) {
                    q_flag_cnt++;
                } else {
                    q_rest_cnt++;
                }
            }
        }
    }
    void dcreveal(std::shared_ptr<const Position> p) {
        log_infer(0, "dcreveal: [%d, %d]", p->row, p->col);
        for (auto&& [inc_r, inc_c] : dirs) {
            int cur_r = p->row + inc_r,
                cur_c = p->col + inc_c;
            if (!is_rest(cur_r, cur_c)) { continue; }
            cmd_queue_.push_back({CommandType::REVEAL, {cur_r, cur_c}});
        }
    }
    void dcflag(std::shared_ptr<const Position> p) {
        log_infer(0, "dcflag: [%d, %d]", p->row, p->col);
        for (auto&& [inc_r, inc_c] : dirs) {
            int cur_r = p->row + inc_r,
                cur_c = p->col + inc_c;
            if (!is_rest(cur_r, cur_c)) { continue; }
            cmd_queue_.push_back({CommandType::FLAG, {cur_r, cur_c}});
        }
    }
    void dcmp(std::shared_ptr<const Position> p, std::shared_ptr<const Position> q) {
        log_infer(0, "dcmp: [%d, %d][%d, %d]", p->row, p->col, q->row, q->col);
        screveal(q, p);
        scflag(p, q);
    }
    void screveal(std::shared_ptr<const Position> p, std::shared_ptr<const Position> q) {
        log_infer(0, "screveal: [%d, %d][%d, %d]", p->row, p->col, q->row, q->col);
        for (auto&& [inc_r, inc_c] : dirs) {
            int cur_r = p->row + inc_r,
                cur_c = p->col + inc_c;
            if (!is_rest(cur_r, cur_c)) { continue; }
            if (q->is_near({cur_r, cur_c})) { continue; }
            cmd_queue_.push_back({CommandType::REVEAL, {cur_r, cur_c}});
        }
    }
    void scflag(std::shared_ptr<const Position> p, std::shared_ptr<const Position> q) {
        log_infer(0, "scflag: [%d, %d]", p->row, p->col);
        for (auto&& [inc_r, inc_c] : dirs) {
            int cur_r = p->row + inc_r,
                cur_c = p->col + inc_c;
            if (!is_rest(cur_r, cur_c)) { continue; }
            if (q->is_near({cur_r, cur_c})) { continue; }
            cmd_queue_.push_back({CommandType::FLAG, {cur_r, cur_c}});
        }
    }
    
    std::pair<float, Command> calc_prob(const PositionPair& pp) {
        auto p = this->board_->get_pos(pp.p1.row, pp.p1.col);
        auto q = this->board_->get_pos(pp.p2.row, pp.p2.col);
        if (p->row == q->row && p->col == q->col) {
            // CHECK: IF NEED
        }

        int m = p->get_num(), n = q->get_num();
        int p_flag_cnt = 0, q_flag_cnt = 0, c_flag_cnt = 0;
        int p_revealed_cnt = 0, q_revealed_cnt = 0, c_revealed_cnt = 0;
        int p_rest_cnt = 0, q_rest_cnt = 0, c_rest_cnt = 0;
        count_pq(p, q, p_flag_cnt, q_flag_cnt, c_flag_cnt, 
                       p_revealed_cnt, q_revealed_cnt, c_revealed_cnt, 
                       p_rest_cnt, q_rest_cnt, c_rest_cnt);
        int p_rest_mine = m - p_flag_cnt - c_flag_cnt;
        int q_rest_mine = n - q_flag_cnt - c_flag_cnt;
        // 应该有一个队列把能确定的都存起来，下次就直接开，不用再推导了
        // 来了嗷 >_0  25.04.26
        if (p_rest_mine == 0 && (p_rest_cnt || c_rest_cnt)) {
            dcreveal(p); 
        } else if (q_rest_mine == 0 && (q_rest_cnt || c_rest_cnt)) {
            dcreveal(q);
        } else if (p_rest_mine && p_rest_mine == p_rest_cnt + c_rest_cnt) {
            dcflag(p);
        } else if (q_rest_mine && q_rest_mine == q_rest_cnt + c_rest_cnt) {
            dcflag(q);
        } else if (p_rest_mine > q_rest_mine && p_rest_mine - q_rest_mine == p_rest_cnt) {
            dcmp(p, q);
        } else if (p_rest_mine < q_rest_mine && q_rest_mine - p_rest_mine == q_rest_cnt) {
            dcmp(q, p);
        } else if (p_rest_mine == q_rest_mine) {  // 挖洞
            if (p_rest_cnt == 0) {
                screveal(q, p);
            }
            if (q_rest_cnt == 0) {
                screveal(p, q);
            }
        }
        if (!cmd_queue_.empty()) {
            Command cmd = std::move(cmd_queue_.back());
            cmd_queue_.pop_back();
            return {1.0F, cmd};
        }

        return {0.5F, randomly_reveal()};  // TODO: cannot randomly reveal, may cause longtime wait
    }

    bool is_in_opening_ = true;
    bool is_once_ = false;
    std::vector<Command> cmd_queue_;
    std::queue<PositionPair> check_queue_;
    std::unordered_set<PositionPair, PositionPairHash, PositionPairEqual> queue_menbers_;
    std::unordered_set<PositionPair, PositionPairHash, PositionPairEqual> all_possible_pairs_;
};  // endof class HumanLikeRobot

}  // endof namespace mfwu

#endif  // __PLAYER_HPP__