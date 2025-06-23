
import React, { useState, useCallback } from 'react';
import { GeminiActionType, NotificationItem, ViewType, DashboardSettings, AggregatedItem, ItemType } from '../types'; 
import { analyzeWithGemini } from '../services/geminiService'; 
import Spinner from '../components/Spinner';

interface AiDataProcessingFeatureProps {
  addNotification: ( 
    type: NotificationItem['type'], 
    message: string, 
    sourceDetails?: { 
      sourceView?: ViewType; 
      sourceItemId?: string;
    }
  ) => void;
  dashboardSettings: DashboardSettings; 
}

interface AiUtilitySectionProps {
  title: string;
  actionType: GeminiActionType;
  onProcess: (
    text: string, 
    action: GeminiActionType,
    customSysInstruction?: string,
    customUserSegment?: string,
    settings?: DashboardSettings 
  ) => Promise<string | string[]>;
  addNotification: AiDataProcessingFeatureProps['addNotification']; 
  dashboardSettings: DashboardSettings; 
  inputType?: 'textarea' | 'text' | 'dual-textarea'; 
  inputPlaceholder?: string;
  inputPlaceholder2?: string; 
  outputLabel?: string;
  buttonText?: string;
  outputType?: 'text' | 'tags' | 'schema' | 'json'; 
  description?: string; 
}

