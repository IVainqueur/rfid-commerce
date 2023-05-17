from dotenv import load_dotenv
from os import getenv as ENV
import sqlite3

load_dotenv()

def get_connection():
    con = sqlite3.connect(DB_NAME)
    return con

def translate_card_input(input: str) -> str:
    pass

def get_card_info() -> dict:
    return {}

def transaction(cardInfo: dict) -> bool:
    return True


if __name__ == "__main__":
    print('Program beginning')
    # Ask for total amount to remove from card
    total = int(input('[input] What is the product total? '))
    # Ask for card to be put on sensor
    print('[input] Please put card on sensor...')
    cardInfo = get_card_info()
    # carry out transaction
    transaction_state = transaction(cardInfo)
    
    if transaction_state:
        print('[output] Transaction successful')
        # write output to serial
    else:
        print('[output] Transaction unsuccessful')
        # write output to serial
    
    