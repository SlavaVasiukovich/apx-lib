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
//=============================================================================
//#include <dmsg.h>
#include <cmath>
#include <string.h>
//-----------------------------------------------------------------------------
#include "MandalaCore.h"

using namespace matrixmath;
//=============================================================================
// AP constants
//=============================================================================
//#ifndef DEFAULT_LOOP_FREQ
//#define DEFAULT_LOOP_FREQ       200     // AHRS Update (main loop freq) [Hz]
//#endif
#define GPS_FREQ 5        // GPS Update rate (for derivatives) [Hz]
#define SIM_FREQ 10       // Simulator servo send rate [Hz]
#define MAX_TELEMETRY 128 // max telemetry packet size [bytes]
//=============================================================================
class Mandala : public MandalaCore
{
public:
    bool get_text_names(uint16_t varmsk, const char **name, const char **descr);
    const char *var_name(uint8_t var_idx);
    uint8_t var_index(const char *name);

    //---- Internal use -----
    bool blockDownstream;
    // telemetry framework
    bool dl_reset;                                                                          // set true to send everything next time
    uint8_t dl_snapshot[1024];                                                              // all archived variables snapshot
    uint8_t dl_reset_mask[(idx_local - idxPAD) / 8 + (((idx_local - idxPAD) & 3) ? 1 : 0)]; // bitmask 1=send var, auto clear after sent
    uint8_t dl_var[3 * 4];                                                                  // one var max size (tmp buf)
    //filled by extract_downstream
    uint32_t dl_frcnt;  // downlink frame cnt (inc by extract_downlink)
    uint32_t dl_errcnt; // errors counter (by extract_downlink)
    uint32_t dl_time_s; // last dl sent time[ms]
    uint16_t dl_ts;     // previous timestamp (to count dl_dt)
    uint16_t dl_Pdt;    // previous delta (to count errors)
    bool dl_e;          // if delta was different (errors counter)
    uint32_t dl_size;   // last telemetry size statistics
    bool dl_hd_save;    // to watch change in alt_bytecnt

    Quat quat; //quaternion from AHRS (bodyframe to NED)

    //=============================================================================
    Mandala();
    void init(void);
    //-----------------------------------------------------------------------------
    // Core overload
    uint32_t extract(const uint8_t *buf, uint32_t cnt, uint32_t var_idx);

    //-----------------------------------------------------------------------------
    //extended - check buf size
    uint32_t extract(const uint8_t *buf, uint32_t size); //overloaded - first byte=var_idx
    //-----------------------------------------------------------------------------

    // some special protocols
    uint32_t pack_downstream(uint8_t *buf, uint32_t maxSize, uint32_t timestamp);
    uint32_t unpack_downstream(const uint8_t *buf, uint32_t cnt);

    //-----------------------------------------------------------------------------
    // additional methods (debug, math, NAV helper functions)
    void dump(const uint8_t *ptr, uint32_t cnt, bool hex = true);
    void dump(const _var_vect &v, const char *str = "");
    void dump(uint8_t var_idx);

    // math operations
    static _var_float boundAngle(_var_float v, _var_float span = 180.0);
    static _var_float boundAngle360(_var_float v);
    static _var_vect boundAngle(const _var_vect &v, _var_float span = 180.0);
    _var_float smoothAngle(_var_float v, _var_float v_prev, _var_float speed);
    void filter_a(const _var_float &v, _var_float *var_p, const _var_float &f);
    uint32_t snap(uint32_t v, uint32_t snapv = 10);
    static _var_float hyst(_var_float err, _var_float hyst);
    static _var_float limit(const _var_float v, const _var_float vL = 1.0);
    static _var_float limit(const _var_float v, const _var_float vMin, const _var_float vMax);

    void calc(void); // calculate vars dependent on current and desired UAV position

    const _var_point lla2ne(const _var_vect &lla) const; // return NED from Lat Lon AGL
    const _var_point llh2ne(const _var_vect llh) const;
    const _var_point llh2ne(const _var_vect llh, const _var_vect home_llh) const;

    _var_float heading(const _var_point &ne,
                       bool back = false) const; //return heading to NED frm (0,0,0)
    _var_float heading(const _var_float N, const _var_float E, bool back = false) const;
    _var_float distance(const _var_point &ne) const; //return distance to to NED frm (0,0,0)
    _var_float distance(const _var_float N, const _var_float E) const;

    //LLH Earth navigation (great circle path)
    static _var_float bearing(const _var_point &ll1, const _var_point &ll2);
    static _var_float distance(const _var_point &ll1, const _var_point &ll2);
    static _var_float distance(const _var_point &ll1, const _var_point &ll2,
                               const _var_point &ll); //crosstrack
    static const _var_point destination(const _var_point &ll, const _var_float &bearing, const _var_float &distance);
    //rhumb LLH nav (straight line on mercator)
    static _var_float bearing_rhumb(const _var_point &ll1, const _var_point &ll2);
    static _var_float distance_rhumb(const _var_point &ll1, const _var_point &ll2);
    static const _var_point destination_rhumb(const _var_point &ll, const _var_float &bearing, const _var_float &distance);

    //helper nav
    const _var_point lineDistance(const _var_point p1, const _var_point p2) const;
    const _var_point rotate(const _var_point &v_in, const _var_float psi) const;
    const _var_point rotate(const _var_float N, const _var_float E, const _var_float psi) const;
    const _var_vect rotate(const _var_vect &v_in, const _var_float psi);
    const _var_vect rotate(const _var_vect &v_in, const _var_vect &theta);
    const _var_point LLH_dist(const _var_vect &llh1, const _var_vect &llh2, const _var_float lat, const _var_float lon) const;
    const _var_point ECEF_dist(const _var_vect &ecef1, const _var_vect &ecef2, const _var_float lat, const _var_float lon) const;
    const _var_point ECEF2Tangent(const _var_vect &ECEF, const _var_float latitude, const _var_float longitude) const;
    const _var_vect Tangent2ECEF(const _var_point &ne, const _var_float latitude, const _var_float longitude) const;
    const _var_point ECEF2ll(const _var_vect &ECEF) const;
    const _var_vect llh2ECEF(const _var_vect &llh) const;
    const _var_point ne2ll(const _var_point &ne) const;
    const _var_point ne2ll(const _var_point &ne, const _var_vect &home_llh) const;
    _var_float sqr(const _var_float x) const;
    _var_float wind_triangle(_var_float crs) const; //return TAS to gSpeed multiplier for given course
    _var_float wind_circle(_var_float crs, _var_float span,
                           _var_float r) const; //return air path length for ground circle flight

private:
    //math optimizations
    _var_vect rotate_s;
    _var_vect rotate_DC[3];
};
