#pragma once
#include "XbusNode.h"
#include "XbusStreamReader.h"
#include "XbusStreamWriter.h"

namespace xbus {
namespace mission {

typedef char title_t[16];

struct file_hdr_s
{
    uint32_t size;
    xbus::node::hash_t hash;

    title_t title;

    struct stats_s
    {
        uint16_t wp;
        uint16_t rw;
        uint16_t tw;
        uint16_t pi;
        uint16_t avoid;
        uint16_t emg;
    };
    stats_s cnt;
    stats_s offset;

    static inline uint16_t psize()
    {
        return sizeof(uint32_t) + sizeof(xbus::node::hash_t) + sizeof(title_t) + sizeof(uint16_t) * 6 * 2;
    }
    inline void read(XbusStreamReader *s)
    {
        *s >> size;
        *s >> hash;
        s->read(title, sizeof(title));
        *s >> cnt.wp;
        *s >> cnt.rw;
        *s >> cnt.tw;
        *s >> cnt.pi;
        *s >> cnt.avoid;
        *s >> cnt.emg;
        *s >> offset.wp;
        *s >> offset.rw;
        *s >> offset.tw;
        *s >> offset.pi;
        *s >> offset.avoid;
        *s >> offset.emg;
    }
    inline void write(XbusStreamWriter *s) const
    {
        *s << size;
        *s << hash;
        s->write(title, sizeof(title));
        *s << cnt.wp;
        *s << cnt.rw;
        *s << cnt.tw;
        *s << cnt.pi;
        *s << cnt.avoid;
        *s << cnt.emg;
        *s << offset.wp;
        *s << offset.rw;
        *s << offset.tw;
        *s << offset.pi;
        *s << offset.avoid;
        *s << offset.emg;
    }
};

struct Header
{
    uint8_t type;   //wp,rw,scr, ..
    uint8_t option; //left,right,line,hdg, ..

    enum itemtypes_e { //4bits
        mi_stop = 0,
        mi_wp,
        mi_rw,
        mi_tw,
        mi_pi,
        mi_action,
        mi_avoid,
        mi_emg
    };

    static inline uint16_t psize()
    {
        return sizeof(uint8_t);
    }
    inline void read(XbusStreamReader *s)
    {
        uint8_t v;
        *s >> v;
        type = v & 0x0F;
        option = v >> 4;
    }
    inline void write(XbusStreamWriter *s) const
    {
        uint8_t v = (type & 0x0F) | ((option << 4) & 0xF0);
        *s << v;
    }
};

struct Waypoint
{
    float lat;
    float lon;
    int16_t alt;

    enum options_e {
        Direct,
        Path,
    };

    static inline uint16_t psize()
    {
        return sizeof(float) * 2
               + sizeof(int16_t);
    }
    inline void read(XbusStreamReader *s)
    {
        *s >> lat;
        *s >> lon;
        *s >> alt;
    }
    inline void write(XbusStreamWriter *s) const
    {
        *s << lat;
        *s << lon;
        *s << alt;
    }
};

struct Runway
{
    float lat;
    float lon;
    int16_t hmsl;
    int16_t dN;
    int16_t dE;
    uint16_t approach;

    enum options_e {
        Left,
        Right,
    };

    static inline uint16_t psize()
    {
        return sizeof(float) * 2
               + sizeof(int16_t)
               + sizeof(int16_t)
               + sizeof(int16_t)
               + sizeof(uint16_t);
    }
    inline void read(XbusStreamReader *s)
    {
        *s >> lat;
        *s >> lon;
        *s >> hmsl;
        *s >> dN;
        *s >> dE;
        *s >> approach;
    }
    inline void write(XbusStreamWriter *s) const
    {
        *s << lat;
        *s << lon;
        *s << hmsl;
        *s << dN;
        *s << dE;
        *s << approach;
    }
};

struct Taxiway
{
    float lat;
    float lon;

    static inline uint16_t psize()
    {
        return sizeof(float) * 2;
    }
    inline void read(XbusStreamReader *s)
    {
        *s >> lat;
        *s >> lon;
    }
    inline void write(XbusStreamWriter *s) const
    {
        *s << lat;
        *s << lon;
    }
};

struct Poi
{
    float lat;
    float lon;
    int16_t hmsl;
    int16_t turnR;
    uint8_t loops;
    uint16_t timeS;

