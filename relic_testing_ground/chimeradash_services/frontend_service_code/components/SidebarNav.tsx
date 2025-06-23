
import React from 'react';
import { ViewType, SidebarNavItem, IconName, DashboardSettings } from '../types';

// --- Icon Components (ensure all used IconNames have a corresponding component) ---
const ClipboardIcon = (props: React.SVGProps<SVGSVGElement>) => (
  <svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" strokeWidth={1.5} stroke="currentColor" {...props}>
    <path strokeLinecap="round" strokeLinejoin="round" d="M15.666 3.888A2.25 2.25 0 0013.5 2.25h-3c-1.03 0-1.9.693-2.166 1.638m7.332 0c.055.194.084.4.084.612v0a.75.75 0 01-.75.75H9a.75.75 0 01-.75-.75v0c0-.212.03-.418.084-.612m7.332 0c.646.049 1.288.11 1.927.184 1.1.128 1.907 1.077 1.907 2.185V19.5a2.25 2.25 0 01-2.25 2.25H6.75A2.25 2.25 0 014.5 19.5V6.257c0-1.108.806-2.057 1.907-2.185a48.208 48.208 0 011.927-.184" />
  </svg>
);

const LinkIcon = (props: React.SVGProps<SVGSVGElement>) => (
  <svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" strokeWidth={1.5} stroke="currentColor" {...props}>
    <path strokeLinecap="round" strokeLinejoin="round" d="M13.19 8.688a4.5 4.5 0 011.242 7.244l-4.5 4.5a4.5 4.5 0 01-6.364-6.364l1.757-1.757m13.35-.622l1.757-1.757a4.5 4.5 0 00-6.364-6.364l-4.5 4.5a4.5 4.5 0 001.242 7.244" />
  </svg>
);

const ExternalLinkIcon = (props: React.SVGProps<SVGSVGElement>) => (
  <svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" strokeWidth={1.5} stroke="currentColor" {...props}>
    <path strokeLinecap="round" strokeLinejoin="round" d="M13.5 6H5.25A2.25 2.25 0 003 8.25v10.5A2.25 2.25 0 005.25 21h10.5A2.25 2.25 0 0018 18.75V10.5m-10.5 6L21 3m0 0h-5.25M21 3v5.25" />
  </svg>
);

const ClockIcon = (props: React.SVGProps<SVGSVGElement>) => (
  <svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" strokeWidth={1.5} stroke="currentColor" {...props}>
    <path strokeLinecap="round" strokeLinejoin="round" d="M12 6v6h4.5m4.5 0a9 9 0 11-18 0 9 9 0 0118 0z" />
  </svg>
);

const ClipboardDocumentCheckIcon = (props: React.SVGProps<SVGSVGElement>) => (
    <svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" strokeWidth={1.5} stroke="currentColor" {...props}>
        <path strokeLinecap="round" strokeLinejoin="round" d="M10.125 2.25h-4.5c-.621 0-1.125.504-1.125 1.125v17.25c0 .621.504 1.125 1.125 1.125h12.75c.621 0 1.125-.504 1.125-1.125v-9M10.125 2.25c.882 0 1.724.222 2.494.626M10.125 2.25a2.25 2.25 0 00-2.25 2.25M10.125 2.25v3.375c0 .621.504 1.125 1.125 1.125h3.375M9 15l2.25 2.25L15 12" />
    </svg>
);

const WrenchScrewdriverIcon = (props: React.SVGProps<SVGSVGElement>) => (
  <svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" strokeWidth={1.5} stroke="currentColor" {...props}>
    <path strokeLinecap="round" strokeLinejoin="round" d="M11.42 15.17L17.25 21A2.652 2.652 0 0021 17.25l-5.83-5.83M11.42 15.17l2.496-3.03c.528-1.036.94-2.196 1.026-3.417l.668-11.578A1.708 1.708 0 0017.15-.009L5.572 1.659c-1.221.089-2.381.502-3.417 1.026L.099 5.18c-.983.983-.983 2.57 0 3.553L3.74 12.37c1.036.528 2.196.94 3.417 1.026l3.031 2.496m-3.582-3.582l.133.133m4.244-4.243l2.331 2.331" />
  </svg>
);

