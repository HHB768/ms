#ifndef __LOGGER_HPP__
#define __LOGGER_HPP__

#include "common.hpp"

namespace mfwu {

enum class LogLevel : size_t {
    INFER = 0,
    DEBUG = 1,
    INFO  = 2,
    WARN  = 3,
    ERROR = 4,
    
    TOTAL
};  // endof enum class LogLevel


struct LogMsg {
    time_t time_stamp;
    // uint32_t pid;
    // uint64_t tid;
    std::string msg;
};  // endof struct LogMsg

class LogFormatter {
public:
    static std::string format(LogLevel level, const LogMsg& msg) {
        std::stringstream ss;
        char buffer[64];
        if (msg.time_stamp == XQ4MS_TIMESTAMP) {
            strcpy(buffer, "                     \0");
            //              [2025-03-12 23:15:00]
            ss << buffer;
        } else {
            tm* info = localtime(&msg.time_stamp);
            strftime(buffer, 64, "%Y-%m-%d %H:%M:%S", info);
            ss << '[' << buffer << ']';
        }
        ss << LogLevelDescription.at(static_cast<size_t>(level))
           << ' ' << msg.msg;
        return ss.str();
    }
// private:
    static const std::vector<std::string> LogLevelDescription;
};  // endof class LogFormatter

class InferFormatter {
    public:
        static std::string format(LogLevel level, const LogMsg& msg) {
            // std::stringstream ss;
            // char buffer[64];
            // if (msg.time_stamp == XQ4MS_TIMESTAMP) {
            //     strcpy(buffer, "                     \0");
            //     //              [2025-03-07 20:57:00]
            //     ss << buffer;
            // } else {
            //     tm* info = localtime(&msg.time_stamp);
            //     strftime(buffer, 64, "%Y-%m-%d %H:%M:%S", info);
            //     ss << '[' << buffer << ']';
            // }
            // ss << LogLevelDescription.at(static_cast<size_t>(level))
            //    << ' ' << msg.msg;
            std::string ret;
            // ret += '{';
            ret += std::to_string(msg.time_stamp - XQ4MS_TIMESTAMP);
            ret += ' ';
            ret += msg.msg;
            return ret;
        }
    private:
        static const std::vector<std::string> LogLevelDescription;
};  // endof class LogFormatter

const std::vector<std::string> LogFormatter::LogLevelDescription = {
    "[INFER]", "[DEBUG]", "[INFO] ","[WANR] ", "[ERROR]"
};
const std::vector<std::string> InferFormatter::LogLevelDescription
= LogFormatter::LogLevelDescription;

class LogAppender {
public:
    LogAppender(LogLevel level) : level_(level), 
        formatter_(std::make_shared<LogFormatter>()) {}

    virtual void append(LogLevel level, const LogMsg& msg) = 0;
protected:
    LogLevel level_;
    std::shared_ptr<LogFormatter> formatter_; 
};  // endof class LogAppender

class StdAppender : public LogAppender {
public:
    StdAppender(LogLevel level) : LogAppender(level) {}
    void append(LogLevel level, const LogMsg& msg) {
        if (level < this->level_) return ;
        std::string res = this->formatter_->format(level, msg);
        std::cout << res << "\n";
    }
};  // endof class StdAppender

class FileAppender : public LogAppender {
public:
    static constexpr const char* dir = "./log";
    FileAppender(LogLevel level, std::string filename="") 
        : LogAppender(level), filename_(filename), fs_() {
        if (filename == std::string("")) {
            std::string str = dir;
            str += '/'; 
            append_time_info(str);
            str += ".log";
            filename_ = str;
            if (!std::filesystem::exists(dir)) {
                bool succ = std::filesystem::create_directories(dir);
                if (!succ) { 
                    std::cerr << "creating dir fails, logfile may be lost\n";
                }
            }
        }
        fs_.open(filename_, std::ios::app);
    }
    ~FileAppender() {
        if (fs_.is_open()) {
            fs_.close();
        }
    }
    void append(LogLevel level, const LogMsg& msg) {
        if (level < this->level_) return ;
        std::string res = this->formatter_->format(level, msg);
        if (!fs_.is_open()) {
            fs_.open(filename_, std::ios::app);
        }
        fs_ << res << "\n";
    }
    void flush() {
        if (!fs_.is_open()) {
            fs_.open(filename_, std::ios::app);
        }
        fs_.flush();
    }

private:
    std::fstream fs_;
    std::string filename_;
};  // endof class FileAppender

class InferAppender /*: public LogAppender*//*: public FileAppender*/ {  // TODO
public:
    static constexpr const char* dir = "./inference";
    InferAppender(LogLevel level, std::string filename="") 
        : level_(level), formatter_(std::make_shared<InferFormatter>()),
          filename_(filename), fs_() {
        if (filename == std::string("")) {
            std::string str = dir;
            str += '/'; 
            append_time_info(str);
            str += ".inf";
            filename_ = str;
            if (!std::filesystem::exists(dir)) {
                bool succ = std::filesystem::create_directories(dir);
                if (!succ) { 
                    std::cerr << "Creating dir fails, logfile may be lost\n";
                }
            }
        }
        fs_.open(filename_, std::ios::app);
    }
    ~InferAppender() {
        if (fs_.is_open()) {
            fs_.close();
        }
    }
    void append(LogLevel level, const LogMsg& msg) {
        if (level < this->level_) return ;
        std::string res = this->formatter_->format(level, msg);
        if (!fs_.is_open()) {
            fs_.open(filename_, std::ios::app);
        }
        fs_ << res << "\n";
    }
    void flush() {
        if (!fs_.is_open()) {
            fs_.open(filename_, std::ios::app);
        }
        fs_.flush();
    }

private:
    LogLevel level_;
    std::shared_ptr<InferFormatter> formatter_; 

