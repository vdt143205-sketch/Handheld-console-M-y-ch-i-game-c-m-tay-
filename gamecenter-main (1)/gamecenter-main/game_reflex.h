const char game_reflex_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1, user-scalable=no">
  <title>Phản Xạ</title>
  <style>
    body { font-family: sans-serif; background: #2d3436; color: white; text-align: center; margin: 0; padding: 10px; user-select: none; }
    .header { display: flex; justify-content: space-between; margin-bottom: 10px; }
    .btn-nav { background: #444; color: #fff; padding: 8px 12px; border-radius: 5px; text-decoration: none; border: none; cursor: pointer; }
    #target-box { height: 100px; width: 100px; margin: 10px auto; background: #555; border-radius: 50%; border: 5px solid #fff; display: flex; justify-content: center; align-items: center; font-size: 30px; font-weight: bold; }
    .grid-btn { display: grid; grid-template-columns: 1fr 1fr; gap: 10px; max-width: 400px; margin: 0 auto; }
    .btn-color { height: 90px; border: none; border-radius: 15px; cursor: pointer; opacity: 0.9; }
    .btn-color:active { transform: scale(0.95); }
    .red { background: #ff7675; } .green { background: #55efc4; } .blue { background: #74b9ff; } .yellow { background: #fdcb6e; }
    #status { height: 20px; color: #fab1a0; margin-bottom: 10px; }
    .score-board { font-size: 14px; margin-bottom: 10px; }
    #modal { display: none; position: fixed; top: 0; left: 0; width: 100%; height: 100%; background: rgba(0,0,0,0.9); flex-direction: column; justify-content: center; align-items: center; z-index: 99; }
    .box { background: #222; padding: 20px; width: 80%; max-width: 300px; border-radius: 10px; border: 1px solid #777; text-align: left; }
  </style>
</head>
<body>

  <div class="header">
    <a href="/" class="btn-nav">🏠 Home</a>
  </div>

  <div class="score-board" id="scores">...</div>
  <div id="target-box">?</div>
  <div id="status">Chuẩn bị...</div>

  <div class="grid-btn">
    <button class="btn-color red" onclick="clickColor(0)"></button>
    <button class="btn-color green" onclick="clickColor(1)"></button>
    <button class="btn-color blue" onclick="clickColor(2)"></button>
    <button class="btn-color yellow" onclick="clickColor(3)"></button>
  </div>

  <div id="modal">
    <div id="win-txt" style="font-size:40px; color:gold; font-weight:bold; margin-bottom:20px;">WINNER!</div>
    <button style="padding:15px 30px; font-size:20px; background:#0984e3; color:white; border:none; border-radius:5px;" onclick="resetGame()">CHƠI LẠI</button>
  </div>

<script>
  // --- 1. CODE ÂM THANH SFX ---
  const AudioContext = window.AudioContext || window.webkitAudioContext;
  const ctx = new AudioContext();
  function playTone(freq, type, duration, delay=0) {
    const osc = ctx.createOscillator(); const gain = ctx.createGain();
    osc.type = type; osc.frequency.value = freq;
    osc.connect(gain); gain.connect(ctx.destination);
    osc.start(ctx.currentTime + delay);
    gain.gain.exponentialRampToValueAtTime(0.00001, ctx.currentTime + delay + duration);
    osc.stop(ctx.currentTime + delay + duration);
  }
  function sfx(name) {
    if(ctx.state === 'suspended') ctx.resume();
    if(name === 'tap') playTone(800, 'sine', 0.1);
    else if(name === 'win') { playTone(523, 'square', 0.1, 0); playTone(659, 'square', 0.1, 0.1); playTone(784, 'square', 0.1, 0.2); playTone(1046, 'square', 0.4, 0.3); }
    else if(name === 'wrong') { playTone(150, 'sawtooth', 0.3); playTone(100, 'sawtooth', 0.3, 0.2); }
    else if(name === 'beep') { playTone(1200, 'triangle', 0.1); }
  }

  // --- 2. LOGIC GAME ---
  let ws = new WebSocket('ws://' + location.host + '/ws');
  let mySlot = -1;
  let currentTarget = -1; // Lưu màu mục tiêu hiện tại để check đúng sai

  ws.onmessage = (evt) => {
    let msg = evt.data;
    if(msg.startsWith("reflex:")) {
      let parts = msg.split(':');
      if(parts[1] === 'welcome') { 
        mySlot = parseInt(parts[2]); 
        document.getElementById('status').innerText = "Bạn là P" + (mySlot+1); 
      }
      else if(parts[1] === 'scores') {
        let s = parts[2].split(',');
        let h = ""; for(let i=0; i<4; i++) h += `P${i+1}: <b>${s[i]}</b> | `;
        document.getElementById('scores').innerHTML = h;
      }
      else if(parts[1] === 'show') {
        let c = ['#ff7675', '#55efc4', '#74b9ff', '#fdcb6e'];
        let colorId = parseInt(parts[2]);
        currentTarget = colorId; // Cập nhật màu đúng
        
        document.getElementById('target-box').style.background = c[colorId];
        document.getElementById('target-box').innerText = ""; 
        document.getElementById('status').innerText = "BẤM!";
        sfx('beep'); // Kêu Bíp khi hiện màu
      }
      else if(parts[1] === 'wait') {
        document.getElementById('target-box').style.background = "#555";
        document.getElementById('target-box').innerText = "...";
        document.getElementById('status').innerText = "Chờ...";
      }
      else if(parts[1] === 'win') {
        document.getElementById('win-txt').innerText = "P" + (parseInt(parts[2])+1) + " THẮNG!";
        document.getElementById('modal').style.display = 'flex';
        sfx('win'); // Kêu nhạc chiến thắng
      }
    }
  };

  function clickColor(id) { 
    if(mySlot != -1) {
      ws.send('reflex:'+id);
      
      // Logic âm thanh phản hồi ngay lập tức
      if(id === currentTarget) {
         sfx('tap'); // Đúng thì kêu ting
      } else {
         sfx('wrong'); // Sai thì kêu è è
      }
    } 
  }
  function resetGame() { ws.send('reflex:reset'); document.getElementById('modal').style.display = 'none'; }
</script>
</body>
</html>
)rawliteral";