    static inline uint16_t psize()
    {
        return sizeof(float) * 2
               + sizeof(int16_t)
               + sizeof(int16_t)
               + sizeof(uint8_t)
               + sizeof(uint16_t);
    }
    inline void read(XbusStreamReader *s)
    {
        *s >> lat;
        *s >> lon;
        *s >> hmsl;
        *s >> turnR;
        *s >> loops;
        *s >> timeS;
    }
    inline void write(XbusStreamWriter *s) const
    {
        *s << lat;
        *s << lon;
        *s << hmsl;
        *s << turnR;
        *s << loops;
        *s << timeS;
    }
};

struct Area
{
    uint8_t pointsCnt;

    struct Point
    {
        float lat;
        float lon;

        static inline uint16_t psize()
        {
            return sizeof(float) * 2;
        }
        inline void read(XbusStreamReader *s)
        {
            *s >> lat;
            *s >> lon;
        }
        inline void write(XbusStreamWriter *s) const
        {
            *s << lat;
            *s << lon;
        }
    };

    static inline uint16_t psize(uint8_t pointsCnt)
    {
        return sizeof(uint8_t) + Point::psize() * pointsCnt;
    }
    inline void read(XbusStreamReader *s)
    {
        *s >> pointsCnt;
    }
    inline void write(XbusStreamWriter *s) const
    {
        *s << pointsCnt;
    }
};

// Actions

struct Action
{
    enum options_e {
        mo_speed,
        mo_poi,
        mo_scr,
        mo_loiter,
        mo_shot,
    };
};

struct ActionSpeed
{
    uint8_t speed; //0=cruise

    static inline uint16_t psize()
    {
        return sizeof(uint8_t);
    }
    inline void read(XbusStreamReader *s)
    {
        *s >> speed;
    }
    inline void write(XbusStreamWriter *s) const
    {
        *s << speed;
    }
};

struct ActionPoi
{
    uint8_t poi; //linked POI [1...n]

    static inline uint16_t psize()
    {
        return sizeof(uint8_t);
    }
    inline void read(XbusStreamReader *s)
    {
        *s >> poi;
    }
    inline void write(XbusStreamWriter *s) const
    {
        *s << poi;
    }
};

struct ActionScr
{
    typedef char scr_t[16]; //public func @name
    scr_t scr;

    static inline uint16_t psize()
    {
        return sizeof(scr_t);
    }
    inline void read(XbusStreamReader *s)
    {
        s->read(scr, sizeof(scr));
    }
    inline void write(XbusStreamWriter *s) const
    {
        s->write(scr, sizeof(scr));
    }
};

struct ActionLoiter
{
    int16_t turnR;
    uint8_t loops;  //loops to loiter
    uint16_t timeS; //time to loiter

    static inline uint16_t psize()
    {
        return sizeof(int16_t) + sizeof(uint8_t) + sizeof(uint16_t);
    }
    inline void read(XbusStreamReader *s)
    {
        *s >> turnR;
        *s >> loops;
        *s >> timeS;
    }
    inline void write(XbusStreamWriter *s) const
    {
        *s << turnR;
        *s << loops;
        *s << timeS;
    }
};

struct ActionShot
{
    uint16_t dist; //distance for series
    uint8_t opt;   //0=single,1=start,2=stop

    static inline uint16_t psize()
    {
        return sizeof(uint16_t);
    }
    inline void read(XbusStreamReader *s)
    {
        uint16_t v;
        *s >> v;
        dist = v & 0x0FFF;
        opt = v >> 12;
    }
    inline void write(XbusStreamWriter *s) const
    {
        uint16_t v = (dist & 0x0FFF) | ((opt << 12) & 0xF000);
        *s << v;
    }
};

inline constexpr uint16_t action_psize(uint8_t option)
{
    switch (option) {
    case Action::mo_speed:
        return ActionSpeed::psize();
    case Action::mo_poi:
        return ActionPoi::psize();
    case Action::mo_scr:
        return ActionScr::psize();
    case Action::mo_loiter:
        return ActionLoiter::psize();
    case Action::mo_shot:
        return ActionShot::psize();
    }
    return 0;
}

} // namespace mission
} // namespace xbus
