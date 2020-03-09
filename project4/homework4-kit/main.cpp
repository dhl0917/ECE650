#include <iostream>
#include <pqxx/pqxx>
#include "assert.h"
#include "exerciser.h"
#include <string>

using namespace std;
using namespace pqxx;

connection* connect(){
  connection *C;
  try{
    //Establish a connection to the database
    //Parameters: database name, user name, user password
    C = new connection("dbname=ACC_BBALL user=postgres password=passw0rd");
    if (C->is_open()) {
      cout << "Opened database successfully: " << C->dbname() << endl;
      return C;
    } else {
      cout << "Can't open database" << endl;
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

int main (int argc, char *argv[]) 
{

  //Allocate & initialize a Postgres connection object
  connection* C = connect();
  assert(C != NULL);

  createTables(C);

  //TODO: create PLAYER, TEAM, STATE, and COLOR tables in the ACC_BBALL database
  //      load each table with rows from the provided source txt files


  exercise(C);


  //Close database connection
  C->disconnect();

  return 0;
}


