from dotenv import load_dotenv
from os import getenv as ENV
import sqlite3
import serial
import re

load_dotenv()
ser = serial.Serial(
    port="/dev/ttyACM0",  # port-and-system dependent
    baudrate=9600,
    parity=serial.PARITY_NONE,
    stopbits=serial.STOPBITS_ONE,
    bytesize=serial.EIGHTBITS,
    timeout=1,
)
DB_NAME = ENV('DB_NAME')

# Constants
SUCCESS_CODE = 0
ERROR_CODE = 1

ser.flush()

def get_connection():
    con = sqlite3.connect(DB_NAME)
    return con

def create_card_table():
    con = get_connection()
    query = """CREATE TABLE cards(
        UUID VARCHAR(12) PRIMARY KEY,
        money INT NOT NULL,
        points INT NOT NULL
    )"""
    con.execute(query)
    con.close()
    
def check_if_table_exists():
    conn = get_connection()
    c = conn.cursor()
    #get the count of tables with the name
    c.execute(''' SELECT count(name) FROM sqlite_master WHERE type='table' AND name='cards' ''')

    #if the count is 1, then table exists
    if c.fetchone()[0]==1 :
        print('Table exists.')
    else :
        create_card_table()

def add_card_to_db(uniqueID: str):
    con = get_connection()
    query = """INSERT INTO cards(UUID, money, points) VALUES(?,?,?)"""
    con.execute(query, (uniqueID, 1000, 0))
    con.commit()
    con.close()

def translate_card_input(input: str) -> str:
    pass

def get_card_info() -> dict:
    while True:
        if ser.in_waiting > 0:
            line = ser.readline().decode("utf-8").rstrip()
            print(line)
            if line[0:11] == "[card_data]":
                """ 
                    The format of card_data is as follows
                    uniqueID;money;points;
                """
                card_data = re.split("(.+?);(.+?);(.+?);", line[11:].strip())[1:-1]
                # check if the card_data is the correct format
                if len(card_data) != 3:
                    print('[error] invalid card_data')
                    return {'success': False}
                return {
                    'success': True,
                    'uniqueID': card_data[0],
                    'money': int(card_data[1]),
                    'points': int(card_data[2])
                }
            
def transaction(cardInfo: dict, price: int) -> dict:
    select_stmt = '''SELECT * FROM cards WHERE UUID=?'''
    con = get_connection()
    cur = con.cursor()
    cur.execute(select_stmt, (cardInfo['uniqueID'],))
    rows = cur.fetchall()
    if len(rows) == 0:
        print(f"[warn] unknown card {cardInfo['uniqueID']}")
        print("[info] adding new card to db")
        add_card_to_db(cardInfo['uniqueID'])
        print("[info::success] added new card. retrying transaction...")
        return transaction(cardInfo, price)
    
    card = {
        'UUID': rows[0][0],
        'money': rows[0][1],
        'points': rows[0][2]
    }
    # if card.money != cardInfo.money:
    #     print("[error] mismatching info on card")
    if card['money'] < price:
        print("[error] insufficient balance on card")
        return {'success': False}
    
    transaction_query = """UPDATE cards SET money=? WHERE UUID=?"""
    con.execute(transaction_query, (card['money']-price,card['UUID']))
    con.commit()
    con.close()
        
    return {
        'success': True, 
        'newInfo': {
            'money': card['money']-price, 
            'points': card['points']
        }
        }


if __name__ == "__main__":
    check_if_table_exists()
    print('WELCOME TO RFID-COMMERCE')
    print('========================')
    while True:
        # Ask for total amount to remove from card
        total = int(input('[input] What is the product total? '))
        # Ask for card to be put on sensor
        print('[input] Please put card on sensor...')
        cardInfo = get_card_info()
        print(cardInfo)
        if not cardInfo['success']:
            ser.write(f"{ERROR_CODE}".encode())
            break
        # carry out transaction
        transaction_state = transaction(cardInfo, total)
        
        if transaction_state['success']:
            print('[info] Transaction successful')
            print(f"{transaction_state['newInfo']['money']};{transaction_state['newInfo']['points']};")
            ser.write(f"{transaction_state['newInfo']['money']};{transaction_state['newInfo']['points']};".encode())
        else:
            print('[info] Transaction unsuccessful')
            ser.write(f"{ERROR_CODE}".encode())
        print("=========================\n\n")
    
    