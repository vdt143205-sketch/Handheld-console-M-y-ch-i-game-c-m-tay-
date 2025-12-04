const char game_snake_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="vi">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1, maximum-scale=1, user-scalable=no">
  <title>Rắn Săn Mồi Neon</title>
  <style>
    body { margin: 0; background: #050505; overflow: hidden; touch-action: none; text-align: center; color: #0f0; font-family: 'Courier New', monospace; }
    canvas { border: 2px solid #333; background: #000; margin-top: 10px; box-shadow: 0 0 20px #00ff0033; }
    #ui { font-size: 24px; margin-top: 10px; font-weight: bold; }
    #btn-home { position: absolute; top: 10px; left: 10px; color: #555; text-decoration: none; font-weight: bold; }
    
    /* Màn hình Nhập tên / Game Over */
    #overlay { 
      position: absolute; top: 0; left: 0; width: 100%; height: 100%; 
      background: rgba(0,0,0,0.9); z-index: 10; display: flex; 
      flex-direction: column; justify-content: center; align-items: center;
    }
    input { padding: 10px; font-size: 20px; text-align: center; border-radius: 5px; border: 2px solid #00ff00; background: #111; color: #fff; width: 200px; margin-bottom: 20px; }
    .btn { padding: 15px 40px; font-size: 20px; background: #00ff00; color: #000; border: none; border-radius: 50px; cursor: pointer; font-weight: bold; box-shadow: 0 0 15px #00ff00; }
    .btn:active { transform: scale(0.95); }
    
    #dpad { display: none; position: absolute; bottom: 20px; left: 50%; transform: translateX(-50%); width: 180px; height: 180px; }
    .dbtn { position: absolute; width: 60px; height: 60px; background: rgba(0,255,0,0.15); border: 1px solid #0f0; border-radius: 15px; display: flex; justify-content: center; align-items: center; font-size: 30px; color: #0f0; }
    #u { top: 0; left: 60px; } #d { bottom: 0; left: 60px; } #l { top: 60px; left: 0; } #r { top: 60px; right: 0; }
    
    .hidden { display: none !important; }
  </style>
</head>
<body>
  <a href="/" id="btn-home">🏠 THOÁT</a>
  <div id="ui">ĐIỂM: <span id="s">0</span></div>
  <canvas id="gc"></canvas>
  
  <!-- Điều khiển ảo -->
  <div id="dpad"><div class="dbtn" id="u">▲</div><div class="dbtn" id="d">▼</div><div class="dbtn" id="l">◄</div><div class="dbtn" id="r">►</div></div>

  <!-- Màn hình Chính (Nhập tên / Kết thúc) -->
  <div id="overlay">
    <h1 id="title" style="color:#0f0; font-size:40px; text-shadow:0 0 10px #0f0">RẮN SĂN MỒI</h1>
    <div id="input-area">
      <p>Nhập tên của bạn:</p>
      <input type="text" id="pName" maxlength="8" placeholder="Tên...">
      <br>
      <button class="btn" onclick="startGame()">BẮT ĐẦU</button>
    </div>
    <div id="over-area" class="hidden">
      <h2 style="color:red">GAME OVER</h2>
      <p style="font-size:24px">Điểm: <span id="final-score">0</span></p>
      <button class="btn" onclick="location.reload()">CHƠI LẠI</button>
      <br><br>
      <a href="/leaderboard" style="color:#fff">🏆 Xem Bảng Xếp Hạng</a>
    </div>
  </div>

<script>
  const c=document.getElementById('gc'), ctx=c.getContext('2d');
  const W=window.innerWidth, H=window.innerHeight;
  const S=20; 
  let TCX=Math.floor((W>600?600:W-20)/S), TCY=Math.floor((H>600?600:H-150)/S);
  c.width=TCX*S; c.height=TCY*S;

  if(/Android|iPhone/i.test(navigator.userAgent)) document.getElementById('dpad').style.display='block';

  let px=10, py=10, gs=S, ax=15, ay=15, xv=0, yv=0, trail=[], tail=5, score=0;
  let isRunning = false;
  let playerName = "NoName";
  let gameInterval;
  
  // WebSocket
  let ws = new WebSocket('ws://'+location.host+'/ws');

  function startGame() {
    let inputName = document.getElementById('pName').value;
    if(inputName.trim() === "") { alert("Vui lòng nhập tên!"); return; }
    playerName = inputName.toUpperCase();
    
    document.getElementById('input-area').classList.add('hidden');
    document.getElementById('overlay').classList.add('hidden');
    
    resetGame();
  }

  function resetGame() {
    px=Math.floor(TCX/2); py=Math.floor(TCY/2);
    xv=0; yv=0; trail=[]; tail=5; score=0;
    document.getElementById('s').innerText=score;
    isRunning = true;
    if(gameInterval) clearInterval(gameInterval);
    gameInterval = setInterval(game, 1000/12); // Tốc độ game
  }

  function game() {
    if(!isRunning) return;

    px+=xv; py+=yv;
    if(px<0) px=TCX-1; if(px>TCX-1) px=0; if(py<0) py=TCY-1; if(py>TCY-1) py=0;

    ctx.fillStyle="black"; ctx.fillRect(0,0,c.width,c.height);
    ctx.fillStyle="lime";
    
    for(let i=0; i<trail.length; i++) {
      ctx.fillRect(trail[i].x*gs+1, trail[i].y*gs+1, gs-2, gs-2);
      // Va chạm thân -> Chết
      if(trail[i].x==px && trail[i].y==py && (xv!=0||yv!=0)) { 
         gameOver();
      }
    }
    trail.push({x:px,y:py}); while(trail.length>tail) trail.shift();

    if(ax==px && ay==py) {
      tail++; score++; 
      document.getElementById('s').innerText=score;
      ax=Math.floor(Math.random()*TCX); ay=Math.floor(Math.random()*TCY);
    }
    ctx.fillStyle="red"; ctx.fillRect(ax*gs+1, ay*gs+1, gs-2, gs-2);
  }

  function gameOver() {
    isRunning = false;
    clearInterval(gameInterval);
    
    // Gửi điểm về Server: snake:record:NAME:SCORE
    ws.send("snake:record:" + playerName + ":" + score);
    
    document.getElementById('overlay').classList.remove('hidden');
    document.getElementById('input-area').classList.add('hidden');
    document.getElementById('over-area').classList.remove('hidden');
    document.getElementById('final-score').innerText = score;
  }

  function move(d) {
    if(!isRunning) return;
    if(d==0 && yv!=1) { xv=0; yv=-1; } 
    if(d==2 && yv!=-1) { xv=0; yv=1; } 
    if(d==3 && xv!=1) { xv=-1; yv=0; } 
    if(d==1 && xv!=-1) { xv=1; yv=0; } 
  }

  window.addEventListener('keydown',e=>{
    if(e.key=='ArrowUp'||e.key=='w') move(0); if(e.key=='ArrowDown'||e.key=='s') move(2);
    if(e.key=='ArrowLeft'||e.key=='a') move(3); if(e.key=='ArrowRight'||e.key=='d') move(1);
  });
  ['u','d','l','r'].forEach((id,i)=>{
    let map=[0,2,3,1]; 
    document.getElementById(id).addEventListener('touchstart', (e)=>{e.preventDefault(); move(map[i]);});
    document.getElementById(id).addEventListener('mousedown', (e)=>{e.preventDefault(); move(map[i]);});
  });

  ws.onmessage = (evt) => {
     let msg = evt.data;
     if(msg.startsWith("snake:input:")) { move(parseInt(msg.split(':')[2])); }
  };
</script>
</body>
</html>
)rawliteral";