const ChartPieIcon = (props: React.SVGProps<SVGSVGElement>) => (
  <svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" strokeWidth={1.5} stroke="currentColor" {...props}>
    <path strokeLinecap="round" strokeLinejoin="round" d="M10.5 6a7.5 7.5 0 107.5 7.5h-7.5V6z" />
    <path strokeLinecap="round" strokeLinejoin="round" d="M13.5 10.5H21A7.5 7.5 0 0013.5 3v7.5z" />
  </svg>
);

const SparklesIcon = (props: React.SVGProps<SVGSVGElement>) => (
  <svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" strokeWidth={1.5} stroke="currentColor" {...props}>
    <path strokeLinecap="round" strokeLinejoin="round" d="M9.813 15.904L9 18.75l-.813-2.846a4.5 4.5 0 00-3.09-3.09L1.25 12l2.846-.813a4.5 4.5 0 003.09-3.09L9 5.25l.813 2.846a4.5 4.5 0 003.09 3.09L15.75 12l-2.846.813a4.5 4.5 0 00-3.09 3.09zM18.25 12l2.846-.813a4.5 4.5 0 003.09-3.09L24 5.25l-.813 2.846a4.5 4.5 0 00-3.09 3.09L18.25 12zm0 0l-2.846.813a4.5 4.5 0 00-3.09 3.09L12 18.75l.813-2.846a4.5 4.5 0 003.09-3.09L18.25 12z" />
  </svg>
);

const CogIcon = (props: React.SVGProps<SVGSVGElement>) => (
    <svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" strokeWidth={1.5} stroke="currentColor" {...props}>
        <path strokeLinecap="round" strokeLinejoin="round" d="M4.5 12a7.5 7.5 0 0015 0m-15 0a7.5 7.5 0 1115 0m-15 0H3m18 0h-1.5m-15.036-7.126L4.536 3.874m14.928 1.002l-1.06-1.06M3.874 19.464l1.06-1.06m14.028-12.428l1.06 1.06M12 9.75v1.5m0 3v1.5m0-4.5v.01M6.126 6.126L7.186 7.186m9.628 9.628l1.06 1.06M6.126 17.874l1.06-1.06m9.628-9.628l1.06-1.06" />
    </svg>
);

const RectangleStackIcon = (props: React.SVGProps<SVGSVGElement>) => (
  <svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" strokeWidth={1.5} stroke="currentColor" {...props}>
    <path strokeLinecap="round" strokeLinejoin="round" d="M6.429 9.75L2.25 12l4.179 2.25m0-4.5l5.571 3 5.571-3m-11.142 0L2.25 7.5 12 2.25l9.75 5.25-3.75 2.016M21.75 12l-4.179-2.25m0 0l4.179-2.25m0 0L12 2.25m0 0L2.25 7.5m0 0l4.179 2.25m0 0l5.571 3m0 0l5.571-3m0 0l4.179-2.25m0 0l-4.179-2.25m-5.571 3l-5.571-3m16.642 0l-5.571 3m5.571-3l-5.571-3M4.5 15.75l7.5 4.125l7.5-4.125M4.5 15.75V4.5m15 11.25V4.5" />
  </svg>
);

const ListBulletIcon = (props: React.SVGProps<SVGSVGElement>) => (
  <svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" strokeWidth={1.5} stroke="currentColor" {...props}>
    <path strokeLinecap="round" strokeLinejoin="round" d="M8.25 6.75h7.5M8.25 12h7.5m-7.5 5.25h7.5M3.75 6.75h.007v.008H3.75V6.75zm.375 0a.375.375 0 11-.75 0 .375.375 0 01.75 0zM3.75 12h.007v.008H3.75V12zm.375 0a.375.375 0 11-.75 0 .375.375 0 01.75 0zM3.75 17.25h.007v.008H3.75v-.008zm.375 0a.375.375 0 11-.75 0 .375.375 0 01.75 0z" />
  </svg>
);

const MagnifyingGlassIcon = (props: React.SVGProps<SVGSVGElement>) => (
  <svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" strokeWidth={1.5} stroke="currentColor" {...props}>
    <path strokeLinecap="round" strokeLinejoin="round" d="M21 21l-5.197-5.197m0 0A7.5 7.5 0 105.196 5.196a7.5 7.5 0 0010.607 10.607z" />
  </svg>
);

const TableCellsIcon = (props: React.SVGProps<SVGSVGElement>) => ( 
  <svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" strokeWidth={1.5} stroke="currentColor" {...props}>
    <path strokeLinecap="round" strokeLinejoin="round" d="M3.75 6.75h16.5M3.75 12h16.5m-16.5 5.25H12" />
  </svg>
);

