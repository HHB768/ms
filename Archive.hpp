#ifndef __ARCHIVE_HPP__
#define __ARCHIVE_HPP__

#include "Logger.hpp"

namespace mfwu {

template <
          size_t height_,
          size_t width_,
          typename Seq_t=std::string, 
          typename Tbl_t=std::vector<std::vector<size_t>>
         >
class Archive_base {
public:
    using Seq_type = Seq_t;
    using Tbl_type = Tbl_t;
    static constexpr const char* dir = "./archive";
    Archive_base(const std::string& archive_filename="") {
        if (archive_filename == std::string("")) {
            std::string str = dir;
            str += '/'; 
            append_time_info(str);
            str += ".arc";
            archive_filename_ = str;
            if (!std::filesystem::exists(dir)) {
                status_ = std::filesystem::create_directories(dir);
                if (!status_) { 
                    log_error("archive error: ");
                    log_error(XQ4MS_TIMESTAMP, "cannot create dir");
                }
            }
        }
        // if dir doesnt exist, fs_ wont create and open the file
        fs_.open(archive_filename_, std::ios::app);
    }
    ~Archive_base() {
        if (fs_.is_open()) {
            fs_.close();
        }
    }
    virtual void init_game() = 0;
    virtual void init_game(const Tbl_type& board) = 0;
    virtual void record(const Seq_type& seq) = 0;
    virtual void record(Seq_type&& seq) = 0;
    virtual void record(const Tbl_type& tbl) = 0;
    virtual void record(Tbl_type&& tbl) = 0;
    // virtual void record(const Command& cmd) = 0;

    virtual Seq_type& get_last_frame_in_seq() = 0;
    virtual Tbl_type& get_last_frame_in_tbl() = 0;
    virtual void pop_last_n_record(int num=1) = 0;


    // warning: will destroy all the frames!
    void flush(GameStatus status) {
        for (Frame& frame : this->frames_) {
            this->flush_frame(frame);
        }
        this->flush_log(status);
        this->fs_.flush();  // flush once after a game

        // reinit for next game
        this->init_game();
    }
    bool get_status() const {
        return status_;
    }

protected:
    struct Frame {
        bool is_seq_valid = false;
        bool is_tbl_valid = false;
        Seq_type seq = {};
        Tbl_type tbl = {};

        Frame() = default;
        Frame(const Seq_type& seq_)
            : is_seq_valid(true), is_tbl_valid(false),
            seq(seq_), tbl() {}
        Frame(Seq_type&& seq_)
            : is_seq_valid(true), is_tbl_valid(false),
            seq(std::move(seq_)), tbl() {}
        Frame(const Tbl_type& tbl_)
            : is_seq_valid(false), is_tbl_valid(true),
            seq(), tbl(tbl_) {}
        Frame(Tbl_type&& tbl_)
            : is_seq_valid(false), is_tbl_valid(true),
            seq(), tbl(std::move(tbl_)) {}

        void update(const Seq_type& seq_) {
            seq = seq_;
            is_seq_valid = true;
        }
        void update(Seq_type&& seq_) {
            seq = std::move(seq_);
            is_seq_valid = true;
        }
        void update(const Tbl_type& tbl_) {
            tbl = tbl_;
            is_tbl_valid = true;
        }
        void update(Tbl_type&& tbl_) {
            tbl = std::move(tbl_);
            is_tbl_valid = true;
        }

