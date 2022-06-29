#include <array>
#include <fstream>
#include <gtest/gtest.h>
#include <iostream>
#include <utility>

#include "../adler32.h"
#include "../delta.h"
#include "../signature.h"

namespace testing {

static constexpr auto WIKIPEDIA_HASH { 0x11E60398U };
static constexpr auto LOREM_IPSUM_HASH { 0xa05ca509U };
static constexpr auto SOME_TEXT_HASH { 0x52D40776U };
static constexpr auto YET_ANOTHER_TEXT_HASH { 0x1F9F0E25U };

static const auto WIKIPEDIA_STR { "Wikipedia" };
static const auto LOREM_IPSUM_STR { "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum." };
static const auto SOME_TEXT_STR { "Some text to be added" };
static const auto YET_ANOTHER_TEXT_STR { "Yet another text that needs to be added" };

class adler32TestSuite : public ::testing::Test {
};

TEST(adler32TestSuite, OneWordTest)
{
    ASSERT_EQ(WIKIPEDIA_HASH, adler32(WIKIPEDIA_STR));
}

TEST(adler32TestSuite, LoremIpsumTest)
{
    ASSERT_EQ(LOREM_IPSUM_HASH, adler32(LOREM_IPSUM_STR));
}

class SignatureBasicTestSuite : public ::testing::Test {
};

TEST(SignatureBasicTestSuite, FileNotFoundTest) {
    EXPECT_THROW({
        const std::string fileName { "not_existing_file.txt" };
        try {
            filediff::Signature signature(fileName, filediff::Signature::InputFileType::BASIS);
        } catch (std::runtime_error& e) {
            const std::string expectedErrorMessage { "File " + fileName + " not found!" };
            EXPECT_STREQ(expectedErrorMessage.c_str(), e.what());
            throw;
        }
    },
        std::runtime_error);
}

TEST(SignatureBasicTestSuite, SingleWordCalculationTest) {
    const std::string testFile{"test.txt"};
    std::ofstream ofs{testFile};
    ofs << WIKIPEDIA_STR;
    ofs.close();

    filediff::Signature signature{testFile, filediff::Signature::InputFileType::BASIS};
    auto hashes{signature.GetHashes()};
    EXPECT_EQ(1, hashes.size());
    EXPECT_EQ(WIKIPEDIA_HASH, hashes[0]);
}

TEST(SignatureBasicTestSuite, SerializeTest) {
    const std::string testFile{"test.txt"};
    std::ofstream ofs{testFile};
    ofs << LOREM_IPSUM_STR;
    ofs.close();

    filediff::Signature signature{testFile, filediff::Signature::InputFileType::BASIS};
    std::stringstream ss;
    signature.Serialize(ss);
    ss.seekp(0);
    filediff::Signature::Metadata metadata;
    ss.read(reinterpret_cast<char*>(&metadata), sizeof (decltype (metadata)));
    uint32_t hash;
    ss.read(reinterpret_cast<char*>(&hash), sizeof (uint32_t));
    EXPECT_EQ(1, metadata.m_numberOfChunks);
    EXPECT_EQ(1, metadata.m_chunkLenght);
    EXPECT_EQ(LOREM_IPSUM_HASH, hash);
}

struct TestingBase {
    void PrepareTestFiles(const std::string& data, uint32_t expectedHash)
    {
        PrepareDataTestFile({ data });
        PrepareSigTestFile({ expectedHash });
    }

    void PrepareDataTestFile(const std::vector<std::string>& lines)
    {
        std::ofstream ofs { m_dataTestFile };
        for (const auto& line : lines) {
            ofs << line << "\n";
        }
        ofs.close();
    }

    void PrepareSigTestFile(std::vector<uint32_t> hashList)
    {
        filediff::Signature signature { m_dataTestFile, filediff::Signature::InputFileType::BASIS };
        auto hashes { signature.GetHashes() };
        EXPECT_EQ(hashList.size(), hashes.size());
        for (const auto& hash : hashList) {
            EXPECT_EQ(hash, hashes[0]);
            hashes.pop_front();
        }

        std::ofstream ofSigStream { m_signatureTestFile, std::ios::binary };
        if (ofSigStream.is_open()) {
            signature.Serialize(ofSigStream);
            ofSigStream.close();
        }
    }

