#ifndef PROTOCOL_BASE_HEADER
#define PROTOCOL_BASE_HEADER

#include "includes.h"
#include "TaskBase.h"

class ProtocolBase : private Noncopiable
{
public:
    /**
     * \brief Callback when protocol need log something.
     *
     * Log informations for protocol plugin.
     * For example: init, read configure, or fail information. No need sufix with '\n'.
     * \param p   The pointer to protocol instance
     * \param log Log text.
     */
    boost::signal<void (ProtocolBase *p, const char *log)> protocolLog;

    ProtocolBase() {}
    virtual ~ProtocolBase() {}

    /**
     * \brief The name of protocol.
     *
     * Return protocol name for display, in utf8 codec.
     * It's better to return a statice string, which won't change in protocol instance life circle.
     *
     * \return The name text in utf8.
     */
    virtual const char* name() = 0;

    /**
     * \brief Get the options detail description.
     *
     * Get the options detail description in XML. The XML format is like:
     * \code
     * <OptionName>
     *   <title>OptionTitleForShort</title>
     *   <desc>Detail description of this option</desc>
     *   <type>value format of this option</type>
     *   <pattern>value pattern of this option, in regex</pattern>
     * </OptionName>
     * \endcode
     * \notice Values of type and pattern follow XML Schema special.
     *
     * \return The XML text in utf8.
     * \see getTaskOptions()
     * \see getAllOptions()
     */
    virtual const char* getOptionsDetail() = 0;

    /**
     * \brief Get protocol's option.
     *
     * Get protocols options. It used to resume the protocol status when relaunch.
     * It's different like the task options which can be different in different task. It's the global options of protocol or default value of task options, shared with all tasks.
     *
     * \return The options.
     * \see saveOptions()
     */
    virtual const char* getOptions() = 0;

    /**
     * \brief Set protocol's option.
     *
     * Set protocols option. It used to resume the protocol status when relaunch.
     * It's different like the task options which can be different in different task. It's the global options of protocol or default value of task options, shared with all tasks.
     *
     * \param options The options.
     * \see loadOptions()
     */
    virtual void setOptions(const char *options) = 0;

    /**
     * \brief Check whether uri can be handled with this protocol.
     *
     * Check whether uri can be handled by this protocol.
     *
     * \param uri The uri need check. It should be in utf8 codec
     * \return The check result.
     */
    virtual bool canProcess(const char *uri) = 0;

    /**
     * \brief Get the options when adding uri as task.
     *
     * Some time user want control the download task, and need use this API get the options of this protocol task.
     * The options is a XML text. The detail of option tag can be found with getOptionsDetail().
     * For example, when download a bt feed, maybe only download one file in feed but not all, then caller can get the file list from getTaskOptions() and set the download file in options when call addTask().
     *
     * Return text like below:
     * \code
     * <OptionName1>value1</OptionName1>
     * <OptionName2>value2</OptionName2>
     * \endcode
     *
     * \param uri task uri in utf8 codec.
     * \return options text in XML.
     * \see getOptionsDetail()
     * \see addTask()
     */
    virtual const char* getTaskOptions(const char *uri) = 0;

    /**
     * \brief Add a task.
     *
     * TaskInfo is controled by manager, can modify in protocol
     * If info->processData doesn't empty, need resume task from processData.
     *
     * \param info The task info.
     */
    virtual std::auto_ptr<TaskBase> getTask(const char *uri,
                                            const char *outputDir,
                                            const char *outputName,
                                            const char *options,
                                            const char *comment) = 0;
};

#endif
