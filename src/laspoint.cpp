/******************************************************************************
 * $Id$
 *
 * Project:  libLAS - http://liblas.org - A BSD library for LAS format data.
 * Purpose:  LAS point class 
 * Author:   Mateusz Loskot, mateusz@loskot.net
 *
 ******************************************************************************
 * Copyright (c) 2008, Mateusz Loskot
 *
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following 
 * conditions are met:
 * 
 *     * Redistributions of source code must retain the above copyright 
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright 
 *       notice, this list of conditions and the following disclaimer in 
 *       the documentation and/or other materials provided 
 *       with the distribution.
 *     * Neither the name of the Martin Isenburg or Iowa Department 
 *       of Natural Resources nor the names of its contributors may be 
 *       used to endorse or promote products derived from this software 
 *       without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS 
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE 
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, 
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS 
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED 
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, 
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT 
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
 * OF SUCH DAMAGE.
 ****************************************************************************/

#include <liblas/laspoint.hpp>
#include <liblas/lasheader.hpp>
#include <liblas/cstdint.hpp>
#include <liblas/exception.hpp>
#include <liblas/detail/utility.hpp>
// std
#include <cstring>

namespace liblas {

Point::Point() :
    m_gpsTime(0),
    m_intensity(0),
    m_pointSourceId(0),
    m_flags(0),
    m_userData(0),
    m_angleRank(0),
    m_hdr(HeaderPtr())
{
    std::memset(m_coords, 0, sizeof(m_coords));
    m_extra_data.resize(0);
    m_format_data.resize(0);
}

Point::Point(Point const& other) :
    m_gpsTime(other.m_gpsTime),
    m_color(other.m_color),
    m_cls(other.m_cls),
    m_intensity(other.m_intensity),
    m_pointSourceId(other.m_pointSourceId),
    m_flags(other.m_flags),
    m_userData(other.m_userData),
    m_angleRank(other.m_angleRank),
    m_hdr(other.m_hdr)
{
    std::memcpy(m_coords, other.m_coords, sizeof(m_coords));
    std::vector<uint8_t>(other.m_extra_data).swap(m_extra_data);
    std::vector<uint8_t>(other.m_format_data).swap(m_format_data);
    
}

Point& Point::operator=(Point const& rhs)
{
    if (&rhs != this)
    {
        m_coords[0] = rhs.m_coords[0];
        m_coords[1] = rhs.m_coords[1];
        m_coords[2] = rhs.m_coords[2];
        m_intensity = rhs.m_intensity;
        m_flags = rhs.m_flags;
        m_cls = rhs.m_cls;
        m_angleRank = rhs.m_angleRank;
        m_userData = rhs.m_userData;
        m_pointSourceId = rhs.m_pointSourceId;
        m_gpsTime = rhs.m_gpsTime;
        m_color = rhs.m_color;
        std::vector<uint8_t>(rhs.m_extra_data).swap(m_extra_data);
        std::vector<uint8_t>(rhs.m_format_data).swap(m_format_data);
        m_hdr = rhs.m_hdr;
    }
    return *this;
}

void Point::SetCoordinates(Header const& header, double x, double y, double z)
{
    double const cx = (x * header.GetScaleX()) + header.GetOffsetX();
    double const cy = (y * header.GetScaleY()) + header.GetOffsetY();
    double const cz = (z * header.GetScaleZ()) + header.GetOffsetZ();

    SetCoordinates(cx, cy, cz);
}

void Point::SetReturnNumber(uint16_t const& num)
{
    // Store value in bits 0,1,2
    uint8_t mask = 0x7 << 0; // 0b00000111
    m_flags &= ~mask;
    m_flags |= mask & (static_cast<uint8_t>(num) << 0);

}

void Point::SetNumberOfReturns(uint16_t const& num)
{
    // Store value in bits 3,4,5
    uint8_t mask = 0x7 << 3; // 0b00111000
    m_flags &= ~mask;
    m_flags |= mask & (static_cast<uint8_t>(num) << 3);
}

void Point::SetScanDirection(uint16_t const& dir)
{
    // Store value in bit 6
    uint8_t mask = 0x1 << 6; // 0b01000000
    m_flags &= ~mask;
    m_flags |= mask & (static_cast<uint8_t>(dir) << 6);
}

void Point::SetFlightLineEdge(uint16_t const& edge)
{
    // Store value in bit 7
    uint8_t mask = 0x1 << 7; // 0b10000000
    m_flags &= ~mask;
    m_flags |= mask & (static_cast<uint8_t>(edge) << 7);}

void Point::SetScanAngleRank(int8_t const& rank)
{
    m_angleRank = rank;
}

void Point::SetUserData(uint8_t const& data)
{
    m_userData = data;
}

Classification const& Point::GetClassification() const
{
    return m_cls;
}

void Point::SetClassification(Classification const& cls)
{
    m_cls = cls;
}

void Point::SetClassification(Classification::bitset_type const& flags)
{
    m_cls = Classification(flags);
}

void Point::SetClassification(liblas::uint8_t const& flags)
{
    m_cls = Classification(flags);
}

bool Point::equal(Point const& other) const
{
    // TODO - mloskot: Default epsilon is too small.
    //                 Is 0.00001 good as tolerance or too wide?
    //double const epsilon = std::numeric_limits<double>::epsilon(); 
    double const epsilon = 0.00001;

    double const dx = m_coords[0] - other.m_coords[0];
    double const dy = m_coords[1] - other.m_coords[1];
    double const dz = m_coords[2] - other.m_coords[2];

    // TODO: Should we compare other data members, besides the coordinates?

    if (((dx <= epsilon) && (dx >= -epsilon))
        && ((dy <= epsilon) && (dy >= -epsilon))
        && ((dz <= epsilon) && (dz >= -epsilon)))
    {
        return true;
    }

    // If we do other members
    // bool compare_classification(uint8_t cls, uint8_t expected)
    // {
    //    // 31 is max index in classification lookup table
    //    clsidx = (cls & 31);
    //    assert(clsidx <= 31); 
    //    return (clsidx == expected);
    // }

    return false;
}

bool Point::Validate() const
{
    unsigned int flags = 0;

    if (this->GetReturnNumber() > 0x07)
        flags |= eReturnNumber;

    if (this->GetNumberOfReturns() > 0x07)
        flags |= eNumberOfReturns;

    if (this->GetScanDirection() > 0x01)
        flags |= eScanDirection;

    if (this->GetFlightLineEdge() > 0x01)
        flags |= eFlightLineEdge;

    if (eScanAngleRankMin > this->GetScanAngleRank()
        || this->GetScanAngleRank() > eScanAngleRankMax)
    {
        flags |= eScanAngleRank;
    }

    if (flags > 0)
    {
        throw invalid_point_data("point data members out of range", flags);
    }

    return true;
}


bool Point::IsValid() const
{
    
    if( eScanAngleRankMin > this->GetScanAngleRank() 
        || this->GetScanAngleRank() > eScanAngleRankMax
      )
        return false;

    if (this->GetFlightLineEdge() > 0x01)
        return false;

    if (this->GetScanDirection() > 0x01)
        return false;

    if (this->GetNumberOfReturns() > 0x07)
        return false;

    if (this->GetReturnNumber() > 0x07)
        return false;


    return true;
}


} // namespace liblas