const AiUtilitySection: React.FC<AiUtilitySectionProps> = ({
  title, actionType, onProcess, addNotification, dashboardSettings, inputType = 'textarea', 
  inputPlaceholder = "Enter text or context...", inputPlaceholder2 = "Enter second text for comparison...",
  outputLabel = "Result", buttonText = "Process with AI", outputType = 'text', description
}) => {
  const [inputText, setInputText] = useState('');
  const [inputText2, setInputText2] = useState(''); 
  const [outputText, setOutputText] = useState<string | string[] | null>(null);
  const [isLoading, setIsLoading] = useState(false);
  const [showAdvanced, setShowAdvanced] = useState(false);
  const [customSystemInstruction, setCustomSystemInstruction] = useState('');
  const [customUserSegment, setCustomUserSegment] = useState('');

  const TEXT_SEPARATOR_FOR_COMPARISON = "---TEXT_SEPARATOR_FOR_AI_COMPARISON---";

  const handleProcessClick = async () => {
    let combinedInput = inputText.trim();
    if (inputType === 'dual-textarea') {
      if (!inputText.trim() || !inputText2.trim()) {
        addNotification('info', 'Please enter text in both input fields for comparison.', { sourceView: ViewType.AI_DATA_PROCESSING });
        return;
      }
      combinedInput = `${inputText.trim()}${TEXT_SEPARATOR_FOR_COMPARISON}${inputText2.trim()}`;
    } else if (!inputText.trim()) {
      addNotification('info', 'Please enter text or context.', { sourceView: ViewType.AI_DATA_PROCESSING });
      return;
    }

    setIsLoading(true); setOutputText(null);
    try {
      const result = await onProcess(
        combinedInput, 
        actionType, 
        customSystemInstruction.trim() || undefined,
        customUserSegment.trim() || undefined,
        dashboardSettings 
      );
      setOutputText(result);
      addNotification('success', `${title} completed.`, { sourceView: ViewType.AI_DATA_PROCESSING });
    } catch (error) {
      const errMsg = error instanceof Error ? error.message : String(error);
      addNotification('error', `${title} failed: ${errMsg}`, { sourceView: ViewType.AI_DATA_PROCESSING });
      setOutputText(null);
    } finally { setIsLoading(false); }
  };

  const handleCopyToClipboard = (text: string | string[]) => {
    const textToCopy = Array.isArray(text) ? text.join('\n') : text; 
    navigator.clipboard.writeText(textToCopy)
      .then(() => addNotification('success', 'Result copied!', { sourceView: ViewType.AI_DATA_PROCESSING }))
      .catch(() => addNotification('error', 'Failed to copy.', { sourceView: ViewType.AI_DATA_PROCESSING }));
  };

  const handleAddToVaultClick = () => {
    if (!outputText) return;

    const suggestedName = `${title} - ${inputText.substring(0,20).trim() || 'Output'}`;
    const itemName = window.prompt("Enter a name for the new vault item:", suggestedName);
    if (!itemName || !itemName.trim()) {
      addNotification('info', 'Vault item creation cancelled.');
      return;
    }

    let finalItemType: ItemType = ItemType.TEXT;
    let finalMimeType: string | undefined = 'text/plain';
    let finalContent: string = '';
    let fileNameSuffix = '.txt';

    if (Array.isArray(outputText) && (actionType === GeminiActionType.TAG || actionType === GeminiActionType.EXTRACT_KEYWORDS || actionType === GeminiActionType.SUGGEST_CSV_CHART_TYPE)) { 
        finalContent = JSON.stringify(outputText, null, 2);
        finalItemType = ItemType.GENERIC_FILE;
        finalMimeType = 'application/json';
        fileNameSuffix = '.json';
    } else if (typeof outputText === 'string') {
        finalContent = outputText; 
        if (outputType === 'json' || 
            [GeminiActionType.ANALYZE_CSV_DATA, GeminiActionType.ANALYZE_STYLE_TONE, GeminiActionType.CATEGORIZE_URL_KEYWORDS, GeminiActionType.CONVERT_CSV_TO_JSON].includes(actionType)) {
            finalItemType = ItemType.GENERIC_FILE;
            finalMimeType = 'application/json';
            fileNameSuffix = '.json';
        } else if (actionType === GeminiActionType.CONVERT_HTML_TO_MARKDOWN) {
            finalItemType = ItemType.GENERIC_FILE;
            finalMimeType = 'text/markdown';
            fileNameSuffix = '.md';
        } else if (actionType === GeminiActionType.CONVERT_JSON_TO_CSV) {
            finalItemType = ItemType.GENERIC_FILE;
            finalMimeType = 'text/csv';
            fileNameSuffix = '.csv';
        }
    } else { // Fallback for unexpected outputText format (e.g. if outputText is an array but not specifically handled above)
        finalContent = Array.isArray(outputText) ? outputText.join('\n') : String(outputText);
    }
    
    const newItemName = (finalItemType === ItemType.GENERIC_FILE && !itemName.toLowerCase().endsWith(fileNameSuffix)) 
        ? `${itemName.trim()}${fileNameSuffix}` 
        : itemName.trim();

    const newItem: AggregatedItem = {
      id: crypto.randomUUID(),
      name: newItemName,
      type: finalItemType,
      content: finalContent,
      originalMimeType: finalMimeType,
      createdAt: new Date(),
      geminiAnalysis: {}, 
      fileSize: finalContent.length,
      parentId: undefined 
    };

    try {
      const currentRAGItemsString = localStorage.getItem('rag-agent-items');
      let currentRAGItems: AggregatedItem[] = [];
      if (currentRAGItemsString) {
        currentRAGItems = JSON.parse(currentRAGItemsString).map((item: any) => ({
          ...item,
          createdAt: item.createdAt ? new Date(item.createdAt) : new Date() 
        }));
      }
      const updatedRAGItems = [...currentRAGItems, newItem];
      localStorage.setItem('rag-agent-items', JSON.stringify(updatedRAGItems.map(item => ({...item, createdAt: item.createdAt.toISOString()}))));
      addNotification('success', `"${newItem.name}" added to RAG Vault.`, { sourceView: ViewType.RAG_AGENT, sourceItemId: newItem.id });
    } catch (e) {
      console.error("Error adding item to RAG vault localStorage:", e);
      addNotification('error', 'Failed to add item to RAG Vault.');
    }
  };


  return (
    <section className="bg-slate-800 p-6 rounded-xl shadow-xl">
      <h3 className="text-xl font-semibold text-indigo-400 mb-2">{title}</h3>
      {description && <p className="text-xs text-slate-400 mb-3">{description}</p>}
      <div className="space-y-4">
        {inputType === 'dual-textarea' ? (
          <div className="grid grid-cols-1 md:grid-cols-2 gap-4">
            <div>
              <label htmlFor={`${actionType}-input1`} className="block text-sm font-medium text-slate-300 mb-1">Input Text 1</label>
              <textarea id={`${actionType}-input1`} rows={6} value={inputText} onChange={(e) => setInputText(e.target.value)} placeholder={inputPlaceholder} className="w-full p-3 bg-slate-700 border border-slate-600 rounded-md focus:ring-2 focus:ring-indigo-500 text-slate-100 font-mono text-sm" />
            </div>
            <div>
              <label htmlFor={`${actionType}-input2`} className="block text-sm font-medium text-slate-300 mb-1">Input Text 2</label>
              <textarea id={`${actionType}-input2`} rows={6} value={inputText2} onChange={(e) => setInputText2(e.target.value)} placeholder={inputPlaceholder2} className="w-full p-3 bg-slate-700 border border-slate-600 rounded-md focus:ring-2 focus:ring-indigo-500 text-slate-100 font-mono text-sm" />
            </div>
          </div>
        ) : (
          <div>
            <label htmlFor={`${actionType}-input`} className="block text-sm font-medium text-slate-300 mb-1">Input / Context</label>
            {inputType === 'textarea' ? (
              <textarea id={`${actionType}-input`} rows={actionType === GeminiActionType.GENERATE_TEMPLATE || actionType === GeminiActionType.EXTRACT_SCHEMA || actionType === GeminiActionType.EXTRACT_CODE_STRUCTURE || actionType === GeminiActionType.CONVERT_JSON_TO_CSV || actionType === GeminiActionType.CONVERT_HTML_TO_MARKDOWN ? 8 : 6} value={inputText} onChange={(e) => setInputText(e.target.value)} placeholder={inputPlaceholder} className="w-full p-3 bg-slate-700 border border-slate-600 rounded-md focus:ring-2 focus:ring-indigo-500 text-slate-100 font-mono text-sm" />
            ) : (
              <input type="text" id={`${actionType}-input`} value={inputText} onChange={(e) => setInputText(e.target.value)} placeholder={inputPlaceholder} className="w-full p-3 bg-slate-700 border border-slate-600 rounded-md focus:ring-2 focus:ring-indigo-500 text-slate-100" />
            )}
          </div>
        )}


        <div className="my-3">
            <button onClick={() => setShowAdvanced(!showAdvanced)} className="text-xs text-sky-400 hover:text-sky-300">
                {showAdvanced ? 'Hide' : 'Show'} Advanced Options {showAdvanced ? '▲' : '▼'}
            </button>
            {showAdvanced && (
                <div className="mt-2 space-y-3 p-3 bg-slate-700/50 rounded-md border border-slate-600">
                    <div>
                        <label htmlFor={`${actionType}-sys-prompt`} className="block text-xs font-medium text-slate-300 mb-0.5">Custom System Instruction (Optional)</label>
                        <textarea id={`${actionType}-sys-prompt`} rows={2} value={customSystemInstruction} onChange={e => setCustomSystemInstruction(e.target.value)} placeholder="e.g., Act as a senior software engineer." className="w-full p-2 bg-slate-600 border-slate-500 rounded text-xs font-mono"/>
                    </div>
                    <div>
                        <label htmlFor={`${actionType}-user-seg`} className="block text-xs font-medium text-slate-300 mb-0.5">Additional User Instructions/Context (Optional)</label>
                        <textarea id={`${actionType}-user-seg`} rows={3} value={customUserSegment} onChange={e => setCustomUserSegment(e.target.value)} placeholder="e.g., Focus on Python. Output as JSON Schema." className="w-full p-2 bg-slate-600 border-slate-500 rounded text-xs font-mono"/>
                    </div>
                </div>
            )}
        </div>

        <button onClick={handleProcessClick} disabled={isLoading || (!inputText.trim() && inputType !== 'dual-textarea') || (inputType === 'dual-textarea' && (!inputText.trim() || !inputText2.trim()))} className="px-4 py-2 bg-indigo-600 hover:bg-indigo-700 text-white font-semibold rounded-md disabled:opacity-60 flex items-center justify-center space-x-2">
          {isLoading ? <Spinner size="w-5 h-5" /> : <SparklesIconInternal className="w-5 h-5" />} <span>{buttonText}</span>
        </button>
        {outputText !== null && (
          <div className="mt-4">
            <div className="flex justify-between items-center mb-1">
                <label className="block text-sm font-medium text-slate-300">{outputLabel}</label>
                <div className="flex space-x-2">
                    <button onClick={() => handleCopyToClipboard(outputText)} className="text-xs text-sky-400 hover:text-sky-300 p-1 flex items-center" title="Copy to Clipboard">
                        <CopyIconInternal className="w-4 h-4 mr-1"/>Copy
                    </button>
                    <button onClick={handleAddToVaultClick} disabled={isLoading} className="text-xs text-emerald-400 hover:text-emerald-300 p-1 flex items-center" title="Add to RAG Vault">
                        <PlusCircleIconInternal className="w-4 h-4 mr-1"/>Add to Vault
                    </button>
                </div>
            </div>
            {outputType === 'tags' && Array.isArray(outputText) ? (
              <div className="flex flex-wrap gap-2 p-3 bg-slate-900 border border-slate-700 rounded-md">{outputText.map((tag, index) => (<span key={index} className="px-3 py-1 bg-teal-600 text-teal-100 text-sm rounded-full shadow">{tag}</span>))}</div>
            ) : outputType === 'json' && typeof outputText === 'string' ? (
              <pre className="p-3 bg-slate-900 border border-slate-700 rounded-md text-slate-100 break-words whitespace-pre-wrap font-mono text-sm max-h-96 overflow-y-auto">
                <code dangerouslySetInnerHTML={{ __html: syntaxHighlightJson(outputText) }} />
              </pre>
            ) : (
              <div className={`p-3 bg-slate-900 border border-slate-700 rounded-md text-slate-100 break-words ${outputType === 'schema' || outputType === 'text' ? 'whitespace-pre-wrap font-mono text-sm max-h-96 overflow-y-auto' : ''}`}>{Array.isArray(outputText) ? outputText.join('\n') : outputText}</div>
            )}
          </div>
        )}
      </div>
    </section>
  );
};

