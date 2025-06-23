
import React, { useState, useCallback } from 'react';
import { NotificationItem, ViewType } from '../types'; 

interface StdUtilsFeatureProps {
  addNotification: ( 
    type: NotificationItem['type'], 
    message: string, 
    sourceDetails?: { 
      sourceView?: ViewType; 
    }
  ) => void;
}

const UtilitySection: React.FC<{ title: string; children: React.ReactNode }> = ({ title, children }) => (
  <section className="bg-slate-800 p-6 rounded-xl shadow-xl">
    <h2 className="text-2xl font-semibold text-amber-400 mb-6">{title}</h2>
    <div className="space-y-4">
      {children}
    </div>
  </section>
);

const ClickToCopyOutput: React.FC<{ id: string; value: string; rows?: number; placeholder?: string; addNotification: StdUtilsFeatureProps['addNotification']; label?: string }> =
  ({ id, value, rows = 5, placeholder = "Output will appear here...", addNotification, label = "Output" }) => {
  const handleCopy = useCallback(() => {
    if (value) {
      navigator.clipboard.writeText(value);
      addNotification('success', 'Output copied to clipboard!', {sourceView: ViewType.STD_UTILS});
    }
  }, [value, addNotification]);

  return (
    <div>
      <label htmlFor={id} className="block text-sm font-medium text-slate-300 mb-1">
        {label} (click to copy)
      </label>
      <textarea
        id={id}
        rows={rows}
        value={value}
        readOnly
        placeholder={placeholder}
        className="w-full p-3 bg-slate-900 border border-slate-700 rounded-md text-slate-100 placeholder-slate-500 cursor-copy"
        aria-label={label}
        onClick={handleCopy}
      />
    </div>
  );
};


