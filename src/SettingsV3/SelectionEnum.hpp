#pragma once
#include <Geode/loader/SettingV3.hpp>
#include <Geode/loader/Mod.hpp>
#include "popup.hpp"
#include "../jsonReader/Json.hpp"
#include <cctype>
#include <algorithm>
#include <fstream>
#include <iostream>

using namespace geode::prelude;

static struct ClicksoundSettingValue {
    std::string  m_currentMemeClick;
    std::string  m_currentClick;
    int m_tab = 0;
    std::string CustomSoundPath;

    // Comparison operator
    bool operator==(ClicksoundSettingValue const& other) const = default;

    // Implicit conversion to std::string
      operator std::string() const {
       return matjson::makeObject({
            {"Tab", m_tab},
            {"Current_Sound_Useful", m_currentClick},
            {"Current_Sound_Meme", m_currentMemeClick},
            {"Custom_Sound_Path", CustomSoundPath}
        }).dump(matjson::NO_INDENTATION);
    }

    // Constructors
    ClicksoundSettingValue() = default;

    ClicksoundSettingValue(int i1,  std::string_view i2,  std::string_view i3, std::string_view val)
        : m_tab(i1), m_currentClick(i2), m_currentMemeClick(i3), CustomSoundPath(val) {}

    ClicksoundSettingValue(ClicksoundSettingValue const&) = default;
};

template <>
struct matjson::Serialize<ClicksoundSettingValue> {

    static Result<ClicksoundSettingValue> fromJson(matjson::Value const& v) {
        GEODE_UNWRAP_INTO(std::string x, v.asString());
        if (x == "") {
           return Ok(ClicksoundSettingValue(0, " ", " ", " ")); 
        }
       try {
        auto value = matjson::parse(x).unwrap();
            
        return Ok(ClicksoundSettingValue(
            value["Tab"].asInt().unwrap(), 
            value["Current_Sound_Useful"].asString().unwrap(), 
            value["Current_Sound_Meme"].asString().unwrap(), 
            value["Custom_Sound_Path"].asString().unwrap()
        ));
        } catch (const std::exception& e) {
            return Ok(ClicksoundSettingValue(0, " ", " ", " "));
        }
    }

    static bool is_json(matjson::Value const& json) {
        return json.isString();
    }
};




class ClicksoundSetterV3 : public SettingBaseValueV3<ClicksoundSettingValue> {
public:
    bool clicksound = false;
    static Result<std::shared_ptr<SettingV3>> parse(std::string const& key, std::string const& modID, matjson::Value const& json) {
        auto res = std::make_shared<ClicksoundSetterV3>();
        auto root = checkJson(json, "selectionclicks");
        res->parseBaseProperties(key, modID, root);
        root.has("clicksound").into(res->clicksound);
        root.checkUnknownKeys();
        return root.ok(std::static_pointer_cast<SettingV3>(res));
    }

    SettingNodeV3* createNode(float width) override;
};

template <>
struct geode::SettingTypeForValueType<ClicksoundSettingValue> {
    using SettingType = ClicksoundSetterV3;
};

class ClicksoundSetterNodeV3 : public SettingValueNodeV3<ClicksoundSetterV3> {
protected:
    std::vector<CCMenuItemToggler*> m_toggles;
    CCMenuItemSpriteExtra* m_folderBtn;
    CCMenu* m_menufolder;
    CCMenu* m_selectionpopup;
    CCLabelBMFont* m_nameLabel;
    bool cs = false;
    void Popup(CCObject*) {
        auto popup = Select::create(static_cast<int>(this->getValue().m_tab) == 0,cs,[=](std::string modid) {
                ClicksoundSettingValue Changes = this->getValue();
                if (static_cast<int>(this->getValue().m_tab) == 0) {
                    Changes.m_currentMemeClick = modid;
                } else {
                    Changes.m_currentClick = modid;
                }
                this->setValue(Changes, nullptr);
        });
        popup->m_noElasticity = false;
        popup->show();
    };
    
