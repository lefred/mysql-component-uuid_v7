/* Copyright (c) 2017, 2023, Oracle and/or its affiliates. All rights reserved.
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License, version 2.0,
  as published by the Free Software Foundation.
  This program is also distributed with certain software (including
  but not limited to OpenSSL) that is licensed under separate terms,
  as designated in a particular file or component or in included license
  documentation.  The authors of MySQL hereby grant you an additional
  permission to link the program and your derivative works with the
  separately licensed software that they have included with MySQL.
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License, version 2.0, for more details.
  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

#define LOG_COMPONENT_TAG "uuid_v7"

#define UUID_T_LENGTH (16)
#define UNIX_TS_LENGTH (6)
#define RAND_A_LENGTH (2)
#define RAND_B_LENGTH (8)

#include <mysql/components/component_implementation.h>
#include <mysql/components/services/log_builtins.h> /* LogComponentErr */
#include <mysqld_error.h>                           /* Errors */
#include <mysql/components/services/udf_metadata.h>
#include <mysql/components/services/udf_registration.h>
#include <mysql/components/services/mysql_runtime_error_service.h>

#include <list>
#include <string>

#include <openssl/rand.h>  // RAND_bytes

#include <ctime>
#include <iostream>
#include <iomanip>


typedef uint8_t uuid_t[UUID_T_LENGTH];

static struct {
    uint64_t  unix_ts;
    uint8_t   rand_a[RAND_A_LENGTH];
    uint8_t   rand_b[RAND_B_LENGTH];
} state;

uint64_t get_milliseconds(void)
{
    struct timespec tp;
    clock_gettime(CLOCK_REALTIME, &tp);
    return ((tp.tv_sec * 1000) + (tp.tv_nsec / 1000000));
}

bool is_hex_char(char c) {
    return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
}

unsigned char hex_to_byte(char c) {
    if (c >= '0' && c <= '9') {
        return c - '0';
    } else if (c >= 'a' && c <= 'f') {
        return c - 'a' + 10;
    } else if (c >= 'A' && c <= 'F') {
        return c - 'A' + 10;
    }
    mysql_error_service_emit_printf(mysql_service_mysql_runtime_error,
           ER_UDF_ERROR, 0, "uuid_v7_to_timestamp", "Invalid hex character");
    return 0;
}

// Return human readable format like
// 2023-08-11 08:08:03.373
std::string get_timestamp(uint64_t milliseconds) {
    std::time_t seconds = milliseconds / 1000;
    std::tm timeinfo;
#ifdef _WIN32
    localtime_s(&timeinfo, &seconds); // Use localtime_s for Windows
#else
    localtime_r(&seconds, &timeinfo); // Use localtime_r for Linux/Unix
#endif

    std::ostringstream oss;
    oss << std::put_time(&timeinfo, "%Y-%m-%d %H:%M:%S") 
	    << '.' << std::setfill('0') << std::setw(3) << milliseconds % 1000;
    return oss.str();
}

// Return longer human readable format like
// Fri Aug 11 08:08:03 2023 CEST
std::string get_timestamp_long(uint64_t milliseconds) {
    std::time_t seconds = milliseconds / 1000;
    std::tm timeinfo;
#ifdef _WIN32
    localtime_s(&timeinfo, &seconds); // Use localtime_s for Windows
#else
    localtime_r(&seconds, &timeinfo); // Use localtime_r for Linux/Unix
#endif

    std::ostringstream oss;
    oss << std::put_time(&timeinfo, "%c %Z");

    return oss.str();
}

void get_random_bytes(uint8_t buffer[], size_t len)
{
    if (!RAND_bytes(buffer, len))
    {
      	mysql_error_service_emit_printf(mysql_service_mysql_runtime_error,
	  ER_GET_ERRMSG, 0, errno, "impossible to generate a random number", "uuid_v7");
    }
}

