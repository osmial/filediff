#include <algorithm>
#include <fstream>
#include <limits>
#include <map>

#include <fmt/core.h>

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

filediff::Delta::Delta(std::string_view sigFileName, std::string_view dataFileName)
    : m_dataFileName { dataFileName }
    , m_baseSignature { sigFileName, filediff::Signature::InputFileType::SIGNATURE }
{
}

auto findMatchingHash(auto itBegin, auto itEnd, uint32_t hash)
{
    return std::find_if(itBegin, itEnd, [hash](const auto& pair) {
        return pair.hash == hash;
    });
}

void filediff::Delta::Calculate()
{
    std::ifstream ifs { m_dataFileName.data() };
    if (!ifs.is_open()) {
        throw std::runtime_error(fmt::format("File {} not found!", m_dataFileName));
    }

    auto updatedFileMetadata = ParseDataFile(ifs);
    auto oldHashes { m_baseSignature.GetHashes() };
    auto lineToBeParsedMarker = std::cbegin(updatedFileMetadata);
    std::vector<decltype(updatedFileMetadata)::const_iterator> matchingRangeMarkers;
    auto it = std::cbegin(updatedFileMetadata);

    auto insertingLambda = [&ifs](const auto& elem) { return std::make_pair(elem.hash, ReadLine(ifs, elem.linePos)); };

    for (auto i = 0U; i < oldHashes.size(); ++i) {
        auto keepIt = it;
        it = findMatchingHash(it, std::cend(updatedFileMetadata), oldHashes[i]);

        if (it == std::cend(updatedFileMetadata)) {
            // chunk not found in new version of the file is considered as removed
            m_delta.emplace_back(oldHashes[i], "");
            it = keepIt;
            continue;
        }

        // TODO: checking here only next element after the one suspected that was removed but found in other place in
        //       the file is not exactly perfect approach, what should be done here is to check if any of the elements
        //       in range (oldHashes[i+1], oldHashes[value of it]] still persists new file (which meeans in range
        //       [keepIt, it) in updatedFileMetadata), if so then 'it' should point to that matching element and all
        //       preceding elements (from oldHashes) shall be considered as removed -> it's not a bug but it could be improved
        auto iter = findMatchingHash(keepIt, std::cend(updatedFileMetadata), oldHashes[i + 1]);
        if (std::distance(std::cbegin(updatedFileMetadata), iter) < std::distance(std::cbegin(updatedFileMetadata), it)) {
            // this means 'it' should be considered as deleted and the fact it was found means there were more such chunks in the file
            m_delta.emplace_back(oldHashes[i], "");
            it = keepIt;
            continue;
        }

        matchingRangeMarkers.emplace_back(it);
        it++; // we don't want to fell into any weird loop in case of having few the same entries in a row

        if (matchingRangeMarkers.size() == 1) {
            // insert new elements prefacing matching chunks
            std::transform(lineToBeParsedMarker, matchingRangeMarkers[0], std::back_inserter(m_delta), insertingLambda);
            lineToBeParsedMarker = std::next(matchingRangeMarkers[0]);
        } else if (matchingRangeMarkers.size() == 2) {
            // insert new elements from matching chunks "block"
            std::transform(std::next(matchingRangeMarkers[0]), matchingRangeMarkers[1], std::back_inserter(m_delta),
                insertingLambda);
            lineToBeParsedMarker = std::next(matchingRangeMarkers[1]);
            matchingRangeMarkers.clear();
        }
    }
    // insert all remaining chunks not matching old hashes
    std::transform(lineToBeParsedMarker, std::cend(updatedFileMetadata), std::back_inserter(m_delta), insertingLambda);
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
