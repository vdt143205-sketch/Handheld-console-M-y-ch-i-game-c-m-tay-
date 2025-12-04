const char game_led_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="vi"> <head>
  <meta charset="UTF-8"> <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>LED Controller</title>
  <style>
    body { 
      background: #111; 
      color: white; 
      text-align: center; 
      /* Font chữ chuẩn đẹp cho tiếng Việt */
      font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif; 
      padding-top: 50px; 
      margin: 0;
    }
    h2 { margin-bottom: 40px; color: #f1c40f; text-transform: uppercase; }
    
    .btn { 
      padding: 20px 40px; 
      font-size: 20px; 
      font-weight: bold;
      border-radius: 15px; 
      border: none; 
      cursor: pointer; 
      margin: 15px; 
      transition: transform 0.1s;
      min-width: 150px;
      color: white;
    }
    .btn:active { transform: scale(0.95); }
    
    .on { background: linear-gradient(45deg, #2ecc71, #27ae60); box-shadow: 0 5px #1e8449; }
    .off { background: linear-gradient(45deg, #e74c3c, #c0392b); box-shadow: 0 5px #922b21; }
    
    .home { 
      display: inline-block; 
      margin-top: 60px; 
      color: #3498db; 
      text-decoration: none; 
      font-size: 16px; 
      padding: 10px 20px;
      border: 1px solid #3498db;
      border-radius: 30px;
    }
    .home:hover { background: #3498db; color: white; }
  </style>
</head>
<body>
  <h2>💡 ĐIỀU KHIỂN ĐÈN</h2>
  
  <button class="btn on" onclick="send('on')">BẬT ĐÈN</button>
  <button class="btn off" onclick="send('off')">TẮT ĐÈN</button>
  
  <br>
  <a href="/" class="home">⬅ Quay về trang chủ</a>

<script>
  var ws = new WebSocket('ws://' + location.host + '/ws');
  
  // Thêm phản hồi rung khi bấm nút (nếu dùng điện thoại)
  function send(cmd) { 
    ws.send('led:' + cmd); 
    if(navigator.vibrate) navigator.vibrate(50);
  } 
</script>
</body>
</html>
)rawliteral";