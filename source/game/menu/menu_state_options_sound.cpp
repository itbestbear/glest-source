// This file is part of Glest <https://github.com/Glest>
//
// Copyright (C) 2018  The Glest team
//
// Glest is a fork of MegaGlest <https://megaglest.org/>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>

#include "menu_state_options_sound.h"

#include "renderer.h"
#include "game.h"
#include "program.h"
#include "sound_renderer.h"
#include "core_data.h"
#include "config.h"
#include "menu_state_root.h"
#include "menu_state_options.h"
#include "util.h"
#include "menu_state_keysetup.h"
#include "menu_state_options_graphics.h"
#include "menu_state_options_network.h"
#include "menu_state_options_sound.h"
#include "string_utils.h"
#include "metrics.h"
#include "leak_dumper.h"

using namespace Shared::Util;

namespace Game {
	// =====================================================
	//      class MenuStateOptions
	// =====================================================
	MenuStateOptionsSound::MenuStateOptionsSound(Program * program,
		MainMenu * mainMenu,
		ProgramState ** parentUI) :
		MenuState(program, mainMenu, "config"),
		buttonOk("Options_Sound", "buttonOk"),
		buttonReturn("Options_Sound", "buttonReturn"),
		buttonKeyboardSetup("Options_Sound", "buttonKeyboardSetup"),
		buttonVideoSection("Options_Sound", "buttonVideoSection"),
		buttonAudioSection("Options_Sound", "buttonAudioSection"),
		buttonMiscSection("Options_Sound", "buttonMiscSection"),
		buttonNetworkSettings("Options_Sound", "buttonNetworkSettings"),
		labelSoundFactory("Options_Sound", "labelSoundFactory"),
		listBoxSoundFactory("Options_Sound", "listBoxSoundFactory"),
		labelVolumeFx("Options_Sound", "labelVolumeFx"),
		listBoxVolumeFx("Options_Sound", "listBoxVolumeFx"),
		labelVolumeAmbient("Options_Sound", "labelVolumeAmbient"),
		listBoxVolumeAmbient("Options_Sound", "listBoxVolumeAmbient"),
		labelVolumeMusic("Options_Sound", "labelVolumeMusic"),
		listBoxVolumeMusic("Options_Sound", "listBoxVolumeMusic"),
		mainMessageBox("Options_Sound", "mainMessageBox") {
		try {
			containerName = "Options_Sound";
			this->
				parentUI = parentUI;
			Lang &
				lang = Lang::getInstance();
			Config &
				config = Config::getInstance();
			this->
				console.
				setOnlyChatMessagesInStoredLines(false);

			int
				leftLabelStart = 100;
			int
				leftColumnStart = leftLabelStart + 300;
			int
				buttonRowPos = 50;
			int
				buttonStartPos = 170;
			int
				lineOffset = 30;
			int
				tabButtonWidth = 200;
			int
				tabButtonHeight = 30;

			mainMessageBox.
				init(lang.getString("Ok"));
			mainMessageBox.
				setEnabled(false);
			mainMessageBoxState = 0;

			buttonAudioSection.
				init(0, 700, tabButtonWidth, tabButtonHeight + 20);
			buttonAudioSection.
				setFont(CoreData::getInstance().getMenuFontVeryBig());
			buttonAudioSection.
				setFont3D(CoreData::getInstance().getMenuFontVeryBig3D());
			buttonAudioSection.
				setText(lang.getString("Audio"));
			// Video Section
			buttonVideoSection.
				init(200, 720, tabButtonWidth, tabButtonHeight);
			buttonVideoSection.
				setFont(CoreData::getInstance().getMenuFontVeryBig());
			buttonVideoSection.
				setFont3D(CoreData::getInstance().getMenuFontVeryBig3D());
			buttonVideoSection.
				setText(lang.getString("Video"));
			//currentLine-=lineOffset;
			//MiscSection
			buttonMiscSection.
				init(400, 720, tabButtonWidth, tabButtonHeight);
			buttonMiscSection.
				setFont(CoreData::getInstance().getMenuFontVeryBig());
			buttonMiscSection.
				setFont3D(CoreData::getInstance().getMenuFontVeryBig3D());
			buttonMiscSection.
				setText(lang.getString("Misc"));
			//NetworkSettings
			buttonNetworkSettings.
				init(600, 720, tabButtonWidth, tabButtonHeight);
			buttonNetworkSettings.
				setFont(CoreData::getInstance().getMenuFontVeryBig());
			buttonNetworkSettings.
				setFont3D(CoreData::getInstance().getMenuFontVeryBig3D());
			buttonNetworkSettings.
				setText(lang.getString("Network"));

			//KeyboardSetup
			buttonKeyboardSetup.
				init(800, 720, tabButtonWidth, tabButtonHeight);
			buttonKeyboardSetup.
				setFont(CoreData::getInstance().getMenuFontVeryBig());
			buttonKeyboardSetup.
				setFont3D(CoreData::getInstance().getMenuFontVeryBig3D());
			buttonKeyboardSetup.
				setText(lang.getString("Keyboardsetup"));

			int
				currentLine = 650;    // reset line pos
			int
				currentLabelStart = leftLabelStart;   // set to right side
			int
				currentColumnStart = leftColumnStart; // set to right side

				//soundboxes
			labelSoundFactory.
				init(currentLabelStart, currentLine);
			labelSoundFactory.
				setText(lang.getString("SoundAndMusic2"));

			listBoxSoundFactory.
				init(currentColumnStart, currentLine, 175);
			listBoxSoundFactory.
				pushBackItem(lang.getString("None"));
			listBoxSoundFactory.
				pushBackItem("OpenAL");

			string
				FSoundMode = lang.getString(config.getString("FactorySound", "OpenAL"));
			listBoxSoundFactory.
				setSelectedItem(FSoundMode);
			currentLine -=
				lineOffset;

			labelVolumeFx.
				init(currentLabelStart, currentLine);
			labelVolumeFx.
				setText(lang.getString("FxVolume"));

			listBoxVolumeFx.
				init(currentColumnStart, currentLine, 80);
			currentLine -=
				lineOffset;

			labelVolumeAmbient.
				init(currentLabelStart, currentLine);

			listBoxVolumeAmbient.
				init(currentColumnStart, currentLine, 80);
			labelVolumeAmbient.
				setText(lang.getString("AmbientVolume"));
			currentLine -=
				lineOffset;

			labelVolumeMusic.
				init(currentLabelStart, currentLine);

			listBoxVolumeMusic.
				init(currentColumnStart, currentLine, 80);
			labelVolumeMusic.
				setText(lang.getString("MusicVolume"));
			//currentLine-=lineOffset;

			for (int i = 0; i <= 100; i++) {
				listBoxVolumeFx.pushBackItem(intToStr(i));
				listBoxVolumeAmbient.
					pushBackItem(intToStr(i));
				listBoxVolumeMusic.
					pushBackItem(intToStr(i));
			}
			listBoxVolumeFx.
				setSelectedItem(intToStr(config.getInt("SoundVolumeFx", "70")));
			listBoxVolumeAmbient.
				setSelectedItem(intToStr
				(config.getInt("SoundVolumeAmbient", "70")));
			listBoxVolumeMusic.
				setSelectedItem(intToStr
				(config.getInt("SoundVolumeMusic", "70")));

			//currentLine-=lineOffset/2;



			//////////////////////////////////////////////////////////////////
			///////// RIGHT SIDE
			//////////////////////////////////////////////////////////////////

			//currentLine=700; // reset line pos
			//currentLabelStart=rightLabelStart; // set to right side
			//currentColumnStart=rightColumnStart; // set to right side


			// buttons
			buttonOk.init(buttonStartPos, buttonRowPos, 100);
			buttonOk.setText(lang.getString("Save"));
			buttonReturn.setText(lang.getString("Return"));

			buttonReturn.init(buttonStartPos + 110, buttonRowPos, 100);

			GraphicComponent::applyAllCustomProperties(containerName);
		} catch (exception & e) {
			SystemFlags::OutputDebug(SystemFlags::debugError,
				"In [%s::%s Line: %d] Error loading options: %s\n",
				__FILE__, __FUNCTION__, __LINE__,
				e.what());
			throw
				game_runtime_error(string("Error loading options msg: ") +
					e.what());
		}
	}

