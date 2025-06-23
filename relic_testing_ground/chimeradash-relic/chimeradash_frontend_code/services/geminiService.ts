
import { GoogleGenAI, GenerateContentResponse, Part, Tool, FunctionDeclaration, SafetySetting, HarmCategory as GoogleHarmCategory, HarmBlockThreshold as GoogleHarmBlockThreshold, GenerateContentConfig } from "@google/genai";
import { GeminiActionType, GeminiContent, GeminiFunctionCall, GeminiPart, DashboardSettings, SafetySettingItem, HarmCategory as AppHarmCategory, HarmBlockThreshold as AppHarmBlockThreshold } from '../types';

// Ensure API_KEY is available in the environment.
const API_KEY = typeof process !== 'undefined' && process.env ? process.env.API_KEY : undefined;

if (!API_KEY) {
  console.error("API_KEY for Gemini is not set in environment variables. AI features will be non-functional.");
}

const ai = new GoogleGenAI({ apiKey: API_KEY || "" });
const DEFAULT_TEXT_MODEL = 'gemini-2.5-flash-preview-04-17';
const DEFAULT_IMAGE_MODEL = 'imagen-3.0-generate-002'; 

const MAX_CSV_CONTENT_LENGTH = 5000; 
const MAX_HTML_CONTENT_LENGTH = 30000; 
const TEXT_SEPARATOR_FOR_COMPARISON = "---TEXT_SEPARATOR_FOR_AI_COMPARISON---";

const safeParseJson = <T,>(jsonString: string): T | null => {
  try {
    let str = jsonString.trim();
    const fenceRegex = /^```(\w*)?\s*\n?(.*?)\n?\s*```$/s;
    const match = str.match(fenceRegex);
    if (match && match[2]) {
      str = match[2].trim();
    }
    return JSON.parse(str) as T;
  } catch (error) {
    console.error("Failed to parse JSON response:", error, "Original string:", jsonString);
    return null;
  }
};

// Helper function to map app safety settings to Google GenAI safety settings
const mapAppSafetySettingsToGoogle = (appSettings?: SafetySettingItem[]): SafetySetting[] | undefined => {
  if (!appSettings || appSettings.length === 0) {
    return undefined;
  }
  // Get the string values of the actual GoogleHarmCategory enum
  const validGoogleCategoryValues = Object.values(GoogleHarmCategory) as string[];

  return appSettings
    .filter(setting => validGoogleCategoryValues.includes(setting.category as string)) // Ensure the category string value is one of Google's
    .map(setting => ({
      category: setting.category as unknown as GoogleHarmCategory, // Cast to unknown first for type compatibility
      threshold: setting.threshold as unknown as GoogleHarmBlockThreshold, 
    }));
};

// Default safety settings using Google GenAI enums if nothing provided from dashboardSettings
const fallbackSafetySettings: SafetySetting[] = [
    { category: GoogleHarmCategory.HARM_CATEGORY_HARASSMENT, threshold: GoogleHarmBlockThreshold.BLOCK_MEDIUM_AND_ABOVE },
    { category: GoogleHarmCategory.HARM_CATEGORY_HATE_SPEECH, threshold: GoogleHarmBlockThreshold.BLOCK_MEDIUM_AND_ABOVE },
    { category: GoogleHarmCategory.HARM_CATEGORY_SEXUALLY_EXPLICIT, threshold: GoogleHarmBlockThreshold.BLOCK_MEDIUM_AND_ABOVE },
    { category: GoogleHarmCategory.HARM_CATEGORY_DANGEROUS_CONTENT, threshold: GoogleHarmBlockThreshold.BLOCK_MEDIUM_AND_ABOVE },
];


