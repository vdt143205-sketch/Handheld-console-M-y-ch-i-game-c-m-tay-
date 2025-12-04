const char game_maze_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="vi">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1, maximum-scale=1, user-scalable=no">
  <title>Mê Cung Ma</title>
  <style>
    /* --- CẤU HÌNH CHUNG --- */
    body { 
      background: #000; color: white; margin: 0; overflow: hidden; 
      font-family: 'Segoe UI', sans-serif; 
      user-select: none; -webkit-user-select: none; 
      touch-action: none; /* Chặn cuộn trang trên điện thoại */
    }
    
    /* UI GAME */
    .header { 
      position: absolute; top: 0; left: 0; width: 100%; 
      padding: 10px; display: flex; justify-content: space-between; align-items: center; 
      pointer-events: none; z-index: 10; box-sizing: border-box;
    }
    .btn-ui { 
      pointer-events: auto; background: #333; color: #fff; 
      border: 1px solid #555; padding: 8px 15px; border-radius: 5px; 
      text-decoration: none; font-size: 14px; cursor: pointer; 
    }
    h2 { margin: 0; color: #00cec9; text-shadow: 0 0 10px #00cec9; font-size: 20px; }

    /* KHUNG MÊ CUNG */
    #game-area {
      position: relative; width: 100vw; height: 100vh;
      display: flex; justify-content: center; align-items: center;
      background: #000; /* Màu tường là đen */
    }

    /* SVG MÊ CUNG */
    svg { 
      width: 95vw; height: 80vh; /* To hơn để dễ di tay */
      max-width: 600px; max-height: 600px; 
      touch-action: none;
    }
    
    /* ĐƯỜNG ĐI (PATH) */
    .path { fill: #00ffff; stroke: none; } 
    .start-zone { fill: #00b894; } /* Vùng xanh lá */
    .end-zone { fill: #ff0000; }   /* Vùng đỏ */

    /* MÀN HÌNH JUMPSCARE */
    #scare-screen {
      display: none; position: fixed; top: 0; left: 0; width: 100%; height: 100%;
      background: black; z-index: 9999;
      justify-content: center; align-items: center;
    }
    #scare-img { width: 100%; height: 100%; object-fit: cover; }
    .flash-anim { animation: flash 0.1s infinite; }

    @keyframes flash {
      0% { opacity: 1; } 50% { opacity: 0.2; } 100% { opacity: 1; }
    }

    /* MODAL HƯỚNG DẪN */
    #help-modal {
      display: flex; position: fixed; top: 0; left: 0; width: 100%; height: 100%;
      background: rgba(0,0,0,0.9); z-index: 100;
      justify-content: center; align-items: center; flex-direction: column;
    }
    .box { 
      background: #222; padding: 20px; width: 85%; max-width: 350px; 
      border-radius: 15px; border: 1px solid #00cec9; text-align: center; 
    }
    .box h3 { color: #00cec9; margin-top: 0; }
    .box p { color: #ccc; font-size: 15px; line-height: 1.5; }
    .start-btn {
      background: #00b894; border: none; padding: 12px 30px; 
      color: white; font-weight: bold; border-radius: 25px; font-size: 18px;
      margin-top: 15px; cursor: pointer; box-shadow: 0 0 10px #00b894;
    }
  </style>
</head>
<body>

  <div class="header">
    <a href="/" class="btn-ui">🏠 Thoát</a>
    <h2 id="level-title">Level 1</h2>
    <button class="btn-ui" onclick="showHelp()">❓</button>
  </div>

  <div id="game-area">
    <svg id="maze-svg" viewBox="0 0 100 100"></svg>
  </div>

  <div id="scare-screen">
    <img id="scare-img" src="https://i.imgur.com/JjB4h60.jpg">
  </div>

  <div id="help-modal">
    <div class="box">
      <h3>LUẬT CHƠI</h3>
      <p>
        1. Giữ ngón tay vào vùng màu <b>XANH LÁ</b> để bắt đầu.<br>
        2. Kéo ngón tay đi theo đường màu xanh dương.<br>
        3. Tuyệt đối <b>KHÔNG</b> chạm vào vùng màu đen (tường).<br>
        4. Đi đến vùng màu <b>ĐỎ</b> để qua màn.
      </p>
      <button class="start-btn" onclick="closeHelp()">BẮT ĐẦU</button>
    </div>
  </div>

  <script>
    let currentLevel = 1;
    let isPlaying = false;
    const svg = document.getElementById('maze-svg');
    const title = document.getElementById('level-title');
    
    // Dữ liệu 3 màn chơi (Vẽ bằng SVG Path cho chính xác)
    const levels = {
      1: `<path class="path" d="M20,90 V20 H80 V30 H30 V90 Z" /> 
          <rect class="start-zone" x="20" y="85" width="10" height="15" />
          <rect class="end-zone" x="80" y="20" width="10" height="10" />`,
      
      2: `<path class="path" d="M10,90 V50 H80 V70 H30 V90 Z  M80,50 V20 H40 V30 H70 V50 Z" />
          <rect class="start-zone" x="10" y="85" width="20" height="15" />
          <rect class="end-zone" x="40" y="20" width="10" height="10" />`,
      
      3: `<path class="path" d="M10,95 V70 H80 V50 H20 V30 H80 V15 H90 V35 H30 V45 H90 V75 H20 V95 Z" />
          <rect class="start-zone" x="10" y="90" width="10" height="10" />
          <rect class="end-zone" x="80" y="10" width="10" height="10" />` 
    };

    // Âm thanh Jumpscare (Base64 ngắn)
    const screamSound = new Audio("data:audio/mp3;base64,//uQZAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAWgAAAAlkAAAL5AAAAAAAAAA0AAAAAAA="); 
    // Lưu ý: Do hạn chế bộ nhớ, đây chỉ là placeholder. 
    // Nếu có internet, code sẽ tự tải file mp3 thật ở hàm triggerJumpscare

    function loadLevel(lvl) {
      if (lvl > 3) return;
      currentLevel = lvl;
      title.innerText = "Level " + lvl;
      svg.innerHTML = levels[lvl];
      isPlaying = false;
      title.style.color = "#00cec9";
    }

    function showHelp() { document.getElementById('help-modal').style.display = 'flex'; isPlaying = false; }
    function closeHelp() { document.getElementById('help-modal').style.display = 'none'; }

    // --- CƠ CHẾ ĐIỀU KHIỂN TRÊN ĐIỆN THOẠI & MÁY TÍNH ---
    
    // Xử lý chung cho việc di chuyển (Touch hoặc Mouse)
    function handleMove(x, y) {
      // Tìm xem ngón tay đang chạm vào cái gì (elementFromPoint)
      let el = document.elementFromPoint(x, y);
      
      if (!el) return; // Không chạm gì cả

      // 1. Logic bắt đầu (Chạm vào ô xanh lá)
      if (el.classList.contains('start-zone')) {
        isPlaying = true;
        title.innerText = "ĐANG CHƠI...";
        title.style.color = "#fff";
        return;
      }

      if (!isPlaying) return; // Chưa bắt đầu thì không tính tiếp

      // 2. Logic Thua (Chạm vào Tường đen hoặc Body)
      // Nếu element KHÔNG PHẢI là path, start, end -> Thì là tường
      if (!el.classList.contains('path') && !el.classList.contains('start-zone') && !el.classList.contains('end-zone')) {
        gameOver();
      }

      // 3. Logic Thắng (Chạm vào ô đỏ)
      if (el.classList.contains('end-zone')) {
        winLevel();
      }
    }

    // Sự kiện Cảm ứng (Điện thoại)
    document.addEventListener('touchmove', function(e) {
      e.preventDefault(); // Chặn cuộn trang
      let touch = e.touches[0];
      handleMove(touch.clientX, touch.clientY);
    }, { passive: false });

    // Sự kiện Chuột (Máy tính)
    document.addEventListener('mousemove', function(e) {
      handleMove(e.clientX, e.clientY);
    });

    function gameOver() {
      if(!isPlaying) return;
      isPlaying = false;
      title.innerText = "THUA RỒI! CHẠM TƯỜNG!";
      title.style.color = "red";
      if(navigator.vibrate) navigator.vibrate(200);
      // Reset về Level 1 sau 1 giây
      setTimeout(() => loadLevel(1), 1000);
    }

    function winLevel() {
      if(!isPlaying) return;
      isPlaying = false;
      if (currentLevel < 3) {
        currentLevel++;
        loadLevel(currentLevel);
      } else {
        triggerJumpscare();
      }
    }

    function triggerJumpscare() {
      const scareScreen = document.getElementById('scare-screen');
      const scareImg = document.getElementById('scare-img');
      
      scareScreen.style.display = 'flex';
      scareImg.classList.add('flash-anim'); // Hiệu ứng chớp tắt

      // Phát tiếng hét (Dùng link online cho to rõ)
      let audio = new Audio('https://www.myinstants.com/media/sounds/scream_1.mp3');
      audio.volume = 1.0;
      audio.play().catch(e => console.log("Cần tương tác để phát âm thanh"));

      // Rung điện thoại mạnh
      if(navigator.vibrate) navigator.vibrate([100, 50, 100, 50, 1000]);

      // Sau 2.5 giây thì hiện thông báo
      setTimeout(() => {
        alert("HẾT HỒN CHƯA BẠN TÔI ƠI? 👻👻👻");
        location.href = "/";
      }, 2500);
    }

    // Khởi chạy
    loadLevel(1);

  </script>
</body>
</html>
)rawliteral";