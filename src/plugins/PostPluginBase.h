#ifndef POST_PLUGIN_BASE_HEAD
#define POST_PLUGIN_BASE_HEAD

#include <utility/Utility.h>
#include <DownloadManager.h>

class PostPluginBase : public Noncopiable
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
    boost::signal<void (PostPluginBase *p, const char *log)> log;

    PostPluginBase(DownloadManager &manager);
    virtual ~PostPluginBase();

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
     * \brief Check if task can be processed.
     *
     * Check if task can be processed.
     *
     * \param info Check the task info need be processed.
     * \return Check result.
     */
    virtual bool canProcess(TaskInfo *info) = 0;

    /**
     * \brief Poress task.
     *
     * Poress task with this post plugin.
     *
     * \param info The task info need be processed.
     */
    virtual void process(TaskInfo *info) = 0;

    /**
     * \brief Return the string of error.
     *
     * \param error Error number.
     * \return The text of error.
     */
    virtual const char *strerror(int error) = 0;
};

inline PostPluginBase::PostPluginBase()
{}

inline PostPluginBase::~PostPluginBase()
{}

#endif
