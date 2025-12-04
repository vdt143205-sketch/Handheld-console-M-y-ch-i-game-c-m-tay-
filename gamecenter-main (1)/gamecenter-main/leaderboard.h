const char leaderboard_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="vi">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>BXH Rắn Săn Mồi</title>
  <style>
    body { font-family: 'Courier New', monospace; background: #000; color: #0f0; text-align: center; padding: 20px; }
    h1 { color: #0f0; text-shadow: 0 0 10px #0f0; margin-bottom: 30px; }
    
    .rank-table {
      width: 100%; max-width: 500px; margin: 0 auto;
      border-collapse: collapse; border: 2px solid #0f0;
      box-shadow: 0 0 20px #00ff0055;
    }
    .rank-table th { background: #003300; color: #fff; padding: 15px; font-size: 18px; border-bottom: 2px solid #0f0; }
    .rank-table td { padding: 15px; border-bottom: 1px solid #004400; font-size: 20px; font-weight: bold; }
    
    .top1 { color: gold; text-shadow: 0 0 5px gold; }
    .top2 { color: silver; }
    .top3 { color: #cd7f32; }
    
    .btn { display: inline-block; margin-top: 30px; padding: 12px 30px; background: #111; color: #0f0; text-decoration: none; border: 1px solid #0f0; border-radius: 5px; font-weight: bold; }
    .btn:hover { background: #0f0; color: #000; }
  </style>
</head>
<body>

  <h1>🏆 TOP SERVER SNAKE 🏆</h1>

  <table class="rank-table">
    <thead>
      <tr>
        <th>HẠNG</th>
        <th>TÊN</th>
        <th>ĐIỂM</th>
      </tr>
    </thead>
    <tbody id="list">
      <tr><td colspan="3">Đang tải dữ liệu...</td></tr>
    </tbody>
  </table>

  <a href="/" class="btn">🏠 VỀ TRANG CHỦ</a>

<script>
  var ws = new WebSocket('ws://' + location.host + '/ws');
  
  ws.onopen = function() {
    ws.send('leaderboard:get');
  };

  ws.onmessage = function(evt) {
    var msg = evt.data;
    // Format: "leaderboard:snake:Ten1,Diem1;Ten2,Diem2;..."
    if(msg.startsWith("leaderboard:snake:")) {
      let dataStr = msg.substring(18);
      let html = "";
      
      if(dataStr.length === 0) {
        html = "<tr><td colspan='3'>Chưa có dữ liệu</td></tr>";
      } else {
        let records = dataStr.split(';');
        
        records.forEach((rec, index) => {
           let parts = rec.split(',');
           let name = parts[0];
           let score = parts[1];
           let rankClass = "";
           
           if(index === 0) rankClass = "top1";
           else if(index === 1) rankClass = "top2";
           else if(index === 2) rankClass = "top3";
           
           html += `<tr class="${rankClass}">
              <td>#${index + 1}</td>
              <td>${name}</td>
              <td>${score}</td>
           </tr>`;
        });
      }
      document.getElementById('list').innerHTML = html;
    }
  };
</script>
</body>
</html>
)rawliteral";