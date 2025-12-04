const char game_fight_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1, user-scalable=no">
  <title>Đại Chiến</title>
  <style>
    body { font-family: sans-serif; background: #2c3e50; color: white; text-align: center; margin: 0; padding: 10px; user-select: none; }
    .header { display: flex; justify-content: space-between; margin-bottom: 15px; }
    .btn-nav { background: #34495e; color: #ecf0f1; padding: 8px; border-radius: 5px; text-decoration: none; border: none; cursor: pointer; font-size: 14px;}
    .hp-container { display: flex; justify-content: space-between; margin-bottom: 20px; padding: 0 5px; }
    .hp-box { width: 48%; }
    .hp-bg { width: 100%; height: 15px; background: #555; border-radius: 10px; overflow: hidden; border: 1px solid #fff; }
    .hp-bar { height: 100%; width: 100%; transition: width 0.3s; }
    #hp1 { background: #e74c3c; } #hp2 { background: #3498db; }
    .arena { height: 120px; display: flex; justify-content: center; align-items: center; gap: 20px; font-size: 50px; background: #34495e; border-radius: 15px; margin-bottom: 20px; }
    .controls { display: grid; grid-template-columns: 1fr 1fr 1fr; gap: 10px; }
    .btn-move { background: #ecf0f1; border: none; border-radius: 10px; padding: 15px 0; font-size: 35px; cursor: pointer; }
    .btn-move:active { background: #bdc3c7; }
    .btn-move.selected { background: #f1c40f; border: 3px solid #e67e22; }
    #modal, #help-modal { display: none; position: fixed; top: 0; left: 0; width: 100%; height: 100%; background: rgba(0,0,0,0.9); flex-direction: column; justify-content: center; align-items: center; z-index: 99; }
    .box { background: #333; padding: 20px; border-radius: 10px; width: 80%; max-width: 300px; text-align: left;}
    .box li { margin-bottom: 5px; color: #ccc; }
  </style>
</head>
<body>

  <div class="header">
    <a href="/" class="btn-nav">🏠 Thoát</a>
    <button class="btn-nav" onclick="toggleHelp()">❓ Luật chơi</button>
  </div>

  <div id="status" style="margin-bottom:10px; font-weight:bold; color:#f1c40f;">Đang kết nối...</div>

  <div class="hp-container">
    <div class="hp-box">P1 <div class="hp-bg"><div id="hp1" class="hp-bar"></div></div></div>
    <div class="hp-box">P2 <div class="hp-bg"><div id="hp2" class="hp-bar"></div></div></div>
  </div>

  <div class="arena">
    <div id="move1">❔</div> <div>VS</div> <div id="move2">❔</div>
  </div>

  <div class="controls">
    <button class="btn-move" onclick="pick('rock')">🪨</button>
    <button class="btn-move" onclick="pick('paper')">📄</button>
    <button class="btn-move" onclick="pick('scissors')">✂️</button>
  </div>

  <div id="help-modal" onclick="toggleHelp()">
    <div class="box">
      <h3 style="color:#f1c40f; margin-top:0; text-align:center;">LUẬT CHƠI</h3>
      <ul><li>Búa > Kéo > Bao > Búa</li></ul>
      <button class="btn-nav" style="width:100%; text-align:center;">Đóng</button>
    </div>
  </div>

  <div id="modal">
    <div id="win-msg" style="font-size:40px; color:gold; font-weight:bold; margin-bottom:20px;">WINNER!</div>
    <button class="btn-move" style="font-size:20px; padding:10px 30px;" onclick="resetGame()">CHƠI LẠI</button>
  </div>

<script>
  // --- ÂM THANH ---
  const audioCtx = new (window.AudioContext || window.webkitAudioContext)();
  function playTone(freq, type, duration, delay=0) {
    if(audioCtx.state === 'suspended') audioCtx.resume();
    const osc = audioCtx.createOscillator(); const gain = audioCtx.createGain();
    osc.type = type; osc.frequency.setValueAtTime(freq, audioCtx.currentTime + delay);
    osc.connect(gain); gain.connect(audioCtx.destination);
    osc.start(audioCtx.currentTime + delay);
    gain.gain.exponentialRampToValueAtTime(0.00001, audioCtx.currentTime + delay + duration);
    osc.stop(audioCtx.currentTime + delay + duration);
  }
  function s_click() { playTone(600, 'sine', 0.1); }
  function s_hit() { playTone(100, 'sawtooth', 0.2); } 
  function s_win() { [523, 659, 784, 1046].forEach((f, i) => playTone(f, 'square', 0.2, i * 0.15)); }

  let ws = new WebSocket('ws://' + location.host + '/ws');
  let mySlot = -1; 
  const moves = { 'rock': '🪨', 'paper': '📄', 'scissors': '✂️', 'none': '❔' };
  
  let oldHp1 = 100; let oldHp2 = 100;

  function toggleHelp() {
    let el = document.getElementById('help-modal');
    el.style.display = (el.style.display === 'flex') ? 'none' : 'flex';
  }

  ws.onmessage = (evt) => {
    let msg = evt.data;
    if(msg.startsWith("fight:")) {
      let parts = msg.split(':');
      if(parts[1] === 'welcome') {
        mySlot = parseInt(parts[2]);
        document.getElementById('status').innerText = "Bạn là P" + (mySlot + 1);
        document.getElementById('status').style.color = mySlot==0?'#e74c3c':'#3498db';
      }
      else if(parts[1] === 'update') {
        let d = parts[2].split(',');
        let newHp1 = parseInt(d[0]);
        let newHp2 = parseInt(d[1]);

        if(newHp1 < oldHp1 || newHp2 < oldHp2) s_hit();
        oldHp1 = newHp1; oldHp2 = newHp2;

        document.getElementById('hp1').style.width = newHp1 + '%';
        document.getElementById('hp2').style.width = newHp2 + '%';
        document.getElementById('move1').innerText = moves[d[2]];
        document.getElementById('move2').innerText = moves[d[3]];
        if(d[4] !== 'null') document.getElementById('status').innerText = d[4];
        
        let btns = document.getElementsByClassName('btn-move');
        for(let b of btns) b.classList.remove('selected');
      }
      else if(parts[1] === 'end') {
        document.getElementById('win-msg').innerText = "PLAYER " + (parseInt(parts[2]) + 1) + " WIN!";
        document.getElementById('modal').style.display = 'flex';
        s_win();
      }
    }
  };

  function pick(m) {
    if(mySlot!=-1) {
      s_click(); 
      ws.send('fight:' + m);
      let btns = document.getElementsByClassName('btn-move');
      for(let b of btns) b.classList.remove('selected');
      event.target.classList.add('selected');
      document.getElementById('status').innerText = "Đã chọn...";
    }
  }
  function resetGame() { 
    ws.send('fight:reset'); 
    document.getElementById('modal').style.display = 'none'; 
    oldHp1 = 100; oldHp2 = 100; 
  }
</script>
</body>
</html>
)rawliteral";