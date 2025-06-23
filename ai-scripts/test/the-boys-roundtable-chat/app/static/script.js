document.addEventListener('DOMContentLoaded', () => {
    const nicknamePromptOverlay = document.getElementById('nickname-prompt-overlay');
    const nicknameInput = document.getElementById('nicknameInput');
    const joinButton = document.getElementById('joinButton');
    const chatArea = document.getElementById('chat-area');
    const messagesDiv = document.getElementById('messages');
    const messageInput = document.getElementById('messageInput');
    const sendButton = document.getElementById('sendButton');
    const connectionStatusDiv = document.getElementById('connectionStatus');
    const chatHeaderTitle = document.getElementById('chatHeaderTitle');

    let ws = null;
    let currentNickname = '';

    fetch('/app-config')
        .then(response => response.json())
        .then(config => {
            document.title = config.chat_title || 'The Roundtable';
            if (chatHeaderTitle) chatHeaderTitle.textContent = config.chat_title || 'The Roundtable';
        })
        .catch(error => {
            console.error('Error fetching app config:', error);
            document.title = 'The Roundtable';
            if (chatHeaderTitle) chatHeaderTitle.textContent = 'The Roundtable';
        });

    function updateConnectionStatus(isConnected, nickname = '') {
        if (isConnected) {
            connectionStatusDiv.textContent = `Status: Connected as ${nickname} (Suping Hard)`;
            connectionStatusDiv.style.color = '#32CD32';
        } else {
            connectionStatusDiv.textContent = 'Status: Disconnected (Probably a Vought Conspiracy)';
            connectionStatusDiv.style.color = '#FF6347';
        }
    }

    function addMessageToUI(messageData) {
        const messageContainer = document.createElement('div');
        messageContainer.classList.add('message-container', messageData.type);
        if (messageData.flair) {
            messageContainer.classList.add(messageData.flair);
        }

        const nicknameSpan = document.createElement('span');
        nicknameSpan.classList.add('message-nickname');
        nicknameSpan.textContent = messageData.nickname + ':';

        const textSpan = document.createElement('span');
        textSpan.classList.add('message-text');
        textSpan.textContent = messageData.text;

        const timestampSpan = document.createElement('span');
        timestampSpan.classList.add('message-timestamp');
        try {
            timestampSpan.textContent = new Date(messageData.timestamp).toLocaleTimeString();
        } catch (e) {
            timestampSpan.textContent = 'invalid date';
        }
        
        if (messageData.type === 'system_notification') {
            messageContainer.appendChild(textSpan);
        } else {
            messageContainer.appendChild(nicknameSpan);
            messageContainer.appendChild(textSpan);
        }
        messageContainer.appendChild(timestampSpan);
        
        messagesDiv.appendChild(messageContainer);
        messagesDiv.scrollTop = messagesDiv.scrollHeight;
    }

    function handleJoinChat() {
        const nickname = nicknameInput.value.trim();
        if (!nickname) {
            alert('A supe needs a codename, genius!');
            return;
        }
        currentNickname = nickname;

        nicknamePromptOverlay.style.display = 'none';
        chatArea.style.display = 'flex';
        messageInput.focus();

        connectWebSocket(nickname);
    }

    function connectWebSocket(nickname) {
        if (ws && ws.readyState === WebSocket.OPEN) {
            ws.close();
        }

        const protocol = window.location.protocol === 'https:' ? 'wss:' : 'ws:';
        ws = new WebSocket(`${protocol}//${window.location.host}/ws/${encodeURIComponent(nickname)}`);

        ws.onopen = () => {
            console.log('Connected to Vought Secure Comms Link.');
            updateConnectionStatus(true, nickname);
        };

        ws.onmessage = (event) => {
            try {
                const data = JSON.parse(event.data);
                if (data.type === 'history_batch' && Array.isArray(data.messages)) {
                    messagesDiv.innerHTML = '';
                    data.messages.forEach(msg => addMessageToUI(msg));
                } else {
                    addMessageToUI(data);
                }
            } catch (e) {
                console.error('Error parsing message or message format incorrect:', event.data, e);
            }
        };

        ws.onerror = (error) => {
            console.error('WebSocket Error:', error);
            addMessageToUI({ type: 'system_notification', nickname: 'System', text: 'Comms link compromised! Error with Vought network.', timestamp: new Date().toISOString(), flair: 'error' });
            updateConnectionStatus(false);
        };

        ws.onclose = (event) => {
            console.log('Disconnected from Vought Secure Comms Link. Reason:', event.reason, 'Code:', event.code);
            updateConnectionStatus(false);
            if (!event.wasClean) {
                addMessageToUI({ type: 'system_notification', nickname: 'System', text: 'Connection severed abruptly. Vought tech support is a joke.', timestamp: new Date().toISOString(), flair: 'error' });
            }
        };
    }

    function sendMessage() {
        if (ws && ws.readyState === WebSocket.OPEN) {
            const messageText = messageInput.value.trim();
            if (messageText) {
                ws.send(JSON.stringify({ text: messageText }));
                messageInput.value = '';
                messageInput.focus();
            }
        } else {
            alert('Not connected to the Roundtable. Try rejoining, you diabolical...');
        }
    }

    joinButton.addEventListener('click', handleJoinChat);
    nicknameInput.addEventListener('keypress', (event) => {
        if (event.key === 'Enter') {
            handleJoinChat();
        }
    });

    sendButton.addEventListener('click', sendMessage);
    messageInput.addEventListener('keypress', (event) => {
        if (event.key === 'Enter') {
            sendMessage();
        }
    });

    updateConnectionStatus(false);
    nicknameInput.focus();
});
