#include <filesystem>
#include <GBA/include/GameBoyAdvance.hpp>

namespace fs = std::filesystem;

int main(int argc, char* argv[])
{
    fs::path biosPath = "";
    fs::path romPath = "";
    fs::path logDir = "";

    if (argc == 4)
    {
        for (int i = 1; i < argc; ++i)
        {
            switch (i)
            {
                case 1:
                    biosPath = argv[1];
                    break;
                case 2:
                    romPath = argv[2];
                    break;
                case 3:
                    logDir = argv[3];
                    break;
            }
        }
    }

    GameBoyAdvance gba(biosPath, romPath, logDir);
    return 0;
}
