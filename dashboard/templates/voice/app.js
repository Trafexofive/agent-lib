// dashboard/app.js

document.addEventListener('DOMContentLoaded', () => {
    const chatbox = document.getElementById('chatbox');
    const userInput = document.getElementById('userInput');
    const sendButton = document.getElementById('sendButton');
    const micButton = document.getElementById('micButton'); // Get the mic button
    const statusDiv = document.getElementById('status'); // Get the status div

    // --- Configuration ---
    const API_ENDPOINT = 'https://agent.clevo.ddnsgeek.com/prompt';
    const REQUEST_KEY = 'prompt';
    const RESPONSE_KEY = 'response';
    // ---------------------

    /**
     * Adds a message bubble to the chatbox.
     * @param {string} text - The message content.
     * @param {'user' | 'agent' | 'error' | 'system'} sender - The sender type.
     */
    function addMessage(text, sender) {
        if (!chatbox) {
            console.error("Chatbox element not found!");
            return;
        }
        const messageDiv = document.createElement('div');
        messageDiv.classList.add('message', sender);
        // Basic sanitization - prevent rendering HTML tags
        messageDiv.textContent = text;
        chatbox.appendChild(messageDiv);
        chatbox.scrollTop = chatbox.scrollHeight; // Scroll down
    }

    /**
     * Sets the status message and disables/enables inputs.
     * @param {string} text - The status text to display.
     * @param {boolean} isBusy - Whether the agent/system is busy (disables inputs).
     */
    function setStatus(text, isBusy) {
        if (statusDiv) statusDiv.textContent = text;
        if (userInput) userInput.disabled = isBusy;
        if (sendButton) sendButton.disabled = isBusy;
        // Also disable mic button when the system is busy sending/receiving messages
        if (micButton) micButton.disabled = isBusy;
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

        // Display user message immediately only if it's not already the last message
        // (prevents duplication if sent via Enter right after voice input finishes)
        const lastMessage = chatbox.lastElementChild;
        if (!lastMessage || !lastMessage.classList.contains('user') || lastMessage.textContent !== userText) {
            addMessage(userText, 'user');
        }

        const currentInputText = userText; // Store before clearing
        userInput.value = ''; // Clear input field
        setStatus('Agent thinking...', true); // Set status and disable inputs

        try {
            const requestBody = {};
            requestBody[REQUEST_KEY] = currentInputText;

            const response = await fetch(API_ENDPOINT, {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json',
                },
                body: JSON.stringify(requestBody)
            });

            if (!response.ok) {
                let errorText = `HTTP error! Status: ${response.status}`;
                try {
                    const errorData = await response.json();
                    errorText += `, Message: ${JSON.stringify(errorData)}`;
                } catch (e) {
                    const textError = await response.text();
                    errorText += `, Body: ${textError}`;
                }
                throw new Error(errorText);
            }

            const data = await response.json();
            const agentResponse = data[RESPONSE_KEY];

            if (agentResponse !== undefined && agentResponse !== null) {
                addMessage(agentResponse, 'agent');
            } else {
                 console.warn(`Response key "${RESPONSE_KEY}" missing or null.`, data);
                 addMessage(`Received response, but couldn't find content (key: "${RESPONSE_KEY}").`, 'error');
            }

        } catch (error) {
            console.error('Error sending/receiving message:', error);
            addMessage(`Error: ${error.message}`, 'error');
        } finally {
            setStatus('', false); // Clear status, re-enable inputs
            userInput.focus();
        }
    }

    // --- Speech Recognition Setup ---
    const SpeechRecognition = window.SpeechRecognition || window.webkitSpeechRecognition;
    let recognition;
    let isRecording = false;
    let finalTranscript = ''; // Store final transcript from speech events

    if (SpeechRecognition && micButton) {
        micButton.classList.remove('hidden'); // Show the button if API is supported
        recognition = new SpeechRecognition();
        recognition.continuous = false; // Process speech after user stops talking
        recognition.interimResults = true; // Show results as they come in
        recognition.lang = 'en-US'; // Adjust language if needed

        recognition.onstart = () => {
            console.log("Speech recognition started");
            isRecording = true;
            finalTranscript = ''; // Clear previous final transcript
            micButton.classList.add('recording');
            micButton.title = "Stop Recording";
            setStatus('Listening...', false); // Indicate listening, keep inputs enabled for now
            sendButton.disabled = true; // Disable send while listening
        };

        recognition.onresult = (event) => {
            let interimTranscript = '';
            finalTranscript = ''; // Recalculate final transcript each time

            for (let i = event.resultIndex; i < event.results.length; ++i) {
                const transcriptPart = event.results[i][0].transcript;
                if (event.results[i].isFinal) {
                    finalTranscript += transcriptPart;
                } else {
                    interimTranscript += transcriptPart;
                }
            }
            // Update the input field with the latest transcript (final takes precedence)
            userInput.value = finalTranscript || interimTranscript;
        };

        recognition.onerror = (event) => {
            console.error("Speech recognition error:", event.error, event.message);
            let errorMsg = `Speech Error: ${event.error}`;
            if (event.error === 'no-speech') {
                errorMsg = "No speech detected. Please try again.";
            } else if (event.error === 'audio-capture') {
                errorMsg = "Microphone error. Ensure it's connected and enabled.";
            } else if (event.error === 'not-allowed') {
                errorMsg = "Microphone access denied. Please allow access in browser settings.";
            } else if (event.error === 'network') {
                 errorMsg = "Network error during speech recognition.";
            }
            addMessage(errorMsg, 'system'); // Show error in chat
            setStatus('', false); // Clear status, enable inputs
            isRecording = false;
            micButton.classList.remove('recording');
            micButton.title = "Start Recording";
            sendButton.disabled = false; // Re-enable send on error
        };

        recognition.onend = () => {
            console.log("Speech recognition ended");
            isRecording = false;
            micButton.classList.remove('recording');
            micButton.title = "Start Recording";
            setStatus('', false); // Clear listening status
            sendButton.disabled = false; // Re-enable send button

            // IMPORTANT: Send the message automatically if we got a final transcript
            if (finalTranscript.trim()) {
                console.log("Final transcript received:", finalTranscript);
                userInput.value = finalTranscript.trim(); // Ensure input field has the final text
                sendMessage(); // <<< Automatically send the message
            } else {
                console.log("No final transcript obtained.");
                userInput.focus(); // Focus input if nothing was sent
            }
        };

        // Microphone Button Event Listener
        micButton.addEventListener('click', () => {
            if (!recognition) return; // Safety check

            if (isRecording) {
                recognition.stop();
                // onend will handle UI changes
            } else {
                // Clear previous text before starting new recording? Optional.
                // userInput.value = '';
                finalTranscript = ''; // Reset transcript
                try {
                    recognition.start();
                    // onstart will handle UI changes
                } catch (e) {
                    // Handle potential errors if start() fails immediately (e.g., mic already in use)
                    console.error("Error starting recognition:", e);
                    addMessage(`Could not start microphone: ${e.message}`, 'error');
                    setStatus('', false); // Reset UI
                }
            }
        });

    } else {
        console.warn("Web Speech Recognition API not supported in this browser.");
        if(micButton) micButton.classList.add('hidden'); // Ensure it stays hidden
    }
    // --- End Speech Recognition Setup ---


    // --- Event Listeners (Send Button and Enter Key) ---
    if (sendButton) {
        sendButton.addEventListener('click', sendMessage);
    } else {
        console.error("Send button not found!");
    }

    if (userInput) {
        userInput.addEventListener('keypress', (event) => {
            if (event.key === 'Enter' && !event.shiftKey && !sendButton.disabled) {
                event.preventDefault();
                sendMessage();
            }
        });
    } else {
        console.error("User input element not found!");
    }

    // Optional: Add initial message
    // addMessage("Agent connected. How can I help?", 'agent'); // Moved to HTML
    userInput?.focus(); // Set focus to input on load
});