void create_uuid7(uuid_t uuid)
{
    // get the timestamp bytes
    uint64_t unix_ts = get_milliseconds();
    for(int i = 0; i < UNIX_TS_LENGTH; i++)
    {
        uuid[UNIX_TS_LENGTH - 1 - i] = (uint8_t) (unix_ts >> (8 * i));
    }

    // get the rand_a bytes
    if (unix_ts > state.unix_ts) {
        get_random_bytes(state.rand_a, RAND_A_LENGTH);
    } else {
		// increment rand_a bytes from right to left
		for (int i = RAND_A_LENGTH - 1; i >= 0; i--) {
			if (++state.rand_a[i] != 0x00) {
				break;
			}
		}
    }
    for(int i = 0; i < RAND_A_LENGTH; i++)
    {
        uuid[UNIX_TS_LENGTH + i] = state.rand_a[i];
    }

    // get the rand_b bytes
    get_random_bytes(state.rand_b, RAND_B_LENGTH);
    for(int i = 0; i < RAND_B_LENGTH; i++)
    {
        uuid[UNIX_TS_LENGTH + RAND_A_LENGTH + i] = state.rand_b[i];
    }

    // save the last timestamp
    state.unix_ts = unix_ts;

    // set version and variant
    uuid[6] = 0x70 | (uuid[6] & 0x0f); // version 7
    uuid[8] = 0x80 | (uuid[8] & 0x3f); // variant 2
}

std::string uuid_to_string(uuid_t uuid)
{
    static const char hex[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};

    int s = 0;
    char str[36 + 1];
    for(int i = 0; i < UUID_T_LENGTH; i++)
    {
        if(i == 4 || i == 6 || i == 8 || i == 10)
        {
            str[s++] = '-';
        }
        str[s++] = hex[uuid[i] >> 4];
        str[s++] = hex[uuid[i] & 0x0f];
    }
    str[s++] ='\0';
    std::string out(str);
    return out;
}

int string_to_uuid(const std::string &str, uuid_t uuid) {
    if (str.size() != 36) {
	mysql_error_service_emit_printf(mysql_service_mysql_runtime_error,
                ER_UDF_ERROR, 0, "uuid_v7_to_timestamp", "Invalid UUID string length");
	return 1;
    }
    if (str[14] != '7') {
	mysql_error_service_emit_printf(mysql_service_mysql_runtime_error,
                ER_UDF_ERROR, 0, "uuid_v7_to_timestamp", "This is not a UUID v7");
	return 1;
    }

    int idx = 0;
    for (int i = 0; i < 16; ++i) {
        if (str[idx] == '-') {
            ++idx;
        }
        if (!is_hex_char(str[idx]) || !is_hex_char(str[idx + 1])) {
	    mysql_error_service_emit_printf(mysql_service_mysql_runtime_error,
                ER_UDF_ERROR, 0, "uuid_v7_to_timestamp", "Invalid hex character in UUID string");
	    return 1;
        }
        uuid[i] = (hex_to_byte(str[idx]) << 4) | hex_to_byte(str[idx + 1]);
        idx += 2;
    }
    return 0;
}

uint64_t uuid_to_unixts(uuid_t uuid) {
        uint64_t unix_ts = 0;

        for (int i = 0; i < UNIX_TS_LENGTH; i++) {
           unix_ts |= ((uint64_t)uuid[UNIX_TS_LENGTH - 1 - i]) << (8 * i);
        }

        return unix_ts;
}

std::string uuid_to_ts(uuid_t uuid) {
        std::string out;
        uint64_t unix_ts = 0;

        for (int i = 0; i < UNIX_TS_LENGTH; i++) {
           unix_ts |= ((uint64_t)uuid[UNIX_TS_LENGTH - 1 - i]) << (8 * i);
        }
        out = get_timestamp(unix_ts);

        return out;
}

std::string uuid_to_ts_long(uuid_t uuid) {
        std::string out;
        uint64_t unix_ts = 0;

        for (int i = 0; i < UNIX_TS_LENGTH; i++) {
           unix_ts |= ((uint64_t)uuid[UNIX_TS_LENGTH - 1 - i]) << (8 * i);
        }
        out = get_timestamp_long(unix_ts);

        return out;
}


extern REQUIRES_SERVICE_PLACEHOLDER(log_builtins);
extern REQUIRES_SERVICE_PLACEHOLDER(log_builtins_string);
extern REQUIRES_SERVICE_PLACEHOLDER(udf_registration);
extern REQUIRES_SERVICE_PLACEHOLDER(mysql_udf_metadata);
extern REQUIRES_SERVICE_PLACEHOLDER(mysql_runtime_error);

extern SERVICE_TYPE(log_builtins) * log_bi;
extern SERVICE_TYPE(log_builtins_string) * log_bs;