    bool init(std::shared_ptr<ClicksoundSetterV3> setting, float width) {
        if (!SettingValueNodeV3::init(setting, width))
            return false;

        try {
            cs = setting->clicksound || false;
        } catch (const std::exception& e) {

        }

        this->setContentSize({ width, 70.f });
        CCSprite* folderSpr = CCSprite::createWithSpriteFrameName("gj_folderBtn_001.png");
        folderSpr->setScale(1.0f);
        m_folderBtn = CCMenuItemSpriteExtra::create(
            folderSpr,
            this,
            menu_selector(ClicksoundSetterNodeV3::onFolder)
        );
        m_nameLabel = CCLabelBMFont::create("", "bigFont.fnt");
        this->removeChild(this->getNameMenu(),false);
         this->removeChild(this->getButtonMenu(),false);
        this->getNameMenu()->setLayout(
            RowLayout::create()
            ->setAxisAlignment(AxisAlignment::Start)
            ->setCrossAxisLineAlignment(AxisAlignment::Start)
            ->setCrossAxisAlignment(AxisAlignment::Start)
        );
        
        this->addChildAtPosition(this->getNameMenu(), Anchor::TopLeft, ccp(10, 0), ccp(0, 1.0f));
        this->addChildAtPosition(this->getButtonMenu(), Anchor::TopRight, ccp(-10, 0), ccp(1.0f, 1.0f));
        this->getStatusLabel()->setPosition(this->getNameMenu()->getPosition() - ccp(0,this->getNameMenu()->getContentHeight() + 2));
        this->getStatusLabel()->setScale(0.175);
        this->getStatusLabel()->setAnchorPoint({0,1});
        this->getButtonMenu()->setScale(1.2);
        this->getNameMenu()->setScale(1.2);
        m_menufolder = CCMenu::create();
        m_menufolder->addChild(m_folderBtn);
        m_menufolder->setScale(1);
        m_menufolder->setLayout(RowLayout::create());
        m_menufolder->setPosition(ccp(this->getContentSize().width / 2, this->getContentSize().height / 2));
        this->addChild(m_menufolder);

        m_selectionpopup = CCMenu::create();
        auto btnspr = CCSprite::create("nobglogo.png"_spr);
        btnspr->setScale(0.4);
        auto nobglogobtn = CCMenuItemSpriteExtra::create(
            btnspr,
            this,
            menu_selector(ClicksoundSetterNodeV3::Popup)
        );
        m_selectionpopup->addChild(nobglogobtn);
        m_selectionpopup->setLayout(RowLayout::create());
        m_selectionpopup->setPosition(ccp(this->getContentSize().width / 2, this->getContentSize().height / 2));
        m_selectionpopup->setAnchorPoint({0.5,0.5});
        this->addChild(m_selectionpopup);

        m_nameLabel->setPosition(m_menufolder->getPosition() - ccp(0,m_menufolder->getContentSize().height));
        m_nameLabel->setScale(0.5);
        m_nameLabel->setAnchorPoint({0.5,0});
        this->addChild(m_nameLabel);

        
        int count = 0;
        for (auto value : {
            std::make_pair(0, "Meme"),
            std::make_pair(1, "Useful"),
            std::make_pair(2, "Custom")
        }) {
            count+=40;
            auto offSpr = ButtonSprite::create(value.second, 40.f, true, "bigFont.fnt", "GJ_button_04.png", 20.f, 1.0f);
            offSpr->setOpacity(90);
            auto onSpr = ButtonSprite::create(value.second, 40.f, true, "bigFont.fnt", "GJ_button_01.png", 20.f, 1.0f);
            auto toggle = CCMenuItemToggler::create(
                offSpr, onSpr, this, menu_selector(ClicksoundSetterNodeV3::onToggle)
            );
            toggle->m_notClickable = true;
            toggle->setTag(static_cast<int>(value.first));
            m_toggles.push_back(toggle);
            this->getButtonMenu()->addChild(toggle);
        }
        this->getButtonMenu()->setLayout(RowLayout::create());
        this->getButtonMenu()->updateLayout();
        this->updateState(nullptr);
        
        return true;
    }
    std::string GetJsonName(auto Infomation) {
        try {
            if (!Infomation.jsonpath.empty() && std::filesystem::exists(Infomation.jsonpath)) {
                std::filesystem::path fs = std::filesystem::path(Infomation.jsonpath);
                std::ifstream file(fs, std::ios::in | std::ios::binary);
                    if (file.is_open()) {
                        std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
                        file.close();
                        try {
                            matjson::Value jsonObject = matjson::parse(content).unwrap();

                            if (jsonObject.contains("name")) {
                                return jsonObject["name"].asString().unwrap();
                            } 
                        } catch (const std::exception& e) {
                            
                        };
                    };
                };
            }
            catch (const std::exception& e) {
                        
        };

        return "";
    };

