#pragma once

#ifndef NOMINMAX
#    define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#    define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include <cstring>
#include <cstdint>
#include <stdexcept>

namespace ninfer::test::io {

// Test-only import interposition. Locate the importing module by a production function address,
// so this works both against ninfer_core.dll and in the standalone source-only test executable.
class ImportReplacement {
public:
    ImportReplacement(const void* module_address, const char* name, void* replacement) {
        // Without dllimport on the public declaration, MSVC takes the address of an executable
        // import thunk. Follow its indirect jump to find the DLL that actually imports ReadFile.
        auto* entry = static_cast<const unsigned char*>(module_address);
        for (unsigned i = 0; i < 4; ++i) {
            if (entry[0] != 0xff || entry[1] != 0x25) break;
            std::int32_t displacement;
            std::memcpy(&displacement, entry + 2, sizeof(displacement));
            const unsigned char* target;
            std::memcpy(&target, entry + 6 + displacement, sizeof(target));
            entry = target;
        }
        HMODULE module = nullptr;
        if (!::GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
                                      GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                                  reinterpret_cast<LPCWSTR>(entry), &module))
            throw std::runtime_error("cannot locate I/O module for interposition");
        const auto base = reinterpret_cast<unsigned char*>(module);
        const auto dos  = reinterpret_cast<const IMAGE_DOS_HEADER*>(base);
        const auto nt   = reinterpret_cast<const IMAGE_NT_HEADERS*>(base + dos->e_lfanew);
        const auto rva =
            nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;
        if (!rva) throw std::runtime_error("I/O module has no imports");
        auto* imports = reinterpret_cast<IMAGE_IMPORT_DESCRIPTOR*>(base + rva);
        for (; imports->Name; ++imports) {
            if (!imports->OriginalFirstThunk) continue;
            auto* names = reinterpret_cast<IMAGE_THUNK_DATA*>(base + imports->OriginalFirstThunk);
            auto* addresses = reinterpret_cast<IMAGE_THUNK_DATA*>(base + imports->FirstThunk);
            for (; names->u1.AddressOfData; ++names, ++addresses) {
                if (IMAGE_SNAP_BY_ORDINAL(names->u1.Ordinal)) continue;
                const auto entry =
                    reinterpret_cast<const IMAGE_IMPORT_BY_NAME*>(base + names->u1.AddressOfData);
                if (std::strcmp(reinterpret_cast<const char*>(entry->Name), name)) continue;
                address_  = reinterpret_cast<void**>(&addresses->u1.Function);
                original_ = *address_;
                set(replacement);
                return;
            }
        }
        throw std::runtime_error("cannot find I/O import for interposition");
    }

    ~ImportReplacement() {
        try {
            set(original_);
        } catch (...) { std::terminate(); }
    }

    ImportReplacement(const ImportReplacement&)            = delete;
    ImportReplacement& operator=(const ImportReplacement&) = delete;

private:
    void set(void* value) {
        DWORD protection;
        if (!::VirtualProtect(address_, sizeof(void*), PAGE_READWRITE, &protection))
            throw std::runtime_error("cannot unprotect I/O import");
        ::InterlockedExchangePointer(address_, value);
        DWORD ignored;
        if (!::VirtualProtect(address_, sizeof(void*), protection, &ignored))
            throw std::runtime_error("cannot restore I/O import protection");
    }

    void** address_ = nullptr;
    void* original_ = nullptr;
};

} // namespace ninfer::test::io
