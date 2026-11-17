#include "artifact/fixture.h"
#include "artifact/reader.h"
#include <iostream>

int main() {
    using namespace ninfer::test::artifact_fixture;
    try {
        Fixture fixture;
        const auto old = fixture.directory;
        const auto renamed = old.parent_path() / (old.filename().string() + " with spaces");
        std::filesystem::rename(old, renamed);
        fixture.directory = renamed;
        fixture.entry = renamed / "model.ninfer";
        // The framing contract accepts JSON whitespace, including CRLF.
        std::string text = fixture.root.dump(2);
        for (std::size_t at = 0; (at = text.find('\n', at)) != std::string::npos; at += 2)
            text.replace(at, 1, "\r\n");
        fixture.write(true, text);
        require(fixture.entry.is_absolute() && fixture.entry.has_root_name(),
                "fixture does not exercise a drive-qualified Windows path");
        {
            ninfer::artifact::Reader reader(fixture.entry);
            const auto bytes = reader.read_object(reader.find("q5"));
            require(bytes.size() == 528 &&
                        std::equal(bytes.begin(), bytes.end(), fixture.payload.begin() + 256),
                    "drive-qualified artifact path or CRLF directory changed bytes");
        }
        // Reader's ownership must release every file before Windows can remove it.
        require(std::filesystem::remove_all(renamed) >= 3,
                "Reader kept file handles alive after destruction");
        require(!std::filesystem::exists(renamed), "fixture cleanup left files behind");
        std::cout << "artifact drive, spaces, CRLF and handle lifetime passed\n";
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
