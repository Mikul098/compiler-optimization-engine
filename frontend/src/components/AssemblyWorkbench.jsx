import React, { useState } from 'react';
import { Cpu, Terminal, Layers, CheckCircle2, AlertTriangle, ArrowRight, ShieldCheck, HardDrive } from 'lucide-react';

export default function AssemblyWorkbench({ pipelineData }) {
  const [activeSubTab, setActiveSubTab] = useState('asm'); // 'asm' | 'regalloc' | 'cfg'

  if (!pipelineData || !pipelineData.success) {
    return (
      <div className="vm-empty-state">
        <p>Run the compiler pipeline to view generated x86-64 assembly and register allocations.</p>
      </div>
    );
  }

  const { x86Assembly, regAlloc, cfg, optimizedTAC } = pipelineData;
  const regMapEntries = Object.entries(regAlloc?.registerMap || {});
  const spillEntries = Object.entries(regAlloc?.stackSpills || {});

  return (
    <div className="workbench-card">
      {/* Top Banner */}
      <div className="assembly-header-banner">
        <div className="banner-left">
          <div className="assembly-icon-box">
            <Cpu size={24} />
          </div>
          <div>
            <h3 className="assembly-title">Target Machine Code Generation &amp; Hardware Register Allocation</h3>
            <p className="assembly-sub">
              Direct translation from optimized Three-Address Code (TAC) into native x86-64 Intel assembly with Chaitin-Briggs K-coloring.
            </p>
          </div>
        </div>

        <div className="banner-badges">
          <div className="stat-pill">
            <span className="pill-k">ABI</span>
            <span className="pill-v">System V / Win x64</span>
          </div>
          <div className="stat-pill">
            <span className="pill-k">STACK FRAME</span>
            <span className="pill-v">{regAlloc?.stackFrameBytes || 48} Bytes</span>
          </div>
          <div className="stat-pill">
            <span className="pill-k">K REGISTERS</span>
            <span className="pill-v">{regAlloc?.kRegisters || 6} Pool</span>
          </div>
        </div>
      </div>

      {/* Sub Navigation */}
      <div className="assembly-sub-nav">
        <button
          className={`sub-nav-btn ${activeSubTab === 'asm' ? 'active' : ''}`}
          onClick={() => setActiveSubTab('asm')}
        >
          <Cpu size={13} /> Native x86-64 Assembly
        </button>
        <button
          className={`sub-nav-btn ${activeSubTab === 'regalloc' ? 'active' : ''}`}
          onClick={() => setActiveSubTab('regalloc')}
        >
          <HardDrive size={13} /> Chaitin Register Map ({regMapEntries.length} Allocated)
        </button>
        <button
          className={`sub-nav-btn ${activeSubTab === 'cfg' ? 'active' : ''}`}
          onClick={() => setActiveSubTab('cfg')}
        >
          <Layers size={13} /> Control Flow Graph ({cfg?.blocks?.length || 0} Blocks)
        </button>
      </div>

      {/* SubTab 1: Assembly Side-by-Side */}
      {activeSubTab === 'asm' && (
        <div className="asm-split-view">
          <div className="asm-column">
            <div className="asm-col-header">
              <Terminal size={14} />
              <span>Optimized Intermediate IR (Three-Address Code)</span>
            </div>
            <div className="asm-code-body">
              {optimizedTAC && optimizedTAC.map((ins, i) => (
                <div key={i} className="code-line-row">
                  <span className="line-num">{i + 1}</span>
                  <span className="line-text">
                    {ins.op === 'LABEL' ? `${ins.arg1}:` :
                     ins.op === 'ASSIGN' ? `${ins.result} = ${ins.arg1}` :
                     ins.op === 'PRINT' ? `print ${ins.arg1}` :
                     ins.op === 'GOTO' ? `goto ${ins.arg1}` :
                     ins.op === 'IF_ZERO' ? `if ${ins.arg1} == 0 goto ${ins.arg2}` :
                     `${ins.result} = ${ins.arg1} ${ins.op.toLowerCase()} ${ins.arg2}`}
                  </span>
                </div>
              ))}
            </div>
          </div>

          <div className="asm-column asm-target-column">
            <div className="asm-col-header">
              <Cpu size={14} />
              <span>Emitted x86-64 Target Assembly (.intel_syntax noprefix)</span>
            </div>
            <pre className="asm-code-body text-emerald font-mono">
              {x86Assembly || '; No assembly generated'}
            </pre>
          </div>
        </div>
      )}

      {/* SubTab 2: Register Allocation Matrix */}
      {activeSubTab === 'regalloc' && (
        <div className="regalloc-view">
          <div className="regalloc-stats-row">
            <div className="reg-stat-card">
              <span className="stat-num">{regMapEntries.length}</span>
              <span className="stat-desc">Variables Allocated to Physical Registers</span>
            </div>
            <div className="reg-stat-card">
              <span className="stat-num">{spillEntries.length}</span>
              <span className="stat-desc">Variables Spilled to Memory Stack Slots</span>
            </div>
            <div className="reg-stat-card">
              <span className="stat-num">{regAlloc?.kRegisters || 6}</span>
              <span className="stat-desc">Chromatic Number K (Register Capacity)</span>
            </div>
          </div>

          <div className="reg-table-container">
            <table className="reg-table">
              <thead>
                <tr>
                  <th>Virtual Variable / Temporary</th>
                  <th>Assigned Hardware Location</th>
                  <th>Storage Type</th>
                  <th>Hardware ABI Register Role</th>
                </tr>
              </thead>
              <tbody>
                {regMapEntries.map(([v, r]) => (
                  <tr key={v}>
                    <td><code>{v}</code></td>
                    <td><strong className="text-emerald">%{r}</strong></td>
                    <td><span className="badge-reg">CPU Register</span></td>
                    <td>General Purpose 64-bit Core Register</td>
                  </tr>
                ))}
                {spillEntries.map(([v, offset]) => (
                  <tr key={v}>
                    <td><code>{v}</code></td>
                    <td><strong className="text-amber">[rbp - {Math.abs(offset)}]</strong></td>
                    <td><span className="badge-spill">Stack Spill</span></td>
                    <td>Function Stack Frame Memory Slot</td>
                  </tr>
                ))}
                {regMapEntries.length === 0 && spillEntries.length === 0 && (
                  <tr>
                    <td colSpan="4" style={{ textAlign: 'center', padding: '16px' }}>
                      No active variables in intermediate state (folded to constants).
                    </td>
                  </tr>
                )}
              </tbody>
            </table>
          </div>
        </div>
      )}

      {/* SubTab 3: CFG Basic Blocks */}
      {activeSubTab === 'cfg' && (
        <div className="cfg-view">
          <div className="cfg-blocks-grid">
            {cfg?.blocks?.map((b) => (
              <div key={b.id} className="cfg-block-card">
                <div className="cfg-block-header">
                  <span>Basic Block B{b.id}</span>
                  {b.label && <span className="block-label-pill">{b.label}</span>}
                </div>
                <div className="cfg-block-meta">
                  <span>Preds: {b.preds.length ? b.preds.map(p => `B${p}`).join(', ') : 'Entry'}</span>
                  <span>Succs: {b.succs.length ? b.succs.map(s => `B${s}`).join(', ') : 'Exit'}</span>
                  <span>Dom: {b.dominators ? b.dominators.map(d => `B${d}`).join(', ') : `B${b.id}`}</span>
                </div>
                <div className="cfg-block-instrs">
                  {b.instructions.map((ins, idx) => (
                    <div key={idx} className="cfg-instr-row">
                      {ins.op === 'LABEL' ? `${ins.arg1}:` :
                       ins.op === 'ASSIGN' ? `${ins.result} = ${ins.arg1}` :
                       ins.op === 'PRINT' ? `print ${ins.arg1}` :
                       `${ins.result} = ${ins.arg1} ${ins.op.toLowerCase()} ${ins.arg2}`}
                    </div>
                  ))}
                </div>
              </div>
            ))}
          </div>
        </div>
      )}
    </div>
  );
}