    std::fstream fs_;
    std::string filename_;
};  // endof class InferAppender

class Logger {
public:
    static Logger& Instance() {
        static Logger logger;
        return logger;
    }

    template <typename... Args>
    void log(LogLevel level, const char* fmt, Args&&... args) {
        log(level, format(fmt, std::forward<Args>(args)...));
    }
    template <typename... Args>
    void log(LogLevel level, time_t time_stamp, const char* fmt, Args&&... args) {
        log(level, time_stamp, format(fmt, std::forward<Args>(args)...));
    }
    template <typename... Args>
    void log(LogLevel level, const std::string& fmt, Args&&... args) {
        log(level, format(fmt.c_str(), std::forward<Args>(args)...));
    }
    template <typename... Args>
    void log(LogLevel level, time_t time_stamp, const std::string& fmt, Args&&... args) {
        log(level, time_stamp, format(fmt.c_str(), std::forward<Args>(args)...));
    }
    // check: if we pass a string with const char*, 
    // should it be accepted by the first one?
    void log(LogLevel level, const std::string& msg) {
        LogMsg lmsg;
        lmsg.time_stamp = time(0);
        lmsg.msg = msg;
        std_appender_.append(level, lmsg);
        file_appender_.append(level, lmsg);
#ifdef __LOG_INFERENCE_ELSEWHERE__
        if (level <= LogLevel::INFER) {
            inference_appender_.append(level, lmsg);
        }
#endif  // __LOG_INFERENCE_ELSEWHERE__
    } 

