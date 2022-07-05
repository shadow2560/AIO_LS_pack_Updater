#include <filesystem>
#include <string>

#include <switch.h>

#define PATH        "/switch/AIO_LS_pack_Updater/"
#define FULL_PATH   "/switch/AIO_LS_pack_Updater/AIO_LS_pack_Updater.nro"
#define CONFIG_PATH "/switch/AIO_LS_pack_Updater/temp.zip"
#define FORWARDER_PATH      "/switch/AIO_LS_pack_Updater/aiosu-forwarder.nro"

int removeDir(const char* path)
{
    Result ret = 0;
    FsFileSystem *fs = fsdevGetDeviceFileSystem("sdmc");
    if (R_FAILED(ret = fsFsDeleteDirectoryRecursively(fs, path))) {
        return ret;
    }
    return 0;
}

int main(int argc, char* argv[])
{
    std::filesystem::create_directory(PATH);
    if(std::filesystem::exists(CONFIG_PATH)){
        std::filesystem::create_directory(PATH);
        std::filesystem::remove(FULL_PATH);
        std::filesystem::rename(CONFIG_PATH, FULL_PATH);
    }

    std::filesystem::remove(FORWARDER_PATH);

    envSetNextLoad(FULL_PATH, ("\"" + std::string(FULL_PATH) + "\"").c_str());
    return 0;
}
