from __future__ import print_function
from sqlalchemy.ext.declarative import declarative_base
from sqlalchemy import ForeignKey, Column, Integer, String, Float
from sqlalchemy.orm import relationship

# Schema
Base = declarative_base()
class State(Base):
    __tablename__ = "STATE"

    state_id = Column(Integer, primary_key=True)
    name = Column(String)

    TEAM = relationship("Team", uselist=False, back_populates="STATE")

class Color(Base):
    __tablename__ = "COLOR"

    color_id = Column(Integer,primary_key=True)
    name = Column(String)

    TEAM = relationship("Team", uselist=False, back_populates="COLOR")

class Team(Base):
    __tablename__ = "TEAM"

    team_id = Column(Integer,primary_key=True)
    name = Column(String)
    state_id = Column(Integer,ForeignKey('STATE.state_id'))
    color_id = Column(Integer,ForeignKey('COLOR.color_id'))
    wins = Column(Integer)
    losses = Column(Integer)
    
    STATE = relationship("State", back_populates="TEAM")
    COLOR = relationship("Color", back_populates="TEAM")
    PLAYER = relationship("Player", uselist=False, back_populates="TEAM")

class Player(Base):
    __tablename__ = "PLAYER"

    player_id = Column(Integer,primary_key=True)
    team_id = Column(Integer,ForeignKey('TEAM.team_id'))
    uniform_num = Column(Integer)
    first_name = Column(String)
    last_name = Column(String)
    mpg = Column(Integer)
    ppg = Column(Integer)
    rpg = Column(Integer)
    apg = Column(Integer)
    spg = Column(Float)
    bpg = Column(Float)

    TEAM = relationship("Team", back_populates="PLAYER")

def dropRelations(engine):
    if(engine.dialect.has_table(engine,"PLAYER")):
        Player.__table__.drop(engine)

    if(engine.dialect.has_table(engine,"TEAM")):
        Team.__table__.drop(engine)

    if(engine.dialect.has_table(engine,"STATE")):
        State.__table__.drop(engine)

    if(engine.dialect.has_table(engine,"COLOR")):
        Color.__table__.drop(engine)


def createSchema(engine):
    Base.metadata.create_all(engine)


def add_state(session,name_):
    session.add(State(name=name_))
    session.commit()

def add_color(session,name_):
    session.add(Color(name=name_))
    session.commit()

def add_team(session,name_,state_id_,color_id_,wins_,losses_):
    session.add(Team(name=name_,state_id=state_id_,color_id=color_id_,wins=wins_,losses=losses_))
    session.commit()

def add_player(session,team_id_,uniform_num_,first_name_,last_name_,mpg_,ppg_,rpg_,apg_,spg_,bpg_):
    session.add(Player(team_id=team_id_,uniform_num=uniform_num_,first_name=first_name_,last_name=last_name_,mpg=mpg_,ppg=ppg_,rpg=rpg_,apg=apg_,spg=spg_,bpg=bpg_))
    session.commit()

def init_colors(session):
    f = open("./color.txt")
    line = f.readline()
    while line:
        color_id,name = line.split()
        add_color(session,name)
        line = f.readline()
    f.close()

def init_states(session):
    f = open("./state.txt")
    line = f.readline()
    while line:
        state_id,name = line.split()
        add_state(session,name)
        line = f.readline()
    f.close()

def init_teams(session):
    f = open("./team.txt")
    line = f.readline()
    while line:
        team_id,name,state_id,color_id,wins,losses = line.split()
        add_team(session,name,state_id,color_id,wins,losses)
        line = f.readline()
    f.close()

def init_players(session):
    f = open("./player.txt")
    line = f.readline()
    while line:
        player_id,team_id,uniform_num,first_name,last_name,mpg,ppg,rpg,apg,spg,bpg = line.split()
        add_player(session,team_id,uniform_num,first_name,last_name,mpg,ppg,rpg,apg,spg,bpg)
        line = f.readline()
    f.close()

def init_relations(session):
    init_colors(session)
    init_states(session)
    init_teams(session)
    init_players(session)

def query1(session,use_mpg,min_mpg,max_mpg,use_ppg,min_ppg,max_ppg,use_rpg,min_rpg,max_rpg,use_apg,min_apg,max_apg,use_spg,min_spg,max_spg,use_bpg,min_bpg,max_bpg):
    rows = session.query(Player)
    if(use_mpg):
        rows = session.query(Player).filter(Player.mpg>=min_mpg,Player.mpg<=max_mpg).intersect(rows)
    if(use_ppg):
        rows = session.query(Player).filter(Player.ppg>=min_ppg,Player.ppg<=max_ppg).intersect(rows)
    if(use_rpg):
        rows = session.query(Player).filter(Player.rpg>=min_rpg,Player.rpg<=max_rpg).intersect(rows)
    if(use_apg):
        rows = session.query(Player).filter(Player.apg>=min_apg,Player.apg<=max_apg).intersect(rows)
    if(use_spg):
        rows = session.query(Player).filter(Player.spg>=min_spg,Player.spg<=max_spg).intersect(rows)
    if(use_bpg):
        rows = session.query(Player).filter(Player.bpg>=min_bpg,Player.bpg<=max_bpg).intersect(rows)
    rows = rows.order_by(Player.player_id).all()

    print("PLAYER_ID TEAM_ID UNIFORM_NUM FIRST_NAME LAST_NAME MPG PPG RPG APG SPG BPG")
    for row in rows:
        print("%d %d %d %s %s %d %d %d %d %.1f %.1f"%(row.player_id,row.team_id,row.uniform_num,row.first_name,row.last_name,row.mpg,row.ppg,row.rpg,row.apg,row.spg,row.bpg))
        

def query2(session,team_color_):
    rows = session.query(Team,Color).filter(Color.color_id==Team.color_id,Color.name==team_color_).all()
    print("NAME")
    for row in rows:
        print("%s"%(row[0].name))

def query3(session,team_name_):
    rows = session.query(Player,Team).filter(Player.team_id==Team.team_id, Team.name==team_name_).order_by(Player.ppg.desc()).all()
    print("FIRST_NAME LAST_NAME")
    for row in rows:
        print("%s %s"%(row[0].first_name,row[0].last_name))
    
def query4(session,team_state_,team_color_):
    rows = session.query(Player,Team,State,Color).filter(Team.state_id==State.state_id,Team.color_id==Color.color_id,Player.team_id==Team.team_id,Color.name==team_color_,State.name==team_state_).all()
    print("FIRST_NAME LAST_NAME UNIFORM_NUM")
    for row in rows:
        print("%s %s %d"%(row[0].first_name,row[0].last_name,row[0].uniform_num))

def query5(session,num_wins_):
    rows = session.query(Player,Team).filter(Player.team_id==Team.team_id,Team.wins>num_wins_).all()
    print("FIRST_NAME LAST_NAME NAME WINS")
    for row in rows:
        print("%s %s %s %d"%(row[0].first_name,row[0].last_name,row[1].name,row[1].wins))



