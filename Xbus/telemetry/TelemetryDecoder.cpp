#include "TelemetryDecoder.h"
#include <crc/crc.h>

#include "TelemetryValueUnpack.h"

using namespace xbus::telemetry;

TelemetryDecoder::TelemetryDecoder()
{
    memset(&_slots, 0, sizeof(_slots));
}

bool TelemetryDecoder::decode(const xbus::pid_s &pid, XbusStreamReader &stream)
{
    if (stream.available() < xbus::telemetry::stream_s::psize())
        return false;

    xbus::telemetry::stream_s hdr;
    hdr.read(&stream);

    uint8_t seq = static_cast<uint8_t>(hdr.spec.seq << 2) | pid.seq;
    bool seq_ok = (seq - _seq) == 1;
    _seq = seq;

    _rate = hdr.spec.rate;

    uint8_t &h = _hash.byte[seq & 3];
    if (h != hdr.feed_hash) {
        if (h)
            reset_fmt();
        h = hdr.feed_hash;
        _hash_valid++;
    } else if (!_is_hash_valid())
        _hash_valid++;

    // check seq and fmt feed
    if (!seq_ok) {
        _fmt_pos = 0;
        _cobs_code = 0;
    }
    _set_feed_fmt(hdr.feed_fmt);

    _valid = false;

    if (!_is_hash_valid())
        return false;

    if (!_slots_cnt)
        return false;

    // fmt looks consistent
    _valid = true;
    return decode_values(stream);
}

void TelemetryDecoder::reset_fmt()
{
    _hash.hash = 0;
    _hash_valid = 0;
    _slots_cnt = 0;
    _valid = false;
    memset(_slots.flags, 0, sizeof(_slots.flags));
    memset(_slots.value, 0, sizeof(_slots.value));
}

void TelemetryDecoder::_set_feed_fmt(uint8_t v)
{
    if (_fmt_pos == 0 && _cobs_code == 0) {
        if (v != 0) // wait for SOF
            return;
        _cobs_code = 0xFF;
        _cobs_copy = 0;
        return;
    }
    uint8_t *buf = reinterpret_cast<uint8_t *>(_slots.fields);

    if (v == 0) {
        // packet delimiter
        do {
            if (_cobs_copy)
                break;
            if (_fmt_pos < (sizeof(field_s) + 1) || ((_fmt_pos - 1) % sizeof(field_s)))
                break;
            // check crc_xor
            uint8_t crc_xor = 0;
            for (size_t i = 0; i < _fmt_pos; ++i)
                crc_xor ^= buf[i];
            if (crc_xor)
                break;
            buf[--_fmt_pos] = 0;
            // check hash
            uint32_t vhash = CRC_32_APX(_slots.fields, _fmt_pos, 0);
            if (!_is_hash_valid() || vhash != _hash.hash)
                break;
            // fmt array ok
            _slots_cnt = _fmt_pos / sizeof(field_s);
            _fmt_pos = 0;
            _cobs_code = 0;
            return;
        } while (0);
        // packet error
        if (_fmt_pos) {
            _slots_cnt = 0;
            _fmt_pos = 0;
        }
        _cobs_code = 0;
        return;
    }
    bool skip = false;
    if (_cobs_copy == 0) {
        skip = _cobs_code == 0xFF;
        _cobs_copy = _cobs_code = v;
        v = 0;
    }
    _cobs_copy--;

    if (skip)
        return;

    if (buf[_fmt_pos] != v) {
        buf[_fmt_pos] = v;
        if (_fmt_pos < (_slots_cnt * sizeof(field_s)))
            _slots_cnt = 0;
    }
    _fmt_pos++;
}

uint8_t TelemetryDecoder::rate_hz()
{
    switch (_rate) {
    default:
        return 0;
    case rate_10Hz:
        return 10;
    case rate_5Hz:
        return 5;
    case rate_1Hz:
        return 1;
    case rate_ts:
        return 0;
    }
}

bool TelemetryDecoder::decode_values(XbusStreamReader &stream)
{
    if (stream.available() == 0)
        return false;

    uint8_t cnt = stream.read<uint8_t>();
    uint8_t code = 0;
    uint8_t code_bit = 0x80;

    bool upd = false;
    for (size_t i = 0; i < _slots_cnt; ++i) {
        auto const &f = _slots.fields[i];

        if (!cnt)
            break;

        if (code_bit == 0x80) {
            code = stream.read<uint8_t>();
            code_bit = 1;
        } else
            code_bit <<= 1;

        if (stream.available() == 0)
            break;

        if (!(code & code_bit))
            continue;

        auto &value = _slots.value[i];
        mandala::type_id_e type;
        size_t sz = unpack_value(stream.ptr(), &value, &type, f.fmt, stream.available());
        if (!sz || sz > stream.available())
            break;
        stream.reset(stream.pos() + sz);

        auto &flags = _slots.flags[i];
        flags.type = type;
        flags.upd = true;
        upd = true;

        cnt--;

        if (stream.available() == 0 || cnt == 0)
            break;
    }

    if (stream.available() != 0 || cnt != 0) {
        //reset_fmt();
        return false;
    }
    return upd;
}
