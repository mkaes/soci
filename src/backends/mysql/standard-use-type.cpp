//
// Copyright (C) 2004-2006 Maciej Sobczak, Stephen Hutton
// MySQL backend copyright (C) 2006 Pawel Aleksander Fedorynski
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#define SOCI_MYSQL_SOURCE
#include "soci/mysql/soci-mysql.h"
#include "common.h"
#include "soci/soci-platform.h"
#include "soci-dtocstr.h"
#include "soci-exchange-cast.h"
// std
#include <ciso646>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <limits>

using namespace soci;
using namespace soci::details;
using namespace soci::details::mysql;


void mysql_standard_use_type_backend::bind_by_pos(
    int &position, void *data, exchange_type type, bool /* readOnly */)
{
    data_ = data;
    type_ = type;
    position_ = position++;
}

void mysql_standard_use_type_backend::bind_by_name(
    std::string const &name, void *data, exchange_type type, bool /* readOnly */)
{
    data_ = data;
    type_ = type;
    name_ = name;
}

void mysql_standard_use_type_backend::pre_use(indicator const *ind)
{
    if (ind != NULL && *ind == i_null)
    {
        buf_ = new char[5];
        std::strcpy(buf_, "NULL");
    }
    else
    {
        // allocate and fill the buffer with text-formatted client data
        switch (type_)
        {
        case x_char:
            {
                char buf[] = { exchange_type_cast<x_char>(data_), '\0' };
                buf_ = quote(statement_.session_.conn_, buf, 1);
            }
            break;
        case x_stdstring:
            {
                std::string const& s = exchange_type_cast<x_stdstring>(data_);
                buf_ = quote(statement_.session_.conn_,
                             s.c_str(), s.size());
            }
            break;
        case x_int8:
            {
                std::size_t const bufSize
                    = std::numeric_limits<int8_t>::digits10 + 3;
                buf_ = new char[bufSize];
                snprintf(buf_, bufSize, "%d", exchange_type_cast<x_int8>(data_));
            }
            break;
        case x_uint8:
            {
                std::size_t const bufSize
                    = std::numeric_limits<uint8_t>::digits10 + 3;
                buf_ = new char[bufSize];
                snprintf(buf_, bufSize, "%u", exchange_type_cast<x_uint8>(data_));
            }
            break;
        case x_int16:
            {
                std::size_t const bufSize
                    = std::numeric_limits<int16_t>::digits10 + 3;
                buf_ = new char[bufSize];
                snprintf(buf_, bufSize, "%d", exchange_type_cast<x_int16>(data_));
            }
            break;
        case x_uint16:
            {
                std::size_t const bufSize
                    = std::numeric_limits<uint16_t>::digits10 + 3;
                buf_ = new char[bufSize];
                snprintf(buf_, bufSize, "%u", exchange_type_cast<x_uint16>(data_));
            }
            break;
        case x_int32:
            {
                std::size_t const bufSize
                    = std::numeric_limits<int32_t>::digits10 + 3;
                buf_ = new char[bufSize];
                snprintf(buf_, bufSize, "%d", exchange_type_cast<x_int32>(data_));
            }
            break;
        case x_uint32:
            {
                std::size_t const bufSize
                    = std::numeric_limits<uint32_t>::digits10 + 3;
                buf_ = new char[bufSize];
                snprintf(buf_, bufSize, "%u", exchange_type_cast<x_uint32>(data_));
            }
            break;
        case x_int64:
            {
                std::size_t const bufSize
                    = std::numeric_limits<int64_t>::digits10 + 3;
                buf_ = new char[bufSize];
                snprintf(buf_, bufSize, "%" LL_FMT_FLAGS "d",
                    static_cast<long long>(exchange_type_cast<x_int64>(data_)));
            }
            break;
        case x_uint64:
            {
                std::size_t const bufSize
                    = std::numeric_limits<uint64_t>::digits10 + 3;
                buf_ = new char[bufSize];
                snprintf(buf_, bufSize, "%" LL_FMT_FLAGS "u",
                    static_cast<unsigned long long>(exchange_type_cast<x_uint64>(data_)));
            }
            break;

        case x_double:
            {
                double const d = exchange_type_cast<x_double>(data_);
                if (is_infinity_or_nan(d)) {
                    throw soci_error(
                        "Use element used with infinity or NaN, which are "
                        "not supported by the MySQL server.");
                }

                std::string const s = double_to_cstring(d);

                buf_ = new char[s.size() + 1];
                std::strcpy(buf_, s.c_str());
            }
            break;
        case x_stdtm:
            {
                std::size_t const bufSize = 80;
                buf_ = new char[bufSize];

                std::tm const& t = exchange_type_cast<x_stdtm>(data_);
                snprintf(buf_, bufSize,
                    "\'%d-%02d-%02d %02d:%02d:%02d\'",
                    t.tm_year + 1900, t.tm_mon + 1, t.tm_mday,
                    t.tm_hour, t.tm_min, t.tm_sec);
            }
            break;
        default:
            throw soci_error("Use element used with non-supported type.");
        }
    }

    if (position_ > 0)
    {
        // binding by position
        statement_.useByPosBuffers_[position_] = &buf_;
    }
    else
    {
        // binding by name
        statement_.useByNameBuffers_[name_] = &buf_;
    }
}

void mysql_standard_use_type_backend::post_use(bool /*gotData*/, indicator* /*ind*/)
{
    // TODO: Is it possible to have the bound element being overwritten
    // by the database?
    // If not, then nothing to do here, please remove this comment.
    // If yes, then use the value of the readOnly parameter:
    // - true:  the given object should not be modified and the backend
    //          should detect if the modification was performed on the
    //          isolated buffer and throw an exception if the buffer was modified
    //          (this indicates logic error, because the user used const object
    //          and executed a query that attempted to modified it)
    // - false: the modification should be propagated to the given object.
    // ...

    clean_up();
}

void mysql_standard_use_type_backend::clean_up()
{
    if (buf_ != NULL)
    {
        delete [] buf_;
        buf_ = NULL;
    }
}
