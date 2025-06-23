/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   import.main.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mlamkadm <mlamkadm@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/13 16:05:39 by mlamkadm          #+#    #+#             */
/*   Updated: 2025/05/13 16:05:39 by mlamkadm         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "inc/Agent.hpp"
#include "inc/Import.hpp"
#include "inc/MiniGemini.hpp"
#include "inc/Tool.hpp"
#include "inc/modelApi.hpp" // For ApiError
#include "json/json.h"      // For Json::Value used by tools
#include <curl/curl.h>      // For curl_global_init/cleanup
#include <memory>           // For Tool pointers (optional but good practice)
#include <stdexcept>
#include <string>
#include <vector>

// #include "externals/file.cpp" // For file operations
// #include "externals/bash.cpp" // For bash command execution


void commandHandler(const std::string &command) {
    std::vector<std::string> commandArgs;
    std::istringstream iss(command);
    std::string arg;

    while (iss >> arg) {
        commandArgs.push_back(arg);
    }

}



// --- Example Usage ---
int	main(void)
{
    MiniGemini	myApi;

	// Assume myApi is an initialized MiniGemini instance
	Agent DEMURGE(myApi);

    // Load agent yaml profile
    if (loadAgentProfile(DEMURGE, "/home/mlamkadm/ai-repos/agents/agent-lib/config/agents/standard-agent-MK1/DEMURGE.yml")) {
        std::cout << "Agent Name after load: " << DEMURGE.getName() << std::endl;

        // Start command loop
        while (true) {
            std::string userInput;
            std::cout << "=======================================\n=> ";
            std::getline(std::cin, userInput);
            if (userInput == "exit")
                break; // Exit condition
            DEMURGE.prompt(userInput); // Assuming prompt method exists
        }
    } else {
        std::cerr << "Failed to load agent profile." << std::endl;
    }
}

// draft -h page :
// // -h, --help: Show this help message
// // -v, --version: Show version information
// // // -c, --config <file>: Specify a configuration file
// // // -l, --list: List available agents
// // // -a, --agent <name>: Specify an agent to use
// // // -t, --tool <name>: Specify a tool to use
// // // -i, --input <text>: Provide input text for the agent
// // // -o, --output <file>: Specify an output file for the agent's response
// // // --verbose: Enable verbose output
// // // --quiet: Suppress output
// // // --debug: Enable debug mode
// // // --no-color: Disable colored output
// // // --config-dir <dir>: Specify a directory for configuration files
// // // --data-dir <dir>: Specify a directory for data files
// // // --cache-dir <dir>: Specify a directory for cache files
// // // --log-file <file>: Specify a file for logging output
// // // --log-level <level>: Set the logging level (e.g., debug, info, warning, error)
// // // --timeout <seconds>: Set a timeout for operations
// // // --retry <count>: Set the number of retries for failed operations
// // // --no-cache: Disable caching
// // // --no-ssl: Disable SSL verification
// // // --proxy <url>: Specify a proxy server to use
// // // --user-agent <string>: Set a custom user agent string