	void
		MenuStateOptionsSound::reloadUI() {
		Lang & lang = Lang::getInstance();

		mainMessageBox.init(lang.getString("Ok"));

		buttonAudioSection.setText(lang.getString("Audio"));
		buttonVideoSection.setText(lang.getString("Video"));
		buttonMiscSection.setText(lang.getString("Misc"));
		buttonNetworkSettings.setText(lang.getString("Network"));
		labelSoundFactory.setText(lang.getString("SoundAndMusic2"));

		std::vector < string > listboxData;
		listboxData.push_back(lang.getString("None"));
		listboxData.push_back("OpenAL");

		listBoxSoundFactory.setItems(listboxData);

		labelVolumeFx.setText(lang.getString("FxVolume"));
		labelVolumeAmbient.setText(lang.getString("AmbientVolume"));
		labelVolumeMusic.setText(lang.getString("MusicVolume"));

		listboxData.clear();

		buttonOk.setText(lang.getString("Save"));
		buttonReturn.setText(lang.getString("Return"));
	}

	void
		MenuStateOptionsSound::mouseClick(int x, int y, MouseButton mouseButton) {

		//Config &config= Config::getInstance();
		CoreData & coreData = CoreData::getInstance();
		SoundRenderer & soundRenderer = SoundRenderer::getInstance();

		if (mainMessageBox.getEnabled()) {
			int
				button = 0;
			if (mainMessageBox.mouseClick(x, y, button)) {
				soundRenderer.playFx(coreData.getClickSoundA());
				if (button == 0) {
					if (mainMessageBoxState == 1) {
						mainMessageBoxState = 0;
						mainMessageBox.setEnabled(false);
						saveConfig();

						Lang & lang = Lang::getInstance();
						mainMessageBox.init(lang.getString("Ok"));
						mainMenu->setState(new MenuStateOptions(program, mainMenu));
					} else {
						mainMessageBox.setEnabled(false);

						Lang & lang = Lang::getInstance();
						mainMessageBox.init(lang.getString("Ok"));
					}
				} else {
					if (mainMessageBoxState == 1) {
						mainMessageBoxState = 0;
						mainMessageBox.setEnabled(false);

						Lang & lang = Lang::getInstance();
						mainMessageBox.init(lang.getString("Ok"));


						this->mainMenu->init();
					}
				}
			}
		} else if (buttonOk.mouseClick(x, y)) {
			soundRenderer.playFx(coreData.getClickSoundA());
			saveConfig();
			//mainMenu->setState(new MenuStateOptions(program, mainMenu));
			return;
		} else if (buttonReturn.mouseClick(x, y)) {
			soundRenderer.playFx(coreData.getClickSoundA());
			if (this->parentUI != NULL) {
				*this->parentUI = NULL;
				delete *
					this->
					parentUI;
			}
			mainMenu->setState(new MenuStateRoot(program, mainMenu));
			return;
		} else if (buttonKeyboardSetup.mouseClick(x, y)) {
			soundRenderer.playFx(coreData.getClickSoundA());
			//mainMenu->setState(new MenuStateKeysetup(program, mainMenu)); // open keyboard shortcuts setup screen
			//mainMenu->setState(new MenuStateOptionsGraphics(program, mainMenu)); // open keyboard shortcuts setup screen
			//mainMenu->setState(new MenuStateOptionsNetwork(program, mainMenu)); // open keyboard shortcuts setup screen
			mainMenu->setState(new MenuStateKeysetup(program, mainMenu)); // open keyboard shortcuts setup screen
			//showMessageBox("Not implemented yet", "Keyboard setup", false);
			return;
		} else if (buttonAudioSection.mouseClick(x, y)) {
			soundRenderer.playFx(coreData.getClickSoundA());
			//mainMenu->setState(new MenuStateOptionsSound(program, mainMenu)); // open keyboard shortcuts setup screen
			return;
		} else if (buttonNetworkSettings.mouseClick(x, y)) {
			soundRenderer.playFx(coreData.getClickSoundA());
			mainMenu->setState(new MenuStateOptionsNetwork(program, mainMenu));   // open keyboard shortcuts setup screen
			return;
		} else if (buttonMiscSection.mouseClick(x, y)) {
			soundRenderer.playFx(coreData.getClickSoundA());
			mainMenu->setState(new MenuStateOptions(program, mainMenu));  // open keyboard shortcuts setup screen
			return;
		} else if (buttonVideoSection.mouseClick(x, y)) {
			soundRenderer.playFx(coreData.getClickSoundA());
			mainMenu->setState(new MenuStateOptionsGraphics(program, mainMenu));  // open keyboard shortcuts setup screen
			return;
		} else {
			listBoxSoundFactory.mouseClick(x, y);
			listBoxVolumeFx.mouseClick(x, y);
			listBoxVolumeAmbient.mouseClick(x, y);
			listBoxVolumeMusic.mouseClick(x, y);
		}
	}

