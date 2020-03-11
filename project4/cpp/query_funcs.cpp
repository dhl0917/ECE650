#include "query_funcs.h"


void add_player(connection *C, int team_id, int jersey_num, string first_name, string last_name,
                int mpg, int ppg, int rpg, int apg, double spg, double bpg)
{
    work W(*C);
    stringstream ss;
    ss<<"INSERT INTO PLAYER (team_id,uniform_num,first_name,last_name,mpg,ppg,rpg,apg,spg,bpg) VALUES ("<<team_id<<","<<jersey_num<<","
        <<W.quote(first_name)<<","<<W.quote(last_name)<<","<<mpg<<","<<ppg<<","<<rpg<<","<<apg<<","<<spg<<","<<bpg<<");";
    string sql = ss.str();
    W.exec(sql);
    W.commit();
}


void add_team(connection *C, string name, int state_id, int color_id, int wins, int losses)
{
    work W(*C);
    stringstream ss;
    ss<<"INSERT INTO TEAM (name,state_id,color_id,wins,losses) VALUES ("<<W.quote(name)<<","<<state_id<<","<<color_id<<","<<wins<<","<<losses<<");";
    string sql = ss.str();
    W.exec(sql);
    W.commit();
}


void add_state(connection *C, string name)
{
    work W(*C);
    string sql = "INSERT INTO STATE (name) VALUES (" + W.quote(name) + ");";
    W.exec(sql);
    W.commit();
}


void add_color(connection *C, string name)
{
    work W(*C);
    string sql = "INSERT INTO COLOR (name) VALUES (" + W.quote(name) + ");";
    W.exec(sql);
    W.commit();
}


void query1(connection *C,
	        int use_mpg, int min_mpg, int max_mpg,
            int use_ppg, int min_ppg, int max_ppg,
            int use_rpg, int min_rpg, int max_rpg,
            int use_apg, int min_apg, int max_apg,
            int use_spg, double min_spg, double max_spg,
            int use_bpg, double min_bpg, double max_bpg
            )
{
    stringstream ss;
    
    if(use_mpg){
        if(!ss.str().empty()){
            ss<< " AND mpg >= "<<min_mpg<<" AND mpg <= "<<max_mpg;
        }
        else{
            ss << "SELECT * FROM PLAYER WHERE mpg >= "<<min_mpg<<" AND mpg <= "<<max_mpg;
        }
    }

    if(use_ppg){
        if(!ss.str().empty()){
            ss<< " AND ppg >= "<<min_ppg<<" AND ppg <= "<<max_ppg;
        }
        else{
            ss << "SELECT * FROM PLAYER WHERE ppg >= "<<min_ppg<<" AND ppg <= "<<max_ppg;
        }
    }

    if(use_rpg){
        if(!ss.str().empty()){
            ss<< " AND rpg >= "<<min_rpg<<" AND rpg <= "<<max_rpg;
        }
        else{
            ss << "SELECT * FROM PLAYER WHERE rpg >= "<<min_rpg<<" AND rpg <= "<<max_rpg;
        }
    }

    if(use_apg){
        if(!ss.str().empty()){
            ss<< " AND apg >= "<<min_apg<<" AND apg <= "<<max_apg;
        }
        else{
            ss << "SELECT * FROM PLAYER WHERE apg >= "<<min_apg<<" AND apg <= "<<max_apg;
        }
    }

    if(use_spg){
        if(!ss.str().empty()){
            ss<< " AND spg >= "<<min_spg<<" AND spg <= "<<max_spg;
        }
        else{
            ss << "SELECT * FROM PLAYER WHERE spg >= "<<min_spg<<" AND spg <= "<<max_spg;
        }
    }

    if(use_bpg){
        if(!ss.str().empty()){
            ss<< " AND bpg >= "<<min_bpg<<" AND bpg <= "<<max_bpg;
        }
        else{
            ss << "SELECT * FROM PLAYER WHERE bpg >= "<<min_bpg<<" AND bpg <= "<<max_bpg;
        }
    }
    if(ss.str().empty()){
        ss<< "SELECT * FROM PLAYER";
    }

    ss<<";";
    
    string sql = ss.str();
    nontransaction N(*C);
    result R(N.exec(sql));
    cout<<"PLAYER_ID TEAM_ID UNIFORM_NUM FIRST_NAME LAST_NAME MPG PPG RPG APG SPG BPG"<<endl;
    for (result::const_iterator c = R.begin(); c != R.end(); ++c) {
        cout << c[0].as<int>() << " " << c[1].as<int>() << " " << c[2].as<int>() << " " << c[3].as<string>() << " "  << c[4].as<string>() << " " 
        << c[5].as<int>() << " " << c[6].as<int>() << " " << c[7].as<int>() << " " << c[8].as<int>() << " " << fixed << setprecision(1) << c[9].as<double>() 
        << " " << c[10].as<double>() << " " << endl;
    }
}