const StdUtilsFeature: React.FC<StdUtilsFeatureProps> = ({ addNotification }) => {
  const [base64Input, setBase64Input] = useState('');
  const [base64Output, setBase64Output] = useState('');
  const [urlInput, setUrlInput] = useState('');
  const [urlOutput, setUrlOutput] = useState('');
  const [timestampInput, setTimestampInput] = useState<string>(Math.floor(Date.now() / 1000).toString());
  const [readableDateOutput, setReadableDateOutput] = useState<string>(new Date().toLocaleString());
  const [unixTimestampOutput, setUnixTimestampOutput] = useState<string>(Math.floor(Date.now() / 1000).toString());
  const [isMillis, setIsMillis] = useState(false);
  const [jsonInput, setJsonInput] = useState('');
  const [jsonOutput, setJsonOutput] = useState('');
  const [jsonIndent, setJsonIndent] = useState<2 | 4 | 'tab'>(2);
  const [textInspectorInput, setTextInspectorInput] = useState('');
  const [generatedUuid, setGeneratedUuid] = useState('');

  const handleBase64Encode = useCallback(() => {
    try {
      const encoded = btoa(unescape(encodeURIComponent(base64Input)));
      setBase64Output(encoded);
      addNotification('success', 'Text encoded to Base64.', {sourceView: ViewType.STD_UTILS});
    } catch (error) {
      addNotification('error', 'Base64 Encoding Failed.', {sourceView: ViewType.STD_UTILS});
      setBase64Output('');
    }
  }, [base64Input, addNotification]);

  const handleBase64Decode = useCallback(() => {
    try {
      const decoded = decodeURIComponent(escape(atob(base64Input)));
      setBase64Output(decoded);
      addNotification('success', 'Base64 decoded.', {sourceView: ViewType.STD_UTILS});
    } catch (error) {
      addNotification('error', 'Base64 Decoding Failed.', {sourceView: ViewType.STD_UTILS});
      setBase64Output('');
    }
  }, [base64Input, addNotification]);

  const handleBase64Swap = useCallback(() => { setBase64Input(base64Output); setBase64Output(base64Input); }, [base64Input, base64Output]);
  const handleBase64Clear = useCallback(() => { setBase64Input(''); setBase64Output(''); }, []);

  const handleUrlEncode = useCallback(() => {
    try { setUrlOutput(encodeURIComponent(urlInput)); addNotification('success', 'URL Encoded.', {sourceView: ViewType.STD_UTILS}); } 
    catch (e) { addNotification('error', 'Failed to encode URL.', {sourceView: ViewType.STD_UTILS}); setUrlOutput(''); }
  }, [urlInput, addNotification]);

  const handleUrlDecode = useCallback(() => {
    try { setUrlOutput(decodeURIComponent(urlInput)); addNotification('success', 'URL Decoded.', {sourceView: ViewType.STD_UTILS}); } 
    catch (e) { addNotification('error', 'Failed to decode URL.', {sourceView: ViewType.STD_UTILS}); setUrlOutput(''); }
  }, [urlInput, addNotification]);
  const handleUrlClear = useCallback(() => { setUrlInput(''); setUrlOutput(''); }, []);

  const handleTimestampToReadable = useCallback(() => {
    try {
      const num = parseInt(timestampInput, 10);
      if (isNaN(num)) throw new Error("Invalid number");
      const date = new Date(isMillis ? num : num * 1000);
      setReadableDateOutput(date.toLocaleString());
      setUnixTimestampOutput(Math.floor(date.getTime() / (isMillis ? 1 : 1000)).toString());
      addNotification('success', 'Timestamp to readable date.', {sourceView: ViewType.STD_UTILS});
    } catch (e) { addNotification('error', 'Invalid timestamp.', {sourceView: ViewType.STD_UTILS}); }
  }, [timestampInput, isMillis, addNotification]);

  const handleReadableToTimestamp = useCallback(() => {
    try {
      const date = new Date(timestampInput);
      if (isNaN(date.getTime())) throw new Error("Invalid date string");
      setUnixTimestampOutput(Math.floor(date.getTime() / (isMillis ? 1 : 1000)).toString());
      setReadableDateOutput(date.toLocaleString());
      addNotification('success', 'Date to timestamp.', {sourceView: ViewType.STD_UTILS});
    } catch (e) { addNotification('error', 'Invalid date string.', {sourceView: ViewType.STD_UTILS}); }
  }, [timestampInput, isMillis, addNotification]);

  const handleTimestampNow = useCallback(() => {
    const now = new Date();
    setTimestampInput(isMillis ? now.getTime().toString() : Math.floor(now.getTime() / 1000).toString());
    setReadableDateOutput(now.toLocaleString());
    setUnixTimestampOutput(isMillis ? now.getTime().toString() : Math.floor(now.getTime() / 1000).toString());
  }, [isMillis]);
  const handleTimestampClear = useCallback(() => { handleTimestampNow(); }, [handleTimestampNow]);

  const handleJsonFormat = useCallback(() => {
    try {
      const parsed = JSON.parse(jsonInput);
      const indentSpace = jsonIndent === 'tab' ? '\t' : jsonIndent;
      setJsonOutput(JSON.stringify(parsed, null, indentSpace));
      addNotification('success', 'JSON formatted.', {sourceView: ViewType.STD_UTILS});
    } catch (e) {
      const error = e instanceof Error ? e.message : "Invalid JSON";
      setJsonOutput(`Error: ${error}`);
      addNotification('error', `JSON error: ${error}`, {sourceView: ViewType.STD_UTILS});
    }
  }, [jsonInput, jsonIndent, addNotification]);
  const handleJsonClear = useCallback(() => { setJsonInput(''); setJsonOutput(''); }, []);

  const charCount = textInspectorInput.length;
  const wordCount = textInspectorInput.trim() ? textInspectorInput.trim().split(/\s+/).length : 0;
  const lineCount = textInspectorInput.split('\n').length;

  const handleGenerateUuid = useCallback(() => {
    const newUuid = crypto.randomUUID();
    setGeneratedUuid(newUuid);
    addNotification('success', 'New UUID v4 generated!', {sourceView: ViewType.STD_UTILS});
  }, [addNotification]);

  return (
    <div className="space-y-10">
      <header className="text-center mb-10">
        <h1 className="text-4xl font-bold text-transparent bg-clip-text bg-gradient-to-r from-amber-400 to-yellow-300">Std::Utils</h1>
        <p className="text-slate-400 mt-1 text-md">Converters, generators, formatters.</p>
      </header>
      <UtilitySection title="Base64 Encoder/Decoder">
        <div><label htmlFor="base64-input" className="block text-sm font-medium text-slate-300 mb-1">Input</label><textarea id="base64-input" rows={4} value={base64Input} onChange={(e) => setBase64Input(e.target.value)} placeholder="Text to encode/decode" className="w-full p-3 bg-slate-700 border border-slate-600 rounded-md focus:ring-2 focus:ring-amber-500 text-slate-100" /></div>
        <div className="flex flex-wrap gap-3 items-center"><button onClick={handleBase64Encode} disabled={!base64Input.trim()} className="px-4 py-2 bg-amber-600 hover:bg-amber-700 text-white font-semibold rounded-md disabled:opacity-50">Encode</button><button onClick={handleBase64Decode} disabled={!base64Input.trim()} className="px-4 py-2 bg-amber-600 hover:bg-amber-700 text-white font-semibold rounded-md disabled:opacity-50">Decode</button><button onClick={handleBase64Swap} disabled={!base64Input.trim() && !base64Output.trim()} className="px-4 py-2 bg-slate-600 hover:bg-slate-500 text-white rounded-md disabled:opacity-50">Swap</button><button onClick={handleBase64Clear} className="px-4 py-2 bg-red-600 hover:bg-red-700 text-white rounded-md">Clear</button></div>
        <ClickToCopyOutput id="base64-output" value={base64Output} addNotification={addNotification} rows={4} />
      </UtilitySection>
      <UtilitySection title="URL Encoder/Decoder">
        <div><label htmlFor="url-input" className="block text-sm font-medium text-slate-300 mb-1">URL/Text</label><textarea id="url-input" rows={3} value={urlInput} onChange={(e) => setUrlInput(e.target.value)} placeholder="URL or text" className="w-full p-3 bg-slate-700 border border-slate-600 rounded-md focus:ring-2 focus:ring-amber-500 text-slate-100" /></div>
        <div className="flex flex-wrap gap-3 items-center"><button onClick={handleUrlEncode} disabled={!urlInput.trim()} className="px-4 py-2 bg-amber-600 hover:bg-amber-700 text-white font-semibold rounded-md disabled:opacity-50">Encode</button><button onClick={handleUrlDecode} disabled={!urlInput.trim()} className="px-4 py-2 bg-amber-600 hover:bg-amber-700 text-white font-semibold rounded-md disabled:opacity-50">Decode</button><button onClick={handleUrlClear} className="px-4 py-2 bg-red-600 hover:bg-red-700 text-white rounded-md">Clear</button></div>
        <ClickToCopyOutput id="url-output" value={urlOutput} addNotification={addNotification} rows={3} />
      </UtilitySection>
      <UtilitySection title="Timestamp Converter">
        <div><label htmlFor="timestamp-input" className="block text-sm font-medium text-slate-300 mb-1">Timestamp or Date String</label><input type="text" id="timestamp-input" value={timestampInput} onChange={(e) => setTimestampInput(e.target.value)} placeholder="Unix timestamp or date string" className="w-full p-3 bg-slate-700 border border-slate-600 rounded-md focus:ring-2 focus:ring-amber-500 text-slate-100" /></div>
        <div className="flex items-center space-x-3 my-2"><label className="flex items-center text-sm text-slate-300"><input type="checkbox" checked={isMillis} onChange={(e) => setIsMillis(e.target.checked)} className="mr-2 h-4 w-4 rounded border-slate-500 text-amber-600 focus:ring-amber-500 bg-slate-700"/>Milliseconds</label></div>
        <div className="flex flex-wrap gap-3 items-center"><button onClick={handleTimestampToReadable} className="px-4 py-2 bg-amber-600 hover:bg-amber-700 text-white font-semibold rounded-md">To Readable</button><button onClick={handleReadableToTimestamp} className="px-4 py-2 bg-amber-600 hover:bg-amber-700 text-white font-semibold rounded-md">To Timestamp</button><button onClick={handleTimestampNow} className="px-4 py-2 bg-sky-600 hover:bg-sky-700 text-white rounded-md">Now</button><button onClick={handleTimestampClear} className="px-4 py-2 bg-red-600 hover:bg-red-700 text-white rounded-md">Clear/Reset</button></div>
        <ClickToCopyOutput id="timestamp-output-readable" value={readableDateOutput} addNotification={addNotification} rows={1} label="Readable Date"/>
        <ClickToCopyOutput id="timestamp-output-unix" value={unixTimestampOutput} addNotification={addNotification} rows={1} label="Unix Timestamp"/>
      </UtilitySection>
      <UtilitySection title="JSON Formatter & Validator">
        <div><label htmlFor="json-input" className="block text-sm font-medium text-slate-300 mb-1">JSON Input</label><textarea id="json-input" rows={8} value={jsonInput} onChange={(e) => setJsonInput(e.target.value)} placeholder="Paste JSON" className="w-full p-3 bg-slate-700 border border-slate-600 rounded-md focus:ring-2 focus:ring-amber-500 text-slate-100 font-mono text-sm" /></div>
        <div className="flex flex-wrap gap-3 items-center"><button onClick={handleJsonFormat} disabled={!jsonInput.trim()} className="px-4 py-2 bg-amber-600 hover:bg-amber-700 text-white font-semibold rounded-md disabled:opacity-50">Format</button><div className="text-sm text-slate-300">Indent:<select value={String(jsonIndent)} onChange={(e) => setJsonIndent(e.target.value === 'tab' ? 'tab' : parseInt(e.target.value) as 2 | 4)} className="ml-2 p-1 bg-slate-700 border border-slate-600 rounded-md text-slate-100"><option value="2">2 spaces</option><option value="4">4 spaces</option><option value="tab">Tabs</option></select></div><button onClick={handleJsonClear} className="px-4 py-2 bg-red-600 hover:bg-red-700 text-white rounded-md">Clear</button></div>
        <ClickToCopyOutput id="json-output" value={jsonOutput} addNotification={addNotification} rows={8} label="Formatted JSON / Error" />
      </UtilitySection>
      <UtilitySection title="Text Inspector">
        <div><label htmlFor="text-inspector-input" className="block text-sm font-medium text-slate-300 mb-1">Text</label><textarea id="text-inspector-input" rows={6} value={textInspectorInput} onChange={(e) => setTextInspectorInput(e.target.value)} placeholder="Paste or type text" className="w-full p-3 bg-slate-700 border border-slate-600 rounded-md focus:ring-2 focus:ring-amber-500 text-slate-100" /></div>
        <div className="grid grid-cols-1 sm:grid-cols-3 gap-4 text-center"><div className="bg-slate-700 p-3 rounded-md"><p className="text-xs text-slate-400">Characters</p><p className="text-2xl font-semibold text-sky-400">{charCount}</p></div><div className="bg-slate-700 p-3 rounded-md"><p className="text-xs text-slate-400">Words</p><p className="text-2xl font-semibold text-sky-400">{wordCount}</p></div><div className="bg-slate-700 p-3 rounded-md"><p className="text-xs text-slate-400">Lines</p><p className="text-2xl font-semibold text-sky-400">{lineCount}</p></div></div>
        <button onClick={() => setTextInspectorInput('')} className="mt-4 px-4 py-2 bg-red-600 hover:bg-red-700 text-white rounded-md">Clear Text</button>
      </UtilitySection>
      <UtilitySection title="UUID v4 Generator">
        <button onClick={handleGenerateUuid} className="px-4 py-2 bg-sky-600 hover:bg-sky-700 text-white font-semibold rounded-md">Generate UUID</button>
        {generatedUuid && (<ClickToCopyOutput id="uuid-output" value={generatedUuid} addNotification={addNotification} rows={1} label="Generated UUID"/>)}
      </UtilitySection>
    </div>
  );
};

export default StdUtilsFeature;