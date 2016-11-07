#include <script/script.hpp>

#include <script/dialogue.hpp>

#include <chaiscript/chaiscript.hpp>

ScriptSystem::ScriptSystem(Manager& manager)
	: System(manager), chai(Script::getChaiScript()),
	  prompt{}, promptUp{}
{
	chai.add_global(chaiscript::var(this), "script");

	// Dialog box
	chai.add(chaiscript::fun(&ScriptSystem::dialogBox), "dialogBox");
	chai(R"(
	def dialog(string text, continuation)
	{
		script.dialogBox(text, continuation);
	}

	def dialog(string text)
	{
		dialog(text, fun(){});
	}
	)");

	// Menu box
	chai.add(chaiscript::fun(&ScriptSystem::menuBox), "menuBox");
	chai.add(chaiscript::vector_conversion<std::vector<std::string>>());
	chai.add(chaiscript::vector_conversion<std::vector<std::function<void()>>>());
	chai(R"(
	def menu(options, continuations)
	{
		script.menuBox(options, continuations);
	}
	)");

	// Prompt box
	chai.add(chaiscript::fun(&ScriptSystem::promptBox), "promptBox");
	chai(R"(
	def prompt(text, options, continuations)
	{
		script.promptBox(text, options, continuations);
	}
	)");
}

void ScriptSystem::update(sf::Time delta)
{
	manager.forEntitiesHaving<TEvent, TDialogClosed>([this](EntityIndex)
	{
		dialogContinuation();
	});

	manager.forEntitiesHaving<TEvent, CMenuClosed>([this](EntityIndex eI)
	{
		auto choice = manager.getComponent<CMenuClosed>(eI).choice;

		if(prompt)
		{
			manager.forEntitiesHaving<TPrompt>([this](EntityIndex eI)
			{
				manager.deleteEntity(eI);
			});

			prompt = false;
			promptOptions.clear();
			promptContinuations.clear();
		}
		
		menuContinuations[choice]();
	});

	if(prompt && !promptUp)
		manager.forEntitiesHaving<TPrompt>([this](EntityIndex eI)
		{
			auto& drawable = manager.getComponent<CDrawable>(eI);	

			auto dialog = static_cast<DialogueBox*>(drawable.drawable);
			if(dialog->allDisplayed())
			{
				menuBox(promptOptions, promptContinuations);
				promptUp = true;
			}
		});
}

void ScriptSystem::dialogBox(std::string text, std::function<void()> continuation)
{
	Dialogue::createDialogue(manager, text);

	dialogContinuation = continuation;
}

void ScriptSystem::menuBox(
    const std::vector<std::string>& options,
    const std::vector<std::function<void()>>& continuations)
{
	Dialogue::createMenu(manager, options);

	menuContinuations = continuations;
}

void ScriptSystem::promptBox(
	const std::string& text,
    const std::vector<std::string>& options,
    const std::vector<std::function<void()>>& continuations)
{
	auto dialogue = Dialogue::createDialogue(manager, text);
	manager.addTag<TPrompt>(dialogue);

	prompt = true;
	promptUp = false;
	promptOptions = options;
	promptContinuations = continuations;
}

void ScriptSystem::runScript(std::string src)
{
	chai(src);
}
