from utils import *

def exercise(session):
    # one field enabled
    query1(session,1,35,40,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0)
    query1(session,0,35,40,1,18,25,0,0,0,0,0,0,0,0,0,0,0,0)
    query1(session,0,35,40,0,18,25,1,3,8,0,0,0,0,0,0,0,0,0)
    query1(session,0,35,40,0,18,25,0,3,8,1,2,3,0,0,0,0,0,0)
    query1(session,0,35,40,0,18,25,0,3,8,0,2,3,1,1.3,2.5,0,0,0)
    query1(session,0,35,40,0,18,25,0,3,8,1,2,3,0,0,0,1,0.7,1.5)
    # no fields enabled
    query1(session,0,35,40,0,18,25,0,3,8,0,2,3,0,0,0,0,0.7,1.5)
    # two fields enabled
    query1(session,1,35,40,0,18,25,1,3,8,0,0,0,0,0,0,0,0,0)
    # three fields enabled
    query1(session,1,35,40,1,18,25,1,3,8,0,0,0,0,0,0,0,0,0)
    # all fields enabled
    query1(session,1,35,40,1,18,25,1,3,8,1,2,3,1,1.3,2.5,1,0.7,1.5)

    # valid colors
    query2(session,"DarkBlue")
    query2(session,"LightBlue")
    # invalid color
    query2(session,"White")

    # valid teams
    query3(session,"Duke")
    query3(session,"UNC")
    # invalid teams
    query3(session,"XJTU")

    # valid
    query4(session,"NC","DarkBlue")
    # invalid
    query4(session,"MA","White")

    query5(session,10)
    query5(session,100)

    add_color(session,"White")
    add_state(session,"SX")
    add_team(session,"XJTU",11,9,15,3)
    add_player(session,16,14,"Hongliang","Dong",35,20,10,10,3.5,2.5)

    query2(session,"White")
    query3(session,"XJTU")
    query1(session,1,33,38,1,18,25,1,8,12,1,8,12,1,2.5,4.5,1,1.5,2.5)
    query4(session,"SX","White")
    query5(session,14)