        void serialize() {
            if (is_seq_valid) return ;
            assert(is_tbl_valid);
            seq.clear();
            seq.reserve(tbl.size() * (tbl[0].size() + 1) * 2);  // check
            for (size_t i = 0; i < tbl.size(); i++) {
                for (size_t j = 0; j < tbl[0].size(); j++) {
                    if (tbl[i][j] >= 0 and tbl[i][j] < 9) {
                        seq += char('0' + tbl[i][j]);
                    } else if (tbl[i][j] == 9) {
                        seq += 'X';
                    } else if (tbl[i][j] == 10) {
                        seq += '+';
                    } else if (tbl[i][j] == 15) {
                        seq += 'F';
                    } else {
                        seq += '?';
                    }
                    
                    seq += ' ';
                }
                seq += '\n';
            }
            is_seq_valid = true;
        }
        void deserialize() {
            if (is_tbl_valid) return ;
            assert(is_seq_valid);
            tbl.clear();
            int i = 0;
            tbl.reserve(height_);  // check
            for (int k = 0; k < seq.size(); k++) {
                if (is_digit(seq[k])) {
                    if (i >= tbl.size()) {
                        tbl.resize(i + 1);  // check
                        tbl[i].reserve(width_);  // check
                    }
                    tbl[i].push_back(seq[k]);
                } else if (seq[k] == '\n') {
                    i++;
                } else {
                    assert(seq[k] == ' ');
                }
            }
            is_tbl_valid = true;
        }

        Seq_type& get_seq() {
            serialize();
            return seq;
        }
        Tbl_type& get_tbl() {
            deserialize();
            return tbl;
        }
    };  // endof struct Frame

    void flush_log(GameStatus status) {
        if (!fs_.is_open()) {
            fs_.open(archive_filename_, std::ios::app);
        }
        fs_ << "[XQMS-SEP]\n"
            << "This game end with status:" 
            << GameStatusDescription.at(static_cast<size_t>(status))
            << "\n\n";
    }
    void flush_frame(Frame& frame) {
        if (!fs_.is_open()) {
            fs_.open(archive_filename_, std::ios::app);
        }
        fs_ << std::move(frame.get_seq()) << "\n";
    }

    std::vector<Frame> frames_;
private:
    std::string archive_filename_;
    std::fstream fs_;
    bool status_ = true;
};  // endof class Archive_base

// always sync frames_ in record()
template <typename ChessBoard_type>
class Archive : public Archive_base<ChessBoard_type::height_,
                                    ChessBoard_type::width_,
                                    typename ChessBoard_type::ArchiveSeq_type, 
                                    typename ChessBoard_type::ArchiveTbl_type> {
public:
    constexpr static const size_t height_ = ChessBoard_type::height_;
    constexpr static const size_t width_  = ChessBoard_type::width_;
    using base_type = Archive_base<height_, width_, typename ChessBoard_type::ArchiveSeq_type, 
                                   typename ChessBoard_type::ArchiveTbl_type>;
    using Frame = typename base_type::Frame;
    using Seq_type = typename base_type::Seq_type;
    using Tbl_type = typename base_type::Tbl_type;

    Archive(const std::string& archive_filename="") 
        : base_type(archive_filename) {}
    ~Archive() {}

    void record(const Seq_type& seq) override {
        this->frames_.emplace_back(seq);
    }
    void record(Seq_type&& seq) override {
        this->frames_.emplace_back(std::move(seq));
    }
    void record(const Tbl_type& tbl) override {
        this->frames_.emplace_back(tbl);
    }
    void record(Tbl_type&& tbl) override {
        this->frames_.emplace_back(std::move(tbl));
    }
    // void record(const Command& cmd) override {}

    Seq_type& get_last_frame_in_seq() override {
        return this->frames_.back().get_seq();
    }
    Tbl_type& get_last_frame_in_tbl() override {
        return this->frames_.back().get_tbl();
    }
    void pop_last_n_record(int num=1) override {
        for (int i = 0; i < num; i++) {
            this->frames_.pop_back();     
        }
    }

    void init_game() override {
        this->frames_.clear();
        // this->frames_.emplace_back(Tbl_type(Size, typename Tbl_type::value_type(Size, 0)));
        // check: we dont need this
    }
    void init_game(const Tbl_type& board) override {
        this->frames_.clear();
        // this->frames_.emplace_back(board);
    }
};  // endof class Archive

}  // endof namespace mfwu

#endif  // __ARCHIVE_HPP__