#include <Geode/modify/MenuLayer.hpp>
#include <Geode/modify/LoadingLayer.hpp>
#include <regex>

using namespace geode::prelude;

std::string customTitleLogo = "Geometry Dash";
std::string cachedEWDString = "Ruminative Dash";
int errorCode = 0;

bool setupTitleLogoReplacement(CCSprite* titleLogo)
{
    auto limitLabelWidth = [](CCLabelBMFont* label, const float width, const float defaultScale, const float minScale) {
        const auto originalWidth = label->getContentSize().width;
        auto scale = label->getScale();
        if (originalWidth > width && width > 0.0f) scale = width / originalWidth;
        if (minScale != 0.0f && minScale >= scale) scale = minScale;
        label->setScale(scale);
    };

    titleLogo->setOpacity(0);

    std::string temp = customTitleLogo;
    if (Mod::get()->getSettingValue<bool>("force-uppercase")) std::ranges::transform(temp, temp.begin(), [](const unsigned char c){ return std::toupper(c); });

    CCLabelBMFont* newTitleLogo = CCLabelBMFont::create(temp.c_str(), "merged_output.fnt"_spr);
    CCLabelBMFont* newTitleLogoUnderlay = CCLabelBMFont::create(temp.c_str(), "underlay_bigsheet.fnt"_spr);
    if (!newTitleLogo || !newTitleLogoUnderlay) return false;

    const float predeterminedWidth = CCDirector::sharedDirector()->getWinSize().width * 0.69f;
    limitLabelWidth(newTitleLogo, predeterminedWidth, 1.25f, 0.25f);
    newTitleLogo->setID("custom-main-title"_spr);
    newTitleLogo->setZOrder(1);
    titleLogo->addChild(newTitleLogo);

    limitLabelWidth(newTitleLogoUnderlay, predeterminedWidth, 1.25f, 0.25f);
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
        // darkmode "compatibility, thank you bitz"
        if (Result<> res = self.setHookPriorityAfterPost("LoadingLayer::init", "bitz.darkmode_v4"); !res || res.isErr()) log::error("Failed to set hook priority: {}", res.unwrapErr());
    }

    $override bool init(const bool fromRefresh)
    {
        // Initial setup - only on the first load
        errorCode = 0;
        std::string rawResponse = "ooh shiny";
        if (!fromRefresh)
        {
            // fetch the last string if the web req somehow didn't make it
            const auto savedCache = Mod::get()->getSavedValue<std::string>("cached-ewd-string");
            cachedEWDString = savedCache;
            if (savedCache.empty()) {
                errorCode += 1;
                cachedEWDString = "Ruminative Dash";
            }

            if (Mod::get()->getSettingValue<bool>("aggresive-prefetch"))
            {
                auto req = web::WebRequest().timeout(std::chrono::seconds(3));
                web::WebTask etask = req.get("https://raw.githubusercontent.com/AnhNguyenlost13/every-word-dash-api/refs/heads/master/badeline.txt");
                // hog the game fr
                while (etask.isPending()) std::this_thread::sleep_for(std::chrono::milliseconds(1));
                if (rawResponse = etask.getFinishedValue()->string().unwrapOr("."); rawResponse == ".") errorCode += 1;
            } else
            {
                // listener bind
                m_fields->m_listener.bind([rawResponse] (web::WebTask::Event* event) mutable
                    {
                        if (const web::WebResponse* response = event->getValue()) rawResponse = response->string().unwrapOr(".");
                        else if (event->isCancelled()) errorCode += 2;
                    }
                );

                // now we fetch (passively, don't hog the game)
                m_fields->m_listener.setFilter(web::WebRequest().get("https://raw.githubusercontent.com/AnhNguyenlost13/every-word-dash-api/refs/heads/master/badeline.txt"));
                if (rawResponse == "." || rawResponse.empty()) errorCode += 4;
                else
                {
                    cachedEWDString = rawResponse;
                    if (savedCache != cachedEWDString) Mod::get()->setSavedValue("cached-ewd-string", cachedEWDString);
                }
            }
            // listener already destroyed itself i think
            // capitalize the string
            std::ranges::transform(cachedEWDString, cachedEWDString.begin(), [](const unsigned char c){ return std::toupper(c); });

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