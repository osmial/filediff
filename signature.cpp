#include <fstream>
#include <stdexcept>

#include <fmt/core.h>

#include "adler32.h"
#include "signature.h"

filediff::Signature::Signature(std::string_view path, InputFileType fileType)
{
    if(fileType == InputFileType::SIGNATURE) {
        std::ifstream isf { path.data(), std::ios::binary };
        if(!isf.is_open()) {
            throw std::runtime_error(std::string { fmt::format("File {} not found!", path) });
        }

        isf.read(reinterpret_cast<char*>(&m_metadata), sizeof(decltype (m_metadata)));

        while (true) {
            uint32_t hash;
            isf.read(reinterpret_cast<char*>(&hash), sizeof(decltype (hash)));

            if(isf.eof()) {
                break;
            }

            m_hashes.emplace_back(hash);
        }
    } else {
        std::ifstream isf { path.data() };
        if(!isf.is_open()) {
            throw std::runtime_error(fmt::format("File {} not found!", path));
        }

        std::string line;
        while(std::getline(isf, line)) {
            m_hashes.push_back(adler32(line));
        }
        m_metadata = Metadata{m_hashes.size(), 1};
    }
}

const std::deque<uint32_t>& filediff::Signature::GetHashes() const noexcept
{
    return m_hashes;
}

const filediff::Signature::Metadata& filediff::Signature::GetMetadata() const noexcept
{
    return m_metadata;
}

void filediff::Signature::Serialize(std::ostream& out) const
{
    out.write(reinterpret_cast<const char*>(&m_metadata), sizeof(decltype(m_metadata)));
    for(auto elem : m_hashes) {
        out.write(reinterpret_cast<char*>(&elem), sizeof(decltype(elem)));
    }
}
