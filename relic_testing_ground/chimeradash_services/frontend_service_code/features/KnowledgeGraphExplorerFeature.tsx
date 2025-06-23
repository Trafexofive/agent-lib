
import React, { useEffect, useRef, useState, useCallback } from 'react';
import { Network } from 'vis-network/standalone/esm/vis-network.js';
import { DataSet } from 'vis-data/peer';
import { AggregatedItem, ItemType, NotificationItem, ViewType, DashboardSettings, GeminiActionType } from '../types';
import Spinner from '../components/Spinner';

// Helper function (can be moved to a utils file later)
const formatFileSize = (bytes?: number, decimals = 2) => {
  if (bytes === undefined || bytes === null || isNaN(bytes)) return 'N/A';
  if (bytes === 0) return '0 Bytes';
  const k = 1024;
  const dm = decimals < 0 ? 0 : decimals;
  const sizes = ['Bytes', 'KB', 'MB', 'GB', 'TB'];
  const i = Math.floor(Math.log(bytes) / Math.log(k));
  return parseFloat((bytes / Math.pow(k, i)).toFixed(dm)) + ' ' + sizes[i];
};

const getSpecificFileTypeForGraph = (item: AggregatedItem): string => {
  if (item.type === ItemType.FOLDER) return 'Folder';
  if (item.type === ItemType.IMAGE) return 'Image';
  if (item.type === ItemType.TEXT && item.originalMimeType === 'text/html-scraped-url') return 'Scraped Webpage';
  if (item.type === ItemType.TEXT) return 'Text Snippet';
  if (item.type === ItemType.VIDEO_URL) return 'Online Video URL';
  if (item.type === ItemType.AUDIO_FILE) return 'Audio File';
  if (item.type === ItemType.RSS_FEED_URL) return 'RSS Feed URL';
  if (item.type === ItemType.GENERIC_FILE) {
    const mime = item.originalMimeType?.toLowerCase();
    const name = item.name?.toLowerCase() || '';
    if (mime === 'application/json' || name.endsWith('.json')) return 'JSON File';
    if (mime === 'text/markdown' || name.endsWith('.md')) return 'Markdown File';
    if (mime === 'text/csv' || name.endsWith('.csv')) return 'CSV File';
    if (mime === 'application/x-yaml' || mime === 'text/yaml' || name.endsWith('.yaml') || name.endsWith('.yml')) return 'YAML File';
    if (mime === 'text/x-python' || name.endsWith('.py')) return 'Python Script';
    if (name.endsWith('.sh') || mime === 'application/x-sh') return 'Shell Script';
    if (mime?.startsWith('text/')) return 'Text File';
    return 'Generic File';
  }
  return 'Unknown Item';
};

const getResolvedColor = (cssVar: string, fallback: string = '#CCCCCC') => {
    if (typeof document === 'undefined') return fallback; 
    return document.documentElement.style.getPropertyValue(cssVar).trim() || fallback;
};


interface KnowledgeGraphExplorerFeatureProps {
  addNotification: ( 
    type: NotificationItem['type'], 
    message: string, 
    sourceDetails?: { sourceView?: ViewType }
  ) => void;
  dashboardSettings: DashboardSettings;
}

