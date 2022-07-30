#ifndef DELTA_HPP
#define DELTA_HPP

#include <deque>
#include <string_view>
#include <utility>

#include "signature.h"

namespace filediff {

class Delta
{
public:
    Delta(std::string_view sigFileName, std::string_view dataFileName);

    void Calculate();

    void SerializeDelta(std::ostream& ostream) const;

    bool IsChanged() const noexcept;

protected:
    const std::deque<std::pair<uint32_t, std::string>>& GetRawDelta() const noexcept;

private:
    struct LineMetadata {
        uint32_t hash;
        uint32_t linePos;
    };

    std::deque<LineMetadata> ParseDataFile(std::ifstream& ifs);

    std::string_view
        m_dataFileName; // this might be suspicious but the lifetime of orginal string is enough to not end up with dangling pointers.
    Signature m_baseSignature;
    std::deque<uint32_t> m_newHashes;
    std::deque<std::pair<uint32_t, std::string>> m_delta;
};

} // filediff
#endif // DELTA_HPP
