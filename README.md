# === RFID-Commerce ===
### Workflow (rough description)
- the user selects a certain good to purchase
- `[in program] `

	- we register the total of the products they're buying
	- the app the prompts for the user's payment card
	- the app reads the user's current balance and decides if the transaction can happen
    
		* `if the balance is **NOT** enough` the transaction will be cancelled and buzzer rings
		* `if the balance is enough` the money will be reducted from the user's account in DB, points increased, and card balance is updated, buzzer plays success sound		
## Contributors
- [ISHIMWE Vainqueur](https://github.com/IVainqueur)
- [Gakuba Edmond](https://github.com/edmondgaks)
- [Manzi Cedrick](https://github.com/Manzi-Cedrick)
- [Ndayishimiye Gilbert](https://github.com/nday-gilbert)
- [UMUTONI Mireille](https://github.com/Umutonimireille)