	void
		MenuStateOptionsSound::mouseMove(int x, int y, const MouseState * ms) {
		if (mainMessageBox.getEnabled()) {
			mainMessageBox.mouseMove(x, y);
		}
		buttonOk.mouseMove(x, y);
		buttonReturn.mouseMove(x, y);
		buttonKeyboardSetup.mouseMove(x, y);
		buttonAudioSection.mouseMove(x, y);
		buttonNetworkSettings.mouseMove(x, y);
		buttonMiscSection.mouseMove(x, y);
		buttonVideoSection.mouseMove(x, y);

		listBoxSoundFactory.mouseMove(x, y);
		listBoxVolumeFx.mouseMove(x, y);
		listBoxVolumeAmbient.mouseMove(x, y);
		listBoxVolumeMusic.mouseMove(x, y);
	}

	//bool MenuStateOptionsSound::isInSpecialKeyCaptureEvent() {
	//      return (activeInputLabel != NULL);
	//}
	//
	//void MenuStateOptionsSound::keyDown(SDL_KeyboardEvent key) {
	//      if(activeInputLabel != NULL) {
	//              keyDownEditLabel(key, &activeInputLabel);
	//      }
	//}

	void
		MenuStateOptionsSound::keyPress(SDL_KeyboardEvent c) {
		//      if(activeInputLabel != NULL) {
		//          //printf("[%d]\n",c); fflush(stdout);
		//              if( &labelPlayerName    == activeInputLabel ||
		//                      &labelTransifexUser == activeInputLabel ||
		//                      &labelTransifexPwd == activeInputLabel ||
		//                      &labelTransifexI18N == activeInputLabel) {
		//                      textInputEditLabel(c, &activeInputLabel);
		//              }
		//      }
		//      else {
		Config & configKeys =
			Config::getInstance(std::pair < ConfigType,
				ConfigType >(cfgMainKeys, cfgUserKeys));
		if (isKey(configKeys.getSDLKey("SaveGUILayout"), c) == true) {
			GraphicComponent::saveAllCustomProperties(containerName);
			//Lang &lang= Lang::getInstance();
			//console.addLine(lang.getString("GUILayoutSaved") + " [" + (saved ? lang.getString("Yes") : lang.getString("No"))+ "]");
		}
		//      }
	}

