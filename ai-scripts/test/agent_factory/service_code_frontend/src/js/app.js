document.addEventListener('DOMContentLoaded', () => {
    const API_URL = `http://localhost:8001/api/v1/agents`; // Using localhost and mapped port
    
    const agentList = document.getElementById('agentList');
    const createNewAgentBtn = document.getElementById('createNewAgentBtn');
    const saveAgentBtn = document.getElementById('saveAgentBtn');
    const deleteAgentBtn = document.getElementById('deleteAgentBtn');
    const detailTitle = document.getElementById('detail-title');
    const placeholderText = document.querySelector('.placeholder');

    const editorContainer = document.getElementById('jsoneditor');
    const editor = new JSONEditor(editorContainer, { mode: 'code' });

    let currentAgentId = null;

    async function fetchAgents() {
        try {
            const response = await fetch(API_URL);
            if (!response.ok) throw new Error('Failed to fetch agents');
            const agents = await response.json();
            renderAgentList(agents);
        } catch (error) {
            console.error('Error fetching agents:', error);
            agentList.innerHTML = '<li>Error loading agents.</li>';
        }
    }

    function renderAgentList(agents) {
        agentList.innerHTML = '';
        agents.forEach(agent => {
            const li = document.createElement('li');
            li.dataset.id = agent.id;
            li.innerHTML = `
                <div class="agent-name">${agent.definition?.agent_profile?.name || 'Unnamed Agent'}</div>
                <div class="agent-role">${agent.definition?.agent_profile?.role || 'No role specified'}</div>
            `;
            li.addEventListener('click', () => selectAgent(agent));
            agentList.appendChild(li);
        });
    }

    function selectAgent(agent) {
        currentAgentId = agent.id;
        detailTitle.textContent = agent.definition?.agent_profile?.name || 'Editing Agent';
        editor.set(agent.definition);
        editorContainer.classList.remove('hidden');
        placeholderText.classList.add('hidden');
        saveAgentBtn.classList.remove('hidden');
        deleteAgentBtn.classList.remove('hidden');

        // Highlight active item
        document.querySelectorAll('#agentList li').forEach(li => {
            li.classList.toggle('active', li.dataset.id == currentAgentId);
        });
    }

    async function saveAgent() {
        if (currentAgentId === 'new') {
            // Create new agent
            try {
                const newAgentDef = { definition: editor.get() };
                const response = await fetch(API_URL, {
                    method: 'POST',
                    headers: { 'Content-Type': 'application/json' },
                    body: JSON.stringify(newAgentDef)
                });
                if (!response.ok) throw new Error('Failed to create agent');
                await fetchAgents();
                // Select the newly created agent (requires getting its ID back)
                const newAgent = await response.json();
                selectAgent(newAgent);
            } catch (error) {
                console.error('Error creating agent:', error);
                alert('Error: Could not create agent.');
            }
        } else {
            // Update existing agent
            try {
                const updatedAgentDef = { definition: editor.get() };
                const response = await fetch(`${API_URL}/${currentAgentId}`, {
                    method: 'PUT',
                    headers: { 'Content-Type': 'application/json' },
                    body: JSON.stringify(updatedAgentDef)
                });
                if (!response.ok) throw new Error('Failed to update agent');
                await fetchAgents();
                alert('Agent saved successfully!');
            } catch (error) {
                console.error('Error updating agent:', error);
                alert('Error: Could not save agent.');
            }
        }
    }

    async function deleteAgent() {
        if (!currentAgentId || currentAgentId === 'new') return;
        if (!confirm('Are you sure you want to delete this agent?')) return;

        try {
            const response = await fetch(`${API_URL}/${currentAgentId}`, {
                method: 'DELETE'
            });
            if (!response.ok) throw new Error('Failed to delete agent');
            resetDetailView();
            await fetchAgents();
            alert('Agent deleted successfully!');
        } catch (error) {
            console.error('Error deleting agent:', error);
            alert('Error: Could not delete agent.');
        }
    }

    function createNewAgent() {
        currentAgentId = 'new';
        detailTitle.textContent = 'Create New Agent Blueprint';
        editor.set({ agent_profile: { name: 'New Agent' } }); // Start with a minimal template
        editorContainer.classList.remove('hidden');
        placeholderText.classList.add('hidden');
        saveAgentBtn.classList.remove('hidden');
        deleteAgentBtn.classList.add('hidden');
        document.querySelectorAll('#agentList li').forEach(li => li.classList.remove('active'));
    }

    function resetDetailView() {
        currentAgentId = null;
        detailTitle.textContent = 'Select an Agent';
        editor.set({});
        editorContainer.classList.add('hidden');
        placeholderText.classList.remove('hidden');
        saveAgentBtn.classList.add('hidden');
        deleteAgentBtn.classList.add('hidden');
    }

    createNewAgentBtn.addEventListener('click', createNewAgent);
    saveAgentBtn.addEventListener('click', saveAgent);
    deleteAgentBtn.addEventListener('click', deleteAgent);

    // Initial Load
    fetchAgents();
    resetDetailView();
});
