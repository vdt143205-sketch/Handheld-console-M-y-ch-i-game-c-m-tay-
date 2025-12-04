const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="vi">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>ESP32 Game Center</title>
  <style>
    body { font-family: 'Segoe UI', sans-serif; background: #121212; color: white; margin: 0; padding: 20px; text-align: center; }
    h1 { color: #f1c40f; text-transform: uppercase; margin-bottom: 20px; text-shadow: 0 0 15px #d35400; }
    .game-grid { display: grid; grid-template-columns: repeat(auto-fit, minmax(150px, 1fr)); gap: 15px; max-width: 800px; margin: 0 auto; }
    .game-card { background: #1e1e1e; border-radius: 15px; padding: 15px; border: 1px solid #333; display: flex; flex-direction: column; align-items: center; }
    .icon { font-size: 40px; margin-bottom: 10px; }
    h3 { margin: 5px 0; font-size: 16px; }
    .btn-play { text-decoration: none; padding: 8px 20px; border-radius: 20px; font-weight: bold; width: 80%; color: white; display: inline-block; margin-top: 5px; }
    
    .race { background: linear-gradient(45deg, #e74c3c, #c0392b); }
    .fight { background: linear-gradient(45deg, #8e44ad, #9b59b6); }
    .reflex { background: linear-gradient(45deg, #00b894, #00cec9); }
    .maze { background: linear-gradient(45deg, #636e72, #2d3436); }
    .simon { background: linear-gradient(45deg, #e67e22, #d35400); }
    .fightv2 { background: linear-gradient(45deg, #34495e, #2c3e50); border: 1px solid #7f8c8d; }
    .shoot { background: linear-gradient(45deg, #2980b9, #3498db); }
    .snake { background: linear-gradient(45deg, #27ae60, #2ecc71); }
    .rank { background: linear-gradient(45deg, #f1c40f, #f39c12); color: #000; }
  </style>
</head>
<body>
  <h1>🎮 ESP32 ARCADE</h1>
  <div class="game-grid">
    <div class="game-card"> <div class="icon">🏎️</div> <h3>ĐUA XE</h3> <a href="/race" class="btn-play race">CHƠI</a> </div>
    <div class="game-card"> <div class="icon">⚔️</div> <h3>ĐẠI CHIẾN</h3> <a href="/fight" class="btn-play fight">CHƠI</a> </div>
    <div class="game-card"> <div class="icon">⚡</div> <h3>PHẢN XẠ</h3> <a href="/reflex" class="btn-play reflex">CHƠI</a> </div>
    <div class="game-card"> <div class="icon">👻</div> <h3>MÊ CUNG</h3> <a href="/maze" class="btn-play maze">CHƠI</a> </div>
    <div class="game-card"> <div class="icon">🧠</div> <h3>TRÍ NHỚ</h3> <a href="/simon" class="btn-play simon">CHƠI</a> </div>
    <div class="game-card"> <div class="icon">🥊</div> <h3>VÕ ĐÀI V2</h3> <a href="/fight_v2" class="btn-play fightv2">CHƠI</a> </div>
    <div class="game-card"> <div class="icon">🚀</div> <h3>PHI THUYỀN</h3> <a href="/shooter" class="btn-play shoot">CHƠI</a> </div>
    <div class="game-card"> <div class="icon">🐍</div> <h3>RẮN NEON</h3> <a href="/snake" class="btn-play snake">CHƠI</a> </div>
    <div class="game-card" style="border-color:gold"> <div class="icon">🏆</div> <h3>XẾP HẠNG</h3> <a href="/leaderboard" class="btn-play rank">XEM</a> </div>
  </div>
</body>
</html>
)rawliteral";