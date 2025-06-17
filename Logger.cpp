#include "Logger.h"
#include <iostream>


Logger::Logger(const std::string& inputFilename) {
    // Constructor: Initializes the logger with output files based on the input filename
    // Get the base name for output files
    std::string base_name = Logger::getOutputFilename(inputFilename);
    this->file_name = base_name;

    // Prepare output file paths
    std::string regular_path = "output_" + base_name;
    std::string detailed_path = "detailed_output_" + base_name;
    
    // Open regular log file
    regular_out.open(regular_path);
    if (!regular_out.is_open()) {
        std::cerr << "Logger: Failed to open " << regular_path << "\n";
    }

    // Open detailed log file
    detailed_out.open(detailed_path);
    if (!detailed_out.is_open()) {
        std::cerr << "Logger: Failed to open " << detailed_path << "\n";
    }
}


Logger::~Logger() {
    // Destructor: Closes the output files if open
    if (regular_out.is_open()) {
        regular_out.close();
    }
    if (detailed_out.is_open()) {
        detailed_out.close();
    }
}


void Logger::logStep(int step, const std::string& message) {
    // Log a message for a specific step in both regular and detailed logs
    if (regular_out.is_open()) {
        regular_out << "Step " << step << ": " << message << "\n";
    }
    if (detailed_out.is_open()) {
        detailed_out << "Step " << step << ": " << message << "\n";
    }
}


void Logger::logFinal(const std::string& message, bool write_to_reg) {
    // Log the final result message in detailed log
    if(regular_out.is_open() && (write_to_reg)) {
        regular_out << message << "\n";
    }
    if (detailed_out.is_open()) {
        detailed_out << "== Final Result ==\n" << message << "\n";
    }
}


void Logger::logLineDetailed(const std::string& message) {
    // Log a single line message in the detailed log
    if (detailed_out.is_open()) {
        detailed_out << message << std::endl;
    }
}


void Logger::logLine(const std::string& message, bool add_newline) {
    // Log a single line message in regular
    if (regular_out.is_open()) {
        regular_out << message;
        if (add_newline) regular_out << std::endl;
    }
}

void Logger::logActionSummary(const std::string& action, bool ignored, bool killed, bool last) {
    // Log a summary of an action in the regular log
    if (!regular_out.is_open()) return;

    regular_out << action;
    if (ignored) regular_out << " (ignored)";
    if (killed) regular_out << " (killed)";
    if (!last) regular_out << ", " ; 
}

void Logger::logActionDetailed(int step, const std::string& message, const std::string& reason) {
    // Log detailed information about an action in the detailed log
    if (!detailed_out.is_open()) return;
    detailed_out << "Step " << step << ": " << message;
    if (!reason.empty()) detailed_out << " (ignored - " << reason << ")";
    detailed_out << "\n";
}


std::string Logger::getOutputFilename(const std::string& inputFile) {
    // Helper function: Extracts the file's name from a path for output file naming
    size_t pos = inputFile.find_last_of("/\\");
    return (pos == std::string::npos) ? inputFile : inputFile.substr(pos + 1);
}
