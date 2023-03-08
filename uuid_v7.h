/* Copyright (c) 2017, 2022, Oracle and/or its affiliates. All rights reserved.
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


#include <time.h>
#include <sys/random.h>

#include <list>
#include <string>

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

void get_random_bytes(uint8_t buffer[], size_t len)
{
    int ignored __attribute__((unused));
    ignored = getentropy(buffer, len);
    if (errno != EXIT_SUCCESS)
    {
        exit(EXIT_FAILURE);
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

extern REQUIRES_SERVICE_PLACEHOLDER(log_builtins);
extern REQUIRES_SERVICE_PLACEHOLDER(log_builtins_string);
extern REQUIRES_SERVICE_PLACEHOLDER(udf_registration);
extern REQUIRES_SERVICE_PLACEHOLDER(mysql_udf_metadata);

extern SERVICE_TYPE(log_builtins) * log_bi;
extern SERVICE_TYPE(log_builtins_string) * log_bs;