	void
		MenuStateOptionsSound::render() {
		Renderer & renderer = Renderer::getInstance();

		if (mainMessageBox.getEnabled()) {
			renderer.renderMessageBox(&mainMessageBox);
		} else {
			renderer.renderButton(&buttonOk);
			renderer.renderButton(&buttonReturn);
			renderer.renderButton(&buttonKeyboardSetup);
			renderer.renderButton(&buttonVideoSection);
			renderer.renderButton(&buttonAudioSection);
			renderer.renderButton(&buttonMiscSection);
			renderer.renderButton(&buttonNetworkSettings);
			renderer.renderListBox(&listBoxSoundFactory);
			renderer.renderLabel(&labelSoundFactory);


			renderer.renderListBox(&listBoxVolumeFx);
			renderer.renderLabel(&labelVolumeFx);
			renderer.renderListBox(&listBoxVolumeAmbient);
			renderer.renderLabel(&labelVolumeAmbient);
			renderer.renderListBox(&listBoxVolumeMusic);
			renderer.renderLabel(&labelVolumeMusic);



		}

		renderer.renderConsole(&console);
		if (program != NULL)
			program->renderProgramMsgBox();
	}

	void
		MenuStateOptionsSound::saveConfig() {
		Config & config = Config::getInstance();
		Lang & lang = Lang::getInstance();
		setActiveInputLable(NULL);

		int
			FSoundIndex = listBoxSoundFactory.getSelectedItemIndex();
		string
			FSoundMode;
		switch (FSoundIndex) {
			case 0:
				FSoundMode = "None";
				break;
			case 1:
				FSoundMode = "OpenAL";
				break;
		}
		config.setString("FactorySound", FSoundMode);

		config.setString("SoundVolumeFx", listBoxVolumeFx.getSelectedItem());
		config.setString("SoundVolumeAmbient",
			listBoxVolumeAmbient.getSelectedItem());
		CoreData::getInstance().getMenuMusic()->
			setVolume(strToInt(listBoxVolumeMusic.getSelectedItem()) / 100.f);
		config.setString("SoundVolumeMusic",
			listBoxVolumeMusic.getSelectedItem());

		config.save();

#ifdef _WIN32
#ifndef SHOW_CONSOLE
		if (config.getBool("ShowConsoleWindows", "false") == true)
			ShowWindow(GetConsoleWindow(), SW_SHOW);
		else if (GlobalStaticFlags::getIsNonGraphicalModeEnabled() == false)
			ShowWindow(GetConsoleWindow(), SW_HIDE);
#endif
#endif

		SoundRenderer & soundRenderer = SoundRenderer::getInstance();
		soundRenderer.stopAllSounds();
		program->stopSoundSystem();
		soundRenderer.init(program->getWindow());
		soundRenderer.loadConfig();
		soundRenderer.setMusicVolume(CoreData::getInstance().getMenuMusic());
		program->startSoundSystem();

		if (CoreData::getInstance().hasMainMenuVideoFilename() == false) {
			soundRenderer.playMusic(CoreData::getInstance().getMenuMusic());
		}

		Renderer::getInstance().loadConfig();
		console.addLine(lang.getString("SettingsSaved"));
	}

	void
		MenuStateOptionsSound::setActiveInputLable(GraphicLabel * newLable) {
	}

} //end namespace
