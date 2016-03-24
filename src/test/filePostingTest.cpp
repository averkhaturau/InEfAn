#include <string>
#include <iostream>
#include "../InEfAn-tray/backend-bridge.h"
#include <comdef.h>


using namespace std::tr2::sys;

namespace
{
    const std::string testString = "жопастекстом";
}

void createTestFile(const path& tempfile)
{
    create_directories(tempfile.parent_path());
    std::ofstream(tempfile) << testString;
}

std::string readTestFile(const path& tempfile)
{
    return readFileToString<std::string>(tempfile);
}

int main()
{

    const path tempFile = temp_directory_path() / "inefan-test" / "test.txt";
    createTestFile(tempFile);
    const std::string readStr = readTestFile(tempFile);
    assert(readStr == testString);

    const path issueFile = temp_directory_path() / "inefan-test" / "filePostingTest.txt";

    auto str = readFileToString<std::string>(issueFile);
    auto based = encodeToBase64<std::wstring>(str);
    auto debased = decodeFromBase64<std::string>(based);
    std::ofstream(path(issueFile).replace_extension(".copy.txt"), std::ios::binary) << debased;

    std::ofstream(path(issueFile).replace_extension(".copy.txt")) <<
            decodeFromBase64<std::string>(encodeToBase64<std::wstring>(readFileToString<std::string>(issueFile)));



}