    void log(LogLevel level, time_t time_stamp, const std::string& msg) {
        LogMsg lmsg;
        lmsg.time_stamp = time_stamp;
        lmsg.msg = msg;
        std_appender_.append(level, lmsg);
        file_appender_.append(level, lmsg);
#ifdef __LOG_INFERENCE_ELSEWHERE__
        if (level <= LogLevel::INFER) {
            inference_appender_.append(level, lmsg);
        }
#endif  // __LOG_INFERENCE_ELSEWHERE__
    }
    template <typename... Args>
    void log_infer(size_t infer_depth, const char* fmt, Args&&... args) {
        log_infer(infer_depth, std::string(fmt), std::forward<Args>(args)...);
    }
    template <typename... Args>
    void log_infer(time_t time_stamp, size_t infer_depth, const char* fmt, Args&&... args) {
        log_infer(time_stamp, infer_depth, std::string(fmt), std::forward<Args>(args)...);
    }
    template <typename... Args>
    void log_infer(size_t infer_depth, const std::string& fmt, Args&&... args) {
        std::string fmt_with_pref = form_infer_msg(infer_depth, fmt);
        log(LogLevel::INFER, fmt_with_pref, infer_depth, std::forward<Args>(args)...);
    }
    template <typename... Args>
    void log_infer(time_t time_stamp, size_t infer_depth, const std::string& fmt, Args&&... args) {
        std::string fmt_with_pref = form_infer_msg(infer_depth, fmt);
        log(LogLevel::INFER, time_stamp, fmt_with_pref, infer_depth, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void log_debug(const char* fmt, Args&&... args) {
        log(LogLevel::DEBUG, fmt, std::forward<Args>(args)...);
    }
    template <typename... Args>
    void log_debug(time_t time_stamp, const char* fmt, Args&&... args) {
        log(LogLevel::DEBUG, time_stamp, fmt, std::forward<Args>(args)...);
    }
    template <typename... Args>
    void log_debug(const std::string& fmt, Args&&... args) {
        log(LogLevel::DEBUG, fmt, std::forward<Args>(args)...);
    }
    template <typename... Args>
    void log_debug(time_t time_stamp, const std::string& fmt, Args&&... args) {
        log(LogLevel::DEBUG, time_stamp, fmt, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void log_info(const char* fmt, Args&&... args) {
        log(LogLevel::INFO, fmt, std::forward<Args>(args)...);
    }
    template <typename... Args>
    void log_info(time_t time_stamp, const char* fmt, Args&&... args) {
        log(LogLevel::INFO, time_stamp, fmt, std::forward<Args>(args)...);
    }
    template <typename... Args>
    void log_info(const std::string& fmt, Args&&... args) {
        log(LogLevel::INFO, fmt, std::forward<Args>(args)...);
    }
    template <typename... Args>
    void log_info(time_t time_stamp, const std::string& fmt, Args&&... args) {
        log(LogLevel::INFO, time_stamp, fmt, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void log_warn(const char* fmt, Args&&... args) {
        log(LogLevel::WARN, fmt, std::forward<Args>(args)...);
    }
    template <typename... Args>
    void log_warn(time_t time_stamp, const char* fmt, Args&&... args) {
        log(LogLevel::WARN, time_stamp, fmt, std::forward<Args>(args)...);
    }
    template <typename... Args>
    void log_warn(const std::string& fmt, Args&&... args) {
        log(LogLevel::WARN, fmt, std::forward<Args>(args)...);
    }
    template <typename... Args>
    void log_warn(time_t time_stamp, const std::string& fmt, Args&&... args) {
        log(LogLevel::WARN, time_stamp, fmt, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void log_error(const char* fmt, Args&&... args) {
        log(LogLevel::ERROR, fmt, std::forward<Args>(args)...);
    }
    template <typename... Args>
    void log_error(time_t time_stamp, const char* fmt, Args&&... args) {
        log(LogLevel::ERROR, time_stamp, fmt, std::forward<Args>(args)...);
    }
    template <typename... Args>
    void log_error(const std::string& fmt, Args&&... args) {
        log(LogLevel::ERROR, fmt, std::forward<Args>(args)...);
    }
    template <typename... Args>
    void log_error(time_t time_stamp, const std::string& fmt, Args&&... args) {
        log(LogLevel::ERROR, time_stamp, fmt, std::forward<Args>(args)...);
    }
    // TODO: though harmless, we should keep it behind end_game() in gc 
    void new_game(size_t board_height, size_t board_width) {
#ifdef __LOG_INFERENCE_ELSEWHERE__
        log(LogLevel::INFER, "{%d,%d}", board_height, board_width);
#endif  // __LOG_INFERENCE_ELSEWHERE__
    }
    void end_game(GameStatus status) {
        log(LogLevel::INFO, "Game ends with status: %s", 
            GameStatusDescription.at(static_cast<size_t>(status)).c_str());
        file_appender_.flush();
    }

private:
// #define __CMD_MODE__  // dont need to define CMD_MODE here actually
#ifdef __CMD_MODE__
    Logger() : std_appender_(LogLevel::TOTAL), 
#ifdef __LOG_INFERENCE_ELSEWHERE__
    file_appender_(LogLevel::DEBUG),
    inference_appender_(LogLevel::INFER)
#else  // !__LOG_INFERENCE_ELSEWHERE__
    file_appender_(LogLevel::INFER) 
#endif  // __LOG_INFERENCE_ELSEWHERE__
    {}
#else  // __GUI_MODE__
    Logger() : std_appender_(LogLevel::ERROR), file_appender_(LogLevel::DEBUG) {}
#endif  // __CMD_MODE__

    // static void infer_log_space(std::string& str, int num) {
    //     for (int i = 0; i < num; i++) {
    //         str += "    ";
    //     }
    // }

    ~Logger() {}
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    
    std::string format(const char* fmt, ...) const {
        int len;
        std::string str;
        va_list args;
        char buffer[256];

        va_start(args, fmt);
        if ((len = vsnprintf(buffer, sizeof(buffer), fmt, args)) > 0) {
            if (len < sizeof(buffer)) {
                str = buffer;
            } else {
                int maxsz = len + 1;
                char* buffer = (char*)malloc(maxsz);
                if (buffer) {
                    len = vsnprintf(buffer, maxsz, fmt, args);
                    if (len > 0 && len < maxsz) {
                        str = buffer;
                    }
                    free(buffer);
                }
            }
        }
        va_end(args);
        return str;
    }

    std::string form_infer_msg(const size_t infer_depth, const std::string& fmt) const {
#ifndef __LOG_INFERENCE_ELSEWHERE__
        std::string fmt_with_pref = "[Depth = %d]";
        // infer_log_space(fmt_with_pref, INFERENCE_DEPTH - infer_depth);
        fmt_with_pref += " ::: "; fmt_with_pref += fmt; fmt_with_pref += " ::: ";
#else  // __LOG_INFERENCE_ELSEWHERE__
        std::string fmt_with_pref = "%d ";
        fmt_with_pref += fmt;
        #endif  // __LOG_INFERENCE_ELSEWHERE__
        return fmt_with_pref;
    }
    StdAppender std_appender_;
    FileAppender file_appender_;
#ifdef __LOG_INFERENCE_ELSEWHERE__
    InferAppender inference_appender_;
#endif  // __LOG_INFERENCE_ELSEWHERE__
};  // endof class Logger

template <typename... Args>
void log(LogLevel level, const char* fmt, Args&&... args) {
    Logger& logger = Logger::Instance();
    logger.log(level, fmt, std::forward<Args>(args)...);
}
template <typename... Args>
void log(LogLevel level, time_t time_stamp, const char* fmt, Args&&... args) {
    Logger& logger = Logger::Instance();
    logger.log(level, time_stamp, fmt, std::forward<Args>(args)...);
}

template <typename... Args>
void log_infer(size_t infer_depth, const char* fmt, Args&&... args) {
    Logger& logger = Logger::Instance();
    logger.log_infer(infer_depth, fmt, std::forward<Args>(args)...);
}
template <typename... Args>
void log_infer(time_t time_stamp, size_t infer_depth, const char* fmt, Args&&... args) {
    Logger& logger = Logger::Instance();
    logger.log_infer(time_stamp, infer_depth, fmt, std::forward<Args>(args)...);
}

template <typename... Args>
void log_debug(const char* fmt, Args&&... args) {
    Logger& logger = Logger::Instance();
    logger.log_debug(fmt, std::forward<Args>(args)...);
}
template <typename... Args>
void log_debug(time_t time_stamp, const char* fmt, Args&&... args) {
    Logger& logger = Logger::Instance();
    logger.log_debug(time_stamp, fmt, std::forward<Args>(args)...);
}

template <typename... Args>
void log_info(const char* fmt, Args&&... args) {
    Logger& logger = Logger::Instance();
    logger.log_info(fmt, std::forward<Args>(args)...);
}
template <typename... Args>
void log_info(time_t time_stamp, const char* fmt, Args&&... args) {
    Logger& logger = Logger::Instance();
    logger.log_info(time_stamp, fmt, std::forward<Args>(args)...);
}

template <typename... Args>
void log_warn(const char* fmt, Args&&... args) {
    Logger& logger = Logger::Instance();
    logger.log_warn(fmt, std::forward<Args>(args)...);
}
template <typename... Args>
void log_warn(time_t time_stamp, const char* fmt, Args&&... args) {
    Logger& logger = Logger::Instance();
    logger.log_warn(time_stamp, fmt, std::forward<Args>(args)...);
}

template <typename... Args>
void log_error(const char* fmt, Args&&... args) {
    Logger& logger = Logger::Instance();
    logger.log_error(fmt, std::forward<Args>(args)...);
}
template <typename... Args>
void log_error(time_t time_stamp, const char* fmt, Args&&... args) {
    Logger& logger = Logger::Instance();
    logger.log_error(time_stamp, fmt, std::forward<Args>(args)...);
}

void log_new_game(size_t board_height, size_t board_width) {
    Logger& logger = Logger::Instance();
    logger.new_game(board_height, board_width);
}
void log_end_game(GameStatus status) {
    Logger& logger = Logger::Instance();
    logger.end_game(status);
}


// ---------------------------------------------
// logerr shortcuts
#define gc_error_exit(mode, size) do {  \
    log_error(ERROR_NEW_GC);  \
    log_error(XQ4MS_TIMESTAMP, "mode: %lu, size: %lu", \
              static_cast<size_t>(mode), static_cast<size_t>(size));  \
    exit(-10086);  \
} while (0);
inline void logerr_unknown_cmdtype()      { log_error(ERROR_UNKNOWN_COMMAND_TYPE); }
inline void logerr_unknown_tile_status() { log_error(ERROR_UNKNOWN_TILE_STATUS); }
inline void logerr_unknown_game_status()  { log_error(ERROR_UNKNOWN_GAME_STATUS);  }

}  // endof namespace mfwu

#endif  // __LOGGER_HPP__