// Simple JSON syntax highlighter (can be expanded)
const syntaxHighlightJson = (json: string) => {
    if (typeof json !== 'string') {
        json = JSON.stringify(json, undefined, 2);
    }
    json = json.replace(/&/g, '&amp;').replace(/</g, '&lt;').replace(/>/g, '&gt;');
    return json.replace(/("(\\u[a-zA-Z0-9]{4}|\\[^u]|[^\\"])*"(\s*:)?|\b(true|false|null)\b|-?\d+(?:\.\d*)?(?:[eE][+\-]?\d+)?)/g, (match) => {
        let cls = 'var(--theme-json-number)'; // number
        if (/^"/.test(match)) {
            if (/:$/.test(match)) {
                cls = 'var(--theme-json-key)'; // key
            } else {
                cls = 'var(--theme-json-string)'; // string
            }
        } else if (/true|false/.test(match)) {
            cls = 'var(--theme-json-boolean)'; // boolean
        } else if (/null/.test(match)) {
            cls = 'var(--theme-json-null)'; // null
        }
        return `<span style="color:${cls}">${match}</span>`;
    }).replace(/([{}[\]])/g, (match) => `<span style="color:var(--theme-json-brackets)">${match}</span>`)
      .replace(/(,)/g, (match) => `<span style="color:var(--theme-json-comma)">${match}</span>`);
};


