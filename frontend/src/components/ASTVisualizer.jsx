import React, { useState, useMemo } from 'react';
import { ZoomIn, ZoomOut, RotateCcw, Image, Network, Info, CheckCircle2 } from 'lucide-react';

const NODE_COLORS = {
  root: { bg: '#4f46e5', border: '#4338ca', text: '#ffffff' },
  assign: { bg: '#0284c7', border: '#0369a1', text: '#ffffff' },
  print: { bg: '#059669', border: '#047857', text: '#ffffff' },
  op: { bg: '#d97706', border: '#b45309', text: '#ffffff' },
  var: { bg: '#0891b2', border: '#0e7490', text: '#ffffff' },
  num: { bg: '#e11d48', border: '#be123c', text: '#ffffff' },
};

function computeLayout(node, depth = 0, nextX = { val: 0 }) {
  const children = (node.children || []).map(child => computeLayout(child, depth + 1, nextX));
  let x = 0;
  if (children.length > 0) {
    x = children.reduce((sum, c) => sum + c.x, 0) / children.length;
  } else {
    x = nextX.val;
    nextX.val += 180;
  }
  const y = depth * 90 + 50;
  return { ...node, x, y, children, depth };
}

export default function ASTVisualizer({ astGraph, astImage }) {
  const [zoom, setZoom] = useState(1);
  const [selectedNode, setSelectedNode] = useState(null);
  const [viewMode, setViewMode] = useState('interactive'); // 'interactive' | 'raster'

  const layoutTree = useMemo(() => {
    if (!astGraph) return null;
    return computeLayout(astGraph);
  }, [astGraph]);

  const { minX, maxX, maxY } = useMemo(() => {
    if (!layoutTree) return { minX: 0, maxX: 800, maxY: 400 };
    let min = Infinity, max = -Infinity, yMax = 0;
    function walk(n) {
      if (n.x < min) min = n.x;
      if (n.x > max) max = n.x;
      if (n.y > yMax) yMax = n.y;
      (n.children || []).forEach(walk);
    }
    walk(layoutTree);
    return { minX: min - 120, maxX: max + 120, maxY: yMax + 100 };
  }, [layoutTree]);

  const svgWidth = Math.max(900, maxX - minX);
  const svgHeight = Math.max(450, maxY);

  function renderEdges(node) {
    if (!node.children || node.children.length === 0) return null;
    return node.children.map(child => {
      const pathD = `M ${node.x - minX} ${node.y} C ${node.x - minX} ${node.y + 45}, ${child.x - minX} ${child.y - 45}, ${child.x - minX} ${child.y}`;
      return (
        <g key={`edge-${node.id}-${child.id}`}>
          <path
            d={pathD}
            fill="none"
            stroke="var(--ast-edge, #cbd5e1)"
            strokeWidth="2"
            strokeDasharray="4 2"
          />
          {renderEdges(child)}
        </g>
      );
    });
  }

  function renderNodes(node) {
    const styling = NODE_COLORS[node.type] || NODE_COLORS.root;
    const isSelected = selectedNode?.id === node.id;
    return (
      <g key={`node-${node.id}`}>
        <g
          transform={`translate(${node.x - minX}, ${node.y})`}
          onClick={() => setSelectedNode(node)}
          style={{ cursor: 'pointer' }}
        >
          <rect
            x="-70"
            y="-20"
            width="140"
            height="40"
            rx="20"
            fill={styling.bg}
            stroke={isSelected ? '#fde047' : styling.border}
            strokeWidth={isSelected ? 3 : 1.5}
            filter="drop-shadow(0 2px 4px rgba(0,0,0,0.12))"
          />
          <text
            x="0"
            y="4"
            textAnchor="middle"
            fill={styling.text}
            fontSize="11.5"
            fontWeight="600"
            fontFamily="var(--font-mono)"
          >
            {node.name.length > 18 ? node.name.substring(0, 16) + '...' : node.name}
          </text>
        </g>
        {(node.children || []).map(renderNodes)}
      </g>
    );
  }

  return (
    <div className="ast-visualizer-container">
      {/* Controls Bar */}
      <div className="ast-toolbar">
        <div className="ast-toolbar-left">
          <button
            className={`ast-toggle-btn ${viewMode === 'interactive' ? 'active' : ''}`}
            onClick={() => setViewMode('interactive')}
          >
            <Network size={14} /> Interactive Graph
          </button>
          {astImage && (
            <button
              className={`ast-toggle-btn ${viewMode === 'raster' ? 'active' : ''}`}
              onClick={() => setViewMode('raster')}
            >
              <Image size={14} /> High-Res ast.png
            </button>
          )}
        </div>

        <div className="ast-toolbar-right">
          <button className="ast-ctrl-btn" onClick={() => setZoom(z => Math.min(z + 0.15, 2))}>
            <ZoomIn size={14} />
          </button>
          <span className="ast-zoom-label">{Math.round(zoom * 100)}%</span>
          <button className="ast-ctrl-btn" onClick={() => setZoom(z => Math.max(z - 0.15, 0.5))}>
            <ZoomOut size={14} />
          </button>
          <button className="ast-ctrl-btn" onClick={() => { setZoom(1); setSelectedNode(null); }}>
            <RotateCcw size={14} />
          </button>
        </div>
      </div>

      {/* Visual Canvas */}
      <div className="ast-canvas-viewport">
        {viewMode === 'interactive' ? (
          layoutTree ? (
            <div
              className="ast-svg-wrapper"
              style={{ transform: `scale(${zoom})`, transformOrigin: 'top center', transition: 'transform 0.2s cubic-bezier(0.4, 0, 0.2, 1)' }}
            >
              <svg width={svgWidth} height={svgHeight} className="ast-svg">
                {renderEdges(layoutTree)}
                {renderNodes(layoutTree)}
              </svg>
            </div>
          ) : (
            <div className="ast-empty-state">
              <Info size={32} />
              <p>No Abstract Syntax Tree generated yet. Run compilation above to view hierarchy.</p>
            </div>
          )
        ) : (
          <div className="ast-raster-view">
            <img src={astImage} alt="AST Visualization" className="ast-raster-img" />
            <div className="ast-raster-caption">
              Direct high-resolution render produced by compiler via Matplotlib (<code>scripts/render_ast.py</code>).
            </div>
          </div>
        )}
      </div>

      {/* Node Details Inspection Bar */}
      {selectedNode && (
        <div className="ast-node-inspector">
          <div className="ast-inspector-title">
            <CheckCircle2 size={14} className="text-emerald" />
            <span>AST Node Inspector</span>
          </div>
          <div className="ast-inspector-tags">
            <span className="ast-tag">
              Type: <strong>{selectedNode.type.toUpperCase()}</strong>
            </span>
            <span className="ast-tag">
              Depth: <strong>Layer {selectedNode.depth}</strong>
            </span>
            <span className="ast-tag">
              Children: <strong>{selectedNode.children?.length || 0}</strong>
            </span>
          </div>
          <div className="ast-inspector-desc">
            Label: <code>{selectedNode.name}</code>
          </div>
        </div>
      )}
    </div>
  );
}
