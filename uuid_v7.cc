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

#define LOG_COMPONENT_TAG  "uuid_v7"
#define NO_SIGNATURE_CHANGE 0
#define SIGNATURE_CHANGE 1

#include <components/uuid_v7/uuid_v7.h>

REQUIRES_SERVICE_PLACEHOLDER(log_builtins);
REQUIRES_SERVICE_PLACEHOLDER(log_builtins_string);
REQUIRES_SERVICE_PLACEHOLDER(udf_registration);
REQUIRES_SERVICE_PLACEHOLDER(mysql_udf_metadata);
REQUIRES_SERVICE_PLACEHOLDER(mysql_runtime_error);

SERVICE_TYPE(log_builtins) * log_bi;
SERVICE_TYPE(log_builtins_string) * log_bs;

class udf_list {
  typedef std::list<std::string> udf_list_t;

 public:
  ~udf_list() { unregister(); }
  bool add_scalar(const char *func_name, enum Item_result return_type,
                  Udf_func_any func, Udf_func_init init_func = NULL,
                  Udf_func_deinit deinit_func = NULL) {
    if (!mysql_service_udf_registration->udf_register(
            func_name, return_type, func, init_func, deinit_func)) {
      set.push_back(func_name);
      return false;
    }
    return true;
  }

  bool unregister() {
    udf_list_t delete_set;
    /* try to unregister all of the udfs */
    for (auto udf : set) {
      int was_present = 0;
      if (!mysql_service_udf_registration->udf_unregister(udf.c_str(),
                                                          &was_present) ||
          !was_present)
        delete_set.push_back(udf);
    }

    /* remove the unregistered ones from the list */
    for (auto udf : delete_set) set.remove(udf);

    /* success: empty set */
    if (set.empty()) return false;

    /* failure: entries still in the set */
    return true;
  }

 private:
  udf_list_t set;
} * list;

