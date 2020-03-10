#include <iostream>
#include <pqxx/pqxx>
#include "assert.h"
#include "exerciser.h"
#include <string>
#include <fstream>
#include <sstream>

using namespace std;
using namespace pqxx;

connection* connect(){
  connection *C;
  try{
    //Establish a connection to the database
    //Parameters: database name, user name, user password
    C = new connection("dbname=ACC_BBALL user=postgres password=passw0rd");
    if (C->is_open()) {
      return C;
    } else {
      return NULL;
    }
  } catch (const std::exception &e){
    cerr << e.what() << std::endl;
    return NULL;
  }
}

void allDrop(connection* C){
  string sql = "DROP TABLE IF EXISTS PLAYER CASCADE;"\
          "DROP TABLE IF EXISTS TEAM CASCADE;" \
          "DROP TABLE IF EXISTS STATE CASCADE;" \
          "DROP TABLE IF EXISTS COLOR CASCADE;"; 
  work W(*C);
  W.exec(sql);
  W.commit();
}

void createPlayer(connection* C){
  string sql = "CREATE TABLE PLAYER("\
    "PLAYER​_​ID SERIAL NOT NULL,"\
    "TEAM_ID INT NOT NULL,"\
    "UNIFORM_NUM INT NOT NULL,"\
    "FIRST_NAME VARCHAR(256) NOT NULL,"\
    "LAST_NAME VARCHAR(256) NOT NULL,"\
    "MPG INT NOT NULL,"\
    "PPG INT NOT NULL,"\
    "RPG INT NOT NULL,"\
    "APG INT NOT NULL,"\
    "SPG FLOAT8 NOT NULL,"\
    "BPG FLOAT8 NOT NULL,"\
    "PRIMARY KEY (PLAYER​_​ID),"\
    "FOREIGN KEY (TEAM_ID) REFERENCES TEAM(TEAM_ID) ON DELETE SET NULL ON UPDATE CASCADE"\
  ");";
  work W(*C);
  W.exec(sql);
  W.commit();
}

void createTeam(connection* C){
  string sql = "CREATE TABLE TEAM("\
    "TEAM_ID SERIAL NOT NULL,"\
    "NAME VARCHAR(256) NOT NULL,"\
    "STATE_ID INT NOT NULL,"\
    "COLOR_ID INT NOT NULL,"\
    "WINS INT NOT NULL,"\
    "LOSSES INT NOT NULL,"\
    "PRIMARY KEY (TEAM_ID),"\
    "FOREIGN KEY (STATE_ID) REFERENCES STATE(STATE_ID) ON DELETE SET NULL ON UPDATE CASCADE,"\
    "FOREIGN KEY (COLOR_ID) REFERENCES COLOR(COLOR_ID) ON DELETE SET NULL ON UPDATE CASCADE"\
  ");";
  work W(*C);
  W.exec(sql);
  W.commit();
}

void createState(connection* C){
  string sql = "CREATE TABLE STATE("\
    "STATE_ID SERIAL NOT NULL,"\
    "NAME VARCHAR(256) NOT NULL,"\
    "PRIMARY KEY (STATE_ID)"\
  ");";
  work W(*C);
  W.exec(sql);
  W.commit();

}
void createColor(connection* C){
  string sql = "CREATE TABLE COLOR("\
    "COLOR_ID SERIAL NOT NULL,"\
    "NAME VARCHAR(256) NOT NULL,"\
    "PRIMARY KEY (COLOR_ID)"\
  ");";
  work W(*C);
  W.exec(sql);
  W.commit();
}
void createTables(connection* C){
  allDrop(C);
  createState(C);
  createColor(C);
  createTeam(C);
  createPlayer(C);
}


void initialState(connection* C){
  string state_id, name, line;
  ifstream ifs;
  ifs.open("state.txt",ifstream::in);
  while(getline(ifs,line)){
    stringstream ss;
    ss<<line;
    ss>>state_id>>name;
    add_state(C,name);
  }
  ifs.close();
}

void initialColor(connection* C){
  string color_id, name, line;
  ifstream ifs;
  ifs.open("color.txt",ifstream::in);
  while(getline(ifs,line)){
    stringstream ss;
    ss<<line;
    ss>>color_id>>name;
    add_color(C,name);
  }
  ifs.close();
}

void initialTeam(connection* C){
  string team_id, name, line;
  int state_id, color_id, wins, losses;
  ifstream ifs;
  ifs.open("team.txt",ifstream::in);
  while(getline(ifs,line)){
    stringstream ss;
    ss<<line;
    ss>>team_id>>name>>state_id>>color_id>>wins>>losses;
    add_team(C,name,state_id,color_id,wins,losses);
  }
  ifs.close();
}

void initialPlayer(connection* C){
  string player_id, name, first_name, last_name, line;
  int team_id, uniform_num, mpg, ppg, rpg, apg;
  double spg, bpg;
  ifstream ifs;
  ifs.open("player.txt",ifstream::in);
  while(getline(ifs,line)){
    stringstream ss;
    ss<<line;
    ss>>player_id>>team_id>>uniform_num>>first_name>>last_name>>mpg>>ppg>>rpg>>apg>>spg>>bpg;
    add_player(C,team_id,uniform_num,first_name,last_name,mpg,ppg,rpg,apg,spg,bpg);
  }
  ifs.close();
}

void initialContent(connection* C){
  initialState(C);
  initialColor(C);
  initialTeam(C);
  initialPlayer(C);
}

int main (int argc, char *argv[]) 
{

  //Allocate & initialize a Postgres connection object
  connection* C = connect();
  assert(C != NULL);

  //TODO: create PLAYER, TEAM, STATE, and COLOR tables in the ACC_BBALL database
  //      load each table with rows from the provided source txt files
  createTables(C);
  initialContent(C);
  exercise(C);

  //Close database connection
  C->disconnect();
  delete(C);

  return 0;
}


