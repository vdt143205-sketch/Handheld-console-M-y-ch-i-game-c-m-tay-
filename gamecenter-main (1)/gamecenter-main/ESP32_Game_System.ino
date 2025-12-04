#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <vector>
#include <algorithm> 

// --- 1. NHÚNG GIAO DIỆN ---
#include "index.h"
#include "game_race.h"
#include "game_fight.h"
#include "game_reflex.h"
#include "game_maze.h"
#include "game_simon.h"     
#include "game_fight_v2.h"  
#include "game_led.h"
#include "leaderboard.h"
#include "game_shooter.h"
#include "game_snake.h"

// --- 2. CẤU HÌNH ---
const char* ssid = "ESP32_Game_Center";
const char* password = "12345678";

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

// --- 3. CẤU TRÚC DỮ LIỆU BẢNG XẾP HẠNG RẮN ---
struct SnakeRecord { String name; int score; };
std::vector<SnakeRecord> snake_highscores;
bool compareSnake(SnakeRecord a, SnakeRecord b) { return a.score > b.score; }

// --- BIẾN TOÀN CỤC ---
int player_wins[4] = {0}; 

#define MAX_RACE_PLAYERS 4
uint32_t race_players[MAX_RACE_PLAYERS]={0}; int race_scores[MAX_RACE_PLAYERS]={0}; bool race_ended=false;
int fight_moves[2]={0}; int fight_hp[2]={100,100}; uint32_t fight_ids[2]={0};
uint32_t reflex_ids[4]={0}; int reflex_scores[4]={0}; int target_color=-1;

// [ĐÃ SỬA LỖI TẠI ĐÂY: Tách dòng #define ra riêng]
unsigned long reflex_timer=0; 
bool reflex_round_active=false; 
#define WIN_REFLEX_SCORE 5

std::vector<int> simon_sequence; int simon_step=0; bool simon_running=false; bool simon_waiting_input=false;
uint32_t fv2_players[2]={0}; int fv2_hp[2]={500,500};
#define LED_PIN 2
uint32_t gamepad_id=0;

void addWin(int i){
  if(i>=0 && i<4) {
    player_wins[i]++;
    Serial.printf("P%d Win! Total: %d\n", i+1, player_wins[i]);
  }
}

// --- LOGIC GAME ---
void broadcastFightState(String t){String m1="none",m2="none";if(fight_moves[0]&&fight_moves[1]){m1="shown";m2="shown";}ws.textAll("fight:update:"+String(fight_hp[0])+","+String(fight_hp[1])+","+m1+","+m2+","+t);}
void checkFightResult(){if(fight_moves[0]&&fight_moves[1]){int p1=fight_moves[0],p2=fight_moves[1];String t="";int d=20;if(p1==p2)t="HOA!";else if((p1==1&&p2==3)||(p1==2&&p2==1)||(p1==3&&p2==2)){t="P1 HIT";fight_hp[1]-=d;}else{t="P2 HIT";fight_hp[0]-=d;}if(fight_hp[0]<=0){fight_hp[0]=0;ws.textAll("fight:end:1");addWin(1);}else if(fight_hp[1]<=0){fight_hp[1]=0;ws.textAll("fight:end:0");addWin(0);}broadcastFightState(t);fight_moves[0]=0;fight_moves[1]=0;}}
void broadcastReflexScores(){String s="reflex:scores:";for(int i=0;i<4;i++)s+=String(reflex_scores[i])+(i<3?",":"");ws.textAll(s);}
void startReflexRound(){target_color=random(0,4);reflex_round_active=true;ws.textAll("reflex:show:"+String(target_color));}
void resetReflexGame(){for(int i=0;i<4;i++)reflex_scores[i]=0;reflex_round_active=false;reflex_timer=millis()+3000;ws.textAll("reflex:wait");broadcastReflexScores();}
void startSimon(){simon_sequence.clear();simon_running=true;nextSimonLevel();}
void nextSimonLevel(){simon_step=0;simon_waiting_input=false;simon_sequence.push_back(random(0,4));String s="simon:play:";for(int i=0;i<simon_sequence.size();i++){s+=String(simon_sequence[i]);if(i<simon_sequence.size()-1)s+=",";}ws.textAll(s);ws.textAll("simon:score:"+String(simon_sequence.size()));simon_waiting_input=true;}
void handleSimonInput(int b){if(!simon_running||!simon_waiting_input)return;if(b==simon_sequence[simon_step]){simon_step++;if(simon_step>=simon_sequence.size()){simon_waiting_input=false;ws.textAll("simon:score:"+String(simon_sequence.size()));delay(1000);nextSimonLevel();if(simon_sequence.size()>5)addWin(0);}}else{simon_running=false;ws.textAll("simon:over");ws.textAll("simon:score:GAME OVER");}}