    const std::string m_dataTestFile { "test.txt" };
    const std::string m_signatureTestFile { "test.txt.sig" };
};

class SignatureCalculationTestSuite : public TestingBase, public ::testing::TestWithParam<std::pair<std::string, uint32_t>> {
};

TEST_P(SignatureCalculationTestSuite, SignatureFileReadWriteSimpleTest) {
    auto params{GetParam()};
    PrepareTestFiles(params.first, params.second);

    //test begins//
    filediff::Signature testSig{m_signatureTestFile, filediff::Signature::InputFileType::SIGNATURE};
    EXPECT_EQ(1, testSig.GetMetadata().m_numberOfChunks);
    EXPECT_EQ(1, testSig.GetMetadata().m_chunkLenght);
    EXPECT_EQ(1, testSig.GetHashes().size());
    EXPECT_EQ(params.second, testSig.GetHashes()[0]);
}

INSTANTIATE_TEST_SUITE_P(SignatureFileTests, SignatureCalculationTestSuite,
    ::testing::Values(std::pair { WIKIPEDIA_STR, WIKIPEDIA_HASH },
        std::pair { LOREM_IPSUM_STR, LOREM_IPSUM_HASH }));

class DeltaTestSuite : public TestingBase, public ::testing::Test {
public:
    class DeltaTesting : public filediff::Delta {
    public:
        DeltaTesting(const std::string& sigFileName, const std::string& dataFile)
            : Delta(sigFileName, dataFile)
        {
        }

