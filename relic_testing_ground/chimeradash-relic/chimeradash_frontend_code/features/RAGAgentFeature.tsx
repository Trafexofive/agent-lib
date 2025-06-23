



import React, { useState, useCallback, useEffect, useRef, FC, ChangeEvent, MouseEvent as ReactMouseEvent, KeyboardEvent as ReactKeyboardEvent } from 'react';
import {
    AggregatedItem,
    ItemType,
    ViewType,
    GeminiActionType,
    NotificationItem,
    ProductivitySubViewType,
    ChatMessage,
    GeminiContent,
    GeminiPart,
    GeminiFunctionCall,
    DashboardSettings
} from '../types';
import { analyzeWithGemini, chatWithAgent } from '../services/geminiService';
import Spinner from '../components/Spinner';
import { Tool, FunctionDeclaration, HarmCategory, HarmBlockThreshold, Type as GoogleGenAiType, SafetySetting } from "@google/genai";

// --- Internal Icons ---
const FolderIconInternal = (props: React.SVGProps<SVGSVGElement>) => <svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" strokeWidth={1.5} stroke="currentColor" {...props}><path strokeLinecap="round" strokeLinejoin="round" d="M2.25 12.75V12A2.25 2.25 0 014.5 9.75h15A2.25 2.25 0 0121.75 12v.75m-8.69-6.44l-2.12-2.12a1.5 1.5 0 00-1.061-.44H4.5A2.25 2.25 0 002.25 6v12a2.25 2.25 0 002.25 2.25h15A2.25 2.25 0 0021.75 18V9a2.25 2.25 0 00-2.25-2.25h-5.379a1.5 1.5 0 01-1.06-.44z" /></svg>;
const FileIconInternal = (props: React.SVGProps<SVGSVGElement>) => <svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" strokeWidth={1.5} stroke="currentColor" {...props}><path strokeLinecap="round" strokeLinejoin="round" d="M19.5 14.25v-2.625a3.375 3.375 0 00-3.375-3.375h-1.5A1.125 1.125 0 0113.5 7.125v-1.5a3.375 3.375 0 00-3.375-3.375H8.25m2.25 0H5.625c-.621 0-1.125.504-1.125 1.125v17.25c0 .621.504 1.125 1.125 1.125h12.75c.621 0 1.125-.504 1.125-1.125V11.25a9 9 0 00-9-9z" /></svg>;
const PythonIconInternal = (props: React.SVGProps<SVGSVGElement>) => <svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" strokeWidth={1.5} stroke="currentColor" {...props}><path strokeLinecap="round" strokeLinejoin="round" d="M14.25 9.75L16.5 12l-2.25 2.25m-4.5 0L7.5 12l2.25-2.25M6 20.25h12A2.25 2.25 0 0020.25 18V5.75A2.25 2.25 0 0018 3.5H6A2.25 2.25 0 003.75 5.75v12.5A2.25 2.25 0 006 20.25z" /></svg>; 
const JsonIconInternal = (props: React.SVGProps<SVGSVGElement>) => <svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" strokeWidth={1.5} stroke="currentColor" {...props}><path strokeLinecap="round" strokeLinejoin="round" d="M10.5 1.5H8.25A2.25 2.25 0 006 3.75v16.5a2.25 2.25 0 002.25 2.25h7.5A2.25 2.25 0 0018 20.25V3.75A2.25 2.25 0 0015.75 1.5m-7.5 0h3.75m-3.75 0V3m0 0V1.5m0 1.5H1.5m5.25 0h3.75M1.5 3V1.5m0 1.5c0 .54.13.97.328 1.375M1.5 3c-.198.405-.328.835-.328 1.375M1.5 3V4.875m0 0V19.5m0-14.625c0 .54.13.97.328 1.375M1.5 4.875c-.198.405-.328.835-.328 1.375M1.5 4.875V6.75m21-3.375V1.5M22.5 3c0-.54-.13-.97-.328-1.375M22.5 3c.198.405.328.835.328 1.375M22.5 3V4.875m0 0V19.5m0-14.625c0 .54.13.97.328 1.375M22.5 4.875c.198.405-.328.835-.328 1.375M22.5 4.875V6.75m-15-3.375V1.5m0 1.5c-.54 0-.97.13-1.375.328M7.5 3c.405-.198.835-.328 1.375-.328M7.5 3V4.875m0 0c0 .54-.13.97-.328 1.375M7.5 4.875c.405-.198.835-.328 1.375-.328M7.5 4.875V6.75m7.5-3.375V1.5m0 1.5c.54 0 .97.13 1.375.328M15 3c-.405-.198-.835-.328-1.375-.328M15 3V4.875m0 0c0 .54.13.97.328 1.375M15 4.875c-.405-.198-.835-.328-1.375-.328M15 4.875V6.75" /></svg>;
const CsvIconInternal = (props: React.SVGProps<SVGSVGElement>) => <svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" strokeWidth={1.5} stroke="currentColor" {...props}><path strokeLinecap="round" strokeLinejoin="round" d="M3.75 6.75h16.5M3.75 12h16.5m-16.5 5.25H12m6-5.25v5.25m0-5.25V6.75m0 5.25H3.75m16.5 0H3.75m0 5.25h16.5m0 0V6.75m0 11.25V6.75M3.75 6.75v11.25M3.75 6.75h16.5" /></svg>;
const MarkdownIconInternal = (props: React.SVGProps<SVGSVGElement>) => <svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" strokeWidth={1.5} stroke="currentColor" {...props}><path strokeLinecap="round" strokeLinejoin="round" d="M3.75 4.5h16.5M3.75 9.75h16.5M3.75 15h16.5M5.25 4.5v15M18.75 4.5v15M9 7.5h6M9 12.75h6" /></svg>;
const YamlIconInternal = (props: React.SVGProps<SVGSVGElement>) => <svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" strokeWidth={1.5} stroke="currentColor" {...props}><path strokeLinecap="round" strokeLinejoin="round" d="M4.5 6.75h15M4.5 12h15M4.5 17.25H12" /></svg>;
const CodeFileIconInternal = (props: React.SVGProps<SVGSVGElement>) => <svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" strokeWidth={1.5} stroke="currentColor" {...props}><path strokeLinecap="round" strokeLinejoin="round" d="M17.25 6.75L22.5 12l-5.25 5.25m-10.5 0L1.5 12l5.25-5.25m7.5-3l-4.5 16.5" /></svg>;
const TextFileIconInternal = (props: React.SVGProps<SVGSVGElement>) => <svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" strokeWidth={1.5} stroke="currentColor" {...props}><path strokeLinecap="round" strokeLinejoin="round" d="M3.75 6.75h16.5M3.75 12h16.5M3.75 17.25h16.5" /></svg>;

