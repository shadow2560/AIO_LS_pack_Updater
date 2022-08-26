
/*

    Goldleaf - Multipurpose homebrew tool for Nintendo Switch
    Copyright (C) 2018-2022 XorTroll

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.

*/

#include "hos/hos_Content.hpp"

namespace hos {

    std::string ContentIdAsString(const NcmContentId &cnt_id) {
        char cnt_id_str[FS_MAX_PATH] = {0};
        const auto lower = __bswap64(*(u64*)cnt_id.c);
        const auto upper = __bswap64(*(u64*)(cnt_id.c + 0x8));
        snprintf(cnt_id_str, FS_MAX_PATH, "%016lx%016lx", lower, upper);
        return cnt_id_str;
    }

    NcmContentId StringAsContentId(const std::string &cnt_id_str) {
        NcmContentId cnt_id = {};
        char lower[0x20] = {0};
        char upper[0x20] = {0};
        memcpy(lower, cnt_id_str.c_str(), 0x10);
        memcpy(upper, cnt_id_str.c_str() + 0x10, 0x10);
        *(u64*)cnt_id.c = __bswap64(strtoul(lower, nullptr, 0x10));
        *(u64*)(cnt_id.c + 0x8) = __bswap64(strtoul(upper, nullptr, 0x10));
        return cnt_id;
    }

}