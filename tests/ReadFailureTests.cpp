#include "TextReader.hpp"
#include <iostream>
#include <sstream>
#include <streambuf>

// Deliver some text, then simulate a device failing partway through reading.
class FailingBuffer : public std::streambuf {
public:
    FailingBuffer() { setg(prefix_, prefix_, prefix_ + 6); }

protected:
    int_type underflow() override {
        throw std::ios_base::failure("simulated read failure");
    }

private:
    char prefix_[7] = "prefix";
};

int main() {
    FailingBuffer buffer;
    std::istream failingInput(&buffer);
    const auto failed = minigit::detail::readTextStream(failingInput, "failure.txt");
    if (failed.ok || !failed.text.empty() ||
        failed.error.find("failure.txt") == std::string::npos) {
        std::cerr << "A mid-read failure must return an error without partial text\n";
        return 1;
    }

    std::istringstream emptyInput("");
    const auto empty = minigit::detail::readTextStream(emptyInput, "empty.txt");
    if (!empty.ok || !empty.text.empty() || !empty.error.empty()) {
        std::cerr << "An empty file must remain a successful read\n";
        return 1;
    }

    const std::string expected = " first line\n\nlast line without newline";
    std::istringstream textInput(expected);
    const auto text = minigit::detail::readTextStream(textInput, "text.txt");
    if (!text.ok || text.text != expected || !text.error.empty()) {
        std::cerr << "Successful reading must preserve all supplied text\n";
        return 1;
    }

    std::cout << "Read failure checks passed\n";
    return 0;
}
