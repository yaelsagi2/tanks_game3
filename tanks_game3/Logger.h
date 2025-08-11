#ifndef LOGGER_H
#define LOGGER_H

#include <fstream>
#include <string>
#include <vector>

/**
 * @class Logger
 * @brief Handles logging of game events, steps, and actions to output files.
 *
 * Provides methods to log regular and detailed messages, steps, and summaries.
 * Supports the Rule of 5 for resource management.
 */
class Logger {
private:
    std::ofstream regular_out;   ///< Output stream for regular log.
    std::ofstream detailed_out;  ///< Output stream for detailed log.
    std::string file_name;       ///< Base file name for logs.

public:
    /**
     * @brief Constructs a Logger with the given base filename.
     * @param filename The base filename for log files.
     */
    explicit Logger(const std::string& filename);

    // Rule of 5:
    /**
     * @brief Destructor. Closes log files.
     */
    ~Logger();

    /**
     * @brief Deleted copy constructor.
     */
    Logger(const Logger&) = delete;

    /**
     * @brief Deleted copy assignment operator.
     */
    Logger& operator=(const Logger&) = delete;

    /**
     * @brief Defaulted move constructor.
     */
    Logger(Logger&& other) noexcept = default;

    /**
     * @brief Defaulted move assignment operator.
     */
    Logger& operator=(Logger&& other) noexcept = default;
    
    /**
     * @brief Logs a message for a specific step in the regular log.
     * @param step The step number.
     * @param message The message to log.
     */
    void logStep(int step, const std::string& message);

    /**
     * @brief Logs a final message in the regular log.
     * @param message The message to log.
     */
    void logFinal(const std::string& message, bool write_to_reg = true);

    /**
     * @brief Logs a single line message in the regular log.
     * @param message The message to log.
     */
    void logLine(const std::string& message, bool add_newline = true);

    /**
     * @brief Logs a message for a specific step in the detailed log.
     * @param step The step number.
     * @param message The message to log.
     */
    void logStepDetailed(int step, const std::string& message);

    /**
     * @brief Logs a single line message in the detailed log.
     * @param message The message to log.
     */
    void logLineDetailed(const std::string& message);

    /**
     * @brief Logs a summary of an action.
     * @param action The action name.
     * @param ignored Whether the action was ignored.
     * @param killed Whether the action resulted in a kill.
     * @param last Whether this is the last action in the summary.
     */
    void logActionSummary(const std::string& action, bool ignored = false, bool killed = false, bool last = false);

    /**
     * @brief Logs detailed information about an action.
     * @param step The step number.
     * @param message The message to log.
     * @param reason Optional reason for the action.
     */
    void logActionDetailed(int step, const std::string& message, const std::string& reason = "");

private:
    /**
     * @brief Generates the output filename based on the input file.
     * @param inputFile The input file name.
     * @return The generated output filename.
     */
    static std::string getOutputFilename(const std::string& inputFile); 
};

#endif // LOGGER_H