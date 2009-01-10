#include "../Global.h"
#include "Protocol.h"

#include "../utility/dl_wrapper.h"

// below type define need move to plugin.h
typedef struct
{
    PROTOCOL_RET_OK = 0;
} protocol_retcode;

typedef enum
{
    BYTES,
    KILOBYTES,
    MEGABYTES,
    GIGABYTES,
 } speed_unit;


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
    ProtocolPrivate()
        {
            memset(this, 0, sizeof(this));
        }

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

Protocol::Protocol(const RawString filename, callback_finish_function) : d(new ProtocolPrivate)
{
    d->handle = dl_open(filename);
    if (dl_handle == NULL)
    {
        throw ProtocolException(dl_errno(), dl_strerror(dl_errno()));
    }

#define DL_GETFUNC(target, func_str) \
    target = dl_getfunc(dl_handle, func_str); \
    if (d->strerror == NULL) \
    { \
        throw ProtocolException(dl_errno(), dl_strerror(dl_errno())); \
    }

    DL_GETFUNC(d->strerror, "protocol_strerror");
    DL_GETFUNC(d->init, "init_protocol");
    DL_GETFUNC(d->release, "release_protocol");
    DL_GETFUNC(d->getSupportTypes, "get_support_types");
    DL_GETFUNC(d->setOptions, "set_options");
    DL_GETFUNC(d->getOptions, "get_options");
    DL_GETFUNC(d->getOptionsHelp, "get_options_help");
    DL_GETFUNC(d->addSession, "add_session");
    DL_GETFUNC(d->delSession, "delete_session");
    DL_GETFUNC(d->hasSession, "is_session_exist");
    DL_GETFUNC(d->getSesBitmap, "get_session_bitmap");
    DL_GETFUNC(d->getSesOptions, "get_session_options");

#undefine DL_GETFUNC
}

Protocol::~Protocol()
{
    dl_close(d->handle);
    if (dl_errno() != DL_OK)
    {
        printf("close dynamic library error: %s\n", dl_strerror(dl_errno()));
    }
}