const Squares2X2Icon = (props: React.SVGProps<SVGSVGElement>) => ( 
  <svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" strokeWidth={1.5} stroke="currentColor" {...props}>
    <path strokeLinecap="round" strokeLinejoin="round" d="M3.75 6A2.25 2.25 0 016 3.75h2.25A2.25 2.25 0 0110.5 6v2.25a2.25 2.25 0 01-2.25 2.25H6a2.25 2.25 0 01-2.25-2.25V6zM3.75 15.75A2.25 2.25 0 016 13.5h2.25a2.25 2.25 0 012.25 2.25V18a2.25 2.25 0 01-2.25 2.25H6A2.25 2.25 0 013.75 18v-2.25zM13.5 6a2.25 2.25 0 012.25-2.25H18A2.25 2.25 0 0120.25 6v2.25A2.25 2.25 0 0118 10.5h-2.25A2.25 2.25 0 0113.5 8.25V6zM13.5 15.75a2.25 2.25 0 012.25-2.25H18a2.25 2.25 0 012.25 2.25V18A2.25 2.25 0 0118 20.25h-2.25A2.25 2.25 0 0113.5 18v-2.25z" />
  </svg>
);

const ChatBubbleLeftRightIcon = (props: React.SVGProps<SVGSVGElement>) => (
  <svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" strokeWidth={1.5} stroke="currentColor" {...props}>
    <path strokeLinecap="round" strokeLinejoin="round" d="M20.25 8.511c.884.284 1.5 1.128 1.5 2.097v4.286c0 1.136-.847 2.1-1.98 2.193-.34.027-.68.052-1.02.072v3.091l-3.68-3.091a4.523 4.523 0 00-1.028-.296H9M3.75 9.75h1.5a.75.75 0 01.75.75v4.5a.75.75 0 01-.75.75h-1.5a.75.75 0 01-.75-.75V10.5a.75.75 0 01.75-.75zM21 9.75A2.25 2.25 0 0018.75 7.5H9A2.25 2.25 0 006.75 9.75v4.5A2.25 2.25 0 009 16.5h2.652a4.516 4.516 0 011.924.227l3.734 3.114A.75.75 0 0018 22.07V16.5A2.25 2.25 0 0015.75 14.25H9.75" />
  </svg>
);

const BoltIcon = (props: React.SVGProps<SVGSVGElement>) => (
  <svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" strokeWidth={1.5} stroke="currentColor" {...props}>
    <path strokeLinecap="round" strokeLinejoin="round" d="M3.75 13.5l10.5-11.25L12 10.5h8.25L9.75 21.75 12 13.5H3.75z" />
  </svg>
);

