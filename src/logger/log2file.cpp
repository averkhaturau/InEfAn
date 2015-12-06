#include "log2file.h"

#include <time.h>

void Log2File::enable(bool makeEnabled)
{
    try {
        if (makeEnabled != logfile.is_open()) {
            if (makeEnabled) {
                openFile(filename);
                logfile << "Enabled logging...\n";
            } else {
                logfile << "Disabling logging...\n";
                logfile.close();
            }
        }
    } catch (...) {
        close();
    }
}

void Log2File::flush()
{
    logfile.flush();
}

void Log2File::close()
{
    logfile.close();
}

bool Log2File::isEnabled() const
{
    return logfile.is_open();
}

std::tr2::sys::path Log2File::logFilename() const
{
    return filename;
}

std::streamoff Log2File::currFileSize()
{
    return logfile.tellp().seekpos();
}

Log2File::Log2File(const std::tr2::sys::path& filename)
{
    openFile(filename);
}

void Log2File::openFile(const std::tr2::sys::path& filename)
{
    if (logfile.is_open() && this->filename == filename)
        return;
    this->filename = filename;
    if (std::tr2::sys::exists(filename) ||
        std::tr2::sys::create_directories(filename.parent_path()))

        logfile.open(filename, std::ios::app);

    //   writeLogHeader();
}

Log2File::~Log2File()
{
    close();
}
