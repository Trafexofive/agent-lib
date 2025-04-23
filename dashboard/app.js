// dashboard/app.js

document.addEventListener('DOMContentLoaded', () => {
    const chatbox = document.getElementById('chatbox');
    const userInput = document.getElementById('userInput');
    const sendButton = document.getElementById('sendButton');

    // --- Configuration ---
    // IMPORTANT: Replace with your actual server endpoint and expected JSON keys
    // If frontend and backend are served from different ports (e.g., 8000 and 8080),
    // use the full backend URL like 'http://localhost:8080/api/prompt'
    // and ensure your C++ server has CORS headers enabled.
    const API_ENDPOINT = 'http://localhost:7777/prompt'; // Example: '/api/prompt' or 'http://localhost:8080/api/prompt'
    const REQUEST_KEY = 'prompt';       // Key for user message in request JSON (e.g., "prompt")
    const RESPONSE_KEY = 'response';    // Key for agent message in response JSON (e.g., "response")
    // ---------------------

    /**
     * Adds a message bubble to the chatbox.
     * @param {string} text - The message content.
     * @param {'user' | 'agent' | 'error'} sender - The sender type ('user', 'agent', or 'error').
     */
    function addMessage(text, sender) {
        if (!chatbox) {
            console.error("Chatbox element not found!");
            return;
        }
        const messageDiv = document.createElement('div');
        messageDiv.classList.add('message', sender);
        messageDiv.textContent = text; // Using textContent for safety against XSS
        chatbox.appendChild(messageDiv);
        // Scroll to the bottom to show the latest message
        chatbox.scrollTop = chatbox.scrollHeight;
    }

    /**
     * Sends the user's message to the backend API and displays the response.
     */
    async function sendMessage() {
        if (!userInput || !sendButton) {
            console.error("Input or button element not found!");
            return;
        }
        const userText = userInput.value.trim();
        if (!userText) {
            return; // Don't send empty messages
        }

        // Display user message immediately
        addMessage(userText, 'user');
        const currentInputText = userText; // Store before clearing
        userInput.value = ''; // Clear input field
        sendButton.disabled = true; // Prevent multiple clicks while waiting
        userInput.disabled = true;

        try {
            // Prepare the request payload
            const requestBody = {};
            requestBody[REQUEST_KEY] = currentInputText;

            const response = await fetch(API_ENDPOINT, {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json',
                    // Add any other headers your server might require
                },
                body: JSON.stringify(requestBody)
            });

            if (!response.ok) {
                // Attempt to get error details from the response body
                let errorText = `HTTP error! Status: ${response.status}`;
                try {
                    const errorData = await response.json();
                    errorText += `, Message: ${JSON.stringify(errorData)}`;
                } catch (e) {
                    // If response is not JSON, use text
                    const textError = await response.text();
                    errorText += `, Body: ${textError}`;
                }
                throw new Error(errorText);
            }

            // Parse the JSON response from the server
            const data = await response.json();

            // Extract the agent's reply using the configured key
            const agentResponse = data[RESPONSE_KEY];

            if (agentResponse !== undefined && agentResponse !== null) {
                addMessage(agentResponse, 'agent');
            } else {
                 console.warn(`Response received, but key "${RESPONSE_KEY}" was missing or null.`, data);
                 addMessage(`Received response, but couldn't find the expected content (key: "${RESPONSE_KEY}").`, 'error');
            }

        } catch (error) {
            console.error('Error sending message or processing response:', error);
            addMessage(`Error: ${error.message}`, 'error');
        } finally {
            // Re-enable input fields regardless of success or failure
            sendButton.disabled = false;
            userInput.disabled = false;
            userInput.focus(); // Set focus back to the input field
        }
    }

    // --- Event Listeners ---
    if (sendButton) {
        sendButton.addEventListener('click', sendMessage);
    } else {
        console.error("Send button not found!");
    }

    if (userInput) {
        userInput.addEventListener('keypress', (event) => {
            // Send message if Enter key is pressed (and Shift key is not held)
            if (event.key === 'Enter' && !event.shiftKey) {
                event.preventDefault(); // Prevent default Enter behavior (like adding a newline)
                sendMessage();
            }
        });
    } else {
        console.error("User input element not found!");
    }

    // Optional: Add a welcome message on load
    addMessage("Agent connected. How can I help you today?", 'agent');
});
