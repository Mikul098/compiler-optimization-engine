import React, { useState, useEffect } from 'react';
import { Play, Pause, ChevronLeft, ChevronRight, RotateCcw, Cpu, Layers, Sparkles } from 'lucide-react';
import { instructionToString } from '../services/compilerEngine';

export default function PassInspector({ passHistory }) {
  const [currentStepIdx, setCurrentStepIdx] = useState(0);
  const [isPlaying, setIsPlaying] = useState(false);

  useEffect(() => {
    if (!passHistory || passHistory.length === 0) return;
    setCurrentStepIdx(0);
    setIsPlaying(false);
  }, [passHistory]);

  useEffect(() => {
    let timer = null;
    if (isPlaying) {
      timer = setInterval(() => {
        setCurrentStepIdx(prev => {
          if (prev >= passHistory.length - 1) {
            setIsPlaying(false);
            return prev;
          }
          return prev + 1;
        });
      }, 1400);
    }
    return () => {
      if (timer) clearInterval(timer);
    };
  }, [isPlaying, passHistory]);

  if (!passHistory || passHistory.length === 0) {
    return (
      <div className="pass-empty-state">
        <p>No optimization pass history recorded yet.</p>
      </div>
    );
  }

  const currentStep = passHistory[currentStepIdx] || passHistory[0];
  const isFirst = currentStepIdx === 0;
  const isLast = currentStepIdx === passHistory.length - 1;

  return (
    <div className="pass-inspector-card">
      {/* Top Playback Controls & Scrubber */}
      <div className="pass-playback-header">
        <div className="playback-controls">
          <button
            className="ctrl-btn"
            onClick={() => setCurrentStepIdx(prev => Math.max(0, prev - 1))}
            disabled={isFirst}
            title="Previous Pass"
          >
            <ChevronLeft size={16} />
          </button>
          <button
            className={`ctrl-btn play-btn ${isPlaying ? 'playing' : ''}`}
            onClick={() => setIsPlaying(!isPlaying)}
            title={isPlaying ? 'Pause Stepper' : 'Auto Play Stepper'}
          >
            {isPlaying ? <Pause size={16} /> : <Play size={16} />}
          </button>
          <button
            className="ctrl-btn"
            onClick={() => setCurrentStepIdx(prev => Math.min(passHistory.length - 1, prev + 1))}
            disabled={isLast}
            title="Next Pass"
          >
            <ChevronRight size={16} />
          </button>
          <button
            className="ctrl-btn"
            onClick={() => { setCurrentStepIdx(0); setIsPlaying(false); }}
            title="Reset to Initial State"
          >
            <RotateCcw size={14} />
          </button>
        </div>

        <div className="stepper-indicator">
          <span className="step-count">
            Step <strong>{currentStepIdx + 1}</strong> of <strong>{passHistory.length}</strong>
          </span>
          <span className="step-badge">
            {currentStep.passName || 'Initial State'}
          </span>
        </div>
      </div>

      {/* Progress Timeline Pills */}
      <div className="pass-timeline-track">
        {passHistory.map((step, idx) => {
          const isActive = idx === currentStepIdx;
          const isPassed = idx < currentStepIdx;
          return (
            <button
              key={`timeline-step-${idx}`}
              className={`timeline-step-btn ${isActive ? 'active' : ''} ${isPassed ? 'passed' : ''}`}
              onClick={() => { setCurrentStepIdx(idx); setIsPlaying(false); }}
              title={step.name}
            >
              <span className="step-dot" />
              <span className="step-label">{step.name.split(':')[0]}</span>
            </button>
          );
        })}
      </div>

      {/* Pass Details Callout */}
      <div className="pass-details-callout">
        <div className="callout-header">
          <div className="callout-title">
            <Cpu size={16} className="text-navy" />
            <h3>{currentStep.name}</h3>
          </div>
          {currentStep.changes !== undefined && (
            <span className={`changes-badge ${currentStep.changes > 0 ? 'has-changes' : 'no-changes'}`}>
              <Sparkles size={12} /> {currentStep.changes} Changes Applied
            </span>
          )}
        </div>
        <p className="callout-summary">{currentStep.summary}</p>
      </div>

      {/* Instructions State Grid */}
      <div className="pass-instructions-container">
        <div className="pass-instructions-header">
          <span>Three-Address Code Snapshot ({currentStep.instructions.length} quads)</span>
          <span className="state-tag">{isLast ? 'Fixed-Point Converged State' : 'Intermediate Working State'}</span>
        </div>

        <div className="pass-instructions-list">
          {currentStep.instructions.map((instr, idx) => (
            <div key={`pass-instr-${idx}`} className="pass-instr-row">
              <span className="instr-num">{idx + 1}</span>
              <span className="instr-op-badge">{instr.op}</span>
              <code className="instr-text">{instructionToString(instr)}</code>
            </div>
          ))}
        </div>
      </div>
    </div>
  );
}