const AiDataProcessingFeature: React.FC<AiDataProcessingFeatureProps> = ({ addNotification, dashboardSettings }) => {
  const processWithAi = useCallback(async (
      text: string, 
      action: GeminiActionType,
      customSysInstruction?: string,
      customUserSegment?: string,
      settings?: DashboardSettings 
    ): Promise<string | string[]> => {
    return analyzeWithGemini(text, action, undefined, customSysInstruction, customUserSegment, settings); 
  }, []);

  return (
    <div className="space-y-10">
      <header className="text-center mb-10">
        <h1 className="text-4xl font-bold text-transparent bg-clip-text bg-gradient-to-r from-indigo-400 to-purple-400">AI Tools</h1>
        <p className="text-slate-400 mt-1 text-md">Leverage Gemini AI for various text, data, and code analysis and generation tasks.</p>
      </header>

      <section>
        <h2 className="text-2xl font-semibold text-sky-300 mb-6 border-b border-sky-300/30 pb-2">Data Analysis & Generation Tools</h2>
        <div className="grid grid-cols-1 md:grid-cols-2 gap-8">
            <AiUtilitySection title="Text Summarizer" actionType={GeminiActionType.SUMMARIZE} onProcess={processWithAi} addNotification={addNotification} dashboardSettings={dashboardSettings} inputPlaceholder="Paste long text for summary..." outputLabel="Summary" buttonText="Summarize" description="Condenses long text into a concise summary." />
            <AiUtilitySection title="Keyword Extractor" actionType={GeminiActionType.EXTRACT_KEYWORDS} onProcess={processWithAi} addNotification={addNotification} dashboardSettings={dashboardSettings} inputPlaceholder="Enter text to extract keywords..." outputLabel="Keywords" buttonText="Extract Keywords" outputType="tags" description="Identifies and extracts key terms from text."/>
            <AiUtilitySection title="Sentiment Analyzer" actionType={GeminiActionType.ANALYZE_SENTIMENT} onProcess={processWithAi} addNotification={addNotification} dashboardSettings={dashboardSettings} inputPlaceholder="Enter text for sentiment analysis..." outputLabel="Sentiment" buttonText="Analyze Sentiment" description="Determines the emotional tone (Positive, Negative, Neutral) of text."/>
            <AiUtilitySection title="Schema Extractor" actionType={GeminiActionType.EXTRACT_SCHEMA} onProcess={processWithAi} addNotification={addNotification} dashboardSettings={dashboardSettings} inputPlaceholder="Paste code, JSON, or text to infer its structure..." outputLabel="Inferred Schema" buttonText="Extract Schema" outputType="schema" description="Infers data structure or schema from content like JSON or code."/>
            <AiUtilitySection title="Template Generator" actionType={GeminiActionType.GENERATE_TEMPLATE} onProcess={processWithAi} addNotification={addNotification} dashboardSettings={dashboardSettings} inputPlaceholder="Describe the template you need (e.g., 'Python script for API call', 'Dockerfile for Node.js app')." outputLabel="Generated Template" buttonText="Generate Template" outputType="text" description="Generates boilerplate code or document templates based on your description."/>
        </div>
      </section>
      
      <section>
        <h2 className="text-2xl font-semibold text-teal-300 mb-6 border-b border-teal-300/30 pb-2">Code Intelligence Suite</h2>
        <div className="grid grid-cols-1 md:grid-cols-2 gap-8">
            <AiUtilitySection title="Code Structure Analyzer" actionType={GeminiActionType.EXTRACT_CODE_STRUCTURE} onProcess={processWithAi} addNotification={addNotification} dashboardSettings={dashboardSettings} inputPlaceholder="Paste code to analyze its structure..." outputLabel="Code Structure Analysis" buttonText="Analyze Structure" outputType="text" description="Identifies main functions, classes, and methods in code and describes their purpose."/>
            <AiUtilitySection title="API Endpoint Identifier" actionType={GeminiActionType.IDENTIFY_API_ENDPOINTS} onProcess={processWithAi} addNotification={addNotification} dashboardSettings={dashboardSettings} inputPlaceholder="Paste code to identify API endpoints..." outputLabel="Identified API Endpoints" buttonText="Identify Endpoints" outputType="text" description="Scans code to find and list web API endpoint definitions."/>
            <AiUtilitySection title="Dependency Lister" actionType={GeminiActionType.LIST_DEPENDENCIES} onProcess={processWithAi} addNotification={addNotification} dashboardSettings={dashboardSettings} inputPlaceholder="Paste code to list its dependencies..." outputLabel="Identified Dependencies" buttonText="List Dependencies" outputType="text" description="Extracts primary import statements or external library dependencies from code."/>
        </div>
      </section>

      <section>
        <h2 className="text-2xl font-semibold text-fuchsia-400 mb-6 border-b border-fuchsia-400/30 pb-2">Advanced Text & Document Processing</h2>
        <div className="grid grid-cols-1 md:grid-cols-2 gap-8">
          <AiUtilitySection title="Comparative Text Summarizer" actionType={GeminiActionType.COMPARATIVE_SUMMARIZE_TEXTS} onProcess={processWithAi} addNotification={addNotification} dashboardSettings={dashboardSettings} inputType="dual-textarea" inputPlaceholder="Enter first text here..." inputPlaceholder2="Enter second text here..." outputLabel="Comparative Summary" buttonText="Compare & Summarize" outputType="text" description="Summarizes two texts and highlights their differences and similarities."/>
          <AiUtilitySection title="Document Q&A (Contextual)" actionType={GeminiActionType.DOCUMENT_QA} onProcess={processWithAi} addNotification={addNotification} dashboardSettings={dashboardSettings} inputPlaceholder="Paste document text here... Then use 'Additional User Instructions' for your question." outputLabel="Answer" buttonText="Ask Question" outputType="text" description="Answers questions based on the provided document text as context."/>
          <AiUtilitySection title="Writing Style & Tone Analyzer" actionType={GeminiActionType.ANALYZE_STYLE_TONE} onProcess={processWithAi} addNotification={addNotification} dashboardSettings={dashboardSettings} inputPlaceholder="Enter text to analyze style and tone..." outputLabel="Style & Tone Analysis" buttonText="Analyze Style" outputType="json" description="Identifies writing style (formal, informal) and emotional tone of text."/>
        </div>
      </section>

      <section>
        <h2 className="text-2xl font-semibold text-rose-400 mb-6 border-b border-rose-400/30 pb-2">Creative Content Generation</h2>
        <div className="grid grid-cols-1 md:grid-cols-2 gap-8">
          <AiUtilitySection title="Story Idea Generator" actionType={GeminiActionType.GENERATE_STORY_IDEAS} onProcess={processWithAi} addNotification={addNotification} dashboardSettings={dashboardSettings} inputPlaceholder="Enter genre, keywords, premise (e.g., Sci-Fi, Space Opera, Alien Artifact)..." outputLabel="Story Ideas" buttonText="Generate Ideas" outputType="text" description="Generates story ideas or plot hooks based on genre, keywords, and premise."/>
          <AiUtilitySection title="AI Marketing Copywriter" actionType={GeminiActionType.GENERATE_MARKETING_COPY} onProcess={processWithAi} addNotification={addNotification} dashboardSettings={dashboardSettings} inputPlaceholder="Product/Service description and target audience (e.g., Eco-friendly water bottle, for outdoor enthusiasts)..." outputLabel="Marketing Copy" buttonText="Write Copy" outputType="text" description="Creates short marketing blurbs or social media posts."/>
          <AiUtilitySection title="Poem & Verse Generator" actionType={GeminiActionType.GENERATE_POEM} onProcess={processWithAi} addNotification={addNotification} dashboardSettings={dashboardSettings} inputPlaceholder="Enter theme and desired style (e.g., Love, Haiku; Nature, Free Verse; Limerick, Humorous)..." outputLabel="Generated Poem" buttonText="Generate Poem" outputType="text" description="Crafts short poems based on a theme and chosen style."/>
        </div>
      </section>
      
      <section>
        <h2 className="text-2xl font-semibold text-amber-400 mb-6 border-b border-amber-400/30 pb-2">Enhanced Data & CSV Utilities</h2>
        <div className="grid grid-cols-1 md:grid-cols-2 gap-8">
            <AiUtilitySection title="CSV Data Analyzer" actionType={GeminiActionType.ANALYZE_CSV_DATA} onProcess={processWithAi} addNotification={addNotification} dashboardSettings={dashboardSettings} inputPlaceholder="Paste CSV data here..." outputLabel="CSV Analysis (JSON)" buttonText="Analyze CSV" outputType="json" description="Analyzes CSV data structure, infers column types, and provides basic stats."/>
            <AiUtilitySection title="CSV to JSON Converter" actionType={GeminiActionType.CONVERT_CSV_TO_JSON} onProcess={processWithAi} addNotification={addNotification} dashboardSettings={dashboardSettings} inputPlaceholder="Paste CSV data to convert to JSON array..." outputLabel="JSON Output" buttonText="CSV to JSON" outputType="json" description="Converts CSV data into a JSON array of objects."/>
            <AiUtilitySection title="CSV to Markdown Table Converter" actionType={GeminiActionType.CONVERT_CSV_TO_MARKDOWN_TABLE} onProcess={processWithAi} addNotification={addNotification} dashboardSettings={dashboardSettings} inputPlaceholder="Paste CSV data to convert to Markdown table..." outputLabel="Markdown Table" buttonText="CSV to Markdown" outputType="text" description="Converts CSV data into a Markdown formatted table."/>
            <AiUtilitySection title="CSV Data Profiler & Insights" actionType={GeminiActionType.PROFILE_CSV_DATA_INSIGHTS} onProcess={processWithAi} addNotification={addNotification} dashboardSettings={dashboardSettings} inputPlaceholder="Paste CSV data here..." outputLabel="Data Profile & Insights" buttonText="Profile & Get Insights" outputType="text" description="Provides a detailed CSV profile and generates textual insights from the data."/>
            <AiUtilitySection title="CSV to Chart Type Suggester" actionType={GeminiActionType.SUGGEST_CSV_CHART_TYPE} onProcess={processWithAi} addNotification={addNotification} dashboardSettings={dashboardSettings} inputPlaceholder="Paste CSV data here..." outputLabel="Chart Suggestions (JSON)" buttonText="Suggest Charts" outputType="json" description="Analyzes CSV data and suggests suitable chart types with reasoning."/>
            <AiUtilitySection title="JSON to CSV Converter" actionType={GeminiActionType.CONVERT_JSON_TO_CSV} onProcess={processWithAi} addNotification={addNotification} dashboardSettings={dashboardSettings} inputPlaceholder="Paste JSON (array of objects or single object)..." outputLabel="CSV Output" buttonText="Convert to CSV" outputType="text" description="Transforms JSON data into CSV format."/>
        </div>
      </section>

      <section>
        <h2 className="text-2xl font-semibold text-lime-400 mb-6 border-b border-lime-400/30 pb-2">Web & URL Intelligence Suite</h2>
        <div className="grid grid-cols-1 md:grid-cols-2 gap-8">
            <AiUtilitySection title="Webpage Change Detector (Conceptual)" actionType={GeminiActionType.DETECT_WEBPAGE_CHANGES_CONCEPTUAL} onProcess={processWithAi} addNotification={addNotification} dashboardSettings={dashboardSettings} inputType="dual-textarea" inputPlaceholder="Current Webpage URL..." inputPlaceholder2="Optional: Previous content snapshot/summary..." outputLabel="Conceptual Change Analysis" buttonText="Analyze Potential Changes" outputType="text" description="Conceptually analyzes potential changes to a webpage based on URL and optional previous content (no live fetching)."/>
            <AiUtilitySection title="URL Categorizer & Keyword Guesser" actionType={GeminiActionType.CATEGORIZE_URL_KEYWORDS} onProcess={processWithAi} addNotification={addNotification} dashboardSettings={dashboardSettings} inputType="text" inputPlaceholder="Enter URL to categorize (e.g., https://www.example.com/blog/tech)..." outputLabel="URL Analysis (JSON)" buttonText="Categorize URL" outputType="json" description="Categorizes a website and guesses primary keywords based on its URL (no live fetching)."/>
            <AiUtilitySection title="HTML to Markdown Converter" actionType={GeminiActionType.CONVERT_HTML_TO_MARKDOWN} onProcess={processWithAi} addNotification={addNotification} dashboardSettings={dashboardSettings} inputPlaceholder="Paste HTML content here..." outputLabel="Markdown Output" buttonText="Convert to Markdown" outputType="text" description="Converts HTML content into Markdown format."/>
        </div>
      </section>

       <section>
        <h2 className="text-2xl font-semibold text-cyan-400 mb-6 border-b border-cyan-400/30 pb-2">Advanced Code & Development Aids</h2>
        <div className="grid grid-cols-1 md:grid-cols-2 gap-8">
            <AiUtilitySection title="Code Refactoring Suggester" actionType={GeminiActionType.SUGGEST_CODE_REFACTORING} onProcess={processWithAi} addNotification={addNotification} dashboardSettings={dashboardSettings} inputPlaceholder="Paste code snippet... Then use 'Additional User Instructions' for desired improvement (e.g., 'make more readable')." outputLabel="Refactoring Suggestions" buttonText="Suggest Refactoring" outputType="text" description="Suggests refactoring approaches for code snippets based on desired improvements."/>
            <AiUtilitySection title="Unit Test Case Idea Generator" actionType={GeminiActionType.GENERATE_UNIT_TEST_CASES} onProcess={processWithAi} addNotification={addNotification} dashboardSettings={dashboardSettings} inputPlaceholder="Paste function/method code here..." outputLabel="Test Case Ideas" buttonText="Generate Test Ideas" outputType="text" description="Generates basic unit test case ideas or stubs for provided code."/>
            <AiUtilitySection title="AI API Documentation Writer" actionType={GeminiActionType.GENERATE_API_DOCUMENTATION} onProcess={processWithAi} addNotification={addNotification} dashboardSettings={dashboardSettings} inputPlaceholder="Paste code snippet (function, class, API route)..." outputLabel="Markdown API Documentation" buttonText="Write API Docs" outputType="text" description="Generates a basic Markdown API documentation outline for code."/>
        </div>
      </section>

      <section className="bg-slate-800 p-6 rounded-xl shadow-xl mt-10 border-2 border-dashed border-slate-700">
        <h2 className="text-2xl font-semibold text-yellow-300 mb-4">Your Custom AI Tools & Prompt Chains</h2>
        <p className="text-slate-400 text-center">
          Forge your own AI-powered utilities here. Define custom prompts and chain actions.
          <br />
          <em>(Advanced feature in development). Stay tuned, Architect.</em>
        </p>
      </section>
    </div>
  );
};

