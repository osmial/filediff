#ifndef SIGNATURE_H
#define SIGNATURE_H

#include <string>
#include <deque>
#include <sstream>

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
    Signature(const std::string& fileName, InputFileType fileType);

    const std::deque<uint32_t>& GetHashes() const noexcept; //not sure if this should be a part of public API but it's usefull for testing now, decide later
    const Metadata& GetMetadata() const noexcept; //same here, review later

    // save calculations + metadata to signature file
    void Serialize(std::ostream& out) const;
private:
    const std::string m_fileName;
    std::deque<uint32_t> m_hashes;
    Metadata m_metadata;
};

} // filediff
#endif // SIGNATURE_H