export const analyzeWithGemini = async (
  content: string, 
  actionType: GeminiActionType,
  mimeType?: string, 
  customSystemInstruction?: string, // Per-call system instruction (appended to global)
  customUserPromptSegment?: string, 
  dashboardSettings?: Partial<DashboardSettings> 
): Promise<string | string[]> => {
  if (!API_KEY) {
    return Promise.reject("Gemini API Key not configured. Please set the API_KEY environment variable.");
  }

  const modelToUse = dashboardSettings?.defaultTextModel || DEFAULT_TEXT_MODEL;
  
  let finalSystemInstruction = customSystemInstruction;
  if (dashboardSettings?.globalSystemInstruction) {
    finalSystemInstruction = customSystemInstruction 
      ? `${dashboardSettings.globalSystemInstruction}\n\n${customSystemInstruction}`.trim()
      : dashboardSettings.globalSystemInstruction;
  }

  const safetySettingsToUse = mapAppSafetySettingsToGoogle(dashboardSettings?.aiSafetySettings) || fallbackSafetySettings;
  
  let thinkingConfigOption: GenerateContentConfig['thinkingConfig'] = undefined;
  if (modelToUse === 'gemini-2.5-flash-preview-04-17' && dashboardSettings?.aiThinkingEnabled !== undefined) {
    thinkingConfigOption = { thinkingBudget: dashboardSettings.aiThinkingEnabled ? undefined : 0 };
  }

  let corePrompt = "";
  const requestParts: Part[] = []; 
  let expectJsonArray = false;
  let expectJsonObject = false;
  let responseMimeType: "application/json" | "text/plain" = "text/plain";

  let processedContent = content;
  if (mimeType === 'text/html-scraped-url' && content.length > MAX_HTML_CONTENT_LENGTH) {
    processedContent = content.substring(0, MAX_HTML_CONTENT_LENGTH) + "\n... (HTML content truncated)";
  }


  switch (actionType) {
    case GeminiActionType.SUMMARIZE:
      corePrompt = customUserPromptSegment ? `${customUserPromptSegment}\n\nSummarize the following text concisely. Focus on the main points:\n\n"${processedContent}"` 
                                        : `Summarize the following text concisely. Focus on the main points:\n\n"${processedContent}"`;
      requestParts.push({ text: corePrompt });
      break;
    case GeminiActionType.DESCRIBE:
      if (!mimeType || !processedContent.startsWith('data:')) {
        return Promise.reject("MIME type and valid base64 data URL are required for image description.");
      }
      const base64ImageData = processedContent.split(',')[1];
      requestParts.push({
        inlineData: { mimeType: mimeType, data: base64ImageData },
      });
      let describeInstruction = "Describe this image in detail, focusing on key objects and their arrangement.";
      if (customUserPromptSegment) describeInstruction += ` ${customUserPromptSegment}`;
      requestParts.push({ text: describeInstruction });
      break;
    case GeminiActionType.TAG:
      corePrompt = `Generate 3-5 relevant keywords (tags) for the following content. ${customUserPromptSegment || ''} Content:\n\n"${processedContent}"\n\nReturn ONLY a valid JSON array of unique strings (e.g., ["tag1", "tag2", "tag3"]).`;
      requestParts.push({ text: corePrompt });
      expectJsonArray = true;
      responseMimeType = "application/json";
      break;
    case GeminiActionType.EXTRACT_KEYWORDS:
      corePrompt = `Extract the most relevant and distinct keywords (around 5-7) from the following text. ${customUserPromptSegment || ''} Return these keywords ONLY as a valid JSON array of unique strings. Text:\n\n"${processedContent}"`;
      requestParts.push({ text: corePrompt });
      expectJsonArray = true;
      responseMimeType = "application/json";
      break;
    case GeminiActionType.ANALYZE_SENTIMENT:
      corePrompt = `Analyze the sentiment of the following text. Classify it as 'Positive', 'Negative', or 'Neutral'. ${customUserPromptSegment || ''} Return only one of these words. Text:\n\n"${processedContent}"`;
      requestParts.push({ text: corePrompt });
      break;
    case GeminiActionType.EXTRACT_SCHEMA:
      corePrompt = `Analyze the following content and infer its data structure or schema. ${customUserPromptSegment || 'Describe the schema in a clear, structured format (e.g., JSON Schema for JSON, class/interface structure for code, or a general data model).'} Content:\n\n${processedContent}`;
      requestParts.push({ text: corePrompt });
      break;
    case GeminiActionType.GENERATE_TEMPLATE:
      corePrompt = `Generate a template file based on the following context/requirements: ${customUserPromptSegment || 'Ensure the template is well-structured and ready for use.'}\n\nContext:\n${processedContent}`;
      requestParts.push({ text: corePrompt });
      break;
    case GeminiActionType.EXTRACT_CODE_STRUCTURE:
      corePrompt = `Analyze the following code. Identify the main functions, classes, and methods. For each, provide a brief one-sentence description of its purpose. ${customUserPromptSegment || 'Present the output in a clear, human-readable list or structured text.'}\n\nCode:\n\`\`\`\n${processedContent}\n\`\`\``;
      requestParts.push({ text: corePrompt });
      responseMimeType = "text/plain";
      break;
    case GeminiActionType.IDENTIFY_API_ENDPOINTS:
      corePrompt = `Examine this code for web API endpoint definitions (e.g., Flask routes, Express.js app.get/post, Spring @RequestMapping). List any identified endpoints, their HTTP methods, and a brief description. ${customUserPromptSegment || 'If no endpoints are apparent, state "No API endpoints identified".'}\n\nCode:\n\`\`\`\n${processedContent}\n\`\`\``;
      requestParts.push({ text: corePrompt });
      responseMimeType = "text/plain";
      break;
    case GeminiActionType.LIST_DEPENDENCIES:
      corePrompt = `List the primary import statements or dependencies from external libraries or modules in the following code. Focus on the most significant ones that define the code's external requirements. ${customUserPromptSegment || 'Return a comma-separated list or a newline-separated list if multiple dependencies are found.'}\n\nCode:\n\`\`\`\n${processedContent}\n\`\`\``;
      requestParts.push({ text: corePrompt });
      responseMimeType = "text/plain";
      break;
    case GeminiActionType.TRANSCRIBE_AUDIO:
      if (!mimeType || !processedContent.startsWith('data:')) {
        return Promise.reject("MIME type and valid base64 data URL are required for audio transcription.");
      }
      const base64AudioData = processedContent.split(',')[1];
      requestParts.push({
        inlineData: { mimeType: mimeType, data: base64AudioData },
      });
      let transcribeInstruction = "Transcribe the following audio accurately and clearly. Include speaker labels if discernible (e.g., Speaker 1:, Speaker 2:).";
      if (customUserPromptSegment) transcribeInstruction += ` ${customUserPromptSegment}`;
      requestParts.push({ text: transcribeInstruction });
      responseMimeType = "text/plain";
      break;
    case GeminiActionType.SUMMARIZE_WEBPAGE: 
      corePrompt = `You are an expert web content analyst. Based *only* on the provided URL and your general knowledge, provide a concise summary of the likely main topic or purpose of the webpage. Do *not* attempt to access or crawl the URL. If the URL is too generic or you cannot reasonably infer its content, clearly state that you cannot provide a summary based on the URL alone. ${customUserPromptSegment || ''}\n\nWebpage URL: "${content}"`; 
      requestParts.push({ text: corePrompt });
      responseMimeType = "text/plain";
      break;
    case GeminiActionType.SCRAPE_WEB_SIMPLE: 
      corePrompt = `From the following HTML content, extract the main article or primary textual content. Provide a concise summary or the core text. ${customUserPromptSegment || ''}\n\nHTML Content (may be truncated):\n\`\`\`html\n${processedContent}\n\`\`\``;
      requestParts.push({ text: corePrompt });
      responseMimeType = "text/plain";
      break;
    case GeminiActionType.SCRAPE_WEB_FULL_TEXT: 
      corePrompt = `From the following HTML content, extract all meaningful textual content. Aim for comprehensiveness while omitting purely structural or irrelevant elements like navigation bars, footers, ads, if identifiable. ${customUserPromptSegment || ''}\n\nHTML Content (may be truncated):\n\`\`\`html\n${processedContent}\n\`\`\``;
      requestParts.push({ text: corePrompt });
      responseMimeType = "text/plain";
      break;
    case GeminiActionType.ANALYZE_WEB_DOCUMENTATION: 
      corePrompt = `Analyze the following HTML content from a single documentation page. Identify and list:
1.  Main sections or topics covered.
2.  Key API endpoints or functions described (if any), including their parameters and purpose.
3.  Any significant code snippets or examples.
${customUserPromptSegment || 'Present this information in a structured, human-readable format (e.g., using Markdown).'}
HTML Content (may be truncated):
\`\`\`html\n${processedContent}\n\`\`\``;
      requestParts.push({ text: corePrompt });
      responseMimeType = "text/plain";
      break;
    case GeminiActionType.GET_VIDEO_INFO: 
      corePrompt = `You are an expert media analyst. Based *only* on the provided video URL and your general knowledge (e.g., common patterns for YouTube, Vimeo, etc.), provide a brief potential description of the video's content, topic, or likely uploader. Do *not* attempt to access or play the video from the URL. If the URL provides no clues, clearly state that. ${customUserPromptSegment || ''}\n\nVideo URL: "${content}"`; 
      requestParts.push({ text: corePrompt });
      responseMimeType = "text/plain";
      break;
    case GeminiActionType.GUESS_RSS_FEED_TOPICS: 
      corePrompt = `You are a web content expert. Based *only* on the provided RSS feed URL and your general knowledge of websites and common RSS feed structures/topics, provide a brief description of the likely topics or type of content this RSS feed might provide. Do *not* attempt to access or parse the RSS feed from the URL. If the URL provides no clues, clearly state that. ${customUserPromptSegment || ''}\n\nRSS Feed URL: "${content}"`; 
      requestParts.push({ text: corePrompt });
      responseMimeType = "text/plain";
      break;
    case GeminiActionType.ANALYZE_CSV_DATA: 
        const csvDataForAnalysis = processedContent.length > MAX_CSV_CONTENT_LENGTH ? processedContent.substring(0, MAX_CSV_CONTENT_LENGTH) + "\n... (data truncated)" : processedContent;
        corePrompt = `Analyze the following CSV data. Provide the following information in a structured format (preferably JSON, but clear text is acceptable if JSON is complex for the result):
1.  columnNames: An array of strings representing the column headers.
2.  rowCount: The total number of data rows (excluding the header row).
3.  columnDetails: An array of objects, where each object has:
    - name: The column name (string).
    - likelyDataType: Suggested data type (e.g., String, Number, Boolean, Date, Mixed).
    - exampleValue: A single, representative example value from that column (string).
${customUserPromptSegment || 'If the CSV is empty or has no clear header, state that.'}
CSV Data:
\`\`\`csv
${csvDataForAnalysis}
\`\`\`
Return the analysis as a single JSON object with keys "columnNames", "rowCount", and "columnDetails". Example: {"columnNames": ["ID", "Name"], "rowCount": 5, "columnDetails": [{"name": "ID", "likelyDataType": "Number", "exampleValue": "101"}, {"name": "Name", "likelyDataType": "String", "exampleValue": "Alice"}]}`;
        requestParts.push({ text: corePrompt });
        expectJsonObject = true; // Expect a single JSON object
        responseMimeType = "application/json"; 
        break;
    case GeminiActionType.CONVERT_CSV_TO_JSON: 
        const csvDataForJsonConversion = processedContent.length > MAX_CSV_CONTENT_LENGTH ? processedContent.substring(0, MAX_CSV_CONTENT_LENGTH) + "\n... (data truncated for conversion preview)" : processedContent;
        corePrompt = `Convert the following CSV data into a valid JSON array of objects. Each object should represent a row, with keys derived from the CSV header.
${customUserPromptSegment || 'Ensure the output is ONLY the JSON array.'}
CSV Data:
\`\`\`csv
${csvDataForJsonConversion}
\`\`\`
Return ONLY the valid JSON array. Handle potential empty lines or malformed CSV rows gracefully if possible, or process up to the first error. If the CSV is very large, process only the first few hundred rows as an example.`;
        requestParts.push({ text: corePrompt });
        expectJsonArray = true; // Expect a JSON array
        responseMimeType = "application/json";
        break;
    case GeminiActionType.CONVERT_CSV_TO_MARKDOWN_TABLE: 
        const csvDataForMarkdown = processedContent.length > MAX_CSV_CONTENT_LENGTH ? processedContent.substring(0, MAX_CSV_CONTENT_LENGTH) + "\n... (data truncated for Markdown preview)" : processedContent;
        corePrompt = `Convert the following CSV data into a Markdown table.
${customUserPromptSegment || 'Ensure the output is ONLY the Markdown table. Format it clearly.'}
CSV Data:
\`\`\`csv
${csvDataForMarkdown}
\`\`\`
Return ONLY the Markdown table.`;
        requestParts.push({ text: corePrompt });
        responseMimeType = "text/plain";
        break;

    // Section 1: Advanced Text & Document Processing
    case GeminiActionType.COMPARATIVE_SUMMARIZE_TEXTS:
        const textsToCompare = processedContent.split(TEXT_SEPARATOR_FOR_COMPARISON);
        if (textsToCompare.length !== 2) return Promise.reject("Exactly two texts, separated by the delimiter, are required for comparison.");
        corePrompt = `You are a text comparison expert. I will provide two texts separated by "${TEXT_SEPARATOR_FOR_COMPARISON}".
First, provide a concise summary for Text 1.
Second, provide a concise summary for Text 2.
Third, provide a comparative summary highlighting the key differences and similarities between Text 1 and Text 2.
${customUserPromptSegment || 'Focus on clarity and distinct points in your comparison.'}

Text 1:
${textsToCompare[0].trim()}

${TEXT_SEPARATOR_FOR_COMPARISON}

Text 2:
${textsToCompare[1].trim()}

Structure your response clearly, perhaps using headings like "Summary of Text 1:", "Summary of Text 2:", and "Comparative Analysis:".`;
        requestParts.push({ text: corePrompt });
        responseMimeType = "text/plain";
        break;
    case GeminiActionType.DOCUMENT_QA:
        corePrompt = `You are a Document Q&A assistant. Use the following document text as context to answer the user's question. If the answer is not found in the document, state that clearly.
Document Context:
---
${processedContent}
---
User Question: ${customUserPromptSegment || "What is this document about?"}

Answer based *only* on the provided document context.`;
        requestParts.push({ text: corePrompt });
        responseMimeType = "text/plain";
        break;
    case GeminiActionType.ANALYZE_STYLE_TONE:
        corePrompt = `Analyze the writing style (e.g., formal, informal, academic, conversational, technical) and emotional tone (e.g., joyful, angry, sad, analytical, neutral, persuasive, urgent) of the following text.
${customUserPromptSegment || 'Provide your analysis as a JSON object with keys "style", "tone", and "confidence" (a float between 0.0 and 1.0 representing your confidence in the overall analysis).'}
Text:
---
${processedContent}
---
Return ONLY a valid JSON object.`;
        requestParts.push({ text: corePrompt });
        expectJsonObject = true;
        responseMimeType = "application/json";
        break;

    // Section 2: Creative Content Generation
    case GeminiActionType.GENERATE_STORY_IDEAS:
        corePrompt = `You are a creative story idea generator. Based on the following user input, generate 3-5 distinct story ideas or plot hooks.
User Input:
---
${processedContent} 
---
${customUserPromptSegment || 'Make each idea concise (1-2 sentences) and intriguing.'}
Present the ideas as a numbered list.`;
        requestParts.push({ text: corePrompt });
        responseMimeType = "text/plain";
        break;
    case GeminiActionType.GENERATE_MARKETING_COPY:
        corePrompt = `You are an AI marketing copywriter. Generate a short, punchy marketing blurb or social media post (approx. 2-3 sentences) for the following product/service.
Product/Service Description & Target Audience:
---
${processedContent}
---
${customUserPromptSegment || 'Focus on benefits and a clear call to action if appropriate. Make it engaging.'}`;
        requestParts.push({ text: corePrompt });
        responseMimeType = "text/plain";
        break;
    case GeminiActionType.GENERATE_POEM:
        corePrompt = `You are a poetry generation AI. Generate a short poem based on the following theme and style requirements.
Theme & Style:
---
${processedContent} 
---
${customUserPromptSegment || 'If no style is specified, a short free verse poem is fine. Aim for evocative language.'}`;
        requestParts.push({ text: corePrompt });
        responseMimeType = "text/plain";
        break;

    // Section 3: Data & CSV Enhanced Tools
    case GeminiActionType.PROFILE_CSV_DATA_INSIGHTS:
        const csvForInsights = processedContent.length > MAX_CSV_CONTENT_LENGTH ? processedContent.substring(0, MAX_CSV_CONTENT_LENGTH) + "\n... (data truncated)" : processedContent;
        corePrompt = `Analyze the following CSV data. Provide:
1.  A brief data profile (column names, number of rows, estimated data types for a few key columns).
2.  2-3 potential textual insights or observations that can be inferred from the data. These insights should be simple, direct, and clearly derivable from the provided CSV sample.
${customUserPromptSegment || 'If the CSV is empty or too small for meaningful insights, state that.'}
CSV Data (sample):
\`\`\`csv
${csvForInsights}
\`\`\`
Return the profile and insights in a clear, human-readable text format. For insights, aim for statements like "Column X shows a trend of..." or "Most entries in Column Y are of type Z".`;
        requestParts.push({ text: corePrompt });
        responseMimeType = "text/plain";
        break;
    case GeminiActionType.SUGGEST_CSV_CHART_TYPE:
        const csvForChartSuggestion = processedContent.length > MAX_CSV_CONTENT_LENGTH ? processedContent.substring(0, MAX_CSV_CONTENT_LENGTH) + "\n... (data truncated)" : processedContent;
        corePrompt = `Analyze the provided CSV data sample. Based on the apparent structure and potential data types of the columns, suggest 1-2 suitable chart types for visualizing this data (e.g., bar chart, line chart, scatter plot, pie chart). 
${customUserPromptSegment || 'For each suggestion, provide a brief (1-sentence) reasoning.'}
CSV Data Sample:
\`\`\`csv
${csvForChartSuggestion}
\`\`\`
Return your suggestions ONLY as a valid JSON array of objects, where each object has "chartType" (string) and "reasoning" (string) keys. Example: [{"chartType": "Bar Chart", "reasoning": "Good for comparing categorical data."}, {"chartType": "Line Chart", "reasoning": "Suitable for time series data if a date column exists."}]`;
        requestParts.push({ text: corePrompt });
        expectJsonArray = true;
        responseMimeType = "application/json";
        break;
    case GeminiActionType.CONVERT_JSON_TO_CSV:
        corePrompt = `Convert the following JSON data (must be an array of objects or a single object) into CSV format.
${customUserPromptSegment || 'Use the keys from the first object as headers. If it\'s a single object, treat it as a single-row CSV. Handle nested objects by flattening keys (e.g., parent.child) or by serializing them as JSON strings within the CSV cell if simple flattening is not feasible.'}
JSON Data:
---
${processedContent}
---
Return ONLY the CSV formatted string.`;
        requestParts.push({ text: corePrompt });
        responseMimeType = "text/plain";
        break;

    // Section 4: Web & URL Intelligence
    case GeminiActionType.DETECT_WEBPAGE_CHANGES_CONCEPTUAL:
        const changeDetectionParts = processedContent.split(TEXT_SEPARATOR_FOR_COMPARISON);
        const currentUrlForChange = changeDetectionParts[0];
        const previousSnapshot = changeDetectionParts.length > 1 ? changeDetectionParts[1] : null;

        corePrompt = `You are a conceptual web content analyst.
Webpage URL: "${currentUrlForChange}"
${previousSnapshot ? `Previous Content Snapshot (textual summary or key elements):\n---\n${previousSnapshot.trim()}\n---` : "No previous content snapshot provided."}

${customUserPromptSegment || `Based on the URL and the provided previous snapshot (if any), describe:
1. What the webpage is likely about.
2. If a previous snapshot is provided, what kind of changes *might* be expected if the page content were to be updated (e.g., updated news, new product listings, revised information).
3. If no snapshot, what elements are typically dynamic on such a page.
Do *not* attempt to access or crawl the URL. Your analysis is purely conceptual based on the given information.`}`;
        requestParts.push({ text: corePrompt });
        responseMimeType = "text/plain";
        break;
    case GeminiActionType.CATEGORIZE_URL_KEYWORDS:
        corePrompt = `Analyze the following URL. Based on its structure, domain, path, and your general knowledge of websites, categorize the website (e.g., e-commerce, news, blog, corporate, educational, forum) and guess 3-5 primary keywords or topics associated with it.
${customUserPromptSegment || 'Do *not* attempt to access or crawl the URL.'}
URL: "${processedContent}"
Return your analysis as a JSON object with keys "category" (string) and "guessedKeywords" (array of strings). Example: {"category": "Technology Blog", "guessedKeywords": ["AI", "software development", "gadgets"]}`;
        requestParts.push({ text: corePrompt });
        expectJsonObject = true;
        responseMimeType = "application/json";
        break;
    case GeminiActionType.CONVERT_HTML_TO_MARKDOWN:
        corePrompt = `Convert the following HTML content into well-formatted Markdown.
${customUserPromptSegment || 'Preserve headings, lists, links, code blocks, and emphasis where possible. Aim for readability.'}
HTML Content:
\`\`\`html
${processedContent}
\`\`\`
Return ONLY the Markdown content.`;
        requestParts.push({ text: corePrompt });
        responseMimeType = "text/plain";
        break;

    // Section 5: Advanced Code & Development Aids
    case GeminiActionType.SUGGEST_CODE_REFACTORING:
        corePrompt = `You are an expert code refactoring assistant. Analyze the following code snippet and the user's desired improvement. Suggest potential refactoring approaches or specific changes.
Code Snippet:
\`\`\`
${processedContent}
\`\`\`
User's Desired Improvement: ${customUserPromptSegment || "Improve general code quality and readability."}

Provide clear, actionable suggestions. Explain the reasoning behind your suggestions.`;
        requestParts.push({ text: corePrompt });
        responseMimeType = "text/plain";
        break;
    case GeminiActionType.GENERATE_UNIT_TEST_CASES:
        corePrompt = `You are a unit test generation assistant. Analyze the following code snippet (likely a function or method). Generate 3-5 basic unit test case ideas or stubs for it.
Code Snippet:
\`\`\`
${processedContent}
\`\`\`
${customUserPromptSegment || 'Consider edge cases, typical inputs, and expected outputs. Present the test cases in a human-readable list or as code stubs in a common testing framework style if appropriate (e.g., Jest, PyTest-like comments).'}
`;
        requestParts.push({ text: corePrompt });
        responseMimeType = "text/plain";
        break;
    case GeminiActionType.GENERATE_API_DOCUMENTATION:
        corePrompt = `You are an API documentation writer. Analyze the following code snippet (e.g., a function, class with methods, or API route definition). Generate a basic Markdown documentation outline for it.
Code Snippet:
\`\`\`
${processedContent}
\`\`\`
${customUserPromptSegment || 'Include sections for Purpose, Parameters (if any, with type and description guesses), Return Value (if any, with type guess), and a brief Example usage if inferable. If it seems to be an API endpoint, include HTTP method and path if possible.'}
Present the documentation in Markdown format.`;
        requestParts.push({ text: corePrompt });
        responseMimeType = "text/plain";
        break;

    default:
      // This exhaustive check ensures all GeminiActionTypes are handled.
      // If a new action type is added to the enum but not here, TypeScript will error.
      const _: never = actionType; 
      return Promise.reject(`Invalid Gemini action type: ${_}`);
  }

  try {
    const config: GenerateContentConfig = {
        ...(finalSystemInstruction && { systemInstruction: finalSystemInstruction }),
        responseMimeType: responseMimeType,
        ...(safetySettingsToUse && { safetySettings: safetySettingsToUse }),
        ...(thinkingConfigOption && { thinkingConfig: thinkingConfigOption }), // Ensure thinkingConfig is correctly structured
    };

    const response: GenerateContentResponse = await ai.models.generateContent({
      model: modelToUse,
      contents: [{ parts: requestParts, role: 'user' }], 
      config: config
    });
    
    const textResponse = response.text;
    
    if (expectJsonArray || expectJsonObject || (responseMimeType === "application/json")) {
      const parsedJson = safeParseJson<any>(textResponse); 

      if (expectJsonObject && parsedJson && typeof parsedJson === 'object' && !Array.isArray(parsedJson)) {
        return JSON.stringify(parsedJson, null, 2); 
      }
      if (expectJsonArray && parsedJson && Array.isArray(parsedJson)) { 
        return parsedJson.map(item => typeof item === 'object' ? JSON.stringify(item, null, 2) : String(item));
      }
      // Fallback for general application/json responses if not specifically object or array
      if (responseMimeType === "application/json" && parsedJson) {
         if (typeof parsedJson === 'object' && !Array.isArray(parsedJson) && 
            (actionType === GeminiActionType.ANALYZE_CSV_DATA || actionType === GeminiActionType.ANALYZE_STYLE_TONE || actionType === GeminiActionType.CATEGORIZE_URL_KEYWORDS)) {
            return JSON.stringify(parsedJson, null, 2);
         }
         if (Array.isArray(parsedJson) && (actionType === GeminiActionType.TAG || actionType === GeminiActionType.EXTRACT_KEYWORDS || actionType === GeminiActionType.CONVERT_CSV_TO_JSON || actionType === GeminiActionType.SUGGEST_CSV_CHART_TYPE)) {
             return parsedJson.map(item => typeof item === 'object' ? JSON.stringify(item, null, 2) : String(item));
         }
      }

      // If JSON parsing failed or structure mismatch for expected JSON
      console.warn(`Gemini ${actionType} response was not the expected JSON structure or failed to parse. Original text:`, textResponse.substring(0,200));
      if (actionType === GeminiActionType.CONVERT_CSV_TO_JSON || actionType === GeminiActionType.ANALYZE_CSV_DATA || 
          actionType === GeminiActionType.SUGGEST_CSV_CHART_TYPE || actionType === GeminiActionType.ANALYZE_STYLE_TONE ||
          actionType === GeminiActionType.CATEGORIZE_URL_KEYWORDS) { 
          return Promise.reject(`Failed to parse JSON for ${actionType}. Gemini output: ${textResponse.substring(0,100)}...`);
      }
      // For TAG or EXTRACT_KEYWORDS, try to make an array from comma-separated string if JSON fails
      if ((actionType === GeminiActionType.TAG || actionType === GeminiActionType.EXTRACT_KEYWORDS) && textResponse.includes(',')) return textResponse.split(',').map(s => s.trim()).filter(s => s);
      return [textResponse.substring(0, 200)]; // Fallback to returning the text as a single-element array
    }
    
    if (actionType === GeminiActionType.ANALYZE_SENTIMENT) {
        const sentiment = textResponse.trim().toLowerCase();
        if (sentiment === 'positive' || sentiment === 'negative' || sentiment === 'neutral') {
            return sentiment.charAt(0).toUpperCase() + sentiment.slice(1);
        }
        console.warn("Unexpected sentiment response:", textResponse, "Defaulting to Neutral.");
        return "Neutral"; 
    }
    return textResponse;
  } catch (error) {
    console.error(`Error during Gemini API call for ${actionType}:`, error);
    if (error instanceof Error) {
        return Promise.reject(`Gemini API Error: ${error.message}`);
    }
    return Promise.reject("An unknown error occurred with Gemini API.");
  }
};


