#include <cstdio>
#include <filesystem>
#include <fstream>
#include <vector>
#include <xl/model.hpp>
#include <xl/pack.hpp>
#include <xl/writer.hpp>

auto read_file(std::filesystem::path const&) -> std::vector<std::byte>;

int main()
{
    auto wb = xl::workbook{};
    wb.app_name = "My App";

    auto& sh = wb.sheets.emplace_back("sheet1");
    sh.columns[2] = xl::column{.width=20};
    
    {
        auto& r = sh.rows.emplace_back();
        r.cells.emplace_back("col1");
        r.cells.emplace_back("col2");
        r.cells.emplace_back("col3");
    }

    {
        auto& r = sh.rows.emplace_back();
        r.cells.emplace_back(1.0f);
        r.cells.emplace_back(2.0f);
        r.cells.emplace_back(3.0f);
    }

    {
        auto& r = sh.rows.emplace_back();
        r.height = 60;
        r.cells.emplace_back("A3").xf.alignment.vertical = "center";
        r.cells.emplace_back("B3").xf.alignment.vertical = "center";
        r.cells.emplace_back("C3").xf.alignment.vertical = "center";
    }

    {
        auto& r = sh.rows.emplace_back();
        r.height = 60;
        {
            auto fn = std::filesystem::path{"./media/sunset.jpeg"};
            auto blob = read_file(fn);
            r.cells.emplace_back(xl::cell_picture{
                .ext = fn.extension().string(),
                .blob = blob,
            });
        }
        {
            auto fn = std::filesystem::path{"./media/teapot.jpg"};
            auto blob = read_file(fn);
            r.cells.emplace_back(xl::cell_picture{
                .ext = fn.extension().string(),
                .blob = blob,
            });
        }
        {
            auto fn = std::filesystem::path{"./media/alarm.png"};
            auto blob = read_file(fn);
            r.cells.emplace_back(xl::cell_picture{
                .ext = fn.extension().string(),
                .blob = blob,
            });
        }
    }

    auto w = xl::writer();
    w.write(wb);

    auto const rootdir = std::string{"./testdata/t1"};

    for (auto const& [name, blob] : w.files) {
        auto fn = std::filesystem::path{rootdir + name};
        auto dir = fn.parent_path();
        std::filesystem::create_directories(dir);
        printf("writing: %s\n", fn.string().c_str());
        auto out = std::ofstream{fn, std::ios_base::out | std::ios::binary};
        out.write(reinterpret_cast<char const*>(blob.data()), blob.size());
    }

    std::vector<char> blob;
    xl::pack(blob, w.files);

    auto fn = "./testdata/t1.xlsx";
    std::ofstream output_file(fn, std::ios::binary);
    if (!output_file) {
        printf("Failed to create output file\n");
        return -1;
    }

    output_file.write(static_cast<const char*>(blob.data()), blob.size());
    output_file.close();

    printf("mission accomplished\n");
}

auto read_file(std::filesystem::path const& filename) -> std::vector<std::byte>
{
    std::ifstream file(filename, std::ios::binary | std::ios::ate);

    if (!file.is_open())
        return {};

    auto size = file.tellg();
    file.seekg(0, std::ios::beg);

    auto buffer = std::vector<std::byte>(size);

    if (!file.read(reinterpret_cast<char*>(buffer.data()), size))
        return {};

    return buffer;
}