const KnowledgeGraphExplorerFeature: React.FC<KnowledgeGraphExplorerFeatureProps> = ({ addNotification, dashboardSettings }) => {
  const graphRef = useRef<HTMLDivElement>(null);
  const networkInstanceRef = useRef<Network | null>(null);
  const [isLoading, setIsLoading] = useState(true);
  const [selectedNodeInfo, setSelectedNodeInfo] = useState<AggregatedItem | null>(null);
  const [ragItems, setRagItems] = useState<AggregatedItem[]>([]);

  const loadRAGItems = useCallback(() => {
    const storedItems = localStorage.getItem('rag-agent-items');
    if (storedItems) {
      try {
        const parsedItems = JSON.parse(storedItems).map((item: any) => ({
          ...item,
          createdAt: new Date(item.createdAt), 
        }));
        setRagItems(parsedItems);
      } catch (e) {
        console.error("Failed to parse RAG items from localStorage", e);
        addNotification('error', 'Failed to load RAG items for graph.');
        setRagItems([]);
      }
    } else {
      setRagItems([]);
      addNotification('info', 'No RAG items found to display in graph.');
    }
  }, [addNotification]);

  useEffect(() => {
    loadRAGItems();
  }, [loadRAGItems]);

  useEffect(() => {
    if (!graphRef.current || ragItems.length === 0) {
        if (ragItems.length === 0 && !isLoading) setIsLoading(false);
        return;
    }
    setIsLoading(true);

    const nodesArr: any[] = [];
    const edgesArr: any[] = [];

    ragItems.forEach(item => {
      let nodeShape = 'ellipse'; 
      let nodeGroup: string = item.type;
      let nodeImage = undefined;
      let nodeIcon: { face?: string; code?: string; size?: number; color?: string; } | undefined = undefined;

      const defaultIconColor = getResolvedColor('--theme-text-primary', '#E0E0E0');
      const defaultIconSize = 30;
      const defaultIconFace = 'FontAwesome'; // Assuming FontAwesome is available via CSS

      switch (item.type) {
        case ItemType.FOLDER:
          nodeShape = 'icon';
          nodeIcon = { face: defaultIconFace, code: '\uf07c', size: defaultIconSize + 5, color: getResolvedColor('--theme-accent-secondary') }; // fa-folder
          break;
        case ItemType.IMAGE:
          if (item.content && item.content.startsWith('data:image')) {
            nodeShape = 'circularImage';
            nodeImage = item.content;
            nodeGroup = ItemType.IMAGE; // Group for successful images
          } else {
            nodeShape = 'icon';
            nodeIcon = { face: defaultIconFace, code: '\uf03e', size: defaultIconSize, color: getResolvedColor('--theme-success') }; // fa-image
            nodeGroup = 'IMAGE_FALLBACK_ICON'; 
          }
          break;
        case ItemType.TEXT:
          nodeShape = 'icon';
          if (item.originalMimeType === 'text/html-scraped-url') {
            nodeIcon = { face: defaultIconFace, code: '\uf0ac', size: defaultIconSize, color: getResolvedColor('--theme-info', '#3B82F6') }; // fa-globe
          } else {
            nodeIcon = { face: defaultIconFace, code: '\uf15c', size: defaultIconSize, color: getResolvedColor('--theme-info') }; // fa-file-alt
          }
          break;
        case ItemType.VIDEO_URL:
          nodeShape = 'icon';
          nodeIcon = { face: defaultIconFace, code: '\uf03d', size: defaultIconSize, color: getResolvedColor('--theme-accent-primary', '#FFC107') }; // fa-video
          break;
        case ItemType.AUDIO_FILE:
          nodeShape = 'icon';
          nodeIcon = { face: defaultIconFace, code: '\uf001', size: defaultIconSize, color: getResolvedColor('--theme-accent-secondary', '#9C27B0') }; // fa-music
          break;
        case ItemType.RSS_FEED_URL:
          nodeShape = 'icon';
          nodeIcon = { face: defaultIconFace, code: '\uf09e', size: defaultIconSize, color: getResolvedColor('--theme-accent-primary', '#FF9800') }; // fa-rss
          break;
        case ItemType.GENERIC_FILE:
          const specificType = getSpecificFileTypeForGraph(item);
          nodeShape = 'icon';
          let fileIconCode = '\uf15b'; // fa-file (generic)
          let fileIconColor = getResolvedColor('--theme-text-secondary');

          if (specificType.includes('JSON')) { fileIconCode = '\uf1c9'; fileIconColor = getResolvedColor('--theme-accent-primary', '#FF9800'); }
          else if (specificType.includes('CSV')) { fileIconCode = '\uf0ce'; fileIconColor = getResolvedColor('--theme-success'); } // fa-table
          else if (specificType.includes('YAML')) { fileIconCode = '\uf1c9'; fileIconColor = getResolvedColor('--theme-accent-secondary', '#FFC107');}
          else if (specificType.includes('Python Script')) { fileIconCode = '\uf1c9'; fileIconColor = getResolvedColor('--theme-info', '#3B82F6'); } // fa-file-code for python
          else if (specificType.includes('Shell Script')) { fileIconCode = '\uf120'; fileIconColor = getResolvedColor('--theme-text-primary'); } // fa-terminal
          else if (specificType.includes('Markdown')) { fileIconCode = '\uf15c'; fileIconColor = getResolvedColor('--theme-accent-secondary', '#EC4899'); } // fa-file-alt (re-use for MD)
          
          nodeIcon = { face: defaultIconFace, code: fileIconCode, size: defaultIconSize, color: fileIconColor };
          break;
        default: 
          nodeShape = 'icon';
          nodeIcon = { face: defaultIconFace, code: '\uf128', size: defaultIconSize, color: defaultIconColor }; // fa-question-circle as fallback
      }

      nodesArr.push({
        id: item.id,
        label: item.name.length > 20 ? item.name.substring(0, 17) + '...' : item.name,
        title: `<b>${item.name}</b><br>Type: ${getSpecificFileTypeForGraph(item)}<br>Size: ${formatFileSize(item.fileSize)}<br>Added: ${new Date(item.createdAt).toLocaleDateString()}`,
        shape: nodeShape,
        group: nodeGroup,
        image: nodeImage,
        icon: nodeIcon, 
      });

      if (item.parentId) {
        edgesArr.push({
          from: item.parentId,
          to: item.id,
          arrows: 'to',
        });
      }
    });

    const nodes = new DataSet(nodesArr);
    const edges = new DataSet(edgesArr);
    
    const data = { nodes, edges };
    const options = {
      layout: {
        randomSeed: undefined,
        improvedLayout: true,
         hierarchical: { enabled: false }
      },
      edges: {
        color: getResolvedColor('--theme-text-secondary', '#888888'),
        smooth: { 
            enabled: true,
            type: 'dynamic',
            roundness: 0.5
        },
        arrows: { to: { enabled: true, scaleFactor: 0.6 } },
        length: 200,
      },
      nodes: { // General node properties, icon-specific props are set on node itself.
        borderWidth: 1,
        borderWidthSelected: 3,
        font: { color: getResolvedColor('--theme-text-primary', '#EEEEEE'), size: 12, face: 'Inter', strokeWidth: 0 },
      },
      groups: { // Primarily for non-icon shapes or broad category coloring if needed. Icon colors are per-node.
        [ItemType.FOLDER]: { color: { border: getResolvedColor('--theme-accent-secondary', '#C22400'), background: getResolvedColor('--theme-accent-secondary', '#C22400') } },
        [ItemType.TEXT]: { color: { border: getResolvedColor('--theme-info', '#0065A9'), background: getResolvedColor('--theme-info', '#0065A9') } },
        [ItemType.IMAGE]: { // Group for successful circularImage
            color: { border: getResolvedColor('--theme-success', '#007842'), background: getResolvedColor('--theme-success', '#007842') },
            shape: 'circularImage',
            brokenImage: 'https://via.placeholder.com/50/FF0000/FFFFFF?Text=ImgLoadErr'
        },
        'IMAGE_FALLBACK_ICON': { // Group for images displayed as icons
            color: { border: getResolvedColor('--theme-success', '#007842'), background: getResolvedColor('--theme-success', '#007842') }
        },
        [ItemType.GENERIC_FILE]: { color: { border: getResolvedColor('--theme-text-secondary', '#444444'), background: getResolvedColor('--theme-text-secondary', '#444444')} },
        [ItemType.VIDEO_URL]: { color: { border: getResolvedColor('--theme-accent-primary', '#FFA000'), background: getResolvedColor('--theme-accent-primary', '#FFA000') } },
        [ItemType.AUDIO_FILE]: { color: { border: getResolvedColor('--theme-accent-secondary', '#7B1FA2'), background: getResolvedColor('--theme-accent-secondary', '#7B1FA2') } },
        [ItemType.RSS_FEED_URL]: { color: { border: getResolvedColor('--theme-accent-primary', '#F57C00'), background: getResolvedColor('--theme-accent-primary', '#F57C00') } },
      },
      physics: {
        enabled: true,
        solver: 'barnesHut',
        barnesHut: {
          gravitationalConstant: -20000, // Adjusted for potentially more nodes
          centralGravity: 0.15,
          springLength: 150,
          springConstant: 0.06,
          damping: 0.1,
          avoidOverlap: 0.3
        },
        stabilization: { iterations: 1500, fit: true },
      },
      interaction: {
        hover: true,
        tooltipDelay: 300,
        navigationButtons: false,
        keyboard: true,
        dragNodes: true,
        dragView: true,
        zoomView: true
      },
    };

    if (networkInstanceRef.current) {
      networkInstanceRef.current.destroy();
    }
    
    const network = new Network(graphRef.current, data, options);
    networkInstanceRef.current = network;

    network.on('stabilizationIterationsDone', () => {
      network.fit(); 
      setIsLoading(false);
    });
    
    const loadingTimeout = setTimeout(() => {
        if (isLoading) { // Check if still loading after timeout
          setIsLoading(false);
          if (networkInstanceRef.current) networkInstanceRef.current.fit();
        }
    }, 5000);


    network.on('click', (params) => {
      if (params.nodes.length > 0) {
        const nodeId = params.nodes[0];
        const clickedItem = ragItems.find(item => item.id === nodeId);
        setSelectedNodeInfo(clickedItem || null);
      } else {
        setSelectedNodeInfo(null);
      }
    });
    
    return () => {
      clearTimeout(loadingTimeout);
      networkInstanceRef.current?.destroy();
      networkInstanceRef.current = null;
    };

  }, [ragItems, dashboardSettings, isLoading, addNotification]); // Added addNotification & isLoading to dependencies

  const handleRefreshGraph = () => {
    setIsLoading(true);
    setSelectedNodeInfo(null);
    loadRAGItems(); 
  };

  return (
    <div className="p-4 sm:p-6 h-full flex flex-col space-y-4">
      <header className="text-center">
        <h1 className="text-3xl sm:text-4xl font-bold text-transparent bg-clip-text bg-gradient-to-r from-purple-400 to-pink-500">
          Knowledge Graph Explorer
        </h1>
        <p className="text-slate-400 mt-1 text-sm sm:text-md">
          Visualize connections within your OmniHoarder vault. (Parent-Child View)
        </p>
      </header>

      <div className="flex justify-end mb-2">
          <button 
            onClick={handleRefreshGraph} 
            className="px-3 py-1.5 bg-[var(--theme-accent-primary)] hover:opacity-80 text-[var(--theme-button-primary-text)] text-xs font-semibold rounded-md shadow"
            disabled={isLoading}
          >
            {isLoading ? <Spinner size="w-4 h-4 inline mr-1" /> : null} Refresh Graph
          </button>
      </div>

      <div className="flex-grow grid grid-cols-1 lg:grid-cols-3 gap-4 min-h-0">
        <div className={`lg:col-span-2 bg-[var(--theme-card-bg)] rounded-xl shadow-xl border border-[var(--theme-border-primary)] relative overflow-hidden ${isLoading ? 'flex items-center justify-center' : ''}`}>
          {isLoading && <Spinner size="w-12 h-12" color="text-[var(--theme-accent-primary)]" />}
          <div ref={graphRef} className="w-full h-[600px] lg:h-full" style={{ visibility: isLoading ? 'hidden' : 'visible' }}></div>
        </div>
        
        <div className="lg:col-span-1 bg-[var(--theme-card-bg)] p-4 rounded-xl shadow-xl border border-[var(--theme-border-primary)] overflow-y-auto max-h-[600px] lg:max-h-none">
          <h2 className="text-lg font-semibold text-pink-400 mb-3 pb-2 border-b border-[var(--theme-border-primary)]">Selected Item Details</h2>
          {selectedNodeInfo ? (
            <div className="text-xs space-y-1.5">
              <p><strong>Name:</strong> <span className="text-slate-300">{selectedNodeInfo.name}</span></p>
              <p><strong>Type:</strong> <span className="text-slate-300">{getSpecificFileTypeForGraph(selectedNodeInfo)}</span></p>
              <p><strong>ID:</strong> <span className="text-slate-400 text-[10px] break-all">{selectedNodeInfo.id}</span></p>
              {selectedNodeInfo.parentId && <p><strong>Parent ID:</strong> <span className="text-slate-400 text-[10px] break-all">{selectedNodeInfo.parentId}</span></p>}
              <p><strong>Added:</strong> <span className="text-slate-300">{new Date(selectedNodeInfo.createdAt).toLocaleString()}</span></p>
              <p><strong>Size:</strong> <span className="text-slate-300">{formatFileSize(selectedNodeInfo.fileSize)}</span></p>
              
              {selectedNodeInfo.geminiAnalysis?.[GeminiActionType.SUMMARIZE] && (
                <div className="mt-2 pt-1.5 border-t border-slate-700">
                    <strong>AI Summary:</strong>
                    <p className="text-slate-300 bg-slate-700/50 p-1.5 rounded max-h-24 overflow-y-auto text-[11px] leading-snug">
                        {Array.isArray(selectedNodeInfo.geminiAnalysis[GeminiActionType.SUMMARIZE]) 
                            ? (selectedNodeInfo.geminiAnalysis[GeminiActionType.SUMMARIZE] as string[]).join('; ') 
                            : String(selectedNodeInfo.geminiAnalysis[GeminiActionType.SUMMARIZE])
                        }
                    </p>
                </div>
              )}
              {selectedNodeInfo.geminiAnalysis?.[GeminiActionType.TAG] && Array.isArray(selectedNodeInfo.geminiAnalysis[GeminiActionType.TAG]) && (selectedNodeInfo.geminiAnalysis[GeminiActionType.TAG] as string[]).length > 0 && (
                <div className="mt-2 pt-1.5 border-t border-slate-700">
                    <strong>AI Tags:</strong>
                    <div className="flex flex-wrap gap-1 mt-0.5">
                        {(selectedNodeInfo.geminiAnalysis[GeminiActionType.TAG] as string[]).map(tag => (
                            <span key={tag} className="px-1.5 py-0.5 bg-teal-600 text-teal-100 text-[10px] rounded-full">{tag}</span>
                        ))}
                    </div>
                </div>
              )}
               <button 
                onClick={() => addNotification('info', 'Action: "Open in RAG Agent" - (Not fully implemented yet, would navigate and focus item).')}
                className="mt-3 w-full px-2 py-1 bg-sky-600 hover:bg-sky-700 text-white text-[11px] font-semibold rounded-md shadow"
                >
                Open in RAG Agent (Placeholder)
              </button>
            </div>
          ) : (
            <p className="text-slate-400 italic text-center py-8">Click on a node in the graph to see its details.</p>
          )}
        </div>
      </div>
    </div>
  );
};

export default KnowledgeGraphExplorerFeature;
