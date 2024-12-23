#include <Geode/Geode.hpp>
#include <Geode/modify/PlayerObject.hpp>
#include <Geode/modify/PauseLayer.hpp>
#include <Geode/ui/GeodeUI.hpp>
#include <Geode/loader/SettingV3.hpp>
#include "SettingsV3/PlaySound.hpp"
#include "SettingsV3/SelectionEnum.hpp"
#include "StaticClasses.hpp"
#include "jsonReader/Getsettingsinfo.hpp"
#include "jsonReader/Json.hpp"
#include <thread>
#include <Geode/modify/MenuLayer.hpp>
#include <Geode/utils/web.hpp>
#include <Geode/loader/Event.hpp>
using namespace geode::prelude;

void onsettingsUpdate() {
    auto selection_release = GetSettingJsonRead("selection-release");
    ReleaseSound->Setsound(selection_release.Custom_Sound_Path);

    auto selection_clicks = GetSettingJsonRead("selection-clicks");
    ClickSound->Setsound(selection_clicks.Custom_Sound_Path);
    if (ClickJson->hassomedata) {

        Custom_OnClick = selection_clicks.M_Tab == 2;
        if (selection_clicks.M_Tab == 0) {
            auto list = ClickJson->GetMemeClicks();
            auto sound = list.find(selection_clicks.Current_Sound_Meme);
            if (sound != list.end()) {
                ClickSoundIndex->SetSounds(sound->second.clicks, "click-volume","selection-clicks");
            }
        } else {
            auto list = ClickJson->GetUsefulClicks();
            auto sound = list.find(selection_clicks.Current_Sound_Useful);
            if (sound != list.end()) {
                ClickSoundIndex->SetSounds(sound->second.clicks, "click-volume","selection-clicks");
            }
        }

        Custom_OnLetGo = selection_release.M_Tab == 2;
         if (selection_release.M_Tab == 0) {
            auto list = ClickJson->GetMemeReleases();
            auto sound = list.find(selection_release.Current_Sound_Meme);
            if (sound != list.end()) {
                ReleaseSoundIndex->SetSounds(sound->second.releases, "release-volume","selection-release");
            }
        } else {
            auto list = ClickJson->GetUsefulReleases();
            auto sound = list.find(selection_release.Current_Sound_Useful);
            if (sound != list.end()) {
                ReleaseSoundIndex->SetSounds(sound->second.releases, "release-volume","selection-release");
            }
        }

    }
}

// the check to see if you should play the sound or not
bool integrityCheck(PlayerObject* object, PlayerButton Pressed) {
    // play sounds when "only play on jump" settings is enabled and the player input is a jump, left movement, or right movement.
    if (Mod::get()->getSettingValue<bool>("only-on-jump")) {
        if (Pressed != PlayerButton::Jump) {
            return false;
        }
    }
    GJGameLevel* Level;
     if (!PlayLayer::get()) {
        if (!LevelEditorLayer::get()) {
            return false;
        }
        Level = LevelEditorLayer::get()->m_level;
     } else {
        PlayLayer* Pl = PlayLayer::get();
        // Mac fix
        if (Pl->m_isPaused) {
            return false;
        }
        Level = Pl->m_level;
     };
     if (object->m_isSecondPlayer && Level->m_twoPlayerMode || !object->m_isSecondPlayer) {
        return true;
     } else {
        return false;
     }
}

class $modify(PlayerObject) {
public:
        // add it to fields to access later but stored in the object (m_fields->Var)
    struct Fields {
         bool directionUp = false;
         bool directionRight = false;
         bool directionLeft = false;
    };
    
