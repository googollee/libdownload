#ifndef _PROTOCOL_PLUGINS_DEFINE_
#define _PROTOCOL_PLUGINS_DEFINE_

#include "../Global.h"

typedef const inner_raw_string (*protocol_strerror)(protocol_retcode);
typedef protocol_retcode (*init_protocol)(Protocol::callback_finish_function func);
typedef protocol_retcode (*release_protocol)();

typedef const inner_raw_string (*get_support_types)();
typedef protocol_retcode (*set_options)(const raw_string options);
// Will alloc memory for return string in below function.
typedef protocol_retcode (*get_options)(raw_string options);
typedef protocol_retcode (*get_options_help)(raw_string help_string);

typedef protocol_retcode (*add_session)(session_id id, session_info *info);
typedef protocol_retcode (*delete_session)(session_id id);
typedef protocol_retcode (*is_session_exist)(session_id id);
typedef protocol_retcode (*get_session_bitmap)(session_id id);
typedef protocol_retcode (*get_session_options)(session_id id);

typedef protocol_retcode (*do_perform)(size_t *downloaded, size_t *uploaded);

#endif

