#ifndef _PROTOCOL_DYNAMIC_LLOADER_
#define _PROTOCOL_DYNAMIC_LLOADER_

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

struct ProtocolPrivate
{
    dl_handle handle;

    protocol_strerror strerror;
    init_protocol init;
    release_protocol release;

    get_support_types getSupportTypes;
    set_options setOptions;
    get_options getOptions;
    get_options_help getOptionsHelp;

    add_session addSession;
    delete_session delSession;
    is_session_exist hasSession;
    get_session_bitmap getSesBitmap;
    get_session_options getSesOptions;

    do_perform perform;
};

#endif