    // For setting bools for setting dir
    void SetupNewDirections(PlayerButton p0, bool Set) { 
        switch (p0) { 
            case PlayerButton::Jump:   m_fields->directionUp = Set; break; 
            case PlayerButton::Right:  m_fields->directionRight = Set; break; 
            case PlayerButton::Left:  m_fields->directionLeft = Set; break; 
            default:break; 
        } 
    }
    // getting the bools in a orderly way
    bool GetNewDirections(PlayerButton p0) { 
        switch (p0) { 
            case PlayerButton::Jump:   return m_fields->directionUp; break; 
            case PlayerButton::Right:  return m_fields->directionRight; break; 
            case PlayerButton::Left:  return m_fields->directionLeft; break; 
            default:break; 
        } 
        return false;
    }
    // click sounds
    bool pushButton(PlayerButton p0) {
        bool ret = PlayerObject::pushButton(p0);
        // check if you can and or check if it is correct
        if (!integrityCheck(this,p0)) {
            return ret;
        };
        auto isClickEnabled = Mod::get()->getSettingValue<bool>("enable-clicksounds");
        auto click_vol = Mod::get()->getSettingValue<int64_t>("click-volume");
        // set the direction bool to true
        SetupNewDirections(p0,true);
        
        // is it enabled or is volume < 0
        if (click_vol <= 0 || !isClickEnabled) return ret;
        
        // Sound Player
        if (Custom_OnClick) {
            ClickSound->Play();
        } else {
            ClickSoundIndex->PlayRandom();
        }

        return ret;
    }

    // release sounds
    bool releaseButton(PlayerButton p0) {
        bool ret = PlayerObject::releaseButton(p0);
        // Did you click? check
        if (!GetNewDirections(p0)) {
            return ret;
        };
        // check if you can and or check if it is correct
         if (!integrityCheck(this,p0)) {
            return ret;
        };

        auto isReleaseEnabled = Mod::get()->getSettingValue<bool>("enable-releasesounds");
        auto release_vol = Mod::get()->getSettingValue<int64_t>("release-volume");

        // set the direction bool to false
        SetupNewDirections(p0,false);
        // is it enabled or is volume < 0
        if (release_vol <= 0 || !isReleaseEnabled) return ret;

        // Sound Player
        if (Custom_OnLetGo) {
            ReleaseSound->Play();
        } else {
            ReleaseSoundIndex->PlayRandom();
        }

        return ret;
    }
};

EventListener<web::WebTask> m_listener;
class $modify(MenuLayer) {
    void SendRequestAPI() {
         m_listener.bind([] (web::WebTask::Event* e) {
                if (web::WebResponse* res = e->getValue()) {
                    if (res->string().unwrapOr("failed") == "failed") {
                        indexzip.Failed = true;
                        indexzip.Finished = true;
                        return;
                    }
                    if (res->into(Mod::get()->getConfigDir() / "Clicks.zip")) {
                        auto unzip = file::Unzip::create(Mod::get()->getConfigDir() / "Clicks.zip");
                        if (!unzip) {
                            indexzip.Failed = true;
                            indexzip.Finished = true;
                            return;
                        }
                        std::filesystem::remove_all(Mod::get()->getConfigDir() / "Clicks");
                        (void) unzip.unwrap().extractAllTo(Mod::get()->getConfigDir() / "Clicks");
                        indexzip.Finished = true;
                        ClickJson->loadData();
                        onsettingsUpdate();
                    }
                } else if (e->isCancelled()) {
                    indexzip.Failed = true;
                    indexzip.Finished = true;
                }
            });
            m_listener.setFilter(web::WebRequest().get("https://github.com/clicksounds/clicks/archive/refs/heads/main.zip"));
    }
    bool init() {
        if (!indexzip.StartedDownloading) {
            // on boot set Sound Caches
            ClickJson->loadData();
            onsettingsUpdate();

            indexzip.StartedDownloading = true;
            this->SendRequestAPI();
        }
        return MenuLayer::init();
    }
};


// on the mod loading
$execute {
    // Does the release-sound path setting change?
    listenForSettingChanges("selection-release", [](ClicksoundSettingValue releaseSoundFile) {
        onsettingsUpdate();
    });
    // Does the click-sound path setting change?
     listenForSettingChanges("selection-clicks", [](ClicksoundSettingValue PressSoundSoundFile) {
        onsettingsUpdate();
    });
}

