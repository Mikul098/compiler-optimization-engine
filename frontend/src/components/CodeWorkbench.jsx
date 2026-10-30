import React, { useState } from 'react';
import { Play, Copy, RotateCcw, Sparkles, Check, FileCode, Terminal } from 'lucide-react';
import { TEST_SUITES } from '../data/testSuites';

export default function CodeWorkbench({ code, setCode, onRun, isCompiling, onLoadSuite, selectedSuiteId }) {
  const [copied, setCopied] = useState(false);

  const lines = code.split('\n');

  function handleCopy() {
    navigator.clipboard.writeText(code);
    setCopied(true);
    setTimeout(() => setCopied(false), 2000);
  }

  function handleReset() {
    const current = TEST_SUITES.find(s => s.id === selectedSuiteId) || TEST_SUITES[0];
    setCode(current.code);
  }

  return (
    <div className="workbench-card">
      {/* Workbench Header */}
      <div className="workbench-header">
        <div className="workbench-title-group">
          <div className="workbench-icon-badge">
            <FileCode size={16} />
          </div>
          <div>
            <h3 className="workbench-heading">INTERACTIVE SOURCE CODE WORKBENCH</h3>
            <span className="workbench-subheading">Toy Language Input Buffer • Dynamic Flex / Bison Grammar Parser</span>
          </div>
        </div>

        <div className="workbench-actions">
          <select
            className="suite-dropdown"
            value={selectedSuiteId}
            onChange={(e) => {
              const suite = TEST_SUITES.find(s => s.id === e.target.value);
              if (suite) onLoadSuite(suite);
            }}
          >
            <option disabled>Select Benchmark Test Suite</option>
            {TEST_SUITES.map(s => (
              <option key={s.id} value={s.id}>{s.name}</option>
            ))}
          </select>

          <button className="secondary-action-btn" onClick={handleCopy} title="Copy code">
            {copied ? <Check size={13} className="text-emerald" /> : <Copy size={13} />}
            <span>{copied ? 'Copied' : 'Copy'}</span>
          </button>

          <button className="secondary-action-btn" onClick={handleReset} title="Reset code">
            <RotateCcw size={13} />
            <span>Reset</span>
          </button>

          <button
            className={`primary-run-btn ${isCompiling ? 'loading' : ''}`}
            onClick={onRun}
            disabled={isCompiling}
          >
            <Play size={14} fill="currentColor" />
            <span>{isCompiling ? 'Running Pipeline...' : 'Run Pipeline & Optimize'}</span>
          </button>
        </div>
      </div>

      {/* Quick Test Chips */}
      <div className="workbench-chips-row">
        <span className="chips-title">Quick Benchmarks:</span>
        {TEST_SUITES.slice(0, 6).map(s => (
          <button
            key={`chip-${s.id}`}
            className={`chip-btn ${selectedSuiteId === s.id ? 'active' : ''}`}
            onClick={() => onLoadSuite(s)}
          >
            {s.name.split(':')[0]}
          </button>
        ))}
      </div>

      {/* Editor Area with Line Numbers */}
      <div className="editor-container">
        <div className="line-numbers-gutter">
          {lines.map((_, i) => (
            <div key={`line-num-${i}`} className="line-num">{i + 1}</div>
          ))}
        </div>

        <div className="textarea-wrapper">
          <textarea
            className="code-textarea"
            value={code}
            onChange={(e) => setCode(e.target.value)}
            spellCheck="false"
            autoCapitalize="off"
            autoComplete="off"
            placeholder="// Enter Toy Language source code here...&#10;a = 10;&#10;b = 20;&#10;c = a * b;&#10;print c;"
            rows={Math.max(14, lines.length + 2)}
          />
        </div>
      </div>

      {/* Editor Footer Status Bar */}
      <div className="workbench-footer">
        <div className="footer-left">
          <span className="footer-stat">{lines.length} Lines</span>
          <span className="footer-stat">{code.length} Characters</span>
          <span className="footer-stat">Target: Three-Address Code</span>
        </div>
        <div className="footer-right">
          <span className="status-pill-ready">
            <span className="status-dot-green" /> Ready to Compile
          </span>
        </div>
      </div>
    </div>
  );
}
