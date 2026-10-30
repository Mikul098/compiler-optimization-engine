import React, { useState, useEffect } from 'react';
import { Moon, Sun, Printer, Play, CheckCircle2, Layers, Cpu, GitCompare, ShieldCheck, BarChart3, FileCode, AlertTriangle } from 'lucide-react';
import { TEST_SUITES } from './data/testSuites';
import { runFullPipeline } from './services/compilerEngine';
import ExecutiveArchitecture from './components/ExecutiveArchitecture';
import CodeWorkbench from './components/CodeWorkbench';
import DiffMatrix from './components/DiffMatrix';
import PassInspector from './components/PassInspector';
import ASTVisualizer from './components/ASTVisualizer';
import VMConsole from './components/VMConsole';
import BenchmarkTelemetry from './components/BenchmarkTelemetry';
import AssemblyWorkbench from './components/AssemblyWorkbench';

export default function App() {
  const urlParams = typeof window !== 'undefined' ? new URLSearchParams(window.location.search) : null;
  const initialSection = urlParams?.get('section') || 'all';
  const initialTheme = urlParams?.get('theme') || 'light';
  const initialSuiteId = urlParams?.get('suite') || 'test11_final_boss';

  const [theme, setTheme] = useState(initialTheme);
  const [selectedSuiteId, setSelectedSuiteId] = useState(initialSuiteId);
  const [code, setCode] = useState(() => {
    const found = TEST_SUITES.find(s => s.id === initialSuiteId);
    return found ? found.code : TEST_SUITES[0].code;
  });
  const [pipelineData, setPipelineData] = useState(null);
  const [isCompiling, setIsCompiling] = useState(false);
  const [activeSection, setActiveSection] = useState(initialSection);

  // Toggle Theme
  useEffect(() => {
    document.documentElement.setAttribute('data-theme', theme);
  }, [theme]);

  // Execute Pipeline
  async function executePipeline(sourceToRun = code) {
    setIsCompiling(true);
    try {
      const data = await runFullPipeline(sourceToRun);
      setPipelineData(data);
    } catch (err) {
      console.error('Compiler pipeline failure:', err);
    } finally {
      setIsCompiling(false);
    }
  }

  // Load Initial Test Suite on Mount
  useEffect(() => {
    const targetSuite = TEST_SUITES.find(s => s.id === initialSuiteId) || TEST_SUITES[0];
    setCode(targetSuite.code);
    setSelectedSuiteId(targetSuite.id);
    executePipeline(targetSuite.code);
  }, [initialSuiteId]);

  function handleLoadSuite(suite) {
    setSelectedSuiteId(suite.id);
    setCode(suite.code);
    executePipeline(suite.code);
  }

  function handlePrint() {
    window.print();
  }

  return (
    <div className="dossier-layout">
      {/* Institutional Master Header (Matching Reference Image) */}
      <header className="institutional-banner">
        <div className="banner-micro-header">
          <span>DEPARTMENT OF COMPUTER SCIENCE &amp; ENGINEERING • PROJECT REVIEW MASTER EVALUATION</span>
          <span>AUTONOMOUS SYSTEMS LABORATORY</span>
        </div>

        <div className="banner-top-row">
          <div className="dossier-pill-badge">
            <span className="pill-dot" />
            <span>DUAL-NOVELTY MASTER DOSSIER • END-TO-END COMPILER OPTIMIZATION ENGINE</span>
          </div>

          <div className="banner-actions">
            <button className="theme-toggle-btn" onClick={() => setTheme(t => t === 'light' ? 'dark' : 'light')}>
              {theme === 'light' ? <Moon size={13} /> : <Sun size={13} />}
              <span>{theme === 'light' ? 'Obsidian Slate' : 'Archival Ivory'}</span>
            </button>
            <button className="print-btn" onClick={handlePrint} title="Print / Export Dossier">
              <Printer size={13} />
              <span>Export Dossier</span>
            </button>
          </div>
        </div>

        <h1 className="dossier-hero-title">
          Autonomous Compiler Optimization Engine: Master Engineering Dossier
        </h1>

        <p className="dossier-hero-subtitle">
          A Rigorous Multi-Pass Engineering Dossier of Three-Address Code Lowering, Fixed-Point Iterative Optimization, Differential Telemetry, and Virtual Machine Semantic Verification
        </p>
      </header>

      {/* Sticky Dossier Navigation Bar */}
      <nav className="dossier-nav-bar">
        <button
          className={`nav-tab-btn ${activeSection === 'all' ? 'active' : ''}`}
          onClick={() => setActiveSection('all')}
        >
          <Layers size={13} /> Full Dossier View
        </button>
        <button
          className={`nav-tab-btn ${activeSection === 'architecture' ? 'active' : ''}`}
          onClick={() => setActiveSection('architecture')}
        >
          1. Architecture
        </button>
        <button
          className={`nav-tab-btn ${activeSection === 'workbench' ? 'active' : ''}`}
          onClick={() => setActiveSection('workbench')}
        >
          2. Code Workbench
        </button>
        <button
          className={`nav-tab-btn ${activeSection === 'diff' ? 'active' : ''}`}
          onClick={() => setActiveSection('diff')}
        >
          3. Differential Matrix
        </button>
        <button
          className={`nav-tab-btn ${activeSection === 'passes' ? 'active' : ''}`}
          onClick={() => setActiveSection('passes')}
        >
          4. Pass Scrubber
        </button>
        <button
          className={`nav-tab-btn ${activeSection === 'ast' ? 'active' : ''}`}
          onClick={() => setActiveSection('ast')}
        >
          5. AST Visualizer
        </button>
        <button
          className={`nav-tab-btn ${activeSection === 'vm' ? 'active' : ''}`}
          onClick={() => setActiveSection('vm')}
        >
          6. VM Verification
        </button>
        <button
          className={`nav-tab-btn ${activeSection === 'telemetry' ? 'active' : ''}`}
          onClick={() => setActiveSection('telemetry')}
        >
          7. Benchmark Telemetry
        </button>
        <button
          className={`nav-tab-btn ${activeSection === 'codegen' ? 'active' : ''}`}
          onClick={() => setActiveSection('codegen')}
        >
          <Cpu size={13} /> 8. Target x86-64 Codegen
        </button>
      </nav>

      {/* Section 1: Executive Abstract & Compiler Pipeline Architecture */}
      {(activeSection === 'all' || activeSection === 'architecture') && (
        <section id="architecture">
          <ExecutiveArchitecture />
        </section>
      )}

      {/* Section 2: Interactive Source Code Workbench */}
      {(activeSection === 'all' || activeSection === 'workbench') && (
        <section id="workbench">
          <div className="section-header-block">
            <span className="section-num">2.</span>
            <h2 className="section-title">INTERACTIVE SOURCE CODE WORKBENCH &amp; LIVE PIPELINE</h2>
            <div className="section-line" />
          </div>
          <CodeWorkbench
            code={code}
            setCode={setCode}
            onRun={() => executePipeline(code)}
            isCompiling={isCompiling}
            onLoadSuite={handleLoadSuite}
            selectedSuiteId={selectedSuiteId}
          />
          {pipelineData?.success === false && (
            <div className="syntax-error-banner">
              <div className="error-banner-header">
                <AlertTriangle size={18} />
                <span>PARSER SYNTAX DIAGNOSTIC</span>
              </div>
              <ul className="error-list">
                {pipelineData.errors?.map((err, i) => (
                  <li key={i}>{err}</li>
                ))}
              </ul>
              {pipelineData.nativeStdout && (
                <pre className="error-native-stdout" style={{ marginTop: '8px', fontSize: '12px', color: 'var(--color-red)' }}>
                  {pipelineData.nativeStdout}
                </pre>
              )}
            </div>
          )}
        </section>
      )}

      {/* Section 3: Differential Matrix (DiffEngine) */}
      {(activeSection === 'all' || activeSection === 'diff') && (
        <section id="diff">
          <div className="section-header-block">
            <span className="section-num">3.</span>
            <h2 className="section-title">SIDE-BY-SIDE THREE-ADDRESS CODE (TAC) DIFFERENTIAL MATRIX</h2>
            <div className="section-line" />
          </div>
          <DiffMatrix diff={pipelineData?.diff} />
        </section>
      )}

      {/* Section 4: Pass Manager Fixed-Point Timeline */}
      {(activeSection === 'all' || activeSection === 'passes') && (
        <section id="passes">
          <div className="section-header-block">
            <span className="section-num">4.</span>
            <h2 className="section-title">PASS-BY-PASS FIXED-POINT ITERATION ENGINE TELEMETRY</h2>
            <div className="section-line" />
          </div>
          <PassInspector passHistory={pipelineData?.passHistory} />
        </section>
      )}

      {/* Section 5: Abstract Syntax Tree (AST) Explorer */}
      {(activeSection === 'all' || activeSection === 'ast') && (
        <section id="ast">
          <div className="section-header-block">
            <span className="section-num">5.</span>
            <h2 className="section-title">INTERACTIVE ABSTRACT SYNTAX TREE (AST) GRAPH VISUALIZER</h2>
            <div className="section-line" />
          </div>
          <ASTVisualizer
            astGraph={pipelineData?.astGraph}
            astImage={pipelineData?.astImage}
          />
        </section>
      )}

      {/* Section 6: Virtual Machine Semantic Verification */}
      {(activeSection === 'all' || activeSection === 'vm') && (
        <section id="vm">
          <div className="section-header-block">
            <span className="section-num">6.</span>
            <h2 className="section-title">VIRTUAL MACHINE (VM) SEMANTIC VERIFICATION CONSOLE</h2>
            <div className="section-line" />
          </div>
          <VMConsole
            verification={pipelineData?.verification}
            elapsed={pipelineData?.elapsed}
          />
        </section>
      )}

      {/* Section 7: 11-Suite Comprehensive Benchmark Telemetry Matrix */}
      {(activeSection === 'all' || activeSection === 'telemetry') && (
        <section id="telemetry">
          <div className="section-header-block">
            <span className="section-num">7.</span>
            <h2 className="section-title">CONSOLIDATED 11-QUERY BENCHMARK TELEMETRY</h2>
            <div className="section-line" />
          </div>
          <BenchmarkTelemetry
            onLoadTest={handleLoadSuite}
            currentTestId={selectedSuiteId}
          />
        </section>
      )}

      {/* Section 8: Target x86-64 Machine Code Generation & Register Allocation */}
      {(activeSection === 'all' || activeSection === 'codegen') && (
        <section id="codegen">
          <div className="section-header-block">
            <span className="section-num">8.</span>
            <h2 className="section-title">TARGET x86-64 MACHINE CODE GENERATION &amp; REGISTER ALLOCATION</h2>
            <div className="section-line" />
          </div>
          <AssemblyWorkbench pipelineData={pipelineData} />
        </section>
      )}

      {/* Institutional Footer */}
      <footer className="dossier-footer">
        <div className="footer-dept-seal">
          DEPARTMENT OF COMPUTER SCIENCE &amp; ENGINEERING • PROJECT EVALUATION DOSSIER
        </div>
        <div className="footer-meta">
          Compiler Optimization Engine • Academic Evaluation Edition 2026
        </div>
      </footer>
    </div>
  );
}
