# CubeWorldExtended
Ever heard of the game cube world? Probably not. Currently this is a WIP 'mod' that makes some extensions to the game. The game is 5+ years old but still super fun

Well, cube world was finally released!
However, I am coming back to provide descriptions of the project.

The 'mod' uses DLL memory injection to modify the game state while in play. So, therefore it's a hack. This was also my first project implementing design patterns and the general design is as follows:

* A Game Singleton to store and fetch all the Handles and pointers to the CubeWorld client
* A player Singleton to store and fetch all the points related to player data. Name, Health, Mana, Inventory, Level etc.
* A GUI class that employs my GUI library to setup, display, and handle typing into the in game command line. The GUI is overlayed on top of the CubeWorld client
 * Despite its name `GUIMediator` is not <i>really</i> a mediator, but it encapsulates all the UI stuff and communicates the commands to the interpreter directly via Observer
* An interpreter that communicates to the GUI class via the Observer pattern and actually parses and executes each command


Overall, the project is quite simple and most involved reading and writing to the different addresses in the game. The one complicated part was the petfood command.
To do this, the petfood inventory had to be located in memory by hooking the function called when the user clicks a certain slot in their inventory. (First useful thing I wrote in Assembly!)
Then, the inventory can be iterated over looking up each item with the petfood list in `pets.txt` and `ids.txt`