void onEvent(AsyncWebSocket *s, AsyncWebSocketClient *c, AwsEventType t, void *arg, uint8_t *data, size_t len){
  if(t==WS_EVT_CONNECT){
    int rf=-1;for(int i=0;i<4;i++)if(reflex_ids[i]==0){reflex_ids[i]=c->id();rf=i;break;}if(rf!=-1)c->text("reflex:welcome:"+String(rf));
    int f=-1;if(fight_ids[0]==0){fight_ids[0]=c->id();f=0;}else if(fight_ids[1]==0){fight_ids[1]=c->id();f=1;}if(f!=-1)c->text("fight:welcome:"+String(f));
    int r=-1;for(int i=0;i<MAX_RACE_PLAYERS;i++)if(race_players[i]==0){race_players[i]=c->id();r=i;break;}if(r!=-1)c->text("welcome,"+String(r));
    int fv2=0;if(fv2_players[0]==0){fv2_players[0]=c->id();fv2=1;}else if(fv2_players[1]==0){fv2_players[1]=c->id();fv2=2;}if(fv2>0)c->text("fv2:welcome:"+String(fv2));
  }
  else if(t==WS_EVT_DISCONNECT){
    if(c->id()==gamepad_id)gamepad_id=0;
    for(int i=0;i<4;i++)if(reflex_ids[i]==c->id()){reflex_ids[i]=0;reflex_scores[i]=0;}
    if(fight_ids[0]==c->id())fight_ids[0]=0;if(fight_ids[1]==c->id())fight_ids[1]=0;
    for(int i=0;i<MAX_RACE_PLAYERS;i++)if(race_players[i]==c->id()){race_players[i]=0;race_scores[i]=0;}
    if(fv2_players[0]==c->id())fv2_players[0]=0;if(fv2_players[1]==c->id())fv2_players[1]=0;
  }
  else if(t==WS_EVT_DATA){
    String msg="";for(size_t i=0;i<len;i++)msg+=(char)data[i]; msg.trim();
    if(msg=="IAM_GAMEPAD"){gamepad_id=c->id();reflex_ids[0]=c->id();fight_ids[0]=c->id();race_players[0]=c->id();fv2_players[0]=c->id();return;}
    
    // LEADERBOARD
    if(msg=="leaderboard:get"){
      String d = "leaderboard:data:" + String(player_wins[0]) + "," + String(player_wins[1]) + "," + String(player_wins[2]) + "," + String(player_wins[3]);
      if(snake_highscores.size() > 0) {
        d += "|leaderboard:snake:";
        for(int i=0; i<snake_highscores.size(); i++) { d += snake_highscores[i].name + "," + String(snake_highscores[i].score); if(i < snake_highscores.size()-1) d += ";"; }
      }
      c->text(d); return;
    }

    // SNAKE RECORD
    if(msg.startsWith("snake:record:")) {
       int idx1 = msg.indexOf(':', 13); String pName = msg.substring(13, idx1); int pScore = msg.substring(idx1 + 1).toInt();
       SnakeRecord newRec = {pName, pScore}; snake_highscores.push_back(newRec);
       std::sort(snake_highscores.begin(), snake_highscores.end(), compareSnake);
       if(snake_highscores.size() > 5) snake_highscores.resize(5);
       return;
    }
    if(msg=="shooter:win") { addWin(0); return; }
    if(msg=="snake:win") { addWin(0); return; }

    // TAY CẦM
    if(msg.startsWith("pad:")){
      ws.textAll(msg); 
      String cmd=msg.substring(4); int btn=cmd.toInt(); if(cmd=="fire")btn=99;
      if(btn>=0 && btn<=3) ws.textAll("snake:input:"+String(btn));
      if(simon_running&&btn>=0&&btn<=3)handleSimonInput(btn);
      if(reflex_round_active&&btn>=0&&btn<=3){if(btn==target_color){reflex_scores[0]++;reflex_round_active=false;reflex_timer=millis()+3000;ws.textAll("reflex:wait");broadcastReflexScores();if(reflex_scores[0]>=WIN_REFLEX_SCORE){ws.textAll("reflex:win:0");addWin(0);}}else{reflex_scores[0]--;broadcastReflexScores();}}
      if(btn>=0&&btn<=2){if(btn==0)fight_moves[0]=1;if(btn==1)fight_moves[0]=2;if(btn==2)fight_moves[0]=3;checkFightResult();}
      if(!race_ended&&(btn>=0||btn==99)){race_scores[0]++;if(race_scores[0]>=100){race_ended=true;ws.textAll("win,0");addWin(0);}else{String u="update";for(int j=0;j<4;j++)u+=","+String(race_scores[j]);ws.textAll(u);}}
      return;
    }

    // FIGHT V2
    if(msg.startsWith("fv2:")){
       String cmd=msg.substring(4);
       if(cmd=="join"){int r=0;if(fv2_players[0]==0){fv2_players[0]=c->id();r=1;}else if(fv2_players[1]==0){fv2_players[1]=c->id();r=2;}c->text("fv2:welcome:"+String(r));if(r==1){fv2_hp[0]=500;fv2_hp[1]=500;ws.textAll("fv2:update:500:500");}}
       else if(cmd.startsWith("in:")){String ct=cmd.substring(3);String p=(c->id()==fv2_players[0])?"p1":"p2";ws.textAll("fv2:sync:"+p+":"+ct);}
       else if(cmd.startsWith("hit:")){
          int c1=cmd.indexOf(':'),c2=cmd.indexOf(':',c1+1),c3=cmd.indexOf(':',c2+1);
          String t=cmd.substring(c1+1,c2); int d=cmd.substring(c2+1,c3==-1?cmd.length():c3).toInt();
          if(t=="p1")fv2_hp[0]-=d;if(t=="p2")fv2_hp[1]-=d;
          ws.textAll("fv2:update:"+String(fv2_hp[0])+":"+String(fv2_hp[1])); ws.textAll("fv2:"+cmd);
          if(fv2_hp[0]<=0){addWin(1);fv2_hp[0]=500;fv2_hp[1]=500;} if(fv2_hp[1]<=0){addWin(0);fv2_hp[0]=500;fv2_hp[1]=500;}
       }
       return;
    }
    
    // WEB CLIENT LOGIC
    if(msg.startsWith("simon:")) { String cmdBody=msg.substring(6); if(cmdBody=="start") startSimon(); else handleSimonInput(cmdBody.toInt()); }
    
    if(msg.startsWith("reflex:")) { 
        String cmdBody=msg.substring(7); 
        if(cmdBody=="reset") resetReflexGame(); 
        else if(reflex_round_active){
            int p=cmdBody.toInt(); int id=-1; for(int i=0;i<4;i++) if(reflex_ids[i]==c->id()) id=i;
            if(id!=-1){ if(p==target_color){reflex_scores[id]++;reflex_round_active=false;reflex_timer=millis()+3000;ws.textAll("reflex:wait");broadcastReflexScores();if(reflex_scores[id]>=WIN_REFLEX_SCORE){ws.textAll("reflex:win:"+String(id));addWin(id);}} else{reflex_scores[id]--;broadcastReflexScores();}} } 
        } 
    
    if(msg.startsWith("fight:")) { 
        String cmdBody=msg.substring(6); int id=(c->id()==fight_ids[0])?0:(c->id()==fight_ids[1])?1:-1; 
        if(id!=-1){ if(cmdBody=="rock")fight_moves[id]=1;else if(cmdBody=="paper")fight_moves[id]=2;else if(cmdBody=="scissors")fight_moves[id]=3;else if(cmdBody=="reset"){fight_hp[0]=100;fight_hp[1]=100;fight_moves[0]=0;fight_moves[1]=0;broadcastFightState("RESET");} checkFightResult(); } 
    }
    
    if(msg=="tap"&&!race_ended){ for(int i=0;i<4;i++)if(race_players[i]==c->id()){race_scores[i]++;if(race_scores[i]>=100){race_ended=true;ws.textAll("win,"+String(i));addWin(i);}else{String u="update";for(int j=0;j<4;j++)u+=","+String(race_scores[j]);ws.textAll(u);}break;} }
    if(msg=="reset"){for(int i=0;i<4;i++)race_scores[i]=0;race_ended=false;ws.textAll("update,0,0,0,0");}
    if(msg.startsWith("led:")){if(msg=="led:on")digitalWrite(LED_PIN,HIGH);if(msg=="led:off")digitalWrite(LED_PIN,LOW);}
  }
}

