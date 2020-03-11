#include "exerciser.h"

void exercise(connection *C)
{
    // one field enabled
    query1(C,1,35,40,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0);
    query1(C,0,35,40,1,18,25,0,0,0,0,0,0,0,0,0,0,0,0);
    query1(C,0,35,40,0,18,25,1,3,8,0,0,0,0,0,0,0,0,0);
    query1(C,0,35,40,0,18,25,0,3,8,1,2,3,0,0,0,0,0,0);
    query1(C,0,35,40,0,18,25,0,3,8,0,2,3,1,1.3,2.5,0,0,0);
    query1(C,0,35,40,0,18,25,0,3,8,1,2,3,0,0,0,1,0.7,1.5);
    // no fields enabled
    query1(C,0,35,40,0,18,25,0,3,8,0,2,3,0,0,0,0,0.7,1.5);
    // two fields enabled
    query1(C,1,35,40,0,18,25,1,3,8,0,0,0,0,0,0,0,0,0);
    // three fields enabled
    query1(C,1,35,40,1,18,25,1,3,8,0,0,0,0,0,0,0,0,0);
    // all fields enabled
    query1(C,1,35,40,1,18,25,1,3,8,1,2,3,1,1.3,2.5,1,0.7,1.5);

    // valid colors
    query2(C,"DarkBlue");
    query2(C,"LightBlue");
    // invalid color
    query2(C,"White");

    // valid teams
    query3(C,"Duke");
    query3(C,"UNC");
    // invalid teams
    query3(C,"XJTU");

    // valid
    query4(C,"NC","DarkBlue");
    // invalid
    query4(C,"MA","White");

    query5(C,10);
    query5(C,100);

    add_color(C,"White");
    add_state(C,"SX");
    add_team(C,"XJTU",11,9,15,3);
    add_player(C,16,14,"Hongliang","Dong",35,20,10,10,3.5,2.5);

    query2(C,"White");
    query3(C,"XJTU");
    query1(C,1,33,38,1,18,25,1,8,12,1,8,12,1,2.5,4.5,1,1.5,2.5);
    query4(C,"SX","White");
    query5(C,14);


}
