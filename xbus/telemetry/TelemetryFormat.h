/*
 * APX Autopilot project <http://docs.uavos.com>
 *
 * Copyright (c) 2003-2020, Aliaksei Stratsilatau <sa@uavos.com>
 * All rights reserved
 *
 * This file is part of APX Shared Libraries.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#pragma once

#include <xbus/XbusPacket.h>
#include <xbus/XbusStreamReader.h>
#include <xbus/XbusStreamWriter.h>

namespace xbus {
namespace telemetry {

#pragma pack(1)

// data element format descriptor
enum fmt_e {
    fmt_none,

    // raw
    fmt_real,
    fmt_dword,
    fmt_word,
    fmt_byte,

    // packed bitfields
    fmt_bit, // one bit
    fmt_opt, // 4 bits

    // packed real numbers
    fmt_f16,       // float16
    fmt_sbyte,     // signed byte
    fmt_sbyte_10,  // signed/10
    fmt_sbyte_01,  // signed*10
    fmt_sbyte_001, // signed*100
    fmt_byte_10,   // unsigned/10
    fmt_byte_01,   // unsigned*10
    fmt_byte_001,  // unsigned*100
    fmt_rad,       // radians -PI..+PI
    fmt_rad2,      // radians -PI/2..+PI/2
    fmt_byte_u,    // units 0..1
    fmt_sbyte_u,   // signed units -1..+1
    fmt_word_10,   // unsigned/10
};

enum seq_e {
    seq_always,
    seq_skip,
    seq_rare,
    seq_scheduled
};

enum dt_e {
    dt_10ms,  // 100 Hz
    dt_20ms,  // 50 Hz
    dt_50ms,  // 20 Hz
    dt_100ms, // 10 Hz
    dt_200ms, // 5 Hz
    dt_1s,    // 1 Hz
    dt_5s,
    dt_off,
};
static constexpr uint32_t dt_ms(dt_e dt)
{
    switch (dt) {
    case dt_10ms:
        return 10;
    case dt_20ms:
        return 20;
    case dt_50ms:
        return 50;
    case dt_100ms:
        return 100;
    case dt_200ms:
        return 200;
    case dt_1s:
        return 1000;
    case dt_5s:
        return 5000;
    case dt_off:
        return 0;
    }
    return 0;
}

struct field_s
{
    xbus::pid_s pid; // seq = seq_e skip mode
    fmt_e fmt : 8;
    // uint8_t _rsv : 4;
};
static_assert(sizeof(field_s) == 3, "size error");

union hash_s {
    uint32_t hash;
    uint8_t byte[4];
};
static_assert(sizeof(hash_s) == 4, "size error");

static constexpr const size_t fmt_block_size = 64;

static constexpr const size_t slots_size{240};

// stream header
struct stream_s
{
    union {
        uint32_t _raw;
        struct
        {
            uint32_t seq : 29;
            dt_e dt : 3;
        };
    } spec;
    uint8_t feed_hash; // dataset structure hash_32 (pid.seq = byte no)
    uint8_t feed_fmt;  // dataset format COBS encoded feed

    static constexpr inline uint16_t psize() { return 6; }
    inline void read(XbusStreamReader *s)
    {
        *s >> spec._raw;
        *s >> feed_hash;
        *s >> feed_fmt;
    }
    inline void write(XbusStreamWriter *s) const
    {
        *s << spec._raw;
        *s << feed_hash;
        *s << feed_fmt;
    }
};
static_assert(sizeof(stream_s) == stream_s::psize(), "size error");

#pragma pack()

} // namespace telemetry
} // namespace xbus
