#include <algorithm>
#include <fstream>
#include <limits>

#include "adler32.h"
#include "delta.h"

static auto ReadLine(std::ifstream& ifs, auto lineNumber)
{
    static auto lineCntr { 0U };
    if (lineCntr >= lineNumber) {
        if (ifs.eof()) {
            ifs.clear();
        }
        ifs.seekg(0); // reset file pointer to be able to read previous lines
        lineCntr = 0;
    }

    while (lineCntr < lineNumber) {
        ifs.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        lineCntr++;
    }
    std::string line;
    std::getline(ifs, line);
    lineCntr++;

    return line;
}

filediff::Delta::Delta(const std::string& sigFileName, const std::string& dataFile)
    : m_dataFile { dataFile }
    , m_baseSignature { sigFileName, filediff::Signature::InputFileType::SIGNATURE }
{
}

void filediff::Delta::Calculate()
{
    std::ifstream ifs { m_dataFile };
    if (!ifs.is_open()) {
        throw std::runtime_error("File " + m_dataFile + " not found!");
    }

    // parse data file //
    auto updatedFileMetadata = ParseDataFile(ifs);

    // diff detection logic
    auto newElementsCntr { 0U };
    auto elementsMovedCntr { 0U };
    auto oldHashes { m_baseSignature.GetHashes() };

    std::unordered_map<uint32_t, uint32_t> oldHashesIndexes;
    for (auto it = std::cbegin(oldHashes); it != std::cend(oldHashes); ++it) {
        oldHashesIndexes.emplace(*it, std::distance(std::cbegin(oldHashes), it));
    }

    auto elemIt = std::cbegin(updatedFileMetadata);
    while (elemIt != std::cend(updatedFileMetadata)) {
        if (auto it = std::find(std::cbegin(oldHashes), std::cend(oldHashes), elemIt->hash); it == std::cend(oldHashes)) {
            // hash not found, new or updated chunk
            m_delta.emplace_back(elemIt->hash, ReadLine(ifs, elemIt->linePos));
            newElementsCntr++;
        } else {
            // hash found, but it might be moved around the file, check against that
            auto oldHashIndex = oldHashesIndexes[*it];
            auto currentHashIndex = std::distance(std::cbegin(updatedFileMetadata), elemIt) - newElementsCntr - elementsMovedCntr;
            if (oldHashIndex != currentHashIndex) {
                // element moved around the file
                m_delta.emplace_back(elemIt->hash, ReadLine(ifs, elemIt->linePos));
                elementsMovedCntr++;
            }
            oldHashes.erase(it);
        }
        elemIt++;
    }

    std::transform(std::cbegin(oldHashes), std::cend(oldHashes), std::back_inserter(m_delta), [](auto& hash) { return std::make_pair(hash, ""); });
    ifs.close();
}

bool filediff::Delta::IsChanged() const noexcept
{
    return m_delta.size();
}

void filediff::Delta::SerializeDelta(std::ostream& ostream) const
{
    if (IsChanged()) {
        for (const auto& elem : m_delta) {
            ostream << std::hex << elem.first << "\n"
                    << elem.second << "\n";
        }
    }
}

const std::deque<std::pair<uint32_t, std::string>>& filediff::Delta::GetRawDelta() const noexcept
{
    return m_delta;
}

std::deque<filediff::Delta::LineMetadata> filediff::Delta::ParseDataFile(std::ifstream& ifs)
{
    std::string line;
    auto lineCounter { 0U };
    std::deque<LineMetadata> metadata;
    while (std::getline(ifs, line)) {
        auto hash { adler32(line) };
        metadata.emplace_back(hash, lineCounter++);
    }

    if (ifs.eof()) {
        ifs.clear();
    }
    ifs.seekg(0);

    return metadata;
}
