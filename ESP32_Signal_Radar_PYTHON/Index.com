<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<title>BioTech Firewall — Command Console</title>
<link href="https://fonts.googleapis.com/css2?family=Orbitron:wght@400;600&family=Rajdhani:wght@400;600&display=swap" rel="stylesheet">
<style>
  :root {
    --neon-cyan: #00eaff;
    --neon-red: #ff3b3b;
    --bg-dark: #030712;
    --glass: rgba(0, 234, 255, 0.05);
  }

  html, body {
    margin: 0; background: var(--bg-dark);
    font-family: 'Rajdhani', sans-serif; color: var(--neon-cyan);
    height: 100%; overflow: hidden;
  }

  /* Cinematic Scanline Overlay */
  body::before {
    content: " "; position: fixed; inset: 0;
    background: linear-gradient(rgba(18, 16, 16, 0) 50%, rgba(0, 0, 0, 0.25) 50%), 
                linear-gradient(90deg, rgba(255, 0, 0, 0.06), rgba(0, 255, 0, 0.02), rgba(0, 0, 255, 0.06));
    background-size: 100% 4px, 3px 100%; z-index: 100; pointer-events: none;
  }

  #console {
    position: fixed; inset: 0; display: flex; flex-direction: column;
    background: radial-gradient(circle at center, #07152e 0%, #02050f 100%);
  }

  /* Glow Header */
  #header {
    padding: 20px 30px; border-bottom: 2px solid var(--neon-cyan);
    display: flex; justify-content: space-between;
    font-family: 'Orbitron'; letter-spacing: 4px;
    text-shadow: 0 0 15px var(--neon-cyan);
    background: rgba(0, 234, 255, 0.02);
  }

  #main { display: grid; grid-template-columns: 1.2fr 1fr; flex: 1; padding: 20px; gap: 20px; }

  /* Map Glass Effect */
  #map-container {
    position: relative; border: 1px solid var(--neon-cyan);
    border-radius: 4px; padding: 10px;
    background: var(--glass);
    box-shadow: inset 0 0 20px rgba(0, 234, 255, 0.1);
  }
  
  #map-container iframe {
    width: 100%; height: 100%; border: none; filter: hue-rotate(180deg) brightness(0.8) contrast(1.2);
    opacity: 0.8;
  }

  /* Right Panel Content */
  #info { padding: 20px; display: flex; flex-direction: column; gap: 20px; }
  
  .glass-panel {
    background: rgba(0, 0, 0, 0.6); border-left: 4px solid var(--neon-cyan);
    padding: 20px; box-shadow: 0 0 20px rgba(0,0,0,0.5);
  }

  h3 { font-family: 'Orbitron'; text-transform: uppercase; margin-top: 0; color: #fff; }

  /* High-Tech Buttons */
  .controls { display: grid; grid-template-columns: 1fr 1fr; gap: 15px; }
  
  button {
    background: transparent; border: 1px solid var(--neon-cyan);
    color: var(--neon-cyan); padding: 15px; font-family: 'Orbitron';
    cursor: pointer; position: relative; overflow: hidden;
    transition: 0.4s; clip-path: polygon(10% 0, 100% 0, 100% 70%, 90% 100%, 0 100%, 0 30%);
  }

  button::after {
    content: ''; position: absolute; top: 0; left: -100%;
    width: 100%; height: 100%; background: linear-gradient(90deg, transparent, rgba(0, 234, 255, 0.4), transparent);
    transition: 0.5s;
  }

  button:hover { background: rgba(0, 234, 255, 0.1); box-shadow: 0 0 15px var(--neon-cyan); }
  button:hover::after { left: 100%; }

  footer {
    padding: 15px; border-top: 1px solid var(--neon-cyan);
    display: flex; justify-content: space-around; font-family: 'Orbitron'; font-size: 12px;
    background: rgba(0, 0, 0, 0.3);
  }

  .alert-text { color: var(--neon-red); text-shadow: 0 0 10px var(--neon-red); animation: blink 1s infinite; }

  @keyframes blink { 50% { opacity: 0.3; } }
</style>
</head>
<body>

<div id="console">
  <div id="header">
    <span>SYSTEMS OVERRIDE // BIO-SHIELD</span>
    <span class="alert-text">FIREWALL: ACTIVE</span>
  </div>

  <div id="main">
    <div id="map-container">
      <iframe src="https://www.google.com/maps?q=Manhattan&output=embed" loading="lazy"></iframe>
    </div>

    <div id="info">
      <div class="glass-panel">
        <h3>Threat Intelligence</h3>
        <p style="font-size: 0.9em; opacity: 0.7;">No breach attempts detected in the last 140ms. Neural sync stable at 99.8%.</p>
      </div>

      <div class="glass-panel">
        <h3>Subsystem Selection</h3>
        <div class="controls">

	<button onclick="location.href='/cyber'">CYBER</button>
	<button onclick="location.href='/medical'">MEDICAL</button>
	<button onclick="location.href='/route'">ROUTE</button>
	<button onclick="location.href='/systems'">SYSTEMS</button>
	<button onclick="location.href='/network'">NETWORK</button>
	<button onclick="location.href='/neural'">NEURAL</button>
	<button onclick="location.href='/cybernetics'">CYBERNETICS</button>
	<button onclick="location.href='/logging'">SYSTEM LOGS</button>

        </div>
      </div>
    </div>
  </div>

  <footer>
    <span>AI_CORE_LINK: ESTABLISHED</span>
    <span style="color: #fff;">USER: AUTHENTICATED</span>
    <span>LOCATION: 40.7128° N, 74.0060° W</span>
  </footer>
</div>

</body>
</html>