const CalendarDaysIcon = (props: React.SVGProps<SVGSVGElement>) => <svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" strokeWidth={1.5} stroke="currentColor" {...props}><path strokeLinecap="round" strokeLinejoin="round" d="M6.75 3v2.25M17.25 3v2.25M3 18.75V7.5a2.25 2.25 0 012.25-2.25h13.5A2.25 2.25 0 0121 7.5v11.25m-18 0A2.25 2.25 0 005.25 21h13.5A2.25 2.25 0 0021 18.75m-18 0v-7.5A2.25 2.25 0 015.25 9h13.5A2.25 2.25 0 0121 11.25v7.5" /></svg>;
const AdjustmentsHorizontalIcon = (props: React.SVGProps<SVGSVGElement>) => <svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" strokeWidth={1.5} stroke="currentColor" {...props}><path strokeLinecap="round" strokeLinejoin="round" d="M10.5 6h9.75M10.5 6a2.25 2.25 0 11-4.5 0 2.25 2.25 0 014.5 0zM3.75 6H7.5m3 12h9.75m-9.75 0a2.25 2.25 0 10-4.5 0 2.25 2.25 0 004.5 0zM3.75 18H7.5M10.5 12h9.75m-9.75 0a2.25 2.25 0 11-4.5 0 2.25 2.25 0 014.5 0zM3.75 12H7.5" /></svg>;
const CodeBracketSquareIcon = (props: React.SVGProps<SVGSVGElement>) => <svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" strokeWidth={1.5} stroke="currentColor" {...props}><path strokeLinecap="round" strokeLinejoin="round" d="M14.25 9.75L16.5 12l-2.25 2.25m-4.5 0L7.5 12l2.25-2.25M6 20.25h12A2.25 2.25 0 0020.25 18V5.75A2.25 2.25 0 0018 3.5H6A2.25 2.25 0 003.75 5.75v12.5A2.25 2.25 0 006 20.25z" /></svg>;
const DocumentTextIcon = (props: React.SVGProps<SVGSVGElement>) => <svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" strokeWidth={1.5} stroke="currentColor" {...props}><path strokeLinecap="round" strokeLinejoin="round" d="M19.5 14.25v-2.625a3.375 3.375 0 00-3.375-3.375h-1.5A1.125 1.125 0 0113.5 7.125v-1.5a3.375 3.375 0 00-3.375-3.375H8.25m0 12.75h.008v.008H8.25v-.008zm0 2.25h.008v.008H8.25V17.25zm0-2.25H5.625c-.621 0-1.125.504-1.125 1.125v17.25c0 .621.504 1.125 1.125 1.125h12.75c.621 0 1.125-.504 1.125-1.125V11.25a9 9 0 00-9-9z" /></svg>; 
const CpuChipIcon = (props: React.SVGProps<SVGSVGElement>) => <svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" strokeWidth={1.5} stroke="currentColor" {...props}><path strokeLinecap="round" strokeLinejoin="round" d="M8.25 3v1.5M4.5 8.25H3m18 0h-1.5M4.5 12H3m18 0h-1.5m-15 .75h.008v.008H4.5v-.008zm0 2.25h.008v.008H4.5v-.008zM8.25 15V21H3V15h5.25zm7.5 0v5.25H21V15h-5.25zM8.25 8.25h7.5V3h-7.5v5.25zM12 12.75h.008v.008H12v-.008z" /></svg>;
const PlayCircleIcon = (props: React.SVGProps<SVGSVGElement>) => <svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" strokeWidth={1.5} stroke="currentColor" {...props}><path strokeLinecap="round" strokeLinejoin="round" d="M21 12a9 9 0 11-18 0 9 9 0 0118 0z" /><path strokeLinecap="round" strokeLinejoin="round" d="M15.91 11.672a.375.375 0 010 .656l-5.603 3.113a.375.375 0 01-.557-.328V8.887c0-.286.307-.466.557-.327l5.603 3.112z" /></svg>;
const BellAlertIcon = (props: React.SVGProps<SVGSVGElement>) => <svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" strokeWidth={1.5} stroke="currentColor" {...props}><path strokeLinecap="round" strokeLinejoin="round" d="M14.857 17.082a23.848 23.848 0 005.454-1.31A8.967 8.967 0 0118 9.75v-.7V9A6 6 0 006 9v.75a8.967 8.967 0 01-2.312 6.022c1.733.64 3.56 1.017 5.454 1.31m5.714 0a24.255 24.255 0 01-5.714 0m5.714 0a3 3 0 11-5.714 0M12 18.75a9.75 9.75 0 007.242-3.346" /></svg>;
const ArchiveBoxArrowDownIcon = (props: React.SVGProps<SVGSVGElement>) => <svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" strokeWidth={1.5} stroke="currentColor" {...props}><path strokeLinecap="round" strokeLinejoin="round" d="M20.25 7.5l-.625 10.632a2.25 2.25 0 01-2.247 2.118H6.622a2.25 2.25 0 01-2.247-2.118L3.75 7.5M10 11.25h4M3.375 7.5h17.25c.621 0 1.125-.504 1.125-1.125V6.375c0-.621-.504-1.125-1.125-1.125H3.375c-.621 0-1.125.504-1.125 1.125v.001c0 .621.504 1.125 1.125 1.125zM12 15.75V21m-3.375-5.25L12 19.125l3.375-3.375" /></svg>;
const InboxStackIcon = (props: React.SVGProps<SVGSVGElement>) => <svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" strokeWidth={1.5} stroke="currentColor" {...props}><path strokeLinecap="round" strokeLinejoin="round" d="M3.75 12h16.5m-16.5 3.75h16.5M3.75 19.5h16.5M5.625 4.5h12.75a1.875 1.875 0 010 3.75H5.625a1.875 1.875 0 010-3.75z" /></svg>;
const ArrowPathIcon = (props: React.SVGProps<SVGSVGElement>) => <svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" strokeWidth={1.5} stroke="currentColor" {...props}><path strokeLinecap="round" strokeLinejoin="round" d="M16.023 9.348h4.992v-.001M2.985 19.644v-4.992m0 0h4.992m-4.993 0l3.181 3.183a8.25 8.25 0 0013.803-3.7M4.031 9.865a8.25 8.25 0 0113.803-3.7l3.181 3.182m0-4.991v4.99" /></svg>;
const KeyIcon = (props: React.SVGProps<SVGSVGElement>) => <svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" strokeWidth={1.5} stroke="currentColor" {...props}><path strokeLinecap="round" strokeLinejoin="round" d="M15.75 5.25a3 3 0 013 3m3 0a6 6 0 01-7.029 5.912c-.563-.097-1.159.026-1.563.43L10.5 17.25H8.25v2.25H6v2.25H2.25v-2.818c0-.597.237-1.17.659-1.591l6.499-6.499c.404-.404.527-1 .43-1.563A6 6 0 1121.75 8.25z" /></svg>;
const VariableIcon = (props: React.SVGProps<SVGSVGElement>) => <svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" strokeWidth={1.5} stroke="currentColor" {...props}><path strokeLinecap="round" strokeLinejoin="round" d="M4.75 4.75h14.5M4.75 19.25h14.5M12 4.75v14.5M4.75 12H12m7.25 0H12M7.5 8.25l3.75 3.75L7.5 15.75M16.5 8.25L12.75 12l3.75 3.75" /></svg>;
const ShareIcon = (props: React.SVGProps<SVGSVGElement>) => <svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" strokeWidth={1.5} stroke="currentColor" {...props}><path strokeLinecap="round" strokeLinejoin="round" d="M7.217 10.907a2.25 2.25 0 100 2.186m0-2.186c.195.025.383.052.571.081M7.217 10.907a2.25 2.25 0 012.186 0m0 0c.025.195.052.383.081.571M16.783 13.093a2.25 2.25 0 100-2.186m0 2.186c-.195-.025-.383-.052-.571-.081m.571.081a2.25 2.25 0 01-2.186 0m0 0c-.025-.195-.052-.383-.081-.571M9.75 12a2.25 2.25 0 100 4.5 2.25 2.25 0 000-4.5z" /></svg>;
const Bars3Icon = (props: React.SVGProps<SVGSVGElement>) => <svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" strokeWidth={1.5} stroke="currentColor" {...props}><path strokeLinecap="round" strokeLinejoin="round" d="M3.75 6.75h16.5M3.75 12h16.5m-16.5 5.25h16.5" /></svg>;
const XMarkIcon = (props: React.SVGProps<SVGSVGElement>) => <svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" strokeWidth={1.5} stroke="currentColor" {...props}><path strokeLinecap="round" strokeLinejoin="round" d="M6 18L18 6M6 6l12 12" /></svg>;
const PhotoIcon = (props: React.SVGProps<SVGSVGElement>) => <svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" strokeWidth={1.5} stroke="currentColor" {...props}><path strokeLinecap="round" strokeLinejoin="round" d="M2.25 15.75l5.159-5.159a2.25 2.25 0 013.182 0l5.159 5.159m-1.5-1.5l1.409-1.409a2.25 2.25 0 013.182 0l2.909 2.909m-18 3.75h16.5a1.5 1.5 0 001.5-1.5V6a1.5 1.5 0 00-1.5-1.5H3.75A1.5 1.5 0 002.25 6v12a1.5 1.5 0 001.5 1.5zm10.5-11.25h.008v.008h-.008V8.25zm.158 0a.225.225 0 11-.45 0 .225.225 0 01.45 0z" /></svg>;
const CommandIcon = (props: React.SVGProps<SVGSVGElement>) => <svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" strokeWidth={1.5} stroke="currentColor" {...props}><path strokeLinecap="round" strokeLinejoin="round" d="M6.75 7.5l3 3-3 3m4.5-6l3 3-3 3m5.25-3h-6.75" /></svg>; 
const ChevronDownIcon = (props: React.SVGProps<SVGSVGElement>) => <svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" strokeWidth={1.5} stroke="currentColor" {...props}><path strokeLinecap="round" strokeLinejoin="round" d="M19.5 8.25l-7.5 7.5-7.5-7.5" /></svg>;
const ChevronUpIcon = (props: React.SVGProps<SVGSVGElement>) => <svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" strokeWidth={1.5} stroke="currentColor" {...props}><path strokeLinecap="round" strokeLinejoin="round" d="M4.5 15.75l7.5-7.5 7.5 7.5" /></svg>;
const ChevronRightIcon = (props: React.SVGProps<SVGSVGElement>) => <svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" strokeWidth={1.5} stroke="currentColor" {...props}><path strokeLinecap="round" strokeLinejoin="round" d="M8.25 4.5l7.5 7.5-7.5 7.5" /></svg>;

