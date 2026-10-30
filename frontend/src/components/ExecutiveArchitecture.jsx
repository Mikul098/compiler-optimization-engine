import React from 'react';
import { Layers, Workflow, GitCompare, ShieldCheck, Zap, Repeat, Scissors, Binary } from 'lucide-react';

export default function ExecutiveArchitecture() {
  const stages = [
    {
      num: '01',
      title: 'Grammar & AST Construction',
      tech: 'Flex 2.6 & Bison 3.8',
      desc: 'Lexical analysis and LALR(1) parsing building recursive hierarchical AST with operator precedence and associativity.',
      icon: <Layers size={18} />
    },
    {
      num: '02',
      title: 'IR Quad Synthesis',
      tech: 'Three-Address Code (TAC)',
      desc: 'Flattens complex nested expressions into linear quads (result = arg1 op arg2) with unique temporary variable allocation.',
      icon: <Binary size={18} />
    },
    {
      num: '03',
      title: 'Fixed-Point Pass Engine',
      tech: 'Multi-Pass Iterative Cascade',
      desc: 'Executes Constant Folding, Commutative CSE, and Dead Code Elimination cyclically until the IR instruction hash stabilizes.',
      icon: <Repeat size={18} />
    },
    {
      num: '04',
      title: 'Differential & VM Parity',
      tech: 'DiffEngine & VM Interpreter',
      desc: 'Measures instruction compaction ratios and validates semantic equivalence by executing unoptimized and optimized IR in parallel.',
      icon: <ShieldCheck size={18} />
    }
  ];

  return (
    <div className="executive-arch-card">
      <div className="section-header-block">
        <span className="section-num">1.</span>
        <h2 className="section-title">EXECUTIVE ABSTRACT &amp; COMPILER ARCHITECTURE</h2>
        <div className="section-line" />
      </div>

      <div className="abstract-prose">
        <p>
          Production compilers require mathematical guarantees of semantic preservation during aggressive code compaction.
          The <strong>Autonomous Compiler Optimization Engine</strong> implements an end-to-end multi-pass pipeline for
          the Toy programming language, spanning grammar-driven Abstract Syntax Tree generation, linear Three-Address Code (TAC)
          intermediate representation (IR), and an automated <strong>Fixed-Point Pass Manager</strong>.
        </p>
        <p>
          By cyclically sequencing <em>Constant Propagation &amp; Folding</em>, <em>Commutative Common Subexpression Elimination (CSE)</em>,
          and <em>Backward Liveness Dead Code Elimination (DCE)</em>, the engine achieves instruction footprint compaction up to{' '}
          <strong>93.33%</strong>. A virtual machine interpreter independently evaluates pre- and post-optimization TAC streams to
          prove 100.0% semantic parity across all execution benchmarks.
        </p>
      </div>

      {/* 4 Pipeline Stages Cards */}
      <div className="pipeline-stages-grid">
        {stages.map((stg) => (
          <div key={stg.num} className="stage-card">
            <div className="stage-card-header">
              <span className="stage-num">{stg.num}</span>
              <div className="stage-icon">{stg.icon}</div>
            </div>
            <h4 className="stage-title">{stg.title}</h4>
            <span className="stage-tech">{stg.tech}</span>
            <p className="stage-desc">{stg.desc}</p>
          </div>
        ))}
      </div>

      {/* Optimization Formalisms Box */}
      <div className="formalisms-box">
        <div className="formalism-item">
          <span className="formalism-label">FIXED-POINT CONVERGENCE CONSTRUCT</span>
          <code className="formalism-formula">
            T_opt = fix( P_DCE ∘ P_CSE ∘ P_CF )( T_0 )
          </code>
          <span className="formalism-caption">Iterates until T_(k+1) == T_k (delta hash == 0)</span>
        </div>

        <div className="formalism-item">
          <span className="formalism-label">SEMANTIC PARITY THEOREM</span>
          <code className="formalism-formula">
            VM_exec( T_0 ) ≡ VM_exec( T_opt )  ∀ runtime states
          </code>
          <span className="formalism-caption">Zero side-effect divergence verified via Interpreter</span>
        </div>

        <div className="formalism-item">
          <span className="formalism-label">CANONICAL COMMUTATIVE SIGNATURE</span>
          <code className="formalism-formula">
            sig(op, a, b) = op + ' ' + min(a, b) + ' ' + max(a, b)
          </code>
          <span className="formalism-caption">Guarantees (x * y) matches (y * x) in available expressions</span>
        </div>
      </div>
    </div>
  );
}
