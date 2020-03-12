import sqlalchemy
from sqlalchemy import create_engine
from sqlalchemy.orm import sessionmaker
from utils import *
from query_funcs import *
from exerciser import exercise

engine = create_engine('postgresql+psycopg2://postgres:passw0rd@127.0.0.1/ACC_BBALL',echo=False)

# Drop relations if applicable
dropRelations(engine)
# Create schema
createSchema(engine)
# Initialize relations
Session = sessionmaker(bind=engine)
session = Session()
init_relations(session)

exercise(session)

#Close session
session.close()