const BanknotesIcon = (props: React.SVGProps<SVGSVGElement>) => (
  <svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" strokeWidth={1.5} stroke="currentColor" {...props}>
    <path strokeLinecap="round" strokeLinejoin="round" d="M2.25 18.75a60.07 60.07 0 0115.797 2.101c.727.198 1.453-.342 1.453-1.096V18.75M3.75 4.5v.75A.75.75 0 013 6A2.25 2.25 0 00.75 8.25v10.5a2.25 2.25 0 002.25 2.25h10.5A2.25 2.25 0 0015.75 18.75V16.5m-13.5-9L12 3m0 0l4.5 4.5M12 3v13.5" />
  </svg>
);
const MapIcon = (props: React.SVGProps<SVGSVGElement>) => (
  <svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" strokeWidth={1.5} stroke="currentColor" {...props}>
    <path strokeLinecap="round" strokeLinejoin="round" d="M9 6.75V15m6-6v8.25m.503 3.498l4.875-2.437c.381-.19.622-.58.622-1.006V4.82c0-.836-.88-1.37-1.625-1.006l-4.875 2.437a1.125 1.125 0 01-1.25 0L9 4.875M9 15l-4.875 2.437A1.125 1.125 0 013 16.437V7.82c0-.836.88-1.37 1.625-1.006L9 9.25m6 6l4.875-2.437A1.125 1.125 0 0021 11.563V4.82c0-.836-.88-1.37-1.625-1.006L15 6.25M15 9.75l-4.875-2.437A1.125 1.125 0 009 8.313V15.18c0 .836.88 1.37 1.625 1.006L15 13.75" />
  </svg>
);
const ShieldCheckIcon = (props: React.SVGProps<SVGSVGElement>) => (
  <svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" strokeWidth={1.5} stroke="currentColor" {...props}>
    <path strokeLinecap="round" strokeLinejoin="round" d="M9 12.75L11.25 15 15 9.75m-3-7.036A11.959 11.959 0 013.598 6 11.99 11.99 0 003 9.749c0 5.592 3.824 10.29 9 11.623 5.176-1.332 9-6.03 9-11.622A11.99 11.99 0 0020.402 6a11.959 11.959 0 01-1.598-2.286" />
  </svg>
);
const ServerStackIcon = (props: React.SVGProps<SVGSVGElement>) => (
  <svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" strokeWidth={1.5} stroke="currentColor" {...props}>
    <path strokeLinecap="round" strokeLinejoin="round" d="M6 12L3.269 3.126A59.768 59.768 0 0121.485 12 59.77 59.77 0 013.27 20.876L5.999 12zm0 0h7.5" />
  </svg>
);
const InformationCircleIcon = (props: React.SVGProps<SVGSVGElement>) => (
  <svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" strokeWidth={1.5} stroke="currentColor" {...props}>
    <path strokeLinecap="round" strokeLinejoin="round" d="M11.25 11.25l.041-.02a.75.75 0 011.063.852l-.708 2.836a.75.75 0 001.063.853l.041-.021M21 12a9 9 0 11-18 0 9 9 0 0118 0zm-9-3.75h.008v.008H12V8.25z" />
  </svg>
);
const PaintBrushIcon = (props: React.SVGProps<SVGSVGElement>) => (
  <svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" strokeWidth={1.5} stroke="currentColor" {...props}>
    <path strokeLinecap="round" strokeLinejoin="round" d="M9.53 16.122a3 3 0 00-5.78 1.128 2.25 2.25 0 01-2.4 2.245 4.5 4.5 0 008.4-2.245c0-.399-.078-.78-.22-1.128zm0 0a15.998 15.998 0 003.388-1.62m-5.043-.025a15.994 15.994 0 011.622-3.395m3.42 3.42a15.995 15.995 0 004.764-4.648l3.876-5.814a1.151 1.151 0 00-1.597-1.597L14.146 6.32a15.996 15.996 0 00-4.649 4.763m3.42 3.42a6.776 6.776 0 00-3.42-3.42" />
  </svg>
);
const Cog8ToothIcon = (props: React.SVGProps<SVGSVGElement>) => (
  <svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" strokeWidth={1.5} stroke="currentColor" {...props}>
    <path strokeLinecap="round" strokeLinejoin="round" d="M10.343 3.94c.09-.542.56-.94 1.11-.94h1.093c.55 0 1.02.398 1.11.94l.149.894c.07.424.384.764.78.93s.89-.093 1.24-.39l.658-.658c.432-.432 1.143-.432 1.574 0l.775.775a1.113 1.113 0 010 1.574l-.658.658c-.302.301-.52.727-.392 1.24s.504.71.93.78l.893.15c.542.09.94.56.94 1.11v1.093c0 .55-.398 1.02-.94 1.11l-.893.149c-.424.07-.764.383-.93.78s.093.89.392 1.24l.658.658c.432.432.432 1.142 0 1.574l-.775.775a1.113 1.113 0 01-1.574 0l-.658-.658c-.302-.3-.727-.52-.1.24-.39s-.71.504-.78.93l-.15.892c-.09.542-.56.94-1.11.94h-1.093c-.55 0-1.02-.398-1.11-.94l-.149-.894c-.07-.424-.384-.764-.78-.93s-.89.093-1.24.39l-.658.658c-.432.432-1.143-.432-1.574 0l-.775-.775a1.113 1.113 0 010-1.574l.658-.658c.302-.301.52-.727.392-1.24s-.504-.71-.93-.78l-.893-.15c-.542-.09-.94-.56-.94-1.11V9.423c0-.55.398-1.02.94-1.11l.893-.149c.424-.07.764-.383.93-.78s-.093-.89-.392-1.24l-.658-.658c-.432-.432-.432-1.142 0 1.574l.775-.775A1.113 1.113 0 018.128 3l.658.658c.3.302.727.52 1.24.39s.71-.504.78-.93l.15-.892z" /><path strokeLinecap="round" strokeLinejoin="round" d="M15 12a3 3 0 11-6 0 3 3 0 016 0z" />
  </svg>
);


