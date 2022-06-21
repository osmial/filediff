#include <gtest/gtest.h>
#include <iostream>
#include <fstream>
#include <utility>

#include "../adler32.h"
#include "../signature.h"

namespace testing {

static constexpr auto WIKIPEDIA_HASH{0x11E60398};
static constexpr auto LOREM_IPSUM_HASH{0xa05ca509};

static const std::string WIKIPEDIA_STR { "Wikipedia" };
static const std::string LOREM_IPSUM_STR { "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum." };

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
                     try {
                         const std::string path{"not_existing_file.txt"};
                         filediff::Signature signature(path, filediff::Signature::InputFileType::BASIS);
                     } catch(std::runtime_error& e) {
                         EXPECT_STREQ("File not found!", e.what());
                         throw;
                     }
                 }, std::runtime_error);
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

class SignatureCalculationTestSuite : public ::testing::TestWithParam<std::pair<std::string, uint32_t>> {


    // Test interface
protected:

    void PrepareTestFile(const std::string& data, uint32_t expectedHash) {
        const std::string testFile{"test.txt"};
        std::ofstream ofs{testFile};
        ofs << data;
        ofs.close();

        filediff::Signature signature{testFile, filediff::Signature::InputFileType::BASIS};
        auto hashes{signature.GetHashes()};
        EXPECT_EQ(1, hashes.size());
        EXPECT_EQ(expectedHash, hashes[0]);

        std::ofstream ofSigStream{m_signatureTestFile, std::ios::binary};
        if(ofSigStream.is_open()) {
            signature.Serialize(ofSigStream);
            ofSigStream.close();
        }
    }

    std::string m_signatureTestFile{"test.txt.sig"};
};

TEST_P(SignatureCalculationTestSuite, SignatureFileReadWriteSimpleTest) {
    auto params{GetParam()};
    PrepareTestFile(params.first, params.second);

    //test begins//
    filediff::Signature testSig{m_signatureTestFile, filediff::Signature::InputFileType::SIGNATURE};
    EXPECT_EQ(1, testSig.GetMetadata().m_numberOfChunks);
    EXPECT_EQ(1, testSig.GetMetadata().m_chunkLenght);
    EXPECT_EQ(1, testSig.GetHashes().size());
    EXPECT_EQ(params.second, testSig.GetHashes()[0]);
}

INSTANTIATE_TEST_SUITE_P(SignatureFileTests, SignatureCalculationTestSuite,
                                              ::testing::Values(std::pair{WIKIPEDIA_STR, WIKIPEDIA_HASH},
                                                                std::pair{LOREM_IPSUM_STR, LOREM_IPSUM_HASH}));
}