// --- Internal Icons ---
const SparklesIconInternal = (props: React.SVGProps<SVGSVGElement>) => <svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" strokeWidth={1.5} stroke="currentColor" {...props}><path strokeLinecap="round" strokeLinejoin="round" d="M9.813 15.904L9 18.75l-.813-2.846a4.5 4.5 0 00-3.09-3.09L1.25 12l2.846-.813a4.5 4.5 0 003.09-3.09L9 5.25l.813 2.846a4.5 4.5 0 003.09 3.09L15.75 12l-2.846.813a4.5 4.5 0 00-3.09 3.09zM18.25 12l2.846-.813a4.5 4.5 0 003.09-3.09L24 5.25l-.813 2.846a4.5 4.5 0 00-3.09 3.09L18.25 12zm0 0l-2.846.813a4.5 4.5 0 00-3.09 3.09L12 18.75l.813-2.846a4.5 4.5 0 003.09-3.09L18.25 12z" /></svg>;
const CopyIconInternal = (props: React.SVGProps<SVGSVGElement>) => <svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" strokeWidth={1.5} stroke="currentColor" {...props}><path strokeLinecap="round" strokeLinejoin="round" d="M15.666 3.888A2.25 2.25 0 0013.5 2.25h-3c-1.03 0-1.9.693-2.166 1.638m7.332 0c.055.194.084.4.084.612v0a.75.75 0 01-.75.75H9a.75.75 0 01-.75-.75v0c0-.212.03-.418.084-.612m7.332 0c.646.049 1.288.11 1.927.184 1.1.128 1.907 1.077 1.907 2.185V19.5a2.25 2.25 0 01-2.25 2.25H6.75A2.25 2.25 0 014.5 19.5V6.257c0-1.108.806-2.057 1.907-2.185a48.208 48.208 0 011.927-.184" /></svg>;
const PlusCircleIconInternal = (props: React.SVGProps<SVGSVGElement>) => <svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" strokeWidth={1.5} stroke="currentColor" {...props}><path strokeLinecap="round" strokeLinejoin="round" d="M12 9v6m3-3H9m12 0a9 9 0 11-18 0 9 9 0 0118 0z" /></svg>;

export default AiDataProcessingFeature;
