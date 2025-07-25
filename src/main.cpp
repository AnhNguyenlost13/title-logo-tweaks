#include <Geode/modify/MenuLayer.hpp>
#include <Geode/modify/LoadingLayer.hpp>
#include <regex>

using namespace geode::prelude;

std::string customTitleLogo = "Geometry Dash";
std::string cachedEWDString = "Ruminative Dash";
int errorCode = 0;

bool setupTitleLogoReplacement(CCSprite* titleLogo)
{
    titleLogo->setOpacity(0);

    std::string temp = customTitleLogo;
    if (Mod::get()->getSettingValue<bool>("force-uppercase")) std::ranges::transform(temp, temp.begin(), [](const unsigned char c){ return std::toupper(c); });

    CCLabelBMFont* newTitleLogo = CCLabelBMFont::create(temp.c_str(), "merged_output.fnt"_spr);
    CCLabelBMFont* newTitleLogoUnderlay = CCLabelBMFont::create(temp.c_str(), "underlay_bigsheet.fnt"_spr);
    if (!newTitleLogo || !newTitleLogoUnderlay) return false;

    newTitleLogo->limitLabelWidth(450.f, 1.25f, 0.25f);
    newTitleLogo->setID("custom-main-title"_spr);
    newTitleLogo->setZOrder(1);
    titleLogo->addChild(newTitleLogo);

    newTitleLogoUnderlay->limitLabelWidth(450.f, 1.25f, 0.25f);
    newTitleLogoUnderlay->setID("custom-main-title-underlay"_spr);
    newTitleLogoUnderlay->setZOrder(0);
    titleLogo->addChild(newTitleLogoUnderlay);
    titleLogo->updateLayout();

    newTitleLogo->setPosition(titleLogo->getContentSize() / 2.f);
    newTitleLogo->setPosition(newTitleLogo->getPositionX() + 6.f, newTitleLogo->getPositionY() - 4.f);
    newTitleLogoUnderlay->setPosition(newTitleLogo->getPosition());

    return true;
}

class $modify(TLTLoadingLayer, LoadingLayer)
{
    struct Fields
    {
        EventListener<web::WebTask> m_listener;
    };

    static void onModify(auto& self)
    {
        if (Result<> res = self.setHookPriorityPre("LoadingLayer::init", Priority::Late); !res || res.isErr()) log::error("Failed to set hook priority: {}", res.unwrapErr());
    }

    $override bool init(const bool fromRefresh)
    {
        // Initial setup - only on the first load
        errorCode = 0; // oops

        if (!fromRefresh)
        {
            web::WebTask task = web::WebRequest().get("https://gdcolon.com/ewd_history.txt");
            while (task.isPending()) std::this_thread::sleep_for(std::chrono::milliseconds(1));
            if (const std::string rawResponse = task.getFinishedValue()->string().unwrapOr("what‼"); rawResponse == "what‼") errorCode += 1;
            else {
                // colon pls lmk if you change the formatting
                // GEOMETRY DASH REFERENCE??
                const auto startPos = rawResponse.find("-> ");
                if (const auto endPos = rawResponse.find("\n"); startPos != std::string::npos || endPos != std::string::npos)
                {
                    cachedEWDString = rawResponse.substr(startPos + 3);
                    cachedEWDString.erase(cachedEWDString.find_first_of("\n"), cachedEWDString.size());
                }
                else errorCode += 2;
            }

            if (errorCode > 1) cachedEWDString = "Ruminative Dash";
            std::ranges::transform(cachedEWDString, cachedEWDString.begin(), [this](const unsigned char c){ return std::toupper(c); });

            CCFileUtils::sharedFileUtils()->addSearchPath((Mod::get()->getTempDir() / "resources").string().c_str());
            CCTextureCache* textureCache = CCTextureCache::sharedTextureCache();
            (void) textureCache->addImage("merged_atlas.png"_spr, false);
            (void) textureCache->addImage("underlay_atlas.png"_spr, false);
            CCLabelBMFont::create("trans rights are human rights", "merged_output.fnt"_spr);
            CCLabelBMFont::create("i should be more careful with bets next time", "underlay_bigsheet.fnt"_spr);
        }

        if (Mod::get()->getSettingValue<bool>("every-word-dash-integration")) customTitleLogo = cachedEWDString;
        else customTitleLogo = Mod::get()->getSettingValue<std::string>("custom-title-logo");

        if (!LoadingLayer::init(fromRefresh)) return false;

        const auto titleLogo = typeinfo_cast<CCSprite*>(getChildByIDRecursive("gd-logo"));
        if (!titleLogo)
        {
            log::error("node 'gd-logo' is missing for some reason?");
            return true;
        }

        return setupTitleLogoReplacement(titleLogo);
    }
};

class $modify(TLTMenuLayer, MenuLayer) {
    $override bool init() override {
        if (!MenuLayer::init()) return false;

        const auto titleLogo = typeinfo_cast<CCSprite*>(getChildByIDRecursive("main-title"));
        if (!titleLogo) {
            log::error("whoever removed the node with ID 'main-title' from the MenuLayer, you are a menace to society!"); // i'm keeping this
            return true;
        }

        if (!Mod::get()->getSettingValue<bool>("every-word-dash-integration")) customTitleLogo = Mod::get()->getSettingValue<std::string>("custom-title-logo");
        else {
            if (errorCode > 1)
                Notification::create(
                    "Failed to fetch/parse EWD string (errcode " + std::to_string(errorCode) + ")",
                    NotificationIcon::Error,
                    1.f
                )->show();
            else customTitleLogo = cachedEWDString;
        }

        return setupTitleLogoReplacement(titleLogo);
    }
};