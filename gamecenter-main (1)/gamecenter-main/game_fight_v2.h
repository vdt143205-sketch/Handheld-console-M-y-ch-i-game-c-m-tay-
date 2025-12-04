const char game_fight_v2_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no, viewport-fit=cover">
  <meta name="mobile-web-app-capable" content="yes">
  <title>Võ Đài V2 Ultimate</title>
  <style>
    /* CHẶN ZOOM & KÉO TRANG */
    * { touch-action: none; -webkit-touch-callout: none; -webkit-user-select: none; user-select: none; -webkit-tap-highlight-color: transparent; }
    
    body { margin: 0; background: #000; overflow: hidden; font-family: 'Segoe UI', sans-serif; width: 100vw; height: 100vh; position: fixed; }
    canvas { display: block; width: 100%; height: 100%; }

    /* MÀN HÌNH BẮT BUỘC XOAY NGANG */
    #rotate-lock {
        display: none; position: fixed; top: 0; left: 0; width: 100%; height: 100%;
        background: #111; z-index: 99999; color: #e74c3c;
        flex-direction: column; justify-content: center; align-items: center; text-align: center;
    }
    #rotate-lock h3 { font-size: 24px; margin-bottom: 10px; color: #fff; }
    @media screen and (orientation: portrait) { #rotate-lock { display: flex; } #game-container { display: none; } }
    
    /* UI HUD */
    #ui-layer { position: absolute; top: 0; left: 0; width: 100%; height: 100%; pointer-events: none; }
    
    .hud-container { position: absolute; top: 10px; width: 44%; height: 80px; display: flex; flex-direction: column; }
    #p1-con { left: 2%; align-items: flex-start; } 
    #p2-con { right: 2%; align-items: flex-end; }
    
    /* Thanh máu */
    .hp-bar-bg { width: 100%; height: 20px; background: #444; border: 2px solid #fff; position: relative; transform: skewX(-20deg); overflow: hidden; }
    .hp-bar { height: 100%; transition: width 0.2s ease-out; max-width: 100%; }
    #p1-hp { background: linear-gradient(90deg, #e74c3c, #c0392b); width: 100%; float: left; }
    #p2-hp { background: linear-gradient(90deg, #2ecc71, #27ae60); width: 100%; float: right; }
    
    .p-name { color: #fff; font-weight: bold; font-size: 16px; margin-bottom: 2px; text-shadow: 1px 1px 0 #000; }
    
    /* COOLDOWN UI */
    .skill-row { display: flex; gap: 4px; margin-top: 5px; }
    .skill-box { 
      width: 32px; height: 32px; background: #222; border: 1px solid #777; 
      color: #fff; font-size: 10px; display: flex; justify-content: center; align-items: center; 
      position: relative; font-weight: bold; border-radius: 4px; overflow: hidden;
    }
    .cd-overlay { position: absolute; bottom: 0; left: 0; width: 100%; height: 0%; background: rgba(0,0,0,0.8); transition: height 0.1s linear; }
    #p1-con .skill-row { flex-direction: row-reverse; }

    #status-text { position: absolute; top: 60px; width: 100%; text-align: center; font-size: 20px; color: yellow; font-weight: bold; text-shadow: 2px 2px 0 #000; }
    
    #msg-overlay { position: absolute; top: 0; left: 0; width: 100%; height: 100%; background: rgba(0,0,0,0.95); display: flex; justify-content: center; align-items: center; color: #fff; font-size: 24px; font-weight: bold; flex-direction: column; z-index: 100; pointer-events: auto; }
    .hidden { display: none !important; }

    /* MOBILE CONTROLS */
    .controls { position: absolute; bottom: 10px; width: 100%; height: 100%; pointer-events: auto; display: none; pointer-events: none; }
    .dpad { position: absolute; left: 20px; bottom: 20px; width: 160px; height: 160px; pointer-events: auto; }
    .btn-move { width: 55px; height: 55px; position: absolute; border-radius: 50%; background: rgba(255,255,255,0.1); border: 2px solid rgba(255,255,255,0.3); color: white; display: flex; justify-content: center; align-items: center; font-size: 20px; backdrop-filter: blur(2px); }
    #btn-up { top: 0; left: 52px; } #btn-down { bottom: 0; left: 52px; font-size: 14px; } 
    #btn-left { top: 52px; left: 0; } #btn-right { top: 52px; right: 0; }
    
    .action-pad { position: absolute; right: 20px; bottom: 20px; width: 260px; height: 140px; display: grid; grid-template-columns: repeat(4, 1fr); gap: 5px; pointer-events: auto; }
    .btn-act { width: 100%; height: 50px; font-size: 11px; display: flex; flex-direction: column; justify-content: center; align-items: center; background: rgba(255,255,255,0.1); border: 1px solid rgba(255,255,255,0.3); border-radius: 8px; color: white; backdrop-filter: blur(2px); }
    .btn-act:active, .btn-move:active { background: rgba(255,255,255,0.4); transform: scale(0.95); }
  </style>
</head>
<body>
  <div id="rotate-lock"> <div style="font-size:50px">📱</div><h3>XOAY NGANG MÁY</h3> </div>
  <div id="game-container">
    <canvas id="gameCanvas"></canvas>
    <div id="ui-layer">
      <div id="status-text">WAITING...</div>
      <!-- P1 HUD -->
      <div id="p1-con" class="hud-container">
        <div class="p-name">P1 <span id="p1-val">500</span></div>
        <div class="hp-bar-bg"><div id="p1-hp" class="hp-bar"></div></div>
        <div class="skill-row" id="p1-skills"></div>
      </div>
      <!-- P2 HUD -->
      <div id="p2-con" class="hud-container">
        <div class="p-name"><span id="p2-val">500</span> P2</div>
        <div class="hp-bar-bg"><div id="p2-hp" class="hp-bar"></div></div>
        <div class="skill-row" id="p2-skills"></div>
      </div>
      
      <!-- Nút ảo cho điện thoại -->
      <div class="controls" id="mobile-ctrl">
        <div class="dpad">
          <div class="btn-move" id="btn-up">▲</div> <div class="btn-move" id="btn-down" ontouchstart="tryBlock(true)" ontouchend="tryBlock(false)">🛡️</div>
          <div class="btn-move" id="btn-left">◄</div> <div class="btn-move" id="btn-right">►</div>
        </div>
        <div class="action-pad">
          <div class="btn-act" style="background:rgba(0,255,255,0.2)" ontouchstart="tryAttack('U')">U<br>Stun</div>
          <div class="btn-act" style="background:rgba(255,0,255,0.2)" ontouchstart="tryAttack('I')">I<br>Hất</div>
          <div class="btn-act" style="background:rgba(255,215,0,0.2)" ontouchstart="tryAttack('O')">O<br>ULT</div>
          <div class="btn-act" style="visibility:hidden"></div>
          <div class="btn-act" ontouchstart="tryAttack('J')">J<br>Jab</div>
          <div class="btn-act" ontouchstart="tryAttack('K')">K<br>Kick</div>
          <div class="btn-act" ontouchstart="tryAttack('L')">L<br>Fire</div>
          <div class="btn-act" style="background:#555" ontouchstart="tryBlock(true)" ontouchend="tryBlock(false)">S<br>Block</div>
        </div>
      </div>
    </div>
    
    <div id="msg-overlay">
      <div>VÕ ĐÀI V2 ULTIMATE</div>
      <div style="font-size:16px; margin-top:10px; color:#aaa">PC: WASD (S: Đỡ) - JKL UIO (Đánh)</div>
      <button onclick="joinGame()" style="padding:15px 40px; font-size:22px; margin-top:30px; background:#e74c3c; color:white; border:none; border-radius:8px; cursor:pointer; font-weight:bold;">THAM CHIẾN</button>
      <a href="/" style="font-size:16px; margin-top:20px; color:#fff; text-decoration:underline;">Thoát</a>
    </div>
  </div>

<script>
  // --- HỆ THỐNG ÂM THANH SFX ---
  const audioCtx = new (window.AudioContext || window.webkitAudioContext)();
  
  function playTone(freq, type, duration, volume=0.1) {
    if(audioCtx.state === 'suspended') audioCtx.resume();
    const osc = audioCtx.createOscillator();
    const gain = audioCtx.createGain();
    osc.type = type;
    osc.frequency.setValueAtTime(freq, audioCtx.currentTime);
    osc.connect(gain);
    gain.connect(audioCtx.destination);
    osc.start();
    gain.gain.setValueAtTime(volume, audioCtx.currentTime);
    gain.gain.exponentialRampToValueAtTime(0.0001, audioCtx.currentTime + duration);
    osc.stop(audioCtx.currentTime + duration);
  }

  function sfx(name) {
    switch(name) {
      case 'hit': playTone(150, 'sawtooth', 0.1, 0.2); break; // Bị đánh
      case 'punch': playTone(300, 'square', 0.05, 0.1); break; // Đấm gió
      case 'fire': playTone(600, 'triangle', 0.3, 0.1); break; // Chưởng
      case 'block': playTone(80, 'square', 0.2, 0.3); break; // Đỡ đòn (Trầm)
      case 'jump': playTone(400, 'sine', 0.2, 0.1); break; // Nhảy
      case 'win': 
        [523, 659, 784, 1046].forEach((f, i) => setTimeout(()=>playTone(f,'square',0.2), i*150));
        break;
    }
  }

  const canvas = document.getElementById('gameCanvas');
  const ctx = canvas.getContext('2d');
  function resize() { canvas.width = window.innerWidth; canvas.height = window.innerHeight; }
  window.addEventListener('resize', resize); resize();
  if(/Android|iPhone|iPad|iPod/i.test(navigator.userAgent)){ document.getElementById('mobile-ctrl').style.display = 'block'; }

  // 1. CHẶN KÉO TRANG TRÊN MOBILE
  document.addEventListener('touchmove', function(e) { e.preventDefault(); }, { passive: false });

  const GRAVITY = 0.8;
  const GROUND_OFFSET = 50; 
  const MAX_HP = 500; 

  // --- CẤU HÌNH CHIÊU THỨC ---
  const SKILLS = {
    'J': { dmg: 15, cd: 400,  stun: 300,  color: '#ffff00', w: 80, h: 40, name: 'Jab', type:'melee' }, 
    'K': { dmg: 30, cd: 1500, stun: 200,  color: '#e67e22', w: 90, h: 50, name: 'Kick', type:'melee' }, 
    'L': { dmg: 25, cd: 1200, stun: 100,  color: '#e74c3c', type: 'projectile', speed: 12, r: 15, name: 'Fire' }, 
    'U': { dmg: 10, cd: 3000, stun: 1000, color: '#00ffff', w: 60, h: 60, name: 'Stun', type:'melee' }, 
    'I': { dmg: 20, cd: 2000, stun: 0,    color: '#9b59b6', w: 70, h: 120, name: 'Upper', type:'melee', lift: -15 }, 
    'O': { dmg: 80, cd: 8000, stun: 100,  color: '#ffd700', w: 250,h: 250, name: 'ULT', type:'melee' },
    'S': { cd: 2250, name: 'BLOCK', type:'block', color: '#3498db' }
  };

  function createSkillUI(pid) {
    let html = '';
    ['J','K','L','U','I','O','S'].forEach(k => {
      let bg = SKILLS[k].color;
      html += `<div class="skill-box" style="border-color:${bg}; color:${bg}">${k}<div class="cd-overlay" id="${pid}-cd-${k}"></div></div>`;
    });
    document.getElementById(pid + '-skills').innerHTML = html;
  }
  createSkillUI('p1'); createSkillUI('p2');

  function updateCooldownUI(prefix, key, percent) {
    let el = document.getElementById(`${prefix}-cd-${key}`);
    if(el) el.style.height = percent + '%';
  }

  class Projectile {
    constructor({x, y, velocity, color, owner}) {
      this.x = x; this.y = y; this.velocity = velocity; this.color = color; this.owner = owner; this.radius = 15;
    }
    draw() { 
      ctx.beginPath(); ctx.arc(this.x, this.y, this.radius, 0, Math.PI*2); ctx.fillStyle=this.color; ctx.fill(); 
      ctx.beginPath(); ctx.arc(this.x-this.velocity.x*1.5, this.y, this.radius*0.6, 0, Math.PI*2); ctx.fillStyle="rgba(255,255,255,0.5)"; ctx.fill();
    }
    update() { this.draw(); this.x += this.velocity.x; }
  }
  const projectiles = [];

  class Fighter {
    constructor(x, color, prefix, isFacingRight) {
      this.width = 50; this.height = 100;
      this.x = x; 
      this.y = canvas.height - GROUND_OFFSET - this.height;
      
      this.color = color; this.baseColor = color; this.prefix = prefix;
      this.velocity = {x: 0, y: 0}; this.isFacingRight = isFacingRight;
      this.health = MAX_HP; 
      
      this.isAttacking = false;
      this.attackBox = { position: {x:0, y:0}, width: 100, height: 50 };
      this.attackColor = 'white';
      
      this.isStunned = false;
      this.isBlocking = false; 
      
      this.lastSkillTime = { J:0, K:0, L:0, U:0, I:0, O:0, S:0 };
      this.currentSkillKey = 'J'; 
    }

    draw() {
      ctx.fillStyle = this.isStunned ? '#777' : (this.isBlocking ? '#3498db' : this.color);
      ctx.fillRect(this.x, this.y, this.width, this.height);
      if(this.isBlocking) { ctx.strokeStyle="#3498db"; ctx.lineWidth=5; ctx.strokeRect(this.x-10, this.y-5, this.width+20, this.height+10); }
      ctx.fillStyle = "white"; if(this.isFacingRight) ctx.fillRect(this.x+35, this.y+10, 10, 10); else ctx.fillRect(this.x+5, this.y+10, 10, 10);
      if(this.isAttacking) {
        ctx.fillStyle = this.attackColor; ctx.globalAlpha = 0.6;
        let attackX = this.isFacingRight ? this.x+this.width : this.x-this.attackBox.width;
        ctx.fillRect(attackX, this.y+20, this.attackBox.width, this.attackBox.height);
        ctx.globalAlpha = 1.0;
      }
    }

    update() {
      this.draw();
      if(!this.isStunned && !this.isBlocking) { this.x += this.velocity.x; this.y += this.velocity.y; }
      
      let groundY = canvas.height - GROUND_OFFSET;
      if (this.y + this.height + this.velocity.y >= groundY) { 
          this.velocity.y = 0; 
          this.y = groundY - this.height; 
      } else { 
          this.velocity.y += GRAVITY; 
      }
      
      if(this.x < 0) this.x = 0; if(this.x > canvas.width - this.width) this.x = canvas.width - this.width;

      for(let k in SKILLS) {
        let elapsed = Date.now() - this.lastSkillTime[k];
        let pct = (elapsed < SKILLS[k].cd) ? 100 - (elapsed / SKILLS[k].cd * 100) : 0;
        updateCooldownUI(this.prefix, k, pct);
      }
    }

    startBlock() {
      if(this.isStunned || this.isAttacking) return;
      if(Date.now() - this.lastSkillTime['S'] < SKILLS['S'].cd) return; 
      this.isBlocking = true;
      sfx('block'); // Âm thanh Block
    }
    stopBlock() { 
      if(this.isBlocking) { this.isBlocking = false; this.lastSkillTime['S'] = Date.now(); } 
    }

    performAttack(key) {
      if(this.isAttacking || this.isStunned || this.isBlocking) return;
      
      const skill = SKILLS[key];
      if(Date.now() - this.lastSkillTime[key] < skill.cd) return;
      this.lastSkillTime[key] = Date.now();
      
      this.currentSkillKey = key; 

      if(skill.type === 'projectile') {
         let spawnX = this.isFacingRight ? this.x + this.width + 20 : this.x - 20;
         projectiles.push(new Projectile({
            x: spawnX, y: this.y+30,
            velocity: { x: this.isFacingRight ? skill.speed : -skill.speed, y: 0 },
            color: skill.color, owner: this.prefix
         }));
         sfx('fire'); // Âm thanh chưởng
         return;
      }

      this.isAttacking = true;
      this.attackBox.width = skill.w;
      this.attackBox.height = skill.h;
      this.attackColor = skill.color;
      sfx('punch'); // Âm thanh đấm
      setTimeout(() => { this.isAttacking = false; }, 150);
    }

    takeHit(dmg, stun, push, lift) {
      if(this.isBlocking) { 
        this.x += this.isFacingRight?-15:15; 
        sfx('block'); // Âm thanh đỡ được
        return; 
      } 
      this.health -= dmg; 
      if(this.health < 0) this.health = 0;
      sfx('hit'); // Âm thanh trúng đòn
      
      this.color = 'white'; setTimeout(() => this.color = this.baseColor, 100);
      if(push > 0) this.velocity.x = this.isFacingRight ? -push : push;
      if(lift < 0) { this.velocity.y = lift; this.y -= 5; }
      if(stun > 0) { this.isStunned = true; setTimeout(() => this.isStunned = false, stun); }
    }
  }

  const p1 = new Fighter(100, '#ff4757', 'p1', true);
  const p2 = new Fighter(canvas.width - 200, '#2ed573', 'p2', false);
  let myRole = 0; let ws = new WebSocket('ws://' + location.host + '/ws');

  function checkCollision(atk, vic) {
    if(!atk.isAttacking) return false;
    let atkX = atk.isFacingRight ? atk.x+atk.width : atk.x-atk.attackBox.width;
    if (
      atkX + atk.attackBox.width >= vic.x &&
      atkX <= vic.x + vic.width &&
      atk.y + 20 + atk.attackBox.height >= vic.y &&
      atk.y + 20 <= vic.y + vic.height
    ) {
      atk.isAttacking = false; return true;
    }
    return false;
  }

  function animate() {
    window.requestAnimationFrame(animate);
    ctx.fillStyle = '#222'; ctx.fillRect(0, 0, canvas.width, canvas.height);
    ctx.fillStyle = '#444'; ctx.fillRect(0, canvas.height - 50, canvas.width, 50);
    p1.update(); p2.update();

    // Body Block
    if (p1.x < p2.x + p2.width && p1.x + p1.width > p2.x && p1.y < p2.y + p2.height && p1.y + p1.height > p2.y) {
        let overlap = (p1.width + p2.width)/2 - Math.abs((p1.x + p1.width/2) - (p2.x + p2.width/2));
        if(overlap>0){ if(p1.x<p2.x){p1.x-=overlap/2;p2.x+=overlap/2;}else{p1.x+=overlap/2;p2.x-=overlap/2;} }
    }

    // --- ĐỒNG BỘ DI CHUYỂN CHO CẢ 2 NGƯỜI CHƠI ---
    // (Thay vì chỉ chạy cho 'myRole', giờ chạy cho cả 2 dựa trên biến keys toàn cục đã được sync)
    
    // Logic P1
    if(!p1.isStunned && !p1.isBlocking){ 
        p1.velocity.x=0; 
        if(keys.a) {p1.velocity.x=-5; p1.isFacingRight=false;} 
        else if(keys.d) {p1.velocity.x=5; p1.isFacingRight=true;} 
    }
    
    // Logic P2
    if(!p2.isStunned && !p2.isBlocking){ 
        p2.velocity.x=0; 
        if(keys.left) {p2.velocity.x=-5; p2.isFacingRight=false;} 
        else if(keys.right) {p2.velocity.x=5; p2.isFacingRight=true;} 
    }

    projectiles.forEach((p, i) => {
        p.update();
        let target = (p.owner === 'p1') ? p2 : p1;
        if(p.x<0||p.x>canvas.width) { projectiles.splice(i, 1); } 
        else if(p.x+p.radius>=target.x && p.x-p.radius<=target.x+target.width && p.y>=target.y && p.y<=target.y+target.height) {
            projectiles.splice(i,1);
            let dmg = SKILLS['L'].dmg;
            if((p.owner=='p1'&&myRole==1)||(p.owner=='p2'&&myRole==2)) sendHit(target.prefix, dmg, 0, 10, 0); 
            target.takeHit(dmg, 0, 10, 0);
        }
    });

    if(myRole==1 && checkCollision(p1, p2)) { 
      let s=SKILLS[p1.currentSkillKey]; 
      sendHit('p2', s.dmg, s.stun, s.push, s.lift); 
      p2.takeHit(s.dmg, s.stun, s.push, s.lift); 
    }
    if(myRole==2 && checkCollision(p2, p1)) { 
      let s=SKILLS[p2.currentSkillKey]; 
      sendHit('p1', s.dmg, s.stun, s.push, s.lift); 
      p1.takeHit(s.dmg, s.stun, s.push, s.lift); 
    }
  }

  function sendHit(target, dmg, stun, push, lift) { 
     ws.send(`fv2:hit:${target}:${dmg}`); 
  }

  function tryAttack(k) { 
      if(myRole==1){ p1.performAttack(k); ws.send(`fv2:in:${k}:atk`); } 
      else{ p2.performAttack(k); ws.send(`fv2:in:${k}:atk`); } 
  }
  
  function tryBlock(isDown) { 
    let k = (myRole==1)?'S':'DOWN'; ws.send(`fv2:in:${k}:${isDown?'down':'up'}`);
    if(myRole==1) isDown?p1.startBlock():p1.stopBlock(); else isDown?p2.startBlock():p2.stopBlock();
  }

  const keys={a:false, d:false, left:false, right:false};
  function sendInput(key, type) { ws.send(`fv2:in:${key}:${type}`); }

  window.addEventListener('keydown', (e) => {
    let k = e.key.toUpperCase(); if(e.repeat) return;
    
    if(myRole==1){
      if(k=='A'){keys.a=true; ws.send('fv2:in:A:down');}
      if(k=='D'){keys.d=true; ws.send('fv2:in:D:down');}
      if(k=='W'&&p1.velocity.y==0){p1.velocity.y=-20; ws.send('fv2:in:W:down'); sfx('jump');}
      if(k=='S') tryBlock(true);
      if(SKILLS[k]) tryAttack(k);
    } else if(myRole==2){
      if(k=='A'||k=='ARROWLEFT'){keys.left=true; ws.send('fv2:in:LEFT:down');}
      if(k=='D'||k=='ARROWRIGHT'){keys.right=true; ws.send('fv2:in:RIGHT:down');}
      if(k=='W'||k=='ARROWUP'){if(p2.velocity.y==0){p2.velocity.y=-20; ws.send('fv2:in:UP:down'); sfx('jump');}}
      if(k=='S'||k=='ARROWDOWN') tryBlock(true);
      if(SKILLS[k]) tryAttack(k);
    }
  });

  window.addEventListener('keyup', (e) => {
    let k = e.key.toUpperCase();
    if(myRole==1){
      if(k=='A'){keys.a=false; ws.send('fv2:in:A:up');}
      if(k=='D'){keys.d=false; ws.send('fv2:in:D:up');}
      if(k=='S') tryBlock(false);
    } else if(myRole==2){
      if(k=='A'||k=='ARROWLEFT'){keys.left=false; ws.send('fv2:in:LEFT:up');}
      if(k=='D'||k=='ARROWRIGHT'){keys.right=false; ws.send('fv2:in:RIGHT:up');}
      if(k=='S'||k=='ARROWDOWN') tryBlock(false);
    }
  });

  document.getElementById('btn-left').addEventListener('touchstart', (e)=>{e.preventDefault(); if(myRole==1){keys.a=true;sendInput('A','down')}else{keys.left=true;sendInput('LEFT','down')}});
  document.getElementById('btn-left').addEventListener('touchend', (e)=>{e.preventDefault(); if(myRole==1){keys.a=false;sendInput('A','up')}else{keys.left=false;sendInput('LEFT','up')}});
  document.getElementById('btn-right').addEventListener('touchstart', (e)=>{e.preventDefault(); if(myRole==1){keys.d=true;sendInput('D','down')}else{keys.right=true;sendInput('RIGHT','down')}});
  document.getElementById('btn-right').addEventListener('touchend', (e)=>{e.preventDefault(); if(myRole==1){keys.d=false;sendInput('D','up')}else{keys.right=false;sendInput('RIGHT','up')}});
  document.getElementById('btn-up').addEventListener('touchstart', (e)=>{e.preventDefault(); if(myRole==1&&p1.velocity.y==0){p1.velocity.y=-20;sendInput('W','down');sfx('jump');} else if(myRole==2&&p2.velocity.y==0){p2.velocity.y=-20;sendInput('UP','down');sfx('jump');} });

  ws.onmessage = (evt) => {
    let msg = evt.data;
    if(msg.startsWith("fv2:welcome:")) { myRole=parseInt(msg.split(':')[2]); document.getElementById('msg-overlay').classList.add('hidden'); document.getElementById('status-text').innerText=(myRole==1)?"YOU ARE P1":"YOU ARE P2"; resize(); animate(); }
    if(msg.startsWith("fv2:update:")) { 
       let parts=msg.split(':'); let hp1 = parseInt(parts[2]); let hp2 = parseInt(parts[3]);
       let p1Pct = Math.max(0, Math.min(100, (hp1 / MAX_HP) * 100));
       let p2Pct = Math.max(0, Math.min(100, (hp2 / MAX_HP) * 100));
       document.getElementById('p1-hp').style.width = p1Pct + '%'; 
       document.getElementById('p2-hp').style.width = p2Pct + '%'; 
       if(document.getElementById('p1-val')) document.getElementById('p1-val').innerText = hp1;
       if(document.getElementById('p2-val')) document.getElementById('p2-val').innerText = hp2;
       if(hp1 <= 0 || hp2 <= 0) { sfx('win'); setTimeout(()=>location.reload(), 2000); }
    }
    if(msg.startsWith("fv2:sync:")) {
       let p=msg.split(':')[2], k=msg.split(':')[3], t=msg.split(':')[4];
       // Cập nhật biến 'keys' toàn cục để animate() xử lý di chuyển
       if(p=='p1' && myRole!=1){ 
          if(k=='A') keys.a=(t=='down'); 
          if(k=='D') keys.d=(t=='down'); 
          if(k=='W') { p1.velocity.y=-20; sfx('jump'); }
          if(k=='S') t=='down'?p1.startBlock():p1.stopBlock(); 
          if(t=='atk') p1.performAttack(k); 
       }
       if(p=='p2' && myRole!=2){ 
          if(k=='LEFT') keys.left=(t=='down'); 
          if(k=='RIGHT') keys.right=(t=='down'); 
          if(k=='UP') { p2.velocity.y=-20; sfx('jump'); }
          if(k=='DOWN') t=='down'?p2.startBlock():p2.stopBlock(); 
          if(t=='atk') p2.performAttack(k); 
       }
    }
    if(msg.startsWith("fv2:hit:")) {
       let parts=msg.split(':'); let target=parts[2], dmg=parseInt(parts[3]);
       if(target=='p1') p1.takeHit(0, 0, 10, 0);
       if(target=='p2') p2.takeHit(0, 0, 10, 0);
    }
  };
  function joinGame() { ws.send("fv2:join"); }
</script>
</body>
</html>
)rawliteral";