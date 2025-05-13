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
	Agent agent1(myApi);
	Agent note(myApi);
	// Load configuration into agent1
	if (loadAgentProfile(agent1, "/home/mlamkadm/ai-repos/agents/agent-lib/config/agents/standard-profiles/standard-agent-MK1/standard-agent-MK1.yml"))
	{
		std::cout << "Agent Name after load: " << agent1.getName() << std::endl;
		// std::cout << "listing conf after load "<< std::endl;
		// agent1.listAllAgentInfo();
		// Modify the agent slightly
		// agent1.addEnvVar("SESSION_ID", "xyz789");
		// agent1.addTask("Final review step");
		// test while to chat.
		while (true)
		{
			std::string userInput;
			std::cout << "=======================================\n=> ";
			std::getline(std::cin, userInput);
			if (userInput == "exit")
				break ;                // Exit condition
			agent1.prompt(userInput); // Assuming chat method exists
		}
		// // Save the modified configuration (profile aspects only)
		// if (saveAgentProfile(agent1,
				// "./agents/order_processor_modified.yaml")) {
		//     std::cout << "Modified profile saved." << std::endl;
		// } else {
		//     std::cerr << "Failed to save modified profile." << std::endl;
		// }
	}
	else
	{
		std::cerr << "Failed to load agent profile." << std::endl;
	}
	return (0);
}


// main for note agent only

// void LoadBuiltinsToAgent(Agent &agentToConfigure)
// {
//     // Load built-in tools into the agent
//     // bash tool
//     Tool *bashTool = new Tool("BashExecutor", "Executes bash commands", 
//                                executeBashCommandReal);
//     agentToConfigure.addTool(bashTool);
// }



// void PrintPizzazLine(const std::string& line) {
//     const int width = 40; // Total width, including borders
//     const int inner_width = width - 2; // Space between borders
//
//     // Top border: ╔═══════════════════════════════════════╗
//     std::cout << "╔" << std::string(inner_width, '═') << "╗" << std::endl;
//
//     // Middle line: center the string between ║ characters
//     int padding = (inner_width - line.length()) / 2;
//     if (padding < 0) padding = 0; // No negative padding if string is too long
//     std::string left_padding(padding, ' ');
//     std::string right_padding(inner_width - line.length() - padding, ' ');
//     std::cout << "║" << left_padding << line << right_padding << "║" << std::endl;
//
//     // Bottom border: ╚═══════════════════════════════════════╝
//     std::cout << "╚" << std::string(inner_width, '═') << "╝" << std::endl;
// }
//
// int main() {
//     PrintPizzazLine("Yo, this is MAX PIZZAZ!");
//     return 0;
// }
//
//
//
// int	main(void)
// {
//     MiniGemini	myApi;
//
// 	// Assume myApi is an initialized MiniGemini instance
// 	Agent agent1(myApi);
// 	// Load configuration into agent1
// 	if (loadAgentProfile(agent1, "./config/agents/standard-profiles/standard-note-agent-MK1/note-agent.yml"))
// 	{
// 		std::cout << "Agent Name after load: " << agent1.getName() << std::endl;
//         // LoadBuiltinsToAgent(agent1);
// 		// agent1.listAllAgentInfo();
// 		// Modify the agent slightly
// 		// agent1.addTask("Final review step");
// 		while (true)
// 		{
// 			std::string userInput;
// 			std::cout << "=======================================\n=> ";
// 			std::getline(std::cin, userInput);
// 			if (userInput == "exit")
// 				break ;                // Exit condition
// 			agent1.prompt(userInput); // Assuming chat method exists
// 		}
// 		// // Save the modified configuration (profile aspects only)
// 		// if (saveAgentProfile(agent1,
// 				// "./agents/order_processor_modified.yaml")) {
// 		//     std::cout << "Modified profile saved." << std::endl;
// 		// } else {
// 		//     std::cerr << "Failed to save modified profile." << std::endl;
// 		// }
// 	}
// 	else
// 	{
// 		std::cerr << "Failed to load agent profile." << std::endl;
// 	}
// 	return (0);
// }