void setup(){
  Serial.begin(115200); pinMode(LED_PIN,OUTPUT); WiFi.setSleep(false); WiFi.softAP(ssid,password); Serial.println(WiFi.softAPIP());
  ws.onEvent(onEvent); server.addHandler(&ws);
  server.on("/",HTTP_GET,[](AsyncWebServerRequest*r){r->send_P(200,"text/html",index_html);});
  server.on("/race",HTTP_GET,[](AsyncWebServerRequest*r){r->send_P(200,"text/html",game_html);});
  server.on("/fight",HTTP_GET,[](AsyncWebServerRequest*r){r->send_P(200,"text/html",game_fight_html);});
  server.on("/reflex",HTTP_GET,[](AsyncWebServerRequest*r){r->send_P(200,"text/html",game_reflex_html);});
  server.on("/led",HTTP_GET,[](AsyncWebServerRequest*r){r->send_P(200,"text/html",game_led_html);});
  server.on("/maze",HTTP_GET,[](AsyncWebServerRequest*r){r->send_P(200,"text/html",game_maze_html);});
  server.on("/simon",HTTP_GET,[](AsyncWebServerRequest*r){r->send_P(200,"text/html",game_simon_html);});
  server.on("/fight_v2",HTTP_GET,[](AsyncWebServerRequest*r){r->send_P(200,"text/html",game_fight_v2_html);});
  server.on("/leaderboard",HTTP_GET,[](AsyncWebServerRequest*r){r->send_P(200,"text/html",leaderboard_html);});
  server.on("/shooter",HTTP_GET,[](AsyncWebServerRequest*r){r->send_P(200,"text/html",game_shooter_html);});
  server.on("/snake",HTTP_GET,[](AsyncWebServerRequest*r){r->send_P(200,"text/html",game_snake_html);});
  server.begin(); reflex_timer=millis()+3000;
}
void loop(){ws.cleanupClients();if(!reflex_round_active&&millis()>reflex_timer&&reflex_timer>0){startReflexRound();reflex_timer=0;}delay(1);}