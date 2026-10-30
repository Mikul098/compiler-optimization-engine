import React, { useState } from 'react';
import { ShieldCheck, ShieldAlert, Terminal, Database, Activity, CheckCircle, Clock } from 'lucide-react';

export default function VMConsole({ verification, elapsed }) {
  const [activeTab, setActiveTab] = useState('registers'); // 'registers' | 'logs'

  if (!verification) {
    return (
      <div className="vm-empty-state">
        <p>Run the compiler pipeline to execute the virtual machine verification.</p>
      </div>
    );
  }

  const { passed, unoptimizedOutput, optimizedOutput, memory, memoryLogs } = verification;
  const memoryEntries = Object.entries(memory || {});

  return (
    <div className="vm-console-card">
      {/* Top Status Banner */}
      <div className={`vm-status-banner ${passed ? 'banner-pass' : 'banner-fail'}`}>
        <div className="status-icon-badge">
          {passed ? <ShieldCheck size={28} /> : <ShieldAlert size={28} />}
        </div>
        <div className="status-info">
          <div className="status-title-row">
            <h3>{passed ? '[PASS] Semantic Verification Validated' : '[FAIL] Semantic Mismatch Detected'}</h3>
            <span className="status-pill">{passed ? 'Equivalence Confirmed' : 'Logic Broken'}</span>
          </div>
          <p className="status-subtext">
            {passed
              ? 'The virtual machine executed both unoptimized and optimized Three-Address Code streams independently and observed identical stdout side-effects.'
              : 'The optimized instruction stream produced divergent output values compared to the unoptimized baseline.'}
          </p>
        </div>

        <div className="status-metrics">
          <div className="sub-metric">
            <span className="sub-label">EXECUTION TIME</span>
            <span className="sub-val"><Clock size={12} /> {elapsed || 0} ms</span>
          </div>
          <div className="sub-metric">
            <span className="sub-label">OUTPUT PARITY</span>
            <span className="sub-val text-emerald">{passed ? '100.0% Match' : 'Divergent'}</span>
          </div>
        </div>
      </div>

      {/* Output Streams Comparison */}
      <div className="vm-output-comparison-grid">
        <div className="stream-box">
          <div className="stream-header">
            <Terminal size={14} />
            <span>Unoptimized TAC Virtual Machine Output</span>
          </div>
          <div className="stream-body">
            {unoptimizedOutput && unoptimizedOutput.length > 0 ? (
              unoptimizedOutput.map((val, idx) => (
                <div key={`unopt-out-${idx}`} className="stream-line">
                  <span className="stream-prompt">&gt;</span>
                  <span className="stream-label">Program Output:</span>
                  <span className="stream-val">{val}</span>
                </div>
              ))
            ) : (
              <span className="stream-empty">No print statements executed.</span>
            )}
          </div>
        </div>

        <div className="stream-box highlight">
          <div className="stream-header">
            <Terminal size={14} className="text-emerald" />
            <span>Optimized TAC Virtual Machine Output</span>
            <span className="parity-badge"><CheckCircle size={12} /> Parity Verified</span>
          </div>
          <div className="stream-body">
            {optimizedOutput && optimizedOutput.length > 0 ? (
              optimizedOutput.map((val, idx) => (
                <div key={`opt-out-${idx}`} className="stream-line">
                  <span className="stream-prompt">&gt;</span>
                  <span className="stream-label">Program Output:</span>
                  <span className="stream-val text-emerald font-bold">{val}</span>
                </div>
              ))
            ) : (
              <span className="stream-empty">No print statements executed.</span>
            )}
          </div>
        </div>
      </div>

      {/* Tabs for Memory Registers vs Stepper Log */}
      <div className="vm-details-section">
        <div className="vm-tab-bar">
          <button
            className={`vm-tab ${activeTab === 'registers' ? 'active' : ''}`}
            onClick={() => setActiveTab('registers')}
          >
            <Database size={13} /> Runtime Memory Map ({memoryEntries.length} Variables)
          </button>
          <button
            className={`vm-tab ${activeTab === 'logs' ? 'active' : ''}`}
            onClick={() => setActiveTab('logs')}
          >
            <Activity size={13} /> Instruction Stepper Log ({memoryLogs?.length || 0} Cycles)
          </button>
        </div>

        {activeTab === 'registers' ? (
          <div className="vm-registers-table-wrap">
            <table className="vm-registers-table">
              <thead>
                <tr>
                  <th>Symbol / Register</th>
                  <th>Decimal Value</th>
                  <th>Hex Representation</th>
                  <th>Data Type</th>
                  <th>Storage Class</th>
                </tr>
              </thead>
              <tbody>
                {memoryEntries.map(([reg, val]) => (
                  <tr key={`reg-${reg}`}>
                    <td>
                      <code className="reg-name">{reg}</code>
                    </td>
                    <td>
                      <span className="reg-val">{val}</span>
                    </td>
                    <td>
                      <code className="reg-hex">
                        0x{Math.abs(val).toString(16).padStart(8, '0').toUpperCase()}
                      </code>
                    </td>
                    <td>
                      <span className="reg-type">int32</span>
                    </td>
                    <td>
                      <span className="reg-alloc">Virtual Register Frame</span>
                    </td>
                  </tr>
                ))}
              </tbody>
            </table>
          </div>
        ) : (
          <div className="vm-logs-list">
            {(memoryLogs || []).map((step, idx) => (
              <div key={`log-${idx}`} className="vm-log-entry">
                <span className="log-cycle">Cycle #{step.step}</span>
                <code className="log-instr">{step.instruction}</code>
                <span className="log-reg-count">
                  Memory State: {Object.keys(step.registers).length} variables allocated
                </span>
              </div>
            ))}
          </div>
        )}
      </div>
    </div>
  );
}