export const chatWithAgent = async (
  history: GeminiContent[], 
  callConfig?: { 
    tools?: Tool[];
    customSystemInstruction?: string; // Per-call system instruction
  },
  dashboardSettings?: Partial<DashboardSettings> 
): Promise<GeminiPart[]> => { 
  if (!API_KEY) {
    throw new Error("Gemini API Key not configured.");
  }

  const modelToUse = dashboardSettings?.defaultTextModel || DEFAULT_TEXT_MODEL;

  let finalSystemInstruction = callConfig?.customSystemInstruction;
  if (dashboardSettings?.globalSystemInstruction) {
    finalSystemInstruction = callConfig?.customSystemInstruction
      ? `${dashboardSettings.globalSystemInstruction}\n\n${callConfig.customSystemInstruction}`.trim()
      : dashboardSettings.globalSystemInstruction;
  }

  const safetySettingsToUse = mapAppSafetySettingsToGoogle(dashboardSettings?.aiSafetySettings) || fallbackSafetySettings;
  
  let thinkingConfigOption: GenerateContentConfig['thinkingConfig'] = undefined;
  if (modelToUse === 'gemini-2.5-flash-preview-04-17' && dashboardSettings?.aiThinkingEnabled !== undefined) {
    thinkingConfigOption = { thinkingBudget: dashboardSettings.aiThinkingEnabled ? undefined : 0 };
  }
  
  try {
    const config: GenerateContentConfig = {
        ...(finalSystemInstruction && { systemInstruction: finalSystemInstruction }),
        ...(callConfig?.tools && { tools: callConfig.tools }),
        ...(safetySettingsToUse && { safetySettings: safetySettingsToUse }),
        ...(thinkingConfigOption && { thinkingConfig: thinkingConfigOption }), // Ensure thinkingConfig is correctly structured
    };

    const response: GenerateContentResponse = await ai.models.generateContent({
      model: modelToUse,
      contents: history,
      config: config
    });
    
    if (!response.candidates || response.candidates.length === 0 || !response.candidates[0].content || !response.candidates[0].content.parts) {
        const safetyRatings = response.candidates?.[0]?.safetyRatings;
        if (safetyRatings?.some(rating => rating.blocked)) {
            console.warn("Gemini response blocked due to safety ratings:", safetyRatings);
            return [{text: "My response was blocked due to safety reasons. Please rephrase your request or try something different."}];
        }
        console.warn("Gemini response has no content parts:", response);
        return [{text: "I'm sorry, I couldn't generate a response for that."}];
    }

    const rawParts = response.candidates[0].content.parts; 
    const resultParts: GeminiPart[] = rawParts.map(part => {
      const geminiPart: GeminiPart = {}; 
      if (part.text !== undefined) {
        geminiPart.text = part.text;
      }
      if (part.inlineData) {
        geminiPart.inlineData = {
          mimeType: part.inlineData.mimeType, 
          data: part.inlineData.data,
        };
      }
      if (part.functionCall) {
        geminiPart.functionCall = {
          name: part.functionCall.name,
          args: part.functionCall.args as Record<string, any>, 
        };
      }
      if (part.functionResponse) {
        geminiPart.functionResponse = {
          name: part.functionResponse.name,
          response: part.functionResponse.response as Record<string, any>, 
        };
      }
      return geminiPart;
    });
    return resultParts;

  } catch (error) {
    console.error("Error during Gemini chatWithAgent API call:", error);
    if (error instanceof Error) {
      return [{ text: `Gemini API Error: ${error.message}` }];
    }
    return [{ text: "An unknown error occurred with the Gemini API." }];
  }
};
