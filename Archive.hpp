#ifndef __ARCHIVE_HPP__
#define __ARCHIVE_HPP__

#include "Logger.hpp"

namespace mfwu {

template <typename Board_type>
class Archive {
public:
    bool get_status() { return true; }
    void flush(GameStatus) {}
    void record(std::string str) {}

};  // endof class Archive

}  // endof namespace mfwu

#endif  // __ARCHIVE_HPP__