        const std::deque<std::pair<uint32_t, std::string>>& GetRawDelta() const noexcept
        {
            return Delta::GetRawDelta();
        }
    };
};

TEST_F(DeltaTestSuite, SameFileNoChangeTest)
{
    PrepareTestFiles(WIKIPEDIA_STR, WIKIPEDIA_HASH);
    filediff::Delta delta { m_signatureTestFile, m_dataTestFile };
    delta.Calculate();
    EXPECT_FALSE(delta.IsChanged());
}

// TODO: this unit test would fail after changing chunk size to something else than one line, it should be probably good
//  to consider changing the impl to make those parametrized where passed params would set expectations, might be easier to maintain
TEST_F(DeltaTestSuite, LineAddedOnFileBeginningTest)
{
    PrepareDataTestFile({ WIKIPEDIA_STR });
    PrepareSigTestFile({ WIKIPEDIA_HASH });
    // update data test file
    PrepareDataTestFile({ LOREM_IPSUM_STR, WIKIPEDIA_STR });

    DeltaTesting delta { m_signatureTestFile, m_dataTestFile };
    delta.Calculate();
    EXPECT_TRUE(delta.IsChanged());

    const auto& rawDelta { delta.GetRawDelta() };
    constexpr auto EXPECTED_NUMBER_OF_ENTRIES_IN_DELTA { 1 };
    ASSERT_EQ(EXPECTED_NUMBER_OF_ENTRIES_IN_DELTA, rawDelta.size());

    // verify if text was added //
    EXPECT_EQ(LOREM_IPSUM_HASH, rawDelta[0].first);
    EXPECT_EQ(LOREM_IPSUM_STR, rawDelta[0].second);
}

TEST_F(DeltaTestSuite, LineAddedOnFileEndTest)
{
    PrepareDataTestFile({ WIKIPEDIA_STR });
    PrepareSigTestFile({ WIKIPEDIA_HASH });
    // update data test file
    PrepareDataTestFile({ WIKIPEDIA_STR, LOREM_IPSUM_STR });

    DeltaTesting delta { m_signatureTestFile, m_dataTestFile };
    delta.Calculate();
    EXPECT_TRUE(delta.IsChanged());

    const auto& rawDelta { delta.GetRawDelta() };
    constexpr auto EXPECTED_NUMBER_OF_ENTRIES_IN_DELTA { 1 };
    ASSERT_EQ(EXPECTED_NUMBER_OF_ENTRIES_IN_DELTA, rawDelta.size());

    // verify if text was added //
    const auto it { rawDelta.cbegin() };
    ASSERT_TRUE(it != std::cend(rawDelta));
    EXPECT_EQ(LOREM_IPSUM_HASH, it->first);
    EXPECT_EQ(LOREM_IPSUM_STR, it->second);
}

TEST_F(DeltaTestSuite, MultipleDifferentLinesAddedInFileTest)
{
    PrepareDataTestFile({ WIKIPEDIA_STR });
    PrepareSigTestFile({ WIKIPEDIA_HASH });
    // update data test file
    PrepareDataTestFile({ WIKIPEDIA_STR, SOME_TEXT_STR, YET_ANOTHER_TEXT_STR, LOREM_IPSUM_STR });

    DeltaTesting delta { m_signatureTestFile, m_dataTestFile };
    delta.Calculate();
    EXPECT_TRUE(delta.IsChanged());

    const auto& rawDelta { delta.GetRawDelta() };

    const std::array<std::pair<std::uint32_t, std::string>, 3> EXPECTED_DELTA_COLLECTION {
        std::make_pair(SOME_TEXT_HASH, SOME_TEXT_STR),
        std::make_pair(YET_ANOTHER_TEXT_HASH, YET_ANOTHER_TEXT_STR),
        std::make_pair(LOREM_IPSUM_HASH, LOREM_IPSUM_STR)
    };
    ASSERT_EQ(EXPECTED_DELTA_COLLECTION.size(), rawDelta.size());

    // verify if text was added //
    for (auto i { 0U }; i < rawDelta.size(); ++i) {
        EXPECT_EQ(EXPECTED_DELTA_COLLECTION[i].first, rawDelta[i].first);
        EXPECT_EQ(EXPECTED_DELTA_COLLECTION[i].second, rawDelta[i].second);
    }
}

TEST_F(DeltaTestSuite, OneLineShiftedTest)
{
    PrepareDataTestFile({ WIKIPEDIA_STR, LOREM_IPSUM_STR });
    PrepareSigTestFile({ WIKIPEDIA_HASH, LOREM_IPSUM_HASH });
    // update data test file //
    PrepareDataTestFile({ LOREM_IPSUM_STR, WIKIPEDIA_STR });

    DeltaTesting delta { m_signatureTestFile, m_dataTestFile };
    delta.Calculate();
    EXPECT_TRUE(delta.IsChanged());

    const auto& rawDelta { delta.GetRawDelta() };
    constexpr auto EXPECTED_NUMBER_OF_ENTRIES_IN_DELTA { 1 };
    ASSERT_EQ(EXPECTED_NUMBER_OF_ENTRIES_IN_DELTA, rawDelta.size());

    // verify if lines have been moved within the file //
    const auto it { rawDelta.cbegin() };
    EXPECT_EQ(LOREM_IPSUM_HASH, it->first);
    EXPECT_EQ(LOREM_IPSUM_STR, it->second);
}

TEST_F(DeltaTestSuite, MultipleLinesShiftedTest)
{
    PrepareDataTestFile({ WIKIPEDIA_STR, SOME_TEXT_STR, LOREM_IPSUM_STR });
    PrepareSigTestFile({ WIKIPEDIA_HASH, SOME_TEXT_HASH, LOREM_IPSUM_HASH });
    // update data test file //
    PrepareDataTestFile({ YET_ANOTHER_TEXT_STR, LOREM_IPSUM_STR, SOME_TEXT_STR, WIKIPEDIA_STR });

    DeltaTesting delta { m_signatureTestFile, m_dataTestFile };
    delta.Calculate();
    EXPECT_TRUE(delta.IsChanged());

    const auto& rawDelta { delta.GetRawDelta() };
    const std::array<std::pair<std::uint32_t, std::string>, 4> EXPECTED_DELTA_COLLECTION {
        std::make_pair(YET_ANOTHER_TEXT_HASH, YET_ANOTHER_TEXT_STR),
        std::make_pair(SOME_TEXT_HASH, SOME_TEXT_STR),
        std::make_pair(WIKIPEDIA_HASH, WIKIPEDIA_STR),
        std::make_pair(LOREM_IPSUM_HASH, LOREM_IPSUM_STR)
    };
    constexpr auto EXPECTED_NUMBER_OF_ENTRIES_IN_DELTA { EXPECTED_DELTA_COLLECTION.size() };
    ASSERT_EQ(EXPECTED_NUMBER_OF_ENTRIES_IN_DELTA, rawDelta.size());

    // verify if lines have been moved within the file //
    for (auto i { 0U }; i < rawDelta.size(); ++i) {
        EXPECT_EQ(EXPECTED_DELTA_COLLECTION[i].first, rawDelta[i].first);
        EXPECT_EQ(EXPECTED_DELTA_COLLECTION[i].second, rawDelta[i].second);
    }
}
} // testing namespace