const PlusIconInternal = (props: React.SVGProps<SVGSVGElement>) => <svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" strokeWidth={1.5} stroke="currentColor" {...props}><path strokeLinecap="round" strokeLinejoin="round" d="M12 4.5v15m7.5-7.5h-15" /></svg>;
const TrashIconInternal = (props: React.SVGProps<SVGSVGElement>) => <svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" strokeWidth={1.5} stroke="currentColor" {...props}><path strokeLinecap="round" strokeLinejoin="round" d="M14.74 9l-.346 9m-4.788 0L9.26 9m9.968-3.21c.342.052.682.107 1.022.166m-1.022-.165L18.16 19.673a2.25 2.25 0 01-2.244 2.077H8.084a2.25 2.25 0 01-2.244-2.077L4.772 5.79m14.456 0a48.108 48.108 0 00-3.478-.397m-12.56 0c1.153 0 2.24.032 3.223.094M21 12a9 9 0 11-18 0 9 9 0 0118 0z" /></svg>;
const DownloadIconInternal = (props: React.SVGProps<SVGSVGElement>) => <svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" strokeWidth={1.5} stroke="currentColor" {...props}><path strokeLinecap="round" strokeLinejoin="round" d="M3 16.5v2.25A2.25 2.25 0 005.25 21h13.5A2.25 2.25 0 0021 18.75V16.5M16.5 12L12 16.5m0 0L7.5 12m4.5 0V3" /></svg>;
const SparklesIconInternal = (props: React.SVGProps<SVGSVGElement>) => <svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" strokeWidth={1.5} stroke="currentColor" {...props}><path strokeLinecap="round" strokeLinejoin="round" d="M9.813 15.904L9 18.75l-.813-2.846a4.5 4.5 0 00-3.09-3.09L1.25 12l2.846-.813a4.5 4.5 0 003.09-3.09L9 5.25l.813 2.846a4.5 4.5 0 003.09 3.09L15.75 12l-2.846.813a4.5 4.5 0 00-3.09 3.09zM18.25 12l2.846-.813a4.5 4.5 0 003.09-3.09L24 5.25l-.813 2.846a4.5 4.5 0 00-3.09 3.09L18.25 12zm0 0l-2.846.813a4.5 4.5 0 00-3.09 3.09L12 18.75l.813-2.846a4.5 4.5 0 003.09-3.09L18.25 12z" /></svg>;
const PencilIconInternal = (props: React.SVGProps<SVGSVGElement>) => <svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" strokeWidth={1.5} stroke="currentColor" {...props}><path strokeLinecap="round" strokeLinejoin="round" d="M16.862 4.487l1.687-1.688a1.875 1.875 0 112.652 2.652L10.582 16.07a4.5 4.5 0 01-1.897 1.13L6 18l.8-2.685a4.5 4.5 0 011.13-1.897l8.932-8.931zm0 0L19.5 7.125M18 14v4.75A2.25 2.25 0 0115.75 21H5.25A2.25 2.25 0 013 18.75V8.25A2.25 2.25 0 015.25 6H10" /></svg>;
const EyeIconInternal = (props: React.SVGProps<SVGSVGElement>) => <svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" strokeWidth={1.5} stroke="currentColor" {...props}><path strokeLinecap="round" strokeLinejoin="round" d="M2.036 12.322a1.012 1.012 0 010-.639C3.423 7.51 7.36 4.5 12 4.5c4.638 0 8.573 3.007 9.963 7.178.07.207.07.431 0 .639C20.577 16.49 16.64 19.5 12 19.5c-4.638 0-8.573-3.007-9.963-7.178z" /><path strokeLinecap="round" strokeLinejoin="round" d="M15 12a3 3 0 11-6 0 3 3 0 016 0z" /></svg>;
const Squares2X2IconInternal = (props: React.SVGProps<SVGSVGElement>) => <svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" strokeWidth={1.5} stroke="currentColor" {...props}><path strokeLinecap="round" strokeLinejoin="round" d="M3.75 6A2.25 2.25 0 016 3.75h2.25A2.25 2.25 0 0110.5 6v2.25a2.25 2.25 0 01-2.25 2.25H6a2.25 2.25 0 01-2.25-2.25V6zM3.75 15.75A2.25 2.25 0 016 13.5h2.25a2.25 2.25 0 012.25 2.25V18a2.25 2.25 0 01-2.25 2.25H6A2.25 2.25 0 013.75 18v-2.25zM13.5 6a2.25 2.25 0 012.25-2.25H18A2.25 2.25 0 0120.25 6v2.25A2.25 2.25 0 0118 10.5h-2.25A2.25 2.25 0 0113.5 8.25V6zM13.5 15.75a2.25 2.25 0 012.25-2.25H18a2.25 2.25 0 012.25 2.25V18A2.25 2.25 0 0118 20.25h-2.25A2.25 2.25 0 0113.5 18v-2.25z" /></svg>;
const ChatBubbleLeftRightIconInternal = (props: React.SVGProps<SVGSVGElement>) => <svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" strokeWidth={1.5} stroke="currentColor" {...props}><path strokeLinecap="round" strokeLinejoin="round" d="M20.25 8.511c.884.284 1.5 1.128 1.5 2.097v4.286c0 1.136-.847 2.1-1.98 2.193-.34.027-.68.052-1.02.072v3.091l-3.68-3.091a4.523 4.523 0 00-1.028-.296H9M3.75 9.75h1.5a.75.75 0 01.75.75v4.5a.75.75 0 01-.75.75h-1.5a.75.75 0 01-.75-.75V10.5a.75.75 0 01.75-.75zM21 9.75A2.25 2.25 0 0018.75 7.5H9A2.25 2.25 0 006.75 9.75v4.5A2.25 2.25 0 009 16.5h2.652a4.516 4.516 0 011.924.227l3.734 3.114A.75.75 0 0018 22.07V16.5A2.25 2.25 0 0015.75 14.25H9.75" /></svg>;
const PaperAirplaneIconInternal = (props: React.SVGProps<SVGSVGElement>) => <svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" strokeWidth={1.5} stroke="currentColor" {...props}><path strokeLinecap="round" strokeLinejoin="round" d="M6 12L3.269 3.126A59.768 59.768 0 0121.485 12 59.77 59.77 0 013.27 20.876L5.999 12zm0 0h7.5" /></svg>;
const FilmIconInternal = (props: React.SVGProps<SVGSVGElement>) => <svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" strokeWidth={1.5} stroke="currentColor" {...props}><path strokeLinecap="round" strokeLinejoin="round" d="M6 20.25h12m-7.5-3.75v3.75m-3.75-3.75v3.75m0-3.75H3.75m9.75 0v3.75M3 13.5h18M3 7.5h18m-1.5-4.5H4.5A2.25 2.25 0 002.25 5.25v13.5A2.25 2.25 0 004.5 21h15a2.25 2.25 0 002.25-2.25V5.25A2.25 2.25 0 0019.5 3zm-6 .75h.008v.008H13.5v-.008zm-3.75 0h.008v.008H9.75v-.008zm-3.75 0h.008v.008H6v-.008z" /></svg>;
const MusicalNoteIconInternal = (props: React.SVGProps<SVGSVGElement>) => <svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" strokeWidth={1.5} stroke="currentColor" {...props}><path strokeLinecap="round" strokeLinejoin="round" d="M9 9l10.5-3m0 6.553v3.75a2.25 2.25 0 01-1.002 1.732l-6.75 4.126a2.25 2.25 0 01-2.496 0L3.75 15.25a2.25 2.25 0 01-1.002-1.732V9.75a2.25 2.25 0 011.002-1.732l6.75-4.126a2.25 2.25 0 012.496 0L20.25 7.5M9 9V4.5M9 9l6.75 4.123" /></svg>;
const RssIconInternal = (props: React.SVGProps<SVGSVGElement>) => <svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" strokeWidth={1.5} stroke="currentColor" {...props}><path strokeLinecap="round" strokeLinejoin="round" d="M12.75 19.5v-.75a7.5 7.5 0 00-7.5-7.5H4.5m0-6.75h.75c7.887 0 14.25 6.363 14.25 14.25v.75M6 18.75a3 3 0 100-6 3 3 0 000 6z" /></svg>;
const GlobeAltIconInternal = (props: React.SVGProps<SVGSVGElement>) => <svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" strokeWidth={1.5} stroke="currentColor" {...props}><path strokeLinecap="round" strokeLinejoin="round" d="M12 21a9.004 9.004 0 008.716-6.747M12 21a9.004 9.004 0 01-8.716-6.747M12 21c.434 0 .86-.034 1.277-.097M12 3c-4.215 0-7.833 2.044-10.001 5.048A.751.751 0 011.25 8.25m19.5 0A.751.751 0 0122.75 8.25c-2.168-3.004-5.786-5.048-10.001-5.048M2.25 8.25h19.5M2.25 8.25c0 1.13.21 2.205.601 3.205L3.247 15H12m-9.753-6.75A8.966 8.966 0 001.25 12c0 4.133 2.802 7.576 6.5 8.618m13.5-8.618a8.966 8.966 0 01.753 3.75c0 4.133-2.802 7.576-6.5 8.618" /></svg>;
const DocumentDuplicateIconInternal = (props: React.SVGProps<SVGSVGElement>) => <svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" strokeWidth={1.5} stroke="currentColor" {...props}><path strokeLinecap="round" strokeLinejoin="round" d="M15.75 17.25v3.375c0 .621-.504 1.125-1.125 1.125h-9.75a1.125 1.125 0 01-1.125-1.125V7.875c0-.621.504 1.125 1.125 1.125H6.75a9.06 9.06 0 011.5.124m7.5 10.376h3.375c.621 0 1.125-.504 1.125-1.125V11.25c0-4.46-3.243-8.161-7.5-8.876a9.06 9.06 0 00-1.5-.124H9.375c-.621 0-1.125.504-1.125 1.125v3.5m7.5 10.375H9.375m7.5 0c.621 0 1.125.504 1.125 1.125v3.375c0 .621-.504 1.125-1.125 1.125h-9.75a1.125 1.125 0 01-1.125-1.125V7.875c0-.621.504-1.125 1.125-1.125H6.75" /></svg>;
const ChevronLeftIconInternal = (props: React.SVGProps<SVGSVGElement>) => <svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" strokeWidth={1.5} stroke="currentColor" {...props}><path strokeLinecap="round" strokeLinejoin="round" d="M15.75 19.5L8.25 12l7.5-7.5" /></svg>;
const XMarkIconInternal = (props: React.SVGProps<SVGSVGElement>) => <svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" strokeWidth={1.5} stroke="currentColor" {...props}><path strokeLinecap="round" strokeLinejoin="round" d="M6 18L18 6M6 6l12 12" /></svg>;
const ListBulletIconInternal = (props: React.SVGProps<SVGSVGElement>) => (
  <svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" strokeWidth={1.5} stroke="currentColor" {...props}>
    <path strokeLinecap="round" strokeLinejoin="round" d="M8.25 6.75h7.5M8.25 12h7.5m-7.5 5.25h7.5M3.75 6.75h.007v.008H3.75V6.75zm.375 0a.375.375 0 11-.75 0 .375.375 0 01.75 0zM3.75 12h.007v.008H3.75V12zm.375 0a.375.375 0 11-.75 0 .375.375 0 01.75 0zM3.75 17.25h.007v.008H3.75v-.008zm.375 0a.375.375 0 11-.75 0 .375.375 0 01.75 0z" />
  </svg>
);
const InboxStackIconInternal = (props: React.SVGProps<SVGSVGElement>) => <svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" strokeWidth={1.5} stroke="currentColor" {...props}><path strokeLinecap="round" strokeLinejoin="round" d="M3.75 12h16.5m-16.5 3.75h16.5M3.75 19.5h16.5M5.625 4.5h12.75a1.875 1.875 0 010 3.75H5.625a1.875 1.875 0 010-3.75z" /></svg>;
const ArrowUturnLeftIconInternal = (props: React.SVGProps<SVGSVGElement>) => <svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" strokeWidth={1.5} stroke="currentColor" {...props}><path strokeLinecap="round" strokeLinejoin="round" d="M9 15L3 9m0 0l6-6M3 9h12a6 6 0 010 12h-3" /></svg>;
const LinkIconInternal = (props: React.SVGProps<SVGSVGElement>) => (
  <svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" strokeWidth={1.5} stroke="currentColor" {...props}>
    <path strokeLinecap="round" strokeLinejoin="round" d="M13.19 8.688a4.5 4.5 0 011.242 7.244l-4.5 4.5a4.5 4.5 0 01-6.364-6.364l1.757-1.757m13.35-.622l1.757-1.757a4.5 4.5 0 00-6.364-6.364l-4.5 4.5a4.5 4.5 0 001.242 7.244" />
  </svg>
);


interface RAGAgentFeatureProps {
  addNotification: (
    type: NotificationItem['type'],
    message: string,
    sourceDetails?: {
      sourceView?: ViewType;
      sourceSubView?: ProductivitySubViewType;
      sourceItemId?: string;
    }
  ) => void;
  dashboardSettings: DashboardSettings;
}

const MOCK_DATA_KEY = 'rag-agent-mock-data-loaded';
const RAG_CHAT_HISTORY_KEY = 'rag-agent-chat-history';
const RAG_ITEMS_KEY = 'rag-agent-items';
const ANALYSIS_QUEUE_DELAY = 1500;

const MEDIA_CONTENT_LOCALSTORAGE_PLACEHOLDER_THRESHOLD_BYTES = 50 * 1024;
const GENERAL_CONTENT_TRUNCATION_LENGTH_FOR_LOCALSTORAGE = 2048;
const ANALYSIS_CONTENT_TRUNCATION_LENGTH_FOR_LOCALSTORAGE = 1024;

const initialMockItems: AggregatedItem[] = [
  { id: 'mock-text-1', type: ItemType.TEXT, name: 'Meeting Notes Summary', content: 'The meeting covered Q3 goals, budget allocations, and new project timelines. Key decisions included approving the Titan project and deferring the Hydra initiative. Action items were assigned to Alice, Bob, and Carol.', createdAt: new Date(2023, 9, 1, 10, 0, 0), fileSize: 230, geminiAnalysis: { [GeminiActionType.SUMMARIZE]: 'Q3 goals, budget, and project timelines discussed. Titan project approved, Hydra deferred. Actions for Alice, Bob, Carol.', [GeminiActionType.TAG]: ['meeting', 'Q3', 'budget', 'Titan project'] } },
  { id: 'mock-img-1', type: ItemType.IMAGE, name: 'System_Architecture.png', content: 'data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAARMAAACOSExAAAAAAnQ4WAAAAARnQU1BAACxjwv8YQUAAAAJcEhZcwAADsMAAA7DAcdvqGQAAAA9SURBVHja7cExAQAAAMKg9U9tCU+gAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAIC3AcDSAAHwK39MAAAAASUVORK5CYII=', originalMimeType: 'image/png', createdAt: new Date(2023, 9, 2, 14, 30, 0), fileSize: 68000, geminiAnalysis: { [GeminiActionType.DESCRIBE]: 'Placeholder image, likely representing a system architecture diagram.', [GeminiActionType.TAG]: ['diagram', 'architecture', 'system'] } },
  { id: 'mock-folder-1', type: ItemType.FOLDER, name: 'Project Alpha Documents', content: 'Contains specifications and reports for Project Alpha.', createdAt: new Date(2023, 9, 3, 9, 15, 0), fileSize: 50, geminiAnalysis: { [GeminiActionType.TAG]: ['project alpha', 'documents', 'specs'] } },
  { id: 'mock-json-1', type: ItemType.GENERIC_FILE, name: 'config.json', content: JSON.stringify({ version: '1.2.0', features: { newLogin: true, betaDashboard: false }, apiEndpoints: { users: '/api/v1/users', products: '/api/v1/products' } }, null, 2), originalMimeType: 'application/json', createdAt: new Date(2023, 9, 4, 11, 0, 0), fileSize: 180, geminiAnalysis: { [GeminiActionType.EXTRACT_SCHEMA]: '{\n  "type": "object",\n  "properties": {\n    "version": {"type": "string"},\n    "features": {"type": "object", "properties": {"newLogin":{"type":"boolean"}, "betaDashboard":{"type":"boolean"}}},\n    "apiEndpoints": {"type": "object", "properties": {"users":{"type":"string"}, "products":{"type":"string"}}}\n  }\n}', [GeminiActionType.TAG]: ['config', 'json', 'settings', 'api'] } },
  { id: 'mock-video-url-1', type: ItemType.VIDEO_URL, name: 'Gemini API Overview', content: 'https://www.youtube.com/watch?v=exampleVideoID', originalMimeType: 'video/online-reference', createdAt: new Date(2023, 9, 15, 10,0,0), fileSize: 45, geminiAnalysis: { [GeminiActionType.GET_VIDEO_INFO]: 'Likely an overview video about the Gemini API from YouTube.', [GeminiActionType.TAG]: ['gemini', 'api', 'video', 'youtube']}},
  { id: 'mock-audio-1', type: ItemType.AUDIO_FILE, name: 'Team_Meeting_Excerpt.mp3', content: 'data:audio/mpeg;base64,SUQzBAAAAAAB...', originalMimeType: 'audio/mpeg', createdAt: new Date(2023,9,16,11,0,0), fileSize: 20000, geminiAnalysis: { [GeminiActionType.TRANSCRIBE_AUDIO]: '(Audio transcription placeholder: "Speaker 1: Let\'s discuss the new features...")', [GeminiActionType.TAG]: ['audio', 'meeting', 'team']}},
  { id: 'mock-py-1', type: ItemType.GENERIC_FILE, name: 'data_processor.py', content: "import pandas as pd\n\ndef process_data(df):\n  # Basic data cleaning\n  df.dropna(inplace=True)\n  df['normalized_value'] = df['value'] / df['value'].max()\n  return df", originalMimeType: 'text/x-python', createdAt: new Date(2023, 9, 5, 10, 0, 0), fileSize: 150, parentId: 'mock-folder-1', geminiAnalysis: { [GeminiActionType.EXTRACT_CODE_STRUCTURE]: 'Function: process_data(df) - Cleans and normalizes data in a pandas DataFrame.', [GeminiActionType.LIST_DEPENDENCIES]: ['pandas as pd'], [GeminiActionType.TAG]: ['python', 'pandas', 'data processing'] } },
];