void query2(connection *C, string team_color)
{
    work W(*C);
    string sql = "SELECT TEAM.NAME FROM TEAM, COLOR WHERE COLOR.COLOR_ID = TEAM.COLOR_ID AND COLOR.NAME = " + W.quote(team_color) + ";";
    W.commit();
    nontransaction N(*C);
    result R(N.exec(sql));
    cout<<"NAME"<<endl;
    for (result::const_iterator c = R.begin(); c != R.end(); ++c) {
         cout << c[0].as<string>() << endl;
      }
}


void query3(connection *C, string team_name)
{
    work W(*C);
    string sql = "SELECT FIRST_NAME, LAST_NAME FROM PLAYER, TEAM WHERE PLAYER.TEAM_ID = TEAM.TEAM_ID AND TEAM.NAME = " + W.quote(team_name) + " ORDER BY PPG DESC" + ";";
    W.commit();
    nontransaction N(*C);
    result R(N.exec(sql));
    cout<<"FIRST_NAME LAST_NAME"<<endl;
    for (result::const_iterator c = R.begin(); c != R.end(); ++c) {
         cout << c[0].as<string>() << " " << c[1].as<string>() << endl;
      }
}


void query4(connection *C, string team_state, string team_color)
{
    work W(*C);
    string sql = "SELECT FIRST_NAME, LAST_NAME, UNIFORM_NUM FROM PLAYER, TEAM, STATE, COLOR WHERE TEAM.STATE_ID = STATE.STATE_ID AND TEAM.COLOR_ID = COLOR.COLOR_ID AND PLAYER.TEAM_ID = TEAM.TEAM_ID AND COLOR.NAME = " + W.quote(team_color) + " AND STATE.NAME = " + W.quote(team_state) + ";";
    W.commit();
    nontransaction N(*C);
    result R(N.exec(sql));
    cout<<"FIRST_NAME LAST_NAME UNIFORM_NUM"<<endl;
    for (result::const_iterator c = R.begin(); c != R.end(); ++c) {
         cout << c[0].as<string>() << " " << c[1].as<string>() << " " << c[2].as<int>() << endl;
      }
}


void query5(connection *C, int num_wins)
{
    stringstream ss;
    ss << "SELECT FIRST_NAME, LAST_NAME, TEAM.NAME, WINS FROM PLAYER, TEAM WHERE PLAYER.TEAM_ID = TEAM.TEAM_ID AND TEAM.WINS > " 
        << num_wins << ";";
    string sql = ss.str();
    nontransaction N(*C);
    result R(N.exec(sql));
    cout<<"FIRST_NAME LAST_NAME NAME WINS"<<endl;
    for (result::const_iterator c = R.begin(); c != R.end(); ++c) {
         cout << c[0].as<string>() << " " << c[1].as<string>() << " " << c[2].as<string>() << " " << c[3].as<int>() << " " << endl;
      }
}
