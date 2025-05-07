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

// --- Example Usage ---
int	main(void)
{
    MiniGemini	myApi;

	// Assume myApi is an initialized MiniGemini instance
	Agent agent1(myApi);
	// Load configuration into agent1
	if (loadAgentProfile(agent1, "./config/agents/standard.yaml"))
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
			std::cout << "User: ";
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