// Map IconName to Component
export const iconMap: Record<IconName, React.ElementType> = {
  ClipboardIcon, LinkIcon, ClockIcon, ClipboardDocumentCheckIcon, WrenchScrewdriverIcon,
  ChartPieIcon, SparklesIcon, CogIcon, ExternalLinkIcon, RectangleStackIcon, ListBulletIcon,
  MagnifyingGlassIcon, TableCellsIcon, Squares2X2Icon, ChatBubbleLeftRightIcon, BoltIcon,
  CalendarDaysIcon, AdjustmentsHorizontalIcon, CodeBracketSquareIcon, DocumentTextIcon,
  CpuChipIcon, PlayCircleIcon, BellAlertIcon, ArchiveBoxArrowDownIcon,
  InboxStackIcon, ArrowPathIcon, KeyIcon, VariableIcon, ShareIcon,
  Bars3Icon, XMarkIcon, PhotoIcon, CommandIcon, ChevronDownIcon, ChevronUpIcon, ChevronRightIcon,
  BanknotesIcon, MapIcon, ShieldCheckIcon, ServerStackIcon, InformationCircleIcon, PaintBrushIcon, Cog8ToothIcon
};


interface SidebarNavProps {
  navItems: SidebarNavItem[];
  activeView: ViewType | null; 
  setActiveView: (view: ViewType) => void;
  isSidebarOpen: boolean;
  toggleSidebar: () => void;
  dashboardSettings: DashboardSettings;
}