namespace udf_impl {
const char *udf_init = "udf_init", *my_udf = "my_udf",
           *my_udf_clear = "my_clear", *my_udf_add = "my_udf_add";

// Get the timestamp from an uuid_v7

static bool uuid_v7_to_timestamp_udf_init(UDF_INIT *initid, UDF_ARGS *, char *) {
  const char* name = "utf8mb4";
  char *value = const_cast<char*>(name);
  initid->ptr = const_cast<char *>(udf_init);
  if (mysql_service_mysql_udf_metadata->result_set(
          initid, "charset",
          const_cast<char *>(value))) {
    LogComponentErr(ERROR_LEVEL, ER_LOG_PRINTF_MSG, "failed to set result charset");
    return false;
  }
  return 0;
}

static void uuid_v7_to_timestamp_udf_deinit(__attribute__((unused)) UDF_INIT *initid) {
  assert(initid->ptr == udf_init || initid->ptr == my_udf);
}

const char *uuid_v7_to_timestamp_udf(UDF_INIT *, UDF_ARGS *args, char *outp,
                          unsigned long *length, char *is_null, char *error) {

    uuid_t uuidv7;
    std::string out;
    if (args->arg_count < 1) {
      mysql_error_service_emit_printf(mysql_service_mysql_runtime_error,
                ER_UDF_ERROR, 0, "uuid_v7_to_timestamp", "this function requires 1 parameteter!"); 
    } else if (args->arg_count > 1) {      
      mysql_error_service_emit_printf(mysql_service_mysql_runtime_error,
                ER_UDF_ERROR, 0, "uuid_v7_to_timestamp", "this function requires only 1 parameteter!"); 
    } else {
      if (!string_to_uuid(args->args[0], uuidv7)) {
         out = uuid_to_ts(uuidv7);
         strcpy(outp, out.c_str());
      }
    }
    *error = 0;
    *is_null = 0;
    *length = strlen(outp);
    return const_cast<char *>(outp);
}

// Get the detailed timestamp from an uuid_v7
static bool uuid_v7_to_timestamp_long_udf_init(UDF_INIT *initid, UDF_ARGS *, char *) {
  const char* name = "utf8mb4";
  char *value = const_cast<char*>(name);
  initid->ptr = const_cast<char *>(udf_init);
  if (mysql_service_mysql_udf_metadata->result_set(
          initid, "charset",
          const_cast<char *>(value))) {
    LogComponentErr(ERROR_LEVEL, ER_LOG_PRINTF_MSG, "failed to set result charset");
    return false;
  }
  return 0;
}

static void uuid_v7_to_timestamp_long_udf_deinit(__attribute__((unused)) UDF_INIT *initid) {
  assert(initid->ptr == udf_init || initid->ptr == my_udf);
}

const char *uuid_v7_to_timestamp_long_udf(UDF_INIT *, UDF_ARGS *args, char *outp,
                          unsigned long *length, char *is_null, char *error) {

    uuid_t uuidv7;
    std::string out;
    if (args->arg_count < 1) {
      mysql_error_service_emit_printf(mysql_service_mysql_runtime_error,
                ER_UDF_ERROR, 0, "uuid_v7_to_timestamp", "this function requires 1 parameteter!"); 
    } else if (args->arg_count > 1) {      
      mysql_error_service_emit_printf(mysql_service_mysql_runtime_error,
                ER_UDF_ERROR, 0, "uuid_v7_to_timestamp", "this function requires only 1 parameteter!"); 
    } else {
      if (!string_to_uuid(args->args[0], uuidv7)) {
         out = uuid_to_ts_long(uuidv7);
         strcpy(outp, out.c_str());
      }
    }
    *error = 0;
    *is_null = 0;
    *length = strlen(outp);
    return const_cast<char *>(outp);
}

// Get the unixtime from an uuid_v7

static bool uuid_v7_to_unixtime_udf_init(UDF_INIT *initid, UDF_ARGS *, char *) {
  initid->ptr = const_cast<char *>(udf_init);
  return false;
}

static void uuid_v7_to_unixtime_udf_deinit(__attribute__((unused)) UDF_INIT *initid) {
  assert(initid->ptr == udf_init || initid->ptr == my_udf);
}

long long uuid_v7_to_unixtime_udf(UDF_INIT *, UDF_ARGS *args, char *,
                          unsigned long *) {

    uuid_t uuidv7;
    uint64_t out=0;
    if (args->arg_count < 1) {
      mysql_error_service_emit_printf(mysql_service_mysql_runtime_error,
                ER_UDF_ERROR, 0, "uuid_v7_to_timestamp", 
                "this function requires 1 parameteter"); 
    } else if (args->arg_count > 1) {      
      mysql_error_service_emit_printf(mysql_service_mysql_runtime_error,
                ER_UDF_ERROR, 0, "uuid_v7_to_timestamp", 
                "this function requires only 1 parameteter"); 
    } else {
      if (!string_to_uuid(args->args[0], uuidv7)) {
         out = uuid_to_unixts(uuidv7) / 1000;
      }
    }
    return (long long)out;
}

// Create a UUID v7
static bool uuid_v7_udf_init(UDF_INIT *initid, UDF_ARGS *, char *) {
  const char* name = "utf8mb4";
  char *value = const_cast<char*>(name);
  initid->ptr = const_cast<char *>(udf_init);
  if (mysql_service_mysql_udf_metadata->result_set(
          initid, "charset",
          const_cast<char *>(value))) {
    LogComponentErr(ERROR_LEVEL, ER_LOG_PRINTF_MSG, "failed to set result charset");
    return false;
  }
  return 0;
}

static void uuid_v7_udf_deinit(__attribute__((unused)) UDF_INIT *initid) {
  assert(initid->ptr == udf_init || initid->ptr == my_udf);
}

const char *uuid_v7_udf(UDF_INIT *, UDF_ARGS *, char *outp,
                          unsigned long *length, char *is_null, char *error) {

    uuid_t uuidv7;
    std::string out;
    create_uuid7(uuidv7);
    out = uuid_to_string(uuidv7);
    strcpy(outp, out.c_str());
    *error = 0;
    *is_null = 0;

    *length = strlen(outp);
    return const_cast<char *>(outp);
}


} /* namespace udf_impl */


