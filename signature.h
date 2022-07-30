#ifndef SIGNATURE_H
#define SIGNATURE_H

#include <deque>
#include <sstream>
#include <string_view>

namespace filediff {

class Signature
{
public:
    struct Metadata {
        size_t m_numberOfChunks;
        uint32_t m_chunkLenght; //in lines - as of now hard coded to 1
    };

    enum class InputFileType {
        BASIS,
        SIGNATURE
    };

    // ctor taking path to signature file
    Signature(std::string_view fileName, InputFileType fileType);

    const std::deque<uint32_t>& GetHashes() const noexcept;

    // save calculations + metadata to signature file
    void Serialize(std::ostream& out) const;

protected:
    const Metadata& GetMetadata() const noexcept;

private:
    std::deque<uint32_t> m_hashes;
    Metadata m_metadata;
};

} // filediff
#endif // SIGNATURE_H