const SidebarNav: React.FC<SidebarNavProps> = ({ navItems, activeView, setActiveView, isSidebarOpen, toggleSidebar, dashboardSettings }) => {
  const handleItemClick = (item: SidebarNavItem) => {
    if (item.type === 'VIEW' && item.view) {
      setActiveView(item.view);
    } else if (item.type === 'URL' && item.url) {
      window.open(item.url, '_blank', 'noopener,noreferrer');
    }
  };

  const sidebarWidthClass = isSidebarOpen ? 'w-64' : 'w-20';
  
  return (
    <aside 
      className={`${sidebarWidthClass} bg-[var(--theme-sidebar-bg)] p-4 space-y-6 fixed top-0 left-0 h-full z-30 shadow-lg transition-all duration-300 ease-in-out flex flex-col`}
    >
      <div className={`flex items-center ${isSidebarOpen ? 'justify-between' : 'justify-center'} mb-6 pt-2 h-10`}>
        {dashboardSettings.showLogo && dashboardSettings.logoBase64 && (
           <img 
             src={dashboardSettings.logoBase64} 
             alt="Dashboard Logo" 
             className={`transition-all duration-300 ease-in-out object-contain ${isSidebarOpen ? 'max-w-[2rem] max-h-[2rem]' : 'max-w-[2.5rem] max-h-[2.5rem]' } mr-2`} 
             style={{
                maxWidth: isSidebarOpen ? (dashboardSettings.logoMaxWidth ? `${dashboardSettings.logoMaxWidth}px` : '2rem') : '2.5rem',
                maxHeight: isSidebarOpen ? (dashboardSettings.logoMaxHeight ? `${dashboardSettings.logoMaxHeight}px` : '2rem') : '2.5rem',
             }}
           />
        )}
        {isSidebarOpen && (
          <span 
            className="text-2xl font-semibold text-[var(--theme-sidebar-accent)] truncate" 
            title={dashboardSettings.dashboardTitle}
            style={{fontFamily: 'var(--theme-font-family-title, sans-serif)'}}
          >
            {dashboardSettings.dashboardTitle}
          </span>
        )}
      </div>
      <nav className="flex-grow">
        <ul>
          {navItems.map(item => {
            const IconComponent = iconMap[item.iconName] || LinkIcon; 
            const isCurrent = item.type === 'VIEW' && activeView === item.view;
            return (
              <li key={item.id} title={isSidebarOpen ? undefined : item.label}>
                <button
                  onClick={() => handleItemClick(item)}
                  className={`w-full flex items-center space-x-3 py-3 rounded-md transition-colors duration-200 text-left group
                    ${isSidebarOpen ? 'px-4' : 'justify-center px-0'}
                    ${isCurrent
                      ? 'bg-[var(--theme-sidebar-accent)] text-white shadow-md'
                      : 'text-[var(--theme-sidebar-text)] hover:bg-[var(--theme-accent-primary)]/20 hover:text-[var(--theme-sidebar-accent)]'
                    }`}
                  aria-current={isCurrent ? 'page' : undefined}
                >
                  <IconComponent 
                     className={`flex-shrink-0 ${isSidebarOpen && isCurrent ? 'text-white' : isSidebarOpen ? 'text-[var(--theme-text-secondary)] group-hover:text-[var(--theme-sidebar-accent)]' : isCurrent ? 'text-white' : 'text-[var(--theme-sidebar-text)] group-hover:text-[var(--theme-sidebar-accent)]'}`} 
                     style={{
                        width: 'var(--theme-sidebar-font-icon-size, 1.25rem)', 
                        height: 'var(--theme-sidebar-font-icon-size, 1.25rem)'
                     }}
                  />
                  {isSidebarOpen && <span className={`truncate`} title={item.label} style={{fontSize: 'var(--theme-sidebar-font-item-text-size, 0.875rem)'}}>{item.label}</span>}
                </button>
              </li>
            );
          })}
        </ul>
      </nav>
       <div className={`mt-auto pt-4 border-t border-[var(--theme-border-primary)] ${isSidebarOpen ? 'pb-2' : 'pb-0'}`}>
        <button
            onClick={toggleSidebar}
            className={`w-full flex items-center ${isSidebarOpen ? 'justify-start px-4' : 'justify-center'} py-2 text-[var(--theme-text-secondary)] hover:text-[var(--theme-sidebar-accent)] hover:bg-[var(--theme-accent-primary)]/10 rounded-md transition-colors`}
            title={isSidebarOpen ? "Collapse Sidebar" : "Expand Sidebar"}
        >
            {isSidebarOpen ? <XMarkIcon className="w-6 h-6 mr-3" /> : <Bars3Icon className="w-6 h-6" />}
            {isSidebarOpen && <span style={{fontSize: 'var(--theme-sidebar-font-item-text-size, 0.875rem)'}}>Collapse</span>}
        </button>
        {isSidebarOpen && (
            <div className="text-xs text-[var(--theme-text-secondary)]/50 text-center mt-2">
                <p>&copy; {new Date().getFullYear()}</p>
            </div>
        )}
      </div>
    </aside>
  );
};

export default SidebarNav;
