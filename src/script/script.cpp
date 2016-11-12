#include <script/script.hpp>

#include <script/dialogue.hpp>

#include <chaiscript/chaiscript.hpp>

ScriptSystem::ScriptSystem(Manager& manager, Overworld& overworld)
	: System(manager), overworld(overworld), chai(Script::getChaiScript())
{
	chai.add_global(chaiscript::var(this), "script");
	
	chai.add(chaiscript::fun(&ScriptSystem::openDialog), "dialog");
	chai.add(chaiscript::fun(&ScriptSystem::openMenu), "menu");
	chai.add(chaiscript::fun(&ScriptSystem::openPrompt), "prompt");

	chai(R"(use("init.chai");)");
}

void ScriptSystem::update(sf::Time delta)
{
	// Interaction
	manager.forEntitiesHaving<TEvent, CInteract>([this](EntityIndex eI)
	{
		auto& interact = manager.getComponent<CInteract>(eI);

		runScript(overworld.getInteractScript(interact.location));
	});

	for(auto it = latentFunctions.begin(); it != latentFunctions.end(); )
	{
		if((*it)())
			it = latentFunctions.erase(it);
		else
			++it;
	}
}

void ScriptSystem::openDialog(std::string text, std::function<void()> callback)
{
	auto dialog = Dialogue::createDialogue(manager, text);

	auto waitForDialogClose = [=]()
	{
		return manager.isDeleted(dialog);
	};

	auto callCallback = [=]() 
	{ 
		callback(); 
		return true; 
	};

	doLatentInOrder({waitForDialogClose, callCallback});
}

void ScriptSystem::openMenu(
    const std::vector<std::string>& options,
    const std::vector<std::function<void()>>& callbacks)
{
	auto menu = Dialogue::createMenu(manager, options);

	auto waitForMenuClose = [=]()
	{
		return manager.isDeleted(menu);
	};

	auto callChosenCallback = [=]()
	{
		bool called = false;

		manager.forEntitiesHaving<TEvent, CMenuClosed>(
		[=, &called](EntityIndex eI)
		{
			// TODO Check that the correct menu was closed
			auto choice = manager.getComponent<CMenuClosed>(eI).choice;

			callbacks[choice]();
			called = true;
		});

		return called;
	};

	doLatentInOrder({waitForMenuClose, callChosenCallback});
}

void ScriptSystem::openPrompt(
	const std::string& text,
    const std::vector<std::string>& options,
    const std::vector<std::function<void()>>& callbacks)
{
	auto dialog = Dialogue::createDialogue(manager, text);
	manager.addTag<TPrompt>(dialog);

	auto waitForDialog = [=]()
	{
		auto& drawable = manager.getComponent<CDrawable>(dialog);	
		auto dialogBox = static_cast<DialogueBox*>(drawable.drawable);

		return dialogBox->allDisplayed();
	};

	auto displayMenu = [=]()
	{
		auto menu = Dialogue::createMenu(manager, options);

		auto waitForMenuClosed = [=]()
		{
			return manager.isDeleted(menu);
		};

		auto callChosenCallback = [=]()
		{
			bool called = false;

			manager.forEntitiesHaving<TEvent, CMenuClosed>(
			[=, &called](EntityIndex eI)
			{
				// TODO Check that the correct menu was closed
				auto choice = manager.getComponent<CMenuClosed>(eI).choice;

				callbacks[choice]();
				called = true;
			});

			return called;
		};

		auto closeDialog = [=]()
		{
			manager.deleteEntity(dialog);
			return true;
		};

		doLatentInOrder({waitForMenuClosed, callChosenCallback, closeDialog});

		return true;
	};

	doLatentInOrder({waitForDialog, displayMenu});
}

void ScriptSystem::runScript(std::string script)
{
	chai(script);
}

void ScriptSystem::doLatent(std::function<bool()> func)
{
	latentFunctions.push_back(func);
}

void ScriptSystem::doLatentInOrder(std::vector<std::function<bool()>> funcs)
{
	auto index = 0;

	auto combined = [index, funcs]() mutable
	{
		while(index < funcs.size())
			if(funcs[index]())
				index++;
			else
				break;

		return index >= funcs.size();
	};

	doLatent(combined);
}