    void updateState(CCNode* invoker) override {
        SettingValueNodeV3::updateState(invoker);
        float shouldEnable = this->getSetting()->shouldEnable();
        
        m_menufolder->setVisible(static_cast<int>(this->getValue().m_tab) == 2);
        m_selectionpopup->setVisible(static_cast<int>(this->getValue().m_tab) != 2);
        if (this->getValue().m_tab == 2) {
            std::error_code ec;
            auto Custompa = this->getValue().CustomSoundPath;
            if (Custompa.empty() || Custompa == " " || !std::filesystem::is_regular_file(Custompa, ec)) {
                m_nameLabel->setColor(ccGRAY);
                m_nameLabel->setOpacity(155);
                m_nameLabel->setString("");
            } else {
                std::filesystem::path filePath(Custompa);
                m_nameLabel->setString(filePath.filename().string().c_str());
                m_nameLabel->setColor(ccWHITE);
                m_nameLabel->setOpacity(255);
            }
        }
        if (this->getValue().m_tab == 1) {
            // current
            auto Custompa = this->getValue().m_currentClick;
            if (Custompa.empty() || Custompa == " ") {
                    m_nameLabel->setColor(ccGRAY);
                    m_nameLabel->setOpacity(155);
                    m_nameLabel->setString("");
                } else {
                    m_nameLabel->setString(GetJsonName(ClickJson->usefulData.at(Custompa)).c_str());
                    m_nameLabel->setColor(ccWHITE);
                    m_nameLabel->setOpacity(255);
                }
        }
        if (this->getValue().m_tab == 0) {
            // meme
            auto Custompa = this->getValue().m_currentMemeClick;
            if (Custompa.empty() || Custompa == " ") {
                    m_nameLabel->setColor(ccGRAY);
                    m_nameLabel->setOpacity(155);
                    m_nameLabel->setString("");
                } else {
                    m_nameLabel->setString(GetJsonName(ClickJson->memeData.at(Custompa)).c_str());
                    m_nameLabel->setColor(ccWHITE);
                    m_nameLabel->setOpacity(255);
                }
        }
        m_folderBtn->setEnabled(shouldEnable);
        if (!shouldEnable) {
           m_nameLabel->setColor(ccGRAY);
        }
        for (auto toggle : m_toggles) {
            if (shouldEnable) {
                toggle->toggle(toggle->getTag() == static_cast<int>(this->getValue().m_tab));
                toggle->setVisible(true);
            } else {
              (toggle->toggle(false)); 
              (toggle->setVisible(false));
            }
            toggle->setEnabled(shouldEnable);
            auto children = toggle->getChildren();
            for (auto children : CCArrayExt<CCNode*>(children)) {
                children->setScale(1);
            }
        }
    }
    void onToggle(CCObject* sender) {
        ClicksoundSettingValue Changes = this->getValue();
        Changes.m_tab = sender->getTag();
        this->setValue(Changes, static_cast<CCNode*>(sender));
    }
     void onFolder(CCObject* sender) {
        file::FilePickOptions::Filter textFilter;
        file::FilePickOptions fileOptions;
        textFilter.description = "Sound effect";
        textFilter.files = {"*.ogg", "*.mp3", "*.wav"};
        fileOptions.filters.push_back(textFilter);

        file::pick(file::PickMode::OpenFile, { Mod::get()->getResourcesDir(), { textFilter } }).listen([this,sender](Result<std::filesystem::path>* res) {
            if (res->isOk()) {
                    std::filesystem::path path = res->unwrap();
                    ClicksoundSettingValue Changes = this->getValue();
                    Changes.CustomSoundPath = path.string().c_str();
                    this->setValue(Changes, static_cast<CCNode*>(sender));
                }
            });
    }


public:
    static ClicksoundSetterNodeV3* create(std::shared_ptr<ClicksoundSetterV3> setting, float width) {
        auto ret = new ClicksoundSetterNodeV3();
        if (ret && ret->init(setting, width)) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};

SettingNodeV3* ClicksoundSetterV3::createNode(float width) {
    return ClicksoundSetterNodeV3::create(std::static_pointer_cast<ClicksoundSetterV3>(shared_from_this()), width);
}

$execute {
        auto ret = Mod::get()->registerCustomSettingType("selectionclicks", &ClicksoundSetterV3::parse);
        if (!ret) {
            log::error("Unable to register setting type: {}", ret.unwrapErr());
        }
}