const getSpecificFileType = (item: AggregatedItem): string => {
  if (item.type === ItemType.FOLDER) return 'Folder';
  if (item.type === ItemType.IMAGE) return 'Image';
  if (item.type === ItemType.TEXT && item.originalMimeType === 'text/html-scraped-url') return 'Scraped Webpage';
  if (item.type === ItemType.TEXT) return 'Text Snippet';
  if (item.type === ItemType.VIDEO_URL) return 'Online Video URL';
  if (item.type === ItemType.AUDIO_FILE) return 'Audio File';
  if (item.type === ItemType.RSS_FEED_URL) return 'RSS Feed URL';

  if (item.type === ItemType.GENERIC_FILE) {
    const mime = item.originalMimeType?.toLowerCase();
    const name = item.name?.toLowerCase() || ''; // Ensure name is a string for .endsWith
    if (mime === 'application/json' || name.endsWith('.json')) return 'JSON File';
    if (mime === 'text/markdown' || name.endsWith('.md')) return 'Markdown File';
    if (mime === 'text/csv' || name.endsWith('.csv')) return 'CSV File';
    if (mime === 'application/x-yaml' || mime === 'text/yaml' || name.endsWith('.yaml') || name.endsWith('.yml')) return 'YAML File';
    if (mime === 'text/x-python' || name.endsWith('.py')) return 'Python Script';
    if (mime === 'application/javascript' || name.endsWith('.js') || name.endsWith('.jsx')) return 'JavaScript File';
    if (mime === 'application/typescript' || name.endsWith('.ts') || name.endsWith('.tsx')) return 'TypeScript File';
    if (name.endsWith('.sh') || mime === 'application/x-sh') return 'Shell Script';
    if (name === 'dockerfile') return 'Dockerfile';
    if (name === 'makefile') return 'Makefile';
    if (mime?.startsWith('text/')) return 'Text File';
    return 'Generic File';
  }
  return 'Unknown Item';
};

const getItemTypeIcon = (item: AggregatedItem): React.ReactNode => {
    const specificType = getSpecificFileType(item);
    const iconClass = "w-5 h-5 text-[var(--theme-accent-secondary)]"; // Default RAG accent

    switch (specificType) {
        case 'Folder': return <FolderIconInternal className={`${iconClass} text-[var(--theme-accent-primary)]`} />; // Sky for folders
        case 'Image': return <FileIconInternal className={`${iconClass} text-emerald-400`} />;
        case 'Scraped Webpage': return <GlobeAltIconInternal className={`${iconClass} text-cyan-400`} />;
        case 'Text Snippet': case 'Text File': return <TextFileIconInternal className={`${iconClass} text-sky-400`} />;
        case 'Online Video URL': return <FilmIconInternal className={iconClass} />;
        case 'Audio File': return <MusicalNoteIconInternal className={iconClass} />;
        case 'RSS Feed URL': return <RssIconInternal className={iconClass} />;
        case 'JSON File': return <JsonIconInternal className={`${iconClass} text-amber-400`} />;
        case 'Markdown File': return <MarkdownIconInternal className={`${iconClass} text-indigo-400`} />;
        case 'CSV File': return <CsvIconInternal className={`${iconClass} text-lime-400`} />;
        case 'YAML File': return <YamlIconInternal className={`${iconClass} text-rose-400`} />;
        case 'Python Script': return <PythonIconInternal className={`${iconClass} text-yellow-400`} />;
        case 'JavaScript File': case 'TypeScript File': case 'Shell Script': case 'Dockerfile': case 'Makefile': return <CodeFileIconInternal className={`${iconClass} text-purple-400`} />;
        case 'Generic File':
        default: return <DocumentDuplicateIconInternal className={iconClass} />;
    }
};

const formatFileSize = (bytes?: number, decimals = 2) => {
  if (bytes === undefined || bytes === null || isNaN(bytes)) return 'N/A';
  if (bytes === 0) return '0 Bytes';
  const k = 1024;
  const dm = decimals < 0 ? 0 : decimals;
  const sizes = ['Bytes', 'KB', 'MB', 'GB', 'TB'];
  const i = Math.floor(Math.log(bytes) / Math.log(k));
  return parseFloat((bytes / Math.pow(k, i)).toFixed(dm)) + ' ' + sizes[i];
};

const getOrCreateRAGFolderPathInternal = (
  baseParentRAGId: string | undefined,
  relativePathParts: string[],
  currentFullItemsList: AggregatedItem[],
  batchCreatedFolders: AggregatedItem[] // Folders created in this current batch operation
): { finalParentId: string | undefined; newFoldersThisCall: AggregatedItem[] } => {
  let currentParentIdInScope = baseParentRAGId;
  const newFoldersThisCall: AggregatedItem[] = [];

  for (const part of relativePathParts) {
    if (!part) continue; // Skip empty parts (e.g., from trailing slashes)
    const existingFolder = [...currentFullItemsList, ...batchCreatedFolders].find(
      (it) => it.type === ItemType.FOLDER && it.name === part && it.parentId === currentParentIdInScope
    );

    if (existingFolder) {
      currentParentIdInScope = existingFolder.id;
    } else {
      // Create new folder
      const newFolderItem: AggregatedItem = {
        id: crypto.randomUUID(),
        type: ItemType.FOLDER,
        name: part,
        content: `Auto-created folder for path: ${[...relativePathParts.slice(0, relativePathParts.indexOf(part) + 1)].join('/')}`,
        createdAt: new Date(),
        parentId: currentParentIdInScope,
        fileSize: 0,
        geminiAnalysis: {}, // Initialize empty analysis
      };
      newFoldersThisCall.push(newFolderItem);
      batchCreatedFolders.push(newFolderItem); // Add to batch for subsequent checks within this same operation
      currentParentIdInScope = newFolderItem.id;
    }
  }
  return { finalParentId: currentParentIdInScope, newFoldersThisCall };
};

interface GeminiAnalysisStatus {
    status?: 'queued' | 'processing' | 'completed' | 'failed';
    error?: string;
}

function isGeminiAnalysisStatus(obj: any): obj is GeminiAnalysisStatus {
    return typeof obj === 'object' && obj !== null && ('status' in obj || 'error' in obj);
}

const renderSummaryContent = (summaryValInput: AggregatedItem['geminiAnalysis'][GeminiActionType.SUMMARIZE]): React.ReactNode => {
    if (summaryValInput === undefined || summaryValInput === null) {
        return <span className="text-slate-500 italic">No summary yet.</span>;
    }
    if (typeof summaryValInput === 'string') {
        return summaryValInput;
    }
    if (Array.isArray(summaryValInput)) {
        const joined = summaryValInput.join('; ');
        return joined.substring(0, 70) + (joined.length > 70 ? '...' : '');
    }

    if (isGeminiAnalysisStatus(summaryValInput)) {
        const statusObj = summaryValInput;
        let statusText = `${statusObj.status || 'Unknown'}`;
        if (statusObj.status === 'failed' && statusObj.error) {
            statusText += `: ${statusObj.error.substring(0,50) || 'Unknown error'}`;
        }
        let statusClassName = 'text-slate-400';
        if (statusObj.status === 'processing') statusClassName = 'text-amber-400 animate-pulse';
        else if (statusObj.status === 'failed') statusClassName = 'text-red-400';
        else if (statusObj.status === 'queued') statusClassName = 'text-sky-400';
        else if (statusObj.status === 'completed') statusClassName = 'text-green-400';
        
        return <span className={`${statusClassName} italic`}>{statusText}</span>;
    }
    
    if (typeof summaryValInput === 'object') {
        try {
            const str = JSON.stringify(summaryValInput);
            return str.substring(0,70) + (str.length > 70 ? '...' : '');
        } catch {
            return <span className="text-slate-500 italic">[Object Summary]</span>;
        }
    }
    return <span className="text-slate-500 italic">Summary pending...</span>;
};


const getSummaryTitleAttribute = (summaryValInput: AggregatedItem['geminiAnalysis'][GeminiActionType.SUMMARIZE]): string => {
    if (summaryValInput === undefined || summaryValInput === null) return 'No summary available.';
    if (typeof summaryValInput === 'string') return summaryValInput;
    if (Array.isArray(summaryValInput)) return summaryValInput.join('; ');

    if (isGeminiAnalysisStatus(summaryValInput)) {
        const statusObj = summaryValInput;
        let statusText = `Status: ${statusObj.status || 'Unknown'}`;
        if (statusObj.status === 'failed' && statusObj.error) {
            statusText += `: ${statusObj.error || 'Unknown error'}`;
        }
        return statusText;
    }
    
    if (typeof summaryValInput === 'object' && summaryValInput !== null) {
      try { return JSON.stringify(summaryValInput, null, 2); } catch { return "[Object Summary - cannot stringify]";}
    }
    return 'Summary information unavailable.';
};

const FolderExplorerModal: FC<{
    show: boolean;
    onClose: () => void;
    folderStack: AggregatedItem[];
    onNavigateToSubFolder: (folder: AggregatedItem) => void;
    onOpenFileDetail: (file: AggregatedItem) => void;
    onNavigateUp: () => void;
    items: AggregatedItem[];
}> = ({ show, onClose, folderStack, onNavigateToSubFolder, onOpenFileDetail, onNavigateUp, items }) => {
    if (!show || folderStack.length === 0) return null;

    const currentFolder = folderStack[folderStack.length - 1];
    const childItems = items.filter(item => item.parentId === currentFolder.id)
                             .sort((a, b) => {
                                if (a.type === ItemType.FOLDER && b.type !== ItemType.FOLDER) return -1;
                                if (a.type !== ItemType.FOLDER && b.type === ItemType.FOLDER) return 1;
                                return (a.name || '').localeCompare(b.name || '');
                             });

    return (
        <div className="fixed inset-0 bg-slate-900/80 backdrop-blur-sm flex items-center justify-center z-40 p-4" onClick={onClose}>
            <div className="bg-[var(--theme-modal-bg)] p-5 rounded-xl shadow-2xl w-full max-w-2xl max-h-[80vh] flex flex-col border border-pink-500/50" onClick={e => e.stopPropagation()}>
                <div className="flex justify-between items-center mb-3 pb-3 border-b border-[var(--theme-border-primary)]">
                    <h2 className="text-lg font-semibold text-pink-400 truncate">Exploring: {currentFolder.name}</h2>
                    <button onClick={onClose} className="p-1.5 text-slate-400 hover:text-pink-300 rounded-full hover:bg-[var(--theme-bg-accent)]/50"><XMarkIconInternal className="w-5 h-5"/></button>
                </div>

                <div className="mb-2 flex items-center text-xs text-slate-400">
                    {folderStack.length > 1 && (
                        <button onClick={onNavigateUp} className="p-1 mr-1 hover:bg-[var(--theme-bg-accent)]/50 rounded-full" title="Go Up">
                            <ArrowUturnLeftIconInternal className="w-4 h-4 text-sky-400"/>
                        </button>
                    )}
                    {folderStack.map((folder, index) => (
                        <React.Fragment key={folder.id}>
                            {index > 0 && <span className="mx-1">/</span>}
                            <button
                                onClick={() => {
                                    if (index < folderStack.length - 1) {
                                      // To enable breadcrumb clicks, pass setFolderExplorerStack here or a more complex nav function
                                    }
                                }}
                                className={`hover:underline ${index === folderStack.length - 1 ? 'text-pink-300 font-medium' : 'text-slate-400'}`}
                                disabled={index === folderStack.length -1}
                            >
                                {folder.name}
                            </button>
                        </React.Fragment>
                    ))}
                </div>
                
                <ul className="flex-grow overflow-y-auto space-y-1 pr-1">
                    {childItems.length === 0 && <li className="text-slate-400 text-sm italic text-center py-4">This folder is empty.</li>}
                    {childItems.map(item => (
                        <li key={item.id} 
                            onClick={() => item.type === ItemType.FOLDER ? onNavigateToSubFolder(item) : onOpenFileDetail(item)}
                            className="flex items-center justify-between p-2 rounded-md hover:bg-[var(--theme-bg-accent)]/70 cursor-pointer group"
                            role="button" tabIndex={0}
                            onKeyDown={(e) => { if (e.key === 'Enter' || e.key === ' ') { item.type === ItemType.FOLDER ? onNavigateToSubFolder(item) : onOpenFileDetail(item); } }}
                        >
                            <div className="flex items-center space-x-2 truncate">
                                {getItemTypeIcon(item)}
                                <span className="text-sm text-slate-200 truncate group-hover:text-pink-300">{item.name}</span>
                            </div>
                            <span className="text-xs text-slate-500 group-hover:text-slate-400 flex-shrink-0">{formatFileSize(item.fileSize)}</span>
                        </li>
                    ))}
                </ul>
            </div>
        </div>
    );
};


