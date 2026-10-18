import React, { useState } from 'react';
import { ArrowRight, Check, Trash2, Zap, BarChart2 } from 'lucide-react';

export default function DiffMatrix({ diff }) {
  const [filter, setFilter] = useState('all'); // 'all' | 'eliminated' | 'surviving'

  if (!diff) {
    return (
      <div className="diff-empty-state">
        <p>No differential data generated yet.</p>
      </div>
    );
  }

  const { beforeCount, afterCount, reduced, percentage, rows } = diff;

  const filteredRows = rows.filter(r => {
    if (filter === 'eliminated') return r.isEliminated;
    if (filter === 'surviving') return r.afterStr !== '';
    return true;
  });

  return (
    <div className="diff-matrix-card">
      {/* Metrics Banner */}
      <div className="diff-metrics-banner">
        <div className="metric-cell">
          <span className="metric-label">INITIAL INSTRUCTION COUNT</span>
          <span className="metric-value before">{beforeCount}</span>
          <span className="metric-caption">Unoptimized Quad Lowering</span>
        </div>

        <div className="metric-cell">
          <span className="metric-label">FINAL INSTRUCTION COUNT</span>
          <span className="metric-value after">{afterCount}</span>
          <span className="metric-caption">Post Fixed-Point Iteration</span>
        </div>

        <div className="metric-cell">
          <span className="metric-label">TOTAL REDUCED</span>
          <span className="metric-value reduced">-{reduced}</span>
          <span className="metric-caption">Dead / Folded Quads</span>
        </div>

        <div className="metric-cell highlight">
          <span className="metric-label">REDUCTION EFFICIENCY</span>
          <div className="metric-percentage-gauge">
            <span className="metric-value percentage">{percentage}%</span>
            <div className="percentage-bar-bg">
              <div className="percentage-bar-fill" style={{ width: `${Math.min(100, Math.max(0, percentage))}%` }} />
            </div>
          </div>
          <span className="metric-caption">Code Footprint Compaction</span>
        </div>
      </div>

      {/* Filter Tabs */}
      <div className="diff-filter-bar">
        <div className="diff-filter-left">
          <span className="filter-title">Filter Rows:</span>
          <button
            className={`filter-btn ${filter === 'all' ? 'active' : ''}`}
            onClick={() => setFilter('all')}
          >
            All Instructions ({rows.length})
          </button>
          <button
            className={`filter-btn ${filter === 'eliminated' ? 'active' : ''}`}
            onClick={() => setFilter('eliminated')}
          >
            <Trash2 size={12} /> Pruned / Folded ({rows.filter(r => r.isEliminated).length})
          </button>
          <button
            className={`filter-btn ${filter === 'surviving' ? 'active' : ''}`}
            onClick={() => setFilter('surviving')}
          >
            <Check size={12} /> Surviving Quads ({rows.filter(r => r.afterStr !== '').length})
          </button>
        </div>

        <div className="diff-legend">
          <span className="legend-item"><span className="legend-dot red" /> Pruned in DCE / Folded</span>
          <span className="legend-item"><span className="legend-dot green" /> Final Optimized IR</span>
        </div>
      </div>

      {/* Side-by-Side Table */}
      <div className="diff-table-container">
        <table className="diff-table">
          <thead>
            <tr>
              <th className="col-num">#</th>
              <th className="col-before">UNOPTIMIZED TAC (Before State)</th>
              <th className="col-arrow"></th>
              <th className="col-after">OPTIMIZED TAC (After Pass Manager)</th>
              <th className="col-status">Status</th>
            </tr>
          </thead>
          <tbody>
            {filteredRows.map((row) => {
              const isDead = row.isEliminated;
              const isAlive = row.afterStr !== '';
              return (
                <tr key={`diff-row-${row.index}`} className={`diff-row ${isDead ? 'row-dead' : 'row-alive'}`}>
                  <td className="cell-num">{row.index}</td>
                  <td className="cell-before">
                    {row.beforeStr ? (
                      <code className={`tac-code before-tac ${isDead ? 'strikethrough' : ''}`}>
                        {row.beforeStr}
                      </code>
                    ) : (
                      <span className="tac-empty">—</span>
                    )}
                  </td>
                  <td className="cell-arrow">
                    <ArrowRight size={13} className="arrow-icon" />
                  </td>
                  <td className="cell-after">
                    {row.afterStr ? (
                      <code className="tac-code after-tac">
                        {row.afterStr}
                      </code>
                    ) : (
                      <span className="tac-empty pruned">[Eliminated by Pass Manager]</span>
                    )}
                  </td>
                  <td className="cell-status">
                    {isDead ? (
                      <span className="badge-status dead">Eliminated</span>
                    ) : (
                      <span className="badge-status alive">Surviving</span>
                    )}
                  </td>
                </tr>
              );
            })}
          </tbody>
        </table>
      </div>
    </div>
  );
}