static mysql_service_status_t uuid_v7_service_init() {
  mysql_service_status_t result = 0;

  log_bi = mysql_service_log_builtins;
  log_bs = mysql_service_log_builtins_string;

  LogComponentErr(INFORMATION_LEVEL, ER_LOG_PRINTF_MSG, "initializing…");

  list = new udf_list();

  if (list->add_scalar("uuid_v7", Item_result::STRING_RESULT,
                       (Udf_func_any)udf_impl::uuid_v7_udf,
                       udf_impl::uuid_v7_udf_init,
                       udf_impl::uuid_v7_udf_deinit)) {
    delete list;
    return 1; /* failure: one of the UDF registrations failed */
  }

  if (list->add_scalar("uuid_v7_to_timestamp", Item_result::STRING_RESULT,
                       (Udf_func_any)udf_impl::uuid_v7_to_timestamp_udf,
                       udf_impl::uuid_v7_to_timestamp_udf_init,
                       udf_impl::uuid_v7_to_timestamp_udf_deinit)) {
    delete list;
    return 1; /* failure: one of the UDF registrations failed */
  }

  if (list->add_scalar("uuid_v7_to_timestamp_long", Item_result::STRING_RESULT,
                       (Udf_func_any)udf_impl::uuid_v7_to_timestamp_long_udf,
                       udf_impl::uuid_v7_to_timestamp_long_udf_init,
                       udf_impl::uuid_v7_to_timestamp_long_udf_deinit)) {
    delete list;
    return 1; /* failure: one of the UDF registrations failed */
  }

  if (list->add_scalar("uuid_v7_to_unixtime", Item_result::INT_RESULT,
                       (Udf_func_any)udf_impl::uuid_v7_to_unixtime_udf,
                       udf_impl::uuid_v7_to_unixtime_udf_init,
                       udf_impl::uuid_v7_to_unixtime_udf_deinit)) {
    delete list;
    return 1; /* failure: one of the UDF registrations failed */
  }

  return result;
}

static mysql_service_status_t uuid_v7_service_deinit() {
  mysql_service_status_t result = 0;

  if (list->unregister()) return 1; /* failure: some UDFs still in use */

  delete list;

  LogComponentErr(INFORMATION_LEVEL, ER_LOG_PRINTF_MSG, "uninstalled.");

  return result;
}

BEGIN_COMPONENT_PROVIDES(uuid_v7_service)
END_COMPONENT_PROVIDES();

BEGIN_COMPONENT_REQUIRES(uuid_v7_service)
    REQUIRES_SERVICE(log_builtins),
    REQUIRES_SERVICE(log_builtins_string),
    REQUIRES_SERVICE(mysql_udf_metadata),
    REQUIRES_SERVICE(udf_registration),
    REQUIRES_SERVICE(mysql_runtime_error),
END_COMPONENT_REQUIRES();

/* A list of metadata to describe the Component. */
BEGIN_COMPONENT_METADATA(uuid_v7_service)
METADATA("mysql.author", "Oracle Corporation"),
    METADATA("mysql.license", "GPL"), METADATA("mysql.dev", "lefred"),
END_COMPONENT_METADATA();

/* Declaration of the Component. */
DECLARE_COMPONENT(uuid_v7_service,
                  "mysql:uuid_v7_service")
uuid_v7_service_init,
    uuid_v7_service_deinit END_DECLARE_COMPONENT();

/* Defines list of Components contained in this library. Note that for now
  we assume that library will have exactly one Component. */
DECLARE_LIBRARY_COMPONENTS &COMPONENT_REF(uuid_v7_service)
    END_DECLARE_LIBRARY_COMPONENTS