const RAGAgentFeature: FC<RAGAgentFeatureProps> = ({ addNotification, dashboardSettings }) => {
  const [items, setItems] = useState<AggregatedItem[]>([]);
  const [currentFolderId, setCurrentFolderId] = useState<string | undefined>(undefined);
  const [folderHistory, setFolderHistory] = useState<string[]>([]); 
  
  const [filter, setFilter] = useState('');
  const [viewMode, setViewMode] = useState<'list' | 'card'>('card');
  
  const [selectedItem, setSelectedItem] = useState<AggregatedItem | null>(null);
  const [isDetailModalOpen, setIsDetailModalOpen] = useState(false);
  const [isAddModalOpen, setIsAddModalOpen] = useState(false);
  const [isEditModalOpen, setIsEditModalOpen] = useState(false);
  const [editingItem, setEditingItem] = useState<AggregatedItem | null>(null);

  const [chatMessages, setChatMessages] = useState<ChatMessage[]>([]);
  const [chatInput, setChatInput] = useState('');
  const [isChatLoading, setIsChatLoading] = useState(false);
  const chatHistoryRef = useRef<HTMLDivElement>(null);
  const analysisQueueRef = useRef<AggregatedItem[]>([]);
  const analysisTimeoutRef = useRef<ReturnType<typeof setTimeout> | null>(null); // Correct type for Node and Browser
  const [activeAnalyses, setActiveAnalyses] = useState<Record<string, boolean>>({});

  const [showFolderExplorer, setShowFolderExplorer] = useState(false);
  const [folderExplorerStack, setFolderExplorerStack] = useState<AggregatedItem[]>([]);

  const fileInputRef = useRef<HTMLInputElement>(null);
  const folderInputRef = useRef<HTMLInputElement>(null);

  const loadItems = useCallback(() => {
    const storedItems = localStorage.getItem(RAG_ITEMS_KEY);
    if (storedItems) {
      try {
        const parsedItems = JSON.parse(storedItems).map((item: any) => {
            const createdAtDate = new Date(item.createdAt); 
            return {
                ...item,
                createdAt: isNaN(createdAtDate.getTime()) ? new Date() : createdAtDate, 
                geminiAnalysis: typeof item.geminiAnalysis === 'object' && item.geminiAnalysis !== null 
                    ? item.geminiAnalysis 
                    : {}, 
            };
        });
        setItems(parsedItems);
      } catch (e) {
        console.error("Failed to parse RAG items from localStorage", e);
        addNotification('error', 'Failed to load RAG items. Data might be corrupted. Clearing corrupted data.', {sourceView: ViewType.RAG_AGENT});
        localStorage.removeItem(RAG_ITEMS_KEY); 
        setItems([]); 
      }
    } else {
        const mockDataLoaded = localStorage.getItem(MOCK_DATA_KEY);
        if (!mockDataLoaded) {
            setItems(initialMockItems);
            localStorage.setItem(MOCK_DATA_KEY, 'true');
        } else {
            setItems([]);
        }
    }
    const storedChatHistory = localStorage.getItem(RAG_CHAT_HISTORY_KEY);
    if (storedChatHistory) {
        try { setChatMessages(JSON.parse(storedChatHistory)); } catch { setChatMessages([]); }
    }
  }, [addNotification]);

  useEffect(() => { loadItems(); }, [loadItems]);

  useEffect(() => {
    localStorage.setItem(RAG_ITEMS_KEY, JSON.stringify(items.map(item => ({...item, createdAt: item.createdAt.toISOString() }))));
  }, [items]);

  useEffect(() => {
    localStorage.setItem(RAG_CHAT_HISTORY_KEY, JSON.stringify(chatMessages));
    if (chatHistoryRef.current) {
      chatHistoryRef.current.scrollTop = chatHistoryRef.current.scrollHeight;
    }
  }, [chatMessages]);
  
  const processAnalysisQueue = useCallback(async () => {
    if (analysisQueueRef.current.length === 0) return;

    const itemToAnalyze = analysisQueueRef.current.shift();
    if (!itemToAnalyze || !itemToAnalyze._actionToRun) return;

    const actionToRun = itemToAnalyze._actionToRun;
    const customPromptForAction = itemToAnalyze._customPromptForAction;

    setActiveAnalyses(prev => ({ ...prev, [`${itemToAnalyze.id}-${actionToRun}`]: true }));
    
    setItems(prevItems => prevItems.map(i => 
      i.id === itemToAnalyze.id ? { ...i, geminiAnalysis: { ...(i.geminiAnalysis || {}), [actionToRun]: { status: 'processing' } } } : i
    ));

    try {
      const result = await analyzeWithGemini(itemToAnalyze._fullContentForAnalysis || itemToAnalyze.content, actionToRun, itemToAnalyze.originalMimeType, undefined, customPromptForAction, dashboardSettings);
      setItems(prevItems => prevItems.map(i => 
        i.id === itemToAnalyze.id ? { ...i, geminiAnalysis: { ...(i.geminiAnalysis || {}), [actionToRun]: result } } : i
      ));
      addNotification('success', `${actionToRun} analysis for "${itemToAnalyze.name}" complete.`, {sourceView: ViewType.RAG_AGENT, sourceItemId: itemToAnalyze.id});
    } catch (error) {
      const errMsg = error instanceof Error ? error.message : String(error);
      setItems(prevItems => prevItems.map(i => 
        i.id === itemToAnalyze.id ? { ...i, geminiAnalysis: { ...(i.geminiAnalysis || {}), [actionToRun]: { status: 'failed', error: errMsg } } } : i
      ));
      addNotification('error', `${actionToRun} analysis for "${itemToAnalyze.name}" failed: ${errMsg}`, {sourceView: ViewType.RAG_AGENT, sourceItemId: itemToAnalyze.id});
    } finally {
      setActiveAnalyses(prev => { const newState = {...prev}; delete newState[`${itemToAnalyze.id}-${actionToRun}`]; return newState; });
      if (analysisQueueRef.current.length > 0) {
        if (analysisTimeoutRef.current) clearTimeout(analysisTimeoutRef.current);
        analysisTimeoutRef.current = setTimeout(processAnalysisQueue, ANALYSIS_QUEUE_DELAY);
      } else {
        if (analysisTimeoutRef.current) clearTimeout(analysisTimeoutRef.current);
        analysisTimeoutRef.current = null;
      }
    }
  }, [addNotification, dashboardSettings]);

  const enqueueAnalysis = useCallback((item: AggregatedItem, actionType: GeminiActionType, customPrompt?:string) => {
    const itemWithAction = { ...item, _actionToRun: actionType, _customPromptForAction: customPrompt };
    analysisQueueRef.current.push(itemWithAction);
    setItems(prevItems => prevItems.map(i => 
        i.id === item.id ? { ...i, geminiAnalysis: { ...(i.geminiAnalysis || {}), [actionType]: { status: 'queued' } } } : i
    ));
    
    if (analysisQueueRef.current.length === 1 && !analysisTimeoutRef.current) { 
      if (analysisTimeoutRef.current) clearTimeout(analysisTimeoutRef.current);
      analysisTimeoutRef.current = setTimeout(processAnalysisQueue, ANALYSIS_QUEUE_DELAY);
    }
  }, [processAnalysisQueue]);

  const defaultActionsForItem = useCallback((item: AggregatedItem): GeminiActionType[] => {
    const actions: GeminiActionType[] = [GeminiActionType.TAG]; 
    switch (item.type) {
      case ItemType.TEXT:
        actions.push(GeminiActionType.SUMMARIZE);
        if (item.originalMimeType === 'text/html-scraped-url') actions.push(GeminiActionType.ANALYZE_WEB_DOCUMENTATION);
        break;
      case ItemType.IMAGE: actions.push(GeminiActionType.DESCRIBE); break;
      case ItemType.VIDEO_URL: actions.push(GeminiActionType.GET_VIDEO_INFO); break;
      case ItemType.AUDIO_FILE: actions.push(GeminiActionType.TRANSCRIBE_AUDIO); break;
      case ItemType.RSS_FEED_URL: actions.push(GeminiActionType.GUESS_RSS_FEED_TOPICS); break;
      case ItemType.GENERIC_FILE:
        const specificType = getSpecificFileType(item);
        if (specificType === 'JSON File') actions.push(GeminiActionType.EXTRACT_SCHEMA);
        else if (specificType === 'Python Script') actions.push(GeminiActionType.EXTRACT_CODE_STRUCTURE, GeminiActionType.LIST_DEPENDENCIES);
        else if (specificType === 'CSV File') actions.push(GeminiActionType.ANALYZE_CSV_DATA, GeminiActionType.SUGGEST_CSV_CHART_TYPE);
        else if (specificType.endsWith('Script') || specificType.endsWith('File') && item.originalMimeType?.startsWith('text/')) actions.push(GeminiActionType.SUMMARIZE);
        break;
    }
    return actions;
  }, []);


  const handleAddItem = useCallback((newItem: Omit<AggregatedItem, 'id' | 'createdAt' | 'geminiAnalysis'>) => {
    const fullNewItem: AggregatedItem = {
      ...newItem, id: crypto.randomUUID(), createdAt: new Date(), geminiAnalysis: {},
    };

    let contentToStore = fullNewItem.content;
    let fullContentForAnalysis = fullNewItem.content;

    if ((fullNewItem.type === ItemType.IMAGE || fullNewItem.type === ItemType.AUDIO_FILE) && 
        fullNewItem.content.length > MEDIA_CONTENT_LOCALSTORAGE_PLACEHOLDER_THRESHOLD_BYTES) {
        contentToStore = `placeholder_for_large_media_content_type_${fullNewItem.originalMimeType || 'unknown'}`;
        addNotification('info', `Large media content for "${fullNewItem.name}" stored as placeholder to save space. Full content available in memory.`, { sourceView: ViewType.RAG_AGENT });
    } else if (fullNewItem.type === ItemType.TEXT || fullNewItem.type === ItemType.GENERIC_FILE) {
        if (fullNewItem.content.length > GENERAL_CONTENT_TRUNCATION_LENGTH_FOR_LOCALSTORAGE) {
            contentToStore = fullNewItem.content.substring(0, GENERAL_CONTENT_TRUNCATION_LENGTH_FOR_LOCALSTORAGE) + '... (truncated in localStorage)';
        }
    }
    
    const itemForStorage = { ...fullNewItem, content: contentToStore, _fullContentForAnalysis: fullContentForAnalysis };
    setItems(prevItems => [...prevItems, itemForStorage].sort((a, b) => new Date(b.createdAt).getTime() - new Date(a.createdAt).getTime()));
    addNotification('success', `Item "${fullNewItem.name}" added.`, {sourceView: ViewType.RAG_AGENT, sourceItemId: fullNewItem.id});
    
    const itemForAnalysisQueue = {...itemForStorage, content: fullContentForAnalysis}; // Use full content for analysis
    const actions = defaultActionsForItem(itemForAnalysisQueue);
    actions.forEach(actionType => enqueueAnalysis(itemForAnalysisQueue, actionType));

    return itemForStorage; 
  }, [addNotification, enqueueAnalysis, defaultActionsForItem]);
  

  const handleEditItemSubmit = useCallback((editedItemData: AggregatedItem) => {
    if (!editingItem) return;

    const itemForStorage = { ...editedItemData };
    let fullContentForAnalysis = editedItemData.content; 
    
    if ((itemForStorage.type === ItemType.IMAGE || itemForStorage.type === ItemType.AUDIO_FILE) && 
        itemForStorage.content.length > MEDIA_CONTENT_LOCALSTORAGE_PLACEHOLDER_THRESHOLD_BYTES &&
        !itemForStorage.content.startsWith('placeholder_for_large_media_content')) {
        itemForStorage.content = `placeholder_for_large_media_content_type_${itemForStorage.originalMimeType || 'unknown'}`;
    } else if ((itemForStorage.type === ItemType.TEXT || itemForStorage.type === ItemType.GENERIC_FILE) &&
               itemForStorage.content.length > GENERAL_CONTENT_TRUNCATION_LENGTH_FOR_LOCALSTORAGE && 
               !itemForStorage.content.endsWith('... (truncated in localStorage)')) {
        itemForStorage.content = itemForStorage.content.substring(0, GENERAL_CONTENT_TRUNCATION_LENGTH_FOR_LOCALSTORAGE) + '... (truncated in localStorage)';
    }

    setItems(prevItems => prevItems.map(item => item.id === editingItem.id ? { ...itemForStorage, geminiAnalysis: item.geminiAnalysis, _fullContentForAnalysis: fullContentForAnalysis } : item)); 
    addNotification('success', `Item "${editedItemData.name}" updated.`, {sourceView: ViewType.RAG_AGENT, sourceItemId: editingItem.id});
    setIsEditModalOpen(false);
    setEditingItem(null);

    if (editedItemData.content !== editingItem.content) {
        const itemForAnalysisQueue = {...itemForStorage, content: fullContentForAnalysis}; 
        const actions = defaultActionsForItem(itemForAnalysisQueue); 
        actions.forEach(actionType => enqueueAnalysis(itemForAnalysisQueue, actionType));
    }
  }, [editingItem, addNotification, enqueueAnalysis, defaultActionsForItem]);

  const handleDeleteItem = useCallback((itemId: string) => {
    const itemToDelete = items.find(item => item.id === itemId);
    if (itemToDelete) {
      if (itemToDelete.type === ItemType.FOLDER) {
        if (confirm(`Delete folder "${itemToDelete.name}" and all its contents? This cannot be undone.`)) {
          const itemsToDelete = new Set<string>();
          const findChildrenRecursive = (folderId: string) => {
            itemsToDelete.add(folderId);
            items.filter(i => i.parentId === folderId).forEach(child => {
              if (child.type === ItemType.FOLDER) findChildrenRecursive(child.id);
              else itemsToDelete.add(child.id);
            });
          };
          findChildrenRecursive(itemId);
          setItems(prevItems => prevItems.filter(item => !itemsToDelete.has(item.id)));
          addNotification('info', `Folder "${itemToDelete.name}" and its contents deleted.`, {sourceView: ViewType.RAG_AGENT});
        }
      } else {
        if (confirm(`Delete item "${itemToDelete.name}"?`)) {
          setItems(prevItems => prevItems.filter(item => item.id !== itemId));
          addNotification('info', `Item "${itemToDelete.name}" deleted.`, {sourceView: ViewType.RAG_AGENT});
        }
      }
      if (selectedItem?.id === itemId) setSelectedItem(null);
    }
  }, [items, addNotification, selectedItem]);

  const handleFileUpload = useCallback(async (event: ChangeEvent<HTMLInputElement>) => {
    const files = event.target.files;
    if (!files || files.length === 0) return;

    const file = files[0];
    const reader = new FileReader();

    reader.onload = (e) => {
      const content = e.target?.result as string;
      let itemType: ItemType;
      const mimeType = file.type.toLowerCase();

      if (mimeType.startsWith('image/')) itemType = ItemType.IMAGE;
      else if (mimeType.startsWith('audio/')) itemType = ItemType.AUDIO_FILE;
      else if (mimeType.startsWith('text/') || mimeType === 'application/json' || mimeType === 'application/xml' || mimeType === 'application/javascript' || mimeType === 'application/typescript' || mimeType === 'application/x-yaml' || mimeType === 'text/yaml' || mimeType === 'text/markdown') {
        itemType = ItemType.GENERIC_FILE; 
      } else {
        itemType = ItemType.GENERIC_FILE;
      }
      handleAddItem({ name: file.name, type: itemType, content, originalMimeType: file.type, parentId: currentFolderId, fileSize: file.size });
    };

    if (file.type.startsWith('image/') || file.type.startsWith('audio/') || file.type.startsWith('video/')) { 
      reader.readAsDataURL(file);
    } else {
      reader.readAsText(file);
    }
    if (fileInputRef.current) fileInputRef.current.value = ""; 
    setIsAddModalOpen(false);
  }, [handleAddItem, currentFolderId]);

  const handleFolderUpload = useCallback(async (event: ChangeEvent<HTMLInputElement>) => {
    const files = event.target.files;
    if (!files || files.length === 0) return;
    setIsAddModalOpen(false); 
    addNotification('info', `Starting OS folder import. This may take a moment for many files...`, { sourceView: ViewType.RAG_AGENT });

    const newItemsBatch: AggregatedItem[] = [];
    const batchCreatedFolders: AggregatedItem[] = []; 

    for (const file of Array.from(files)) {
        const pathParts = (file.webkitRelativePath || file.name).split('/');
        const fileName = pathParts.pop() || file.name;
        const relativeFolderPathParts = pathParts;

        const { finalParentId, newFoldersThisCall } = getOrCreateRAGFolderPathInternal(
            currentFolderId, 
            relativeFolderPathParts, 
            items, 
            batchCreatedFolders 
        );
        newFoldersThisCall.forEach(nf => newItemsBatch.push(nf));

        const reader = new FileReader();
        const fileReadPromise = new Promise<void>((resolve, reject) => {
            reader.onload = (e_onload) => {
                const content = e_onload.target?.result as string;
                let itemType: ItemType = ItemType.GENERIC_FILE;
                const mimeType = file.type.toLowerCase();

                if (mimeType.startsWith('image/')) itemType = ItemType.IMAGE;
                else if (mimeType.startsWith('audio/')) itemType = ItemType.AUDIO_FILE;
                else if (mimeType.startsWith('text/') || mimeType === 'application/json' || mimeType === 'application/xml' || mimeType === 'application/javascript' || mimeType === 'application/typescript' || mimeType === 'application/x-yaml' || mimeType === 'text/yaml' || mimeType === 'text/markdown') {
                  itemType = ItemType.GENERIC_FILE; 
                }

                let contentToStore = content;
                let fullContentForAnalysis = content; 

                if ((itemType === ItemType.IMAGE || itemType === ItemType.AUDIO_FILE) && content.length > MEDIA_CONTENT_LOCALSTORAGE_PLACEHOLDER_THRESHOLD_BYTES) {
                    contentToStore = `placeholder_for_large_media_content_type_${mimeType || 'unknown'}`;
                } else if (itemType === ItemType.GENERIC_FILE && content.length > GENERAL_CONTENT_TRUNCATION_LENGTH_FOR_LOCALSTORAGE) {
                    contentToStore = content.substring(0, GENERAL_CONTENT_TRUNCATION_LENGTH_FOR_LOCALSTORAGE) + '... (truncated in localStorage)';
                }

                const newItem: AggregatedItem = {
                    id: crypto.randomUUID(), name: fileName, type: itemType,
                    content: contentToStore, 
                    _fullContentForAnalysis: fullContentForAnalysis, 
                    originalMimeType: file.type, parentId: finalParentId,
                    createdAt: new Date(), fileSize: file.size, geminiAnalysis: {}
                };
                newItemsBatch.push(newItem);
                resolve();
            };
            reader.onerror = (e_onerror) => {
                console.error(`Error reading file ${fileName}:`, e_onerror);
                addNotification('error', `Error reading file ${fileName}. Skipping.`, { sourceView: ViewType.RAG_AGENT });
                reject(e_onerror);
            };

            if (file.type.startsWith('image/') || file.type.startsWith('audio/') || file.type.startsWith('video/')) {
                reader.readAsDataURL(file);
            } else {
                reader.readAsText(file);
            }
        });
        try { await fileReadPromise; } catch {}
    }

    setItems(prevItems => [...prevItems, ...newItemsBatch].sort((a, b) => new Date(b.createdAt).getTime() - new Date(a.createdAt).getTime()));
    addNotification('success', `Folder import processed. ${newItemsBatch.length} items/folders added or identified. Analysis queued.`, {sourceView: ViewType.RAG_AGENT});
    
    newItemsBatch.filter(item => item.type !== ItemType.FOLDER).forEach(newItem => {
        const itemForAnalysis = {...newItem, content: newItem._fullContentForAnalysis || newItem.content}; 
        delete itemForAnalysis._fullContentForAnalysis; 
        const actions = defaultActionsForItem(itemForAnalysis);
        actions.forEach(actionType => enqueueAnalysis(itemForAnalysis, actionType));
    });

    if (folderInputRef.current) folderInputRef.current.value = "";
  }, [currentFolderId, items, addNotification, defaultActionsForItem, enqueueAnalysis]);

  const handleItemClick = (item: AggregatedItem) => {
    if (item.type === ItemType.FOLDER) {
      setFolderHistory(prev => [...prev, currentFolderId || 'root']); 
      setCurrentFolderId(item.id);
    } else {
      setSelectedItem(item);
      setIsDetailModalOpen(true);
    }
  };

  const handleFolderExplorerOpen = () => {
    const rootFolder = items.find(item => item.id === currentFolderId);
    if (rootFolder && rootFolder.type === ItemType.FOLDER) {
        setFolderExplorerStack([rootFolder]);
    } else { 
        const virtualRoot: AggregatedItem = { id: 'root-explorer', type: ItemType.FOLDER, name: 'My Vault (Root)', content: '', createdAt: new Date() };
        setFolderExplorerStack([virtualRoot]);
    }
    setShowFolderExplorer(true);
  };

  const handleFolderExplorerNavigateToSubFolder = (folder: AggregatedItem) => {
    setFolderExplorerStack(prev => [...prev, folder]);
  };
  const handleFolderExplorerNavigateUp = () => {
    if (folderExplorerStack.length > 1) {
      setFolderExplorerStack(prev => prev.slice(0, -1));
    }
  };
  const handleFolderExplorerFileClick = (file: AggregatedItem) => {
    handleItemClick(file); 
    setShowFolderExplorer(false); 
  };


  const goBack = () => {
    if (folderHistory.length > 0) {
      const previousFolderId = folderHistory[folderHistory.length - 1];
      setCurrentFolderId(previousFolderId === 'root' ? undefined : previousFolderId);
      setFolderHistory(prev => prev.slice(0, -1));
    }
  };

  const getBreadcrumbs = () => {
    const path: AggregatedItem[] = [];
    let currentId = currentFolderId;
    while (currentId) {
      const folder = items.find(item => item.id === currentId);
      if (folder) {
        path.unshift(folder);
        currentId = folder.parentId;
      } else {
        break;
      }
    }
    return path;
  };
  const breadcrumbs = getBreadcrumbs();

  const filteredItems = items.filter(item =>
    item.parentId === currentFolderId &&
    (
      (item.name || '').toLowerCase().includes((filter || '').toLowerCase()) ||
      (item.type === ItemType.FOLDER && (item.content || '').toLowerCase().includes((filter || '').toLowerCase())) ||
      (item.geminiAnalysis?.[GeminiActionType.TAG] && Array.isArray(item.geminiAnalysis[GeminiActionType.TAG]) && (item.geminiAnalysis[GeminiActionType.TAG] as string[]).some(tag => tag.toLowerCase().includes((filter || '').toLowerCase())))
    )
  ).sort((a, b) => {
    if (a.type === ItemType.FOLDER && b.type !== ItemType.FOLDER) return -1;
    if (a.type !== ItemType.FOLDER && b.type === ItemType.FOLDER) return 1;
    return new Date(b.createdAt).getTime() - new Date(a.createdAt).getTime();
  });


  const [customPromptAction, setCustomPromptAction] = useState<GeminiActionType | null>(null);
  const [customPromptText, setCustomPromptText] = useState('');
  const [isCustomPromptModalOpen, setIsCustomPromptModalOpen] = useState(false);
  
  const openCustomPromptModal = (item: AggregatedItem, action: GeminiActionType) => {
    setSelectedItem(item);
    setCustomPromptAction(action);
    setIsCustomPromptModalOpen(true);
    setCustomPromptText('');
  };

  const handleCustomPromptSubmit = () => {
    if (selectedItem && customPromptAction) {
      enqueueAnalysis(selectedItem, customPromptAction, customPromptText);
      setIsCustomPromptModalOpen(false);
      setSelectedItem(null);
      setCustomPromptAction(null);
    }
  };

  const handleChatSend = async () => {
    if (!chatInput.trim() || isChatLoading) return;

    const userMessage: ChatMessage = { id: crypto.randomUUID(), sender: 'user', text: chatInput.trim(), timestamp: new Date().toISOString() };
    setChatMessages(prev => [...prev, userMessage]);
    setIsChatLoading(true);
    setChatInput('');

    const historyForApi: GeminiContent[] = chatMessages
      .filter(msg => {
        if (msg.sender === 'system' && !msg.functionResponse) {
          return false;
        }
        return true;
      })
      .map(msg => ({
        role: msg.sender === 'user' ? 'user' :
              msg.sender === 'ai'   ? 'model' :
              'function', 
        parts: msg.text ? [{ text: msg.text }] :
               (msg.functionCall ? [{functionCall: msg.functionCall}] :
               (msg.functionResponse ? [{functionResponse: {name: msg.functionResponse.name, response: msg.functionResponse.response}}] :
               [])), 
      }));

    historyForApi.push({ role: 'user', parts: [{ text: userMessage.text! }] });

    const tools: Tool[] = [{
        functionDeclarations: [
            {
                name: "retrieve_rag_items",
                description: "Retrieves a list of RAG items based on search criteria like name, type, or tags. Useful for finding relevant documents or files.",
                parameters: {
                    type: GoogleGenAiType.OBJECT,
                    properties: {
                        query: { type: GoogleGenAiType.STRING, description: "Search query for item name or content snippet." },
                        itemType: { type: GoogleGenAiType.STRING, enum: Object.values(ItemType), description: "Filter by item type." },
                        tags: { type: GoogleGenAiType.ARRAY, items: { type: GoogleGenAiType.STRING }, description: "Array of tags to filter by." },
                        limit: { type: GoogleGenAiType.NUMBER, description: "Maximum number of items to return. Default 5."}
                    }
                }
            },
            {
                name: "get_rag_item_detail",
                description: "Gets detailed information about a specific RAG item by its ID, including its content and AI analyses.",
                parameters: {
                    type: GoogleGenAiType.OBJECT,
                    properties: {
                        itemId: { type: GoogleGenAiType.STRING, description: "The unique ID of the RAG item." }
                    },
                    required: ["itemId"]
                }
            }
        ]
    }];
    
    try {
        const responseParts = await chatWithAgent(historyForApi, { tools, customSystemInstruction: dashboardSettings.globalSystemInstruction }, dashboardSettings);
        let aiResponses: ChatMessage[] = [];

        for (const part of responseParts) {
            if (part.functionCall) {
                aiResponses.push({ id: crypto.randomUUID(), sender: 'ai', functionCall: part.functionCall, timestamp: new Date().toISOString(), isLoading: false });
                const functionName = part.functionCall.name;
                const args = part.functionCall.args;
                let functionResponseContent: any;

                if (functionName === "retrieve_rag_items") {
                    const { query, itemType, tags, limit = 5 } = args;
                    let results = items;
                    if (query) results = results.filter(i => i.name.toLowerCase().includes((query as string).toLowerCase()));
                    if (itemType) results = results.filter(i => i.type === itemType);
                    if (tags && Array.isArray(tags)) {
                        results = results.filter(i => i.geminiAnalysis?.[GeminiActionType.TAG] && (i.geminiAnalysis[GeminiActionType.TAG] as string[]).some(tag => tags.includes(tag)));
                    }
                    functionResponseContent = results.slice(0, limit).map(i => ({id: i.id, name: i.name, type: i.type, summary: getSummaryTitleAttribute(i.geminiAnalysis?.[GeminiActionType.SUMMARIZE]) || i.content.substring(0,50)}));
                } else if (functionName === "get_rag_item_detail") {
                    const item = items.find(i => i.id === args.itemId);
                    functionResponseContent = item ? { id: item.id, name: item.name, type: item.type, contentSnippet: item.content.substring(0, 200), analyses: item.geminiAnalysis } : { error: "Item not found" };
                } else {
                    functionResponseContent = { error: `Unknown function ${functionName}` };
                }
                
                aiResponses.push({ id: crypto.randomUUID(), sender: 'system', functionResponse: {name: functionName, response: functionResponseContent}, timestamp: new Date().toISOString(), isLoading: false });
                
                historyForApi.push({role: 'model', parts: [{functionCall: part.functionCall}]});
                historyForApi.push({role: 'function', parts: [{functionResponse: {name: functionName, response: functionResponseContent}}]});
                
                const followupResponseParts = await chatWithAgent(historyForApi, {tools, customSystemInstruction: dashboardSettings.globalSystemInstruction }, dashboardSettings);
                followupResponseParts.forEach(fp => {
                    if (fp.text) aiResponses.push({ id: crypto.randomUUID(), sender: 'ai', text: fp.text, timestamp: new Date().toISOString(), isLoading: false });
                });

            } else if (part.text) {
                 aiResponses.push({ id: crypto.randomUUID(), sender: 'ai', text: part.text, timestamp: new Date().toISOString(), isLoading: false });
            }
        }
        setChatMessages(prev => [...prev, ...aiResponses]);

    } catch (error) {
        const errMsg = error instanceof Error ? error.message : String(error);
        addNotification('error', `Chat error: ${errMsg}`, {sourceView: ViewType.RAG_AGENT});
        setChatMessages(prev => [...prev, { id: crypto.randomUUID(), sender: 'ai', text: `Sorry, I encountered an error: ${errMsg}`, timestamp: new Date().toISOString() }]);
    } finally {
        setIsChatLoading(false);
    }
  };
  
  const handleChatInputKeydown = (e: ReactKeyboardEvent<HTMLTextAreaElement>) => {
    if (e.key === 'Enter' && !e.shiftKey) {
        e.preventDefault();
        handleChatSend();
    }
  };

  const renderItem = (item: AggregatedItem, viewType: 'list' | 'card') => {
    const summaryPElement: React.ReactNode = renderSummaryContent(item.geminiAnalysis?.[GeminiActionType.SUMMARIZE]);
    const summaryTitle = getSummaryTitleAttribute(item.geminiAnalysis?.[GeminiActionType.SUMMARIZE]);

    if (viewType === 'card') {
      return (
        <div key={item.id} onClick={() => handleItemClick(item)}
          className="bg-[var(--theme-card-bg)] rounded-lg shadow-lg hover:shadow-xl transition-all duration-200 ease-in-out cursor-pointer p-4 flex flex-col justify-between border border-[var(--theme-border-primary)] hover:border-[var(--theme-accent-primary)] min-h-[180px] group relative">
          <div>
            <div className="flex justify-between items-start mb-2">
              <div className="flex items-center space-x-2 min-w-0">
                 <div className="flex-shrink-0">{getItemTypeIcon(item)}</div>
                 <h3 className="text-md font-semibold text-sky-300 break-words truncate" title={item.name}>
                    {item.name}
                 </h3>
              </div>
            </div>
            <p className="text-xs text-slate-400 mb-1 h-10 overflow-hidden" title={summaryTitle}>
              {summaryPElement || item.content.substring(0, 70) + (item.content.length > 70 ? '...' : '')}
            </p>
            {item.geminiAnalysis?.[GeminiActionType.TAG] && Array.isArray(item.geminiAnalysis[GeminiActionType.TAG]) && (item.geminiAnalysis[GeminiActionType.TAG] as string[]).length > 0 && (
              <div className="mt-1.5 flex flex-wrap gap-1 max-h-10 overflow-y-auto">
                {(item.geminiAnalysis[GeminiActionType.TAG] as string[]).slice(0,3).map(tag => (
                  <span key={tag} className="px-1.5 py-0.5 bg-teal-700 text-teal-200 text-[10px] rounded-full">{tag}</span>
                ))}
                {(item.geminiAnalysis[GeminiActionType.TAG] as string[]).length > 3 && <span className="text-teal-400 text-[10px]">+{ (item.geminiAnalysis[GeminiActionType.TAG] as string[]).length - 3} more</span>}
              </div>
            )}
          </div>
          <div className="text-xs text-slate-500 mt-2 pt-2 border-t border-[var(--theme-border-primary)]/50 flex justify-between items-center">
            <span>{new Date(item.createdAt).toLocaleDateString()}</span>
            <span>{formatFileSize(item.fileSize)}</span>
          </div>
          <div className="absolute top-2 right-2 opacity-0 group-hover:opacity-100 transition-opacity duration-150 flex space-x-1">
            <button onClick={(e) => { e.stopPropagation(); setEditingItem(item); setIsEditModalOpen(true); }} className="p-1 bg-slate-700/80 hover:bg-amber-600 rounded-full text-slate-300 hover:text-white" title="Edit"><PencilIconInternal className="w-3.5 h-3.5"/></button>
            <button onClick={(e) => { e.stopPropagation(); handleDeleteItem(item.id); }} className="p-1 bg-slate-700/80 hover:bg-red-600 rounded-full text-slate-300 hover:text-white" title="Delete"><TrashIconInternal className="w-3.5 h-3.5"/></button>
          </div>
        </div>
      );
    } else { 
      return (
        <li key={item.id} onClick={() => handleItemClick(item)}
          className="flex items-center justify-between p-3 hover:bg-[var(--theme-bg-accent)]/50 rounded-md cursor-pointer group border border-transparent hover:border-[var(--theme-accent-primary)]/30 transition-colors duration-150">
          <div className="flex items-center space-x-3 truncate flex-grow min-w-0">
            <span className="text-slate-400 group-hover:text-sky-300 flex-shrink-0">{getItemTypeIcon(item)}</span>
            <span className="text-sm text-slate-200 truncate group-hover:text-sky-200" title={item.name}>{item.name}</span>
          </div>
          <div className="flex items-center space-x-3 text-xs flex-shrink-0 ml-2">
            <span className="text-slate-400 hidden md:inline truncate max-w-[200px] xl:max-w-[300px]" title={summaryTitle}>
              {summaryPElement || (item.type !== ItemType.FOLDER ? item.content.substring(0,40)+'...' : '')}
            </span>
            <span className="text-slate-500 w-20 text-right hidden sm:inline">{formatFileSize(item.fileSize)}</span>
            <span className="text-slate-500 hidden lg:inline w-24 text-right">{new Date(item.createdAt).toLocaleDateString()}</span>
            <div className="flex items-center space-x-1 opacity-0 group-hover:opacity-100 focus-within:opacity-100 transition-opacity">
                <button onClick={(e) => { e.stopPropagation(); setEditingItem(item); setIsEditModalOpen(true); }} className="p-1 text-slate-400 hover:text-amber-400" title="Edit"><PencilIconInternal className="w-4 h-4"/></button>
                <button onClick={(e) => { e.stopPropagation(); handleDeleteItem(item.id); }} className="p-1 text-slate-400 hover:text-red-400" title="Delete"><TrashIconInternal className="w-4 h-4"/></button>
            </div>
          </div>
        </li>
      );
    }
  };

  const renderItemsRecursive = (parentId?: string, level = 0) => {
    const children = items.filter(item => item.parentId === parentId)
      .sort((a,b) => {
        if (a.type === ItemType.FOLDER && b.type !== ItemType.FOLDER) return -1;
        if (a.type !== ItemType.FOLDER && b.type === ItemType.FOLDER) return 1;
        return new Date(b.createdAt).getTime() - new Date(a.createdAt).getTime();
      });
      
    if (children.length === 0 && level === 0 && !currentFolderId) { 
        return <div className="text-center py-10 text-slate-400 col-span-full flex flex-col items-center justify-center h-full">
                    <InboxStackIconInternal className="w-16 h-16 text-slate-500 mb-4 opacity-70"/>
                    <p className="text-lg">Your OmniHoarder vault is empty.</p>
                    <p className="text-sm text-slate-500">Click "Add Item" to populate your knowledge base.</p>
                </div>;
    }
    if (children.length === 0 && currentFolderId && level === 0){ 
         return <div className="text-center py-10 text-slate-400 col-span-full flex flex-col items-center justify-center h-full">
                    <FolderIconInternal className="w-16 h-16 text-slate-500 mb-4 opacity-70"/>
                    <p className="text-lg">This folder is empty.</p>
                </div>;
    }
    return children.map(item => renderItem(item, viewMode));
  };


  return (
    <div className="h-full flex flex-col lg:flex-row space-y-4 lg:space-y-0 lg:space-x-4 overflow-hidden p-1">
      <div className="flex-grow lg:w-2/3 flex flex-col space-y-3 overflow-hidden p-1">
        <header className="flex flex-col sm:flex-row justify-between items-center gap-2 p-2 rounded-lg bg-[var(--theme-topbar-bg)]/80 sticky top-0 z-10 backdrop-blur-sm shadow-sm border-b border-[var(--theme-border-primary)]">
            <div className="flex items-center space-x-1 text-sm w-full sm:w-auto flex-shrink min-w-0">
                {currentFolderId && (
                    <button onClick={goBack} className="p-1.5 hover:bg-[var(--theme-bg-accent)]/70 rounded-full text-pink-400 flex-shrink-0" title="Go back">
                        <ChevronLeftIconInternal className="w-5 h-5"/>
                    </button>
                )}
                 <div className="text-slate-300 truncate flex items-center">
                    <button onClick={() => { setCurrentFolderId(undefined); setFolderHistory([]); }} className={`hover:underline text-ellipsis overflow-hidden whitespace-nowrap ${!currentFolderId ? 'font-semibold text-pink-300' : 'text-slate-400'}`}>{ 'My Vault' }</button>
                    {breadcrumbs.map(folder => (
                        <React.Fragment key={folder.id}>
                            <span className="text-slate-500 mx-1 flex-shrink-0">/</span>
                            <button onClick={() => {
                                const historyIndex = folderHistory.findIndex(id => id === folder.id);
                                if (historyIndex !== -1) {
                                    setFolderHistory(prev => prev.slice(0, historyIndex));
                                }
                                setCurrentFolderId(folder.id);
                            }} className={`hover:underline text-ellipsis overflow-hidden whitespace-nowrap ${folder.id === currentFolderId ? 'font-semibold text-pink-300' : 'text-slate-400'}`}>{folder.name}</button>
                        </React.Fragment>
                    ))}
                </div>
                <button onClick={handleFolderExplorerOpen} title="Explore Folders" className="p-1.5 text-pink-400 hover:bg-[var(--theme-bg-accent)]/70 rounded-full ml-auto sm:ml-1 flex-shrink-0">
                    <InboxStackIconInternal className="w-5 h-5" />
                </button>
            </div>
            <div className="flex items-center space-x-2 w-full sm:w-auto justify-between sm:justify-end flex-shrink-0">
              <input type="search" placeholder="Filter current view..." value={filter} onChange={e => setFilter(e.target.value)} className="p-2 text-sm bg-[var(--theme-input-bg)] border border-[var(--theme-input-border)] rounded-md focus:ring-1 focus:ring-pink-500 text-[var(--theme-input-text)] w-full sm:w-40"/>
              <div className="flex items-center bg-[var(--theme-input-bg)] rounded-md border border-[var(--theme-input-border)] p-0.5">
                <button onClick={() => setViewMode('list')} className={`p-1.5 rounded ${viewMode === 'list' ? 'bg-pink-600 text-white' : 'text-slate-400 hover:bg-[var(--theme-bg-accent)]'}`} title="List view"><ListBulletIconInternal className="w-4 h-4"/></button>
                <button onClick={() => setViewMode('card')} className={`p-1.5 rounded ${viewMode === 'card' ? 'bg-pink-600 text-white' : 'text-slate-400 hover:bg-[var(--theme-bg-accent)]'}`} title="Card view"><Squares2X2IconInternal className="w-4 h-4"/></button>
              </div>
              <button onClick={() => setIsAddModalOpen(true)} className="px-3 py-2 bg-pink-600 hover:bg-pink-700 text-white text-sm font-semibold rounded-md shadow-md flex items-center space-x-1.5">
                <PlusIconInternal className="w-4 h-4"/> <span className="hidden sm:inline">Add</span>
              </button>
            </div>
        </header>
        <div className={`flex-grow overflow-y-auto p-1 ${viewMode === 'card' ? 'grid grid-cols-1 sm:grid-cols-2 md:grid-cols-2 lg:grid-cols-3 xl:grid-cols-3 2xl:grid-cols-4 gap-4' : 'space-y-1.5'}`}>
          {filteredItems.length === 0 && filter ? <p className="text-center text-slate-400 py-8 col-span-full">No items match your filter in this folder.</p> : renderItemsRecursive(currentFolderId)}
        </div>
      </div>

      <div className="lg:w-1/3 lg:max-w-md bg-[var(--theme-bg-secondary)] rounded-lg shadow-xl flex flex-col border border-[var(--theme-border-primary)] p-1 sm:p-2">
        <h2 className="text-lg font-semibold text-pink-400 p-2 border-b border-[var(--theme-border-primary)]/70 flex items-center"><ChatBubbleLeftRightIconInternal className="w-5 h-5 mr-2"/> AI Chat Assistant</h2>
        <div ref={chatHistoryRef} className="flex-grow overflow-y-auto space-y-3 p-2 text-sm min-h-[200px]">
          {chatMessages.map(msg => (
            <div key={msg.id} className={`flex ${msg.sender === 'user' ? 'justify-end' : 'justify-start'}`}>
              <div className={`max-w-[85%] p-2.5 rounded-xl shadow ${msg.sender === 'user' ? 'bg-sky-600 text-white rounded-br-none' : (msg.sender === 'system' ? 'bg-slate-600 text-slate-200 rounded-bl-none italic text-xs' : 'bg-slate-700 text-slate-100 rounded-bl-none')}`}>
                {msg.isLoading && <Spinner size="w-4 h-4 inline mr-1"/>}
                {msg.text}
                {msg.functionCall && <div className="text-xs mt-1 p-1.5 bg-black/20 rounded"><p className="font-semibold">Function Call:</p> <code className="block whitespace-pre-wrap">{msg.functionCall.name}({JSON.stringify(msg.functionCall.args, null, 1)})</code></div>}
                {msg.functionResponse && <div className="text-xs mt-1 p-1.5 bg-black/20 rounded"><p className="font-semibold">Function Response ({msg.functionResponse.name}):</p> <code className="block whitespace-pre-wrap">{JSON.stringify(msg.functionResponse.response, null, 1)}</code></div>}
                <div className="text-[10px] mt-1 opacity-70">{new Date(msg.timestamp).toLocaleTimeString([], { hour: '2-digit', minute: '2-digit' })}</div>
              </div>
            </div>
          ))}
        </div>
        <div className="p-2 border-t border-[var(--theme-border-primary)]/70">
          <div className="flex items-end space-x-2">
            <textarea value={chatInput} onChange={e => setChatInput(e.target.value)} onKeyDown={handleChatInputKeydown} placeholder="Ask about your vault..." rows={2} className="flex-grow p-2 bg-[var(--theme-input-bg)] border border-[var(--theme-input-border)] rounded-lg resize-none focus:ring-1 focus:ring-pink-500 text-sm text-[var(--theme-input-text)]"></textarea>
            <button onClick={handleChatSend} disabled={isChatLoading || !chatInput.trim()} className="p-2.5 bg-pink-600 hover:bg-pink-700 text-white rounded-lg disabled:opacity-60 disabled:cursor-not-allowed self-stretch flex items-center justify-center" title="Send (Enter)">
              {isChatLoading ? <Spinner size="w-5 h-5"/> : <PaperAirplaneIconInternal className="w-5 h-5"/>}
            </button>
          </div>
        </div>
      </div>
      
      {isAddModalOpen && (
        <div className="fixed inset-0 bg-slate-900/80 backdrop-blur-sm flex items-center justify-center z-40 p-4" onClick={() => setIsAddModalOpen(false)}>
          <div className="bg-[var(--theme-modal-bg)] p-6 rounded-xl shadow-2xl w-full max-w-lg space-y-3 border border-pink-500/50" onClick={e => e.stopPropagation()}>
            <h3 className="text-xl font-semibold text-pink-400">Add New Item to <strong className="text-pink-300">{currentFolderId ? items.find(i=>i.id===currentFolderId)?.name : "My Vault (Root)"}</strong></h3>
            <button onClick={() => { if(fileInputRef.current) fileInputRef.current.click(); }} className="w-full px-4 py-3 bg-sky-600 hover:bg-sky-700 text-white rounded-md text-sm text-left flex items-center space-x-2"><FileIconInternal className="w-4 h-4"/><span>Upload File (Text, Image, Audio, JSON, CSV, Code...)</span></button>
            <input type="file" ref={fileInputRef} onChange={handleFileUpload} className="hidden" />
            <button onClick={() => { if(folderInputRef.current) folderInputRef.current.click(); }} className="w-full px-4 py-3 bg-sky-600 hover:bg-sky-700 text-white rounded-md text-sm text-left flex items-center space-x-2"><FolderIconInternal className="w-4 h-4"/><span>Upload OS Folder</span></button>
            <input type="file" ref={folderInputRef} onChange={handleFolderUpload} className="hidden" {...({ webkitdirectory: "", mozdirectory: "", directory: "" } as any)} />
            <button onClick={() => { handleAddItem({ name: 'New Text Snippet', type: ItemType.TEXT, content: 'Write something here...', parentId: currentFolderId, originalMimeType: 'text/plain' }); setIsAddModalOpen(false); }} className="w-full px-4 py-3 bg-teal-600 hover:bg-teal-700 text-white rounded-md text-sm text-left flex items-center space-x-2"><TextFileIconInternal className="w-4 h-4"/><span>Add Text Snippet</span></button>
            <button onClick={() => { handleAddItem({ name: 'New Folder', type: ItemType.FOLDER, content: 'Folder description (optional)', parentId: currentFolderId }); setIsAddModalOpen(false); }} className="w-full px-4 py-3 bg-amber-600 hover:bg-amber-700 text-white rounded-md text-sm text-left flex items-center space-x-2"><FolderIconInternal className="w-4 h-4"/><span>Create New Folder</span></button>
             <button onClick={() => { const url = prompt("Enter Video URL (e.g., YouTube, Vimeo):"); if (url) handleAddItem({ name: 'Video Link', type: ItemType.VIDEO_URL, content: url, parentId: currentFolderId, originalMimeType: 'video/online-reference' }); setIsAddModalOpen(false); }} className="w-full px-4 py-3 bg-indigo-600 hover:bg-indigo-700 text-white rounded-md text-sm text-left flex items-center space-x-2"><FilmIconInternal className="w-4 h-4"/><span>Add Video URL</span></button>
             <button onClick={() => { const url = prompt("Enter RSS Feed URL:"); if (url) handleAddItem({ name: 'RSS Feed', type: ItemType.RSS_FEED_URL, content: url, parentId: currentFolderId, originalMimeType: 'application/rss+xml' }); setIsAddModalOpen(false); }} className="w-full px-4 py-3 bg-purple-600 hover:bg-purple-700 text-white rounded-md text-sm text-left flex items-center space-x-2"><RssIconInternal className="w-4 h-4"/><span>Add RSS Feed URL</span></button>
             <button onClick={() => { const url = prompt("Enter Webpage URL to Scrape:"); if (url) { addNotification('info', 'Web scraping initiated. This might take a moment...'); fetch(`https://api.allorigins.win/get?url=${encodeURIComponent(url)}`).then(response => response.json()).then(data => { if(data.contents) handleAddItem({ name: `Scraped: ${url.substring(0,50)}...`, type: ItemType.TEXT, content: data.contents, parentId: currentFolderId, originalMimeType: 'text/html-scraped-url' }); else addNotification('error', 'Failed to scrape webpage.'); }).catch(() => addNotification('error', 'Error during web scraping.'));} setIsAddModalOpen(false); }} className="w-full px-4 py-3 bg-green-600 hover:bg-green-700 text-white rounded-md text-sm text-left flex items-center space-x-2"><GlobeAltIconInternal className="w-4 h-4"/><span>Scrape Webpage Content</span></button>
            <div className="text-right mt-2">
                <button onClick={() => setIsAddModalOpen(false)} className="px-4 py-2 text-slate-300 hover:bg-slate-700 rounded-md text-sm">Cancel</button>
            </div>
          </div>
        </div>
      )}

      {isEditModalOpen && editingItem && (
        <div className="fixed inset-0 bg-slate-900/80 backdrop-blur-sm flex items-center justify-center z-40 p-4" onClick={() => setIsEditModalOpen(false)}>
          <form onSubmit={(e) => { e.preventDefault(); handleEditItemSubmit(editingItem); }} className="bg-[var(--theme-modal-bg)] p-6 rounded-xl shadow-2xl w-full max-w-lg space-y-4 border border-pink-500/50 max-h-[80vh] flex flex-col" onClick={e => e.stopPropagation()}>
            <h3 className="text-xl font-semibold text-pink-400">Edit: {editingItem.name}</h3>
            <div className="flex-grow overflow-y-auto pr-2 space-y-3">
                <div><label className="block text-sm text-slate-300 mb-1">Name</label><input type="text" value={editingItem.name} onChange={e => setEditingItem(prev => prev ? {...prev, name: e.target.value} : null)} className="w-full p-2 bg-[var(--theme-input-bg)] border border-[var(--theme-input-border)] rounded focus:ring-1 focus:ring-pink-500"/></div>
                {editingItem.type !== ItemType.FOLDER && editingItem.type !== ItemType.IMAGE && editingItem.type !== ItemType.AUDIO_FILE && (
                <div><label className="block text-sm text-slate-300 mb-1">Content</label><textarea value={editingItem.content.startsWith('placeholder_for_large_media_content') ? '(Large media content - view in details)' : editingItem.content} onChange={e => setEditingItem(prev => prev ? {...prev, content: e.target.value} : null)} rows={6} className="w-full p-2 bg-[var(--theme-input-bg)] border border-[var(--theme-input-border)] rounded focus:ring-1 focus:ring-pink-500" disabled={editingItem.content.startsWith('placeholder_for_large_media_content')}/></div>
                )}
                 {editingItem.type === ItemType.FOLDER && (
                <div><label className="block text-sm text-slate-300 mb-1">Description</label><textarea value={editingItem.content} onChange={e => setEditingItem(prev => prev ? {...prev, content: e.target.value} : null)} rows={3} className="w-full p-2 bg-[var(--theme-input-bg)] border border-[var(--theme-input-border)] rounded focus:ring-1 focus:ring-pink-500"/></div>
                )}
            </div>
            <div className="flex justify-end space-x-2 pt-3 border-t border-[var(--theme-border-primary)]">
                <button type="button" onClick={() => setIsEditModalOpen(false)} className="px-4 py-2 text-slate-300 hover:bg-slate-700 rounded">Cancel</button>
                <button type="submit" className="px-4 py-2 bg-pink-600 hover:bg-pink-700 text-white rounded">Save Changes</button>
            </div>
          </form>
        </div>
      )}

      {isDetailModalOpen && selectedItem && (
         <div className="fixed inset-0 bg-slate-900/80 backdrop-blur-sm flex items-center justify-center z-40 p-4" onClick={() => setIsDetailModalOpen(false)}>
            <div className="bg-[var(--theme-modal-bg)] p-6 rounded-xl shadow-2xl w-full max-w-2xl max-h-[90vh] flex flex-col border border-pink-500/50" onClick={e => e.stopPropagation()}>
                <div className="flex justify-between items-center mb-3 pb-3 border-b border-[var(--theme-border-primary)]">
                    <div className="flex items-center space-x-2 min-w-0">
                        <span className="flex-shrink-0">{getItemTypeIcon(selectedItem)}</span>
                        <h3 className="text-xl font-semibold text-pink-400 truncate" title={selectedItem.name}>{selectedItem.name}</h3>
                    </div>
                    <button onClick={() => setIsDetailModalOpen(false)} className="p-1.5 text-slate-400 hover:text-pink-300 rounded-full hover:bg-[var(--theme-bg-accent)]/50"><XMarkIconInternal className="w-5 h-5"/></button>
                </div>
                <div className="flex-grow overflow-y-auto pr-2 text-sm space-y-3">
                    <p><strong>Type:</strong> <span className="text-slate-300">{getSpecificFileType(selectedItem)}</span></p>
                    <p><strong>Added:</strong> <span className="text-slate-300">{new Date(selectedItem.createdAt).toLocaleString()}</span></p>
                    <p><strong>Size:</strong> <span className="text-slate-300">{formatFileSize(selectedItem.fileSize)}</span></p>
                    {selectedItem.parentId && <p><strong>In Folder:</strong> <button onClick={() => { setIsDetailModalOpen(false); setCurrentFolderId(selectedItem.parentId); setFolderHistory(prev => { const bh = getBreadcrumbs(); return bh.slice(0, bh.findIndex(f=>f.id===selectedItem.parentId)).map(f=>f.id) || []; }); }} className="text-sky-400 hover:underline">{items.find(i=>i.id === selectedItem.parentId)?.name || 'Unknown'}</button></p>}
                    {selectedItem.originalMimeType && <p><strong>MIME Type:</strong> <span className="text-slate-300">{selectedItem.originalMimeType}</span></p>}

                    {selectedItem.type === ItemType.IMAGE && (selectedItem._fullContentForAnalysis || selectedItem.content).startsWith('data:image') && <div className="my-2"><img src={selectedItem._fullContentForAnalysis || selectedItem.content} alt={selectedItem.name} className="max-w-full max-h-64 rounded-md border border-[var(--theme-input-border)] shadow-md mx-auto"/></div>}
                    {selectedItem.type === ItemType.AUDIO_FILE && (selectedItem._fullContentForAnalysis || selectedItem.content).startsWith('data:audio') && <div className="my-2"><audio controls src={selectedItem._fullContentForAnalysis || selectedItem.content} className="w-full">Your browser does not support the audio element.</audio></div>}
                    {selectedItem.type === ItemType.VIDEO_URL && <div className="my-2"><p><strong>URL:</strong> <a href={selectedItem.content} target="_blank" rel="noopener noreferrer" className="text-sky-400 hover:underline break-all">{selectedItem.content}</a></p></div>}
                    {(selectedItem.type === ItemType.TEXT || selectedItem.type === ItemType.GENERIC_FILE) && !(selectedItem._fullContentForAnalysis || selectedItem.content).startsWith('placeholder_for_large_media_content') && (
                        <div className="mt-2"><strong className="block mb-1">Content Preview:</strong><pre className="p-2 bg-[var(--theme-input-bg)]/50 border border-[var(--theme-input-border)] rounded-md max-h-48 overflow-y-auto text-xs whitespace-pre-wrap break-words">{ (selectedItem._fullContentForAnalysis || selectedItem.content).substring(0,1000)}{(selectedItem._fullContentForAnalysis || selectedItem.content).length > 1000 && '...'}</pre></div>
                    )}
                    {(selectedItem._fullContentForAnalysis || selectedItem.content).startsWith('placeholder_for_large_media_content') && <p className="text-amber-400 text-xs italic my-2">Full content for this large media item is not displayed here to conserve browser resources. It's available in memory for AI analysis.</p>}
                    
                    <div className="mt-3 pt-3 border-t border-[var(--theme-border-primary)]">
                        <h4 className="text-md font-semibold text-sky-300 mb-2">AI Analyses:</h4>
                        {Object.keys(selectedItem.geminiAnalysis || {}).length === 0 && <p className="text-slate-400 italic">No AI analyses performed yet.</p>}
                        <div className="space-y-2 text-xs">
                        {Object.entries(selectedItem.geminiAnalysis || {}).map(([action, result]) => {
                            const actionLabel = action.replace(/_/g, ' ').replace(/\b\w/g, l => l.toUpperCase());
                            let displayResult: React.ReactNode = <span className="text-slate-500 italic">Processing...</span>;
                            let titleResult: string = 'Analysis result';

                            if (isGeminiAnalysisStatus(result)) {
                                displayResult = renderSummaryContent(result);
                                titleResult = getSummaryTitleAttribute(result);
                            } else if (Array.isArray(result)) {
                                displayResult = result.join(', ');
                                titleResult = result.join(', ');
                            } else if (typeof result === 'object' && result !== null) {
                                try { 
                                    displayResult = JSON.stringify(result, null, 2);
                                    titleResult = displayResult as string;
                                } catch { 
                                    displayResult = <span className="text-slate-500 italic">[Object data - view in console]</span>;
                                    titleResult = "[Object data - view in console]";
                                    console.log(`Analysis data for ${actionLabel}:`, result);
                                }
                            } else if (result !== undefined && result !== null) {
                                displayResult = String(result);
                                titleResult = String(result);
                            } else {
                                displayResult = <span className="text-slate-500 italic">No result yet.</span>;
                                titleResult = 'No result available.';
                            }
                            const isProcessingThis = activeAnalyses[`${selectedItem.id}-${action}`];

                            return (
                                <div key={action} className="p-2 bg-[var(--theme-input-bg)]/40 rounded-md border border-[var(--theme-input-border)]/50">
                                    <div className="flex justify-between items-center">
                                        <strong className="text-slate-300">{actionLabel}:</strong>
                                        {isProcessingThis ? (<Spinner size="w-3 h-3 text-amber-400"/>) : (
                                            <button onClick={() => openCustomPromptModal(selectedItem, action as GeminiActionType)} className="p-0.5 text-sky-400 hover:text-sky-300" title="Re-run with custom prompt">
                                                <SparklesIconInternal className="w-3 h-3"/>
                                            </button>
                                        )}
                                    </div>
                                    <div className="mt-0.5 text-slate-200 max-h-32 overflow-y-auto whitespace-pre-wrap break-words" title={titleResult}>{displayResult}</div>
                                </div>
                            );
                        })}
                        </div>
                    </div>
                    <div className="mt-2 pt-2 border-t border-[var(--theme-border-primary)]">
                        <p className="text-xs text-slate-400 mb-1">Run New Analysis:</p>
                        <div className="flex flex-wrap gap-1.5">
                            {Object.values(GeminiActionType).map(action => (
                                <button key={action} onClick={() => openCustomPromptModal(selectedItem, action)} className="px-2 py-1 bg-indigo-700 hover:bg-indigo-600 text-white text-[10px] rounded-md shadow-sm">
                                   {action.replace(/_/g, ' ').replace(/\b\w/g, l => l.toUpperCase())}
                                </button>
                            ))}
                        </div>
                    </div>
                </div>
                <div className="flex justify-end space-x-2 mt-4 pt-3 border-t border-[var(--theme-border-primary)]">
                     <button onClick={() => { setIsDetailModalOpen(false); setEditingItem(selectedItem); setIsEditModalOpen(true); }} className="px-3 py-1.5 bg-amber-600 hover:bg-amber-700 text-white text-xs rounded">Edit</button>
                     {(selectedItem.type === ItemType.TEXT || selectedItem.type === ItemType.GENERIC_FILE) && !(selectedItem._fullContentForAnalysis || selectedItem.content).startsWith("placeholder_") && (
                        <button onClick={() => { const blob = new Blob([(selectedItem._fullContentForAnalysis || selectedItem.content)], {type: selectedItem.originalMimeType || 'text/plain'}); const url = URL.createObjectURL(blob); const a = document.createElement('a'); a.href = url; a.download = selectedItem.name; a.click(); URL.revokeObjectURL(url); }} className="px-3 py-1.5 bg-green-600 hover:bg-green-700 text-white text-xs rounded flex items-center"><DownloadIconInternal className="w-3 h-3 mr-1"/>Download</button>
                     )}
                    <button onClick={() => { handleDeleteItem(selectedItem.id); setIsDetailModalOpen(false); }} className="px-3 py-1.5 bg-red-600 hover:bg-red-700 text-white text-xs rounded">Delete</button>
                </div>
            </div>
         </div>
      )}
       {isCustomPromptModalOpen && selectedItem && customPromptAction && (
        <div className="fixed inset-0 bg-slate-900/80 backdrop-blur-sm flex items-center justify-center z-50 p-4" onClick={() => setIsCustomPromptModalOpen(false)}>
          <div className="bg-[var(--theme-modal-bg)] p-6 rounded-xl shadow-2xl w-full max-w-lg space-y-3 border border-pink-500/50" onClick={e => e.stopPropagation()}>
            <h3 className="text-lg font-semibold text-pink-300">Custom Prompt for {customPromptAction.replace(/_/g, ' ')}</h3>
            <p className="text-xs text-slate-400">Item: <span className="font-medium text-slate-300">{selectedItem.name}</span></p>
            <textarea value={customPromptText} onChange={e => setCustomPromptText(e.target.value)} rows={4} placeholder="Enter your custom instructions or context here... (Optional)" className="w-full p-2 bg-[var(--theme-input-bg)] border border-[var(--theme-input-border)] rounded text-sm focus:ring-1 focus:ring-pink-500"/>
            <div className="flex justify-end space-x-2">
              <button onClick={() => setIsCustomPromptModalOpen(false)} className="px-3 py-1.5 text-slate-300 hover:bg-slate-700 rounded text-xs">Cancel</button>
              <button onClick={handleCustomPromptSubmit} className="px-4 py-1.5 bg-pink-600 hover:bg-pink-700 text-white text-xs rounded">Run Analysis</button>
            </div>
          </div>
        </div>
      )}
      {showFolderExplorer && (
          <FolderExplorerModal
            show={showFolderExplorer}
            onClose={() => setShowFolderExplorer(false)}
            folderStack={folderExplorerStack}
            onNavigateToSubFolder={handleFolderExplorerNavigateToSubFolder}
            onOpenFileDetail={handleFolderExplorerFileClick}
            onNavigateUp={handleFolderExplorerNavigateUp}
            items={items}
          />
      )}
    </div>
  );
};

export default RAGAgentFeature;
