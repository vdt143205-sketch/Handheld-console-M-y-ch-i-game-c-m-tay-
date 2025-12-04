const char game_shooter_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1, maximum-scale=1, user-scalable=no">
  <title>Phi Thuyền</title>
  <style>
    body { margin: 0; background: #000; overflow: hidden; font-family: 'Segoe UI', sans-serif; touch-action: none; }
    canvas { display: block; width: 100%; height: 100%; }
    
    #ui { position: absolute; top: 10px; left: 10px; color: #fff; font-size: 20px; font-weight: bold; pointer-events: none; }
    #score { color: #e67e22; }
    
    #game-over {
      display: none; position: absolute; top: 50%; left: 50%; transform: translate(-50%, -50%);
      text-align: center; color: white; z-index: 10;
    }
    h1 { font-size: 40px; color: #e74c3c; text-shadow: 0 0 10px red; margin: 0 0 20px 0; }
    
    .btn { padding: 15px 30px; font-size: 20px; background: #2ecc71; color: white; border: none; border-radius: 50px; cursor: pointer; box-shadow: 0 0 15px #2ecc71; }
    .btn-home { background: #555; box-shadow: none; margin-top: 10px; font-size: 16px; padding: 10px 20px; }

    /* Mobile Controls */
    #mobile-ctrl { display: none; position: absolute; bottom: 20px; width: 100%; height: 150px; pointer-events: auto; }
    .dpad { position: absolute; left: 20px; bottom: 0; width: 150px; height: 150px; }
    .btn-move { position: absolute; width: 50px; height: 50px; background: rgba(255,255,255,0.2); border-radius: 50%; border: 1px solid rgba(255,255,255,0.5); display: flex; justify-content: center; align-items: center; color: white; font-size: 20px; }
    #b-up { top: 0; left: 50px; } #b-down { bottom: 0; left: 50px; }
    #b-left { top: 50px; left: 0; } #b-right { top: 50px; right: 0; }
    
    #b-fire { position: absolute; right: 30px; bottom: 30px; width: 80px; height: 80px; background: rgba(231, 76, 60, 0.5); border-radius: 50%; border: 2px solid #e74c3c; color: white; font-weight: bold; display: flex; justify-content: center; align-items: center; }
    #b-fire:active { background: rgba(231, 76, 60, 0.8); transform: scale(0.95); }
  </style>
</head>
<body>

  <div id="ui">ĐIỂM: <span id="score">0</span></div>
  
  <div id="game-over">
    <h1>GAME OVER</h1>
    <button class="btn" onclick="resetGame()">CHƠI LẠI</button>
    <br><br>
    <a href="/" class="btn btn-home" style="text-decoration:none">Thoát</a>
  </div>

  <div id="mobile-ctrl">
    <div class="dpad">
      <div class="btn-move" id="b-up">▲</div>
      <div class="btn-move" id="b-down">▼</div>
      <div class="btn-move" id="b-left">◄</div>
      <div class="btn-move" id="b-right">►</div>
    </div>
    <div id="b-fire">BẮN</div>
  </div>

  <canvas id="gameCanvas"></canvas>

<script>
  const canvas = document.getElementById('gameCanvas');
  const ctx = canvas.getContext('2d');
  
  // Resize
  function resize() { canvas.width = window.innerWidth; canvas.height = window.innerHeight; }
  window.addEventListener('resize', resize); resize();
  if(/Android|iPhone/i.test(navigator.userAgent)) document.getElementById('mobile-ctrl').style.display = 'block';

  // --- GAME LOGIC ---
  let score = 0;
  let gameOver = false;
  const player = { x: 100, y: 300, w: 40, h: 40, speed: 5, color: '#00cec9' };
  const bullets = [];
  const enemies = [];
  const stars = [];
  let enemyTimer = 0;

  // Input State
  const keys = { w:false, a:false, s:false, d:false, fire:false };

  // Init Stars background
  for(let i=0; i<50; i++) stars.push({x: Math.random()*canvas.width, y: Math.random()*canvas.height, s: Math.random()*2});

  function spawnEnemy() {
    const size = 30 + Math.random() * 20;
    enemies.push({
      x: canvas.width,
      y: Math.random() * (canvas.height - size),
      w: size, h: size,
      speed: 3 + Math.random() * 3 + (score/50), // Tăng tốc theo điểm
      color: `hsl(${Math.random()*360}, 70%, 60%)`
    });
  }

  function update() {
    if(gameOver) return;

    // 1. Player Move
    if(keys.w && player.y > 0) player.y -= player.speed;
    if(keys.s && player.y < canvas.height - player.h) player.y += player.speed;
    if(keys.a && player.x > 0) player.x -= player.speed;
    if(keys.d && player.x < canvas.width - player.w) player.x += player.speed;

    // 2. Bullets
    for(let i=bullets.length-1; i>=0; i--) {
      bullets[i].x += 10;
      if(bullets[i].x > canvas.width) bullets.splice(i, 1);
    }

    // 3. Enemies
    enemyTimer++;
    if(enemyTimer > 60 - Math.min(40, score/2)) { // Spawn nhanh dần
      spawnEnemy();
      enemyTimer = 0;
    }

    for(let i=enemies.length-1; i>=0; i--) {
      let e = enemies[i];
      e.x -= e.speed;

      // Check trúng đạn
      for(let j=bullets.length-1; j>=0; j--) {
        let b = bullets[j];
        if(b.x < e.x + e.w && b.x + b.w > e.x && b.y < e.y + e.h && b.y + b.h > e.y) {
           enemies.splice(i, 1);
           bullets.splice(j, 1);
           score += 10;
           document.getElementById('score').innerText = score;
           break; 
        }
      }

      // Check va chạm Player
      if(player.x < e.x + e.w && player.x + player.w > e.x && player.y < e.y + e.h && player.y + player.h > e.y) {
         gameOver = true;
         document.getElementById('game-over').style.display = 'block';
      }

      if(e.x + e.w < 0) enemies.splice(i, 1);
    }

    // 4. Stars
    stars.forEach(s => { s.x -= s.s; if(s.x < 0) s.x = canvas.width; });
  }

  function draw() {
    ctx.fillStyle = '#000'; ctx.fillRect(0, 0, canvas.width, canvas.height);

    // Stars
    ctx.fillStyle = '#fff';
    stars.forEach(s => { ctx.beginPath(); ctx.arc(s.x, s.y, s.s, 0, Math.PI*2); ctx.fill(); });

    // Player (🚀)
    ctx.fillStyle = player.color;
    ctx.beginPath(); ctx.moveTo(player.x + player.w, player.y + player.h/2);
    ctx.lineTo(player.x, player.y); ctx.lineTo(player.x + 10, player.y + player.h/2);
    ctx.lineTo(player.x, player.y + player.h); ctx.fill();
    // Lửa đuôi
    ctx.fillStyle = 'orange';
    ctx.beginPath(); ctx.arc(player.x - 5, player.y + player.h/2, 5 + Math.random()*5, 0, Math.PI*2); ctx.fill();

    // Bullets
    ctx.fillStyle = '#f1c40f';
    bullets.forEach(b => ctx.fillRect(b.x, b.y, b.w, b.h));

    // Enemies (👾)
    enemies.forEach(e => {
      ctx.fillStyle = e.color;
      ctx.fillRect(e.x, e.y, e.w, e.h);
      // Vẽ mắt quái
      ctx.fillStyle = '#000'; 
      ctx.fillRect(e.x + 5, e.y + 10, 5, 5); ctx.fillRect(e.x + 5, e.y + e.h - 15, 5, 5);
    });

    if(!gameOver) requestAnimationFrame(() => { update(); draw(); });
  }

  function shoot() {
    if(!gameOver) bullets.push({ x: player.x + player.w, y: player.y + player.h/2 - 2, w: 10, h: 4 });
  }

  function resetGame() {
    score = 0; document.getElementById('score').innerText = "0";
    enemies.length = 0; bullets.length = 0;
    player.x = 100; player.y = canvas.height/2;
    gameOver = false;
    document.getElementById('game-over').style.display = 'none';
    draw();
  }

  // --- INPUT HANDLER ---
  window.addEventListener('keydown', e => {
    if(e.key=='w' || e.key=='ArrowUp') keys.w = true;
    if(e.key=='s' || e.key=='ArrowDown') keys.s = true;
    if(e.key=='a' || e.key=='ArrowLeft') keys.a = true;
    if(e.key=='d' || e.key=='ArrowRight') keys.d = true;
    if(e.code=='Space' || e.key=='Enter') shoot();
  });
  window.addEventListener('keyup', e => {
    if(e.key=='w' || e.key=='ArrowUp') keys.w = false;
    if(e.key=='s' || e.key=='ArrowDown') keys.s = false;
    if(e.key=='a' || e.key=='ArrowLeft') keys.a = false;
    if(e.key=='d' || e.key=='ArrowRight') keys.d = false;
  });

  // Mobile Touch
  const bind = (id, k) => {
    let el = document.getElementById(id);
    el.addEventListener('touchstart', (e)=>{e.preventDefault(); keys[k]=true;});
    el.addEventListener('touchend', (e)=>{e.preventDefault(); keys[k]=false;});
  };
  bind('b-up', 'w'); bind('b-down', 's'); bind('b-left', 'a'); bind('b-right', 'd');
  document.getElementById('b-fire').addEventListener('touchstart', (e)=>{e.preventDefault(); shoot();});

  // --- WEBSOCKET (NHẬN TÍN HIỆU TAY CẦM) ---
  let ws = new WebSocket('ws://' + location.host + '/ws');
  ws.onmessage = (evt) => {
    // Server sẽ gửi: pad:0, pad:1...
    let msg = evt.data;
    if(msg.startsWith("pad:")) {
       let c = msg.substring(4);
       // Map nút tay cầm sang di chuyển
       // 0:Lên, 1:Phải, 2:Xuống, 3:Trái, fire:Bắn
       
       // Reset keys mỗi frame để tránh trôi (hoặc dùng cơ chế nhấn giữ nếu tay cầm gửi liên tục)
       // Ở đây giả sử tay cầm gửi liên tục khi giữ
       if(c=="0") { keys.w = true; setTimeout(()=>keys.w=false, 100); }
       if(c=="1") { keys.d = true; setTimeout(()=>keys.d=false, 100); }
       if(c=="2") { keys.s = true; setTimeout(()=>keys.s=false,100); }
       if(c=="3") { keys.a = true; setTimeout(()=>keys.a=false,100); }
       if(c=="fire") shoot();
    }
  };

  draw();
</script>
</body>
</html>
)rawliteral";