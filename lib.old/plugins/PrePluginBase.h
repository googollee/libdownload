#ifndef PRE_PLUGIN_BASE_HEAD
#define PRE_PLUGIN_BASE_HEAD

#include <utility/Utility.h>
#include <DownloadManager.h>

class PrePluginBase : public Noncopiable
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
    boost::signal<void (PrePluginBase *p, const char *log)> log;

    PrePluginBase(DownloadManager &manager);
    virtual ~PrePluginBase();

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
     *   <format>value format of this option</format>
     * </OptionName>
     * \endcode
     *
     * \return The XML text in utf8.
     * \see getTaskOptions()
     * \see getAllOptions()
     * \todo Doesn't sure the format of option format now, should use some thing easy convert to html.
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
    virtual void addTask(const char *uri,
                         const char *outputPath,
                         const char *outputName,
                         const char *options,
                         const char *comment) = 0;

    /**
     * \brief Return the string of error.
     *
     * \param error Error number.
     * \return The text of error.
     */
    virtual const char *strerror(int error) = 0;
};

inline PrePluginBase::PrePluginBase()
{}

inline PrePluginBase::~PrePluginBase()
{}

#endif
