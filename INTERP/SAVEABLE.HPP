//	saveable.hpp

#ifndef SAVEABLE_HPP
#define SAVEABLE_HPP

class SaveGame;

struct Saveable {
	typedef void (Saveable::*Func)(SaveGame*, Bool);

	virtual ~Saveable() {}

	virtual void	Dump(SaveGame* game, Bool) { SaveRestore(game, False); }
	virtual void	SaveRestoreBefore(SaveGame*, Bool) {}
	virtual void	SaveRestore(SaveGame*, Bool) {}
	virtual void	SaveRestoreAfter(SaveGame*, Bool) {}
	
	virtual void	SaveRestoreDo(Func func, SaveGame* saveGame, Bool save)
							{ (this->*func)(saveGame, save); }
};

#endif
