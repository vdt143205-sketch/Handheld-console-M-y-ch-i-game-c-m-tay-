const char game_simon_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1, user-scalable=no">
  <title>Vua Trí Nhớ</title>
  <style>
    body { font-family: 'Segoe UI', sans-serif; background: #222; color: white; text-align: center; user-select: none; touch-action: manipulation; }
    
    h2 { color: #ffa502; margin-bottom: 5px; text-transform: uppercase; text-shadow: 0 0 10px #e67e22; }
    
    /* Bàn chơi Simon hình tròn chia 4 */
    .simon-board {
      display: grid; grid-template-columns: 1fr 1fr; gap: 15px;
      width: 320px; height: 320px; margin: 20px auto;
      border-radius: 50%; padding: 10px; background: #333;
      box-shadow: 0 10px 30px rgba(0,0,0,0.5);
      position: relative;
    }
    
    /* Các nút màu */
    .btn { 
      width: 100%; height: 100%; 
      opacity: 0.5; cursor: pointer; 
      transition: all 0.1s; border: none;
      -webkit-tap-highlight-color: transparent;
    }
    
    /* Hiệu ứng khi sáng lên */
    .btn.active, .btn:active { opacity: 1; filter: brightness(1.5); transform: scale(0.95); box-shadow: 0 0 20px currentColor; }
    
    /* Màu sắc khớp với tay cầm Uno/ESP32 */
    #b0 { background: #ff4757; border-top-left-radius: 100%; border: 4px solid #ff4757; color: #ff4757; } /* Đỏ (A) */
    #b1 { background: #2ed573; border-top-right-radius: 100%; border: 4px solid #2ed573; color: #2ed573; } /* Xanh Lá (B) */
    #b3 { background: #ffa502; border-bottom-left-radius: 100%; border: 4px solid #ffa502; color: #ffa502; } /* Vàng (D - Trái) */
    #b2 { background: #1e90ff; border-bottom-right-radius: 100%; border: 4px solid #1e90ff; color: #1e90ff; } /* Lam (C - Xuống) */
    
    /* Trung tâm hiển thị Level */
    .center-circle {
      position: absolute; top: 50%; left: 50%; transform: translate(-50%, -50%);
      width: 100px; height: 100px; background: #222; border-radius: 50%;
      border: 5px solid #444; display: flex; flex-direction: column;
      justify-content: center; align-items: center; z-index: 10;
    }
    #level-txt { font-size: 30px; font-weight: bold; color: white; }
    .label { font-size: 10px; color: #aaa; text-transform: uppercase; }

    #status { font-size: 20px; font-weight: bold; margin: 15px 0; min-height: 30px; color: #f1c40f; }
    
    /* Nút điều khiển */
    .ctrl-btn { 
      padding: 12px 30px; font-size: 18px; border-radius: 30px; border: none; 
      background: linear-gradient(45deg, #ffa502, #ff7f50); color: #fff; font-weight: bold; 
      cursor: pointer; margin-top: 10px; box-shadow: 0 5px 15px rgba(255, 165, 2, 0.4);
    }
    .ctrl-btn:active { transform: translateY(2px); }
    
    .home-link { display: block; margin-top: 20px; color: #7f8c8d; text-decoration: none; font-size: 14px; }
  </style>
</head>
<body>

  <h2>🧠 VUA TRÍ NHỚ</h2>
  <div id="status">Bấm Bắt Đầu để chơi</div>

  <div class="simon-board">
    <div id="b0" class="btn" onclick="send(0)"></div> <!-- Đỏ -->
    <div id="b1" class="btn" onclick="send(1)"></div> <!-- Xanh Lá -->
    <div id="b3" class="btn" onclick="send(3)"></div> <!-- Vàng -->
    <div id="b2" class="btn" onclick="send(2)"></div> <!-- Lam -->
    
    <div class="center-circle">
      <div class="label">Level</div>
      <div id="level-txt">--</div>
    </div>
  </div>

  <button class="ctrl-btn" onclick="startGame()">BẮT ĐẦU GAME</button>
  <br>
  <a href="/" class="home-link">🏠 Quay về Sảnh</a>

<script>
  // --- 1. HỆ THỐNG ÂM THANH (Web Audio API) ---
  const audioCtx = new (window.AudioContext || window.webkitAudioContext)();
  
  function playTone(freq, duration) {
    if(audioCtx.state === 'suspended') audioCtx.resume(); // Mở khóa âm thanh
    const osc = audioCtx.createOscillator(); 
    const gain = audioCtx.createGain();
    
    osc.type = 'sine';
    osc.frequency.value = freq; 
    
    osc.connect(gain); 
    gain.connect(audioCtx.destination);
    
    osc.start(); 
    // Giảm âm lượng dần để tiếng kêu êm tai hơn
    gain.gain.setValueAtTime(0.1, audioCtx.currentTime);
    gain.gain.exponentialRampToValueAtTime(0.00001, audioCtx.currentTime + duration);
    
    osc.stop(audioCtx.currentTime + duration);
  }
  
  // Tần số nốt nhạc cho 4 màu: Do, Mi, Sol, Do(cao)
  const tones = [261.63, 329.63, 392.00, 523.25]; 

  // --- 2. KẾT NỐI SERVER ---
  let ws = new WebSocket('ws://' + location.host + '/ws');
  
  ws.onmessage = (evt) => {
    let msg = evt.data;
    if(msg.startsWith("simon:")) {
      let parts = msg.split(':');
      let cmd = parts[1];

      // A. Server bắt đầu phát chuỗi: "simon:play:0,2,1..."
      if(cmd === 'play') { 
        let seq = parts[2].split(',').map(Number);
        document.getElementById('status').innerText = "👀 QUAN SÁT!";
        document.getElementById('status').style.color = "#f1c40f"; // Vàng
        playSequence(seq);
      } 
      
      // B. Cập nhật điểm/Level: "simon:score:5"
      else if(cmd === 'score') { 
        let lvl = parts[2];
        if(lvl !== "GAME OVER") {
           document.getElementById('level-txt').innerText = lvl;
           document.getElementById('status').innerText = "✅ CHÍNH XÁC! TIẾP TỤC...";
           document.getElementById('status').style.color = "#2ecc71"; // Xanh
        }
      }
      
      // C. Game Over: "simon:over"
      else if(cmd === 'over') { 
        document.getElementById('status').innerText = "💀 SAI RỒI! GAME OVER";
        document.getElementById('status').style.color = "#ff4757"; // Đỏ
        playTone(100, 1.0); // Âm thanh "Buzz" thua cuộc
        
        // Hiệu ứng màn hình đỏ
        document.body.style.background = "#500";
        setTimeout(() => document.body.style.background = "#222", 500);
      }
    }
  };

  // --- 3. LOGIC TRÌNH DIỄN (PLAYBACK) ---
  async function playSequence(seq) {
    // Khóa không cho người chơi bấm khi máy đang chạy
    document.querySelector('.simon-board').style.pointerEvents = 'none';
    
    await new Promise(r => setTimeout(r, 1000)); // Nghỉ 1s trước khi phát
    
    for (let i = 0; i < seq.length; i++) {
      await flashButton(seq[i]); // Sáng đèn + Kêu
      await new Promise(r => setTimeout(r, 300)); // Nghỉ giữa các nốt
    }
    
    // Mở khóa cho người chơi bấm
    document.getElementById('status').innerText = "👉 ĐẾN LƯỢT BẠN!";
    document.getElementById('status').style.color = "#fff";
    document.querySelector('.simon-board').style.pointerEvents = 'auto'; 
  }

  // Hàm làm sáng nút + phát tiếng
  function flashButton(id) {
    return new Promise(resolve => {
      let btn = document.getElementById('b' + id);
      btn.classList.add('active'); // Thêm class CSS để sáng lên
      playTone(tones[id], 0.4);    // Phát tiếng
      
      setTimeout(() => {
        btn.classList.remove('active'); // Tắt sáng
        resolve();
      }, 400); // Thời gian sáng
    });
  }

  // --- 4. TƯƠNG TÁC NGƯỜI CHƠI ---
  function send(id) {
    ws.send('simon:' + id); // Gửi lựa chọn lên Server
    flashButton(id);        // Hiệu ứng phản hồi ngay lập tức
  }

  function startGame() {
    ws.send('simon:start'); // Gửi lệnh bắt đầu game
    document.getElementById('level-txt').innerText = "1";
  }
</script>
</body>
</html>
)rawliteral";